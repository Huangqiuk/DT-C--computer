#include "common.h"
#include "timer.h"
#include "delay.h"
#include "uartDrive.h"
#include "uartProtocol3.h"
#include "uartProtocol.h"
#include "param.h"
#include "state.h"

/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void UART_PROTOCOL3_DataStructInit(UART_PROTOCOL3_CB* pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL3_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL3_RxFIFOProcess(UART_PROTOCOL3_CB* pCB);

// UART����֡����������
void UART_PROTOCOL3_CmdFrameProcess(UART_PROTOCOL3_CB* pCB);

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL3_CheckSUM(UART_PROTOCOL3_RX_CMD_FRAME* pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL3_ConfirmTempCmdFrameBuff(UART_PROTOCOL3_CB* pCB);

// ͨѶ��ʱ����-����
void UART_PROTOCOL3_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void UART_PROTOCOL3_StopRxTimeOutCheck(void);

// Э��㷢�ʹ������
void UART_PROTOCOL3_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL3_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// ������������·����������־
void UART_PROTOCOL3_CALLBACK_SetTxPeriodRequest(uint32 param);

// ���������ͨѶʱ����
void UART_PROTOCOL3_DataCommunicationSequenceProcess(void);

// ����ͨѶ��ʱ�ж�����
void UART_PROTOCOL3_StartTimeoutCheckTask(void);

// UART���߳�ʱ������
void UART_PROTOCOL3_CALLBACK_UartBusError(uint32 param);

//=========================================================================
// �����������ϱ�����
void UART_PROTOCOL3_OnReceiveReport(uint8* pBuff, uint32 len);

// �������ò������������
void UART_PROTOCOL3_SendConfigParam(void);

// �������в������������
void UART_PROTOCOL3_SendRuntimeParam(void);

// �ϱ�д���ò������
void UART_PROTOCOL3_ReportWriteParamResult(uint32 param);

// �ϱ����ò���
void UART_PROTOCOL3_ReportConfigureParam(uint32 param);

//==================================================================================
// �ٶ�ƽ���˲�����
void UART_PROTOCOL3_CALLBACK_SpeedFilterProcess(uint32 param);


// ȫ�ֱ�������
UART_PROTOCOL3_CB uartProtocolCB3;

UART_PROTOCOL3_PARAM_CB hmiDriveCB3;

// ��������������������������������������������������������������������������������������
// ����ͨѶ��ʱ�ж�����
void UART_PROTOCOL3_StartTimeoutCheckTask(void)
{
	// ���ͨѶ�쳣
	if(ERROR_TYPE_COMMUNICATION_TIME_OUT == PARAM_GetErrorCode())
	{
		PARAM_SetErrorCode(ERROR_TYPE_NO_ERROR);
	}

	// ����UARTͨѶ��ʱ��ʱ��
#if __SYSTEM_NO_TIME_OUT_ERROR__		// �з�����ʱ������
#else
	TIMER_AddTask(TIMER_ID_PROTOCOL_TXRX_TIME_OUT,
					PARAM_GetBusAliveTime(),
					UART_PROTOCOL3_CALLBACK_UartBusError,
					TRUE,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
#endif
}

// Э���ʼ��
void UART_PROTOCOL3_Init(void)
{
	// Э������ݽṹ��ʼ��
	UART_PROTOCOL3_DataStructInit(&uartProtocolCB3);

	// ��������ע�����ݽ��սӿ�
	UART_DRIVE_RegisterDataSendService(UART_PROTOCOL3_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	UART_PROTOCOL3_RegisterDataSendService(UART_DRIVE_AddTxArray);

	// ע������������ʱ��
	TIMER_AddTask(TIMER_ID_PROTOCOL_PARAM_TX,
					UART_PROTOCOL3_CMD_SEND_TIME,
					UART_PROTOCOL3_CALLBACK_SetTxPeriodRequest,
					TRUE,
					TIMER_LOOP_FOREVER,
					ACTION_MODE_ADD_TO_QUEUE);

	// ���ò�����������ڸ���ʱ��
	PARAM_SetParamCycleTime(UART_PROTOCOL3_CMD_SEND_TIME);

	// ����UART����ͨѶ��ʱ�ж�
	UART_PROTOCOL3_StartTimeoutCheckTask();

	// �����ٶ���ʾ�˲����ƶ�ʱ��
	TIMER_AddTask(TIMER_ID_PROTOCOL_SPEED_FILTER,
					SMOOTH_BASE_TIME,
					UART_PROTOCOL3_CALLBACK_SpeedFilterProcess,
					TRUE,
					TIMER_LOOP_FOREVER,
					ACTION_MODE_ADD_TO_QUEUE);
}

// �ٶ�ƽ���˲�����
void UART_PROTOCOL3_CALLBACK_SpeedFilterProcess(uint32 param)
{
	if (uartProtocolCB3.speedFilter.realSpeed >= uartProtocolCB3.speedFilter.proSpeed)
	{
			uartProtocolCB3.speedFilter.proSpeed += uartProtocolCB3.speedFilter.difSpeed;

			if (uartProtocolCB3.speedFilter.proSpeed >= uartProtocolCB3.speedFilter.realSpeed)
			{
				uartProtocolCB3.speedFilter.proSpeed = uartProtocolCB3.speedFilter.realSpeed;
			}
	}
	else
	{
		if (uartProtocolCB3.speedFilter.proSpeed >= uartProtocolCB3.speedFilter.difSpeed)
		{
			uartProtocolCB3.speedFilter.proSpeed -= uartProtocolCB3.speedFilter.difSpeed;
		}
		else
		{
			uartProtocolCB3.speedFilter.proSpeed = 0;
		}
	}

	PARAM_SetSpeed(uartProtocolCB3.speedFilter.proSpeed);

	// �ٶȲ�Ϊ0�����ö�ʱ�ػ�����
	if (PARAM_GetSpeed() != 0)
	{
		STATE_ResetAutoPowerOffControl();
	}
	
}

// UARTЭ�����̴���
void UART_PROTOCOL3_Process(void)
{
	// UART����FIFO����������
	UART_PROTOCOL3_RxFIFOProcess(&uartProtocolCB3);

	// UART���������������
	UART_PROTOCOL3_CmdFrameProcess(&uartProtocolCB3);
	
	// UARTЭ��㷢�ʹ������
	UART_PROTOCOL3_TxStateProcess();

	// ���������ͨѶʱ����
	UART_PROTOCOL3_DataCommunicationSequenceProcess();

	// ��JЭ����Ƕ��KM5SЭ��
	UART_PROTOCOL_Process();
}

// ��������֡�������������
void UART_PROTOCOL3_TxAddData(uint8 data)
{
	uint16 head = uartProtocolCB3.tx.head;
	uint16 end =  uartProtocolCB3.tx.end;
	UART_PROTOCOL3_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB3.tx.end];	

	// ���ͻ������������������
	if((end + 1) % UART_PROTOCOL3_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ��β����֡�������˳�
	if(pCmdFrame->length >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void UART_PROTOCOL3_TxAddFrame(void)
{
	uint8 checkSum = 0;
	uint16 i = 0;
	uint8 xorVal;
	uint16 head = uartProtocolCB3.tx.head;
	uint16 end  = uartProtocolCB3.tx.end;
	UART_PROTOCOL3_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB3.tx.end];
	uint16 length = pCmdFrame->length;	

	// ���ͻ������������������
	if((end + 1) % UART_PROTOCOL3_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ����֡���Ȳ��㣬������������ݣ��˳�
	if(UART_PROTOCOL3_CMD_FRAME_LENGTH_MIN-3 > length)	// ��ȥ"������ֵ�����У�顢������Ox0D"3���ֽ�
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// ��β����֡�������˳�
	if((length >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX) 
		|| (length+1 >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX) 
		|| (length+2 >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX))
	{
		return;
	}

	// ��Э��������ֵ�������У��Ϊ0x0D����ı��ֵ������У��
	for (xorVal = 0; xorVal <= 255; xorVal++)
	{
		pCmdFrame->buff[pCmdFrame->length++] = xorVal;
		length = pCmdFrame->length;
		
		for (i = 1; i < length; i++)
		{
			checkSum ^= pCmdFrame->buff[i];
		}

		// ���У��Ϊ0x0D����ı��ֵ������У��
		if (0x0D == checkSum)
		{
			pCmdFrame->length--;
		}
		else
		{
			break;
		}
	}

	// ���У��ֵ
	pCmdFrame->buff[pCmdFrame->length++] = checkSum;

	// ������ʶ
	pCmdFrame->buff[pCmdFrame->length++] = 0x0D;

	uartProtocolCB3.tx.end ++;
	uartProtocolCB3.tx.end %= UART_PROTOCOL3_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void UART_PROTOCOL3_DataStructInit(UART_PROTOCOL3_CB* pCB)
{
	uint16 i;
	
	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}

	pCB->tx.txBusy = FALSE;
	pCB->tx.index = 0;
	pCB->tx.head = 0;
	pCB->tx.end = 0;
	for(i = 0; i < UART_PROTOCOL3_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<UART_PROTOCOL3_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL3_MacProcess(uint16 standarID, uint8* pData, uint16 length)
{
	uint16 end = uartProtocolCB3.rxFIFO.end;
	uint16 head = uartProtocolCB3.rxFIFO.head;
	uint8 rxdata = 0x00;
	
	// ��������
	rxdata = *pData;

	// һ���������������������
	if((end + 1)%UART_PROTOCOL3_RX_FIFO_SIZE == head)
	{
		return;
	}
	// һ��������δ�������� 
	else
	{
		// �����յ������ݷŵ���ʱ��������
		uartProtocolCB3.rxFIFO.buff[end] = rxdata;
		uartProtocolCB3.rxFIFO.end ++;
		uartProtocolCB3.rxFIFO.end %= UART_PROTOCOL3_RX_FIFO_SIZE;
	}	

	// ����KM5SЭ�����
	UART_PROTOCOL_MacProcess(standarID, pData, length);
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL3_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{		
	uartProtocolCB3.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL3_ConfirmTempCmdFrameBuff(UART_PROTOCOL3_CB* pCB)
{
	UART_PROTOCOL3_RX_CMD_FRAME* pCmdFrame = NULL;
	
	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return FALSE;
	}

	// ��ʱ������Ϊ�գ��������
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	if(0 == pCmdFrame->length)
	{
		return FALSE;
	}

	// ���
	pCB->rx.end ++;
	pCB->rx.end %= UART_PROTOCOL3_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
	
	return TRUE;
}

// Э��㷢�ʹ������
void UART_PROTOCOL3_TxStateProcess(void)
{
	uint16 head = uartProtocolCB3.tx.head;
	uint16 end =  uartProtocolCB3.tx.end;
	uint16 length = uartProtocolCB3.tx.cmdQueue[head].length;
	uint8* pCmd = uartProtocolCB3.tx.cmdQueue[head].buff;
	uint16 localDeviceID = uartProtocolCB3.tx.cmdQueue[head].deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == uartProtocolCB3.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������		
	if (!(*uartProtocolCB3.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	uartProtocolCB3.tx.cmdQueue[head].length = 0;
	uartProtocolCB3.tx.head ++;
	uartProtocolCB3.tx.head %= UART_PROTOCOL3_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL3_RxFIFOProcess(UART_PROTOCOL3_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	UART_PROTOCOL3_RX_CMD_FRAME* pCmdFrame = NULL;
	uint16 length = 0;
	uint8 currentData = 0;
	
	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}
	
	// һ��������Ϊ�գ��˳�
	if(head == end)
	{
		return;
	}

	// ��ȡ��ʱ������ָ��
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	// ȡ����ǰҪ������ֽ�
	currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
	
	// ��ʱ����������Ϊ0ʱ���������ֽ�
	if(0 == pCmdFrame->length)
	{
		switch (currentData)
		{
			case UART_PROTOCOL3_CMD_HMI_CTRL_RUN:
				pCB->rxFIFO.curCmdFrameLength = UART_PROTOCOL3_RX_CMD_HMI_CTRL_RUN_LENGTH_MAX;
				break;

			default:
				// ����ͷ����ɾ����ǰ�ֽڲ��˳�
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= UART_PROTOCOL3_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				pCB->rxFIFO.curCmdFrameLength = 0;
				break;
		}

		// �������Ϊ0˵���Ƿ����˳�
		if (0 == pCB->rxFIFO.curCmdFrameLength)
		{
			return;
		}

		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if((pCB->rx.end + 1)%UART_PROTOCOL3_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// ���UARTͨѶ��ʱʱ������-2016.1.5����
#if UART_PROTOCOL3_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
						UART_PROTOCOL3_BUS_UNIDIRECTIONAL_TIME_OUT,
						UART_PROTOCOL3_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL3_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if(pCmdFrame->length >= UART_PROTOCOL3_RX_CMD_FRAME_LENGTH_MAX)
		{
#if UART_PROTOCOL3_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			UART_PROTOCOL3_StopRxTimeOutCheck();
#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0;	// 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= UART_PROTOCOL3_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// һֱȡ��ĩβ
		while(end != pCB->rxFIFO.currentProcessIndex)
		{
			// ȡ����ǰҪ������ֽ�
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			// ������δ������������գ���������ӵ���ʱ��������
			pCmdFrame->buff[pCmdFrame->length++] = currentData;
			pCB->rxFIFO.currentProcessIndex ++;
			pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL3_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������
			
			// �����ж�����֡��С���ȣ�һ�����������������ٰ���4���ֽ�: ����:��ʼ����������ֵ��������Ϣ�����У�飬��˲���4���ֽڵıض�������
			if(pCmdFrame->length < UART_PROTOCOL3_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡����У��
			length = pCmdFrame->length;
			if(length < pCB->rxFIFO.curCmdFrameLength)
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if(!UART_PROTOCOL3_CheckSUM(pCmdFrame))
			{
#if UART_PROTOCOL3_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				UART_PROTOCOL3_StopRxTimeOutCheck();
#endif
				
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= UART_PROTOCOL3_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if UART_PROTOCOL3_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			UART_PROTOCOL3_StopRxTimeOutCheck();
#endif
			
			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= UART_PROTOCOL3_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			UART_PROTOCOL3_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL3_CheckSUM(UART_PROTOCOL3_RX_CMD_FRAME* pCmdFrame)
{
	uint8 checkSum = 0;
	uint16 i = 0;
	
	if(NULL == pCmdFrame)
	{
		return FALSE;
	}

	// ������1��ʼ����У����֮ǰ��һ���ֽڣ����ν����������
	for(i=1; i<pCmdFrame->length-1; i++)
	{
		checkSum ^= pCmdFrame->buff[i];
	}
	
	// �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
	if(pCmdFrame->buff[pCmdFrame->length-1] != checkSum)
	{
		return FALSE;
	}
	
	return TRUE;
}

// UART����֡����������
void UART_PROTOCOL3_CmdFrameProcess(UART_PROTOCOL3_CB* pCB)
{
	UART_PROTOCOL3_CMD cmd = UART_PROTOCOL3_CMD_NULL;
	UART_PROTOCOL3_RX_CMD_FRAME* pCmdFrame = NULL;

	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}

	// ����֡������Ϊ�գ��˳�
	if(pCB->rx.head == pCB->rx.end)
	{
		return;
	}

	// ��ȡ��ǰҪ���������ָ֡��
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.head];
	
	// ��ʼ���������ַǷ����˳�
	if((UART_PROTOCOL3_CMD_HMI_CTRL_RUN != pCmdFrame->buff[UART_PROTOCOL3_CMD_START_SYMBOL_CMD_INDEX])
	&& (UART_PROTOCOL3_CMD_WRITE_CONTROL_PARAM != pCmdFrame->buff[UART_PROTOCOL3_CMD_START_SYMBOL_CMD_INDEX]) 
	&& (UART_PROTOCOL3_CMD_READ_CONTROL_PARAM != pCmdFrame->buff[UART_PROTOCOL3_CMD_START_SYMBOL_CMD_INDEX]))
	{
		// ɾ������֡
		pCB->rx.head ++;
		pCB->rx.head %= UART_PROTOCOL3_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (UART_PROTOCOL3_CMD)pCmdFrame->buff[UART_PROTOCOL3_CMD_START_SYMBOL_CMD_INDEX];
	
	// ִ������֡
	switch(cmd)
	{
		// ���������ִ��
		case UART_PROTOCOL3_CMD_NULL:
			break;

		// ��������״̬����������״̬
		// ע��:�����Ǳ���F��S��������ʼ�ջظ�F
		case UART_PROTOCOL3_CMD_HMI_CTRL_RUN:

			UART_PROTOCOL3_OnReceiveReport(&pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX], UART_PROTOCOL3_RX_PARAM_SIZE);

			// �������óɹ�
			uartProtocolCB3.paramSetOK = TRUE;

			// ���������־��
			PARAM_SetRecaculateRequest(TRUE);
			
			break;	
			
		default:
			break;
	}

	// ����UART����ͨѶ��ʱ�ж�
	UART_PROTOCOL3_StartTimeoutCheckTask();
	
	// ɾ������֡
	pCB->rx.head ++;
	pCB->rx.head %= UART_PROTOCOL3_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if UART_PROTOCOL3_RX_TIME_OUT_CHECK_ENABLE
void UART_PROTOCOL3_CALLBACK_RxTimeOut(uint32 param)
{
	UART_PROTOCOL3_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &uartProtocolCB3.rx.cmdQueue[uartProtocolCB3.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0;	// 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	uartProtocolCB3.rxFIFO.head ++;
	uartProtocolCB3.rxFIFO.head %= UART_PROTOCOL3_RX_FIFO_SIZE;
	uartProtocolCB3.rxFIFO.currentProcessIndex = uartProtocolCB3.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void UART_PROTOCOL3_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

// ������������·����������־
void UART_PROTOCOL3_SetTxAtOnceRequest(uint32 param)
{
	uartProtocolCB3.txAtOnceRequest = (BOOL)param;
}

// ������������·����������־
void UART_PROTOCOL3_CALLBACK_SetTxPeriodRequest(uint32 param)
{
	uartProtocolCB3.txPeriodRequest = (BOOL)param;
}

// ���������ͨѶʱ����
void UART_PROTOCOL3_DataCommunicationSequenceProcess(void)
{
	// ��������������������Է�������
	if ((uartProtocolCB3.txAtOnceRequest) || (uartProtocolCB3.txPeriodRequest))
	{
		// �������óɹ����������в�������
		if (uartProtocolCB3.paramSetOK)
		{
			UART_PROTOCOL3_SendRuntimeParam();
		}
		// ��������δ�ɹ�������ϵͳ���ò�������
		else
		{
			// ����ϵͳ�������ø�������
			UART_PROTOCOL3_SendConfigParam();
		}

		if (uartProtocolCB3.txAtOnceRequest)
		{
			// ���¸�λ�����·���ʱ��
			TIMER_ResetTimer(TIMER_ID_PROTOCOL_PARAM_TX);
		}

		uartProtocolCB3.txAtOnceRequest = FALSE;
		uartProtocolCB3.txPeriodRequest = FALSE;
	}
}

// 	UART���߳�ʱ������
void UART_PROTOCOL3_CALLBACK_UartBusError(uint32 param)
{
	PARAM_SetErrorCode(ERROR_TYPE_COMMUNICATION_TIME_OUT);
}

// �������ò������������
void UART_PROTOCOL3_SendConfigParam(void)
{
	uint8 i;
	uint8 wheelSizeCode = 0;

	// �����ʼ��->������
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HMI_CTRL_PARAM_SET);	

	// ������������1����: ��λ�����������ò���ʱ����Ҫ����Ϊ�⼸�������Ƕ�̬��
	hmiDriveCB3.set.param.assitLevel = 0x00;
	hmiDriveCB3.set.param.isPushModeOn = 0x00;
	hmiDriveCB3.set.param.reserveByte1Bit65 = 0x00;
	hmiDriveCB3.set.param.isLightOn = 0x00;

	// ������������2����: ���ٴ����Լ��־�����
	hmiDriveCB3.set.param.speedLimitCode = paramCB.nvm.param.common.speed.limitVal/10 - 20;
	if (paramCB.nvm.param.common.speed.limitVal > 510)
	{
		hmiDriveCB3.set.param.speedLimitCode = 0;
	}
	/*switch (PARAM_GetWheelSizeID())
	{		
		case PARAM_WHEEL_SIZE_16_INCH:
			hmiDriveCB3.set.param.wheelSizeCode = 0;
			break;
			
		case PARAM_WHEEL_SIZE_18_INCH:
			hmiDriveCB3.set.param.wheelSizeCode = 1;
			break;
			
		case PARAM_WHEEL_SIZE_20_INCH:
			hmiDriveCB3.set.param.wheelSizeCode = 2;
			break;
			
		case PARAM_WHEEL_SIZE_22_INCH:
			hmiDriveCB3.set.param.wheelSizeCode = 3;
			break;
			
		case PARAM_WHEEL_SIZE_24_INCH:
			hmiDriveCB3.set.param.wheelSizeCode = 4;
			break;
			
		case PARAM_WHEEL_SIZE_26_INCH:
			hmiDriveCB3.set.param.wheelSizeCode = 5;
			break;

		case PARAM_WHEEL_SIZE_700C:
			hmiDriveCB3.set.param.wheelSizeCode = 6;
			break;
			
		case PARAM_WHEEL_SIZE_28_INCH:
			hmiDriveCB3.set.param.wheelSizeCode = 7;
			break;

		default:
			hmiDriveCB3.set.param.wheelSizeCode = 5;
			break;
	}*/

	switch (PARAM_GetNewWheelSizeInch()) // ���־���λ0.1inch
	{
		case 160:
			wheelSizeCode = 0;
			break;

		case 180:
			wheelSizeCode = 1;
			break;

		case 200:
			wheelSizeCode = 2;
			break;

		case 220:
			wheelSizeCode = 3;
			break;

		case 240:
			wheelSizeCode = 4;
			break;

		case 260:
			wheelSizeCode = 5;
			break;

		case 275:
			wheelSizeCode = 6;
			break;

		case 280:
			wheelSizeCode = 7;
			break;

		case 290:
			wheelSizeCode = 7;
			break;

		// �Ƿ�ֵ��Ĭ��Ϊ26��
		default:
			wheelSizeCode = 5;
			break;
	}
	hmiDriveCB3.set.param.wheelSizeCode = wheelSizeCode;

	// ������װ����
	for (i = 0; i < UART_PROTOCOL3_SET_PARAM_SIZE; i++)
	{
		UART_PROTOCOL3_TxAddData(hmiDriveCB3.set.buff[i]);
	}

	// ��Ӽ���ͣ������������
	UART_PROTOCOL3_TxAddFrame();
}

// ����ʵʱ���в�����������
void UART_PROTOCOL3_SendRuntimeParam(void)
{
	uint8 i;
	uint8 wheelSizeCode = 0;

	// �����ʼ��->������
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HMI_CTRL_RUN);

	// ������������1����: ��ƿ��أ��Ƴ���������λ
	hmiDriveCB3.running.param.isPushModeOn = PARAM_IsPushAssistOn();
	hmiDriveCB3.running.param.isLightOn = PARAM_GetLightSwitch();
	
	// 0��ʱ����0x0F�����ܳ���
	/*if (ASSIST_0 == PARAM_GetAssistLevel())
	{
		hmiDriveCB3.running.param.assitLevel = 0x0F;
	}
	else
	{
		// ���ı䵵λʱ�������·���ǰ��λ(��˱�־����Ϊ�ı䵵λ��ʱ��������٣����·���λ)
		if (paramCB.runtime.changeAssistFlag)
		{
			paramCB.runtime.changeAssistFlag = FALSE;
			hmiDriveCB3.running.param.assitLevel = PARAM_GetAssistLevel();
		}
		else
		{
			// ���ٻ�6KMʱ����0��
			if ((PARAM_GetSpeed() > PARAM_GetSpeedLimit()) || PARAM_IsPushAssistOn())
			{
				hmiDriveCB3.running.param.assitLevel = 0;
			}
			else
			{
				hmiDriveCB3.running.param.assitLevel = PARAM_GetAssistLevel();
			}
		}
	}*/
	
//	if (PARAM_IsPushAssistOn())
//	{
//		hmiDriveCB3.running.param.assitLevel = 0;
//	}
//	else
//	{
		hmiDriveCB3.running.param.assitLevel = PARAM_GetAssistLevel();

		// 0��ʱ����0x0F
		if (ASSIST_0 == PARAM_GetAssistLevel())
		{
			hmiDriveCB3.running.param.assitLevel = 0x0F;
		}
//	}

	// ������������2����: ���ٴ����Լ��־�����
	hmiDriveCB3.running.param.speedLimitCode = paramCB.nvm.param.common.speed.limitVal/10 - 20;
	if (paramCB.nvm.param.common.speed.limitVal > 510)
	{
		hmiDriveCB3.set.param.speedLimitCode = 0;
	}
	/*(switch (PARAM_GetWheelSizeID())
	{		
		case PARAM_WHEEL_SIZE_16_INCH:
			hmiDriveCB3.running.param.wheelSizeCode = 0;
			break;
			
		case PARAM_WHEEL_SIZE_18_INCH:
			hmiDriveCB3.running.param.wheelSizeCode = 1;
			break;
			
		case PARAM_WHEEL_SIZE_20_INCH:
			hmiDriveCB3.running.param.wheelSizeCode = 2;
			break;
			
		case PARAM_WHEEL_SIZE_22_INCH:
			hmiDriveCB3.running.param.wheelSizeCode = 3;
			break;
			
		case PARAM_WHEEL_SIZE_24_INCH:
			hmiDriveCB3.running.param.wheelSizeCode = 4;
			break;
			
		case PARAM_WHEEL_SIZE_26_INCH:
			hmiDriveCB3.running.param.wheelSizeCode = 5;
			break;

		case PARAM_WHEEL_SIZE_700C:
			hmiDriveCB3.running.param.wheelSizeCode = 6;
			break;
			
		case PARAM_WHEEL_SIZE_28_INCH:
			hmiDriveCB3.running.param.wheelSizeCode = 7;
			break;

		default:
			hmiDriveCB3.running.param.wheelSizeCode = 5;
			break;
	}*/

	switch (PARAM_GetNewWheelSizeInch()) // ���־���λ0.1inch
	{
		case 160:
			wheelSizeCode = 0;
			break;

		case 180:
			wheelSizeCode = 1;
			break;

		case 200:
			wheelSizeCode = 2;
			break;

		case 220:
			wheelSizeCode = 3;
			break;

		case 240:
			wheelSizeCode = 4;
			break;

		case 260:
			wheelSizeCode = 5;
			break;

		case 275:
			wheelSizeCode = 6;
			break;

		case 280:
			wheelSizeCode = 7;
			break;

		case 290:
			wheelSizeCode = 7;
			break;

		// �Ƿ�ֵ��Ĭ��Ϊ26��
		default:
			wheelSizeCode = 5;
			break;
	}
	hmiDriveCB3.running.param.wheelSizeCode = wheelSizeCode;

	// ������װ����
	for (i = 0; i < UART_PROTOCOL3_RUNNING_PARAM_SIZE; i++)
	{
		UART_PROTOCOL3_TxAddData(hmiDriveCB3.running.buff[i]);
	}

	// ��Ӽ���ͣ������������
	UART_PROTOCOL3_TxAddFrame();
}

// �������ϱ���������
void UART_PROTOCOL3_OnReceiveReport(uint8* pBuff, uint32 len)
{
	uint8 i;
	double fSpeedTemp;
	//PARAM_WHEEL_SIZE_ID wheelSizeID = PARAM_GetWheelSizeID();
	uint16 perimeter = 0;
	uint16 tempVal;

	CHECK_PARAM_SAME_RETURN(NULL, pBuff);	
	CHECK_PARAM_SAME_RETURN(0, len);
	//CHECK_PARAM_OVER_SAME_RETURN(wheelSizeID, PARAM_WHEEL_SIZE_MAX);

	// �ӻ���ȡ�����ݵ��ṹ��
	for (i = 0; (i < len) && (i < UART_PROTOCOL3_RX_PARAM_SIZE); i++)
	{
		hmiDriveCB3.rx.buff[i] = pBuff[i];
	}

	// ����������������ϱ������ݷ����� hmiDriveCB3.rx.buff ��

	// ��ȡ��������:�������ֵ���ѹ��0��ʾǷѹ��1��5��ʾ����ֵ��
	//if (hmiDriveCB3.rx.param.batteryVoltageLevel <= 5)
	//{
		// ע�⣬�˲����Ѿ���adcģ��ʵ��
		//PARAM_SetBatteryPercent(hmiDriveCB3.rx.param.batteryVoltageLevel*20);
	//}

	if (BATTERY_DATA_SRC_CONTROLLER == PARAM_GetBatteryDataSrc())
	{
		// JЭ��û�е�ѹ�ϱ����ֶΣ���Ϊ�˼�����Ϊ���ù̶���ѹ
		tempVal = PARAM_GetBatteryVoltageLevel();
		tempVal *= 1000;											// ��λת��: 1Vת��Ϊ1000mV	
		
		PARAM_SetBatteryVoltage(tempVal);
	}
	else if (BATTERY_DATA_SRC_BMS == PARAM_GetBatteryDataSrc())
	{
		// ����
		// ��ȡ��������:�������ֵ���ѹ��0��ʾǷѹ��1��5��ʾ����ֵ��
		PARAM_SetBatteryPercent(hmiDriveCB3.rx.param.batteryVoltageLevel*20);
	}

	// ��ȡ����ֵ
	PARAM_SetBatteryCurrent((uint16)((float)hmiDriveCB3.rx.param.batteryCurrent / 3.0f * 1000)); 	// ����Э���޸ģ���λת��: 1Aת��Ϊ1000mA 

	// ������4��Ϊ����תһȦʱ��
	tempVal = hmiDriveCB3.rx.param.oneCycleTimeH;
	tempVal <<= 8;
	tempVal |= hmiDriveCB3.rx.param.oneCycleTimeL;
	// ���յ��������ϱ�תһȦʱ��Ϊ3500ʱ������Ϊ�ٶ�Ϊ0
	if(tempVal >= 3341)
	{
		uartProtocolCB3.speedFilter.realSpeed = 0;
	}
	else
	{
		//perimeter = PARAM_GetPerimeter((uint8)wheelSizeID);
		perimeter = PARAM_GetNewperimeter();
		fSpeedTemp = (tempVal ? (3.6 / tempVal * perimeter) : 0);
		LIMIT_TO_MAX(fSpeedTemp, 99.9);
		
		uartProtocolCB3.speedFilter.realSpeed = (uint16)(fSpeedTemp*10);		// ת��Ϊ0.1KM/H
	}

	// ����һ�ַ����˲�2017/12/04
	if (uartProtocolCB3.speedFilter.realSpeed >= uartProtocolCB3.speedFilter.proSpeed)
	{
		uartProtocolCB3.speedFilter.difSpeed = (uartProtocolCB3.speedFilter.realSpeed - uartProtocolCB3.speedFilter.proSpeed) / 5.0f;
	}
	else
	{
		uartProtocolCB3.speedFilter.difSpeed = (uartProtocolCB3.speedFilter.proSpeed -  uartProtocolCB3.speedFilter.realSpeed) / 5.0f;

		// ʵ���ٶ�Ϊ0����ֵҲΪ0����ʾ�ٶ�ҲӦ��Ϊ0������1km/h���µ�ֵ
		if ((0 == uartProtocolCB3.speedFilter.difSpeed) && (0 == uartProtocolCB3.speedFilter.realSpeed))
		{
			uartProtocolCB3.speedFilter.proSpeed = 0;
		}
	}

	// �������ϱ��Ĵ������
	if ((hmiDriveCB3.rx.param.ucErrorCode == ERROR_TYPE_CURRENT_ERROR) || (hmiDriveCB3.rx.param.ucErrorCode      == ERROR_TYPE_THROTTLE_ERROR)
	|| (hmiDriveCB3.rx.param.ucErrorCode  == ERROR_TYPE_MOTOR_PHASE_ERROR ) || (hmiDriveCB3.rx.param.ucErrorCode == ERROR_TYPE_HALL_SENSOR_ERROR)
	|| (hmiDriveCB3.rx.param.ucErrorCode  == ERROR_TYPE_BRAKE_ERROR) || (hmiDriveCB3.rx.param.ucErrorCode        == ERROR_TYPE_TEMP_ERROR)
	|| (hmiDriveCB3.rx.param.ucErrorCode  == ERROR_TYPE_MOTOR_BLOCK_ERROR) || (hmiDriveCB3.rx.param.ucErrorCode  == ERROR_TYPE_OVER_VOLTAGE_ERROR))
	{
		PARAM_SetErrorCode((ERROR_TYPE_E)hmiDriveCB3.rx.param.ucErrorCode);	
	}
	else
	{
		PARAM_SetErrorCode(ERROR_TYPE_NO_ERROR);
	}
}

// �ϱ�д���ò������
void UART_PROTOCOL3_ReportWriteParamResult(uint32 param)
{
	// �����ʼ��->������
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_WRITE_CONTROL_PARAM_RESULT);
	
	// д����
	UART_PROTOCOL3_TxAddData(param);

	// ��Ӽ���ͣ������������
	UART_PROTOCOL3_TxAddFrame();
}

// �ϱ����ò���
void UART_PROTOCOL3_ReportConfigureParam(uint32 param)
{	
	// �����ʼ��->������
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_READ_CONTROL_PARAM_REPORT);

	// ��������
	UART_PROTOCOL3_TxAddData(PARAM_GetBrightness());

	// ����ʱ��
	UART_PROTOCOL3_TxAddData(PARAM_GetPowerOffTime());

	// ϵͳ��ѹ
	UART_PROTOCOL3_TxAddData(PARAM_GetBatteryVoltageLevel()/1000);

	// ������ʾ�仯ʱ��
	UART_PROTOCOL3_TxAddData(PARAM_GetBatteryCapVaryTime());

	// ���߹��ϳ�ʱʱ��
	UART_PROTOCOL3_TxAddData(PARAM_GetBusAliveTime());

	// ������λ��
	UART_PROTOCOL3_TxAddData(PARAM_GetMaxAssist());

	// ��������
	UART_PROTOCOL3_TxAddData(paramCB.nvm.param.common.speed.limitVal/10);

	// �־�����
	/*switch (PARAM_GetWheelSizeID())
	{
		case PARAM_WHEEL_SIZE_16_INCH:
			UART_PROTOCOL3_TxAddData(0);
			break;

		case PARAM_WHEEL_SIZE_18_INCH:
			UART_PROTOCOL3_TxAddData(1);
			break;

		case PARAM_WHEEL_SIZE_20_INCH:
			UART_PROTOCOL3_TxAddData(2);
			break;

		case PARAM_WHEEL_SIZE_22_INCH:
			UART_PROTOCOL3_TxAddData(3);
			break;

		case PARAM_WHEEL_SIZE_24_INCH:
			UART_PROTOCOL3_TxAddData(4);
			break;

		case PARAM_WHEEL_SIZE_26_INCH:
			UART_PROTOCOL3_TxAddData(5);
			break;

		case PARAM_WHEEL_SIZE_700C:
			UART_PROTOCOL3_TxAddData(6);
			break;

		case PARAM_WHEEL_SIZE_28_INCH:
			UART_PROTOCOL3_TxAddData(7);
			break;

		default:
			UART_PROTOCOL3_TxAddData(5);
			break;
	}*/
	// �־�����
	switch (PARAM_GetNewWheelSizeInch())
	{
		case 160:
			UART_PROTOCOL_TxAddData(0);
			break;

		case 180:
			UART_PROTOCOL_TxAddData(1);
			break;

		case 200:
			UART_PROTOCOL_TxAddData(2);
			break;

		case 220:
			UART_PROTOCOL_TxAddData(3);
			break;

		case 240:
			UART_PROTOCOL_TxAddData(4);
			break;

		case 260:
			UART_PROTOCOL_TxAddData(5);
			break;

		case 275:
			UART_PROTOCOL_TxAddData(6);
			break;

		case 280:
			UART_PROTOCOL_TxAddData(7);
			break;

		default:
			UART_PROTOCOL_TxAddData(0);
			break;
	}
	
	// �ٶ�ƽ���ȵȼ�
	UART_PROTOCOL3_TxAddData(PARAM_GetSpeedFilterLevel());

	// ��Ӽ���ͣ������������
	UART_PROTOCOL3_TxAddFrame();
}

