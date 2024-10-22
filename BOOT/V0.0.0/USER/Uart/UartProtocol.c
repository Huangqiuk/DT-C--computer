#include "main.h"

/******************************************************************************
* 【内部接口声明】
******************************************************************************/

// 数据结构初始化
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB* pCB);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB* pCB);

// UART命令帧缓冲区处理
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB* pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB* pCB);

// 通讯超时处理-单向
void UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// 停止RX通讯超时检测任务
void UART_PROTOCOL_StopRxTimeOutCheck(void);

// 协议层发送处理过程
void UART_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// 码表设置周期下发数据请求标志
void UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param);

// 码表发送数据通讯时序处理
void UART_PROTOCOL_DataCommunicationSequenceProcess(void);

// 启动通讯超时判断任务
void UART_PROTOCOL_StartTimeoutCheckTask(void);

// UART总线超时错误处理
void UART_PROTOCOL_CALLBACK_UartBusError(uint32 param);

//=========================================================================
// 发送命令带结果
void UART_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result);

// 发送按键值
void UART_PROTOCOL_SendCmdCheckKeyValue(uint32 param);

// 发送命令带结果
void UART_PROTOCOL_SendLdoV(uint8 cmdWord, uint32 result);

// 全局变量定义
UART_PROTOCOL_CB uartProtocolCB;

// 启动通讯超时判断任务
void UART_PROTOCOL_StartTimeoutCheckTask(void)
{
	// 清除通讯异常
//	if(ERROR_TYPE_COMMUNICATION_TIME_OUT == PARAM_GetErrorCode())
//	{
//		PARAM_SetErrorCode(ERROR_TYPE_NO_ERROR);
//	}

//	// 开启UART通讯超时定时器
//#if __SYSTEM_NO_TIME_OUT_ERROR__		// 研发调试时不启用
//#else
//	TIMER_AddTask(TIMER_ID_PROTOCOL_TXRX_TIME_OUT,
//					PARAM_GetBusAliveTime(),
//					UART_PROTOCOL_CALLBACK_UartBusError,
//					TRUE,
//					1,
//					ACTION_MODE_ADD_TO_QUEUE);
//#endif

	// 到此处证明通信与命令帧符合要求，则设置电量获取方式正常
//	PARAM_SetBatteryDataSrc((BATTERY_DATA_SRC_E)PARAM_GetPercentageMethod());
}


// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// 协议初始化
void UART_PROTOCOL_Init(void)
{
	// 协议层数据结构初始化
	UART_PROTOCOL_DataStructInit(&uartProtocolCB);

	// 向驱动层注册数据接收接口
	UART_DRIVE_RegisterDataSendService(UART_PROTOCOL_MacProcess);

	// 向驱动层注册数据发送接口
	UART_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);

//	// 注册间隔参数设置时间
//	TIMER_AddTask(TIMER_ID_PROTOCOL_PARAM_TX,
//					UART_PROTOCOL_CMD_SEND_TIME,
//					UART_PROTOCOL_CALLBACK_SetTxPeriodRequest,
//					TRUE,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);

//	// 设置参数层参数周期更新时间
//	PARAM_SetParamCycleTime(UART_PROTOCOL_CMD_SEND_TIME);

	// 启动UART总线通讯超时判断
//	UART_PROTOCOL_StartTimeoutCheckTask();

//	// 启动速度显示滤波控制定时器 (暂时测试屏蔽)
//	TIMER_AddTask(TIMER_ID_PROTOCOL_SPEED_FILTER,
//					SMOOTH_BASE_TIME,
//					UART_PROTOCOL_CALLBACK_SpeedFilterProcess,
//					TRUE,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);
}

// UART协议层过程处理
void UART_PROTOCOL_Process(void)
{
	// UART接收FIFO缓冲区处理
	UART_PROTOCOL_RxFIFOProcess(&uartProtocolCB);

	// UART接收命令缓冲区处理
	UART_PROTOCOL_CmdFrameProcess(&uartProtocolCB);
	
	// UART协议层发送处理过程
	UART_PROTOCOL_TxStateProcess();

}

// 向发送命令帧队列中添加数据
void UART_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = uartProtocolCB.tx.head;
	uint16 end =  uartProtocolCB.tx.end;
	UART_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];

	// 发送缓冲区已满，不予接收
	if((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 队尾命令帧已满，退出
	if(pCmdFrame->length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 数据添加到帧末尾，并更新帧长度
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void UART_PROTOCOL_TxAddFrame(void)
{
	uint16 checkSum = 0;
	uint16 i = 0;
	uint16 head = uartProtocolCB.tx.head;
	uint16 end  = uartProtocolCB.tx.end;
	UART_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// 发送缓冲区已满，不予接收
	if((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 命令帧长度不足，清除已填充的数据，退出
	if(UART_PROTOCOL_CMD_FRAME_LENGTH_MIN-4 > length)	// 减去"校验和L、校验和H、结束标识0xD、结束标识OxA"4个字节
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// 队尾命令帧已满，退出
	if((length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length+1 >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	|| (length+2 >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length+3 >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
	pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] = length - 4;	// 重设数据长度，减去"命令头、设备地址、命令字、数据长度"4个字节
	for(i=1; i<length; i++)
	{
		checkSum += pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);			// 低字节在前
	pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF);	// 高字节在后

	// 结束标识
	pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
	pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

	uartProtocolCB.tx.end ++;
	uartProtocolCB.tx.end %= UART_PROTOCOL_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

// 数据结构初始化
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB* pCB)
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
	for(i = 0; i < UART_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<UART_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART报文接收处理函数(注意根据具体模块修改)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length)
{
	uint16 end = uartProtocolCB.rxFIFO.end;
	uint16 head = uartProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;
	
	// 接收数据
	rxdata = *pData;

	// 一级缓冲区已满，不予接收
	if((end + 1)%UART_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// 一级缓冲区未满，接收 
	else
	{
		// 将接收到的数据放到临时缓冲区中
		uartProtocolCB.rxFIFO.buff[end] = rxdata;
		uartProtocolCB.rxFIFO.end ++;
		uartProtocolCB.rxFIFO.end %= UART_PROTOCOL_RX_FIFO_SIZE;
	}	
}

// UART协议层向驱动层注册数据发送接口
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{		
	uartProtocolCB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB* pCB)
{
	UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
	// 参数合法性检验
	if(NULL == pCB)
	{
		return FALSE;
	}

	// 临时缓冲区为空，不予添加
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	if(0 == pCmdFrame->length)
	{
		return FALSE;
	}

	// 添加
	pCB->rx.end ++;
	pCB->rx.end %= UART_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区
	
	return TRUE;
}

// 协议层发送处理过程
void UART_PROTOCOL_TxStateProcess(void)
{
	uint16 head = uartProtocolCB.tx.head;
	uint16 end =  uartProtocolCB.tx.end;
	uint16 length = uartProtocolCB.tx.cmdQueue[head].length;
	uint8* pCmd = uartProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = uartProtocolCB.tx.cmdQueue[head].deviceID;

	// 发送缓冲区为空，说明无数据
	if (head == end)
	{
		return;
	}

	// 发送函数没有注册直接返回
	if (NULL == uartProtocolCB.sendDataThrowService)
	{
		return;
	}

	// 协议层有数据需要发送到驱动层		
	if (!(*uartProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// 发送环形队列更新位置
	uartProtocolCB.tx.cmdQueue[head].length = 0;
	uartProtocolCB.tx.head ++;
	uartProtocolCB.tx.head %= UART_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	uint16 length = 0;
	uint8 currentData = 0;
	
	// 参数合法性检验
	if(NULL == pCB)
	{
		return;
	}
	
	// 一级缓冲区为空，退出
	if(head == end)
	{
		return;
	}

	// 获取临时缓冲区指针
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	// 取出当前要处理的字节
	currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
	
	// 临时缓冲区长度为0时，搜索首字节
	if(0 == pCmdFrame->length)
	{
		// 命令头错误，删除当前字节并退出
		if(UART_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// 命令头正确，但无临时缓冲区可用，退出
		if((pCB->rx.end + 1)%UART_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// 添加UART通讯超时时间设置-2016.1.5增加
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
						UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
						UART_PROTOCOL_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;
	}
	// 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
	else
	{
		// 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
		if(pCmdFrame->length >= UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			UART_PROTOCOL_StopRxTimeOutCheck();
#endif

			// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
			pCmdFrame->length = 0;	// 2016.1.5增加
			// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// 一直取到末尾
		while(end != pCB->rxFIFO.currentProcessIndex)
		{
			// 取出当前要处理的字节
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			// 缓冲区未溢出，正常接收，将数据添加到临时缓冲区中
			pCmdFrame->buff[pCmdFrame->length++] = currentData;
			pCB->rxFIFO.currentProcessIndex ++;
			pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;

			// ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■
			
			// 首先判断命令帧最小长度，一个完整的命令字至少包括8个字节: 命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，因此不足8个字节的必定不完整
			if(pCmdFrame->length < UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 继续接收
				continue;
			}

			// 命令帧长度数值越界，说明当前命令帧错误，停止接收
			if(pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] > UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX-UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				UART_PROTOCOL_StopRxTimeOutCheck();
#endif
			
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// 命令帧长度校验，在命令长度描述字的数值上，增加命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，即为命令帧实际长度
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] + UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 长度要求不一致，说明未接收完毕，退出继续
				continue;
			}

			// 命令帧长度OK，则进行校验，失败时删除命令头
			if(!UART_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				UART_PROTOCOL_StopRxTimeOutCheck();
#endif
				
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			UART_PROTOCOL_StopRxTimeOutCheck();
#endif
			
			// 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			UART_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// 对传入的命令帧进行校验，返回校验结果
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame)
{
	uint16 checkSum = 0;
	uint16 sumTemp;
	uint16 i = 0;
	
	if(NULL == pCmdFrame)
	{
		return FALSE;
	}

	// 从设备地址开始，到校验码之前的一个字节，依次进行累加运算
	for(i=1; i<pCmdFrame->length-4; i++)
	{
		checkSum += pCmdFrame->buff[i];
	}

	// 累加和，低字节在前，高字节在后
	sumTemp = pCmdFrame->buff[pCmdFrame->length-3];		// 高字节
	sumTemp <<= 8;
	sumTemp += pCmdFrame->buff[pCmdFrame->length-4];	// 低字节
	
	// 判断计算得到的校验码与命令帧中的校验码是否相同
	if(sumTemp != checkSum)
	{
		return FALSE;
	}
	
	return TRUE;
}

// UART命令帧缓冲区处理
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB* pCB)
{
	UART_PROTOCOL_CMD cmd = UART_PROTOCOL_CMD_NULL;
	UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	uint32 temp = 0;
	uint32 temp2= 0;
	uint16 length = 0;
	uint8 errStatus;
	uint16 i;
	BOOL bTemp;
	uint8* updateBuff = NULL;
	uint8 localBuf[50];
	
	// 参数合法性检验
	if(NULL == pCB)
	{
		return;
	}

	// 命令帧缓冲区为空，退出
	if(pCB->rx.head == pCB->rx.end)
	{
		return;
	}

	// 获取当前要处理的命令帧指针
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.head];
	
	// 命令头非法，退出
	if(UART_PROTOCOL_CMD_HEAD != pCmdFrame->buff[UART_PROTOCOL_CMD_HEAD_INDEX])
	{
		// 删除命令帧
		pCB->rx.head ++;
		pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// 命令头合法，则提取命令
	cmd = (UART_PROTOCOL_CMD)pCmdFrame->buff[UART_PROTOCOL_CMD_CMD_INDEX];

	// 执行命令帧
	switch(cmd)
	{
		// 空命令，不予执行
		case UART_PROTOCOL_CMD_NULL:
			break;
            

		// APP擦除
		case UART_PROTOCOL_CMD_DOWN_IAP_ERASE_FLASH:
			// 先擦除APP代码区
			IAP_EraseAPPArea();

			// 擦除完后需要读出来检查是否为空
//			errStatus = IAP_CheckAppFlashIsBlank();

			// 发送结果
			UART_PROTOCOL_SendCmdWithResult(UART_PROTOCOL_CMD_UP_IAP_ERASE_FLASH_RESULT, TRUE);

			break;
			
		// APP数据写入
		case UART_PROTOCOL_CMD_DOWN_IAP_WRITE_FLASH:
			{
				uint8 encryptionMode;
				uint8 key;
				static uint8 arrbuf[UART_PROTOCOL_CMD_LENGTH_INDEX];
				
				encryptionMode = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];
				key 		   = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA2_INDEX];

				switch (encryptionMode)
				{
					case 0x00:	// 无加密，【密钥】无意义
						temp  = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA3_INDEX];
						temp <<= 8;
						temp += pCmdFrame->buff[UART_PROTOCOL_CMD_DATA4_INDEX];
						temp <<= 8;
						temp += pCmdFrame->buff[UART_PROTOCOL_CMD_DATA5_INDEX];
						temp <<= 8;
						temp += pCmdFrame->buff[UART_PROTOCOL_CMD_DATA6_INDEX];

						length = pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] - 6;

						// 写入数据
						errStatus = IAP_WriteAppBin(temp, (uint8*)&pCmdFrame->buff[UART_PROTOCOL_CMD_DATA7_INDEX], length);
						break;

					case 0x01:	// 加密，地址与数据均与【密钥】相亦或后使用
						temp  = (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA3_INDEX] ^ key);
						temp <<= 8;
						temp += (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA4_INDEX] ^ key);
						temp <<= 8;
						temp += (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA5_INDEX] ^ key);
						temp <<= 8;
						temp += (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA6_INDEX] ^ key);

						length = pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] - 6;

						for (i = 0; i < length; i++)
						{
							arrbuf[i] = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA7_INDEX + i] ^ key;
						}

						// 写入数据
						errStatus = IAP_WriteAppBin(temp, arrbuf, length);
						break;

					case 0x02:
						errStatus = FALSE;
						break;

					default:
						errStatus = FALSE;
						break;
				}

				// 发送结果
				UART_PROTOCOL_SendCmdWithResult(UART_PROTOCOL_CMD_UP_IAP_WRITE_FLASH_RESULT, errStatus);
			}

			break;

//		// 查空
//		case UART_PROTOCOL_CMD_DOWN_CHECK_FALSH_BLANK:
//			errStatus = IAP_CheckApp2FlashIsBlank();

//			// 发送结果
//			UART_PROTOCOL_SendCmdWithResult(UART_PROTOCOL_CMD_UP_CHECK_FLASH_BLANK_RESULT, errStatus);
//			break;

//		// APP升级完成
//		case UART_PROTOCOL_CMD_DOWN_UPDATA_FINISH:
//			// 串口升级完成需要擦除蓝牙升级标志，否则不会进入应用APP
//			// 清除蓝牙升级标志
//			SPI_FLASH_WriteByte(SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS,0xFF);

//			// 串口升级完成提示
//			LCD_PrintString(50, 160, "Uart Update...", LCD_COLOR_BLACK, LCD_OPERATOR_MODE_DRAW);
//			LCD_PrintString(50, 160, "Please Shutdown and restart!", LCD_COLOR_WHITE, LCD_OPERATOR_MODE_DRAW);
//			
//			// 发送应答
//			UART_PROTOCOL_SendCmdNoResult(UART_PROTOCOL_CMD_UP_UPDATA_FINISH_RESULT);
//			break;

		default:
			break;
	}

	// 启动UART总线通讯超时判断
	UART_PROTOCOL_StartTimeoutCheckTask();
	
	// 删除命令帧
	pCB->rx.head ++;
	pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
}

// RX通讯超时处理-单向
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &uartProtocolCB.rx.cmdQueue[uartProtocolCB.rx.end];

	// 超时错误，将命令帧长度清零，即认为抛弃该命令帧
	pCmdFrame->length = 0;	// 2016.1.6增加
	// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
	uartProtocolCB.rxFIFO.head ++;
	uartProtocolCB.rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
	uartProtocolCB.rxFIFO.currentProcessIndex = uartProtocolCB.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void UART_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

// 码表设置立刻下发数据请求标志
void UART_PROTOCOL_SetTxAtOnceRequest(uint32 param)
{
//	uartProtocolCB.txAtOnceRequest = (BOOL)param;
}

// 码表设置周期下发数据请求标志
void UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param)
{
//	uartProtocolCB.txPeriodRequest = (BOOL)param;

}

// 码表发送数据通讯时序处理
void UART_PROTOCOL_DataCommunicationSequenceProcess(void)
{
	// 立即发送请求或是周期性发送请求
//	if ((uartProtocolCB.txAtOnceRequest) || (uartProtocolCB.txPeriodRequest))
//	{
//		// 参数设置成功，则发送运行参数命令
//		if (uartProtocolCB.paramSetOK)
//		{
//			UART_PROTOCOL_SendRuntimeParam();
//		}
//		// 参数设置未成功，则发送系统设置参数命令
//		else
//		{
//			// 发送系统参数设置给控制器
//			UART_PROTOCOL_SendConfigParam();
//		}

//		if (uartProtocolCB.txAtOnceRequest)
//		{
//			// 重新复位参数下发定时器
//			TIMER_ResetTimer(TIMER_ID_PROTOCOL_PARAM_TX);
//		}

//		uartProtocolCB.txAtOnceRequest = FALSE;
//		uartProtocolCB.txPeriodRequest = FALSE;
//	}
}

// 	UART总线超时错误处理
void UART_PROTOCOL_CALLBACK_UartBusError(uint32 param)
{
//	PARAM_SetErrorCode(ERROR_TYPE_COMMUNICATION_TIME_OUT);
//	PARAM_SetBatteryDataSrc(BATTERY_DATA_SRC_ADC);
	
//uartProtocolCB.speedFilter.realSpeed = 0;
//uartProtocolCB.speedFilter.proSpeed = 0;
//uartProtocolCB.speedFilter.difSpeed = 0;
}

// 上报命令字
void UART_PROTOCOL_ReportCmd(uint32 cmd)
{
    // 添加命令头
    UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

    // 添加设备地址
    UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

    // 添加命令字
    UART_PROTOCOL_TxAddData(cmd);

    // 添加数据长度
    UART_PROTOCOL_TxAddData(0);

    // 添加检验和与结束符，并添加至发送
    UART_PROTOCOL_TxAddFrame();
}


// 发送命令带结果
void UART_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
	// 添加命令头
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

	// 添加设备地址
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// 添加命令字
	UART_PROTOCOL_TxAddData(cmdWord);

	// 添加数据长度
	UART_PROTOCOL_TxAddData(1);
	
	UART_PROTOCOL_TxAddData(result);
	
	UART_PROTOCOL_TxAddFrame();
}

// 发送按键值
void UART_PROTOCOL_SendCmdCheckKeyValue(uint32 param)
{
	// 启动UART总线通讯超时判断
	UART_PROTOCOL_StartTimeoutCheckTask();
	
	UART_PROTOCOL_SendCmdWithResult(UART_PROTOCOL_CMD_KEY_CHECK_TEST, param);
}

// 上报升级成功
void  UP_Finish_Report(uint32 param)
{
    UART_PROTOCOL_SendCmdWithResult(UART_ECO_CMD_UP_FINISH, TRUE);
}

