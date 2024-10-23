#include "common.h"
#include "timer.h"
#include "Param.h"
#include "state.h"
#include "spiflash.h"
#include "CanProtocolUpTest.h"
#include "spiflash.h"
#include "iap.h"
#include "DutInfo.h"

/******************************************************************************
* 【内部接口声明】
******************************************************************************/

// 数据结构初始化
void CAN_TEST_PROTOCOL_DataStructInit(CAN_TEST_PROTOCOL_CB* pCB);

// UART报文接收处理函数(注意根据具体模块修改)
// void CAN_TEST_PROTOCOL_MacProcess(uint32 standarID, uint8* pData, uint16 length);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_TEST_PROTOCOL_RxFIFOProcess(CAN_TEST_PROTOCOL_CB* pCB);

// UART命令帧缓冲区处理
void CAN_TEST_PROTOCOL_CmdFrameProcess(CAN_TEST_PROTOCOL_CB* pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_TEST_PROTOCOL_CheckSUM(CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_TEST_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_TEST_PROTOCOL_CB* pCB);

// 通讯超时处理-单向
void CAN_TEST_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// 停止RX通讯超时检测任务
void CAN_TEST_PROTOCOL_StopRxTimeOutCheck(void);

// 协议层发送处理过程
void CAN_TEST_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void CAN_TEST_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length));

// 启动通讯超时判断任务
void CAN_TEST_PROTOCOL_StartTimeoutCheckTask(void);

// UART总线超时错误处理
void CAN_TEST_PROTOCOL_CALLBACK_UartBusError(uint32 param);

// 发送命令回复，带数据CRC
void CAN_TEST_PROTOCOL_SendCmdWithCrc(uint8 ackCmd, uint8 ackParam, uint32 Crc);

uint32 SPI_FlashReadForCrc32(uint32 startAddr, uint32 datalen);

// 全局变量定义
CAN_TEST_PROTOCOL_CB canTestProtocolCB;

// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// 启动通讯超时判断任务
void CAN_TEST_PROTOCOL_StartTimeoutCheckTask(void)
{

}

// 协议初始化
void CAN_TEST_PROTOCOL_Init(void)
{
	// 协议层数据结构初始化
	CAN_TEST_PROTOCOL_DataStructInit(&canTestProtocolCB);

	// 向驱动层注册数据接收接口
//	CAN_DRIVE_RegisterDataSendService(CAN_TEST_PROTOCOL_MacProcess);

	// 向驱动层注册数据发送接口
	CAN_TEST_PROTOCOL_RegisterDataSendService(CAN_DRIVE_AddTxArray);
}

// UART协议层过程处理
void CAN_TEST_PROTOCOL_Process(void)
{
	// UART接收FIFO缓冲区处理
	CAN_TEST_PROTOCOL_RxFIFOProcess(&canTestProtocolCB);

	// UART接收命令缓冲区处理
	CAN_TEST_PROTOCOL_CmdFrameProcess(&canTestProtocolCB);
	
	// UART协议层发送处理过程
	CAN_TEST_PROTOCOL_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void CAN_TEST_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = canTestProtocolCB.tx.head;
	uint16 end =  canTestProtocolCB.tx.end;
	CAN_TEST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &canTestProtocolCB.tx.cmdQueue[canTestProtocolCB.tx.end];

	// 发送缓冲区已满，不予接收
	if((end + 1) % CAN_TEST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 队尾命令帧已满，退出
	if(pCmdFrame->length >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 数据添加到帧末尾，并更新帧长度
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void CAN_TEST_PROTOCOL_TxAddFrame(void)
{
	uint16 checkSum = 0;
	uint16 i = 0;
	uint16 head = canTestProtocolCB.tx.head;
	uint16 end  = canTestProtocolCB.tx.end;
	CAN_TEST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &canTestProtocolCB.tx.cmdQueue[canTestProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;
    
    pCmdFrame->deviceID = 1;

	// 发送缓冲区已满，不予接收
	if((end + 1) % CAN_TEST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 命令帧长度不足，清除已填充的数据，退出
	if(CAN_TEST_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length)	// 减去"校验码"1个字节
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// 队尾命令帧已满，退出
	if((length >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	|| (length + 2 >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
	{
		return;
	}

	// 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
	pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_LENGTH_INDEX] = length - 3;	// 重设数据长度，减去"命令头、命令字、数据长度"4个字节
	
    for(i = 0; i < length; i++)
	{
		checkSum ^= pCmdFrame->buff[i];
	}
    
	// 校验码
	pCmdFrame->buff[pCmdFrame->length++] = checkSum;

	canTestProtocolCB.tx.end ++;
	canTestProtocolCB.tx.end %= CAN_TEST_PROTOCOL_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

// 数据结构初始化
void CAN_TEST_PROTOCOL_DataStructInit(CAN_TEST_PROTOCOL_CB* pCB)
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
	for(i = 0; i < CAN_TEST_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<CAN_TEST_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART报文接收处理函数(注意根据具体模块修改)
void CAN_TEST_PROTOCOL_MacProcess(uint32 standarID, uint8* pData, uint16 length)
{
	uint16 end = canTestProtocolCB.rxFIFO.end;
	uint16 head = canTestProtocolCB.rxFIFO.head;
	uint16 i = 0;
	uint8 *rxdata = pData;
	
	//■■环形列队，入队■■
	// 一级缓冲区已满，不予接收
	if ((end + 1) % CAN_TEST_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	
	for (i = 0; i < length; i++)
	{
		// 单个字节读取，并放入FIFO中
		canTestProtocolCB.rxFIFO.buff[canTestProtocolCB.rxFIFO.end] = *rxdata++;
		
		canTestProtocolCB.rxFIFO.end++;
		
		// 一级缓冲区已满，不予接收
		if ((canTestProtocolCB.rxFIFO.end + 1) % CAN_TEST_PROTOCOL_RX_FIFO_SIZE == head)
		{
			break;
		}
		
		canTestProtocolCB.rxFIFO.end %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
	}
}

// UART协议层向驱动层注册数据发送接口
void CAN_TEST_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length))
{		
	canTestProtocolCB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_TEST_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_TEST_PROTOCOL_CB* pCB)
{
	CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
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
	pCB->rx.end %= CAN_TEST_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区
	
	return TRUE;
}

// 协议层发送处理过程
void CAN_TEST_PROTOCOL_TxStateProcess(void)
{
	uint16 head = canTestProtocolCB.tx.head;
	uint16 end =  canTestProtocolCB.tx.end;
	uint16 length = canTestProtocolCB.tx.cmdQueue[head].length;
	uint8* pCmd = canTestProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = canTestProtocolCB.tx.cmdQueue[head].deviceID;

	// 发送缓冲区为空，说明无数据
	if (head == end)
	{
		return;
	}

	// 发送函数没有注册直接返回
	if (NULL == canTestProtocolCB.sendDataThrowService)
	{
		return;
	}

	// 协议层有数据需要发送到驱动层		
	if (!(*canTestProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// 发送环形队列更新位置
	canTestProtocolCB.tx.cmdQueue[head].length = 0;
	canTestProtocolCB.tx.head ++;
	canTestProtocolCB.tx.head %= CAN_TEST_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_TEST_PROTOCOL_RxFIFOProcess(CAN_TEST_PROTOCOL_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
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
		if(CAN_TEST_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// 命令头正确，但无临时缓冲区可用，退出
		if((pCB->rx.end + 1)%CAN_TEST_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// 添加UART通讯超时时间设置-2016.1.5增加
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_CAN_COMMUNICATION_TIME_OUT,
						CAN_TEST_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
						CAN_TEST_PROTOCOL_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
	}
	// 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
	else
	{
		// 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
		if(pCmdFrame->length >= CAN_TEST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			CAN_TEST_PROTOCOL_StopRxTimeOutCheck();
#endif

			// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
			pCmdFrame->length = 0;	// 2016.1.5增加
			// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
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
			pCB->rxFIFO.currentProcessIndex %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;

			// ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■
			
			// 首先判断命令帧最小长度，一个完整的命令字至少包括8个字节: 命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，因此不足8个字节的必定不完整
			if(pCmdFrame->length < CAN_TEST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 继续接收
				continue;
			}

			// 命令帧长度数值越界，说明当前命令帧错误，停止接收
			if(pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_LENGTH_INDEX] > CAN_TEST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX-CAN_TEST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				CAN_TEST_PROTOCOL_StopRxTimeOutCheck();
#endif
			
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// 命令帧长度校验，在命令长度描述字的数值上，增加命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，即为命令帧实际长度
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_LENGTH_INDEX] + CAN_TEST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 长度要求不一致，说明未接收完毕，退出继续
				continue;
			}

			// 命令帧长度OK，则进行校验，失败时删除命令头
			if(!CAN_TEST_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				CAN_TEST_PROTOCOL_StopRxTimeOutCheck();
#endif
				
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			CAN_TEST_PROTOCOL_StopRxTimeOutCheck();
#endif
			
			// 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			CAN_TEST_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_TEST_PROTOCOL_CheckSUM(CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame)
{
	uint16 checkSum = 0;
	uint16 sumTemp;
	uint16 i = 0;
    
    uint8 length = pCmdFrame->length;
	
	if(NULL == pCmdFrame)
	{
		return FALSE;
	}

	// 从起始符开始，到校验码之前的一个字节，依次进行异或运算
	for(i = 0; i < pCmdFrame->length - 1; i++)
	{
		checkSum ^= pCmdFrame->buff[i];
	}
	
	// 判断计算得到的校验码与命令帧中的校验码是否相同
	if(pCmdFrame->buff[pCmdFrame->length-1] != checkSum)
	{
		return FALSE;
	}
	
	return TRUE;
}

// UART命令帧缓冲区处理
void CAN_TEST_PROTOCOL_CmdFrameProcess(CAN_TEST_PROTOCOL_CB* pCB)
{
	CAN_TEST_PROTOCOL_CMD cmd = CAN_TEST_PROTOCOL_CMD_NULL;
	CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
    
    uint8 eraResual;
    uint32 temp = 0;
    uint16 length = 0;	
    
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
	if(CAN_TEST_PROTOCOL_CMD_HEAD != pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_HEAD_INDEX])
	{
		// 删除命令帧
		pCB->rx.head ++;
		pCB->rx.head %= CAN_TEST_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// 命令头合法，则提取命令
	cmd = (CAN_TEST_PROTOCOL_CMD)pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_INDEX];

	// 执行命令帧
	switch(cmd)
	{
		// 空命令，不予执行
		case CAN_TEST_PROTOCOL_CMD_NULL:
			break;

        case CAN_TEST_PROTOCOL_CMD_BOOT_ERASE_FLASH: 			        // DUT_BOOT擦除
            eraResual = pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_DATA1_INDEX];
            
            // 擦除失败
            if (0 == eraResual)
            {
                // 擦除失败再擦除一次
                STATE_SwitchStep(STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_EAR);
                break;
            }

            // 发送BOOT数据
            dut_info.currentBootSize = 0;
            TIMER_KillTask(TIMER_ID_UPGRADE_DUT_BOOT);
            STATE_SwitchStep(STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_PACKET);                
            break;
            
        case CAN_TEST_PROTOCOL_CMD_BOOT_WRITE_FLASH: 				    // DUT_BOOT写入
            eraResual = pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_DATA1_INDEX];

            if (0 == eraResual) // 如果仪表写入失败就退出
            {
                dut_info.currentBootSize--;
            }
            if (dut_info.currentBootSize < dut_info.bootSize)
            {
                STATE_SwitchStep(STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_PACKET);
            }
            else
            {
                STATE_SwitchStep(STEP_CM_CAN_BC_UPGRADE_BOOT_UP_SUCCESS);
            }                
            break;
            
        case CAN_TEST_PROTOCOL_CMD_BOOT_UPDATA_FINISH: 		            	// DUT_BOOT写入完成
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ITEM_FINISH);
            break;    
                    
        default:
            break;
	}

	// 启动UART总线通讯超时判断
	CAN_TEST_PROTOCOL_StartTimeoutCheckTask();
	
	// 删除命令帧
	pCB->rx.head ++;
	pCB->rx.head %= CAN_TEST_PROTOCOL_RX_QUEUE_SIZE;
}

// RX通讯超时处理-单向
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void CAN_TEST_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &canTestProtocolCB.rx.cmdQueue[canTestProtocolCB.rx.end];

	// 超时错误，将命令帧长度清零，即认为抛弃该命令帧
	pCmdFrame->length = 0;	// 2016.1.6增加
	// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
	canTestProtocolCB.rxFIFO.head ++;
	canTestProtocolCB.rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
	canTestProtocolCB.rxFIFO.currentProcessIndex = canTestProtocolCB.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void CAN_TEST_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_CAN_COMMUNICATION_TIME_OUT);
}
#endif


// 	UART总线超时错误处理
void CAN_TEST_PROTOCOL_CALLBACK_UartBusError(uint32 param)
{

}

// 发送命令带结果
void CAN_TEST_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
	// 添加命令头
	CAN_TEST_PROTOCOL_TxAddData(CAN_TEST_PROTOCOL_CMD_HEAD);

	// 添加命令字
	CAN_TEST_PROTOCOL_TxAddData(cmdWord);

	// 添加数据长度
	CAN_TEST_PROTOCOL_TxAddData(1);
	
	CAN_TEST_PROTOCOL_TxAddData(result);
	
	CAN_TEST_PROTOCOL_TxAddFrame();
}

// 发送命令无结果
void CAN_TEST_PROTOCOL_SendCmdNoResult(uint8 cmdWord)
{
	// 添加命令头
	CAN_TEST_PROTOCOL_TxAddData(CAN_TEST_PROTOCOL_CMD_HEAD);

	// 添加命令字
	CAN_TEST_PROTOCOL_TxAddData(cmdWord);
	
	CAN_TEST_PROTOCOL_TxAddData(0);		// 数据长度
	
	CAN_TEST_PROTOCOL_TxAddFrame();
}

// 发送命令回复，带数据CRC
void CAN_TEST_PROTOCOL_SendCmdWithCrc(uint8 ackCmd, uint8 ackParam, uint32 Crc)
{
    CAN_TEST_PROTOCOL_TxAddData(CAN_TEST_PROTOCOL_CMD_HEAD);
    CAN_TEST_PROTOCOL_TxAddData(ackCmd);
    CAN_TEST_PROTOCOL_TxAddData(5);
    CAN_TEST_PROTOCOL_TxAddData(ackParam);
    CAN_TEST_PROTOCOL_TxAddData((Crc >> 24) & 0xFF);
    CAN_TEST_PROTOCOL_TxAddData((Crc >> 16) & 0xFF);
    CAN_TEST_PROTOCOL_TxAddData((Crc >> 8) & 0xFF);
    CAN_TEST_PROTOCOL_TxAddData(Crc & 0xFF);  
    CAN_TEST_PROTOCOL_TxAddFrame();
}

// 发送升级数据
void CAN_TEST_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_TEST_PROTOCOL_CMD cmd, uint32 flashAddr, uint32 addr)
{
	uint8 i = 0;
    
	switch (upDataType)
	{
        case DUT_FILE_TYPE_HEX:							 // hex
            dut_info.reconnectionRepeatContent[0] = 121; // 总长度
            dut_info.reconnectionRepeatContent[1] = CAN_TEST_PROTOCOL_CMD_HEAD;
            dut_info.reconnectionRepeatContent[2] = cmd;
            dut_info.reconnectionRepeatContent[3] = 0;											  // 数据长度
            dut_info.reconnectionRepeatContent[4] = 0;											  // 添加是否加密
            dut_info.reconnectionRepeatContent[5] = 0;											  // 添加密钥
            SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[6], flashAddr + (addr), 116); // 工具读取 4+112 个字节
            for (i = 0; i < 121; i++)															  // 添加数据
            {
                CAN_TEST_PROTOCOL_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
            }
            break;

        case DUT_FILE_TYPE_BIN:							 // bin
            dut_info.reconnectionRepeatContent[0] = 135; // 总长度
            dut_info.reconnectionRepeatContent[1] = CAN_TEST_PROTOCOL_CMD_HEAD;
            dut_info.reconnectionRepeatContent[2] = cmd;
            dut_info.reconnectionRepeatContent[3] = 0;											  // 数据长度
            dut_info.reconnectionRepeatContent[4] = ((addr) & 0xFF000000) >> 24;				  // 数据长度
            dut_info.reconnectionRepeatContent[5] = ((addr) & 0x00FF0000) >> 16;				  // 数据长度
            dut_info.reconnectionRepeatContent[6] = ((addr) & 0x0000FF00) >> 8;					  // 数据长度
            dut_info.reconnectionRepeatContent[7] = ((addr) & 0x000000FF);						  // 数据长度
            SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[8], flashAddr + (addr), 128); // 工具读取128字节
            for (i = 0; i < 135; i++)															  // 添加数据
            {
                CAN_TEST_PROTOCOL_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
            }
            break;
            
        default:
            break;
	}
	CAN_TEST_PROTOCOL_TxAddFrame(); // 调整帧格式,修改长度和添加校验
}
