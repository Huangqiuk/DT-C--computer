#include "common.h"
#include "timer.h"
#include "delay.h"
#include "uartDrive.h"
#include "uartProtocol.h"
#include "uartProtocol2.h"
#include "param.h"
#include "state.h"

/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void UART_PROTOCOL2_DataStructInit(UART_PROTOCOL2_CB* pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL2_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL2_RxFIFOProcess(UART_PROTOCOL2_CB* pCB);

// UART����֡����������
void UART_PROTOCOL2_CmdFrameProcess(UART_PROTOCOL2_CB* pCB);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL2_ConfirmTempCmdFrameBuff(UART_PROTOCOL2_CB* pCB);

// ͨѶ��ʱ����-����
void UART_PROTOCOL2_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void UART_PROTOCOL2_StopRxTimeOutCheck(void);

// Э��㷢�ʹ������
void UART_PROTOCOL2_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL2_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// ������������·����������־
void UART_PROTOCOL2_CALLBACK_SetTxPeriodRequest(uint32 param);

// ���������ͨѶʱ����
void UART_PROTOCOL2_DataCommunicationSequenceProcess(void);

// ����ͨѶ��ʱ�ж�����
void UART_PROTOCOL2_StartTimeoutCheckTask(void);

// UART���߳�ʱ������
void UART_PROTOCOL2_CALLBACK_UartBusError(uint32 param);

//=============================================================================
void UART_PROTOCOL2_ReadProtocolVersion(uint8 param);
void UART_PROTOCOL2_ReadDriverStatus(uint8 param);
void UART_PROTOCOL2_ReadSysWorkStatus(uint8 param);
void UART_PROTOCOL2_ReadInstantaneousCurrent(uint8 param);
void UART_PROTOCOL2_ReadBatteryCapacity(uint8 param);
void UART_PROTOCOL2_ReadRPM(uint8 param);
void UART_PROTOCOL2_ReadLightSwitch(uint8 param);
void UART_PROTOCOL2_ReadBatteryInfo1(uint8 param);
void UART_PROTOCOL2_ReadBatteryInfo2(uint8 param);
void UART_PROTOCOL2_ReadRemainderMileage(uint8 param);
void UART_PROTOCOL2_ReadDriverErrorCode(uint8 param);
void UART_PROTOCOL2_ReadCadence(uint8 param);
void UART_PROTOCOL2_ReadTQKG(uint8 param);
void UART_PROTOCOL2_WriteLimitSpeed(uint16 param);
void UART_PROTOCOL2_WriteAssistLevel(uint8 maxAssistLevel, uint8 assistLevel);
void UART_PROTOCOL2_WriteLightSwitch(uint8 param);
void UART_PROTOCOL2_WriteClearDriverErrorCode(uint8 param);
// �Ϳ��������ݽ������账��
void UART_PROTOCOL2_DataInteractionProcess(uint32 param);

// ȫ�ֱ�������
UART_PROTOCOL2_CB uartProtocolCB2;

// ��������������������������������������������������������������������������������������
// ����ͨѶ��ʱ�ж�����
void UART_PROTOCOL2_StartTimeoutCheckTask(void)
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
					UART_PROTOCOL2_CALLBACK_UartBusError,
					TRUE,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
#endif
}

// Э���ʼ��
void UART_PROTOCOL2_Init(void)
{
	// Э������ݽṹ��ʼ��
	UART_PROTOCOL2_DataStructInit(&uartProtocolCB2);

	// ��������ע�����ݽ��սӿ�
	UART_DRIVE_RegisterDataSendService(UART_PROTOCOL2_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	UART_PROTOCOL2_RegisterDataSendService(UART_DRIVE_AddTxArray);

	// ע������������ʱ��
	TIMER_AddTask(TIMER_ID_PROTOCOL_PARAM_TX,
					UART_PROTOCOL2_CMD_SEND_TIME,
					UART_PROTOCOL2_CALLBACK_SetTxPeriodRequest,
					TRUE,
					TIMER_LOOP_FOREVER,
					ACTION_MODE_ADD_TO_QUEUE);

	// ���ò�����������ڸ���ʱ��
	PARAM_SetParamCycleTime(UART_PROTOCOL2_CMD_SEND_TIME*7);	// 6������Ϊ1��ѭ��+1���յȴ�дʱ��

	// ����UART����ͨѶ��ʱ�ж�
	UART_PROTOCOL2_StartTimeoutCheckTask();
}

// UARTЭ�����̴���
void UART_PROTOCOL2_Process(void)
{
	// UART����FIFO����������
	UART_PROTOCOL2_RxFIFOProcess(&uartProtocolCB2);

	// UART���������������
	UART_PROTOCOL2_CmdFrameProcess(&uartProtocolCB2);
	
	// UARTЭ��㷢�ʹ������
	UART_PROTOCOL2_TxStateProcess();

	// ���������ͨѶʱ����
	UART_PROTOCOL2_DataCommunicationSequenceProcess();

//=========================================================
	// �ڰ˷�Э����Ƕ��KM5SЭ��
	UART_PROTOCOL_Process();
}

// ��������֡�������������
void UART_PROTOCOL2_TxAddData(uint8 data)
{
	uint16 head = uartProtocolCB2.tx.head;
	uint16 end =  uartProtocolCB2.tx.end;
	UART_PROTOCOL2_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB2.tx.cmdQueue[uartProtocolCB2.tx.end];

	// ���ͻ������������������
	if((end + 1) % UART_PROTOCOL2_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ��β����֡�������˳�
	if(pCmdFrame->length >= UART_PROTOCOL2_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void UART_PROTOCOL2_TxAddFrame(void)
{
	uint16 head = uartProtocolCB2.tx.head;
	uint16 end  = uartProtocolCB2.tx.end;
	UART_PROTOCOL2_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB2.tx.cmdQueue[uartProtocolCB2.tx.end];
	uint16 length = pCmdFrame->length;

	// ���ͻ������������������
	if((end + 1) % UART_PROTOCOL2_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ����֡���Ȳ��㣬������������ݣ��˳�
	if(length < UART_PROTOCOL2_TX_CMD_FRAME_LENGTH_MIN)
	{
		pCmdFrame->length = 0;
		
		return;
	}
	// ��β����֡�������˳�
	if(length > UART_PROTOCOL2_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}
	
	uartProtocolCB2.tx.end ++;
	uartProtocolCB2.tx.end %= UART_PROTOCOL2_TX_QUEUE_SIZE;
}

// ���ݽṹ��ʼ��
void UART_PROTOCOL2_DataStructInit(UART_PROTOCOL2_CB* pCB)
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
	for(i = 0; i < UART_PROTOCOL2_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<UART_PROTOCOL2_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL2_MacProcess(uint16 standarID, uint8* pData, uint16 length)
{
	uint16 end = uartProtocolCB2.rxFIFO.end;
	uint16 head = uartProtocolCB2.rxFIFO.head;
	uint8 rxdata = 0x00;
	
	// ��������
	rxdata = *pData;

	// һ���������������������
	if((end + 1)%UART_PROTOCOL2_RX_FIFO_SIZE == head)
	{
		return;
	}
	// һ��������δ�������� 
	else
	{
		// �����յ������ݷŵ���ʱ��������
		uartProtocolCB2.rxFIFO.buff[end] = rxdata;
		uartProtocolCB2.rxFIFO.end ++;
		uartProtocolCB2.rxFIFO.end %= UART_PROTOCOL2_RX_FIFO_SIZE;
	}	

//====================================================================
	// ����KM5SЭ�����
	UART_PROTOCOL_MacProcess(standarID, pData, length);
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL2_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{		
	uartProtocolCB2.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL2_ConfirmTempCmdFrameBuff(UART_PROTOCOL2_CB* pCB)
{
	UART_PROTOCOL2_RX_CMD_FRAME* pCmdFrame = NULL;
	
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
	pCB->rx.end %= UART_PROTOCOL2_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
	
	return TRUE;
}

// Э��㷢�ʹ������
void UART_PROTOCOL2_TxStateProcess(void)
{
	uint16 head = uartProtocolCB2.tx.head;
	uint16 end =  uartProtocolCB2.tx.end;
	uint16 length = uartProtocolCB2.tx.cmdQueue[head].length;
	uint8* pCmd = uartProtocolCB2.tx.cmdQueue[head].buff;
	uint16 localDeviceID = uartProtocolCB2.tx.cmdQueue[head].deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == uartProtocolCB2.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������		
	if (!(*uartProtocolCB2.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	uartProtocolCB2.tx.cmdQueue[head].length = 0;
	uartProtocolCB2.tx.head ++;
	uartProtocolCB2.tx.head %= UART_PROTOCOL2_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL2_RxFIFOProcess(UART_PROTOCOL2_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	UART_PROTOCOL2_RX_CMD_FRAME* pCmdFrame = NULL;
	
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

	// ����ʱ���������ã��˳�
	if((pCB->rx.end + 1)%UART_PROTOCOL2_RX_QUEUE_SIZE == pCB->rx.head)
	{
		return;
	}
	
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];

	// ��������ӵ���ʱ��������
	pCmdFrame->buff[pCmdFrame->length++] = pCB->rxFIFO.buff[head];

	// ���ڽ�������
	uartProtocolCB2.rxFIFO.rxFIFOBusy = TRUE;

	// ��ӳɹ�����һ����������ɾ����ǰ����
	pCB->rxFIFO.head ++;
	pCB->rxFIFO.head %= UART_PROTOCOL2_RX_FIFO_SIZE;

	// ���յ��������Ƿ�ﵽ����������
	if(pCmdFrame->length < pCB->rx.expectLength)
	{
		return;
	}

	// �����Ѿ��������
	uartProtocolCB2.rxFIFO.rxFIFOBusy = FALSE;

	// ���ݳ����Ѿ����㣬˵�����յ���һ������������ȷ������֡����ʱ��Ҫ�������
	UART_PROTOCOL2_ConfirmTempCmdFrameBuff(pCB);
}

// UART����֡����������
void UART_PROTOCOL2_CmdFrameProcess(UART_PROTOCOL2_CB* pCB)
{
	UART_PROTOCOL2_CMD cmd = UART_PROTOCOL2_CMD_NULL;
	UART_PROTOCOL2_RX_CMD_FRAME* pCmdFrame = NULL;
	//uint16 i;
	uint32 temp;

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

	cmd = pCB->cmdType;
	
	// ִ������֡
	switch(cmd)
	{
		// ���������ִ��
		case UART_PROTOCOL2_CMD_NULL:
			break;

		// ��Э��汾
		case UART_PROTOCOL2_CMD_READ_PROTOCOL_VERSION:
			break;

		// д����
		case UART_PROTOCOL2_CMD_WRITE_LIMIT_SPEED:
			break;
//============================================================================================
//============================================================================================
		// ��������״̬
		case UART_PROTOCOL2_CMD_READ_DRIVER_STATUS:

			// �κ����ε�ɲ������
			if (0x03 == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX])
			{
				break;
			}
			
			PARAM_SetErrorCode((ERROR_TYPE_E)pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX]);

			// Ϊ��ͨ�ø���Э�飬���˷����޴���0x01ת��0x00
			if (0x01 == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX])
			{
				
				PARAM_SetErrorCode(ERROR_TYPE_NO_ERROR);
			}
			
			break;

		// ������״̬
		case UART_PROTOCOL2_CMD_READ_WORK_STATUS:
			if (pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX] == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX])
			{
				// ������״̬
				if (0x30 == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX])
				{
					//PARAM_SetCruiseSwitch(FALSE);
				}
				// ����״̬
				else if (0x31 == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX])
				{
					//PARAM_SetCruiseSwitch(TRUE);
				}
			}			
			break;

		// ��˲ʱ����
		case UART_PROTOCOL2_CMD_READ_INSTANTANEOUS_CURRENT:
			if (pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX] == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX])
			{
				temp = pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX] * 0.5f * 1000;	// �ϱ��ĵ�λΪ0.5A
				PARAM_SetBatteryCurrent(temp);
			}

			break;

		// ���������
		case UART_PROTOCOL2_CMD_READ_BATTERY_CAPACITY:
			//if (pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX] == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX])
			//{
			//	PARAM_SetBatteryPercent(pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX]);
			//}

			if (BATTERY_DATA_SRC_CONTROLLER == PARAM_GetBatteryDataSrc())
			{
				// û�е�ѹ�ϱ����ֶΣ���Ϊ�˼�����Ϊ���ù̶���ѹ
				temp = PARAM_GetBatteryVoltageLevel();
				temp *= 1000;											// ��λת��: 1Vת��Ϊ1000mV	
				
				PARAM_SetBatteryVoltage(temp);
			}
			else if (BATTERY_DATA_SRC_BMS == PARAM_GetBatteryDataSrc())
			{
				// ����
				if (pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX] == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX])
				{
					PARAM_SetBatteryPercent(pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX]);
				}
			}
			
			break;

		// ��ת��
		case UART_PROTOCOL2_CMD_READ_RPM:
			if (((pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX] + pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX] + 0x20)&0xFF) == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA3_INDEX])
			{
				double speed;
				
				temp = pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX];
				temp <<= 8;
				temp += pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX];

				//speed = 0.00006f * temp * PARAM_GetPerimeter(PARAM_GetWheelSizeID());
				speed = 0.00006f * temp * PARAM_GetNewperimeter();
				LIMIT_TO_MAX(speed, 99.9);
				PARAM_SetSpeed((uint16)(speed*10));	// ת��Ϊ0.1KM/H�����뵽������

				// �ٶȲ�Ϊ0�����ö�ʱ�ػ�����
				if(PARAM_GetSpeed() != 0)
				{
					STATE_ResetAutoPowerOffControl();
				}
			}

			// ���������־��
			PARAM_SetRecaculateRequest(TRUE);
			
			break;

		// ����ƿ���ֵ
		case UART_PROTOCOL2_CMD_READ_LIGHT_SWITCH:
			break;
		// �������Ϣ1
		case UART_PROTOCOL2_CMD_READ_BATTERY_INFO1:
			break;
		// �������Ϣ2
		case UART_PROTOCOL2_CMD_READ_BATTERY_INFO2:
			break;

		// ��ʣ�����
		case UART_PROTOCOL2_CMD_READ_REMAINDER_MILEAGE:
			if (pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX] + pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX] == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA3_INDEX])
			{
				temp = pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX];
				temp <<= 8;
				temp += pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX];

				#warning "��������Ҫ����ʣ����̱���"
			}						
			
			break;

		// ���������������
		case UART_PROTOCOL2_CMD_READ_DRIVER_ERROR_CODE:
			break;

		// ��̤Ƶ
		case UART_PROTOCOL2_CMD_READ_CADENCE:
			if (pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX] + pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX] == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA3_INDEX])
			{
				temp = pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX];
				temp <<= 8;
				temp += pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX];
				
				PARAM_SetCadence(temp);
			}
			break;

		// ����̤��Ȧ�������ֵ
		case UART_PROTOCOL2_CMD_READ_TQKG:
			if (pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX] == pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA2_INDEX])
			{
				PARAM_SetTorque(pCmdFrame->buff[UART_PROTOCOL2_CMD_DATA1_INDEX]);
			}
			break;
		
//===================================================
//===================================================
		// д��λ
		case UART_PROTOCOL2_CMD_WRITE_ASSIST_LEVEL:
			break;

		// д��ƿ���
		case UART_PROTOCOL2_CMD_WRITE_LIGHT_SWITCH:
			break;

		// д���������������
		case UART_PROTOCOL2_CMD_WRITE_CLEAR_DRIVER_ERROR_CODE:
			break;
					
		default:
			break;
	}

	// ����UART����ͨѶ��ʱ�ж�
	UART_PROTOCOL2_StartTimeoutCheckTask();
	
	// ɾ������֡
	pCB->rx.head ++;
	pCB->rx.head %= UART_PROTOCOL2_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if UART_PROTOCOL2_RX_TIME_OUT_CHECK_ENABLE
void UART_PROTOCOL2_CALLBACK_RxTimeOut(uint32 param)
{
	UART_PROTOCOL2_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0;	// 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	uartProtocolCB2.rxFIFO.head ++;
	uartProtocolCB2.rxFIFO.head %= UART_PROTOCOL2_RX_FIFO_SIZE;
	uartProtocolCB2.rxFIFO.currentProcessIndex = uartProtocolCB2.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void UART_PROTOCOL2_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

// ������������·����������־
void UART_PROTOCOL2_SetTxAtOnceRequest(uint32 param)
{
	uartProtocolCB2.txAtOnceRequest = (BOOL)param;
}

// ������������·���������
void UART_PROTOCOL2_SetTxAtOnceCmdType(UART_PROTOCOL2_TX_AT_ONCE_CMD_TYPE type)
{
	uartProtocolCB2.txAtOnceCmdType = type;
}

// ������������·����������־
void UART_PROTOCOL2_CALLBACK_SetTxPeriodRequest(uint32 param)
{
	uartProtocolCB2.txPeriodRequest = (BOOL)param;
}

// 	UART���߳�ʱ������
void UART_PROTOCOL2_CALLBACK_UartBusError(uint32 param)
{
	PARAM_SetErrorCode(ERROR_TYPE_COMMUNICATION_TIME_OUT);
}

// ���������ͨѶʱ����
void UART_PROTOCOL2_DataCommunicationSequenceProcess(void)
{
	// ������ڽ������ݣ���������;���
	//if (uartProtocolCB2.rxFIFO.rxFIFOBusy)
	//{
	//	return;
	//}
	
	// ������������
	if (uartProtocolCB2.txAtOnceRequest)
	{
		// ���¸�λ�����·���ʱ��
		TIMER_ResetTimer(TIMER_ID_PROTOCOL_PARAM_TX);

		uartProtocolCB2.txAtOnceRequest = FALSE;
#if 0
		if (UART_PROTOCOL2_TX_CMD_TYPE_ASSIST == uartProtocolCB2.txAtOnceCmdType)
		{
			// д��λ
			UART_PROTOCOL2_WriteAssistLevel(PARAM_GetMaxAssist(), PARAM_GetAssistLevel());
		}
		else if (UART_PROTOCOL2_TX_CMD_TYPE_LIGHT == uartProtocolCB2.txAtOnceCmdType)
		{
			// д���
			UART_PROTOCOL2_WriteLightSwitch(PARAM_GetLightSwitch());
		}
#endif
		// д��λ
		UART_PROTOCOL2_WriteAssistLevel(PARAM_GetMaxAssist(), PARAM_GetAssistLevel());
		// д���״̬
		UART_PROTOCOL2_WriteLightSwitch(PARAM_GetLightSwitch());
		// д����
		UART_PROTOCOL2_WriteLimitSpeed(paramCB.nvm.param.common.speed.limitVal);

		return;
	}

	// �����Է�������
	if (uartProtocolCB2.txPeriodRequest)
	{
		uartProtocolCB2.txPeriodRequest = FALSE;

		UART_PROTOCOL2_DataInteractionProcess(TRUE);		
	}	
}

// �Ϳ��������ݽ������账��
void UART_PROTOCOL2_DataInteractionProcess(uint32 param)
{
	switch (uartProtocolCB2.step)
	{
		case 0:
			// ��ȡЭ��汾
			UART_PROTOCOL2_ReadProtocolVersion(TRUE);

			uartProtocolCB2.step ++;
			
			break;

		case 1:			
			// д��λ
			UART_PROTOCOL2_WriteAssistLevel(PARAM_GetMaxAssist(), PARAM_GetAssistLevel());
			// д���״̬
			UART_PROTOCOL2_WriteLightSwitch(PARAM_GetLightSwitch());
			// д����
			UART_PROTOCOL2_WriteLimitSpeed(paramCB.nvm.param.common.speed.limitVal);

			uartProtocolCB2.step ++;
			
			break;

		case 2:			
			// �������ʱ�䣬��д��λ��������г�ֵ�ʱ�䣬��ֹ�������쳣
			uartProtocolCB2.step ++;
			
			break;

		case 3:
			// ��ȡת��
			UART_PROTOCOL2_ReadRPM(TRUE);

			uartProtocolCB2.step ++;
			
			break;

		case 4:
			// ��������״̬
			UART_PROTOCOL2_ReadDriverStatus(TRUE);

			uartProtocolCB2.step ++;
			
			break;

		case 5:
			// ��ȡ�������
			UART_PROTOCOL2_ReadBatteryCapacity(TRUE);

			uartProtocolCB2.step ++;
			
			break;
			
		case 6:
			// ��ϵͳ����״̬
			UART_PROTOCOL2_ReadSysWorkStatus(TRUE);
		
			uartProtocolCB2.step ++;
			
			break;
			

		case 7:
			// ��˲ʱ����
			UART_PROTOCOL2_ReadInstantaneousCurrent(TRUE);

			uartProtocolCB2.step = 1;
			
			break;

		default:
			uartProtocolCB2.step = 1;
			break;	
	}
}

// ��ȡЭ��汾
void UART_PROTOCOL2_ReadProtocolVersion(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_PROTOCOL_VERSION;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 3;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_PROTOCOL_VERSION&0xFF00)>>8);

	// ����Ϊ��Э��汾
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_PROTOCOL_VERSION&0x00FF);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
	
}

// ��ȡ������״̬
void UART_PROTOCOL2_ReadDriverStatus(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_DRIVER_STATUS;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 1;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_DRIVER_STATUS&0xFF00)>>8);

	// ����Ϊ��������״̬
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_DRIVER_STATUS&0x00FF);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡϵͳ����״̬
void UART_PROTOCOL2_ReadSysWorkStatus(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_WORK_STATUS;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 2;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_WORK_STATUS&0xFF00)>>8);

	// ����Ϊ��ϵͳ����״̬
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_WORK_STATUS&0x00FF);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡ˲ʱ����
void UART_PROTOCOL2_ReadInstantaneousCurrent(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_INSTANTANEOUS_CURRENT;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 2;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_INSTANTANEOUS_CURRENT&0xFF00)>>8);

	// ����Ϊ��˲ʱ����
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_INSTANTANEOUS_CURRENT&0x00FF);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡ�������
void UART_PROTOCOL2_ReadBatteryCapacity(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_BATTERY_CAPACITY;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 2;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_BATTERY_CAPACITY&0xFF00)>>8);

	// ����Ϊ���������
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_BATTERY_CAPACITY&0x00FF);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡ����ת��
void UART_PROTOCOL2_ReadRPM(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_RPM;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 3;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_RPM&0xFF00)>>8);

	// ����Ϊ��ȡ����ת��
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_RPM&0x00FF);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡ���ֵ
void UART_PROTOCOL2_ReadLightSwitch(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_LIGHT_SWITCH;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 2;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_LIGHT_SWITCH&0xFF00)>>8);

	// ����Ϊ��ȡ���ֵ
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_LIGHT_SWITCH&0x00FF);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡ�����Ϣ1
void UART_PROTOCOL2_ReadBatteryInfo1(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_BATTERY_INFO1;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 27;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_BATTERY_INFO1&0xFF00)>>8);

	// ����Ϊ��ȡ�����Ϣ1
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_BATTERY_INFO1&0x00FF);

	// У���
	UART_PROTOCOL2_TxAddData(((UART_PROTOCOL2_CMD_READ_BATTERY_INFO1&0xFF00)>>8)+(UART_PROTOCOL2_CMD_READ_BATTERY_INFO1&0x00FF));

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡ�����Ϣ2
void UART_PROTOCOL2_ReadBatteryInfo2(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_BATTERY_INFO2;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = (13+1)*2;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_BATTERY_INFO2&0xFF00)>>8);

	// ����Ϊ��ȡ�����Ϣ2
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_BATTERY_INFO2&0x00FF);

	// У���
	UART_PROTOCOL2_TxAddData(((UART_PROTOCOL2_CMD_READ_BATTERY_INFO2&0xFF00)>>8)+(UART_PROTOCOL2_CMD_READ_BATTERY_INFO2&0x00FF));

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡʣ�����
void UART_PROTOCOL2_ReadRemainderMileage(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_REMAINDER_MILEAGE;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 3;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_REMAINDER_MILEAGE&0xFF00)>>8);

	// ����Ϊ��ȡʣ�����
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_REMAINDER_MILEAGE&0x00FF);

	// У���
	UART_PROTOCOL2_TxAddData(((UART_PROTOCOL2_CMD_READ_REMAINDER_MILEAGE&0xFF00)>>8)+(UART_PROTOCOL2_CMD_READ_REMAINDER_MILEAGE&0x00FF));

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡ������������
void UART_PROTOCOL2_ReadDriverErrorCode(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_DRIVER_ERROR_CODE;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 11;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_DRIVER_ERROR_CODE&0xFF00)>>8);

	// ����Ϊ��ȡ������������
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_DRIVER_ERROR_CODE&0x00FF);

	// У���
	UART_PROTOCOL2_TxAddData(((UART_PROTOCOL2_CMD_READ_DRIVER_ERROR_CODE&0xFF00)>>8)+(UART_PROTOCOL2_CMD_READ_DRIVER_ERROR_CODE&0x00FF));

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡ̤Ƶ
void UART_PROTOCOL2_ReadCadence(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_CADENCE;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 3;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_CADENCE&0xFF00)>>8);

	// ����Ϊ��ȡ̤Ƶ
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_CADENCE&0x00FF);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// ��ȡ��̤��Ȧ�������ֵ����
void UART_PROTOCOL2_ReadTQKG(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_READ_TQKG;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 2;
	
	// ��ȡָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_READ_TQKG&0xFF00)>>8);

	// ����Ϊ��ȡ��̤��Ȧ�������ֵ����
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_READ_TQKG&0x00FF);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

//============================================================
// д����
void UART_PROTOCOL2_WriteLimitSpeed(uint16 param)
{
	uint8 checkVal;
	//uint16 perimeter = PARAM_GetPerimeter(PARAM_GetWheelSizeID());
	uint16 perimeter = PARAM_GetNewperimeter();
	uint16 speedRpm = (uint16)(param*100000.0f/perimeter/60 + 0.5);

	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_WRITE_LIMIT_SPEED;
	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 0;
	
	// дָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_WRITE_LIMIT_SPEED&0xFF00)>>8);

	// ����Ϊд����
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_WRITE_LIMIT_SPEED&0x00FF);

	UART_PROTOCOL2_TxAddData(speedRpm>>8);
	UART_PROTOCOL2_TxAddData(speedRpm&0x00FF);

	checkVal = ((UART_PROTOCOL2_CMD_WRITE_LIMIT_SPEED&0xFF00)>>8) + (UART_PROTOCOL2_CMD_WRITE_LIMIT_SPEED&0x00FF) + (speedRpm>>8) + (speedRpm&0x00FF); 
	
	// У���
	UART_PROTOCOL2_TxAddData(checkVal);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// д��λ
void UART_PROTOCOL2_WriteAssistLevel(uint8 maxAssistLevel, uint8 assistLevel)
{
	uint8 assistCode;
	uint8 checkVal;
	
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_WRITE_ASSIST_LEVEL;
	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 0;
	
	// дָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_WRITE_ASSIST_LEVEL&0xFF00)>>8);

	// ����Ϊд��λ
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_WRITE_ASSIST_LEVEL&0x00FF);

	switch (maxAssistLevel)
	{
		case 3:
			switch (assistLevel)
			{
				case ASSIST_0:
					assistCode = 0;
					break;

				case ASSIST_1:
					assistCode = 12;
					break;

				case ASSIST_2:
					assistCode = 2;
					break;

				case ASSIST_3:
					assistCode = 3;
					break;

				// �Ƴ�
				case ASSIST_P:
					assistCode = 6;
					break;

				// ����
				case ASSIST_S:
					assistCode = 10;
					break;
			}
			break;

		case 5:
			switch (assistLevel)
			{
				case ASSIST_0:
					assistCode = 0;
					break;

				case ASSIST_1:
					assistCode = 11;
					break;

				case ASSIST_2:
					assistCode = 13;
					break;

				case ASSIST_3:
					assistCode = 21;
					break;

				case ASSIST_4:
					assistCode = 23;
					break;

				case ASSIST_5:
					assistCode = 3;
					break;

				// �Ƴ�
				case ASSIST_P:
					assistCode = 6;
					break;

				// ����
				case ASSIST_S:
					assistCode = 10;
					break;
			}
			break;

		case 9:
			switch (assistLevel)
			{
				case ASSIST_0:
					assistCode = 0;
					break;

				case ASSIST_1:
					assistCode = 1;
					break;

				case ASSIST_2:
					assistCode = 11;
					break;

				case ASSIST_3:
					assistCode = 12;
					break;

				case ASSIST_4:
					assistCode = 13;
					break;

				case ASSIST_5:
					assistCode = 2;
					break;

				case ASSIST_6:
					assistCode = 21;
					break;

				case ASSIST_7:
					assistCode = 22;
					break;

				case ASSIST_8:
					assistCode = 23;
					break;

				case ASSIST_9:
					assistCode = 3;
					break;

				// �Ƴ�
				case ASSIST_P:
					assistCode = 6;
					break;

				// ����
				case ASSIST_S:
					assistCode = 10;
					break;
			}
			break;
	}
	UART_PROTOCOL2_TxAddData(assistCode);

	checkVal = ((UART_PROTOCOL2_CMD_WRITE_ASSIST_LEVEL&0xFF00)>>8) + (UART_PROTOCOL2_CMD_WRITE_ASSIST_LEVEL&0x00FF) + assistCode; 
	
	// У���
	UART_PROTOCOL2_TxAddData(checkVal);

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// д���
void UART_PROTOCOL2_WriteLightSwitch(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_WRITE_LIGHT_SWITCH;
	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 0;
	
	// дָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_WRITE_LIGHT_SWITCH&0xFF00)>>8);

	// ����Ϊд���
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_WRITE_LIGHT_SWITCH&0x00FF);

	if (param)
	{
		UART_PROTOCOL2_TxAddData(0xF1);	// �����
	}
	else
	{
		UART_PROTOCOL2_TxAddData(0xF0);	// �ش��
	}

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

// д���������������
void UART_PROTOCOL2_WriteClearDriverErrorCode(uint8 param)
{
	// ������������
	uartProtocolCB2.cmdType = UART_PROTOCOL2_CMD_WRITE_CLEAR_DRIVER_ERROR_CODE;

	// ���ý������ݳ���
	uartProtocolCB2.rx.expectLength = 2;
	
	
	// дָ��
	UART_PROTOCOL2_TxAddData((UART_PROTOCOL2_CMD_WRITE_CLEAR_DRIVER_ERROR_CODE&0xFF00)>>8);

	// ����Ϊд���������������
	UART_PROTOCOL2_TxAddData(UART_PROTOCOL2_CMD_WRITE_CLEAR_DRIVER_ERROR_CODE&0x00FF);

	// У���
	UART_PROTOCOL2_TxAddData(((UART_PROTOCOL2_CMD_WRITE_CLEAR_DRIVER_ERROR_CODE&0xFF00)>>8) + (UART_PROTOCOL2_CMD_WRITE_CLEAR_DRIVER_ERROR_CODE&0x00FF));

	// �����������֡
	UART_PROTOCOL2_TxAddFrame();


	// ���һ�����ն����Լ�����֡����
	uartProtocolCB2.rxFIFO.head = uartProtocolCB2.rxFIFO.end;
	uartProtocolCB2.rx.head = uartProtocolCB2.rx.end;
	uartProtocolCB2.rx.cmdQueue[uartProtocolCB2.rx.end].length = 0;
}

