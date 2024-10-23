#include "common.h"
#include "timer.h"
#include "Param.h"
#include "state.h"
#include "spiflash.h"
#include "CanProtocolUpMD.h"
#include "spiflash.h"
#include "iap.h"
#include "DutInfo.h"

/******************************************************************************
* 【内部接口声明】
******************************************************************************/

// 数据结构初始化
void CAN_MD_PROTOCOL_DataStructInit(CAN_MD_PROTOCOL_CB* pCB);

// UART报文接收处理函数(注意根据具体模块修改)
// void CAN_MD_PROTOCOL_MacProcess(uint32 standarID, uint8* pData, uint16 length);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_MD_PROTOCOL_RxFIFOProcess(CAN_MD_PROTOCOL_CB* pCB);

// UART命令帧缓冲区处理
void CAN_MD_PROTOCOL_CmdFrameProcess(CAN_MD_PROTOCOL_CB* pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_MD_PROTOCOL_CheckSUM(uint8* buff, uint32 len, uint16 id);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_MD_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_MD_PROTOCOL_CB* pCB);

// 通讯超时处理-单向
void CAN_MD_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// 停止RX通讯超时检测任务
void CAN_MD_PROTOCOL_StopRxTimeOutCheck(void);

// 协议层发送处理过程
void CAN_MD_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void CAN_MD_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length));

// 启动通讯超时判断任务
void CAN_MD_PROTOCOL_StartTimeoutCheckTask(void);

// UART总线超时错误处理
void CAN_MD_PROTOCOL_CALLBACK_UartBusError(uint32 param);

// CRC32 计算方法
uint32 CRC32_Calculate( uint8 *pData, uint16 Length);

// 全局变量定义
CAN_MD_PROTOCOL_CB canMDProtocolCB;
uint32 Ccrc = 0;

// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// 启动通讯超时判断任务
void CAN_MD_PROTOCOL_StartTimeoutCheckTask(void)
{

}

// 协议初始化
void CAN_MD_PROTOCOL_Init(void)
{
	// 协议层数据结构初始化
	CAN_MD_PROTOCOL_DataStructInit(&canMDProtocolCB);

	// 向驱动层注册数据接收接口
//	CAN_DRIVE_RegisterDataSendService(CAN_MD_PROTOCOL_MacProcess);

	// 向驱动层注册数据发送接口
	CAN_MD_PROTOCOL_RegisterDataSendService(CAN_DRIVE_AddTxArray);
}

// UART协议层过程处理
void CAN_MD_PROTOCOL_Process(void)
{
	// CAN接收FIFO缓冲区处理
	CAN_MD_PROTOCOL_RxFIFOProcess(&canMDProtocolCB);

	// CAN接收命令缓冲区处理
	CAN_MD_PROTOCOL_CmdFrameProcess(&canMDProtocolCB);
	
	// CAN协议层发送处理过程
	CAN_MD_PROTOCOL_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void CAN_MD_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = canMDProtocolCB.tx.head;
	uint16 end =  canMDProtocolCB.tx.end;
	CAN_MD_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &canMDProtocolCB.tx.cmdQueue[canMDProtocolCB.tx.end];

	// 发送缓冲区已满，不予接收
	if((end + 1) % CAN_MD_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 队尾命令帧已满，退出
	if(pCmdFrame->length >= CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 数据添加到帧末尾，并更新帧长度
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void CAN_MD_PROTOCOL_TxAddFrame(void)
{
	uint16 checkSum = 0;
	uint8 i = 0, j = 0;
	uint16 head = canMDProtocolCB.tx.head;
	uint16 end  = canMDProtocolCB.tx.end;
	CAN_MD_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &canMDProtocolCB.tx.cmdQueue[canMDProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;
	uint8 *buff = canMDProtocolCB.tx.cmdQueue[end].buff;    
	uint8 cmdHead = buff[CAN_MD_PROTOCOL_CMD_HEAD];  // 帧头
	uint8 uBuff[CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];
	uint32 check = 0;
	
	// 发送缓冲区已满，不予接收
	if((end + 1) % CAN_MD_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 命令头正确，但数据长度不对(数据长度减去CRC的长度)
	if((CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX - CAN_MD_PROTOCOL_CMD_CHECK_BYTE_SIZE < length) || (CAN_MD_PROTOCOL_CMD_FRAME_LENGTH_MIN - CAN_MD_PROTOCOL_CMD_CHECK_BYTE_SIZE > length))
	{
	    // 把帧数据索引放在最开始位置
		canMDProtocolCB.tx.cmdQueue[end].length = 0;
        
		buff[CAN_MD_PROTOCOL_CMD_HEAD_INDEX] = CAN_MD_PROTOCOL_CMD_NONHEAD; // 避免数据没更新时，再次进入
		
		return;
	}

	// 执行到此，命令头正确，数据长度正确,发送缓冲区未满
	// 长度扶正
	canMDProtocolCB.tx.cmdQueue[end].buff[CAN_MD_PROTOCOL_CMD_LENGTH_INDEX] = length + CAN_MD_PROTOCOL_CMD_CHECK_BYTE_SIZE + CAN_MD_PROTOCOL_CMD_TAIL_BYTE - CAN_MD_PROTOCOL_CMD_FRAME_LENGTH_MIN;
	
	// CAN_ID 插入到帧头和帧模式
	for (i = 0, j = 0;j < length;i++,j++)
	{
		if (i == 2)
		{
			uBuff[i++] = 0x07;
			uBuff[i++] = (0x53 & 0xFF) ;
//			uBuff[i++] = 0;
//			uBuff[i++] = 0 ;			
		}
		uBuff[i] = buff[j];
	}
	
	// 填充校验码
	check = CRC32_Calculate(uBuff, length+2);

	buff[canMDProtocolCB.tx.cmdQueue[end].length++] = (check >> 24) & 0xFF;
	buff[canMDProtocolCB.tx.cmdQueue[end].length++] = (check >> 16) & 0xFF;
	buff[canMDProtocolCB.tx.cmdQueue[end].length++] = (check >> 8) & 0xFF;
	buff[canMDProtocolCB.tx.cmdQueue[end].length++] = (check & 0xFF);
	
	// 填充帧尾
	buff[canMDProtocolCB.tx.cmdQueue[end].length] = CAN_MD_PROTOCOL_CMD_TAIL;
	canMDProtocolCB.tx.cmdQueue[end].length++;

	// 发送环形队列更新位置
	canMDProtocolCB.tx.end ++;
	canMDProtocolCB.tx.end %= CAN_MD_PROTOCOL_TX_QUEUE_SIZE;
	canMDProtocolCB.tx.cmdQueue[canMDProtocolCB.tx.end].length = 0;	//2015.12.2修改
}

// 数据结构初始化
void CAN_MD_PROTOCOL_DataStructInit(CAN_MD_PROTOCOL_CB* pCB)
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
	for(i = 0; i < CAN_MD_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<CAN_MD_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART报文接收处理函数(注意根据具体模块修改)
void CAN_MD_PROTOCOL_MacProcess(uint32 standarID, uint8* pData, uint16 length)
{
	uint16 end = canMDProtocolCB.rxFIFO.end;
	uint16 head = canMDProtocolCB.rxFIFO.head;
	uint16 i = 0;
	uint8 *rxdata = pData;
    uint16 destinationID = 0x000;

	// 取出目标节点
	destinationID = standarID & 0x07FF;

	canMDProtocolCB.rxFIFO.deviceID = destinationID;
	
	//■■环形列队，入队■■
	// 一级缓冲区已满，不予接收
	if ((end + 1) % CAN_MD_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	
	for (i = 0; i < length; i++)
	{
		// 单个字节读取，并放入FIFO中
		canMDProtocolCB.rxFIFO.buff[canMDProtocolCB.rxFIFO.end] = *rxdata++;
		
		canMDProtocolCB.rxFIFO.end++;
		
		// 一级缓冲区已满，不予接收
		if ((canMDProtocolCB.rxFIFO.end + 1) % CAN_MD_PROTOCOL_RX_FIFO_SIZE == head)
		{
			break;
		}
		
		canMDProtocolCB.rxFIFO.end %= CAN_MD_PROTOCOL_RX_FIFO_SIZE;
	}
}

// UART协议层向驱动层注册数据发送接口
void CAN_MD_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length))
{		
	canMDProtocolCB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_MD_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_MD_PROTOCOL_CB* pCB)
{
	CAN_MD_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
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
	pCB->rx.end %= CAN_MD_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区
	
	return TRUE;
}

// 协议层发送处理过程
void CAN_MD_PROTOCOL_TxStateProcess(void)
{
	uint16 head = canMDProtocolCB.tx.head;
	uint16 end =  canMDProtocolCB.tx.end;
	uint16 length = canMDProtocolCB.tx.cmdQueue[head].length;
	uint8* pCmd = canMDProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = canMDProtocolCB.tx.cmdQueue[head].deviceID;

	// 发送缓冲区为空，说明无数据
	if (head == end)
	{
		return;
	}

	// 发送函数没有注册直接返回
	if (NULL == canMDProtocolCB.sendDataThrowService)
	{
		return;
	}

	// 协议层有数据需要发送到驱动层		
	if (!(*canMDProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// 发送环形队列更新位置
	canMDProtocolCB.tx.cmdQueue[head].length = 0;
	canMDProtocolCB.tx.head ++;
	canMDProtocolCB.tx.head %= CAN_MD_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_MD_PROTOCOL_RxFIFOProcess(CAN_MD_PROTOCOL_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	CAN_MD_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
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
		if(CAN_MD_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= CAN_MD_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// 命令头正确，但无临时缓冲区可用，退出
		if((pCB->rx.end + 1)%CAN_MD_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// 添加UART通讯超时时间设置-2016.1.5增加
#if CAN_MD_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_CAN_COMMUNICATION_TIME_OUT,
						CAN_MD_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
						CAN_MD_PROTOCOL_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= CAN_MD_PROTOCOL_RX_FIFO_SIZE;
	}
	// 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
	else
	{
		// 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
		if(pCmdFrame->length >= CAN_MD_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if CAN_MD_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			CAN_MD_PROTOCOL_StopRxTimeOutCheck();
#endif

			// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
			pCmdFrame->length = 0;	// 2016.1.5增加
			// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= CAN_MD_PROTOCOL_RX_FIFO_SIZE;
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
			pCB->rxFIFO.currentProcessIndex %= CAN_MD_PROTOCOL_RX_FIFO_SIZE;

			// ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■
			
			// 首先判断命令帧最小长度，一个完整的命令字至少包括8个字节: 命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，因此不足8个字节的必定不完整
			if(pCmdFrame->length < CAN_MD_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 继续接收
				continue;
			}

			// 命令帧长度数值越界，说明当前命令帧错误，停止接收
			if(pCmdFrame->buff[CAN_MD_PROTOCOL_CMD_LENGTH_INDEX] > CAN_MD_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX-CAN_MD_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
#if CAN_MD_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				CAN_MD_PROTOCOL_StopRxTimeOutCheck();
#endif
			
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= CAN_MD_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// 命令帧长度校验，在命令长度描述字的数值上，增加命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，即为命令帧实际长度
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[CAN_MD_PROTOCOL_CMD_LENGTH_INDEX] + CAN_MD_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 长度要求不一致，说明未接收完毕，退出继续
				continue;
			}

			// 命令帧长度OK，则进行校验，失败时删除命令头
			if(!CAN_MD_PROTOCOL_CheckSUM(pCmdFrame->buff, pCmdFrame->length, pCB->rxFIFO.deviceID))
			{
#if CAN_MD_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				CAN_MD_PROTOCOL_StopRxTimeOutCheck();
#endif
				
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= CAN_MD_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if CAN_MD_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			CAN_MD_PROTOCOL_StopRxTimeOutCheck();
#endif
			
			// 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= CAN_MD_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			CAN_MD_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_MD_PROTOCOL_CheckSUM(uint8* buff, uint32 len, uint16 id)
{
	uint32 cc = 0;
	uint16 i, j;
	uint8 uBuff[CAN_MD_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];
	
	if(NULL == buff)
	{
		return FALSE;
	}

	// CAN_ID 插入到帧头和帧模式之前，长度要去掉4个crc和1个帧尾共5个字节
	for (i = 0, j = 0; j < len-5; i++, j++)
	{
		if (i == 2)
		{
			uBuff[i++] = (id >> 8) & 0xFF;
			uBuff[i++] = (id & 0xFF);
		}
		
		uBuff[i] = buff[j];
	}
	
	cc = ((uint32)buff[len - 5] << 24) | ((uint32)buff[len - 4] << 16) | ((uint32)buff[len - 3] << 8) | buff[len - 2];

	// 判断计算得到的校验码与命令帧中的校验码是否相同，长度+2是增加can ID的2个字节
	if(CRC32_Calculate(uBuff, len-5+2) != cc)
	{
		return FALSE;
	}
	
	return TRUE;
}

// CAN命令帧缓冲区处理
void CAN_MD_PROTOCOL_CmdFrameProcess(CAN_MD_PROTOCOL_CB* pCB)
{
	CAN_MD_PROTOCOL_CMD cmd = CAN_MD_PROTOCOL_CMD_NULL;
	CAN_MD_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
    
    uint8 eraResual;
    uint32 temp = 0;
    uint16 length = 0;	
	BOOL 	writeRes;
	uint8 	i;
	uint32 	dataByteNum;
	uint32 	u32TempVal;
	uint32  temp1,temp2,temp3;
	static BOOL flag = TRUE;
    
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
	if(CAN_MD_PROTOCOL_CMD_HEAD != pCmdFrame->buff[CAN_MD_PROTOCOL_CMD_HEAD_INDEX])
	{
		// 删除命令帧
		pCB->rx.head ++;
		pCB->rx.head %= CAN_MD_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// 命令头合法，则提取命令
	cmd = (CAN_MD_PROTOCOL_CMD)((((uint16)pCmdFrame->buff[CAN_MD_PROTOCOL_CMD_CMD_H_INDEX]) << 8) | pCmdFrame->buff[CAN_MD_PROTOCOL_CMD_CMD_L_INDEX]);

	// 执行命令帧
	switch(cmd)
	{
		// 空命令，不予执行
		case CAN_MD_PROTOCOL_CMD_NULL:
			break;

        // 收到ECO申请
        case CAN_MD_PROTOCOL_CMD_UP_PROJECT_APPLY:
            if (!dut_info.dutPowerOnAllow)
            {
                // 延时50ms允许进入工程模式
                STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST);
                dut_info.dutPowerOnAllow = TRUE; // 不在接收eco请求
            }                        
            break;
            
        // 收到ECO准备就绪
        case CAN_MD_PROTOCOL_CMD_UP_PROJECT_READY:   
            if (dut_info.appUpFlag)
            {
                STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT); // 发送第一帧数据会擦除APP
            }                   
            break;

        // APP数据写入,正确应答
        case CAN_MD_PROTOCOL_CMD_UP_IAP_WRITE_RIGHT_RESULT: 
            if (dut_info.currentAppSize < dut_info.appSize)
            {
                STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE);
            }
            else
            {
                STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS);
            }            
            break;

        // APP数据写入,错误应答
        case CAN_MD_PROTOCOL_CMD_UP_IAP_WRITE_ERROR_RESULT:    
              dut_info.currentAppSize--;
            if (dut_info.currentAppSize < dut_info.appSize)
            {
                STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE);
            }
            else
            {
                STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS);
            }
            break;
            
        // APP数据完成
        case CAN_MD_PROTOCOL_CMD_UP_UPDATA_FINISH_RESULT: 
		    STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_ITEM_FINISH);
            break;
            
        default:
            break;
	}

	// 启动UART总线通讯超时判断
	CAN_MD_PROTOCOL_StartTimeoutCheckTask();
	
	// 删除命令帧
	pCB->rx.head ++;
	pCB->rx.head %= CAN_MD_PROTOCOL_RX_QUEUE_SIZE;
}

// RX通讯超时处理-单向
#if CAN_MD_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void CAN_MD_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	CAN_MD_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &canMDProtocolCB.rx.cmdQueue[canMDProtocolCB.rx.end];

	// 超时错误，将命令帧长度清零，即认为抛弃该命令帧
	pCmdFrame->length = 0;	// 2016.1.6增加
	// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
	canMDProtocolCB.rxFIFO.head ++;
	canMDProtocolCB.rxFIFO.head %= CAN_MD_PROTOCOL_RX_FIFO_SIZE;
	canMDProtocolCB.rxFIFO.currentProcessIndex = canMDProtocolCB.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void CAN_MD_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_CAN_COMMUNICATION_TIME_OUT);
}
#endif


// 	UART总线超时错误处理
void CAN_MD_PROTOCOL_CALLBACK_UartBusError(uint32 param)
{

}

// 发送命令带结果
void CAN_MD_PROTOCOL_SendCmdWithResult(uint16 cmdWord, uint8 result)
{
	// 添加命令头
	CAN_MD_PROTOCOL_TxAddData(CAN_MD_PROTOCOL_CMD_HEAD);
	CAN_MD_PROTOCOL_TxAddData(CAN_MD_PROTOCOL_CMD_HEAD2);
	CAN_MD_PROTOCOL_TxAddData(CAN_MD_PROTOCOL_MODE_ACK);
    
	// 添加数据长度
	CAN_MD_PROTOCOL_TxAddData(2);
	// 添加命令字
	CAN_MD_PROTOCOL_TxAddData(cmdWord>>8); 	
	CAN_MD_PROTOCOL_TxAddData(cmdWord&0x00FF);
	
	CAN_MD_PROTOCOL_TxAddData(result);
	
	CAN_MD_PROTOCOL_TxAddFrame();
}

// 发送命令无结果
void CAN_MD_PROTOCOL_SendCmdNoResult(uint16 cmdWord)
{
	// 添加命令头
	CAN_MD_PROTOCOL_TxAddData(CAN_MD_PROTOCOL_CMD_HEAD);
	CAN_MD_PROTOCOL_TxAddData(CAN_MD_PROTOCOL_CMD_HEAD2);
	CAN_MD_PROTOCOL_TxAddData(CAN_MD_PROTOCOL_MODE_ACK);
    
	// 添加数据长度
	CAN_MD_PROTOCOL_TxAddData(2);
	// 添加命令字
	CAN_MD_PROTOCOL_TxAddData(cmdWord>>8); 	
	CAN_MD_PROTOCOL_TxAddData(cmdWord&0x00FF);
		
	CAN_MD_PROTOCOL_TxAddFrame();
}

// 发送升级数据
void CAN_MD_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_MD_PROTOCOL_CMD cmd, uint32 flashAddr, uint32 addr)
{
	uint8 i = 0;
    
	switch (upDataType)
	{
//        case DUT_FILE_TYPE_HEX:							 // hex
//            dut_info.reconnectionRepeatContent[0] = 121; // 总长度
//            dut_info.reconnectionRepeatContent[1] = CAN_MD_PROTOCOL_CMD_HEAD;
//            dut_info.reconnectionRepeatContent[2] = cmd;
//            dut_info.reconnectionRepeatContent[3] = 0;											  // 数据长度
//            dut_info.reconnectionRepeatContent[4] = 0;											  // 添加是否加密
//            dut_info.reconnectionRepeatContent[5] = 0;											  // 添加密钥
//            SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[6], flashAddr + (addr), 116); // 工具读取 4+112 个字节
//            for (i = 0; i < 121; i++)															  // 添加数据
//            {
//                CAN_MD_PROTOCOL_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
//            }
//            break;

        case DUT_FILE_TYPE_BIN:							 // bin
            dut_info.reconnectionRepeatContent[0] = 139; // 总长度
            
            dut_info.reconnectionRepeatContent[1] = CAN_MD_PROTOCOL_CMD_HEAD;
            dut_info.reconnectionRepeatContent[2] = CAN_MD_PROTOCOL_CMD_HEAD2;
            dut_info.reconnectionRepeatContent[3] = CAN_MD_PROTOCOL_MODE_WRITE; 
            dut_info.reconnectionRepeatContent[4] = 0; // 总长度									  // 数据长度            
            dut_info.reconnectionRepeatContent[5] = (cmd>>8);
            dut_info.reconnectionRepeatContent[6] = (cmd&0x00FF);   

            dut_info.reconnectionRepeatContent[7] = 0;                // boot没对第一个数据进行处理
           
            dut_info.reconnectionRepeatContent[9] = ((dut_info.currentAppSize+1)>>8);				 
            dut_info.reconnectionRepeatContent[8] = ((dut_info.currentAppSize+1)&0x00FF);
            
            dut_info.reconnectionRepeatContent[10] = 0;					          // 128
            dut_info.reconnectionRepeatContent[11] = 0x80;						  // 
            
            SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[12], flashAddr + (addr), 128); // 工具读取128字节
            for (i = 0; i < 139; i++)															  // 添加数据
            {
                CAN_MD_PROTOCOL_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
            }
            break;
            
        default:
            break;
	}
	CAN_MD_PROTOCOL_TxAddFrame(); // 调整帧格式,修改长度和添加校验
}

const uint32 Crc32Table[256] =
{
	0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
	0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
	0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD, 0x4C11DB70, 0x48D0C6C7,
	0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
	0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3,
	0x709F7B7A, 0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
	0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58, 0xBAEA46EF,
	0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
	0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB,
	0xCEB42022, 0xCA753D95, 0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
	0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077, 0x30476DC0,
	0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
	0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4,
	0x0808D07D, 0x0CC9CDCA, 0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
	0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08,
	0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
	0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC,
	0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
	0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A, 0xE0B41DE7, 0xE4750050,
	0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
	0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34,
	0xDC3ABDED, 0xD8FBA05A, 0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
	0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB, 0x4F040D56, 0x4BC510E1,
	0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
	0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5,
	0x3F9B762C, 0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
	0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E, 0xF5EE4BB9,
	0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
	0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD,
	0xCDA1F604, 0xC960EBB3, 0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
	0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 0x9B3660C6, 0x9FF77D71,
	0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
	0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2,
	0x470CDD2B, 0x43CDC09C, 0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
	0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E,
	0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
	0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A,
	0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
	0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C, 0xE3A1CBC1, 0xE760D676,
	0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
	0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
	0x933EB0BB, 0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
	0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4 };

// CRC32 计算方法
uint32 CRC32_Calculate( uint8 *pData, uint16 Length)
{
	uint32 nReg;
	uint32 nTemp = 0;
	uint16 i, n;
	nReg = 0xFFFFFFFF;
	for ( n = 0; n < Length; n++ )
	{
		nReg ^= (uint32) pData[ n ];
		for ( i = 0; i < 4; i++ )
		{
			nTemp = Crc32Table[ ( uint8 )( ( nReg >> 24 ) & 0xFF ) ];
			nReg <<= 8;
			nReg ^= nTemp;
		}
	}
	return nReg;
}
