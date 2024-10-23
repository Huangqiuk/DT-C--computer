#include "common.h"
#include "timer.h"
#include "BleHostUart.h"
//#include "stringOperation.h"
#include "Param.h"
#include "BleHostProtocol.h"

/******************************************************************************
* 【内部接口声明】
******************************************************************************/
// UART初始化
void BLE_HOST_UART_HwInit(uint32 baud);

// 数据结构初始化
void BLE_HOST_UART_DataStructureInit(BLE_HOST_UART_CB* pCB);

// 发送处理
void BLE_HOST_UART_TxProcess(BLE_HOST_UART_CB* pCB);

// 启动中断字节发送
void BLE_HOST_UART_IR_StartSendData(uint8 data);

//  蓝牙连接状态判断函数
void BLE_HOST_UART_BleConnectState(uint32 param);

// 阻塞发送一个字节数据
//void BLE_HOST_UART_BC_SendData(uint8 data);

// UART接收与发送中断处理，用于内部通讯
void UART2_Handler(void);

// 全局变量定义
BLE_HOST_UART_CB bleHostUartCB;

// UART初始化
void BLE_HOST_UART_HwInit(uint32 baud)
{
		GPIO_InitType GPIO_InitStructure;
		USART_InitType USART_InitStructure;
		NVIC_InitType NVIC_InitStructure;
	
		RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB|RCC_APB2_PERIPH_AFIO, ENABLE);    
		RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART3, ENABLE);                       //                                      
	

    // Initialize GPIO_InitStructure
    GPIO_InitStruct(&GPIO_InitStructure);
    // USART Tx 
    GPIO_InitStructure.Pin            = GPIO_PIN_10;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_USART3;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    // USART Rx 
    GPIO_InitStructure.Pin            = GPIO_PIN_11;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Input;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_USART3;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    // USART configuration
    USART_InitStructure.BaudRate            = baud;
    USART_InitStructure.WordLength          = USART_WL_8B;
    USART_InitStructure.StopBits            = USART_STPB_1;
    USART_InitStructure.Parity              = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;
    USART_Init(BLE_HOST_UART_TYPE_DEF, &USART_InitStructure);

    // Enable the USART Interrupt
    NVIC_InitStructure.NVIC_IRQChannel            = BLE_HOST_UART_IRQn_DEF;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable USART Receive and Transmit interrupts   
    USART_ConfigInt(BLE_HOST_UART_TYPE_DEF, USART_INT_RXDNE, ENABLE);    
		
    // USART_ConfigInt(UART_DRIVE_TYPE_DEF, USART_INT_TXC, ENABLE);    
    // Enable the USART
    USART_Enable(BLE_HOST_UART_TYPE_DEF, ENABLE);
		
}	

// 数据结构初始化
void BLE_HOST_UART_DataStructureInit(BLE_HOST_UART_CB* pCB)
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
	for(i = 0; i < BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}	

	bleHostUartCB.rcvBleMacOK = FALSE;
}

// 发送处理
void BLE_HOST_UART_TxProcess(BLE_HOST_UART_CB* pCB)
{
	// 中断方式
#if (BLE_HOST_UART_TX_MODE == BLE_HOST_UART_INTERRUPT_TX_MODE)
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
		BLE_HOST_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
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
		BLE_HOST_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// 当前命令帧发送完时，删除之
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

// 启动中断字节发送
void BLE_HOST_UART_IR_StartSendData(uint8 data)
{
	// 发送一个字节
	USART_SendData(BLE_HOST_UART_TYPE_DEF, data);

	// 打开发送完成中断
	USART_ConfigInt(BLE_HOST_UART_TYPE_DEF, USART_INT_TXC, ENABLE);
}


// 阻塞发送一个字节数据
void BLE_HOST_UART_BC_SendData(uint8 data)
{	
	// 填充数据
	USART_SendData(BLE_HOST_UART_TYPE_DEF, data);

	// 未发送完，持续等待
	while(USART_GetFlagStatus(BLE_HOST_UART_TYPE_DEF, USART_FLAG_TXC) == RESET);	
}

/**************************************************************
 * @brief  void BLE_HOST_UART_BleConnectState(void);
 * @input  蓝牙连接状态判断函数
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
uint8 bleHostRxBuff[200] = {0};
uint8 bleHostIndex = 0;
void BLE_HOST_UART_BleConnectState(uint32 param)
{	
	// 解析TTM命令
	BLE_HOST_UART_TTM_AnalysisCmd(bleHostRxBuff, bleHostIndex);

	bleHostIndex = 0;
}

/**************************************************************
 * @brief  void UART2_Handler(void);
 * @input  串口x接收函数，串口x的中断发送函数
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
void USART3_IRQHandler(void)
{
	// 判断DR是否有数据，中断接收
	if (USART_GetIntStatus(BLE_HOST_UART_TYPE_DEF, USART_INT_RXDNE) != RESET)
	{
		uint8 rxdata = 0x00;
		uint32_t chr;
		
		// 接收数据
		rxdata = (uint8)USART_ReceiveData(BLE_HOST_UART_TYPE_DEF);

		if (NULL != bleHostUartCB.receiveDataThrowService)
		{			
			(*bleHostUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}
		
//=================================================================
		// 蓝牙连接判断
		if ((bleHostIndex + 1)< sizeof(bleHostRxBuff)/sizeof(bleHostRxBuff[0]))
		{
			bleHostRxBuff[bleHostIndex++] = rxdata;
			bleHostRxBuff[bleHostIndex] = '\0';
		}

		TIMER_AddTask(TIMER_ID_BLE_HOST_CONNECT_STATE,
					10,
					BLE_HOST_UART_BleConnectState,
					0,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
	}


// 中断模式发送
#if (BLE_HOST_UART_TX_MODE == BLE_HOST_UART_INTERRUPT_TX_MODE)
	// 判断DR是否有数据，中断发送
	if (USART_GetIntStatus(BLE_HOST_UART_TYPE_DEF, USART_INT_TXC) != RESET)
	{
		uint16 head = bleHostUartCB.tx.head;
		uint16 end;
		uint16 index = bleHostUartCB.tx.index;
		uint8 txdata = 0x00;

		// 清除发送中断标志
		USART_ClrIntPendingBit(BLE_HOST_UART_TYPE_DEF, USART_INT_TXC);

		// 执行到这里，说明上一个数据已经发送完毕，当前命令帧未发送完时，取出一个字节放到发送寄存器中
		if (index < bleHostUartCB.tx.cmdQueue[head].length)
		{
			txdata = bleHostUartCB.tx.cmdQueue[head].buff[bleHostUartCB.tx.index++];
			
			// 填充数据
			USART_SendData(BLE_HOST_UART_TYPE_DEF, txdata);
		}
		// 当前命令帧发送完时，删除之
		else
		{
			bleHostUartCB.tx.cmdQueue[head].length = 0;
			bleHostUartCB.tx.head ++;
			bleHostUartCB.tx.head %= BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE;
			bleHostUartCB.tx.index = 0;

			head = bleHostUartCB.tx.head;
			end = bleHostUartCB.tx.end;
			
			// 命令帧队列非空，继续发送下一个命令帧
			if (head != end)
			{
				txdata = bleHostUartCB.tx.cmdQueue[head].buff[bleHostUartCB.tx.index++];

				// 填充数据
				USART_SendData(BLE_HOST_UART_TYPE_DEF, txdata);
			}
			// 命令帧队列为空停止发送，设置空闲
			else
			{
				// 关闭发送完成中断
				USART_ConfigInt(BLE_HOST_UART_TYPE_DEF, USART_INT_TXC, DISABLE);
				
				bleHostUartCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif
	// Other USARTx interrupts handler can go here ...				 
	if(USART_GetFlagStatus(BLE_HOST_UART_TYPE_DEF, USART_FLAG_OREF) != RESET) //----------------------- 接收溢出中断 	
	{
		USART_ClrFlag(BLE_HOST_UART_TYPE_DEF,USART_FLAG_OREF); 		  //----------------------- 清除接收溢出中断标志位 
		USART_ReceiveData(BLE_HOST_UART_TYPE_DEF);						  //----------------------- 清空寄存器
	}
}


// UART初始化
void BLE_HOST_UART_Init(void)
{
	// 串口初始化
	BLE_HOST_UART_HwInit(BLE_HOST_UART_BAUD_RATE);

	// UART数据结构初始化
	BLE_HOST_UART_DataStructureInit(&bleHostUartCB);
//	
//				TIMER_AddTask(TIMER_ID_BLE_HELMET_PAIR,
//							BLE_OUTFIT_CONNECT_TIME,
//							BLE_HOST_UART_CALLBALL_OutfitConnect,
//							TRUE,
//							BLE_OUTFIT_PAIR_COUNT,
//							ACTION_MODE_DO_AT_ONCE);
}

// UART模块处理入口
void BLE_HOST_UART_Process(void)
{
	// 发送处理
	BLE_HOST_UART_TxProcess(&bleHostUartCB);
}

// 注册数据抛出接口服务
void BLE_HOST_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	bleHostUartCB.receiveDataThrowService = service;
}

// 向发送缓冲区中添加一条待发送序列
BOOL BLE_HOST_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = bleHostUartCB.tx.head;
	uint16 end = bleHostUartCB.tx.end;
	
	// 参数检验
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// 发送缓冲区已满，不予接收
	if((end + 1) % BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	bleHostUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		bleHostUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	bleHostUartCB.tx.cmdQueue[end].length = length;
	
	// 发送环形队列更新位置
	bleHostUartCB.tx.end ++;
	bleHostUartCB.tx.end %= BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE;
	bleHostUartCB.tx.cmdQueue[bleHostUartCB.tx.end].length = 0;

	return TRUE;
}
