#include "common.h"
#include "timer.h"
#include "Param.h"
#include "ds1302.h"
#include "state.h"
#include "display.h"
#include "record.h"
#include "backlight.h"
#include "spiflash.h"
#include "iap.h"
#include "stringOperation.h"
#include "BleHostUart.h"
#include "BleHostProtocol.h"
#include "BleProtocol1.h"

/******************************************************************************
* 【内部接口声明】
******************************************************************************/

// 数据结构初始化
void BLE_HOST_PROTOCOL_DataStructInit(BLE_HOST_PROTOCOL_CB* pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void BLE_HOST_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void BLE_HOST_PROTOCOL_RxFIFOProcess(BLE_HOST_PROTOCOL_CB* pCB);

// UART命令帧缓冲区处理
void BLE_HOST_PROTOCOL_CmdFrameProcess(BLE_HOST_PROTOCOL_CB* pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL BLE_HOST_PROTOCOL_CheckSUM(BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL BLE_HOST_PROTOCOL_ConfirmTempCmdFrameBuff(BLE_HOST_PROTOCOL_CB* pCB);

// 通讯超时处理-单向
void BLE_HOST_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// 停止RX通讯超时检测任务
void BLE_HOST_PROTOCOL_StopRxTimeOutCheck(void);

// TXRX通讯超时处理-双向
void BLE_HOST_PROTOCOL_CALLBACK_TxRxTimeOut(uint32 param);

// 停止TXRX通讯超时检测任务
void BLE_HOST_PROTOCOL_StopTxRxTimeOutCheck(void);

// 协议层发送处理过程
void BLE_HOST_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void BLE_HOST_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// 启动通讯超时判断任务
void BLE_HOST_PROTOCOL_StartTimeoutCheckTask(void);

// UART总线超时错误处理
void BLE_HOST_PROTOCOL_CALLBACK_UartBusError(uint32 param);

// 状态迁移
void BLE_PairEnterState(uint32 state);

//=======================================================================================
// 全局变量定义
BLE_HOST_PROTOCOL_CB bleHostProtocolCB;
extern uint8 callringBuff[20];

// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// 启动通讯超时判断任务
void BLE_HOST_PROTOCOL_StartTimeoutCheckTask(void)
{
}


// 协议初始化
void BLE_HOST_PROTOCOL_Init(void)
{
	// 协议层数据结构初始化
	BLE_HOST_PROTOCOL_DataStructInit(&bleHostProtocolCB);

	// 向驱动层注册数据接收接口
	BLE_HOST_UART_RegisterDataSendService(BLE_HOST_PROTOCOL_MacProcess);

	// 向驱动层注册数据发送接口
	BLE_HOST_PROTOCOL_RegisterDataSendService(BLE_HOST_UART_AddTxArray);

	// 上电进入蓝牙配对初始化
	BLE_PairEnterState(BLE_PAIR_INIT);	
}

// BLE协议层过程处理
void BLE_HOST_PROTOCOL_Process(void)
{
	// BLE接收FIFO缓冲区处理
	BLE_HOST_PROTOCOL_RxFIFOProcess(&bleHostProtocolCB);

	// BLE接收命令缓冲区处理
	BLE_HOST_PROTOCOL_CmdFrameProcess(&bleHostProtocolCB);
	
	// BLE协议层发送处理过程
	BLE_HOST_PROTOCOL_TxStateProcess();
	
	// 蓝牙配对处理过程
	BLE_PairProcess();
}

// 向发送命令帧队列中添加数据
void BLE_HOST_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = bleHostProtocolCB.tx.head;
	uint16 end =  bleHostProtocolCB.tx.end;
	BLE_HOST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &bleHostProtocolCB.tx.cmdQueue[bleHostProtocolCB.tx.end];

	// 发送缓冲区已满，不予接收
	if((end + 1) % BLE_HOST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 队尾命令帧已满，退出
	if(pCmdFrame->length >= BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 数据添加到帧末尾，并更新帧长度
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void BLE_HOST_PROTOCOL_TxAddFrame(void)
{
	uint16 cc = 0;
	uint16 i = 0;
	uint16 head = bleHostProtocolCB.tx.head;
	uint16 end  = bleHostProtocolCB.tx.end;
	BLE_HOST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &bleHostProtocolCB.tx.cmdQueue[bleHostProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// 发送缓冲区已满，不予接收
	if((end + 1) % BLE_HOST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 命令帧长度不足，清除已填充的数据，退出
	if(BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN-1 > length)	// 减去"校验和"1个字节
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// 队尾命令帧已满，退出
	if(length >= BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
	pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_LENGTH_INDEX] = length - 3;	// 重设数据长度

	for(i=0; i<length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = ~cc ;

	bleHostProtocolCB.tx.end ++;
	bleHostProtocolCB.tx.end %= BLE_HOST_PROTOCOL_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

// 添加TTM命令
void BLE_HOST_PROTOCOL_TxAddTtmCmd(uint8 data[], uint16 length, uint16 id)
{
	uint16 i = 0;
	uint16 head = bleHostProtocolCB.ttmTx.head;
	uint16 end  = bleHostProtocolCB.ttmTx.end;
	BLE_HOST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &bleHostProtocolCB.ttmTx.cmdQueue[bleHostProtocolCB.ttmTx.end];

	// 发送缓冲区已满，不予接收
	if((end + 1) % BLE_HOST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 队尾命令帧已满，退出
	if(length >= BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	for (i = 0; i < length; i++)
	{
		pCmdFrame->buff[i] = data[i];
	}
	
	pCmdFrame->length = length;

	pCmdFrame->deviceID = id;

	bleHostProtocolCB.ttmTx.end ++;
	bleHostProtocolCB.ttmTx.end %= BLE_HOST_PROTOCOL_TX_QUEUE_SIZE;
}

// 数据结构初始化
void BLE_HOST_PROTOCOL_DataStructInit(BLE_HOST_PROTOCOL_CB* pCB)
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
	for(i = 0; i < BLE_HOST_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<BLE_HOST_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}

	pCB->isTimeCheck = FALSE;
	
	pCB->pair.state = BLE_PAIR_INIT;
	pCB->pair.preState = BLE_PAIR_INIT;
	pCB->pair.mode = BLE_NORMAL_MODE;
}

// UART报文接收处理函数(注意根据具体模块修改)
void BLE_HOST_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length)
{
	uint16 end = bleHostProtocolCB.rxFIFO.end;
	uint16 head = bleHostProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;
	
	// 接收数据
	rxdata = *pData;

	// 一级缓冲区已满，不予接收
	if((end + 1)%BLE_HOST_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// 一级缓冲区未满，接收 
	else
	{
		// 将接收到的数据放到临时缓冲区中
		bleHostProtocolCB.rxFIFO.buff[end] = rxdata;
		bleHostProtocolCB.rxFIFO.end ++;
		bleHostProtocolCB.rxFIFO.end %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
	}

	// 嵌一套协议
	BLE_PROTOCOL1_MacProcess(standarID, pData, length);
}

// UART协议层向驱动层注册数据发送接口
void BLE_HOST_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{		
	bleHostProtocolCB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL BLE_HOST_PROTOCOL_ConfirmTempCmdFrameBuff(BLE_HOST_PROTOCOL_CB* pCB)
{
	BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
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
	pCB->rx.end %= BLE_HOST_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区
	
	return TRUE;
}

// 协议层发送处理过程
void BLE_HOST_PROTOCOL_TxStateProcess(void)
{
	uint16 head = bleHostProtocolCB.tx.head;
	uint16 end =  bleHostProtocolCB.tx.end;
	uint16 length = bleHostProtocolCB.tx.cmdQueue[head].length;
	uint8* pCmd = bleHostProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = bleHostProtocolCB.tx.cmdQueue[head].deviceID;

	// 发送缓冲区为空，说明无数据
	if (head == end)
	{
		return;
	}

	// 发送函数没有注册直接返回
	if (NULL == bleHostProtocolCB.sendDataThrowService)
	{
		return;
	}

	// 协议层有数据需要发送到驱动层		
	if (!(*bleHostProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// 发送环形队列更新位置
	bleHostProtocolCB.tx.cmdQueue[head].length = 0;
	bleHostProtocolCB.tx.head ++;
	bleHostProtocolCB.tx.head %= BLE_HOST_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void BLE_HOST_PROTOCOL_RxFIFOProcess(BLE_HOST_PROTOCOL_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
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
		if(BLE_HOST_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// 命令头正确，但无临时缓冲区可用，退出
		if((pCB->rx.end + 1)%BLE_HOST_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// 添加UART通讯超时时间设置-2016.1.5增加
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
						BLE_HOST_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
						BLE_HOST_PROTOCOL_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
	}
	// 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
	else
	{
		// 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
		if(pCmdFrame->length >= BLE_HOST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			BLE_HOST_PROTOCOL_StopRxTimeOutCheck();
#endif

			// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
			pCmdFrame->length = 0;	// 2016.1.5增加
			// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
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
			pCB->rxFIFO.currentProcessIndex %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;

			// ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正 ■■
						
			// 首先判断命令帧最小长度，一个完整的命令字至少包括4个字节: 命令帧最小长度，包含:命令头、命令字、数据长度、校验和，因此不足4个字节的必定不完整
			if(pCmdFrame->length < BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 继续接收
				continue;
			}

			// 命令帧长度数值越界，说明当前命令帧错误，停止接收
			if(pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_LENGTH_INDEX] > (BLE_HOST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN))
			{
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				BLE_HOST_PROTOCOL_StopRxTimeOutCheck();
#endif
			
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// 命令帧长度校验
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_LENGTH_INDEX] + BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 长度要求不一致，说明未接收完毕，退出继续
				continue;
			}

			// 命令帧长度OK，则进行校验，失败时删除命令头
			if(!BLE_HOST_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				BLE_HOST_PROTOCOL_StopRxTimeOutCheck();
#endif
				
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			BLE_HOST_PROTOCOL_StopRxTimeOutCheck();
#endif
			
			// 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			BLE_HOST_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// 对传入的命令帧进行校验，返回校验结果
BOOL BLE_HOST_PROTOCOL_CheckSUM(BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame)
{
	uint8 cc = 0;
	uint16 i = 0;
	
	if(NULL == pCmdFrame)
	{
		return FALSE;
	}

	// 从命令头开始，到校验码之前的一个字节，依次进行异或运算
	for(i=0; i<pCmdFrame->length-1; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}

	cc = ~cc;
	
	// 判断计算得到的校验码与命令帧中的校验码是否相同
	if(pCmdFrame->buff[pCmdFrame->length-1] != cc)
	{
		return FALSE;
	}
	
	return TRUE;
}

// BLE命令帧缓冲区处理
void BLE_HOST_PROTOCOL_CmdFrameProcess(BLE_HOST_PROTOCOL_CB* pCB)
{
	BLE_HOST_PROTOCOL_CMD cmd = BLE_HOST_CMD_MAX;
	BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	uint8 i = 0;
	
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
	if(BLE_HOST_PROTOCOL_CMD_HEAD != pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_HEAD_INDEX])
	{
		// 删除命令帧
		pCB->rx.head ++;
		pCB->rx.head %= BLE_HOST_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// 命令头合法，则提取命令
	cmd = (BLE_HOST_PROTOCOL_CMD)pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_CMD_INDEX];
	
	// 执行命令帧
	switch (cmd)
	{
		// 头盔控制命令
		case BLE_OUTFIT_PROTOCOL_CMD_HELMET:

			// 转向灯
			if (0x01 == pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA1_INDEX])
			{
				PARAM_SetTurnState((TRUN_STATE)pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA3_INDEX]);
			}

			break;

		// 头盔状态
		case BLE_OUTFIT_PROTOCOL_CMD_HELMET_STATE:
			
			// 头盔电量百分比获取
			if (0x01 == pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA1_INDEX])
			{
				PARAM_SetHelmetBattery(pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA2_INDEX]);
			}
			// 播放状态,音量值
			else if (0x02 == pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA1_INDEX])
			{
				paramCB.runtime.musicState = pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA2_INDEX];
				paramCB.runtime.musicVolume = pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA4_INDEX];				
			}
	
			break;
			
		// 头盔来电提醒命令
		case BLE_OUTFIT_PROTOCOL_CMD_HELMET_PHONE:

			PARAM_SetHelmetCaller(pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA1_INDEX]);

			// 接听或者挂断清除忽略标识
			if ((0x01 == PARAM_GetHelmetCaller()) || (0x02 == PARAM_GetHelmetCaller()))
			{
				PARAM_SetCallerIgnoreState(FALSE);
			}
			
			memset(callringBuff, 0xFF, sizeof((char *)callringBuff));
			for (i = 0; i < 11; i++)
			{
				callringBuff[i] = pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA2_INDEX + i] - '0';
			} 
			break;
			
		default:
			break;
	}

	// 启动UART总线通讯超时判断
	BLE_HOST_PROTOCOL_StartTimeoutCheckTask();
	
	// 删除命令帧
	pCB->rx.head ++;
	pCB->rx.head %= BLE_HOST_PROTOCOL_RX_QUEUE_SIZE;
}

// RX通讯超时处理-单向
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void BLE_HOST_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &bleHostProtocolCB.rx.cmdQueue[bleHostProtocolCB.rx.end];

	// 超时错误，将命令帧长度清零，即认为抛弃该命令帧
	pCmdFrame->length = 0;	// 2016.1.6增加
	// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
	bleHostProtocolCB.rxFIFO.head ++;
	bleHostProtocolCB.rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
	bleHostProtocolCB.rxFIFO.currentProcessIndex = bleHostProtocolCB.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void BLE_HOST_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

#if BLE_HOST_PROTOCOL_TXRX_TIME_OUT_CHECK_ENABLE
// TXRX通讯超时处理-双向
void BLE_HOST_PROTOCOL_CALLBACK_TxRxTimeOut(uint32 param)
{
	
}

// 停止TxRX通讯超时检测任务
void BLE_HOST_PROTOCOL_StopTxRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_TXRX_TIME_OUT_CONTROL);
}
#endif

// 上电断开头盔连接
void BLE_HOST_UART_SendCmdDisconnectHelmet(uint32 param)
{
	uint8 bleStr[100] = "TTM:DISCONNECT=55:34:FF:93:58:44\r\n";

	// 头盔无配对不处理
	if (!PARAM_GetHelmetPairState())
	{
		TIMER_KillTask(TIMER_ID_BLE_HELMET_PAIR);
		return;
	}
	
	// 读写handle固定
	sprintf((char *)bleStr, "TTM:DISCONNECT=%s\r\n", PARAM_GetHelmetMacBuff());

	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), BLE_OUTFIT_HELMET_ID);
}

// 连接头盔
void BLE_HOST_UART_SendCmdConnectHelmet(uint32 param)
{
	uint8 bleStr[100] = "TTM:CONNECT=55:34:FF:93:58:44,66,13\r\n";

	// 头盔无配对不处理
	if (!PARAM_GetHelmetPairState())
	{
		TIMER_KillTask(TIMER_ID_BLE_HELMET_PAIR);
		return;
	}
			
	// 读写handle固定
	sprintf((char *)bleStr, "TTM:CONNECT=%s,66,13\r\n", PARAM_GetHelmetMacBuff());
	
	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), BLE_OUTFIT_HELMET_ID);
}

// 上电断开无线按键连接
void BLE_HOST_UART_SendCmdDisconnectKey(uint32 param)
{
	uint8 bleStr[100] = "TTM:DISCONNECT=7C:69:6B:52:24:24\r\n";

	// 按键无配对不处理
	if (!PARAM_GetLockPairState())
	{
		TIMER_KillTask(TIMER_ID_BLE_LOCK_PAIR);
		return;
	}
	
	// 读写handle固定
	sprintf((char *)bleStr, "TTM:DISCONNECT=%s\r\n", PARAM_GetLockMacBuff());
	
	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), BLE_OUTFIT_LOCK_ID);
}

// 连接无线按键
void BLE_HOST_UART_SendCmdConnectKey(uint32 param)
{
	uint8 bleStr[100] = "TTM:CONNECT=7C:69:6B:52:24:24,17,13\r\n";

	// 按键无配对不处理
	if (!PARAM_GetLockPairState())
	{
		TIMER_KillTask(TIMER_ID_BLE_LOCK_PAIR);
		return;
	}
	
	// 读写handle固定
	sprintf((char *)bleStr, "TTM:CONNECT=%s,14,11\r\n", PARAM_GetLockMacBuff());
	
	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), BLE_OUTFIT_LOCK_ID);
}

// 扫描蓝牙设备
void BLE_HOST_UART_SendScanBleDevice(uint32 param)
{
	uint8 bleStr[100] = "TTM:SCAN=1,5\r\n";
	
	sprintf((char *)bleStr, "TTM:SCAN=1,%ld\r\n", param);
	
	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), 0x00);
}

// 连接设备
void BLE_HOST_UART_SendConnectBleDevice(BLE_PAIR_DEVICE device, uint8 macBuff[])
{
	uint8 bleStr[100] = "TTM:CONNECT=7C:69:6B:52:24:24,17,13\r\n";

	if (BLE_HELMET_DEVICE == device)
	{
		// 读写handle固定
		sprintf((char *)bleStr,  (char *)"TTM:CONNECT=%s,66,13\r\n", macBuff);
		
		// 添加到队列发送
		BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), 0x00);
	}
	else if (BLE_LOCK_DEVICE == device)
	{
		// 读写handle固定
		sprintf((char *)bleStr, "TTM:CONNECT=%s,17,13\r\n", macBuff);
		
		// 添加到队列发送
		BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), 0x00);
	}
}

// 定时器回调发送设备连接指令
void  BLE_HOST_UART_CALLBALL_OutfitConnect(uint32 param)
{
	if (param)
	{
		BLE_HOST_UART_SendCmdConnectHelmet(TRUE);
	}
	else
	{
		BLE_HOST_UART_SendCmdConnectKey(TRUE);
	}
}

// 头盔信息查询
void BLE_HOST_PROTOCOL_SendCmdGetHeadInfoPhone(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[7] = {0x55, 0xB2, 0x01, 0x01, 0x18, 0x0D, 0x0A};		// 获取状态组序号1
	uint8 str2[7] = {0x55, 0xB2, 0x01, 0x02, 0x1B, 0x0D, 0x0A};		// 获取状态组序号2

	// 无连接不发送
	if (!PARAM_GetHelmetConnectState())
	{
		return;
	}

	sprintf((char *)Buff, (char *)"TTM:SEND=%s,", PARAM_GetHelmetMacBuff());
	Index = strlen((char *)Buff);
	
	if (1 == param)
	{
		for (i = 0; i < sizeof(str1); i++)
		{
			Buff[Index++] = str1[i];
		}
	}
	else
	{
		for (i = 0; i < sizeof(str2); i++)
		{
			Buff[Index++] = str2[i];
		}
	}

	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// 来电控制功能
void BLE_HOST_PROTOCOL_SendCmdControlPhone(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[9] = {0x55, 0xB1, 0x03, 0x07, 0x00, 0x01, 0x1E, 0x0D, 0x0A};		// 接听
	uint8 str2[9] = {0x55, 0xB1, 0x03, 0x07, 0x00, 0x03, 0x1C, 0x0D, 0x0A};		// 挂断

	// 无连接不发送
	if (!PARAM_GetHelmetConnectState())
	{
		return;
	}

	sprintf((char *)Buff, (char *)"TTM:SEND=%s,", PARAM_GetHelmetMacBuff());
	Index = strlen((char *)Buff);
	
	if (1 == param)
	{
		for (i = 0; i < sizeof(str1); i++)
		{
			Buff[Index++] = str1[i];
		}
	}
	else
	{
		for (i = 0; i < sizeof(str2); i++)
		{
			Buff[Index++] = str2[i];
		}
	}

	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// 多媒体控制功能
void BLE_HOST_PROTOCOL_SendCmdControlMp3(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x01, 0x11, 0x0D, 0x0A};	// 播放
	uint8 str2[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x02, 0x12, 0x0D, 0x0A}; // 暂停
	uint8 str3[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x03, 0x13, 0x0D, 0x0A}; // 上一曲
	uint8 str4[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x04, 0x14, 0x0D, 0x0A}; // 下一曲
	uint8 str5[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x08, 0x18, 0x0D, 0x0A}; // 音量+
	uint8 str6[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x09, 0x19, 0x0D, 0x0A}; // 音量-
	
	// 无连接不发送
	if (!PARAM_GetHelmetConnectState())
	{
		return;
	}

	sprintf((char *)Buff, (char *)"TTM:SEND=%s,", PARAM_GetHelmetMacBuff());
	Index = strlen((char *)Buff);
	
	switch (param)
	{
		// 播放
		case BLE_MUSIC_PLAY:
			for (i = 0; i < sizeof(str1); i++)
			{
				Buff[Index++] = str1[i];
			}
			break;
		
		// 暂停	
		case BLE_MUSIC_PAUSE:
			for (i = 0; i < sizeof(str2); i++)
			{
				Buff[Index++] = str2[i];
			}
			break;
		
		// 上一曲	
		case BLE_MUSIC_UP:
			for (i = 0; i < sizeof(str3); i++)
			{
				Buff[Index++] = str3[i];
			}
			break;
		
		// 下一曲			
		case BLE_MUSIC_NEXT:
			for (i = 0; i < sizeof(str4); i++)
			{
				Buff[Index++] = str4[i];
			}
			break;
		
		// 音量+	
		case BLE_MUSIC_ADD:
			for (i = 0; i < sizeof(str5); i++)
			{
				Buff[Index++] = str5[i];
			}
			break;	
		
		// 音量-		
		case BLE_MUSIC_DEC:
			for (i = 0; i < sizeof(str6); i++)
			{
				Buff[Index++] = str6[i];
			}
			break;	
			
		default:
			break;
		
	}
	
	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// 左转向灯控制
void BLE_HOST_PROTOCOL_SendCmdLeftTurnLight(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[9] = {0x55, 0xB1, 0x03, 0x01, 0x00, 0x01, 0x18, 0x0D, 0x0A};
	uint8 str2[9] = {0x55, 0xB1, 0x03, 0x01, 0x00, 0x00, 0x19, 0x0D, 0x0A};

	// 无连接不发送
	if (!PARAM_GetHelmetConnectState())
	{
		return;
	}

	sprintf((char *)Buff, (char *)"TTM:SEND=%s,", PARAM_GetHelmetMacBuff());
	Index = strlen((char *)Buff);
	
	if (param)
	{
		for (i = 0; i < sizeof(str1); i++)
		{
			Buff[Index++] = str1[i];
		}
	}
	else
	{
		for (i = 0; i < sizeof(str2); i++)
		{
			Buff[Index++] = str2[i];
		}
	}

	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// 右转向灯控制
void BLE_HOST_PROTOCOL_SendCmdRightTurnLight(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[9] = {0x55, 0xB1, 0x03, 0x01, 0x00, 0x02, 0x1B, 0x0D, 0x0A};
	uint8 str2[9] = {0x55, 0xB1, 0x03, 0x01, 0x00, 0x00, 0x19, 0x0D, 0x0A};

	// 无连接不发送
	if (!PARAM_GetHelmetConnectState())
	{
		return;
	}

	sprintf((char *)Buff, (char *)"TTM:SEND=%s,", PARAM_GetHelmetMacBuff());
	Index = strlen((char *)Buff);
	
	if (param)
	{
		for (i = 0; i < sizeof(str1); i++)
		{
			Buff[Index++] = str1[i];
		}
	}
	else
	{
		for (i = 0; i < sizeof(str2); i++)
		{
			Buff[Index++] = str2[i];
		}
	}

	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// 电子锁控制
void BLE_HOST_PROTOCOL_SendCmdGetKey(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[17] = {0xA3, 0xA4, 0x08, 0x50, 0x1E, 0x1F, 0x67, 0x51, 0x4A, 0x73, 0x55, 0x2B, 0x2E, 0x64, 0x00, 0x0D, 0x0A};

	// 无连接不发送
	if (!PARAM_GetLockConnectState())
	{
		return;
	}

	sprintf((char *)Buff, (char *)"TTM:SEND=%s,", PARAM_GetLockMacBuff());
	Index = strlen((char *)Buff);
	
	for (i = 0; i < sizeof(str1); i++)
	{
		Buff[Index++] = str1[i];
	}
	
	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, BLE_OUTFIT_LOCK_ID);
}

// 打开电子锁
void BLE_HOST_PROTOCOL_SendCmdLockOpen(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[100] = {0};
	uint8 length = 0;
		
	// 无连接不发送
	if (!PARAM_GetLockConnectState())
	{
		return;
	}

	sprintf((char *)Buff, (char *)"TTM:SEND=%s,", PARAM_GetLockMacBuff());
	Index = strlen((char *)Buff);

	BLE_PROTOCOL1_TxAddData(BLE_PROTOCOL1_CMD_HEAD);
	BLE_PROTOCOL1_TxAddData(BLE_PROTOCOL1_CMD_HEAD1);
	BLE_PROTOCOL1_TxAddData(0x00);								// 长度
	BLE_PROTOCOL1_TxAddData(0x1E);								// 随机数
	BLE_PROTOCOL1_TxAddData(PARAM_GetLockKey());				// 秘钥
	BLE_PROTOCOL1_TxAddData(BLE_PROTOCOL1_OPEN_LOCK_CMD);		// 开锁
	BLE_PROTOCOL1_TxAddData(0x01);								// 数据
	BLE_PROTOCOL1_TxAddData(0x00);
	BLE_PROTOCOL1_TxAddData(0x00);
	BLE_PROTOCOL1_TxAddData(0x00);
	BLE_PROTOCOL1_TxAddData(0x01);
	BLE_PROTOCOL1_TxAddData(0x00);
	BLE_PROTOCOL1_TxAddData(0x00);
	BLE_PROTOCOL1_TxAddData(0x00);
	BLE_PROTOCOL1_TxAddData(0x01);
	BLE_PROTOCOL1_TxAddData(0x00);
	
	length = BLE_PROTOCOL1_TxAddFrame(str1);
	
	str1[length++] = 0x0D;
	str1[length++] = 0x0A;
	
	for (i = 0; i < length; i++)
	{
		Buff[Index++] = str1[i];
	}
	
	// 添加到队列发送
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, BLE_OUTFIT_LOCK_ID);
}

// 解析蓝牙收到的TTM命令
void BLE_HOST_UART_TTM_AnalysisCmd(uint8 buff[], uint8 length)
{
	uint8 bleIndex = 0;
	uint16 macIndex = 0;
		
	if((-1 != STRING_Find(buff, (uint8 *)"OK"))    ||
	   (-1 != STRING_Find(buff, (uint8 *)"ERROR")) ||
	   (-1 != STRING_Find(buff, (uint8 *)"FAIL")))
	{
		bleHostProtocolCB.sendCmd.Flag = TRUE;
	}

	// 配对模式
	if (BLE_PAIR_MODE == bleHostProtocolCB.pair.mode)
	{
		if (-1 != STRING_Find(buff, (uint8 *)"TTM:SCAN="))
		{
			// 扫描成功不再处理
			if (bleHostProtocolCB.pair.scanFlag)
			{
				return;
			}

			switch (bleHostProtocolCB.pair.device)
			{
				case BLE_HELMET_DEVICE:

					// 根据协议来，头盔配对状态下蓝牙名称为：PDTT-XXXXXXXXXXXX
					if (-1 != STRING_Find(buff, (uint8 *)"PDTT"))
					{
						bleIndex = STRING_Find(buff, (uint8 *)"PDTT");
						
						// 截取mac地址
						STRING_Left(&buff[bleIndex - 18], 17, bleHostProtocolCB.pair.macBuff);
						bleHostProtocolCB.pair.scanFlag = TRUE;
					}
					break;

				case BLE_LOCK_DEVICE:

					// 根据协议来，按键配对状态下蓝牙名称为：PDTK-XXXXXXXXXXXX
					if (-1 != STRING_Find(buff, (uint8 *)"PDTK"))
					{
						bleIndex = STRING_Find(buff, (uint8 *)"PDTK");
						
						// 截取mac地址
						STRING_Left(&buff[bleIndex - 18], 17, bleHostProtocolCB.pair.macBuff);
						bleHostProtocolCB.pair.scanFlag = TRUE;
					}
					break;

				default:
					break;
			}
		}
		else if (-1 != STRING_Find(buff, (uint8 *)"TTM:CONNECT="))
		{
			if (-1 != STRING_Find(buff, bleHostProtocolCB.pair.macBuff))
			{
				macIndex = STRING_FindCh(buff, ',');
				switch (bleHostProtocolCB.pair.device)
				{
					case BLE_HELMET_DEVICE:

						if ('0' == buff[macIndex+1])
						{
							PARAM_SetHelmetConnectState(FALSE);
						}
						else if ('1' == buff[macIndex+1])
						{
							PARAM_SetHelmetConnectState(TRUE);
						}	
						break;

					case BLE_LOCK_DEVICE:

						if ('0' == buff[macIndex+1])
						{
							PARAM_SetLockConnectState(FALSE);
						}
						else if ('1' == buff[macIndex+1])
						{
							PARAM_SetLockConnectState(TRUE);
						}	
						break;

					default:
						break;
				}
			}
		}
		else if ((-1 != STRING_Find(buff, (uint8 *)"TTM:DISCONNECTED=")) 
			    &&(-1 != STRING_Find(buff, bleHostProtocolCB.pair.macBuff)))
		{
			switch (bleHostProtocolCB.pair.device)
			{
				case BLE_HELMET_DEVICE:

					PARAM_SetHelmetConnectState(FALSE);
					break;

				case BLE_LOCK_DEVICE:

					PARAM_SetLockConnectState(FALSE);
					break;

				default:
					break;
			}
		}
	}
	else
	{
		// 从设备连接状态
		if (-1 != STRING_Find(buff, (uint8 *)"TTM:CONNECT="))
		{
			if (-1 != STRING_Find(buff, (uint8 *)PARAM_GetHelmetMacBuff()))
			{
				macIndex = STRING_FindCh(buff, ',');
				if ('0' == buff[macIndex+1])
				{
					PARAM_SetHelmetConnectState(FALSE);
				}
				else if ('1' == buff[macIndex+1])
				{
					PARAM_SetHelmetConnectState(TRUE);
					TIMER_KillTask(TIMER_ID_BLE_HELMET_PAIR);

				}	
			}
			else if (-1 != STRING_Find(buff, (uint8 *)PARAM_GetLockMacBuff()))
			{
				macIndex = STRING_FindCh(buff, ',');
				if ('0' == buff[macIndex+1])
				{
					PARAM_SetLockConnectState(FALSE);
				}
				else if ('1' == buff[macIndex+1])
				{
					PARAM_SetLockConnectState(TRUE);
					TIMER_KillTask(TIMER_ID_BLE_LOCK_PAIR);

					// 获取电子锁key
					TIMER_AddTask(TIMER_ID_BLE_LOCK_PAIR,
							1000,
							BLE_HOST_PROTOCOL_SendCmdGetKey,
							TRUE,
							BLE_OUTFIT_PAIR_COUNT,
							ACTION_MODE_DO_AT_ONCE);	
				}	
			}
			
			if (-1 != STRING_Find(buff, (uint8 *)"TTM:DISCONNECTED="))
			{
				// 头盔
				if (-1 != STRING_Find(buff, (uint8 *)PARAM_GetHelmetMacBuff()))
				{
					PARAM_SetCallerIgnoreState(FALSE);
					PARAM_SetHelmetConnectState(FALSE);
					
					TIMER_AddTask(TIMER_ID_BLE_HELMET_PAIR,
							BLE_OUTFIT_CONNECT_TIME,
							BLE_HOST_UART_CALLBALL_OutfitConnect,
							TRUE,
							BLE_OUTFIT_PAIR_COUNT,
							ACTION_MODE_DO_AT_ONCE);	
				}
				// 无线按键
				else if (-1 != STRING_Find(buff, (uint8 *)PARAM_GetLockMacBuff()))
				{
					PARAM_SetLockConnectState(FALSE);

					TIMER_AddTask(TIMER_ID_BLE_LOCK_PAIR,
							BLE_OUTFIT_CONNECT_TIME,
							BLE_HOST_UART_CALLBALL_OutfitConnect,
							FALSE,
							BLE_OUTFIT_PAIR_COUNT,
							ACTION_MODE_DO_AT_ONCE);	
				}
			}
		}
		// 从设备连接状态
		else if (-1 != STRING_Find(buff, (uint8 *)"TTM:DISCONNECTED="))
		{
			// 头盔
			if (-1 != STRING_Find(buff, (uint8 *)PARAM_GetHelmetMacBuff()))
			{
				PARAM_SetCallerIgnoreState(FALSE);
				PARAM_SetHelmetConnectState(FALSE);
				
				TIMER_AddTask(TIMER_ID_BLE_HELMET_PAIR,
						BLE_OUTFIT_CONNECT_TIME,
						BLE_HOST_UART_CALLBALL_OutfitConnect,
						TRUE,
						BLE_OUTFIT_PAIR_COUNT,
						ACTION_MODE_DO_AT_ONCE);	
			}
			// 无线按键
			else if (-1 != STRING_Find(buff, (uint8 *)PARAM_GetLockMacBuff()))
			{
				PARAM_SetLockConnectState(FALSE);

				TIMER_AddTask(TIMER_ID_BLE_LOCK_PAIR,
						BLE_OUTFIT_CONNECT_TIME,
						BLE_HOST_UART_CALLBALL_OutfitConnect,
						FALSE,
						BLE_OUTFIT_PAIR_COUNT,
						ACTION_MODE_DO_AT_ONCE);	
			}
		}
	}
}

// 切换发送命令工作步骤
void BLE_SendCmdSwitchStep(uint32 param)
{
	bleHostProtocolCB.sendCmd.sendCmdStep = (BLE_SEND_CMD_STEP)param;
}

// 模块发送命令和接受命令处理
void BLE_SendAndRevCmdHandle(void)
{
	uint16 head = bleHostProtocolCB.ttmTx.head;
	uint16 end = bleHostProtocolCB.ttmTx.end;
	uint16 index = bleHostProtocolCB.ttmTx.index;

	switch (bleHostProtocolCB.sendCmd.sendCmdStep)
	{						
		// 空闲状态
		case BLE_SEND_CMD_IDE:
			
			// 队列为空，不处理
			if(head == end)
			{
				return;
			}
			else
			{
				BLE_SendCmdSwitchStep(BLE_SEND_CMD_SEND);
			}
			break;
			
		// 发送命令 发送
		case BLE_SEND_CMD_SEND:
			
			// 设置等待的标志位位等待状态
			bleHostProtocolCB.sendCmd.Flag = FALSE;
			
			// 当前命令帧未发送完时，持续发送
			while(index < bleHostProtocolCB.ttmTx.cmdQueue[head].length)
			{
				// 一直填充发送
				BLE_HOST_UART_BC_SendData(bleHostProtocolCB.ttmTx.cmdQueue[head].buff[bleHostProtocolCB.ttmTx.index++]);
				
				index = bleHostProtocolCB.ttmTx.index;
			}

			if (BLE_OUTFIT_HELMET_ID == bleHostProtocolCB.ttmTx.cmdQueue[head].deviceID ||
				BLE_OUTFIT_LOCK_ID == bleHostProtocolCB.ttmTx.cmdQueue[head].deviceID)
			{
				// 创建定时器任务，等待指定时间进入异常处理
				TIMER_AddTask(TIMER_ID_BLE_SEND_CMD,
								1000,
								BLE_SendCmdSwitchStep,
								BLE_SEND_CMD_TIMOUT,
								1,
								ACTION_MODE_DO_AT_ONCE);

				// 切换去等结果
				BLE_SendCmdSwitchStep(BLE_SEND_CMD_DELAY);
			}
			else
			{
				// 创建定时器任务，等待指定时间进入异常处理
				TIMER_AddTask(TIMER_ID_BLE_SEND_CMD,
								BLE_SEND_CMD_TIMEOUT,
								BLE_SendCmdSwitchStep,
								BLE_SEND_CMD_TIMOUT,
								1,
								ACTION_MODE_DO_AT_ONCE);
				// 切换去等结果
				BLE_SendCmdSwitchStep(BLE_SEND_CMD_WAIT);
			}
			break;
			
		// 发送命令 等待结果
		case BLE_SEND_CMD_WAIT:

			if(bleHostProtocolCB.sendCmd.Flag)
			{
				// 及时注销定时器
				TIMER_KillTask(TIMER_ID_BLE_SEND_CMD);
				
				// 设置发送命令标志位
				bleHostProtocolCB.sendCmd.Flag = FALSE;
				
				// 跳转到空闲
				BLE_SendCmdSwitchStep(BLE_SEND_CMD_IDE);

				// 当前命令帧发送完时，删除之
				bleHostProtocolCB.ttmTx.cmdQueue[head].length = 0;
				bleHostProtocolCB.ttmTx.head ++;
				bleHostProtocolCB.ttmTx.head %= BLE_HOST_PROTOCOL_TX_QUEUE_SIZE;
				bleHostProtocolCB.ttmTx.index = 0;
			}
			
		 	break;
		
		// 发送命令 等待超时
		case BLE_SEND_CMD_TIMOUT:
			
			// 跳转到空闲
			BLE_SendCmdSwitchStep(BLE_SEND_CMD_IDE);

			// 设置发送命令标志位
			bleHostProtocolCB.sendCmd.Flag = FALSE;
			
			// 当前命令帧发送完时，删除之
			bleHostProtocolCB.ttmTx.cmdQueue[head].length = 0;
			bleHostProtocolCB.ttmTx.head ++;
			bleHostProtocolCB.ttmTx.head %= BLE_HOST_PROTOCOL_TX_QUEUE_SIZE;
			bleHostProtocolCB.ttmTx.index = 0;
			break;

		// 延时等待
		case BLE_SEND_CMD_DELAY:
			break;
			
		default: 
			break;
	}
}


// 状态迁移
void BLE_PairEnterState(uint32 state)
{
	// 让当前的状态成为历史
	bleHostProtocolCB.pair.preState = bleHostProtocolCB.pair.state;

	// 设置新的状态
	bleHostProtocolCB.pair.state = (BLE_PAIR_STATE)state;	

	switch (state)
	{
		// 初始化状态
		case BLE_PAIR_INIT:
			
			// 头盔设备固定死
			//PARAM_SetHelmetPairState(TRUE);
			//sprintf((char*)PARAM_GetHelmetMacBuff(), "%s", (char*)"16:F0:0E:EC:F6:3F");
			
			// 电子锁设备固定死
			PARAM_SetLockPairState(TRUE);
			sprintf((char*)PARAM_GetLockMacBuff(), "%s", (char*)"FD:D9:00:FB:69:09");
			//sprintf((char*)PARAM_GetLockMacBuff(), "%s", (char*)"F3:52:6E:C8:E9:3B");
			
			// 上电断开头盔设备
			BLE_HOST_UART_SendCmdDisconnectHelmet(TRUE);

			// 上电断开无线按键设备
			BLE_HOST_UART_SendCmdDisconnectKey(TRUE);
			
			// 上电查询配对设备
			TIMER_AddTask(TIMER_ID_BLE_HELMET_PAIR,
							BLE_OUTFIT_CONNECT_TIME,
							BLE_HOST_UART_CALLBALL_OutfitConnect,
							TRUE,
							BLE_OUTFIT_PAIR_COUNT,
							ACTION_MODE_DO_AT_ONCE);

			TIMER_AddTask(TIMER_ID_BLE_LOCK_PAIR,
							BLE_OUTFIT_CONNECT_TIME,
							BLE_HOST_UART_CALLBALL_OutfitConnect,
							FALSE,
							BLE_OUTFIT_PAIR_COUNT,
							ACTION_MODE_DO_AT_ONCE);

			// 正常模式
			bleHostProtocolCB.pair.mode = BLE_NORMAL_MODE;
			bleHostProtocolCB.pair.scanFlag = FALSE;
			bleHostProtocolCB.pair.count = 0;
			
			// 跳转待机状态
			BLE_PairEnterState(BLE_PAIR_STANDBY);
			break;

		// 待机状态
		case BLE_PAIR_STANDBY:
			break;
			
		// 扫描状态
		case BLE_PAIR_SCAN:

			bleHostProtocolCB.pair.scanFlag = FALSE;
				
			// 发送扫描命令
			BLE_HOST_UART_SendScanBleDevice(BLE_OUTFIT_PAIR_SCAN_TIME);

			// 扫描完成进入连接状态
			TIMER_AddTask(TIMER_ID_BLE_LOCK_PAIR,
							(BLE_OUTFIT_PAIR_SCAN_TIME+1)*1000,
							BLE_PairEnterState,
							BLE_PAIR_CONNECT,
							1,
							ACTION_MODE_DO_AT_ONCE);
			break;

		// 连接状态
		case BLE_PAIR_CONNECT:

			if (!bleHostProtocolCB.pair.scanFlag)
			{
				BLE_PairEnterState(BLE_PAIR_FAILURE);
				break;
			}
			
			// 发送连接命令
			BLE_HOST_UART_SendConnectBleDevice(bleHostProtocolCB.pair.device, bleHostProtocolCB.pair.macBuff);
			
			// 连接超时就跳转失败处理
			TIMER_AddTask(TIMER_ID_BLE_LOCK_PAIR,
							3000,
							BLE_PairEnterState,
							BLE_PAIR_FAILURE,
							1,
							ACTION_MODE_DO_AT_ONCE);
			break;

		// 配对成功状态
		case BLE_PAIR_SUCCEED:
			
			switch (bleHostProtocolCB.pair.device)
			{
				case BLE_HELMET_DEVICE:

					// 保存配对mac地址
					strcpy((char *)PARAM_GetHelmetMacBuff(), (char *)bleHostProtocolCB.pair.macBuff);
					PARAM_SetHelmetPairState(TRUE);
					break;

				case BLE_LOCK_DEVICE:

					// 保存配对mac地址
					strcpy((char *)PARAM_GetLockMacBuff(), (char *)bleHostProtocolCB.pair.macBuff);
					PARAM_SetLockPairState(TRUE);
					break;
					
				default:
					break;
			}

			NVM_SetDirtyFlag(TRUE);

			BLE_PairEnterState(BLE_PAIR_STOP);
			break;

		// 配对失败状态
		case BLE_PAIR_FAILURE:

			bleHostProtocolCB.pair.count++;
			if (BLE_OUTFIT_PAIR_COUNT <= bleHostProtocolCB.pair.count)
			{
				BLE_PairEnterState(BLE_PAIR_STOP);
			}
			else
			{
				BLE_PairEnterState(BLE_PAIR_SCAN);
			}
			break;

		// 配对停止状态
		case BLE_PAIR_STOP:

			TIMER_KillTask(TIMER_ID_BLE_HELMET_PAIR);
			TIMER_KillTask(TIMER_ID_BLE_LOCK_PAIR);
			
			bleHostProtocolCB.pair.count = 0;
			bleHostProtocolCB.pair.scanFlag = FALSE;
			bleHostProtocolCB.pair.mode = BLE_NORMAL_MODE;

			BLE_PairEnterState(BLE_PAIR_STANDBY);

			// 配对设备断开连接就继续发指令去重连
			TIMER_AddTask(TIMER_ID_BLE_HELMET_PAIR,
							BLE_OUTFIT_CONNECT_TIME,
							BLE_HOST_UART_CALLBALL_OutfitConnect,
							TRUE,
							BLE_OUTFIT_PAIR_COUNT,
							ACTION_MODE_DO_AT_ONCE);

			TIMER_AddTask(TIMER_ID_BLE_LOCK_PAIR,
							BLE_OUTFIT_CONNECT_TIME,
							BLE_HOST_UART_CALLBALL_OutfitConnect,
							FALSE,
							BLE_OUTFIT_PAIR_COUNT,
							ACTION_MODE_DO_AT_ONCE);
			break;

		default:
			break;
	}
}

// 状态机处理
void BLE_PairProcess(void)
{
	// 模块发送命令和接受命令处理
	BLE_SendAndRevCmdHandle();

	switch(bleHostProtocolCB.pair.state)
	{
		// 待机状态
		case BLE_PAIR_STANDBY:
			
			switch (bleHostProtocolCB.pair.mode)
			{
				// 正常模式
				case BLE_NORMAL_MODE:
					break;

				// 配对模式
				case BLE_PAIR_MODE:
					
					BLE_PairEnterState(BLE_PAIR_SCAN);
					break;

				default:
					break;
			}
			break;
			
		// 扫描状态
		case BLE_PAIR_SCAN:

			if (BLE_PAIR_MODE != bleHostProtocolCB.pair.mode)
			{
				BLE_PairEnterState(BLE_PAIR_STOP);
			}
			break;

		// 连接状态
		case BLE_PAIR_CONNECT:

			if (BLE_HELMET_DEVICE == bleHostProtocolCB.pair.device)
			{
				if (PARAM_GetHelmetConnectState())
				{
					BLE_PairEnterState(BLE_PAIR_SUCCEED);
				}
			}
			else
			{
				if (PARAM_GetLockConnectState())
				{
					BLE_PairEnterState(BLE_PAIR_SUCCEED);
				}
			}
			break;
			
		default:
			break;
	}
}

// 切换成配对模式
BOOL BLE_SwitchPairMode(BLE_PAIR_DEVICE device)
{
	if (BLE_PAIR_STANDBY != bleHostProtocolCB.pair.state)
	{
		return FALSE;
	}

	switch (device)
	{
		case BLE_HELMET_DEVICE:

			// 已连接状态不进行配对
			if (PARAM_GetHelmetConnectState())
			{
				return FALSE;
			}

			// 进入配对状态后取消之前的配对
			PARAM_SetHelmetPairState(FALSE);
			break;

		case BLE_LOCK_DEVICE:

			// 已连接状态不进行配对
			if (PARAM_GetLockConnectState())
			{
				return FALSE;
			}

			// 进入配对状态后取消之前的配对
			PARAM_SetLockPairState(FALSE);
			break;

		default:

			break;
	}
		
	bleHostProtocolCB.pair.count = 0;
	bleHostProtocolCB.pair.scanFlag = FALSE;
	bleHostProtocolCB.pair.device = device;
	bleHostProtocolCB.pair.mode = BLE_PAIR_MODE;

	TIMER_KillTask(TIMER_ID_BLE_HELMET_PAIR);
	TIMER_KillTask(TIMER_ID_BLE_LOCK_PAIR);
	
	return TRUE;
}

// 取消配对
void BLE_PairStop(void)
{
	BLE_PairEnterState(BLE_PAIR_STOP);
}


