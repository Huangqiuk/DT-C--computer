#include "common.h"
#include "timer.h"
#include "delay.h"
#include "uartDrive.h"
#include "uartProtocol.h"
#include "uartProtocol1.h"
#include "param.h"
#include "state.h"
#include "iap.h"


/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void UART_PROTOCOL1_DataStructInit(UART_PROTOCOL1_CB* pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL1_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL1_RxFIFOProcess(UART_PROTOCOL1_CB* pCB);

// UART����֡����������
void UART_PROTOCOL1_CmdFrameProcess(UART_PROTOCOL1_CB* pCB);

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL1_CheckSUM(UART_PROTOCOL1_RX_CMD_FRAME* pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL1_ConfirmTempCmdFrameBuff(UART_PROTOCOL1_CB* pCB);

// ͨѶ��ʱ����-����
void UART_PROTOCOL1_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void UART_PROTOCOL1_StopRxTimeOutCheck(void);

// Э��㷢�ʹ������
void UART_PROTOCOL1_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL1_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// ������������·����������־
void UART_PROTOCOL1_CALLBACK_SetTxPeriodRequest(uint32 param);

// ���������ͨѶʱ����
void UART_PROTOCOL1_DataCommunicationSequenceProcess(void);

// ����ͨѶ��ʱ�ж�����
void UART_PROTOCOL1_StartTimeoutCheckTask(void);

// UART���߳�ʱ������
void UART_PROTOCOL1_CALLBACK_UartBusError(uint32 param);

//=========================================================================
// �����������ϱ�����
void UART_PROTOCOL1_OnReceiveReport(uint8* pBuff, uint32 len);

// �������в������������
void UART_PROTOCOL1_SendRuntimeParam(void);

// �ϱ�д���ò������
void UART_PROTOCOL1_ReportWriteParamResult(uint32 param);

// �ϱ����ò���
void UART_PROTOCOL1_ReportConfigureParam(uint32 param);

//==================================================================================
// �ٶ�ƽ���˲�����
void UART_PROTOCOL1_CALLBACK_SpeedFilterProcess(uint32 param);


// ȫ�ֱ�������
UART_PROTOCOL1_CB uartProtocolCB1;

UART_PROTOCOL1_PARAM_CB hmiDriveCB1;

// ��������������������������������������������������������������������������������������
// ����ͨѶ��ʱ�ж�����
void UART_PROTOCOL1_StartTimeoutCheckTask(void)
{
	// ���ͨѶ�쳣
	if(ERROR_TYPE_COMMUNICATION_TIME_OUT == PARAM_GetErrorCode())
	{
		PARAM_SetErrorCode(ERROR_TYPE_NO_ERROR);
	}

	// ����UARTͨѶ��ʱ��ʱ��
#if __SYSTEM_NO_TIME_OUT_ERROR__	// �з�����ʱ������
#else
	TIMER_AddTask(TIMER_ID_PROTOCOL_TXRX_TIME_OUT,
					PARAM_GetBusAliveTime(),
					UART_PROTOCOL1_CALLBACK_UartBusError,
					TRUE,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
#endif
}

// Э���ʼ��
void UART_PROTOCOL1_Init(void)
{
	// Э������ݽṹ��ʼ��
	UART_PROTOCOL1_DataStructInit(&uartProtocolCB1);

	// ��������ע�����ݽ��սӿ�
	UART_DRIVE_RegisterDataSendService(UART_PROTOCOL1_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	UART_PROTOCOL1_RegisterDataSendService(UART_DRIVE_AddTxArray);

	// ע������������ʱ��
	TIMER_AddTask(TIMER_ID_PROTOCOL_PARAM_TX,
					UART_PROTOCOL1_CMD_SEND_TIME,
					UART_PROTOCOL1_CALLBACK_SetTxPeriodRequest,
					TRUE,
					TIMER_LOOP_FOREVER,
					ACTION_MODE_ADD_TO_QUEUE);

	// ���ò�����������ڸ���ʱ��
	PARAM_SetParamCycleTime(UART_PROTOCOL1_CMD_SEND_TIME);

	// ����UART����ͨѶ��ʱ�ж�
	UART_PROTOCOL1_StartTimeoutCheckTask();

	// �����ٶ���ʾ�˲����ƶ�ʱ��
	TIMER_AddTask(TIMER_ID_PROTOCOL_SPEED_FILTER,
					SMOOTH_BASE_TIME1,
					UART_PROTOCOL1_CALLBACK_SpeedFilterProcess,
					TRUE,
					TIMER_LOOP_FOREVER,
					ACTION_MODE_ADD_TO_QUEUE);

}

// �ٶ�ƽ���˲�����
void UART_PROTOCOL1_CALLBACK_SpeedFilterProcess(uint32 param)
{
	if (uartProtocolCB1.speedFilter.realSpeed >= uartProtocolCB1.speedFilter.proSpeed)
	{
		uartProtocolCB1.speedFilter.proSpeed += uartProtocolCB1.speedFilter.difSpeed;

		if (uartProtocolCB1.speedFilter.proSpeed >= uartProtocolCB1.speedFilter.realSpeed)
		{
			uartProtocolCB1.speedFilter.proSpeed = uartProtocolCB1.speedFilter.realSpeed;
		}
	}
	else
	{
		if (uartProtocolCB1.speedFilter.proSpeed >= uartProtocolCB1.speedFilter.difSpeed)
		{
			uartProtocolCB1.speedFilter.proSpeed -= uartProtocolCB1.speedFilter.difSpeed;
		}
		else
		{
			uartProtocolCB1.speedFilter.proSpeed = 0;
		}
	}

	PARAM_SetSpeed(uartProtocolCB1.speedFilter.proSpeed);

	// �ٶȲ�Ϊ0�����ö�ʱ�ػ�����
	if (PARAM_GetSpeed() != 0)
	{
		STATE_ResetAutoPowerOffControl();
	}
}


// UARTЭ�����̴���
void UART_PROTOCOL1_Process(void)
{
	// UART����FIFO����������
	UART_PROTOCOL1_RxFIFOProcess(&uartProtocolCB1);

	// UART���������������
	UART_PROTOCOL1_CmdFrameProcess(&uartProtocolCB1);
	
	// UARTЭ��㷢�ʹ������
	UART_PROTOCOL1_TxStateProcess();

	// ���������ͨѶʱ����
	UART_PROTOCOL1_DataCommunicationSequenceProcess();

//=========================================================
	// ��﮵�2��Э����Ƕ��KM5SЭ��
	UART_PROTOCOL_Process();
}

// ��������֡�������������
void UART_PROTOCOL1_TxAddData(uint8 data)
{
	uint16 head = uartProtocolCB1.tx.head;
	uint16 end =  uartProtocolCB1.tx.end;
	UART_PROTOCOL1_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB1.tx.cmdQueue[uartProtocolCB1.tx.end];

	// ���ͻ������������������
	if((end + 1) % UART_PROTOCOL1_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ��β����֡�������˳�
	if(pCmdFrame->length >= UART_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void UART_PROTOCOL1_TxAddFrame(void)
{
	uint16 cc = 0;
	uint16 i = 0;
	uint16 head = uartProtocolCB1.tx.head;
	uint16 end  = uartProtocolCB1.tx.end;
	UART_PROTOCOL1_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB1.tx.cmdQueue[uartProtocolCB1.tx.end];
	uint16 length = pCmdFrame->length;

	// ���ͻ������������������
	if((end + 1) % UART_PROTOCOL1_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ����֡���Ȳ��㣬������������ݣ��˳�
	if(UART_PROTOCOL1_CMD_FRAME_LENGTH_MIN-1 > length)	// ��ȥ"У���"1���ֽ�
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// ��β����֡�������˳�
	if(length >= UART_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	pCmdFrame->buff[UART_PROTOCOL1_CMD_LENGTH_INDEX] = length + 1;	// �������ݳ��ȣ�Ҫ����1�ֽ�У�鳤��

	for(i=0; i<length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = cc ;

	uartProtocolCB1.tx.end ++;
	uartProtocolCB1.tx.end %= UART_PROTOCOL1_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void UART_PROTOCOL1_DataStructInit(UART_PROTOCOL1_CB* pCB)
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
	for(i = 0; i < UART_PROTOCOL1_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<UART_PROTOCOL1_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL1_MacProcess(uint16 standarID, uint8* pData, uint16 length)
{
	uint16 end = uartProtocolCB1.rxFIFO.end;
	uint16 head = uartProtocolCB1.rxFIFO.head;
	uint8 rxdata = 0x00;
	
	// ��������
	rxdata = *pData;

	// һ���������������������
	if((end + 1)%UART_PROTOCOL1_RX_FIFO_SIZE == head)
	{
		return;
	}
	// һ��������δ�������� 
	else
	{
		// �����յ������ݷŵ���ʱ��������
		uartProtocolCB1.rxFIFO.buff[end] = rxdata;
		uartProtocolCB1.rxFIFO.end ++;
		uartProtocolCB1.rxFIFO.end %= UART_PROTOCOL1_RX_FIFO_SIZE;
	}

//====================================================================
	// ����KM5SЭ�����
	UART_PROTOCOL_MacProcess(standarID, pData, length);
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL1_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{		
	uartProtocolCB1.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL1_ConfirmTempCmdFrameBuff(UART_PROTOCOL1_CB* pCB)
{
	UART_PROTOCOL1_RX_CMD_FRAME* pCmdFrame = NULL;
	
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
	pCB->rx.end %= UART_PROTOCOL1_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
	
	return TRUE;
}

// Э��㷢�ʹ������
void UART_PROTOCOL1_TxStateProcess(void)
{
	uint16 head = uartProtocolCB1.tx.head;
	uint16 end =  uartProtocolCB1.tx.end;
	uint16 length = uartProtocolCB1.tx.cmdQueue[head].length;
	uint8* pCmd = uartProtocolCB1.tx.cmdQueue[head].buff;
	uint16 localDeviceID = uartProtocolCB1.tx.cmdQueue[head].deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == uartProtocolCB1.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������		
	if (!(*uartProtocolCB1.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	uartProtocolCB1.tx.cmdQueue[head].length = 0;
	uartProtocolCB1.tx.head ++;
	uartProtocolCB1.tx.head %= UART_PROTOCOL1_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL1_RxFIFOProcess(UART_PROTOCOL1_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	UART_PROTOCOL1_RX_CMD_FRAME* pCmdFrame = NULL;
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
		// ����ͷ����ɾ����ǰ�ֽڲ��˳�
		if(UART_PROTOCOL1_CMD_RX_DEVICE_ADDR != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= UART_PROTOCOL1_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if((pCB->rx.end + 1)%UART_PROTOCOL1_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// ���UARTͨѶ��ʱʱ������-2016.1.5����
#if UART_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
						UART_PROTOCOL1_BUS_UNIDIRECTIONAL_TIME_OUT,
						UART_PROTOCOL1_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL1_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if(pCmdFrame->length >= UART_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX)
		{
#if UART_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			UART_PROTOCOL1_StopRxTimeOutCheck();
#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0;	// 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= UART_PROTOCOL1_RX_FIFO_SIZE;
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
			pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL1_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ���������� ����
						
			// �����ж�����֡��С���ȣ�һ�����������������ٰ���4���ֽ�: ����֡��С���ȣ�����:�豸��ַ�����ݳ��ȡ������֡�У��ͣ���˲���4���ֽڵıض�������
			if(pCmdFrame->length < UART_PROTOCOL1_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
			if(pCmdFrame->buff[UART_PROTOCOL1_CMD_LENGTH_INDEX] > UART_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX)
			{
#if UART_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				UART_PROTOCOL1_StopRxTimeOutCheck();
#endif
			
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= UART_PROTOCOL1_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// ����֡����У��
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[UART_PROTOCOL1_CMD_LENGTH_INDEX])
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if(!UART_PROTOCOL1_CheckSUM(pCmdFrame))
			{
#if UART_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				UART_PROTOCOL1_StopRxTimeOutCheck();
#endif
				
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= UART_PROTOCOL1_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if UART_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			UART_PROTOCOL1_StopRxTimeOutCheck();
#endif
			
			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= UART_PROTOCOL1_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			UART_PROTOCOL1_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL1_CheckSUM(UART_PROTOCOL1_RX_CMD_FRAME* pCmdFrame)
{
	uint16 cc = 0;
	uint16 i = 0;
	
	if(NULL == pCmdFrame)
	{
		return FALSE;
	}

	// ���豸��ַ��ʼ����У����֮ǰ��һ���ֽڣ����ν����������
	for(i=0; i<pCmdFrame->length-1; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	
	// �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
	if(pCmdFrame->buff[pCmdFrame->length-1] != cc)
	{
		return FALSE;
	}
	
	return TRUE;
}

// UART����֡����������
void UART_PROTOCOL1_CmdFrameProcess(UART_PROTOCOL1_CB* pCB)
{
	UART_PROTOCOL1_CMD cmd = UART_PROTOCOL1_CMD_NULL;
	UART_PROTOCOL1_RX_CMD_FRAME* pCmdFrame = NULL;
	uint32 tempVal, tempVal2;
	uint16 i;
	uint8* pBuff = NULL;
	BOOL bTemp;
	uint16 powerPassword, menuPassword;

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
	
	// ����ͷ�Ƿ����˳�
	if(UART_PROTOCOL1_CMD_RX_DEVICE_ADDR != pCmdFrame->buff[UART_PROTOCOL1_CMD_DEVICE_ADDR_INDEX])
	{
		// ɾ������֡
		pCB->rx.head ++;
		pCB->rx.head %= UART_PROTOCOL1_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (UART_PROTOCOL1_CMD)pCmdFrame->buff[UART_PROTOCOL1_CMD_CMD_INDEX];
	
	// ִ������֡
	switch(cmd)
	{
		// ���������ִ��
		case UART_PROTOCOL1_CMD_NULL:
			break;

		// ��������״̬
		case UART_PROTOCOL1_CMD_HMI_CTRL_RUN:

			UART_PROTOCOL1_OnReceiveReport(&pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA1_INDEX], UART_PROTOCOL1_RX_PARAM_SIZE);
			
			// ���������־��
			PARAM_SetRecaculateRequest(TRUE);
					
			break;

		// ��λ��д�Ǳ����
		case UART_PROTOCOL1_CMD_WRITE_CONTROL_PARAM:
			// ��������
			if ((pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA1_INDEX] >= 1) && (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA1_INDEX] <= 5))
			{
				PARAM_SetBrightness(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA1_INDEX]);
			}

			// ����ʱ��
			if (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA2_INDEX] <= 10)
			{
				PARAM_SetPowerOffTime(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA2_INDEX]);
			}

			// ϵͳʱ�䣬������ʱ��
			if ((pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA3_INDEX] <= 99) 
			&& (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA4_INDEX] >= 1) && (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA4_INDEX] <= 12)
			&& (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA5_INDEX] >= 1) && (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA5_INDEX] <= 31)
			&& (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA6_INDEX] <= 23)
			&& (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA7_INDEX] <= 59))
			{
/*				DS1302_SetDate(2000+pCmdFrame->buff[UART_PROTOCOL_CMD_DATA3_INDEX], 
							   pCmdFrame->buff[UART_PROTOCOL_CMD_DATA4_INDEX],
							   pCmdFrame->buff[UART_PROTOCOL_CMD_DATA5_INDEX],
							   pCmdFrame->buff[UART_PROTOCOL_CMD_DATA6_INDEX],
							   pCmdFrame->buff[UART_PROTOCOL_CMD_DATA7_INDEX],
							   0);
				PARAM_SetRTC(2000+pCmdFrame->buff[UART_PROTOCOL_CMD_DATA3_INDEX], 
							   pCmdFrame->buff[UART_PROTOCOL_CMD_DATA4_INDEX],
							   pCmdFrame->buff[UART_PROTOCOL_CMD_DATA5_INDEX],
							   pCmdFrame->buff[UART_PROTOCOL_CMD_DATA6_INDEX],
							   pCmdFrame->buff[UART_PROTOCOL_CMD_DATA7_INDEX],
							   0);
				*/
			}

			// ϵͳ��ѹ
			if ((24 == pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA8_INDEX]) || (36 == pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA8_INDEX]) || (48 == pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA8_INDEX]))
			{
				PARAM_SetBatteryVoltageLevel(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA8_INDEX]);	// ��λ:V
			}

			// ������λ��
			if ((3 == pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA9_INDEX]) || (5 == pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA9_INDEX]) || (9 == pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA9_INDEX]))
			{
				PARAM_SetMaxAssist((ASSIST_ID_E)pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA9_INDEX]);
			}

			// ��������
			PARAM_SetAssistDirection((BOOL)pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA10_INDEX]);

			// ������ʼ�Ÿ���
			if ((pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA11_INDEX] >= 2) && (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA11_INDEX] <= 64))
			{
				PARAM_SetAssitStartOfSteelNum(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA11_INDEX]);
			}

			// ��������
			PARAM_SetAssistPercent(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA12_INDEX]);

			// ת�ѷֵ�
			PARAM_SetTurnbarLevel((BOOL)pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA13_INDEX]);

			// ת������6kmph
			PARAM_SetTurnBarSpeed6kmphLimit((BOOL)pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA14_INDEX]);

			// ����������
			if (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA15_INDEX] <= 3)
			{
				PARAM_SetSlowStart(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA15_INDEX]);
			}

			// ���ٴŸ���
			if (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA16_INDEX] <= 15)
			{
				PARAM_SetCycleOfSteelNum(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA16_INDEX]);
			}

			// Ƿѹ����
			tempVal = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA17_INDEX];
			tempVal <<= 8;
			tempVal |= pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA18_INDEX];			
			PARAM_SetLowVoltageThreshold(tempVal);

			// ��������
			PARAM_SetCurrentLimit(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA19_INDEX]*1000);	// ��λ:mA

			// ��������
			if ((pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA20_INDEX] >= 10) && (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA20_INDEX] <= 31))
			{
				PARAM_SetSpeedLimit(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA20_INDEX]*10);	// ��λ:0.1km/h
			}

			// �־�����
			switch (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA21_INDEX])
			{
				case 0:
					PARAM_SetWheelSizeID(PARAM_WHEEL_SIZE_16_INCH);

					PARAM_SetNewWheelSizeInch(160);

					break;

				case 1:
					PARAM_SetWheelSizeID(PARAM_WHEEL_SIZE_18_INCH);

					PARAM_SetNewWheelSizeInch(180);

					break;

				case 2:
					PARAM_SetWheelSizeID(PARAM_WHEEL_SIZE_20_INCH);

					PARAM_SetNewWheelSizeInch(200);

					break;

				case 3:
					PARAM_SetWheelSizeID(PARAM_WHEEL_SIZE_22_INCH);

					PARAM_SetNewWheelSizeInch(220);

					break;

				case 4:
					PARAM_SetWheelSizeID(PARAM_WHEEL_SIZE_24_INCH);

					PARAM_SetNewWheelSizeInch(240);

					break;

				case 5:
					PARAM_SetWheelSizeID(PARAM_WHEEL_SIZE_26_INCH);

					PARAM_SetNewWheelSizeInch(260);

					break;

				case 6:
					PARAM_SetWheelSizeID(PARAM_WHEEL_SIZE_700C);

					PARAM_SetNewWheelSizeInch(275);

					break;

				case 7:
					PARAM_SetWheelSizeID(PARAM_WHEEL_SIZE_28_INCH);

					PARAM_SetNewWheelSizeInch(280);
					
					break;

				default:
					break;
			}	

			// ������ʾ�仯ʱ�䣬��λ:s
			if ((pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA22_INDEX] >= 1) && (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA22_INDEX] <= 60))
			{
				PARAM_SetBatteryCapVaryTime(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA22_INDEX]);
			}

			// ���߹��ϳ�ʱʱ�䣬��λ:ms
			if ((pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA23_INDEX] >= 5) && (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA23_INDEX] <= 255))
			{
				PARAM_SetBusAliveTime(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA23_INDEX] * 1000UL);	// ��תΪ����
			}
			
			// �ٶ�ƽ���ȵȼ�
			if (pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA24_INDEX] <= 10)
			{
				PARAM_SetSpeedFilterLevel(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA24_INDEX]);
			}

			// ��ʾ��λ
			// ��Ӣ�Ƶ�λ 0���� 1Ӣ��
			PARAM_SetUnit((pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA25_INDEX]?UNIT_INCH:UNIT_METRIC));

			// ���� 0������ 1������
			PARAM_SetExistBle((pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA26_INDEX]?TRUE:FALSE));
			
			// �ܳ�
			tempVal = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA31_INDEX];
			tempVal <<= 8;
			tempVal |= pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA32_INDEX];
			PARAM_SetNewperimeter(tempVal);

			// Э��
			PARAM_SetUartProtocol(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA33_INDEX]);

			// �������㷽ʽ
			PARAM_SetPercentageMethod((BATTERY_DATA_SRC_E)(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA34_INDEX]));

			// �Ƴ���������
			PARAM_SetPushSpeedSwitch(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA35_INDEX]);

			// Ĭ�ϵ�λ
			PARAM_SetDefaultAssist(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA36_INDEX]);

			// LOGO��
			PARAM_SetlogoMenu(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA37_INDEX]);

			// ����ͨ�ŵ�ƽ
			PARAM_SetUartLevel(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA38_INDEX]);

			// ����������
			PARAM_SetBeepSwitch((BOOL)pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA39_INDEX]);

			// ���ٷ���������
			PARAM_SetLimitSpeedBeep(pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA40_INDEX]);
			
			// ����Ѳ������
			PARAM_SetCruiseEnableSwitch((BOOL)pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA41_INDEX]);

			powerPassword = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA42_INDEX];
			powerPassword <<= 8;
			powerPassword |= pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA43_INDEX];
			
			// ��������
			PARAM_SetPowerOnPassword(powerPassword);

			menuPassword = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA44_INDEX];
			menuPassword <<= 8;
			menuPassword |= pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA45_INDEX];
			
			// �˵�����
			PARAM_SetMenuPassword(menuPassword);

			// �ָ���������
			PARAM_SetResFactorySet((BOOL)pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA46_INDEX]);

			// NVM���£�����д��
			NVM_SetDirtyFlag(TRUE);
			
			// д��������ñ���
			NVM_Save_FactoryReset(TRUE);
			
			// ����Ӧ��
			UART_PROTOCOL1_ReportWriteParamResult(TRUE);
			break;

		// ��λ��������
		case UART_PROTOCOL1_CMD_READ_CONTROL_PARAM:
			UART_PROTOCOL1_ReportConfigureParam(TRUE);			
			break;

		case UART_PROTOCOL1_CMD_FLAG_ARRAY_READ:
			// �������ݲ����ڷ���			
			pBuff = (uint8 *)malloc(128);
			if (NULL == pBuff)
			{
				break;
			}
			
			// ����豸��ַ
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_TX_DEVICE_ADDR);	
						
			// ���֡��
			UART_PROTOCOL1_TxAddData(64+5);
							
			// ��������� 
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_FLAG_ARRAY_READ);

			// ��־�����ݳ���
			UART_PROTOCOL1_TxAddData(64);

			// ����FALSHָ����������
			IAP_FlashReadWordArray(PARAM_MCU_TEST_FLAG_ADDEESS, (uint32 *)pBuff, 16);
			
			// ���ֽ���ǰ�����ֽ��ں�
			for (i = 0; i < 16; i++)
			{
				UART_PROTOCOL1_TxAddData(pBuff[4*i+3]);
				UART_PROTOCOL1_TxAddData(pBuff[4*i+2]);
				UART_PROTOCOL1_TxAddData(pBuff[4*i+1]);
				UART_PROTOCOL1_TxAddData(pBuff[4*i+0]);
			}
			
			UART_PROTOCOL1_TxAddFrame();

			free(pBuff);
			break;
			
		case UART_PROTOCOL1_CMD_FLAG_ARRAY_WRITE:
			// д���־λ��
			tempVal = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA1_INDEX];

			// д���־����
			tempVal2 = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA2_INDEX];	
			tempVal2 = ((tempVal2 << 8) | pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA3_INDEX]);	
			tempVal2 = ((tempVal2 << 8) | pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA4_INDEX]);	
			tempVal2 = ((tempVal2 << 8) | pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA5_INDEX]);
		
			// д������
			IAP_FlashWriteWordArrayWithErase(PARAM_MCU_TEST_FLAG_ADDEESS + tempVal*4, &tempVal2, 1);

			// ����豸��ַ
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_TX_DEVICE_ADDR);	
						
			// ���֡��
			UART_PROTOCOL1_TxAddData(5);
							
			// ��������� 
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_FLAG_ARRAY_WRITE);

			UART_PROTOCOL1_TxAddData(TRUE);
			
			UART_PROTOCOL1_TxAddFrame();

			break;
			
		case UART_PROTOCOL1_CMD_VERSION_TYPE_WRITE:
			// ��һ��д������
			tempVal = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA1_INDEX];
			
			// �ڶ���Ϊ��Ϣ����
			tempVal2 = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA2_INDEX];

			pBuff = (uint8 *)malloc(128);
			if (NULL == pBuff)
			{
				break;
			}
			
			// ����
			for (i = 0; i < 128; i++)
			{
				pBuff[i] = 0;
			}
			// ������Ϣ
			for (i = 0; i < tempVal2 + 1; i++)
			{
				pBuff[i] = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA1_INDEX+1+i];
			}
			
			// �ж����������Ƿ�Ϸ�
			if (16 >= tempVal)
			{
				bTemp = IAP_FlashWriteWordArrayWithErase(PARAM_MCU_VERSION_ADDRESS[tempVal], (uint32 *)pBuff, (tempVal2 + 1 + 3) / 4);
			}
			else
			{
				bTemp = FALSE;
			}

			// ����豸��ַ
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_TX_DEVICE_ADDR);	
						
			// ���֡��
			UART_PROTOCOL1_TxAddData(5);
							
			// ��������� 
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_VERSION_TYPE_WRITE);

			// ����
			UART_PROTOCOL1_TxAddData(tempVal);

			// ���
			UART_PROTOCOL1_TxAddData(bTemp);
			
			UART_PROTOCOL1_TxAddFrame();

			free(pBuff);
			
			break;
			
		case UART_PROTOCOL1_CMD_VERSION_TYPE_READ:
			tempVal = pCmdFrame->buff[UART_PROTOCOL1_CMD_DATA1_INDEX];
			
			pBuff = (uint8 *)malloc(128);
			if (NULL == pBuff)
			{
				break;
			}
			// ����
			for (i = 0; i < 128; i++)
			{
				pBuff[i] = 0;
			}

			// Ŀǰ�Ϊflag��Ϊ64���ֽ�
			IAP_FlashReadWordArray(PARAM_MCU_VERSION_ADDRESS[tempVal], (uint32 *)pBuff, 64/4);

			// ����豸��ַ
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_TX_DEVICE_ADDR);	
						
			// ���֡��
			UART_PROTOCOL1_TxAddData(5);
							
			// ��������� 
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_VERSION_TYPE_READ);

			// ����
			UART_PROTOCOL1_TxAddData(tempVal);

			switch (tempVal)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
					if ((pBuff[0] > 0) && (pBuff[0] < 64))
					{
						// ��Ϣ����
						UART_PROTOCOL1_TxAddData(pBuff[0]);

						// ������Ϣ
						for (i = 0; i < pBuff[0]; i++)
						{
							UART_PROTOCOL1_TxAddData(pBuff[i+1]);
						}
					}
					else
					{
						// ��Ϣ����
						UART_PROTOCOL1_TxAddData(0);
					}
					break;

				case 5:
				case 6:
					// ��Ϣ����
					UART_PROTOCOL1_TxAddData(4);
					
					// ������Ϣ�����ֽ���ǰ�����ֽ��ں�
					UART_PROTOCOL1_TxAddData(pBuff[1]);
					UART_PROTOCOL1_TxAddData(pBuff[2]);
					UART_PROTOCOL1_TxAddData(pBuff[3]);
					UART_PROTOCOL1_TxAddData(pBuff[4]);
					break;
					
				case 7:
					// ��Ϣ����
					UART_PROTOCOL1_TxAddData(32);

					// ������Ϣ
					for (i = 0; i < 32; i++)
					{
						UART_PROTOCOL1_TxAddData(pBuff[i+1]);
					}
					break;

				default:
					break;
			}

			UART_PROTOCOL1_TxAddFrame();

			free(pBuff);
			
			break;

		case UART_PROTOCOL1_CMD_TEST_LCD:


			// �������ͷ
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL_CMD_HEAD);

			// ����豸��ַ
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

			// ���������
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL_CMD_TEST_LCD);

			// ���ݳ���
			UART_PROTOCOL1_TxAddData(0);

			// ���
			UART_PROTOCOL1_TxAddData(0);

			// ��Ӽ���ͣ������������	
			UART_PROTOCOL1_TxAddFrame();
			break;
		
		case UART_PROTOCOL1_CMD_TEST_KEY:
			// �������ͷ
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL_CMD_HEAD);

			// ����豸��ַ
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

			// ���������
			UART_PROTOCOL1_TxAddData(UART_PROTOCOL_CMD_TEST_KEY);

			// ���ݳ���
			UART_PROTOCOL1_TxAddData(0);

			// ���
			UART_PROTOCOL1_TxAddData( PARAM_GetKeyValue() );

			// ��Ӽ���ͣ������������	
			UART_PROTOCOL1_TxAddFrame();
			break;
			
		default:
			break;
	}

	// ����UART����ͨѶ��ʱ�ж�
	UART_PROTOCOL1_StartTimeoutCheckTask();
	
	// ɾ������֡
	pCB->rx.head ++;
	pCB->rx.head %= UART_PROTOCOL1_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if UART_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
void UART_PROTOCOL1_CALLBACK_RxTimeOut(uint32 param)
{
	UART_PROTOCOL1_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &uartProtocolCB1.rx.cmdQueue[uartProtocolCB1.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0;	// 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	uartProtocolCB1.rxFIFO.head ++;
	uartProtocolCB1.rxFIFO.head %= UART_PROTOCOL1_RX_FIFO_SIZE;
	uartProtocolCB1.rxFIFO.currentProcessIndex = uartProtocolCB1.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void UART_PROTOCOL1_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

// ������������·����������־
void UART_PROTOCOL1_SetTxAtOnceRequest(uint32 param)
{
	uartProtocolCB1.txAtOnceRequest = (BOOL)param;
}

// ������������·����������־
void UART_PROTOCOL1_CALLBACK_SetTxPeriodRequest(uint32 param)
{
	uartProtocolCB1.txPeriodRequest = (BOOL)param;
}

// ���������ͨѶʱ����
void UART_PROTOCOL1_DataCommunicationSequenceProcess(void)
{
	// ��������������������Է�������
	if ((uartProtocolCB1.txAtOnceRequest) || (uartProtocolCB1.txPeriodRequest))
	{
		// �������в�������
		UART_PROTOCOL1_SendRuntimeParam();

		if (uartProtocolCB1.txAtOnceRequest)
		{
			// ���¸�λ�����·���ʱ��
			TIMER_ResetTimer(TIMER_ID_PROTOCOL_PARAM_TX);
		}

		uartProtocolCB1.txAtOnceRequest = FALSE;
		uartProtocolCB1.txPeriodRequest = FALSE;
	}
}

// 	UART���߳�ʱ������
void UART_PROTOCOL1_CALLBACK_UartBusError(uint32 param)
{
	PARAM_SetErrorCode(ERROR_TYPE_COMMUNICATION_TIME_OUT);
}

// �������в������������
void UART_PROTOCOL1_SendRuntimeParam(void)
{
	static uint8 assistCode;
	uint16 wheelSize;
	
	uint16 pwm;
	uint16 lowVoltageThreshold;

	// ����豸��ַ
	UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_TX_DEVICE_ADDR);

	// ������ݳ��ȣ��̶�Ϊ20
	UART_PROTOCOL1_TxAddData(20);

	// ���������
	UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_HMI_CTRL_RUN);

	// ������ʽ�趨
	// 0 : ��������(ͨ��������λ�������������������ʱת����Ч)��
	// 1 : ������(ͨ��ת������, ��ʱ������λ��Ч)��
	// 2 : ���������͵�����ͬʱ����(������������״̬����Ч)��
	UART_PROTOCOL1_TxAddData(PARAM_GetDriverControlMode());

	// ������λ
	switch (PARAM_GetMaxAssist())
	{
		case ASSIST_3:
			switch (PARAM_GetAssistLevel())
			{
				case ASSIST_0:
					assistCode = 0;
					break;

				case ASSIST_1:
					assistCode = 5;
					break;

				case ASSIST_2:
					assistCode = 10;
					break;

				case ASSIST_3:
					assistCode = 15;
					break;

				// �Ƴ�
				case ASSIST_P:
					assistCode = assistCode;
					break;
			}
			break;

		case ASSIST_5:
			switch (PARAM_GetAssistLevel())
			{
				case ASSIST_0:
					assistCode = 0;
					break;

				case ASSIST_1:
					assistCode = 3;
					break;

				case ASSIST_2:
					assistCode = 6;
					break;

				case ASSIST_3:
					assistCode = 9;
					break;

				case ASSIST_4:
					assistCode = 12;
					break;

				case ASSIST_5:
					assistCode = 15;
					break;

				// �Ƴ�
				case ASSIST_P:
					assistCode = assistCode;
					break;

			}
			break;

		case ASSIST_9:
			switch (PARAM_GetAssistLevel())
			{
				case ASSIST_0:
					assistCode = 0;
					break;

				case ASSIST_1:
					assistCode = 1;
					break;

				case ASSIST_2:
					assistCode = 3;
					break;

				case ASSIST_3:
					assistCode = 5;
					break;

				case ASSIST_4:
					assistCode = 7;
					break;

				case ASSIST_5:
					assistCode = 9;
					break;

				case ASSIST_6:
					assistCode = 11;
					break;

				case ASSIST_7:
					assistCode = 13;
					break;

				case ASSIST_8:
					assistCode = 14;
					break;

				case ASSIST_9:
					assistCode = 15;
					break;

				// �Ƴ�
				case ASSIST_P:
					assistCode = assistCode;
					break;
			}
			break;
	}
	UART_PROTOCOL1_TxAddData(assistCode);

	// �����������趨1
	// bit7��0: ��������������1: ����������
	// bit6��0: ������ 1: ��������
	// bit5���ƹ���� 0: �ر� 1: ��
	// bit4��0: ͨѶ���� 1: ͨѶ����(���ܽ��յ�������������)
	// bit3���л�Ѳ���ķ�ʽ
	//		 0: ʹ�ô�����ʽ���Ƿ�Ѳ����BIT0 (�Ǳ�Ĭ��״̬)
	//		 1: ʹ�ó�������ʽ ��BIT1
	//		 ��������£�����������ʱ��Ѳ����ʽ��ȷ�������Բ����Ǳ�Bit3��ֱ�Ӹ�����Ҫ��ȡBit1������Bit0 ����
	// bit2������״̬ 1����������ֵ��0��δ��������ֵ��������һֱΪ0)��
	// bit1��Ѳ������ģʽ (0: ��Ѳ����1: Ѳ����)
	//		(���Ҫ��6KM/���ơ� ���ְ�ס���֣������Ƴ���Ѳ��������������ֱ�Ӷ���λ�ã�
	// bit0��Ѳ���л�ģʽ (0: ����Ҫ�л�Ѳ��״̬ 1: Ҫ�л�Ѳ��״̬)
	//		�������Ҫ��ס��ת״̬��Ѳ�������綨��Ѳ��������ֱ�Ӷ���λ�ã�
	hmiDriveCB1.DriverSet1.param.driverState = 1;
	hmiDriveCB1.DriverSet1.param.zeroStartOrNot = PARAM_GetZeroStartOrNot();
	hmiDriveCB1.DriverSet1.param.isLightOn = PARAM_GetLightSwitch();
	hmiDriveCB1.DriverSet1.param.comunicationError = 0;
	hmiDriveCB1.DriverSet1.param.switchCruiseWay = PARAM_GetSwitchCruiseWay();

	// ����ʹ��
	if (PARAM_GetSpeedLimitSwitch())
	{
		// ��������ֵ
		if (paramCB.runtime.speed > paramCB.nvm.param.common.speed.limitVal)
		{
			hmiDriveCB1.DriverSet1.param.speedLimitState = 1;
		}
		// δ��������ֵ
		else
		{
			hmiDriveCB1.DriverSet1.param.speedLimitState = 0;
		}
	}
	// ������һֱΪ0
	else
	{
		hmiDriveCB1.DriverSet1.param.speedLimitState = 0;
	}

	// ��ʱѲ����6KM����
	/*if (PARAM_GetSwitchCruiseWay())	// ��������ʽ
	{
		//hmiDriveCB1.DriverSet1.param.isPushModeOn = PARAM_IsPushAssistOn();
		
		hmiDriveCB1.DriverSet1.param.switchCruiseMode = 1;	// ��������ʽ,�����Ĵ�λ
	}
	else							// ������ʽ
	{
		hmiDriveCB1.DriverSet1.param.isPushModeOn = 0;		// ������ʽ,�����Ĵ�λ

		hmiDriveCB1.DriverSet1.param.switchCruiseMode = PARAM_GetCruiseSwitch();

	}*/

	// ��ʱѲ����6KM����
	if (PARAM_GetSwitchCruiseWay())	// ��������ʽ
	{		
		hmiDriveCB1.DriverSet1.param.switchCruiseMode = 0;
	}
	else							// ������ʽ
	{
		if (PARAM_GetCruiseSwitch())
		{
			hmiDriveCB1.DriverSet1.param.switchCruiseMode = 1;

			// Ѳ������ʹ�ú����̹ر�
			PARAM_SetCruiseSwitch(FALSE);
		}
		else
		{
			hmiDriveCB1.DriverSet1.param.switchCruiseMode = 0;
		}

	}

	// �Ƴ�����״̬
	hmiDriveCB1.DriverSet1.param.isPushModeOn = PARAM_IsPushAssistOn();
	
	UART_PROTOCOL1_TxAddData(hmiDriveCB1.DriverSet1.val);

	// ���ٴŸ�
	UART_PROTOCOL1_TxAddData(PARAM_GetCycleOfSteelNum());

	// �־�
	/*switch (PARAM_GetWheelSizeID())
	{
		case PARAM_WHEEL_SIZE_14_INCH:
			wheelSize = 140;
			break;

		case PARAM_WHEEL_SIZE_16_INCH:
			wheelSize = 160;
			break;

		case PARAM_WHEEL_SIZE_18_INCH:
			wheelSize = 180;
			break;

		case PARAM_WHEEL_SIZE_20_INCH:
			wheelSize = 200;
			break;

		case PARAM_WHEEL_SIZE_22_INCH:
			wheelSize = 220;
			break;

		case PARAM_WHEEL_SIZE_24_INCH:
			wheelSize = 240;
			break;

		case PARAM_WHEEL_SIZE_26_INCH:
			wheelSize = 260;
			break;

		case PARAM_WHEEL_SIZE_27_INCH:
			wheelSize = 270;
			break;

		case PARAM_WHEEL_SIZE_27_5_INCH:
			wheelSize = 275;
			break;

		case PARAM_WHEEL_SIZE_28_INCH:
			wheelSize = 280;
			break;

		case PARAM_WHEEL_SIZE_29_INCH:
			wheelSize = 290;
			break;

		case PARAM_WHEEL_SIZE_700C:
			wheelSize = 700;
			break;

		default:
			wheelSize = 260;
			break;
			
	}
	*/

	wheelSize = PARAM_GetNewWheelSizeInch();
	UART_PROTOCOL1_TxAddData(wheelSize>>8);
	UART_PROTOCOL1_TxAddData(wheelSize&0xFF);

	// ����������
	UART_PROTOCOL1_TxAddData(PARAM_GetAssistSensitivity());

	// ��������ǿ��
	UART_PROTOCOL1_TxAddData(PARAM_GetAssistStartIntensity());

	// ����ר�õ����������Ÿ�Ƭ��
	UART_PROTOCOL1_TxAddData(PARAM_GetReversalHolzerSteelNum());

	// ����ֵ���˴�һ��Ҫ�ù���km/h
	UART_PROTOCOL1_TxAddData(paramCB.nvm.param.common.speed.limitVal/10);

	// ����������ֵ
	UART_PROTOCOL1_TxAddData(PARAM_GetCurrentLimit()/1000);		// �ͻ��ĵ�λΪ1A

	// ������Ƿѹֵ
	lowVoltageThreshold = PARAM_GetLowVoltageThreshold()/100;
	UART_PROTOCOL1_TxAddData(lowVoltageThreshold>>8);			// �ͻ��ĵ�λΪ0.1V
	UART_PROTOCOL1_TxAddData(lowVoltageThreshold&0xFF);

	// ת�ѵ���PWMռ�ձ�
	//pwm = PARAM_GetPwmTopLimit(PARAM_GetAssistLevel())*10;			// �ͻ��ĵ�λΪ0.1%
	pwm = 0;
	UART_PROTOCOL1_TxAddData(pwm>>8);
	UART_PROTOCOL1_TxAddData(pwm&0xFF);

	// �����������趨2 + �����Ÿ��̴Ÿָ���
	// bit7��������־ 0: ��Ч��1: ����
	// bit6���Զ�Ѳ����־ 0: ���Զ�Ѳ���� 1: ���Զ�Ѳ����ʱ�������ȷ����
	// bit5������0
	// bit4������0
	// bit3 2 1 0�������Ÿ��̴Ÿָ���
	// 			   5 : 5 �ŴŸ�
	// 			   8 : 8 �ŴŸ�
	// 			   12: 12 �ŴŸ�
	hmiDriveCB1.DriverSet2.param.astern = 0;
	hmiDriveCB1.DriverSet2.param.autoCruise = 0;
	hmiDriveCB1.DriverSet2.param.assistSteelType = PARAM_GetSteelType();
	UART_PROTOCOL1_TxAddData(hmiDriveCB1.DriverSet2.val);

	// ��Ӽ���ͣ������������
	UART_PROTOCOL1_TxAddFrame();
}

// �������ϱ���������
void UART_PROTOCOL1_OnReceiveReport(uint8* pBuff, uint32 len)
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
	for (i = 0; (i < len) && (i < UART_PROTOCOL1_RX_PARAM_SIZE); i++)
	{
		hmiDriveCB1.rx.buff[i] = pBuff[i];
	}

	// ����������������ϱ������ݷ����� hmiDriveCB1.rx.buff ��

	// ��ȡ����״̬
	// ɲ�ѹ���
	if (hmiDriveCB1.rx.param.breakError)
	{
		PARAM_SetErrorCode(ERROR_TYPE_BREAK_ERROR);
	}
	// Ƿѹ����
	else if (hmiDriveCB1.rx.param.underVoltageProtectionError)
	{
		PARAM_SetErrorCode(ERROR_TYPE_BATTERY_UNDER_VOLTAGE_ERROR);
	}
	// ���ȱ�����
	else if (hmiDriveCB1.rx.param.motorPhaseError)
	{
		PARAM_SetErrorCode(ERROR_TYPE_MOTOR_ERROR);
	}
	// ת�ѹ���
	else if (hmiDriveCB1.rx.param.turnBarError)
	{
		PARAM_SetErrorCode(ERROR_TYPE_TURN_ERROR);
	}
	// ����������
	else if (hmiDriveCB1.rx.param.driverError)
	{
		PARAM_SetErrorCode(ERROR_TYPE_DRIVER_ERROR);
	}
	// ��������
	else if (hmiDriveCB1.rx.param.holzerError)
	{
		PARAM_SetErrorCode(ERROR_TYPE_HALL_ERROR);
	}
	// ���յ�������ų����еĹ��ϣ�ȷ���޴�����
	else
	{
		PARAM_SetErrorCode(ERROR_TYPE_NO_ERROR);
	}

	// ��ȡ��ʱѲ��״̬
	if (hmiDriveCB1.rx.param.cruiseState)
	{
		PARAM_SetCruiseState(TRUE);
	}
	else
	{
		PARAM_SetCruiseState(FALSE);
	}

	// ��ȡ6KM�Ƴ�����״̬
	if (hmiDriveCB1.rx.param.cruise6kmState)
	{
		PARAM_SetPushAssistState(TRUE);
	}
	else
	{
		PARAM_SetPushAssistState(FALSE);
	}

	// ��ȡ���е���
	// Ԥ�����ֽڵ�bit7Ϊ������Ŀǰ����Ϊ0; �����ֽڵ�bit6Ϊ1ʱ����ʾ��λΪ0.1A��Ϊ0ʱ�������ĵ�λ��Ȼ��1A
	tempVal = hmiDriveCB1.rx.param.ucCurBatteryCurrentH;
	tempVal <<= 8;
	tempVal |= hmiDriveCB1.rx.param.ucCurBatteryCurrentL;
	if (tempVal & 0x4000)
	{
		tempVal &= ~0xC000;
		PARAM_SetBatteryCurrent((uint16)(tempVal*0.1f*1000));
	}
	else
	{
		tempVal &= ~0xC000;
		PARAM_SetBatteryCurrent((uint16)(tempVal*1000));
	}

	// ��ȡʵʱ�ٶȣ���λ:KM/H
	tempVal = 0;
	tempVal = hmiDriveCB1.rx.param.oneCycleTimeH;
	tempVal <<= 8;
	tempVal |= hmiDriveCB1.rx.param.oneCycleTimeL;
	// ��ŵ�﹤ȷ�ϴ��ڵ���3000ʱ�ٶ�Ϊ��
	// ���յ��������ϱ�תһȦʱ��Ϊ3000ʱ������Ϊ�ٶ�Ϊ0
	if(tempVal >= 3000UL)
	{
		uartProtocolCB1.speedFilter.realSpeed = 0;
	}
	else
	{
		//perimeter = PARAM_GetPerimeter((uint8)wheelSizeID);
		perimeter = PARAM_GetNewperimeter();
		fSpeedTemp = (tempVal ? (3.6 / tempVal * perimeter) : 0);		
		LIMIT_TO_MAX(fSpeedTemp, 99.9);
		
		uartProtocolCB1.speedFilter.realSpeed = (uint16)(fSpeedTemp*10);		// ת��Ϊ0.1KM/H
	}

	// ����һ�ַ����˲�2017/12/04
	if (uartProtocolCB1.speedFilter.realSpeed >= uartProtocolCB1.speedFilter.proSpeed)
	{
		uartProtocolCB1.speedFilter.difSpeed = (uartProtocolCB1.speedFilter.realSpeed - uartProtocolCB1.speedFilter.proSpeed) / 5.0f;
	}
	else
	{
		uartProtocolCB1.speedFilter.difSpeed = (uartProtocolCB1.speedFilter.proSpeed -  uartProtocolCB1.speedFilter.realSpeed) / 5.0f;

		// ʵ���ٶ�Ϊ0����ֵҲΪ0����ʾ�ٶ�ҲӦ��Ϊ0������1km/h���µ�ֵ
		if ((0 == uartProtocolCB1.speedFilter.difSpeed) && (0 == uartProtocolCB1.speedFilter.realSpeed))
		{
			uartProtocolCB1.speedFilter.proSpeed = 0;
		}
	}

	if (BATTERY_DATA_SRC_CONTROLLER == PARAM_GetBatteryDataSrc())
	{
		// ﮵�2��û�е�ѹ�ϱ����ֶΣ���Ϊ�˼�����Ϊ���ù̶���ѹ
		tempVal = PARAM_GetBatteryVoltageLevel();
		tempVal *= 1000;											// ��λת��: 1Vת��Ϊ1000mV	
		
		PARAM_SetBatteryVoltage(tempVal);
	}
	else if (BATTERY_DATA_SRC_BMS == PARAM_GetBatteryDataSrc())
	{
		// ����
		PARAM_SetBatteryPercent(hmiDriveCB1.rx.param.batteryCapacity);
	}

}


// �ϱ�д���ò������
void UART_PROTOCOL1_ReportWriteParamResult(uint32 param)
{
	// ����豸��ַ
	UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_TX_DEVICE_ADDR);

	// ������ݳ��ȣ��̶�Ϊ5
	UART_PROTOCOL1_TxAddData(5);

	// ���������
	UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_WRITE_CONTROL_PARAM_RESULT);

	// д����
	UART_PROTOCOL1_TxAddData(param);

	// ��Ӽ���ͣ������������
	UART_PROTOCOL1_TxAddFrame();
}

// �ϱ����ò���
void UART_PROTOCOL1_ReportConfigureParam(uint32 param)
{
	// ����豸��ַ
	UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_TX_DEVICE_ADDR);

	// ������ݳ���
	UART_PROTOCOL1_TxAddData(27);

	// ���������
	UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_READ_CONTROL_PARAM_REPORT);

	// ��������
	UART_PROTOCOL1_TxAddData(PARAM_GetBrightness());

	// ����ʱ��
	UART_PROTOCOL1_TxAddData(PARAM_GetPowerOffTime());

	// ϵͳ��ѹ
	UART_PROTOCOL1_TxAddData(PARAM_GetBatteryVoltageLevel());

	// ������λ��
	UART_PROTOCOL1_TxAddData(PARAM_GetMaxAssist());

	// ��������
	UART_PROTOCOL1_TxAddData(PARAM_GetAssistDirection());

	// ������ʼ�Ÿ���
	UART_PROTOCOL1_TxAddData(PARAM_GetAssitStartOfSteelNum());

	// ��������
	UART_PROTOCOL1_TxAddData(PARAM_GetAssistPercent());

	// ת�ѷֵ�
	UART_PROTOCOL1_TxAddData(PARAM_GetTurnbarLevel());

	// ת������6kmph
	UART_PROTOCOL1_TxAddData(PARAM_GetTurnBarSpeed6kmphLimit());

	// ����������
	UART_PROTOCOL1_TxAddData(PARAM_GetSlowStart());

	// ���ٴŸ���
	UART_PROTOCOL1_TxAddData(PARAM_GetCycleOfSteelNum());

	// Ƿѹ����
	UART_PROTOCOL1_TxAddData(PARAM_GetLowVoltageThreshold()>>8);
	UART_PROTOCOL1_TxAddData(PARAM_GetLowVoltageThreshold()&0x00FF);

	// ��������
	UART_PROTOCOL1_TxAddData(PARAM_GetCurrentLimit()/1000);

	// ��������
	UART_PROTOCOL1_TxAddData(paramCB.nvm.param.common.speed.limitVal/10);

	// �־�����
	switch (PARAM_GetNewWheelSizeInch())
	{
		case 160:
			UART_PROTOCOL1_TxAddData(0);
			break;

		case 180:
			UART_PROTOCOL1_TxAddData(1);
			break;

		case 200:
			UART_PROTOCOL1_TxAddData(2);
			break;

		case 220:
			UART_PROTOCOL1_TxAddData(3);
			break;

		case 240:
			UART_PROTOCOL1_TxAddData(4);
			break;

		case 260:
			UART_PROTOCOL1_TxAddData(5);
			break;

		case 275:
			UART_PROTOCOL1_TxAddData(6);
			break;

		case 280:
			UART_PROTOCOL1_TxAddData(7);
			break;

		default:
			UART_PROTOCOL1_TxAddData(0);
			break;
	}
	
	// ������ʾ�仯ʱ��
	UART_PROTOCOL1_TxAddData(PARAM_GetBatteryCapVaryTime());

	// ���߹��ϳ�ʱʱ��
	UART_PROTOCOL1_TxAddData(PARAM_GetBusAliveTime()/1000);

	// �ٶ�ƽ���ȵȼ�
	UART_PROTOCOL1_TxAddData(PARAM_GetSpeedFilterLevel());

	// ��Ӣ�Ƶ�λ
	UART_PROTOCOL1_TxAddData(PARAM_GetUnit());
	
	// Ӳ���Ƿ�������
	UART_PROTOCOL1_TxAddData(PARAM_GetExistBle());
	
	// �ܳ�
	UART_PROTOCOL1_TxAddData(PARAM_GetNewperimeter()>>8);
	UART_PROTOCOL1_TxAddData(PARAM_GetNewperimeter()&0x00FF);

	// Э��
	UART_PROTOCOL1_TxAddData(PARAM_GetUartProtocol());

	// �������㷽ʽ
	UART_PROTOCOL1_TxAddData(PARAM_GetPercentageMethod());

	// �Ƴ���������
	UART_PROTOCOL1_TxAddData(PARAM_GetPushSpeedSwitch());
	
	// Ĭ�ϵ�λ
	UART_PROTOCOL1_TxAddData(PARAM_GetDefaultAssist());

	// LOGO��
	UART_PROTOCOL1_TxAddData(PARAM_GetlogoMenu());

	// ����ͨ�ŵ�ƽ
	UART_PROTOCOL1_TxAddData(PARAM_GetUartLevel());

	// ����������
	UART_PROTOCOL1_TxAddData(PARAM_GetBeepSwitch());

	// ���ٷ���������
	UART_PROTOCOL1_TxAddData(PARAM_GetLimitSpeedBeep());

	// ����Ѳ������
	UART_PROTOCOL1_TxAddData(PARAM_GetCruiseEnableSwitch());

	// ��������
	UART_PROTOCOL1_TxAddData(PARAM_GetPowerOnPassword() >> 8);
	UART_PROTOCOL1_TxAddData(PARAM_GetPowerOnPassword() & 0x00FF);

	// �˵�����
	UART_PROTOCOL1_TxAddData(PARAM_GetMenuPassword() >> 8);
	UART_PROTOCOL1_TxAddData(PARAM_GetMenuPassword() & 0x00FF);

	// �ָ���������
	UART_PROTOCOL1_TxAddData(PARAM_GetResFactorySet());
	
	// ��Ӽ���ͣ������������
	UART_PROTOCOL1_TxAddFrame();
}

// �����ϻ��ۼӴ���
void UART_PROTOCOL1_SendCmdAging(uint32 param)
{
	// ����豸��ַ
	UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_RX_DEVICE_ADDR);

	// ������ݳ��ȣ��̶�Ϊ20
	UART_PROTOCOL1_TxAddData(20);

	// ���������
	UART_PROTOCOL1_TxAddData(UART_PROTOCOL1_CMD_HMI_CTRL_RUN);

	// ���ٵ����ݼ���
	UART_PROTOCOL1_TxAddData(1);
	UART_PROTOCOL1_TxAddData(2);
	UART_PROTOCOL1_TxAddData(3);
	UART_PROTOCOL1_TxAddData(4);
	UART_PROTOCOL1_TxAddData(5);
	UART_PROTOCOL1_TxAddData(6);
	UART_PROTOCOL1_TxAddData(7);
	UART_PROTOCOL1_TxAddData(8);
	UART_PROTOCOL1_TxAddData(9);
	UART_PROTOCOL1_TxAddData(10);

	// ��Ӽ���ͣ������������
	UART_PROTOCOL1_TxAddFrame();
}
