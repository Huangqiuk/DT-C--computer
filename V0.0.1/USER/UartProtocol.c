#include "common.h"
#include "system.h"
#include "timer.h"
#include "delay.h"
#include "uartDrive.h"
#include "uartProtocol.h"
#include "iap.h"
#include "segment.h"
#include "param.h"
#include "state.h"
#include "pwmLed.h"
#include "Source_Relay.h"
#include "meterControl.h"

/******************************************************************************
* 【内部接口声明】
******************************************************************************/

// 数据结构初始化
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB* pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length);

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

// UART总线超时错误处理
void UART_PROTOCOL_CALLBACK_UartBusError(uint32 param);

//=========================================================================
void UART_PROTOCOL_SendCmdTwoBytesDataAck(uint8* pBuf, uint8 length1, uint8 length2);
void UART_PROTOCOL_SendCmdOneByteDataAck(uint8* pBuf, uint8 length1);
void UART_PROTOCOL_SendCmdNoDataAck(uint8* pBuf);
void UART_PROTOCOL_SendCmdAppVersion(uint8* pBuf);

// 版本发送命令
void UART_PROTOCOL_SendCmdVersion(uint8 versionCmd, uint8* pBuf);

// 读取FLASH回复
void UART_PROTOCOL_SendCmdReadFlashRespond(uint32 length, uint8* pBuf);

// 上电发送匹配命令
void UART_PROTOCOL_SendSnMatching(uint8 length, uint8 *Sn);

// 全局变量定义
UART_PROTOCOL_CB uartProtocolCB;


// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

uint32 uartProtocolOutTimeDelay[6] = {30*60*1000,			// 空命令
									30*60*1000,				// LOGO
									30*60*1000,				// 提示
									1000,					// 正常骑行
									1000,					// 异常骑行提示
									30*60*1000,};// 升级

// use the crc8
uint8 UART_GetCRCValue(uint8 * ptr, uint16 len)
{
	uint8 crc;
	uint8 i;
	
	crc = 0;

	while (len--)
	{
		crc ^= *ptr++;
		
		for (i = 0; i < 8; i++)
		{
			if (crc & 0x01)
			{
				crc = (crc >> 1) ^ 0x8C;
			}
			else
			{
				crc >>= 1;
			}
		}
	}
	
	return crc;
}



// 协议初始化
void UART_PROTOCOL_Init(void)
{
	// 协议层数据结构初始化
	UART_PROTOCOL_DataStructInit(&uartProtocolCB);

	// 向驱动层注册数据接收接口
	UART_DRIVE_RegisterDataSendService(UART_PROTOCOL_MacProcess);

	// 向驱动层注册数据发送接口
	UART_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);
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
	
    uint16_t checkSum = 0;
	uint16_t i = 0;
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
	if(UART_PROTOCOL_CMD_FRAME_LENGTH_MIN-1 > length)	// 减去"校验码"1个字节
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// 队尾命令帧已满，退出
	if(length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
	// 重设数据长度，需要减去10=(3同步头1命令字+1数据长度)
	pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] = (length - 5);
	

	//cc = UART_GetCRCValue(&pCmdFrame->buff[UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - UART_PROTOCOL_HEAD_BYTE));
	/* 异或取反计算校验码 */
    for(i = 0; i < pCmdFrame->length; i++)
	{
		checkSum ^= pCmdFrame->buff[i];
	}
	checkSum = ~checkSum;
	
	pCmdFrame->buff[length] = checkSum;
	pCmdFrame->length ++;

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

	pCB->tx.txBusy = bFALSE;
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
		return bFALSE;
	}

	// 临时缓冲区为空，不予添加
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	if(0 == pCmdFrame->length)
	{
		return bFALSE;
	}

	// 添加
	pCB->rx.end ++;
	pCB->rx.end %= UART_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区
	
	return bTRUE;
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
		if(UART_PROTOCOL_CMD_HEAD1 != currentData)
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
			
			// 首先判断命令帧最小长度，一个完整的命令字至少包括10个字节,因此不足8个字节的必定不完整
			if(pCmdFrame->length < UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 继续接收
				continue;
			}

			// 命令帧长度数值越界，说明当前命令帧错误，停止接收
			length = (uint16)pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] ;
			if(length > UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
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

			// 命令帧长度校验，在命令长度描述字的数值上，增加命令帧最小长度，即为命令帧实际长度
			if(pCmdFrame->length < (length + UART_PROTOCOL_CMD_FRAME_LENGTH_MIN))
			{
				// 长度要求不一致，说明未接收完毕，退出继续
				continue;
			}

			// 命令帧长度OK，则进行校验，失败时删除命令头
			if(!UART_CheckSUM_(pCmdFrame))
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
			length = pCmdFrame->length;
			// 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			UART_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}
/***************************************************************
 * 函 数 名：UART2_CheckSUM
 * 函数入参：UART2_RX_CMD_FRAME *pCmdFrame -> 串口接收命令帧结构体
 * 函数出参：None
 * 返 回 值：None
 * 功能描述：对传入的命令帧进行校验，返回校验结果
***************************************************************/
static bool UART_CheckSUM_(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint8_t checkSum = 0;
	uint8_t sumTemp;
	uint16_t i = 0;
	
    /* 参数合法性检验 */
	if(NULL == pCmdFrame)
	{
		return false;
	}

	/* 从设备地址开始，到校验码之前的一个字节，依次进行异或运算 */
	for(i = 0;  i < pCmdFrame->length - 1; i++)
	{
		checkSum ^= pCmdFrame->buff[i];
	}
	
	/* 异或和取反 */
	checkSum = ~checkSum ;

    /* 获取命令帧中校验码 */
	sumTemp = pCmdFrame->buff[pCmdFrame->length - 1];
	
	/* 判断计算得到的校验码与命令帧中的校验码是否相同 */
	if(sumTemp != checkSum)
	{
		return false;
	}
	
	return true;
}

// 对传入的命令帧进行校验，返回校验结果
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame)
{
	uint8 cc = 0;
	
	if(NULL == pCmdFrame)
	{
		return bFALSE;
	}

	// 从协议版本开始，到校验码之前的一个字节，依次进行CRC8运算
	//cc = UART_GetCRCValue(&pCmdFrame->buff[UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - (UART_PROTOCOL_HEAD_BYTE + UART_PROTOCOL_CHECK_BYTE)));
     
	// 判断计算得到的校验码与命令帧中的校验码是否相同
	if(pCmdFrame->buff[pCmdFrame->length-1] != cc)
	{
		return bFALSE;
	}
	
	return bTRUE;
	
}

// 上报命令执行结果
void UART_PROTOCOL_SendCmdResult(uint32 cmd, uint32 param)
{
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD1);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD2);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD3);

	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_PROTOCOL_VERSION);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
	UART_PROTOCOL_TxAddData(cmd);
	UART_PROTOCOL_TxAddData(0);
	UART_PROTOCOL_TxAddData(2);

	// 写入字节数
	UART_PROTOCOL_TxAddData(1);

	// 写入结果
	UART_PROTOCOL_TxAddData((uint8)param);
	
	UART_PROTOCOL_TxAddFrame();
}


// 上报命令应答
void UART_PROTOCOL_SendCmdAck(uint32 cmd)
{
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD1);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD2);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD3);
	UART_PROTOCOL_TxAddData(cmd);
	//长度
	UART_PROTOCOL_TxAddData(0);
	//目标板编号
	UART_PROTOCOL_TxAddData(BOARD_ARM);
	// 组装命令帧
	UART_PROTOCOL_TxAddFrame();
}

// 设备上报函数
void APP_ReportMsg(uint8_t cmd, uint8_t *msg, uint8_t length)
{		
	uint8 i = 0;
	
	//	命令头1
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD1);
	//	命令头2
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD2);
	//	命令头3
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD3);
	//	命令字
	UART_PROTOCOL_TxAddData(cmd);
	// 	长度，长度会在UART2_TxAddData函数里自动更新
	UART_PROTOCOL_TxAddData(0);
	//	增加发送目的单板编号
	UART_PROTOCOL_TxAddData(BOARD_ARM);
	
	//	写入数据
	for(i = 0; i < length; i++)
	{
		UART_PROTOCOL_TxAddData(msg[i]);
	}
	
	// 组装命令帧
	UART_PROTOCOL_TxAddFrame();
	
	
}


// 上报模块写入结果
void UART_PROTOCOL_SendCmdWriteResult(uint32 cmd,uint16 byteNum, uint32 exeResoult)
{
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD1);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD2);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD3);

	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_PROTOCOL_VERSION);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
	UART_PROTOCOL_TxAddData(cmd);
	UART_PROTOCOL_TxAddData(0);
	UART_PROTOCOL_TxAddData(2);

	// 写入字节数
	UART_PROTOCOL_TxAddData(byteNum/256);
	UART_PROTOCOL_TxAddData(byteNum%256);

	// 写入结果
	UART_PROTOCOL_TxAddData((uint8)exeResoult);
	
	UART_PROTOCOL_TxAddFrame();
}


// 询问固件版本
void Uart2_CmdAskVersionACK(void)
{	
	uint8_t aSendMsg[7];

	// APP固件版本
	aSendMsg[2] = (0);
	aSendMsg[1] = (0);
	aSendMsg[0] = (1);
	//// APP固件序号
	aSendMsg[3] = (1);
	//	boot固件版本
	aSendMsg[6] = (0);
	aSendMsg[5] = (0);
	aSendMsg[4] = (1);
			
	// 机器上报数据
	APP_ReportMsg(UART_CMD_CALL, aSendMsg, 7);
}

extern LOCAL_PARAM_CB 	localPrame; 
// 询问固件版本
void Uart2_CmdAskVerACK(void)
{	
	uint8_t aSendMsg[7];

	// APP固件版本
	aSendMsg[2] = ((localPrame.data.prame.AppVersion >> 16) & 0xff);
	aSendMsg[1] = ((localPrame.data.prame.AppVersion >> 8) & 0xff);
	aSendMsg[0] = ((localPrame.data.prame.AppVersion >> 0) & 0xff);
	//// APP固件序号
	aSendMsg[3] = (localPrame.data.prame.AppVersionNum);
	//	boot固件版本
	aSendMsg[6] = ((localPrame.data.prame.BootVersion >> 16) & 0xff);
	aSendMsg[5] = ((localPrame.data.prame.BootVersion >> 8) & 0xff);
	aSendMsg[4] = ((localPrame.data.prame.BootVersion >> 0) & 0xff);
			
	// 机器上报数据
	APP_ReportMsg(UART_CMD_CALL, aSendMsg, 7);
}




/*******************************************************************************
* Function Name  : MesureType_SwitchToGears (AVO_MEASURE_TYPE MesureType )
* Description    : 将通讯协议的测量类型转换为万用表的类型。(主要是为了新PUCS万用表和老的万用逻辑兼容)
* Input          : MesureType ：测量类型
* Output         : None
* Return         : AVOMETER_GEARS_CMD 类型值，若为0则表示失败
*******************************************************************************/
AVOMETER_GEARS_CMD  MesureType_SwitchToGears (AVO_MEASURE_TYPE MesureType )
{
	   AVOMETER_GEARS_CMD  AVOMETER_GEARS=0;
       switch(MesureType)
		{	
			//直流电压（V）					
			case MEASURE_DCV :					 				
			      AVOMETER_GEARS=AVOMETER_GEARS_DCV;
				  break;
			//交流电压（V）
			case MEASURE_ACV :
				  AVOMETER_GEARS=AVOMETER_GEARS_ACV;
				  break;
			//电阻（Ω）
			case MEASURE_OHM :
				  AVOMETER_GEARS=AVOMETER_GEARS_OHM;
				  break;					     
			//直流电流（mA）		 
			case MEASURE_DCMA:						 
				  AVOMETER_GEARS=AVOMETER_GEARS_DCMA;				
				  break;
			//交流电流（mA）		 
			case MEASURE_ACMA:						 
				  AVOMETER_GEARS=AVOMETER_GEARS_ACMA;
				  break;
			//直流电流（A）
			case MEASURE_DCA :
                  AVOMETER_GEARS=AVOMETER_GEARS_DCA;				
				  break;
			//交流电流（A）		
			case MEASURE_ACA :	
                  AVOMETER_GEARS=AVOMETER_GEARS_ACA;				
				  break;					
		}
        return AVOMETER_GEARS;
}

void SysRest(uint32 param)
{
	__set_FAULTMASK(1); // 关闭所有中断
    NVIC_SystemReset(); // 复位
}


//复位指令应答
void Uart2_CmdRstACK(void)
{
   
   TIMER_AddTask(	TIMER_ID_RESET,
					200,
					SysRest,
					0,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
   APP_ReportMsg(UART_CMD_RES, 0, 0); 	
}


// UART命令帧缓冲区处理
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB* pCB)
{
	UART_PROTOCOL_CMD cmd = UART_CMD_NULL;
	UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
	AVO_MEASURE_TYPE mesureType;
	AVOMETER_GEARS_CMD  AVO_GEARS=0;
	uint8 chl=0;
	uint8 res=0;
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
	if(UART_PROTOCOL_CMD_HEAD1 != pCmdFrame->buff[UART_PROTOCOL_CMD_HEAD1_INDEX])
	{
		// 删除命令帧
		pCB->rx.head ++;
		pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// 命令头合法，则提取命令
	cmd = (UART_PROTOCOL_CMD)pCmdFrame->buff[UART_PROTOCOL_CMD_INDEX];
	
	
	// 执行命令帧
	switch(cmd)
	{
		
		case UART_CMD_CALL:
			    Uart2_CmdAskVersionACK(); 
		        break;
		 case    UART_CMD_RES :
			    Uart2_CmdRstACK();   
				break;	
		// 万用表测量命令
		case UART_CMD_AVOMETER:	
                // 一开始就复位所以继电器	20220420		
			    RelayReset();
		        Delayms(20); 
			    mesureType =(AVO_MEASURE_TYPE)pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];
		        AVO_GEARS=MesureType_SwitchToGears(mesureType);
		        if(AVO_GEARS==0)
				{
				  break;
				}	
	            chl=pCmdFrame->buff[UART_PROTOCOL_CMD_DATA2_INDEX]; 		     	
		        switch(mesureType)
				{	
                    //直流电压（V）					
					case MEASURE_DCV :
						  if(chl<RELAY_VIN_MAX)
						  {
							 COM_CHL_ON(); 						
							 VIN_RelayCtrl(chl,ON); 
						  }							
					      break;
					//交流电压（V）
					case MEASURE_ACV :
						  if(chl<RELAY_VIN_MAX)
						  {
							 COM_CHL_ON(); 	
						     VIN_RelayCtrl(chl,ON);
						  }							  
					      break;
					//电阻（Ω）
					case MEASURE_OHM :
						  if(chl<RELAY_OHM_MAX)
						  {
						     COM_CHL_ON(); 	
						     OHM_RelayCtrl(chl,ON); 
						  }
						  break;	
                    //直流电流（mA）		 
					case MEASURE_DCMA:						 
						 COM_CHL_ON(); 				
						  break;
					//交流电流（mA）		 
					case MEASURE_ACMA:						 
						 COM_CHL_ON();
						  break;
					//直流电流（A）
					case MEASURE_DCA :
						 COM_CHL_ON();				
						  break;
					//交流电流（A）		
					case MEASURE_ACA :	
						 COM_CHL_ON(); 			
				          break;										
					default:
			              break;					
				}
				// 设置档位
				Avoment_StitchGear(AVO_GEARS);
				meterControlCB.dataDir=1;
				meterControlCB.resPara.canUse=0;
				meterControlCB.resPara.chl=chl;
				meterControlCB.resPara.mesureType=mesureType;
				TIMER_AddTask(TIMER_ID_REOPRT_INVALID_RESULT,
								4000,
								Avoment_ReportMeasureResultACK,
								9,
								1,
								ACTION_MODE_ADD_TO_QUEUE);
				printf(" 查询表头档位命令\r\n");					
				
			    break;				
		// 万用表复位命令
		case UART_CMD_AVORESET :
			    res=1;
			    RelayReset();
			    APP_ReportMsg(UART_CMD_AVORESET_ACK, &res, 1);
                break;
		
		// 广播复位命令
        case UART_CMD_RESET_SOURCE:	
			    RelayReset();
		        break;		
		default:
			    break;
	}	
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
	uartProtocolCB.txAtOnceRequest = (BOOL)param;
}

// 码表设置周期下发数据请求标志
void UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param)
{
	uartProtocolCB.txPeriodRequest = (BOOL)param;
}


//========================================================================



