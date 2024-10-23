#include "common.h"
#include "delay.h"
#include "system.h"
#include "timer.h"
#include "BleUart.h"
#include "stringOperation.h"
#include "Spiflash.h"
#include "param.h"

/******************************************************************************
* 【内部接口声明】
******************************************************************************/
// UART初始化
void BLE_UART_HwInit(uint32 baud);

// 阻塞发送一个字节数据
void BLE_UART_BC_SendData(uint8 data);

// 启动中断字节发送
void BLE_UART_IR_StartSendData(uint8 data);

// UART接收与发送中断处理，用于内部通讯
void USART1_IRQHandler(void);

// 接收蓝牙mac地址
void BLE_UART_RcvBleMacAddr(uint8 data);

// 数据结构初始化
void BLE_UART_DataStructureInit(BLE_UART_CB* pCB);

// 发送处理
void BLE_UART_TxProcess(BLE_UART_CB* pCB);

// 全局变量定义
BLE_UART_CB bleUartCB;

// UART初始化
void BLE_UART_Init(void)
{
	// 由安卓端点击蓝牙升级时，应用层跳转boot需要屏蔽掉蓝牙模块初始化，防止蓝牙断开连接
	if(0xAA == SPI_FLASH_ReadByte(SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS + 1))
	{
		// 清除蓝牙升级时，跳转boot屏蔽初始化标志位，重启之后需要重新初始化蓝牙，防止升级失败
		SPI_FLASH_WriteByte(SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS + 1, 0xFF);
	}
	else
	{
		BLE_UART_HwInit(BLE_UART_BAUD_RATE);
	}
	
	// UART数据结构初始化
	BLE_UART_DataStructureInit(&bleUartCB);	
}

// UART初始化
void BLE_UART_HwInit(uint32 baud)
{
	// 打开时钟
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_9);
	
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_10);

	// UART时钟配置
	rcu_periph_clock_enable(RCU_USART0);
	usart_deinit(BLE_UART_TYPE_DEF);									// 复位串口
		
	// 串口配置
	usart_baudrate_set(BLE_UART_TYPE_DEF, baud);						// 波特率
	usart_word_length_set(BLE_UART_TYPE_DEF, USART_WL_8BIT);			// 8位数据位
	usart_stop_bit_set(BLE_UART_TYPE_DEF, USART_STB_1BIT); 				// 一个停止位
	usart_parity_config(BLE_UART_TYPE_DEF, USART_PM_NONE); 				// 无奇偶校验
	usart_hardware_flow_rts_config(BLE_UART_TYPE_DEF, USART_RTS_DISABLE); // 无硬件数据流控制
	usart_hardware_flow_cts_config(BLE_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(BLE_UART_TYPE_DEF, USART_TRANSMIT_ENABLE);	// 使能发射
	usart_receive_config(BLE_UART_TYPE_DEF, USART_RECEIVE_ENABLE); 		// 使能接收

	// 中断配置
	nvic_irq_enable(BLE_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(BLE_UART_TYPE_DEF, USART_INT_RBNE); 			// 开启中断

	usart_enable(BLE_UART_TYPE_DEF);									// 使能串口 

//====================================================================================================================

	rcu_periph_clock_enable(RCU_GPIOC);

	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_11);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_12);

	// 释放复位
	BLE_RST_RELEASE();
	// 允许开始广播
	BLE_EN_ENABLE();

	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_12);

	// 上电后一直处于发送准备状态
	gpio_bit_reset(GPIOC, GPIO_PIN_12);
}

// 阻塞发送一个字节数据
void BLE_UART_BC_SendData(uint8 data)
{	
	// 防止丢失第一个字节(清除空标志位,务必加) 
	usart_flag_get(BLE_UART_TYPE_DEF, USART_FLAG_TC);		

	// 填充数据
	usart_data_transmit(BLE_UART_TYPE_DEF, data);

	// 未发送完，持续等待
	while(usart_flag_get(BLE_UART_TYPE_DEF, USART_FLAG_TC) != SET);		
}

// 启动中断字节发送
void BLE_UART_IR_StartSendData(uint8 data)
{
	// 先读SR，再填充DR会把TC标志清掉
	usart_flag_get(BLE_UART_TYPE_DEF, USART_FLAG_TC);

	// 发送一个字节
	usart_data_transmit(BLE_UART_TYPE_DEF, data);

	// 打开发送完成中断
	usart_interrupt_enable(BLE_UART_TYPE_DEF, USART_INT_TC);	
}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  串口x接收函数，串口x的中断发送函数
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
uint8 bleRxBuff[20] = {0};
uint8 bleIndex = 0;
void BLE_UART_BleConnectState(uint32 param)
{	
	bleIndex = 0;

	if (-1 != STRING_Find(bleRxBuff, "TTM:CONNECTED"))
	{
		// 开启鉴权定时器，10s钟内如果没有鉴权成功则退出蓝牙连接
//		TIMER_AddTask(TIMER_ID_BLE_CONNECT_VERIFICATE,
//					60000,
//					BLE_MODULE_Reset,
//					0,
//					1,
//					ACTION_MODE_ADD_TO_QUEUE);
	}
}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  串口x接收函数，串口x的中断发送函数
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
void USART0_IRQHandler(void)
{
	// 判断DR是否有数据，中断接收
	if (usart_interrupt_flag_get(BLE_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET) 
	{
		uint8 rxData = 0x00;
		
		// 接收数据
		rxData = (uint8)usart_data_receive(BLE_UART_TYPE_DEF);

		// 手机app数据交互通道
		if (NULL != bleUartCB.receiveDataThrowService)
		{			
			(*bleUartCB.receiveDataThrowService)(0xFFF, &rxData, 1);
		}

		// 只有在注册的时候才允许解析AT指令
		if (paramCB.runtime.qrRegEnable)
		{
			BLE_UART_RcvBleMacAddr(rxData);
		}
		
//=================================================================
		// 蓝牙连接判断
		if ((bleIndex + 1)< sizeof(bleRxBuff)/sizeof(bleRxBuff[0]))
		{
			bleRxBuff[bleIndex++] = rxData;
			bleRxBuff[bleIndex] = '\0';
		}

		TIMER_AddTask(TIMER_ID_BLE_CONNECT_STATE,
					100,
					BLE_UART_BleConnectState,
					0,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
	}

// 中断模式发送
#if (BLE_UART_TX_MODE == BLE_UART_INTERRUPT_TX_MODE)
	// 判断DR是否有数据，中断发送
	if (usart_interrupt_flag_get(BLE_UART_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = bleUartCB.tx.head;
		uint16 end;
		uint16 index = bleUartCB.tx.index;
		uint8 txdata = 0x00;

		// 执行到这里，说明上一个数据已经发送完毕，当前命令帧未发送完时，取出一个字节放到发送寄存器中
		if (index < bleUartCB.tx.cmdQueue[head].length)
		{
			txdata = bleUartCB.tx.cmdQueue[head].buff[bleUartCB.tx.index++];
			
			// 填充数据
			usart_data_transmit(BLE_UART_TYPE_DEF, txdata);
		}
		// 当前命令帧发送完时，删除之
		else
		{
			bleUartCB.tx.cmdQueue[head].length = 0;
			bleUartCB.tx.head ++;
			bleUartCB.tx.head %= BLE_UART_DRIVE_TX_QUEUE_SIZE;
			bleUartCB.tx.index = 0;

			head = bleUartCB.tx.head;
			end = bleUartCB.tx.end;
			
			// 命令帧队列非空，继续发送下一个命令帧
			if (head != end)
			{
				txdata = bleUartCB.tx.cmdQueue[head].buff[bleUartCB.tx.index++];

				// 填充数据
				usart_data_transmit(BLE_UART_TYPE_DEF, txdata);
			}
			// 命令帧队列为空停止发送，设置空闲
			else
			{
				// 关闭发送完成中断
				usart_interrupt_disable(BLE_UART_TYPE_DEF, USART_INT_TC);
				bleUartCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif
	
	// Other USARTx interrupts handler can go here ...				 
	if (usart_interrupt_flag_get(BLE_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET)	//----------------------- 接收溢出中断 
	{
		usart_flag_get(BLE_UART_TYPE_DEF, USART_FLAG_ORERR); 			//----------------------- 清除接收溢出中断标志位 
		usart_data_receive(BLE_UART_TYPE_DEF);									//----------------------- 清空寄存器
	}
}

// 发送获取MAC地址
void BLE_UART_SendCmdGetBleMacAddr(uint32 param)
{
	uint8 i;
	uint8 bleMacStr[] = "TTM:MAC-?";
	
	// 使能BLE发送
	BLE_BRTS_TX_REQUEST();

	// 查询蓝牙mac指令,启动阻塞式发送
	for (i = 0; bleMacStr[i] != '\0'; i++)
	{
		// 填充数据，启动发送
		BLE_UART_BC_SendData(bleMacStr[i]);
	}	
}

// 接收蓝牙mac地址
void BLE_UART_RcvBleMacAddr(uint8 data)
{
	uint8 i;
	
	if (!bleUartCB.rx.startFlag)
	{
		if (data == 'T')				// 起始标志
		{
			bleUartCB.rx.startFlag = TRUE;

			bleUartCB.rx.index = 0;

			bleUartCB.rx.fifoBuff[bleUartCB.rx.index++] = data;		
			
			return;
		}
	}
	else
	{
		// 连续接收数据
		bleUartCB.rx.fifoBuff[bleUartCB.rx.index++] = data;

		// 越界判断
		if (bleUartCB.rx.index >= sizeof(bleUartCB.rx.fifoBuff))
		{
			bleUartCB.rx.startFlag = TRUE;
			bleUartCB.rx.index = 0;

			return;
		}
		
		if (data == '\n')			// 收到结束标志
		{
			bleUartCB.rx.startFlag = FALSE;
			bleUartCB.rx.index = 0;

			bleUartCB.rx.macBuff[2] = ':';
			bleUartCB.rx.macBuff[5] = ':';
			bleUartCB.rx.macBuff[8] = ':';
			bleUartCB.rx.macBuff[11] = ':';
			bleUartCB.rx.macBuff[14] = ':';
			bleUartCB.rx.macBuff[17] = '\0';

			// 过滤回显指令
			if ((strstr((const char*)bleUartCB.rx.fifoBuff, (const char*)"TTM:MAC-?") != NULL))
			{
				return ;
			}

			// 将读到的蓝牙mac地址提取出来
			if ((strstr((const char*)bleUartCB.rx.fifoBuff, (const char*)"TTM:MAC-") != NULL) 
			&& (bleUartCB.rx.fifoBuff[0] == 'T') && (bleUartCB.rx.fifoBuff[1] == 'T') && (bleUartCB.rx.fifoBuff[2] == 'M'))
			{
				bleUartCB.rx.macBuff[0] = bleUartCB.rx.fifoBuff[8];
				bleUartCB.rx.macBuff[1] = bleUartCB.rx.fifoBuff[9];

				bleUartCB.rx.macBuff[3] = bleUartCB.rx.fifoBuff[10];
				bleUartCB.rx.macBuff[4] = bleUartCB.rx.fifoBuff[11];

				bleUartCB.rx.macBuff[6] = bleUartCB.rx.fifoBuff[12];
				bleUartCB.rx.macBuff[7] = bleUartCB.rx.fifoBuff[13];

				bleUartCB.rx.macBuff[9] = bleUartCB.rx.fifoBuff[14];
				bleUartCB.rx.macBuff[10] = bleUartCB.rx.fifoBuff[15];

				bleUartCB.rx.macBuff[12] = bleUartCB.rx.fifoBuff[16];
				bleUartCB.rx.macBuff[13] = bleUartCB.rx.fifoBuff[17];

				bleUartCB.rx.macBuff[15] = bleUartCB.rx.fifoBuff[18];
				bleUartCB.rx.macBuff[16] = bleUartCB.rx.fifoBuff[19];

				//for (i = 0; i < 12; i++)
				//{
				//	bleUartCB.rx.macBuff[i] = bleUartCB.rx.fifoBuff[i+10];
				//}
				//bleUartCB.rx.macBuff[i] = '\0';
				
				for (i = 0; i < sizeof(bleUartCB.rx.fifoBuff); i++)
				{
					bleUartCB.rx.fifoBuff[i] = 0;
				}				

				// 注销该定时器
				TIMER_KillTask(TIMER_ID_BLE_CONTROL);

				// 蓝牙地址接收完成
				bleUartCB.rcvBleMacOK = TRUE;
			}	
		}
	}
}

// 定时器回调发送获取蓝牙地址命令
void BLE_UART_CALLBALL_GetBleMacAddrRequest(uint32 param)
{
	// 发送获取MAC地址
	BLE_UART_SendCmdGetBleMacAddr(TRUE);
}

// 数据结构初始化
void BLE_UART_DataStructureInit(BLE_UART_CB* pCB)
{
	uint16 i;
	
	// 参数合法性检验
	if (NULL == pCB)
	{
		return;
	}

	pCB->tx.txBusy = FALSE;
	pCB->tx.index = 0;
	pCB->tx.head = 0;
	pCB->tx.end = 0;
	for(i = 0; i < BLE_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}
}

// 发送处理
void BLE_UART_TxProcess(BLE_UART_CB* pCB)
{
// 中断方式
#if (BLE_UART_TX_MODE == BLE_UART_INTERRUPT_TX_MODE)
	uint16 index = pCB->tx.index;							// 当前发送数据的索引号
	uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length;	// 当前发送的命令帧的长度
	uint16 head = pCB->tx.head;								// 发送命令帧队列头索引号
	uint16 end = pCB->tx.end;								// 发送命令帧队列尾索引号

	// 参数合法性检验
	if (NULL == pCB)
	{
		return;
	}
	
	// 队列为空，不处理
	if (head == end)
	{
		return;
	}

	// 发送忙，退出
	if (pCB->tx.txBusy)
	{
		return;
	}

	// ■■执行到这里，说明队列非空■■
	
	// 当前命令帧未发送完时，取出一个字节放到发送寄存器中
	if (index < length)
	{		
		BLE_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		// 设置发送忙状态
		pCB->tx.txBusy = TRUE;
	}


// 阻塞方式时启动发送
#else
	uint16 index = pCB->tx.index;
	uint16 head = pCB->tx.head;
	uint16 end = pCB->tx.end;

	// 参数合法性检验
	if(NULL == pCB)
	{
		return;
	}

	// 队列为空，不处理
	if(head == end)
	{
		return;
	}
	
	// 当前命令帧未发送完时，持续发送
	while(index < pCB->tx.cmdQueue[head].length)
	{
		// 一直填充发送
		BLE_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// 当前命令帧发送完时，删除之
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= BLE_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

// UART模块处理入口
void BLE_UART_Process(void)
{
	// 发送处理
	BLE_UART_TxProcess(&bleUartCB);
}

// 注册数据抛出接口服务
void BLE_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	bleUartCB.receiveDataThrowService = service;
}

// 向发送缓冲区中添加一条待发送序列
BOOL BLE_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = bleUartCB.tx.head;
	uint16 end = bleUartCB.tx.end;
	
	// 参数检验
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// 发送缓冲区已满，不予接收
	if((end + 1) % BLE_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	bleUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		bleUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	bleUartCB.tx.cmdQueue[end].length = length;
	
	// 发送环形队列更新位置
	bleUartCB.tx.end ++;
	bleUartCB.tx.end %= BLE_UART_DRIVE_TX_QUEUE_SIZE;
	bleUartCB.tx.cmdQueue[bleUartCB.tx.end].length = 0;

	return TRUE;
}

// BLE模块复位
void BLE_MODULE_Reset(uint32 param)
{
	// 蓝牙复位
	BLE_RST_RESET();
	Delayms(10);
	
	// 释放复位
	BLE_RST_RELEASE();

	// 允许开始广播
	BLE_EN_ENABLE();
}

