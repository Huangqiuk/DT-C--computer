#include "n32g003.h"
#include "UartDrive.h"
#include <stdio.h>
#include "main.h"

/******************************************************************************
* 【内部接口声明】
******************************************************************************/

// 数据结构初始化
void UART_DRIVE_DataStructInit(UART_DRIVE_CB* pCB);

// UART初始化
void UART_DRIVE_HwInit(uint32 baud);

// 发送处理
void UART_DRIVE_TxProcess(UART_DRIVE_CB* pCB);

// 启动中断字节发送
void UART_DRIVE_IR_StartSendData(uint8 data);

// 阻塞发送一个字节数据
void UART_DRIVE_BC_SendData(uint8 data);

// UART接收与发送中断处理，用于内部通讯
void USART3_IRQHandler(void);

// 全局变量定义
UART_DRIVE_CB uartDriveCB;

// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// UART初始化
void UART_DRIVE_Init(uint32 baud)
{	
	// 硬件UART配置
	UART_DRIVE_HwInit(baud);

	// UART数据结构初始化
	UART_DRIVE_DataStructInit(&uartDriveCB);
}

// UART模块处理入口
void UART_DRIVE_Process(void)
{
	// 内部通讯接口的发送处理
	UART_DRIVE_TxProcess(&uartDriveCB);
}

// 向协议层注册数据接收接口
void UART_DRIVE_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	uartDriveCB.receiveDataThrowService = service;
}

// 数据结构初始化
void UART_DRIVE_DataStructInit(UART_DRIVE_CB* pCB)
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
	for(i = 0; i < UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}
}

//串口一初始化
//TX - B.0
//RX - B.1
//BaudRate - 115200
void UART_DRIVE_HwInit(uint32 baud)
{

    GPIO_InitType GPIO_InitStructure;
    UART_InitType UART_InitStructure;
    NVIC_InitType NVIC_InitStructure;
    
    //Clock Init
    RCC_APB_Peripheral_Clock_Enable(RCC_APB_PERIPH_IOPB);
    RCC_APB_Peripheral_Clock_Enable(RCC_APB_PERIPH_UART1);

    //GPIO Init
    /* Initialize GPIO_InitStructure */
    GPIO_Structure_Initialize(&GPIO_InitStructure);

    /* Configure UART1 Tx as alternate function push-pull */
    GPIO_InitStructure.Pin            = GPIO_PIN_0;
    GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF2_UART1;
    GPIO_Peripheral_Initialize(GPIOB, &GPIO_InitStructure);

    /* Configure UART1 Rx as alternate function push-pull */
    GPIO_InitStructure.Pin            = GPIO_PIN_1;
    GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_INPUT;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF2_UART1;
    GPIO_Peripheral_Initialize(GPIOB, &GPIO_InitStructure);

    /* UART1 configuration */
    UART_InitStructure.BaudRate   = baud;
    UART_InitStructure.WordLength = UART_WL_8B;
    UART_InitStructure.StopBits   = UART_STPB_1;
    UART_InitStructure.Parity     = UART_PE_NO;
    UART_InitStructure.Mode       = UART_MODE_RX | UART_MODE_TX;

    /* Configure UARTx */
    UART_Initializes(UART1, &UART_InitStructure);
    /* Enable the UARTx */
    UART_Enable(UART1);
    //------------------------------中断配置---------------------------------
    NVIC_InitStructure.NVIC_IRQChannel         = UART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = NVIC_PRIORITY_0;
    NVIC_InitStructure.NVIC_IRQChannelCmd      = ENABLE;
    NVIC_Initializes(&NVIC_InitStructure);

    /* Enable UART1 Receive and Transmit interrupts */
    UART_Interrput_Enable(UART1, UART_INT_RXDNE);
//    UART_Interrput_Enable(UART1, UART_INT_TXDE);
}

//---------------------中断函数--------------------

void UART1_IRQHandler(void)
{    
    //接受中断
    if (UART_Interrupt_Status_Get(UART1, UART_INT_RXDNE) != RESET)
    {
        uint8_t rxdata = 0;
        rxdata = (uint8)UART_Data_Receive(UART1);
        
		if (NULL != uartDriveCB.receiveDataThrowService)
		{			
			(*uartDriveCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}
    }
    
	// 中断模式发送
#if (UART_DRIVE_TX_MODE == UART_DRIVE_INTERRUPT_TX_MODE)
	// 判断DR是否有数据，中断发送
	if (UART_Interrupt_Status_Get(UART_DRIVE_TYPE_DEF, UART_INT_TXC) != RESET)
	{
		uint16 head = uartDriveCB.tx.head;
		uint16 end;
		uint16 index = uartDriveCB.tx.index;
		uint8 txdata = 0x00;

		// 清除发送中断标志
		UART_Interrupt_Status_Clear(UART_DRIVE_TYPE_DEF, UART_INT_TXC);

		// 执行到这里，说明上一个数据已经发送完毕，当前命令帧未发送完时，取出一个字节放到发送寄存器中
		if (index < uartDriveCB.tx.cmdQueue[head].length)
		{
			txdata = uartDriveCB.tx.cmdQueue[head].buff[uartDriveCB.tx.index++];
			
			// 填充数据
			UART_Data_Send(UART_DRIVE_TYPE_DEF, txdata);
		}
		// 当前命令帧发送完时，删除之
		else
		{
			uartDriveCB.tx.cmdQueue[head].length = 0;
			uartDriveCB.tx.head ++;
			uartDriveCB.tx.head %= UART_DRIVE_TX_QUEUE_SIZE;
			uartDriveCB.tx.index = 0;

			head = uartDriveCB.tx.head;
			end = uartDriveCB.tx.end;
			
			// 命令帧队列非空，继续发送下一个命令帧
			if (head != end)
			{
				txdata = uartDriveCB.tx.cmdQueue[head].buff[uartDriveCB.tx.index++];

				// 填充数据
				UART_Data_Send(UART_DRIVE_TYPE_DEF, txdata);
			}
			// 命令帧队列为空停止发送，设置空闲
			else
			{
				// 关闭发送完成中断
				UART_Interrput_Disable(UART_DRIVE_TYPE_DEF, UART_INT_TXC);
				
				uartDriveCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif

	// Other USARTx interrupts handler can go here ...				 
	if(UART_Interrupt_Status_Get(UART_DRIVE_TYPE_DEF, UART_FLAG_OREF) != RESET) //----------------------- 接收溢出中断 	
	{
		UART_Interrupt_Status_Clear(UART_DRIVE_TYPE_DEF,UART_FLAG_OREF); 		  //----------------------- 清除接收溢出中断标志位 
		UART_Data_Receive(UART_DRIVE_TYPE_DEF);						  //----------------------- 清空寄存器
	}
}

// 发送处理,发现发送缓冲区非空时,启动中断发送
void UART_DRIVE_TxProcess(UART_DRIVE_CB* pCB)
{
// 中断方式
#if (UART_DRIVE_TX_MODE == UART_DRIVE_INTERRUPT_TX_MODE)
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
		UART_DRIVE_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		// 设置发送忙状态
		pCB->tx.txBusy = TRUE;
	}


// 阻塞方式时启动发送
#else
	UART_DRIVE_BC_StartTx(pCB);
#endif
}

// 启动中断字节发送
void UART_DRIVE_IR_StartSendData(uint8 data)
{
	// 先读SR，再填充DR会把TC标志清掉
	//UART_GetITStatus(UART_DRIVE_TYPE_DEF, UART_IT_TC);

	// 发送一个字节
	UART_Data_Send(UART_DRIVE_TYPE_DEF, data);

	// 打开发送完成中断
	UART_Interrput_Enable(UART_DRIVE_TYPE_DEF, UART_INT_TXC);
}

// 阻塞发送一个字节数据
void UART_DRIVE_BC_SendData(uint8 data)
{	
	// 防止丢失第一个字节(清除空标志位,务必加) 
	//UART_ClearITPendingBit(UART_DRIVE_TYPE_DEF, UART_CLR_TC);		

	// 填充数据
	UART_Data_Send(UART_DRIVE_TYPE_DEF, data);

	// 未发送完，持续等待
	while(UART_Interrupt_Status_Get(UART_DRIVE_TYPE_DEF, UART_FLAG_TXC) == RESET);	
}

// 启动阻塞发送
void UART_DRIVE_BC_StartTx(UART_DRIVE_CB* pCB)
{
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
	while(index < pCB->tx.cmdQueue[head].length)
	{
		// 一直填充发送
		UART_DRIVE_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// 当前命令帧发送完时，删除之
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
}

// 向发送缓冲区中添加一条待发送序列
BOOL UART_DRIVE_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = uartDriveCB.tx.head;
	uint16 end = uartDriveCB.tx.end;
	
	// 参数检验
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// 发送缓冲区已满，不予接收
	if ((end + 1) % UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	uartDriveCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		uartDriveCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	uartDriveCB.tx.cmdQueue[end].length = length;
	
	// 发送环形队列更新位置
	uartDriveCB.tx.end ++;
	uartDriveCB.tx.end %= UART_DRIVE_TX_QUEUE_SIZE;
	uartDriveCB.tx.cmdQueue[uartDriveCB.tx.end].length = 0;

	return TRUE;
}


