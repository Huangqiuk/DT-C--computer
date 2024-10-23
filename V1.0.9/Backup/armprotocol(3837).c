#include "common.h"
#include "ArmUart.h"
#include "ArmProtocol.h"
#include "timer.h"
// #include "param.h"
#include "State.h"
#include "Spiflash.h"
#include "system.h"
#include "iap.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "CanProtocolUpGB.h"
#include "state.h"

/******************************************************************************
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void ARM_PROTOCOL_DataStructInit(ARM_PROTOCOL_CB *pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void ARM_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void ARM_PROTOCOL_RxFIFOProcess(ARM_PROTOCOL_CB *pCB);

// UART命令帧缓冲区处理
void ARM_PROTOCOL_CmdFrameProcess(ARM_PROTOCOL_CB *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL ARM_PROTOCOL_CheckSUM(ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL ARM_PROTOCOL_ConfirmTempCmdFrameBuff(ARM_PROTOCOL_CB *pCB);

// 协议层发送处理过程
void ARM_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void ARM_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// 发送命令回复
void ARM_PROTOCOL_SendCmdAck(uint8 ackCmd);

// 发送带应答的回复
void ARM_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// 仪表状态信息回复
void ARM_PROTOCOL_SendCmdStatusAck(void);

//=======================================================================================
// 全局变量定义
ARM_PROTOCOL_CB armProtocolCB;

// 协议初始化
void ARM_PROTOCOL_Init(void)
{
	// 协议层数据结构初始化
	ARM_PROTOCOL_DataStructInit(&armProtocolCB);

	// 向驱动层注册数据接收接口
	ARM_UART_RegisterDataSendService(ARM_PROTOCOL_MacProcess);

	// 向驱动层注册数据发送接口
	ARM_PROTOCOL_RegisterDataSendService(ARM_UART_AddTxArray);
}

// ARM协议层过程处理
void ARM_PROTOCOL_Process(void)
{
	// ARM接收FIFO缓冲区处理
	ARM_PROTOCOL_RxFIFOProcess(&armProtocolCB);

	// ARM接收命令缓冲区处理
	ARM_PROTOCOL_CmdFrameProcess(&armProtocolCB);

	// ARM协议层发送处理过程
	ARM_PROTOCOL_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void ARM_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = armProtocolCB.tx.head;
	uint16 end = armProtocolCB.tx.end;
	ARM_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &armProtocolCB.tx.cmdQueue[armProtocolCB.tx.end];

	// 发送缓冲区已满，不予接收
	if ((end + 1) % ARM_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// 队尾命令帧已满，退出
	if (pCmdFrame->length >= ARM_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 数据添加到帧末尾，并更新帧长度
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void ARM_PROTOCOL_TxAddFrame(void)
{
	uint16 cc = 0;
	uint16 i = 0;
	uint16 head = armProtocolCB.tx.head;
	uint16 end = armProtocolCB.tx.end;
	ARM_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &armProtocolCB.tx.cmdQueue[armProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// 发送缓冲区已满，不予接收
	if ((end + 1) % ARM_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// 命令帧长度不足，清除已填充的数据，退出
	if (ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length) // 减去"校验和"1个字节
	{
		pCmdFrame->length = 0;

		return;
	}

	// 队尾命令帧已满，退出
	if (length >= ARM_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
	pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] = length - 3; // 重设数据长度

	for (i = 0; i < length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = ~cc;

	armProtocolCB.tx.end++;
	armProtocolCB.tx.end %= ARM_PROTOCOL_TX_QUEUE_SIZE;
	// pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

// 数据结构初始化
void ARM_PROTOCOL_DataStructInit(ARM_PROTOCOL_CB *pCB)
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
	for (i = 0; i < ARM_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end = 0;
	for (i = 0; i < ARM_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}

	pCB->isTimeCheck = FALSE;
}

// UART报文接收处理函数(注意根据具体模块修改)
void ARM_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
	uint16 end = armProtocolCB.rxFIFO.end;
	uint16 head = armProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;

	// 接收数据
	rxdata = *pData;

	// 一级缓冲区已满，不予接收
	if ((end + 1) % ARM_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// 一级缓冲区未满，接收
	else
	{
		// 将接收到的数据放到临时缓冲区中
		armProtocolCB.rxFIFO.buff[end] = rxdata;
		armProtocolCB.rxFIFO.end++;
		armProtocolCB.rxFIFO.end %= ARM_PROTOCOL_RX_FIFO_SIZE;
	}
}

// UART协议层向驱动层注册数据发送接口
void ARM_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
	armProtocolCB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL ARM_PROTOCOL_ConfirmTempCmdFrameBuff(ARM_PROTOCOL_CB *pCB)
{
	ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	// 参数合法性检验
	if (NULL == pCB)
	{
		return FALSE;
	}

	// 临时缓冲区为空，不予添加
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	if (0 == pCmdFrame->length)
	{
		return FALSE;
	}

	// 添加
	pCB->rx.end++;
	pCB->rx.end %= ARM_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0; // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

	return TRUE;
}

// 协议层发送处理过程
void ARM_PROTOCOL_TxStateProcess(void)
{
	uint16 head = armProtocolCB.tx.head;
	uint16 end = armProtocolCB.tx.end;
	uint16 length = armProtocolCB.tx.cmdQueue[head].length;
	uint8 *pCmd = armProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = armProtocolCB.tx.cmdQueue[head].deviceID;

	// 发送缓冲区为空，说明无数据
	if (head == end)
	{
		return;
	}

	// 发送函数没有注册直接返回
	if (NULL == armProtocolCB.sendDataThrowService)
	{
		return;
	}

	// 协议层有数据需要发送到驱动层
	if (!(*armProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// 发送环形队列更新位置
	armProtocolCB.tx.cmdQueue[head].length = 0;
	armProtocolCB.tx.head++;
	armProtocolCB.tx.head %= ARM_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void ARM_PROTOCOL_RxFIFOProcess(ARM_PROTOCOL_CB *pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
	uint16 length = 0;
	uint8 currentData = 0;

	// 参数合法性检验
	if (NULL == pCB)
	{
		return;
	}

	// 一级缓冲区为空，退出
	if (head == end)
	{
		return;
	}

	// 获取临时缓冲区指针
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	// 取出当前要处理的字节
	currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];

	// 临时缓冲区长度为0时，搜索首字节
	if (0 == pCmdFrame->length)
	{
		// 命令头错误，删除当前字节并退出
		if (ARM_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// 命令头正确，但无临时缓冲区可用，退出
		if ((pCB->rx.end + 1) % ARM_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// 添加UART通讯超时时间设置-2016.1.5增加
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
					  BLE_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
					  BLE_PROTOCOL_CALLBACK_RxTimeOut,
					  0,
					  1,
					  ACTION_MODE_ADD_TO_QUEUE);
#endif

		// 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
		pCmdFrame->buff[pCmdFrame->length++] = currentData;
		pCB->rxFIFO.currentProcessIndex++;
		pCB->rxFIFO.currentProcessIndex %= ARM_PROTOCOL_RX_FIFO_SIZE;
	}
	// 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
	else
	{
		// 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
		if (pCmdFrame->length >= ARM_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

			// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
			pCmdFrame->length = 0; // 2016.1.5增加
			// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// 一直取到末尾
		while (end != pCB->rxFIFO.currentProcessIndex)
		{
			// 取出当前要处理的字节
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			// 缓冲区未溢出，正常接收，将数据添加到临时缓冲区中
			pCmdFrame->buff[pCmdFrame->length++] = currentData;
			pCB->rxFIFO.currentProcessIndex++;
			pCB->rxFIFO.currentProcessIndex %= ARM_PROTOCOL_RX_FIFO_SIZE;

			// ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正 ■■

			// 首先判断命令帧最小长度，一个完整的命令字至少包括4个字节: 命令帧最小长度，包含:命令头、命令字、数据长度、校验和，因此不足4个字节的必定不完整
			if (pCmdFrame->length < ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 继续接收
				continue;
			}

			// 命令帧长度数值越界，说明当前命令帧错误，停止接收
			if (pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] > (ARM_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN))
			{
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// 命令帧长度校验
			length = pCmdFrame->length;
			if (length < pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] + ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 长度要求不一致，说明未接收完毕，退出继续
				continue;
			}

			// 命令帧长度OK，则进行校验，失败时删除命令头
			if (!ARM_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			BLE_PROTOCOL_StopRxTimeOutCheck();
#endif
			// 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			ARM_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// 对传入的命令帧进行校验，返回校验结果
BOOL ARM_PROTOCOL_CheckSUM(ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
	uint8 cc = 0;
	uint16 i = 0;

	if (NULL == pCmdFrame)
	{
		return FALSE;
	}

	// 从命令头开始，到校验码之前的一个字节，依次进行异或运算
	for (i = 0; i < pCmdFrame->length - 1; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}

	cc = ~cc;

	// 判断计算得到的校验码与命令帧中的校验码是否相同
	if (pCmdFrame->buff[pCmdFrame->length - 1] != cc)
	{
		return FALSE;
	}

	return TRUE;
}

void dutProgressReport(uint32 temp)
{
	uint8 i;

	if (dut_info.appUpFlag)
	{
		dut_info.dutProgress[3] = (dut_info.currentAppSize * 100) / dut_info.appSize;
	}
	else if (dut_info.uiUpFlag)
	{
		dut_info.dutProgress[5] = (dut_info.currentUiSize * 100) / dut_info.uiSize;
	}
	else if (dut_info.configUpFlag)
	{
		// 配置信息进度只有0%和100%。
	}
	else
	{
		// 所有项升级完毕
		TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
		TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);
		dut_info.configUpFaile = FALSE;
		dut_info.appUpFaile = FALSE;
		dut_info.uiUpFaile = FALSE;
	}

	if (dut_info.appUpFaile) // app升级失败
	{
		dut_info.appUpFaile = FALSE;
		dut_info.dutProgress[3] = 102;
		dut_info.currentAppSize = 0;
		TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
		dut_info.appUpFlag = FALSE;
	}
	
	if (dut_info.uiUpFaile) // ui升级失败
	{
		dut_info.dutProgress[5] = 102;
		dut_info.currentUiSize = 0;
		TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
		TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION); // 高标的超时检测
		dut_info.uiUpFaile = FALSE;
		dut_info.uiUpFlag = FALSE;
	}	
	
	if (dut_info.configUpFaile) // config升级失败
	{
		dut_info.dutProgress[7] = 102;
		TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
		dut_info.configUpFaile = FALSE;
		dut_info.configUpFlag = FALSE;
	}
	
	if (dut_info.appUpSuccesss)
	{
		dut_info.dutProgress[3] = 100;
		dut_info.currentAppSize = 0;
		dut_info.appUpSuccesss = FALSE;
	}

	if (dut_info.uiUpSuccesss)
	{
		dut_info.dutProgress[5] = 100;
		dut_info.currentUiSize = 0;
		dut_info.uiUpSuccesss = FALSE;
	}

	if (dut_info.configUpSuccesss)
	{
		dut_info.dutProgress[7] = 100;
		dut_info.configUpSuccesss = FALSE;
	}
	ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);		   // 添加命令头
	ARM_PROTOCOL_TxAddData(UART_ARM_CMD_TOOL_DUT_PROCESS); // 添加命令字0x15
	ARM_PROTOCOL_TxAddData(0x00);						   // 数据长度临时为0
	for (i = 0; i < 8; i++)
	{
		ARM_PROTOCOL_TxAddData(dut_info.dutProgress[i]);
	}
	ARM_PROTOCOL_TxAddFrame(); // 设置数据长度和添加校验
}

// 升级项控制
void setUpItem(uint8 upItem, uint8 ctrlState)
{
	dut_info.appUpFlag = FALSE;
	dut_info.uiUpFlag = FALSE;
	dut_info.configUpFlag = FALSE;
	// 清除状态
	dut_info.currentAppSize = 0;
	dut_info.currentUiSize = 0;
	if (1 == ctrlState)
	{
		if (0x02 == (upItem & 0x02)) // 升级app
		{
			dut_info.appUpFlag = TRUE;
		}

		if (0x04 == (upItem & 0x04)) // 升级ui
		{
			dut_info.uiUpFlag = TRUE;
		}

		if (0x08 == (upItem & 0x08)) // 升级config
		{
			dut_info.configUpFlag = TRUE;
		}
	}
}

// 设置进度上报信息
void setDutProgressReportInfo()
{
	dut_info.dutProgress[0] = 0;   // boot
	dut_info.dutProgress[1] = 101; // boot
	dut_info.dutProgress[2] = 1;   // app
	dut_info.dutProgress[4] = 2;   // ui
	dut_info.dutProgress[6] = 3;   // config

	if (!dut_info.appUpFlag)
	{
		dut_info.dutProgress[3] = 101;
	}
	else
	{
		dut_info.dutProgress[3] = 0;
	}

	if (!dut_info.uiUpFlag)
	{
		dut_info.dutProgress[5] = 101;
	}
	else
	{
		dut_info.dutProgress[5] = 0;
	}

	if (!dut_info.configUpFlag)
	{
		dut_info.dutProgress[7] = 101;
	}
	else
	{
		dut_info.dutProgress[7] = 0;
	}

	// 进度上报
	TIMER_AddTask(TIMER_ID_SET_DUT_PROGRESS_REPORT,
				  300,
				  dutProgressReport,
				  TRUE,
				  TIMER_LOOP_FOREVER,
				  ACTION_MODE_ADD_TO_QUEUE);
}

// 根据升级信息进入不同的状态机
void enterState()
{
	if (dut_info.appUpFlag || dut_info.uiUpFlag || dut_info.configUpFlag)
	{
		switch (dut_info.dutBusType)
		{
		case 0: // 串口
			switch (dut_info.ID)
			{
			case DUT_TYPE_SEG:
				// 进入数码管、段码屏升级
				STATE_EnterState(STATE_CM_UART_SEG_UPGRADE);
				break;
			case DUT_TYPE_CM:
				// 进入通用彩屏升级
				STATE_EnterState(STATE_CM_UART_BC_UPGRADE);
				break;
			case DUT_TYPE_HUAXIN:
				// 进入华芯微特升级
				STATE_EnterState(STATE_HUAXIN_UART_BC_UPGRADE);
				break;
			case DUT_TYPE_KAIYANG:
				// 进入开阳升级
				STATE_EnterState(STATE_KAIYANG_UART_BC_UPGRADE);
				break;
			default:
				break;
			}

			break;

		case 1: // can
			switch (dut_info.ID)
			{
			case DUT_TYPE_GB:
				// 进入高标升级
				STATE_EnterState(STATE_GAOBIAO_CAN_UPGRADE);
				break;

			case DUT_TYPE_SEG:
				// 进入数码管、段码屏升级
				STATE_EnterState(STATE_SEG_CAN_UPGRADE);
				break;

			case DUT_TYPE_HUAXIN:
				// 华芯微特升级
				STATE_EnterState(STATE_HUAXIN_CAN_UPGRADE);
				break;

			case DUT_TYPE_KAIYANG:
				// 进入开阳升级
				STATE_EnterState(STATE_KAIYANG_CAN_UPGRADE);
				break;

			default:
				// 进入通用can升级
				STATE_EnterState(STATE_CM_CAN_UPGRADE);
				break;
			}
			break;

		default:
			break;
		}
	}
	else
	{
		STATE_EnterState(STATE_STANDBY);
	}
}

// ARM命令帧缓冲区处理
void ARM_PROTOCOL_CmdFrameProcess(ARM_PROTOCOL_CB *pCB)
{
	ARM_PROTOCOL_CMD cmd = ARM_CMD_MAX;
	ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	BOOL writeRes;

	// 我的变量
	uint8 versionBoot[3] = {0};
	uint8 versionApp[3] = {0};

	uint8 upItem = 0;
	uint8 clearItem = 0;
	uint8 dutAll[50] = {0};
	uint8 dutName_i = 0;
	uint8 packetSize = 0;
	uint8 addr1 = 0;
	uint8 addr2 = 0;
	uint8 addr3 = 0;
	uint8 addr4 = 0;
	uint32 writeAddr = 0;
	static uint32 allPacketSize = 0;
	static uint8 allPacket[4] = {0};
	uint8 dataLen = 0;
	uint8 endItem = 0;
	uint8 ctrlState = 0;
	uint8 dut_info_len = 0;
	uint8 size[4] = {0};
	static uint32 currPacketNum = 0xFFFF;

	// 参数合法性检验
	if (NULL == pCB)
	{
		return;
	}

	// 命令帧缓冲区为空，退出
	if (pCB->rx.head == pCB->rx.end)
	{
		return;
	}

	// 获取当前要处理的命令帧指针
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.head];

	// 命令头非法，退出
	if (ARM_PROTOCOL_CMD_HEAD != pCmdFrame->buff[ARM_PROTOCOL_CMD_HEAD_INDEX])
	{
		// 删除命令帧
		pCB->rx.head++;
		pCB->rx.head %= ARM_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// 命令头合法，则提取命令
	cmd = (ARM_PROTOCOL_CMD)pCmdFrame->buff[ARM_PROTOCOL_CMD_CMD_INDEX];

	// 执行命令帧
	switch (cmd)
	{
	// 获取烧录器工具信息
	case UART_ARM_CMD_GET_TOOL_INFO:										 // 0x10,
		SPI_FLASH_ReadArray(versionBoot, SPI_FLASH_BOOT_VERSION_ADDEESS, 3); // 获取BOOT版本
		SPI_FLASH_ReadArray(versionApp, SPI_FLASH_APP_VERSION_ADDEESS, 3);	 // 获取APP版本
		ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);						 // 添加命令头
		ARM_PROTOCOL_TxAddData(UART_ARM_CMD_GET_TOOL_INFO);					 // 添加命令字
		ARM_PROTOCOL_TxAddData(0x00);										 // 数据长度临时为0
		ARM_PROTOCOL_TxAddData(versionBoot[0]);								 // 添加BOOT版本
		ARM_PROTOCOL_TxAddData(versionBoot[1]);
		ARM_PROTOCOL_TxAddData(versionBoot[2]);
		ARM_PROTOCOL_TxAddData(versionApp[0]); // 添加APP版本
		ARM_PROTOCOL_TxAddData(versionApp[1]);
		ARM_PROTOCOL_TxAddData(versionApp[2]);
		ARM_PROTOCOL_TxAddFrame(); // 设置数据长度和添加校验
		break;

	// 烧录器工具升级开始
	case UART_ARM_CMD_TOOL_UP_BEGIN: // 0x11,
		// 擦除片外flash，大概要1s钟
		SPI_FLASH_EraseRoom(SPI_FLASH_TOOL_APP_ADDEESS, 256 * 1024); // 256k
		// 返回信息给安卓机
		allPacketSize = 0;
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_UP_BEGIN, TRUE);
		break;

	// 烧录器工具数据写入
	case UART_ARM_CMD_TOOL_WRITE_DATA: // 0x12,
		// 向片外flash写入安卓机发送的数据
		packetSize = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 4;
		addr1 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		addr2 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];
		addr3 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX];
		addr4 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX];
		writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);

		// 指向要写入的数据缓冲区的指针pBuffer，一个表示写入地址的无符号32位整数WriteAddr，以及一个表示要写入的字节数的无符号16位整数NumByteToWrite。
		writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_TOOL_APP_ADDEESS + writeAddr, packetSize);

		// 返回信息给安卓机
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_WRITE_DATA, writeRes);
		allPacketSize++;
		break;

	// 烧录器工具升级结束
	case UART_ARM_CMD_TOOL_UP_END: // 0x13,
		// 发送升级成功应答
		// 设置升级标志位标志
		allPacket[0] = allPacketSize >> 24;
		allPacket[1] = allPacketSize >> 16;
		allPacket[2] = allPacketSize >> 8;
		allPacket[3] = allPacketSize;

		SPI_FLASH_EraseSector(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS); // 擦除标志位存储区
		SPI_FLASH_EraseSector(SPI_FLASH_TOOL_APP_DATA_SIZE);	   // 擦除大小存储区

		SPI_FLASH_WriteByte(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS, 0xAA);
		SPI_FLASH_WriteArray(allPacket, SPI_FLASH_TOOL_APP_DATA_SIZE, 4);

		ARM_PROTOCOL_SendCmdAck(UART_ARM_CMD_TOOL_UP_END);

		/*100ms后跳转回BOOT，读取标志位，然后替换替换工具APP区*/
		TIMER_AddTask(TIMER_ID_TOOL_APP_TO_BOOT,
					  100,
					  IAP_JumpToAppFun,
					  IAP_GD32_FLASH_BASE,
					  1,
					  ACTION_MODE_ADD_TO_QUEUE);
		break;

	// 升级DUT控制命令
	case UART_ARM_CMD_TOOL_DUT_UP: // 0x14,
		upItem = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		ctrlState = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];

		// 设置要升级的项
		setUpItem(upItem, ctrlState);

		// 设置进度上报信息
		setDutProgressReportInfo();

		// 根据升级信息，进入不同的状态机
		enterState();

		// 应答
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_DUT_UP, TRUE);
		break;

	// DUT升级进度上报
	case UART_ARM_CMD_TOOL_DUT_PROCESS: // 0x15,
		break;

	// 清空升级缓冲区
	case UART_ARM_CMD_TOOL_CLEAR_BUFF: // 0x16,

		clearItem = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		if (0x01 == (clearItem & 0x01))
		{
			/*擦除片外的dut_boot区*/
			dut_info.bootSize = 0;
			SPI_FLASH_EraseRoom(SPI_FLASH_DUT_BOOT_ADDEESS, 128 * 1024); // 128k
		}
		if (0x02 == (clearItem & 0x02))
		{
			/*擦除片外的dut_app区*/
			dut_info.appSize = 0;
			SPI_FLASH_EraseRoom(SPI_FLASH_DUT_APP_ADDEESS, 1024 * 1024); // 1M
		}
		if (0x04 == (clearItem & 0x04))
		{
			/*擦除片外的dut_ui区*/
			dut_info.uiSize = 0;
			SPI_FLASH_EraseRoom(SPI_FLASH_DUT_UI_ADDEESS, 14 * 1024 * 1024); // 14M
		}
		if (0x08 == (clearItem & 0x08))
		{
			/*擦除片外的dut_config区*/
			SPI_FLASH_EraseSector(SPI_FLASH_DUT_CONFIG_ADDEESS); // 4k
		}
		SPI_FLASH_EraseSector(SPI_FLASH_DUT_INFO); // 擦除dut机型信息
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_CLEAR_BUFF, TRUE);

		break;

	// 烧录器写入DUT机型信息
	case UART_ARM_CMD_TOOL_SET_DUT_INFO: // 0x17,
		dut_info_len = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 6;

		for (dutName_i = 0; dutName_i < dut_info_len; dutName_i++)
		{
			dutAll[dutName_i] = pCmdFrame->buff[dutName_i + 6 + 3];
		}
		dutAll[20] = dut_info_len;									// 长度
		dutAll[21] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX]; // 电压
		dutAll[22] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX]; // 通讯类型
		dutAll[23] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX]; // bootType
		dutAll[24] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX]; // appType
		dutAll[25] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX]; // uiType
		dutAll[26] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA6_INDEX]; // configType

		/* 将机型信息写入到片外flash */
		SPI_FLASH_EraseSector(SPI_FLASH_DUT_INFO);
		writeRes = SPI_FLASH_WriteWithCheck(dutAll, SPI_FLASH_DUT_INFO, 30);

		if (writeRes)
		{
			DutInfoUpdata(); // 更新dutinfo数据
			if (dut_info.voltage)// 更新电压数据
			{
				DutVol_24();
			}
			else
			{
				DutVol_12();
			}
		}
		/* 返回信息给安卓机 */
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_SET_DUT_INFO, writeRes);
		break;

	// 获取烧录器记录的DUT机型信息
	case UART_ARM_CMD_TOOL_GET_DUT_INFO: // 0x18
		SPI_FLASH_ReadArray(dutAll, SPI_FLASH_DUT_INFO, 30);
		if (0xFF == dutAll[20]) // 机型为空
		{
			dutAll[20] = 10;
		}
		ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);			 // 添加命令头
		ARM_PROTOCOL_TxAddData(UART_ARM_CMD_TOOL_GET_DUT_INFO);	 // 添加命令字
		ARM_PROTOCOL_TxAddData(0x00);							 // 数据长度临时为0
		ARM_PROTOCOL_TxAddData(dutAll[21]);						 // 添加电压
		ARM_PROTOCOL_TxAddData(dutAll[22]);						 // 添加通讯类型
		ARM_PROTOCOL_TxAddData(dutAll[23]);						 // bootType
		ARM_PROTOCOL_TxAddData(dutAll[24]);						 // appType
		ARM_PROTOCOL_TxAddData(dutAll[25]);						 // uiType
		ARM_PROTOCOL_TxAddData(dutAll[26]);						 // configType
		for (dutName_i = 0; dutName_i < dutAll[20]; dutName_i++) // 添加机型信息
		{
			ARM_PROTOCOL_TxAddData(dutAll[dutName_i]);
		}
		ARM_PROTOCOL_TxAddFrame(); // 设置数据长度和添加校验
		break;

	// 0x19:设置工具序列号
	case UART_ARM_CMD_SET_TOOL_RANK:
		SPI_FLASH_EraseSector(SPI_FLASH_TOOL_RANK_ADDEESS); // 擦除序号存储区
		dut_info.toolRank = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		SPI_FLASH_WriteByte(SPI_FLASH_TOOL_RANK_ADDEESS, dut_info.toolRank); // 写入序号
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_SET_TOOL_RANK, 0x01);		 // 设置数据长度和添加校验
		break;

	// 0x20:获取工具序列号
	case UART_ARM_CMD_GET_TOOL_RANK:
		dut_info.toolRank = SPI_FLASH_ReadByte(SPI_FLASH_TOOL_RANK_ADDEESS);
		ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);		// 添加命令头
		ARM_PROTOCOL_TxAddData(UART_ARM_CMD_GET_TOOL_RANK); // 添加命令字
		ARM_PROTOCOL_TxAddData(0x00);						// 添加临时长度
		ARM_PROTOCOL_TxAddData(dut_info.toolRank);
		ARM_PROTOCOL_TxAddFrame();
		break;

	// 写入DUT参数（命令字=0x30
	case UART_DUT_CMD_UP_CONFIG:
		// 读取配置信息的flash区，然后写入DUT
		dataLen = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX];
		writeRes = SPI_FLASH_WriteWithCheck(&(pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX]), SPI_FLASH_DUT_CONFIG_ADDEESS, dataLen);
		// 返回信息给安卓机
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_CONFIG, writeRes);
		break;

	case UART_DUT_CMD_UP_UI: // UI数据写入（命令字=0x31）
		// 读取UI的flash区，然后写入DUT
		packetSize = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 4;
		addr1 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		addr2 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];
		addr3 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX];
		addr4 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX];
		writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);
		if (currPacketNum != writeAddr)
		{
			dut_info.uiSize++;
			currPacketNum = writeAddr;
		}
		writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_DUT_UI_ADDEESS + writeAddr, packetSize);
		// 返回信息给安卓机
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_UI, writeRes);
		break;

	// BOOT数据写入（命令字=0x32）
	case UART_DUT_CMD_UP_BOOT:
		// 读取BOOT的flash区，然后写入DUT
		packetSize = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 4;
		addr1 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		addr2 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];
		addr3 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX];
		addr4 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX];
		writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);
		if (currPacketNum != writeAddr)
		{
			dut_info.bootSize++;
			currPacketNum = writeAddr;
		}
		writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_DUT_BOOT_ADDEESS + writeAddr, packetSize);
		// 返回信息给安卓机
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_BOOT, writeRes);
		break;

	// APP数据写入（命令字=0x33）
	case UART_DUT_CMD_UP_APP:
		packetSize = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 4;
		addr1 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		addr2 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];
		addr3 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX];
		addr4 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX];
		writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);
		if (currPacketNum != writeAddr)
		{
			dut_info.appSize++;
			currPacketNum = writeAddr;
		}
		writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_DUT_APP_ADDEESS + writeAddr, packetSize);
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_APP, writeRes);
		break;

	// 升级项结束（命令字=0x34）
	case UART_DUT_CMD_UP_END:
		endItem = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		switch (endItem)
		{
		case 0:
			SPI_FLASH_EraseSector(SPI_FLASH_DUT_BOOT_SIZE_ADDEESS);
			size[0] = dut_info.bootSize >> 24;
			size[1] = dut_info.bootSize >> 16;
			size[2] = dut_info.bootSize >> 8;
			size[3] = dut_info.bootSize;
			SPI_FLASH_WriteArray(size, SPI_FLASH_DUT_BOOT_SIZE_ADDEESS, 4);
			break;
		case 1:
			SPI_FLASH_EraseSector(SPI_FLASH_DUT_APP_SIZE_ADDEESS);
			size[0] = dut_info.appSize >> 24;
			size[1] = dut_info.appSize >> 16;
			size[2] = dut_info.appSize >> 8;
			size[3] = dut_info.appSize;
			SPI_FLASH_WriteArray(size, SPI_FLASH_DUT_APP_SIZE_ADDEESS, 4);
			break;
		case 2:
			SPI_FLASH_EraseSector(SPI_FLASH_DUT_UI_SIZE_ADDEESS);
			size[0] = dut_info.uiSize >> 24;
			size[1] = dut_info.uiSize >> 16;
			size[2] = dut_info.uiSize >> 8;
			size[3] = dut_info.uiSize;
			SPI_FLASH_WriteArray(size, SPI_FLASH_DUT_UI_SIZE_ADDEESS, 4);
			break;
		}
		currPacketNum = 0xFFFF;
		// 返回信息给安卓机
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_END, endItem);
		break;

	default:
		break;
	}

	// 删除命令帧
	pCB->rx.head++;
	pCB->rx.head %= ARM_PROTOCOL_RX_QUEUE_SIZE;
}

// RX通讯超时处理-单向
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void BLE_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	pCmdFrame = &armProtocolCB.rx.cmdQueue[armProtocolCB.rx.end];

	// 超时错误，将命令帧长度清零，即认为抛弃该命令帧
	pCmdFrame->length = 0; // 2016.1.6增加
	// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
	armProtocolCB.rxFIFO.head++;
	armProtocolCB.rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
	armProtocolCB.rxFIFO.currentProcessIndex = armProtocolCB.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void BLE_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

#if BLE_PROTOCOL_TXRX_TIME_OUT_CHECK_ENABLE
// TXRX通讯超时处理-双向
void BLE_PROTOCOL_CALLBACK_TxRxTimeOut(uint32 param)
{
}

// 停止TxRX通讯超时检测任务
void BLE_PROTOCOL_StopTxRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_TXRX_TIME_OUT_CONTROL);
}
#endif

// 发送命令回复
void ARM_PROTOCOL_SendCmdAck(uint8 ackCmd)
{
	ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);
	ARM_PROTOCOL_TxAddData(ackCmd);
	ARM_PROTOCOL_TxAddData(0x00);
	ARM_PROTOCOL_TxAddFrame();
}

// 发送命令回复，带一个参数
void ARM_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{

	ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);
	ARM_PROTOCOL_TxAddData(ackCmd);
	ARM_PROTOCOL_TxAddData(0x01);

	ARM_PROTOCOL_TxAddData(ackParam);
	ARM_PROTOCOL_TxAddFrame();
}

// 发送获取MAC地址
void ARM_PROTOCOL_SendGetMacAddr(uint32 param)
{
	uint8 bleMacStr[] = "TTM:MAC-?";

	// 使能BLE发送
	ARM_BRTS_TX_REQUEST();

	// 将待发送命令添加到物理层缓冲区
	ARM_UART_AddTxArray(0xFFFF, bleMacStr, sizeof(bleMacStr));
}
