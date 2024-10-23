#include "common.h"
#include "timer.h"
#include "CanProtocolUpDT.h"
#include "canProtocolUpGB.h"
#include "spiflash.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "state.h"

/******************************************************************************
 * 【内部接口声明】
 ******************************************************************************/

CAN_PROTOCOL_PARAM_RX_CB canRxDriveCB;

// 数据结构初始化
void CAN_PROTOCOL_DataStructInit(CAN_PROTOCOL_CB *pCB);

// CAN 报文接收处理函数(注意根据具体模块修改)
void CAN_PROTOCOL_MacProcess(uint32 standarID, uint8 *pData, uint8 length);

// 一级报文接收缓冲区处理函数，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_PROTOCOL_RxFIFOProcess(CAN_PROTOCOL_CB *pCB);

// CAN命令帧缓冲区处理
void CAN_PROTOCOL_CmdFrameProcess(CAN_PROTOCOL_RX *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_PROTOCOL_CheckSum(uint8 *buff, uint32 len);

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_PROTOCOL_AddSum(uint8 *buff, uint32 len);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_PROTOCOL_CB *pCB, uint8 sourceSelect);

// 协议层发送处理过程
void CAN_PROTOCOL_TxStateProcess(void);

// CAN协议层向驱动层注册数据发送接口
void CAN_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length));

//===============================================================================================================

// 码表设置周期下发数据请求标志
void CAN_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param);

// 码表发送数据通讯时序处理
void CAN_PROTOCOL_DataCommunicationSequenceProcess(void);

// 	CAN	总线BMS超时错误处理
void CAN_PROTOCOL_CALLBACK_BmsCanBusError(uint32 param);

// 	CAN	总线ECU超时错误处理
void CAN_PROTOCOL_CALLBACK_EcuCanBusError(uint32 param);

// 启动通讯超时判断任务
void CAN_PROTOCOL_StartTimeoutCheckTask(void);

// CAN总线超时错误处理
void CAN_PROTOCOL_CALLBACK_CanBusError(uint32 param);

//==================================================================================
// 平滑处理机制
void CAN_PROTOCOL_CALLBACK_SmoothMechanismProcess(uint32 param);

// 码表发送控制电机指令D0 模式 D1 档位 D2 大灯 按键 D3 单里程清零	100ms
void CMD_CAN_HmiToDriverSendData(uint32 param);

// 转向灯
void CMD_CAN_HmiTurnToDriverSendData(uint32 param);

// 设置参数状态 0x182A1828
void CMD_CAN_HmiFactoryAck(uint32 param);

// 发送骑行数据
void CAN_SendRidingParam(uint32 param);

// 档位名称
void CAN_SendAssistNameParam(uint32 param);

// 故障信息
void CAN_SendErrorInfoParam(uint32 param);

// 版本信息
void CAN_SendHmiVersionParam(uint32 param);

// 仪表序列号
void CAN_SendHmiSnParam(uint32 param);

//=======================================================================================
// 全局变量定义
CAN_PROTOCOL_CB canProtocolCB;
CAN_PROTOCOL_RX canProtocolRX;

// CAN协议层初始化
void CAN_PROTOCOL_Init(void)
{
	// 协议层数据结构初始化
	CAN_PROTOCOL_DataStructInit(&canProtocolCB);

	// 向驱动层注册数据接收接口
	//	CAN_DRIVE_RegisterDataSendService(CAN_PROTOCOL_MacProcess);

	// 向驱动层注册数据发送接口
	CAN_PROTOCOL_RegisterDataSendService(CAN_DRIVE_AddTxArray);

	// 发送完毕，清空buff
	memset(canProtocolCB.sendBuff, 0xFF, 200);
	//	TIMER_AddTask(TIMER_ID_CAN_TEST,
	//					100,
	//					CAN_UP_RESET,
	//					1,
	//					TIMER_LOOP_FOREVER,
	//					ACTION_MODE_ADD_TO_QUEUE);
	//	CAN_UP_RESET(1);
}

// 向发送命令帧队列中添加帧源，帧目的
void CAN_PROTOCOL_TxAddNewFrame(uint8 priority, uint16 pgn, uint8 destinationID, uint8 sourceID)
{
	CAN_PROTOCOL_CMD_FRAME *pCmdFrame = &canProtocolCB.tx.cmdQueue[canProtocolCB.tx.end];

	// 帧源和帧目的超出范围
	if ((sourceID > 0xFF) || (destinationID > 0xFF))
	{
		return;
	}

	// 数据添加到帧末尾，并更新帧长度
	// pCmdFrame->deviceID = ((sourceID << 4) | destinationID);

	pCmdFrame->EXTID.sDet.nHeard = 0;				   // 3bit:000
	pCmdFrame->EXTID.sDet.priority = priority;		   // 3bit:优先级
	pCmdFrame->EXTID.sDet.pgn = pgn;				   // 10bit:pgn
	pCmdFrame->EXTID.sDet.destination = destinationID; // 8bit:目的地址
	pCmdFrame->EXTID.sDet.source = sourceID;		   // 8bit:源地址
}

// 向发送命令帧队列中添加数据
void CAN_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = canProtocolCB.tx.head;
	uint16 end = canProtocolCB.tx.end;
	CAN_PROTOCOL_CMD_FRAME *pCmdFrame = &canProtocolCB.tx.cmdQueue[canProtocolCB.tx.end];

	// 发送缓冲区已满，不予接收
	if ((end + 1) % CAN_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// 队尾命令帧已满，退出
	if (pCmdFrame->length >= CAN_PROTOCOL_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 数据添加到帧末尾，并更新帧长度
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length++;
}

// 发送命令帧处理,自动扶正帧长度以及检验码
void CAN_PROTOCOL_TxAddFrame(void)
{
	uint16 head = canProtocolCB.tx.head;
	uint16 end = canProtocolCB.tx.end;

	// 发送缓冲区已满，不予接收
	if ((end + 1) % CAN_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// 发送环形队列更新位置
	canProtocolCB.tx.end++;
	canProtocolCB.tx.end %= CAN_PROTOCOL_TX_QUEUE_SIZE;
	canProtocolCB.tx.cmdQueue[canProtocolCB.tx.end].length = 0; // 2015.12.2修改
}

// CAN协议层过程处理
void CAN_PROTOCOL_Process_GB(void)
{
	// CAN接收FIFO缓冲区处理
	// CAN_PROTOCOL_RxFIFOProcess(&canProtocolCB);

	// CAN接收命令缓冲区处理
	CAN_PROTOCOL_CmdFrameProcess(&canProtocolRX);

	// CAN协议层发送处理过程
	CAN_PROTOCOL_TxStateProcess();
}

// 数据结构初始化
void CAN_PROTOCOL_DataStructInit(CAN_PROTOCOL_CB *pCB)
{
	uint16 i;
	uint16 j;

	// 参数合法性检验
	if (NULL == pCB)
	{
		return;
	}

	pCB->tx.txBusy = FALSE;
	pCB->tx.index = 0;
	pCB->tx.head = 0;
	pCB->tx.end = 0;
	for (i = 0; i < CAN_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	for (i = 0; i < CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX; i++)
	{
		pCB->rxFIFO.rxFIFOEachNode[i].head = 0;
		pCB->rxFIFO.rxFIFOEachNode[i].end = 0;
		pCB->rxFIFO.rxFIFOEachNode[i].currentProcessIndex = 0;
	}

	for (i = 0; i < CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX; i++)
	{
		for (j = 0; j < CAN_PROTOCOL_RX_QUEUE_SIZE; j++)
		{
			//		pCB->rx.rxEachNode[i].cmdQueue[j].length = 0;
		}
		pCB->rx.rxEachNode[i].head = 0;
		pCB->rx.rxEachNode[i].end = 0;
	}

	memset(canProtocolCB.sendBuff, 0xFF, 128);
	canProtocolCB.tripData.distance = 0;
	canProtocolCB.tripData.times = 0;
	canProtocolCB.hmiState = 0;
	canProtocolCB.UPDATA1 = TRUE;
	canProtocolCB.UPDATA = TRUE;
}

// CAN 报文接收处理函数(注意根据具体模块修改)
void CAN_PROTOCOL_MacProcess(uint32 standarID, uint8 *pData, uint8 length)
{
	uint8 i = 0;

	canProtocolRX.cmdQueue[canProtocolRX.end].uExtId.ExtId = standarID;
	for (i = 0; i < length; i++)
	{
		canProtocolRX.cmdQueue[canProtocolRX.end].Data[i] = *pData++;
	}
	canProtocolRX.cmdQueue[canProtocolRX.end].DLC = length;
	canProtocolRX.end++;
	canProtocolRX.end %= CAN_PROTOCOL_RX_QUEUE_SIZE;
}

// 一级报文接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_PROTOCOL_RxFIFOProcess(CAN_PROTOCOL_CB *pCB)
{
}

// CAN命令帧缓冲区处理
void CAN_PROTOCOL_CmdFrameProcess(CAN_PROTOCOL_RX *pCB)
{
	if (pCB->head == pCB->end)
	{
		return;
	}

	switch (pCB->cmdQueue[pCB->head].uExtId.ExtId)
	{
	case 0x1801FF28: // 仪表的状态
	
		// 升级流程是从app跳入boot开始升级
		if (!canProtocolCB.UPDATA1) // 升级标志位
		{
			// 01表示当前在app
			if (0x01 == (pCB->cmdQueue[pCB->head].Data[0] & 0x01))
			{
				// 如果需要升级就跳入boot
				if (!canProtocolCB.UPDATA)
				{
					canProtocolCB.UPDATA = TRUE;
					// 切换到跳入boot步骤
					STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_SEND_BOOT_CMD);
				}
			}
			else // 在boot中，发送第一包升级的数据包，升级开始
			{
				canProtocolCB.UPDATA1 = TRUE;
				STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY);
			}
		}
		break;

	case 0x18ECF927: // 发送多包数据
					 // 0xEF：一包132Byte数据，4个地址加128个数据。0x13：当前数据包仪表接收完成，
		if ((0xEF == pCB->cmdQueue[pCB->head].Data[6]) && (0x13 == pCB->cmdQueue[pCB->head].Data[0]))
		{
			// 0x01：校验通过，发送下一个数据包
			if (0x01 == pCB->cmdQueue[pCB->head].Data[4])
			{
				//				CAN_UP_DATA(1);
				STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY);
			}
			else
			{
				// 校验不通过
				STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WRITE_ERROR);
			}
		}
		// 升级PGN，容许发送升级数据，0x11：仪表允许工具发送数据
		else if ((0xEF == pCB->cmdQueue[pCB->head].Data[6]) && (0x11 == pCB->cmdQueue[pCB->head].Data[0]))
		{
			// STATE_PGN_SEND_2：工具发送数据
			STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_SEND_DATA);
			// CAN_PGN_RequestSendData(STATE_PGN_SEND_2, pCB->cmdQueue[pCB->head].Data[6], 0x27, 0xF9);
		}

		break;

	default:
		break;
	}

	pCB->head++;
	pCB->head %= CAN_PROTOCOL_RX_QUEUE_SIZE;
}

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_PROTOCOL_CheckSum(uint8 *buff, uint32 len)
{
	uint8 cc = 0;
	uint16 i = 0;

	if (NULL == buff)
	{
		return FALSE;
	}

	// 从命令段长度开始(节点ID已经在解析前去掉了，所以i为3)，到校验码之前的一个字节，依次进行异或运算
	for (i = CAN_PROTOCOL_RX_CMD_LENGTH_INDEX; i < (len - CAN_PROTOCOL_CHECK_BYTE - CAN_PROTOCOL_TAIL_BYTE); i++)
	{
		cc ^= buff[i];
	}

	// 判断计算得到的校验码与命令帧中的校验码是否相同
	if (buff[len - 2] != cc)
	{
		return FALSE;
	}

	return TRUE;
}

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_PROTOCOL_AddSum(uint8 *buff, uint32 len)
{
	uint8 cc = 0;
	uint16 i = 0;

	if (NULL == buff)
	{
		return FALSE;
	}

	// 从命令段长度开始(节点ID已经单独发送了，所以i为3)，到校验码之前的一个字节，依次进行异或运算
	for (i = CAN_PROTOCOL_TX_CMD_LENGTH_INDEX; i < len; i++)
	{
		cc ^= buff[i];
	}

	// 添加校验码
	buff[len] = cc;

	return TRUE;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_PROTOCOL_CB *pCB, uint8 sourceSelect)
{
	CAN_PROTOCOL_CMD_FRAME *pCmdFrame = NULL;

	// 参数合法性检验
	if (NULL == pCB)
	{
		return FALSE;
	}

	// 临时缓冲区为空，不予添加
	//	pCmdFrame = &pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end];

	if (0 == pCmdFrame->length)
	{
		return FALSE;
	}

	// 添加
	pCB->rx.rxEachNode[sourceSelect].end++;
	pCB->rx.rxEachNode[sourceSelect].end %= CAN_PROTOCOL_RX_QUEUE_SIZE;
	// 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区
	//	pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end].length = 0;

	return TRUE;
}

// 协议层发送处理过程
void CAN_PROTOCOL_TxStateProcess(void)
{
	uint16 head = canProtocolCB.tx.head;
	uint16 end = canProtocolCB.tx.end;
	uint16 length = canProtocolCB.tx.cmdQueue[head].length;
	uint8 *pCmd = canProtocolCB.tx.cmdQueue[head].buff;
	uint32 localDeviceID = canProtocolCB.tx.cmdQueue[head].EXTID.deviceID;

	// 发送缓冲区为空，说明无数据
	if (head == end)
	{
		return;
	}

	// 发送函数没有注册直接返回
	if (NULL == canProtocolCB.sendDataThrowService)
	{
		return;
	}

	// 协议层有数据需要发送到驱动层
	if (!(*canProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// 发送环形队列更新位置
	canProtocolCB.tx.cmdQueue[head].length = 0;
	canProtocolCB.tx.head++;
	canProtocolCB.tx.head %= CAN_PROTOCOL_TX_QUEUE_SIZE;
}

// CAN协议层向驱动层注册数据发送接口
void CAN_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length))
{
	canProtocolCB.sendDataThrowService = service;
}

// state : 当前状态   DestAddress:目的地址   SourceAddress:源地址
void CAN_PGN_RequestSendData(STATE_PGN state, uint8 PGN, uint8 DestAddress, uint8 SourceAddress)
{
	uint16 dataLength = 0, dataNumber = 0;
	uint8 number = 1;

	canProtocolCB.preState = canProtocolCB.state;
	canProtocolCB.state = state;

	switch (PGN)
	{
	// 升级包数据大小
	case 0xEF:

		// 发送的字节数
		dataLength = 132;

		// 发送的总包数
		dataNumber = 19;

		break;

	// 下发总成编码
	case 0xED:

		// 发送的字节数
		dataLength = 43;

		// 发送的总包数
		dataNumber = 7;
		break;

	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
		// 发送的字节数
		dataLength = 23;

		// 发送的总包数
		dataNumber = 4;
		break;
	default:
		break;
	}

	switch (state)
	{
	case STATE_PGN_NULL:

		break;

	// 发送PGN数据第一步:请求发送
	case STATE_PGN_SEND_1:

		CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEC, DestAddress, SourceAddress);

		CAN_PROTOCOL_TxAddData(0x10);

		CAN_PROTOCOL_TxAddData(dataLength & 0xFF);

		CAN_PROTOCOL_TxAddData((dataLength >> 8) & 0xFF);

		CAN_PROTOCOL_TxAddData(dataNumber);

		CAN_PROTOCOL_TxAddData(0xFF);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(PGN);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
		break;

	// 发送PGN数据第二步:目的地址回应允许后下发对应的数据
	case STATE_PGN_SEND_2:

		for (number = 1; number <= dataNumber; number++)
		{
			CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEB, DestAddress, SourceAddress);

			CAN_PROTOCOL_TxAddData(number);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 0]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 1]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 2]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 3]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 4]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 5]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 6]);

			CAN_PROTOCOL_TxAddFrame();
		}

		break;
	default:
		break;
	}
}

// state : 当前状态   DestAddress:目的地址   SourceAddress:源地址
void CAN_PGN_RequestGetData(STATE_PGN state, uint8 PGN, uint8 DestAddress, uint8 SourceAddress)
{
	uint16 dataLength = 0, dataNumber = 0;

	canProtocolCB.preState = canProtocolCB.state;
	canProtocolCB.state = state;

	switch (PGN)
	{
	// 升级数据包大小下发
	case 0x50:
		// 发送的字节数
		dataLength = 28;

		// 发送的总包数
		if (dataLength % 7)
		{
			dataNumber = (dataLength / 7) + 1;
		}
		else
		{
			dataNumber = dataLength / 7;
		}

		break;

	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
		// 发送的字节数
		dataLength = 23;

		// 发送的总包数
		if (dataLength % 7)
		{
			dataNumber = (dataLength / 7) + 1;
		}
		else
		{
			dataNumber = dataLength / 7;
		}
		break;

	default:
		break;
	}

	switch (state)
	{
	case STATE_PGN_NULL:

		break;

	// 请求PGN数据 第一步:发送请求PGN  EA00
	case STATE_PGN_GET_1:

		CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEA, DestAddress, SourceAddress);

		CAN_PROTOCOL_TxAddData(0x00);

		CAN_PROTOCOL_TxAddData(PGN);

		CAN_PROTOCOL_TxAddData(0x00);

		CAN_PROTOCOL_TxAddFrame();
		break;

	// 请求PGN数据 第二步:发送请求PGN  EC00
	case STATE_PGN_GET_2:

		CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEC, DestAddress, SourceAddress);

		CAN_PROTOCOL_TxAddData(0x11); // 准许发送

		CAN_PROTOCOL_TxAddData(dataNumber);

		CAN_PROTOCOL_TxAddData(0x01);

		CAN_PROTOCOL_TxAddData(0xFF);

		CAN_PROTOCOL_TxAddData(0xFF);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(PGN);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
		break;

	// 请求PGN数据 第三步:发送请求PGN  EC00
	case STATE_PGN_GET_3:

		CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEC, DestAddress, SourceAddress);

		CAN_PROTOCOL_TxAddData(0x13); // 结束应答

		CAN_PROTOCOL_TxAddData(dataLength & 0xFF); // 	字节数

		CAN_PROTOCOL_TxAddData((dataLength >> 8) & 0xFF);

		CAN_PROTOCOL_TxAddData(dataNumber); // 	包数

		CAN_PROTOCOL_TxAddData(0x01); // 数据校验成功

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(PGN);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
		break;

	default:
		break;
	}
}

void CAN_UP_DATA(uint32 param)
{
	dut_info.currentUiSize++;
	// 如果升级完成
	if (dut_info.currentUiSize > dut_info.uiSize)
	{
		STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_FINISH);
		return;
	}
	// 向发送buff添加地址编号
	canProtocolCB.sendBuff[0] = ((dut_info.currentUiSize >> 24) & 0xFF);
	canProtocolCB.sendBuff[1] = ((dut_info.currentUiSize >> 16) & 0xFF);
	canProtocolCB.sendBuff[2] = ((dut_info.currentUiSize >> 8) & 0xFF);
	canProtocolCB.sendBuff[3] = (dut_info.currentUiSize & 0xFF);
	// 向发送buff添加数据
	SPI_FLASH_ReadArray(&canProtocolCB.sendBuff[4], (SPI_FLASH_DUT_UI_ADDEESS + (dut_info.currentUiSize - 1) * 128), 128);
	// STATE_PGN_SEND_1：工具请求仪表接收数据
	CAN_PGN_RequestSendData(STATE_PGN_SEND_1, 0xEF, 0x27, 0xF9);
}

void CAN_UP_RESET(uint32 param)
{
	if (1 == param)
	{
		CAN_PROTOCOL_TxAddNewFrame(0x06, 0x01, 0xFF, 0xF9);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
	}
	else if (2 == param)
	{
		CAN_PROTOCOL_TxAddNewFrame(0x06, 0x01, 0xFF, 0xF9);

		CAN_PROTOCOL_TxAddData(0x40);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0x02);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
	}
}
