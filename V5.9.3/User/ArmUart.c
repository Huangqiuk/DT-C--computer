#include "common.h"
#include "system.h"
#include "timer.h"
#include "ArmUart.h"
// #include "Param.h"
#include "ArmProtocol.h"

/******************************************************************************
 * 【内部接口声明】
 ******************************************************************************/
// UART初始化
void ARM_UART_HwInit(uint32 baud);

// 数据结构初始化
void ARM_UART_DataStructureInit(ARM_UART_CB *pCB);

// 发送处理
void ARM_UART_TxProcess(ARM_UART_CB *pCB);

// 启动中断字节发送
void ARM_UART_IR_StartSendData(uint8 data);

// 阻塞发送一个字节数据
void ARM_UART_BC_SendData(uint8 data);

// UART接收与发送中断处理，用于内部通讯
void USART0_IRQHandler(void);

// BLE模块辅助控制引脚
void ARM_CMD_GPIO_CtrlPin(void);

// 全局变量定义
ARM_UART_CB armUartCB;

// UART初始化
void ARM_UART_HwInit(uint32 baud)
{
	// 打开时钟
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

	// UART时钟配置
	rcu_periph_clock_enable(RCU_USART1);
	usart_deinit(ARM_UART_TYPE_DEF); // 复位串口

	// 串口配置
	usart_baudrate_set(ARM_UART_TYPE_DEF, baud);						  // 波特率
	usart_word_length_set(ARM_UART_TYPE_DEF, USART_WL_8BIT);			  // 8位数据位
	usart_stop_bit_set(ARM_UART_TYPE_DEF, USART_STB_1BIT);				  // 一个停止位
	usart_parity_config(ARM_UART_TYPE_DEF, USART_PM_NONE);				  // 无奇偶校验
	usart_hardware_flow_rts_config(ARM_UART_TYPE_DEF, USART_RTS_DISABLE); // 无硬件数据流控制
	usart_hardware_flow_cts_config(ARM_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(ARM_UART_TYPE_DEF, USART_TRANSMIT_ENABLE); // 使能发射
	usart_receive_config(ARM_UART_TYPE_DEF, USART_RECEIVE_ENABLE);	 // 使能接收

	// 中断配置
	nvic_irq_enable(ARM_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(ARM_UART_TYPE_DEF, USART_INT_RBNE); // 开启中断

	usart_enable(ARM_UART_TYPE_DEF); // 使能串口
}

// 数据结构初始化
void ARM_UART_DataStructureInit(ARM_UART_CB *pCB)
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
	for (i = 0; i < ARM_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	armUartCB.rcvBleMacOK = FALSE;
}

// 发送处理
void ARM_UART_TxProcess(ARM_UART_CB *pCB)
{
// 中断方式
#if (ARM_UART_TX_MODE == ARM_UART_INTERRUPT_TX_MODE)
	uint16 index = pCB->tx.index;						   // 当前发送数据的索引号
	uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length; // 当前发送的命令帧的长度
	uint16 head = pCB->tx.head;							   // 发送命令帧队列头索引号
	uint16 end = pCB->tx.end;							   // 发送命令帧队列尾索引号

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
		ARM_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		// 设置发送忙状态
		pCB->tx.txBusy = TRUE;
	}

// 阻塞方式时启动发送
#else
	uint16 index = pCB->tx.index;
	uint16 head = pCB->tx.head;
	uint16 end = pCB->tx.end;

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

	// 当前命令帧未发送完时，持续发送
	while (index < pCB->tx.cmdQueue[head].length)
	{
		// 一直填充发送
		ARM_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		index = pCB->tx.index;
	}

	// 当前命令帧发送完时，删除之
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head++;
	pCB->tx.head %= ARM_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

// 启动中断字节发送
void ARM_UART_IR_StartSendData(uint8 data)
{
	// 先读SR，再填充DR会把TC标志清掉
	usart_flag_get(ARM_UART_TYPE_DEF, USART_FLAG_TC);

	// 发送一个字节
	usart_data_transmit(ARM_UART_TYPE_DEF, data);

	// 打开发送完成中断
	usart_interrupt_enable(ARM_UART_TYPE_DEF, USART_INT_TC);
}

// 阻塞发送一个字节数据
void ARM_UART_BC_SendData(uint8 data)
{
	// 防止丢失第一个字节(清除空标志位,务必加)
	usart_flag_get(ARM_UART_TYPE_DEF, USART_FLAG_TC);

	// 填充数据
	usart_data_transmit(ARM_UART_TYPE_DEF, data);

	// 未发送完，持续等待
	while (usart_flag_get(ARM_UART_TYPE_DEF, USART_FLAG_TC) != SET)
		;
}

// 发送lock 应答
void BLE_PROTOCOL_SendLockOk(uint32 param)
{
	uint8 bleMacStr[] = "TTM:LOCK-OK";

	// 使能BLE发送
	ARM_BRTS_TX_REQUEST();

	// 将待发送命令添加到物理层缓冲区
	ARM_UART_AddTxArray(0xFFFF, bleMacStr, sizeof(bleMacStr));
}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  串口x接收函数，串口x的中断发送函数
 * @output None
 * @return None
 * @Notes
 *************************************************************/
uint8 armRxBuff[200] = {0};
uint8 callringBuff[20] = {0};
uint8 bleIndex = 0;

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  串口x接收函数，串口x的中断发送函数
 * @output None
 * @return None
 * @Notes
 *************************************************************/
void USART1_IRQHandler(void)
{
	// 判断DR是否有数据，中断接收
	if (usart_interrupt_flag_get(ARM_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET)
	{
		uint8 rxdata = 0x00;

		// 接收数据
		rxdata = (uint8)usart_data_receive(ARM_UART_TYPE_DEF);

		if (NULL != armUartCB.receiveDataThrowService)
		{
			(*armUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}

		if (!armUartCB.rcvBleMacOK)
		{
			//			BLE_UART_AtChannelDataProcess(rxdata);
		}

		//=================================================================
		// 蓝牙连接判断
		if ((bleIndex + 1) < sizeof(armRxBuff) / sizeof(armRxBuff[0]))
		{
			armRxBuff[bleIndex++] = rxdata;
			armRxBuff[bleIndex] = '\0';
		}
	}

// 中断模式发送
#if (ARM_UART_TX_MODE == ARM_UART_INTERRUPT_TX_MODE)
	// 判断DR是否有数据，中断发送
	if (usart_interrupt_flag_get(ARM_UART_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = armUartCB.tx.head;
		uint16 end;
		uint16 index = armUartCB.tx.index;
		uint8 txdata = 0x00;

		// 执行到这里，说明上一个数据已经发送完毕，当前命令帧未发送完时，取出一个字节放到发送寄存器中
		if (index < armUartCB.tx.cmdQueue[head].length)
		{
			txdata = armUartCB.tx.cmdQueue[head].buff[armUartCB.tx.index++];

			// 填充数据
			usart_data_transmit(ARM_UART_TYPE_DEF, txdata);
		}
		// 当前命令帧发送完时，删除之
		else
		{
			armUartCB.tx.cmdQueue[head].length = 0;
			armUartCB.tx.head++;
			armUartCB.tx.head %= ARM_UART_DRIVE_TX_QUEUE_SIZE;
			armUartCB.tx.index = 0;

			head = armUartCB.tx.head;
			end = armUartCB.tx.end;

			// 命令帧队列非空，继续发送下一个命令帧
			if (head != end)
			{
				txdata = armUartCB.tx.cmdQueue[head].buff[armUartCB.tx.index++];

				// 填充数据
				usart_data_transmit(ARM_UART_TYPE_DEF, txdata);
			}
			// 命令帧队列为空停止发送，设置空闲
			else
			{
				// 关闭发送完成中断
				usart_interrupt_disable(ARM_UART_TYPE_DEF, USART_INT_TC);

				armUartCB.tx.txBusy = FALSE;
			}
		}
	}
#endif

	// Other USARTx interrupts handler can go here ...
	if (usart_interrupt_flag_get(ARM_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- 接收溢出中断
	{
		usart_flag_get(ARM_UART_TYPE_DEF, USART_FLAG_ORERR); 							//----------------------- 清除接收溢出中断标志位
		usart_data_receive(ARM_UART_TYPE_DEF);				 							//----------------------- 清空寄存器
	}
}

// UART初始化
void ARM_UART_Init(void)
{
	// ARM串口初始化
	ARM_UART_HwInit(ARM_UART_BAUD_RATE);

	// ARM数据结构初始化
	ARM_UART_DataStructureInit(&armUartCB);
}

// UART模块处理入口
void ARM_UART_Process(void)
{
	// 发送处理
	ARM_UART_TxProcess(&armUartCB);
}

// 注册数据抛出接口服务
void ARM_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	armUartCB.receiveDataThrowService = service;
}

// 向发送缓冲区中添加一条待发送序列
BOOL ARM_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = armUartCB.tx.head;
	uint16 end = armUartCB.tx.end;

	// 参数检验
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// 发送缓冲区已满，不予接收
	if ((end + 1) % ARM_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	armUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		armUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	armUartCB.tx.cmdQueue[end].length = length;

	// 发送环形队列更新位置
	armUartCB.tx.end++;
	armUartCB.tx.end %= ARM_UART_DRIVE_TX_QUEUE_SIZE;
	armUartCB.tx.cmdQueue[armUartCB.tx.end].length = 0;

	return TRUE;
}

// ARM复位
void ARM_MODULE_Reset(uint32 param)
{
	// ARM复位
	ARM_RST_RESET();

	// 释放复位
	ARM_RST_RELEASE();

	ARM_EN_ENABLE();
}
