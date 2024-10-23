#include "uartDrive.h"
// #include "param.h"

/******************************************************************************
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void UART_DRIVE_DataStructInit(UART_DRIVE_CB *pCB);

// UART初始化
void UART_DRIVE_HwInit(uint32 baud);

// 发送处理
void UART_DRIVE_TxProcess(UART_DRIVE_CB *pCB);

// 启动中断字节发送
void UART_DRIVE_IR_StartSendData(uint8 data);

// 阻塞发送一个字节数据
void UART_DRIVE_BC_SendData(uint8 data);

// UART接收与发送中断处理，用于内部通讯
void USART1_IRQHandler(void);

// 全局变量定义
UART_DRIVE_CB uartDriveCB;

// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// UART初始化
void UART_DRIVE_Init(void)
{
	// 硬件UART配置
	UART_DRIVE_HwInit(UART_DRIVE_BAUD_RATE);

	// UART数据结构初始化
	UART_DRIVE_DataStructInit(&uartDriveCB);
}

void UART_DRIVE_InitSelect(uint32 baud)
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
void UART_DRIVE_DataStructInit(UART_DRIVE_CB *pCB)
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
	for (i = 0; i < UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}
}

// UART初始化
void UART_DRIVE_HwInit(uint32 baud)
{
	// GD32
	// 时钟配置
	rcu_periph_clock_enable(RCU_GPIOA);

	// GPIO配置
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_9); // 复用推挽

	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_10); // 上拉输入

	// UART时钟配置
	rcu_periph_clock_enable(RCU_USART0); // 时能USART时钟
	usart_deinit(UART_DRIVE_TYPE_DEF);	 // 复位串口

	// 时钟配置
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_14);

	// 串口电平切换 3.3/5V
	//	if (PARAM_GetUartLevel())
	//	{
	//		gpio_bit_reset(GPIOB, GPIO_PIN_14);
	//	}
	//	else
	//	{
	//		gpio_bit_set(GPIOB, GPIO_PIN_14);
	//	}

	// 串口配置
	usart_baudrate_set(UART_DRIVE_TYPE_DEF, baud);							// 波特率
	usart_word_length_set(UART_DRIVE_TYPE_DEF, USART_WL_8BIT);				// 8位数据位
	usart_stop_bit_set(UART_DRIVE_TYPE_DEF, USART_STB_1BIT);				// 一个停止位
	usart_parity_config(UART_DRIVE_TYPE_DEF, USART_PM_NONE);				// 无奇偶校验
	usart_hardware_flow_rts_config(UART_DRIVE_TYPE_DEF, USART_RTS_DISABLE); // 无硬件数据流控制
	usart_hardware_flow_cts_config(UART_DRIVE_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(UART_DRIVE_TYPE_DEF, USART_TRANSMIT_ENABLE); // 使能发射
	usart_receive_config(UART_DRIVE_TYPE_DEF, USART_RECEIVE_ENABLE);   // 使能接收

	// 中断配置
	nvic_irq_enable(UART_DRIVE_IRQn_DEF, 2, 1);

	usart_interrupt_enable(UART_DRIVE_TYPE_DEF, USART_INT_RBNE); // 开启中断

	usart_enable(UART_DRIVE_TYPE_DEF); // 使能串口
}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  串口接收函数，串口的中断发送函数
 * @output None
 * @return None
 * @Notes
 *************************************************************/
void USART0_IRQHandler(void)
{
	// 判断DR是否有数据，中断接收
	if (usart_interrupt_flag_get(UART_DRIVE_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET)
	{
		uint8 rxdata = 0x00;

		// 接收数据
		rxdata = (uint8)usart_data_receive(UART_DRIVE_TYPE_DEF);

		if (NULL != uartDriveCB.receiveDataThrowService)
		{
			(*uartDriveCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}
	}

// 中断模式发送
#if (UART_DRIVE_TX_MODE == UART_DRIVE_INTERRUPT_TX_MODE)
	// 判断DR是否有数据，中断发送
	if (usart_interrupt_flag_get(UART_DRIVE_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = uartDriveCB.tx.head;
		uint16 end;
		uint16 index = uartDriveCB.tx.index;
		uint8 txdata = 0x00;

		// 执行到这里，说明上一个数据已经发送完毕，当前命令帧未发送完时，取出一个字节放到发送寄存器中
		if (index < uartDriveCB.tx.cmdQueue[head].length)
		{
			txdata = uartDriveCB.tx.cmdQueue[head].buff[uartDriveCB.tx.index++];

			// 填充数据
			usart_data_transmit(UART_DRIVE_TYPE_DEF, txdata);
		}
		// 当前命令帧发送完时，删除之
		else
		{
			uartDriveCB.tx.cmdQueue[head].length = 0;
			uartDriveCB.tx.head++;
			uartDriveCB.tx.head %= UART_DRIVE_TX_QUEUE_SIZE;
			uartDriveCB.tx.index = 0;

			head = uartDriveCB.tx.head;
			end = uartDriveCB.tx.end;

			// 命令帧队列非空，继续发送下一个命令帧
			if (head != end)
			{
				txdata = uartDriveCB.tx.cmdQueue[head].buff[uartDriveCB.tx.index++];

				// 填充数据
				usart_data_transmit(UART_DRIVE_TYPE_DEF, txdata);
			}
			// 命令帧队列为空停止发送，设置空闲
			else
			{
				// 关闭发送空中断
				usart_interrupt_disable(UART_DRIVE_TYPE_DEF, USART_INT_TC);

				uartDriveCB.tx.txBusy = FALSE;
			}
		}
	}
#endif

	// Other USARTx interrupts handler can go here ...
	if (usart_interrupt_flag_get(UART_DRIVE_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- 接收溢出中断
	{
		usart_flag_get(UART_DRIVE_TYPE_DEF, USART_FLAG_ORERR); //----------------------- 清除接收溢出中断标志位
		usart_data_receive(UART_DRIVE_TYPE_DEF);			   //----------------------- 清空寄存器
	}
}

// 启动中断字节发送
void UART_DRIVE_IR_StartSendData(uint8 data)
{
	// 先读SR，再填充DR会把TC标志清掉
	usart_flag_get(UART_DRIVE_TYPE_DEF, USART_FLAG_TC);

	// 发送一个字节
	usart_data_transmit(UART_DRIVE_TYPE_DEF, data);

	// 打开发送完成中断
	usart_interrupt_enable(UART_DRIVE_TYPE_DEF, USART_INT_TC);
}

// 阻塞发送一个字节数据
void UART_DRIVE_BC_SendData(uint8 data)
{
	// 防止丢失第一个字节(清除空标志位,务必加)
	usart_flag_get(UART_DRIVE_TYPE_DEF, USART_FLAG_TC);

	// 填充数据
	usart_data_transmit(UART_DRIVE_TYPE_DEF, data);

	// 未发送完，持续等待
	while (usart_flag_get(UART_DRIVE_TYPE_DEF, USART_FLAG_TC) != SET)
		;
}

// 启动阻塞发送
void UART_DRIVE_BC_StartTx(UART_DRIVE_CB *pCB)
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
	while (index < pCB->tx.cmdQueue[head].length)
	{
		// 一直填充发送
		UART_DRIVE_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		index = pCB->tx.index;
	}

	// 当前命令帧发送完时，删除之
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head++;
	pCB->tx.head %= UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
}

// 发送处理,发现发送缓冲区非空时,启动中断发送
void UART_DRIVE_TxProcess(UART_DRIVE_CB *pCB)
{
// 中断方式
#if (UART_DRIVE_TX_MODE == UART_DRIVE_INTERRUPT_TX_MODE)
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
		UART_DRIVE_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		// 设置发送忙状态
		pCB->tx.txBusy = TRUE;
	}

// 阻塞方式时启动发送
#else
	UART_DRIVE_BC_StartTx(pCB);
#endif
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
	uartDriveCB.tx.end++;
	uartDriveCB.tx.end %= UART_DRIVE_TX_QUEUE_SIZE;
	uartDriveCB.tx.cmdQueue[uartDriveCB.tx.end].length = 0;

	return TRUE;
}
