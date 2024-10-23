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
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void BLE_HOST_PROTOCOL_DataStructInit(BLE_HOST_PROTOCOL_CB* pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void BLE_HOST_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void BLE_HOST_PROTOCOL_RxFIFOProcess(BLE_HOST_PROTOCOL_CB* pCB);

// UART����֡����������
void BLE_HOST_PROTOCOL_CmdFrameProcess(BLE_HOST_PROTOCOL_CB* pCB);

// �Դ��������֡����У�飬����У����
BOOL BLE_HOST_PROTOCOL_CheckSUM(BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL BLE_HOST_PROTOCOL_ConfirmTempCmdFrameBuff(BLE_HOST_PROTOCOL_CB* pCB);

// ͨѶ��ʱ����-����
void BLE_HOST_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void BLE_HOST_PROTOCOL_StopRxTimeOutCheck(void);

// TXRXͨѶ��ʱ����-˫��
void BLE_HOST_PROTOCOL_CALLBACK_TxRxTimeOut(uint32 param);

// ֹͣTXRXͨѶ��ʱ�������
void BLE_HOST_PROTOCOL_StopTxRxTimeOutCheck(void);

// Э��㷢�ʹ������
void BLE_HOST_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void BLE_HOST_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// ����ͨѶ��ʱ�ж�����
void BLE_HOST_PROTOCOL_StartTimeoutCheckTask(void);

// UART���߳�ʱ������
void BLE_HOST_PROTOCOL_CALLBACK_UartBusError(uint32 param);

// ״̬Ǩ��
void BLE_PairEnterState(uint32 state);

//=======================================================================================
// ȫ�ֱ�������
BLE_HOST_PROTOCOL_CB bleHostProtocolCB;
extern uint8 callringBuff[20];

// ��������������������������������������������������������������������������������������
// ����ͨѶ��ʱ�ж�����
void BLE_HOST_PROTOCOL_StartTimeoutCheckTask(void)
{
}


// Э���ʼ��
void BLE_HOST_PROTOCOL_Init(void)
{
	// Э������ݽṹ��ʼ��
	BLE_HOST_PROTOCOL_DataStructInit(&bleHostProtocolCB);

	// ��������ע�����ݽ��սӿ�
	BLE_HOST_UART_RegisterDataSendService(BLE_HOST_PROTOCOL_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	BLE_HOST_PROTOCOL_RegisterDataSendService(BLE_HOST_UART_AddTxArray);

	// �ϵ����������Գ�ʼ��
	BLE_PairEnterState(BLE_PAIR_INIT);	
}

// BLEЭ�����̴���
void BLE_HOST_PROTOCOL_Process(void)
{
	// BLE����FIFO����������
	BLE_HOST_PROTOCOL_RxFIFOProcess(&bleHostProtocolCB);

	// BLE���������������
	BLE_HOST_PROTOCOL_CmdFrameProcess(&bleHostProtocolCB);
	
	// BLEЭ��㷢�ʹ������
	BLE_HOST_PROTOCOL_TxStateProcess();
	
	// ������Դ������
	BLE_PairProcess();
}

// ��������֡�������������
void BLE_HOST_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = bleHostProtocolCB.tx.head;
	uint16 end =  bleHostProtocolCB.tx.end;
	BLE_HOST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &bleHostProtocolCB.tx.cmdQueue[bleHostProtocolCB.tx.end];

	// ���ͻ������������������
	if((end + 1) % BLE_HOST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ��β����֡�������˳�
	if(pCmdFrame->length >= BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void BLE_HOST_PROTOCOL_TxAddFrame(void)
{
	uint16 cc = 0;
	uint16 i = 0;
	uint16 head = bleHostProtocolCB.tx.head;
	uint16 end  = bleHostProtocolCB.tx.end;
	BLE_HOST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &bleHostProtocolCB.tx.cmdQueue[bleHostProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// ���ͻ������������������
	if((end + 1) % BLE_HOST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ����֡���Ȳ��㣬������������ݣ��˳�
	if(BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN-1 > length)	// ��ȥ"У���"1���ֽ�
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// ��β����֡�������˳�
	if(length >= BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_LENGTH_INDEX] = length - 3;	// �������ݳ���

	for(i=0; i<length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = ~cc ;

	bleHostProtocolCB.tx.end ++;
	bleHostProtocolCB.tx.end %= BLE_HOST_PROTOCOL_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���TTM����
void BLE_HOST_PROTOCOL_TxAddTtmCmd(uint8 data[], uint16 length, uint16 id)
{
	uint16 i = 0;
	uint16 head = bleHostProtocolCB.ttmTx.head;
	uint16 end  = bleHostProtocolCB.ttmTx.end;
	BLE_HOST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &bleHostProtocolCB.ttmTx.cmdQueue[bleHostProtocolCB.ttmTx.end];

	// ���ͻ������������������
	if((end + 1) % BLE_HOST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ��β����֡�������˳�
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

// ���ݽṹ��ʼ��
void BLE_HOST_PROTOCOL_DataStructInit(BLE_HOST_PROTOCOL_CB* pCB)
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

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void BLE_HOST_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length)
{
	uint16 end = bleHostProtocolCB.rxFIFO.end;
	uint16 head = bleHostProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;
	
	// ��������
	rxdata = *pData;

	// һ���������������������
	if((end + 1)%BLE_HOST_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// һ��������δ�������� 
	else
	{
		// �����յ������ݷŵ���ʱ��������
		bleHostProtocolCB.rxFIFO.buff[end] = rxdata;
		bleHostProtocolCB.rxFIFO.end ++;
		bleHostProtocolCB.rxFIFO.end %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
	}

	// Ƕһ��Э��
	BLE_PROTOCOL1_MacProcess(standarID, pData, length);
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void BLE_HOST_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{		
	bleHostProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL BLE_HOST_PROTOCOL_ConfirmTempCmdFrameBuff(BLE_HOST_PROTOCOL_CB* pCB)
{
	BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
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
	pCB->rx.end %= BLE_HOST_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
	
	return TRUE;
}

// Э��㷢�ʹ������
void BLE_HOST_PROTOCOL_TxStateProcess(void)
{
	uint16 head = bleHostProtocolCB.tx.head;
	uint16 end =  bleHostProtocolCB.tx.end;
	uint16 length = bleHostProtocolCB.tx.cmdQueue[head].length;
	uint8* pCmd = bleHostProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = bleHostProtocolCB.tx.cmdQueue[head].deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == bleHostProtocolCB.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������		
	if (!(*bleHostProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	bleHostProtocolCB.tx.cmdQueue[head].length = 0;
	bleHostProtocolCB.tx.head ++;
	bleHostProtocolCB.tx.head %= BLE_HOST_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void BLE_HOST_PROTOCOL_RxFIFOProcess(BLE_HOST_PROTOCOL_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
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
		if(BLE_HOST_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if((pCB->rx.end + 1)%BLE_HOST_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// ���UARTͨѶ��ʱʱ������-2016.1.5����
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
						BLE_HOST_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
						BLE_HOST_PROTOCOL_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if(pCmdFrame->length >= BLE_HOST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			BLE_HOST_PROTOCOL_StopRxTimeOutCheck();
#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0;	// 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
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
			pCB->rxFIFO.currentProcessIndex %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ���������� ����
						
			// �����ж�����֡��С���ȣ�һ�����������������ٰ���4���ֽ�: ����֡��С���ȣ�����:����ͷ�������֡����ݳ��ȡ�У��ͣ���˲���4���ֽڵıض�������
			if(pCmdFrame->length < BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
			if(pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_LENGTH_INDEX] > (BLE_HOST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN))
			{
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				BLE_HOST_PROTOCOL_StopRxTimeOutCheck();
#endif
			
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// ����֡����У��
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_LENGTH_INDEX] + BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if(!BLE_HOST_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				BLE_HOST_PROTOCOL_StopRxTimeOutCheck();
#endif
				
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			BLE_HOST_PROTOCOL_StopRxTimeOutCheck();
#endif
			
			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			BLE_HOST_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// �Դ��������֡����У�飬����У����
BOOL BLE_HOST_PROTOCOL_CheckSUM(BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame)
{
	uint8 cc = 0;
	uint16 i = 0;
	
	if(NULL == pCmdFrame)
	{
		return FALSE;
	}

	// ������ͷ��ʼ����У����֮ǰ��һ���ֽڣ����ν����������
	for(i=0; i<pCmdFrame->length-1; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}

	cc = ~cc;
	
	// �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
	if(pCmdFrame->buff[pCmdFrame->length-1] != cc)
	{
		return FALSE;
	}
	
	return TRUE;
}

// BLE����֡����������
void BLE_HOST_PROTOCOL_CmdFrameProcess(BLE_HOST_PROTOCOL_CB* pCB)
{
	BLE_HOST_PROTOCOL_CMD cmd = BLE_HOST_CMD_MAX;
	BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	uint8 i = 0;
	
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
	if(BLE_HOST_PROTOCOL_CMD_HEAD != pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_HEAD_INDEX])
	{
		// ɾ������֡
		pCB->rx.head ++;
		pCB->rx.head %= BLE_HOST_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (BLE_HOST_PROTOCOL_CMD)pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_CMD_INDEX];
	
	// ִ������֡
	switch (cmd)
	{
		// ͷ����������
		case BLE_OUTFIT_PROTOCOL_CMD_HELMET:

			// ת���
			if (0x01 == pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA1_INDEX])
			{
				PARAM_SetTurnState((TRUN_STATE)pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA3_INDEX]);
			}

			break;

		// ͷ��״̬
		case BLE_OUTFIT_PROTOCOL_CMD_HELMET_STATE:
			
			// ͷ�������ٷֱȻ�ȡ
			if (0x01 == pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA1_INDEX])
			{
				PARAM_SetHelmetBattery(pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA2_INDEX]);
			}
			// ����״̬,����ֵ
			else if (0x02 == pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA1_INDEX])
			{
				paramCB.runtime.musicState = pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA2_INDEX];
				paramCB.runtime.musicVolume = pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA4_INDEX];				
			}
	
			break;
			
		// ͷ��������������
		case BLE_OUTFIT_PROTOCOL_CMD_HELMET_PHONE:

			PARAM_SetHelmetCaller(pCmdFrame->buff[BLE_HOST_PROTOCOL_CMD_DATA1_INDEX]);

			// �������߹Ҷ�������Ա�ʶ
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

	// ����UART����ͨѶ��ʱ�ж�
	BLE_HOST_PROTOCOL_StartTimeoutCheckTask();
	
	// ɾ������֡
	pCB->rx.head ++;
	pCB->rx.head %= BLE_HOST_PROTOCOL_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void BLE_HOST_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	BLE_HOST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &bleHostProtocolCB.rx.cmdQueue[bleHostProtocolCB.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0;	// 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	bleHostProtocolCB.rxFIFO.head ++;
	bleHostProtocolCB.rxFIFO.head %= BLE_HOST_PROTOCOL_RX_FIFO_SIZE;
	bleHostProtocolCB.rxFIFO.currentProcessIndex = bleHostProtocolCB.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void BLE_HOST_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

#if BLE_HOST_PROTOCOL_TXRX_TIME_OUT_CHECK_ENABLE
// TXRXͨѶ��ʱ����-˫��
void BLE_HOST_PROTOCOL_CALLBACK_TxRxTimeOut(uint32 param)
{
	
}

// ֹͣTxRXͨѶ��ʱ�������
void BLE_HOST_PROTOCOL_StopTxRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_TXRX_TIME_OUT_CONTROL);
}
#endif

// �ϵ�Ͽ�ͷ������
void BLE_HOST_UART_SendCmdDisconnectHelmet(uint32 param)
{
	uint8 bleStr[100] = "TTM:DISCONNECT=55:34:FF:93:58:44\r\n";

	// ͷ������Բ�����
	if (!PARAM_GetHelmetPairState())
	{
		TIMER_KillTask(TIMER_ID_BLE_HELMET_PAIR);
		return;
	}
	
	// ��дhandle�̶�
	sprintf((char *)bleStr, "TTM:DISCONNECT=%s\r\n", PARAM_GetHelmetMacBuff());

	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), BLE_OUTFIT_HELMET_ID);
}

// ����ͷ��
void BLE_HOST_UART_SendCmdConnectHelmet(uint32 param)
{
	uint8 bleStr[100] = "TTM:CONNECT=55:34:FF:93:58:44,66,13\r\n";

	// ͷ������Բ�����
	if (!PARAM_GetHelmetPairState())
	{
		TIMER_KillTask(TIMER_ID_BLE_HELMET_PAIR);
		return;
	}
			
	// ��дhandle�̶�
	sprintf((char *)bleStr, "TTM:CONNECT=%s,66,13\r\n", PARAM_GetHelmetMacBuff());
	
	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), BLE_OUTFIT_HELMET_ID);
}

// �ϵ�Ͽ����߰�������
void BLE_HOST_UART_SendCmdDisconnectKey(uint32 param)
{
	uint8 bleStr[100] = "TTM:DISCONNECT=7C:69:6B:52:24:24\r\n";

	// ��������Բ�����
	if (!PARAM_GetLockPairState())
	{
		TIMER_KillTask(TIMER_ID_BLE_LOCK_PAIR);
		return;
	}
	
	// ��дhandle�̶�
	sprintf((char *)bleStr, "TTM:DISCONNECT=%s\r\n", PARAM_GetLockMacBuff());
	
	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), BLE_OUTFIT_LOCK_ID);
}

// �������߰���
void BLE_HOST_UART_SendCmdConnectKey(uint32 param)
{
	uint8 bleStr[100] = "TTM:CONNECT=7C:69:6B:52:24:24,17,13\r\n";

	// ��������Բ�����
	if (!PARAM_GetLockPairState())
	{
		TIMER_KillTask(TIMER_ID_BLE_LOCK_PAIR);
		return;
	}
	
	// ��дhandle�̶�
	sprintf((char *)bleStr, "TTM:CONNECT=%s,14,11\r\n", PARAM_GetLockMacBuff());
	
	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), BLE_OUTFIT_LOCK_ID);
}

// ɨ�������豸
void BLE_HOST_UART_SendScanBleDevice(uint32 param)
{
	uint8 bleStr[100] = "TTM:SCAN=1,5\r\n";
	
	sprintf((char *)bleStr, "TTM:SCAN=1,%ld\r\n", param);
	
	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), 0x00);
}

// �����豸
void BLE_HOST_UART_SendConnectBleDevice(BLE_PAIR_DEVICE device, uint8 macBuff[])
{
	uint8 bleStr[100] = "TTM:CONNECT=7C:69:6B:52:24:24,17,13\r\n";

	if (BLE_HELMET_DEVICE == device)
	{
		// ��дhandle�̶�
		sprintf((char *)bleStr,  (char *)"TTM:CONNECT=%s,66,13\r\n", macBuff);
		
		// ��ӵ����з���
		BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), 0x00);
	}
	else if (BLE_LOCK_DEVICE == device)
	{
		// ��дhandle�̶�
		sprintf((char *)bleStr, "TTM:CONNECT=%s,17,13\r\n", macBuff);
		
		// ��ӵ����з���
		BLE_HOST_PROTOCOL_TxAddTtmCmd(bleStr, strlen((char *)bleStr), 0x00);
	}
}

// ��ʱ���ص������豸����ָ��
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

// ͷ����Ϣ��ѯ
void BLE_HOST_PROTOCOL_SendCmdGetHeadInfoPhone(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[7] = {0x55, 0xB2, 0x01, 0x01, 0x18, 0x0D, 0x0A};		// ��ȡ״̬�����1
	uint8 str2[7] = {0x55, 0xB2, 0x01, 0x02, 0x1B, 0x0D, 0x0A};		// ��ȡ״̬�����2

	// �����Ӳ�����
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

	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// ������ƹ���
void BLE_HOST_PROTOCOL_SendCmdControlPhone(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[9] = {0x55, 0xB1, 0x03, 0x07, 0x00, 0x01, 0x1E, 0x0D, 0x0A};		// ����
	uint8 str2[9] = {0x55, 0xB1, 0x03, 0x07, 0x00, 0x03, 0x1C, 0x0D, 0x0A};		// �Ҷ�

	// �����Ӳ�����
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

	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// ��ý����ƹ���
void BLE_HOST_PROTOCOL_SendCmdControlMp3(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x01, 0x11, 0x0D, 0x0A};	// ����
	uint8 str2[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x02, 0x12, 0x0D, 0x0A}; // ��ͣ
	uint8 str3[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x03, 0x13, 0x0D, 0x0A}; // ��һ��
	uint8 str4[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x04, 0x14, 0x0D, 0x0A}; // ��һ��
	uint8 str5[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x08, 0x18, 0x0D, 0x0A}; // ����+
	uint8 str6[9] = {0x55, 0xB1, 0x03, 0x08, 0x00, 0x09, 0x19, 0x0D, 0x0A}; // ����-
	
	// �����Ӳ�����
	if (!PARAM_GetHelmetConnectState())
	{
		return;
	}

	sprintf((char *)Buff, (char *)"TTM:SEND=%s,", PARAM_GetHelmetMacBuff());
	Index = strlen((char *)Buff);
	
	switch (param)
	{
		// ����
		case BLE_MUSIC_PLAY:
			for (i = 0; i < sizeof(str1); i++)
			{
				Buff[Index++] = str1[i];
			}
			break;
		
		// ��ͣ	
		case BLE_MUSIC_PAUSE:
			for (i = 0; i < sizeof(str2); i++)
			{
				Buff[Index++] = str2[i];
			}
			break;
		
		// ��һ��	
		case BLE_MUSIC_UP:
			for (i = 0; i < sizeof(str3); i++)
			{
				Buff[Index++] = str3[i];
			}
			break;
		
		// ��һ��			
		case BLE_MUSIC_NEXT:
			for (i = 0; i < sizeof(str4); i++)
			{
				Buff[Index++] = str4[i];
			}
			break;
		
		// ����+	
		case BLE_MUSIC_ADD:
			for (i = 0; i < sizeof(str5); i++)
			{
				Buff[Index++] = str5[i];
			}
			break;	
		
		// ����-		
		case BLE_MUSIC_DEC:
			for (i = 0; i < sizeof(str6); i++)
			{
				Buff[Index++] = str6[i];
			}
			break;	
			
		default:
			break;
		
	}
	
	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// ��ת��ƿ���
void BLE_HOST_PROTOCOL_SendCmdLeftTurnLight(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[9] = {0x55, 0xB1, 0x03, 0x01, 0x00, 0x01, 0x18, 0x0D, 0x0A};
	uint8 str2[9] = {0x55, 0xB1, 0x03, 0x01, 0x00, 0x00, 0x19, 0x0D, 0x0A};

	// �����Ӳ�����
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

	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// ��ת��ƿ���
void BLE_HOST_PROTOCOL_SendCmdRightTurnLight(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[9] = {0x55, 0xB1, 0x03, 0x01, 0x00, 0x02, 0x1B, 0x0D, 0x0A};
	uint8 str2[9] = {0x55, 0xB1, 0x03, 0x01, 0x00, 0x00, 0x19, 0x0D, 0x0A};

	// �����Ӳ�����
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

	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, 0x00);
}

// ����������
void BLE_HOST_PROTOCOL_SendCmdGetKey(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[17] = {0xA3, 0xA4, 0x08, 0x50, 0x1E, 0x1F, 0x67, 0x51, 0x4A, 0x73, 0x55, 0x2B, 0x2E, 0x64, 0x00, 0x0D, 0x0A};

	// �����Ӳ�����
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
	
	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, BLE_OUTFIT_LOCK_ID);
}

// �򿪵�����
void BLE_HOST_PROTOCOL_SendCmdLockOpen(uint32 param)
{
	uint8 i = 0;
	uint8 Index = 0;
	uint8 Buff[100] = {0};
	uint8 str1[100] = {0};
	uint8 length = 0;
		
	// �����Ӳ�����
	if (!PARAM_GetLockConnectState())
	{
		return;
	}

	sprintf((char *)Buff, (char *)"TTM:SEND=%s,", PARAM_GetLockMacBuff());
	Index = strlen((char *)Buff);

	BLE_PROTOCOL1_TxAddData(BLE_PROTOCOL1_CMD_HEAD);
	BLE_PROTOCOL1_TxAddData(BLE_PROTOCOL1_CMD_HEAD1);
	BLE_PROTOCOL1_TxAddData(0x00);								// ����
	BLE_PROTOCOL1_TxAddData(0x1E);								// �����
	BLE_PROTOCOL1_TxAddData(PARAM_GetLockKey());				// ��Կ
	BLE_PROTOCOL1_TxAddData(BLE_PROTOCOL1_OPEN_LOCK_CMD);		// ����
	BLE_PROTOCOL1_TxAddData(0x01);								// ����
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
	
	// ��ӵ����з���
	BLE_HOST_PROTOCOL_TxAddTtmCmd(Buff, Index, BLE_OUTFIT_LOCK_ID);
}

// ���������յ���TTM����
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

	// ���ģʽ
	if (BLE_PAIR_MODE == bleHostProtocolCB.pair.mode)
	{
		if (-1 != STRING_Find(buff, (uint8 *)"TTM:SCAN="))
		{
			// ɨ��ɹ����ٴ���
			if (bleHostProtocolCB.pair.scanFlag)
			{
				return;
			}

			switch (bleHostProtocolCB.pair.device)
			{
				case BLE_HELMET_DEVICE:

					// ����Э������ͷ�����״̬����������Ϊ��PDTT-XXXXXXXXXXXX
					if (-1 != STRING_Find(buff, (uint8 *)"PDTT"))
					{
						bleIndex = STRING_Find(buff, (uint8 *)"PDTT");
						
						// ��ȡmac��ַ
						STRING_Left(&buff[bleIndex - 18], 17, bleHostProtocolCB.pair.macBuff);
						bleHostProtocolCB.pair.scanFlag = TRUE;
					}
					break;

				case BLE_LOCK_DEVICE:

					// ����Э�������������״̬����������Ϊ��PDTK-XXXXXXXXXXXX
					if (-1 != STRING_Find(buff, (uint8 *)"PDTK"))
					{
						bleIndex = STRING_Find(buff, (uint8 *)"PDTK");
						
						// ��ȡmac��ַ
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
		// ���豸����״̬
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

					// ��ȡ������key
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
				// ͷ��
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
				// ���߰���
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
		// ���豸����״̬
		else if (-1 != STRING_Find(buff, (uint8 *)"TTM:DISCONNECTED="))
		{
			// ͷ��
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
			// ���߰���
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

// �л��������������
void BLE_SendCmdSwitchStep(uint32 param)
{
	bleHostProtocolCB.sendCmd.sendCmdStep = (BLE_SEND_CMD_STEP)param;
}

// ģ�鷢������ͽ��������
void BLE_SendAndRevCmdHandle(void)
{
	uint16 head = bleHostProtocolCB.ttmTx.head;
	uint16 end = bleHostProtocolCB.ttmTx.end;
	uint16 index = bleHostProtocolCB.ttmTx.index;

	switch (bleHostProtocolCB.sendCmd.sendCmdStep)
	{						
		// ����״̬
		case BLE_SEND_CMD_IDE:
			
			// ����Ϊ�գ�������
			if(head == end)
			{
				return;
			}
			else
			{
				BLE_SendCmdSwitchStep(BLE_SEND_CMD_SEND);
			}
			break;
			
		// �������� ����
		case BLE_SEND_CMD_SEND:
			
			// ���õȴ��ı�־λλ�ȴ�״̬
			bleHostProtocolCB.sendCmd.Flag = FALSE;
			
			// ��ǰ����֡δ������ʱ����������
			while(index < bleHostProtocolCB.ttmTx.cmdQueue[head].length)
			{
				// һֱ��䷢��
				BLE_HOST_UART_BC_SendData(bleHostProtocolCB.ttmTx.cmdQueue[head].buff[bleHostProtocolCB.ttmTx.index++]);
				
				index = bleHostProtocolCB.ttmTx.index;
			}

			if (BLE_OUTFIT_HELMET_ID == bleHostProtocolCB.ttmTx.cmdQueue[head].deviceID ||
				BLE_OUTFIT_LOCK_ID == bleHostProtocolCB.ttmTx.cmdQueue[head].deviceID)
			{
				// ������ʱ�����񣬵ȴ�ָ��ʱ������쳣����
				TIMER_AddTask(TIMER_ID_BLE_SEND_CMD,
								1000,
								BLE_SendCmdSwitchStep,
								BLE_SEND_CMD_TIMOUT,
								1,
								ACTION_MODE_DO_AT_ONCE);

				// �л�ȥ�Ƚ��
				BLE_SendCmdSwitchStep(BLE_SEND_CMD_DELAY);
			}
			else
			{
				// ������ʱ�����񣬵ȴ�ָ��ʱ������쳣����
				TIMER_AddTask(TIMER_ID_BLE_SEND_CMD,
								BLE_SEND_CMD_TIMEOUT,
								BLE_SendCmdSwitchStep,
								BLE_SEND_CMD_TIMOUT,
								1,
								ACTION_MODE_DO_AT_ONCE);
				// �л�ȥ�Ƚ��
				BLE_SendCmdSwitchStep(BLE_SEND_CMD_WAIT);
			}
			break;
			
		// �������� �ȴ����
		case BLE_SEND_CMD_WAIT:

			if(bleHostProtocolCB.sendCmd.Flag)
			{
				// ��ʱע����ʱ��
				TIMER_KillTask(TIMER_ID_BLE_SEND_CMD);
				
				// ���÷��������־λ
				bleHostProtocolCB.sendCmd.Flag = FALSE;
				
				// ��ת������
				BLE_SendCmdSwitchStep(BLE_SEND_CMD_IDE);

				// ��ǰ����֡������ʱ��ɾ��֮
				bleHostProtocolCB.ttmTx.cmdQueue[head].length = 0;
				bleHostProtocolCB.ttmTx.head ++;
				bleHostProtocolCB.ttmTx.head %= BLE_HOST_PROTOCOL_TX_QUEUE_SIZE;
				bleHostProtocolCB.ttmTx.index = 0;
			}
			
		 	break;
		
		// �������� �ȴ���ʱ
		case BLE_SEND_CMD_TIMOUT:
			
			// ��ת������
			BLE_SendCmdSwitchStep(BLE_SEND_CMD_IDE);

			// ���÷��������־λ
			bleHostProtocolCB.sendCmd.Flag = FALSE;
			
			// ��ǰ����֡������ʱ��ɾ��֮
			bleHostProtocolCB.ttmTx.cmdQueue[head].length = 0;
			bleHostProtocolCB.ttmTx.head ++;
			bleHostProtocolCB.ttmTx.head %= BLE_HOST_PROTOCOL_TX_QUEUE_SIZE;
			bleHostProtocolCB.ttmTx.index = 0;
			break;

		// ��ʱ�ȴ�
		case BLE_SEND_CMD_DELAY:
			break;
			
		default: 
			break;
	}
}


// ״̬Ǩ��
void BLE_PairEnterState(uint32 state)
{
	// �õ�ǰ��״̬��Ϊ��ʷ
	bleHostProtocolCB.pair.preState = bleHostProtocolCB.pair.state;

	// �����µ�״̬
	bleHostProtocolCB.pair.state = (BLE_PAIR_STATE)state;	

	switch (state)
	{
		// ��ʼ��״̬
		case BLE_PAIR_INIT:
			
			// ͷ���豸�̶���
			//PARAM_SetHelmetPairState(TRUE);
			//sprintf((char*)PARAM_GetHelmetMacBuff(), "%s", (char*)"16:F0:0E:EC:F6:3F");
			
			// �������豸�̶���
			PARAM_SetLockPairState(TRUE);
			sprintf((char*)PARAM_GetLockMacBuff(), "%s", (char*)"FD:D9:00:FB:69:09");
			//sprintf((char*)PARAM_GetLockMacBuff(), "%s", (char*)"F3:52:6E:C8:E9:3B");
			
			// �ϵ�Ͽ�ͷ���豸
			BLE_HOST_UART_SendCmdDisconnectHelmet(TRUE);

			// �ϵ�Ͽ����߰����豸
			BLE_HOST_UART_SendCmdDisconnectKey(TRUE);
			
			// �ϵ��ѯ����豸
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

			// ����ģʽ
			bleHostProtocolCB.pair.mode = BLE_NORMAL_MODE;
			bleHostProtocolCB.pair.scanFlag = FALSE;
			bleHostProtocolCB.pair.count = 0;
			
			// ��ת����״̬
			BLE_PairEnterState(BLE_PAIR_STANDBY);
			break;

		// ����״̬
		case BLE_PAIR_STANDBY:
			break;
			
		// ɨ��״̬
		case BLE_PAIR_SCAN:

			bleHostProtocolCB.pair.scanFlag = FALSE;
				
			// ����ɨ������
			BLE_HOST_UART_SendScanBleDevice(BLE_OUTFIT_PAIR_SCAN_TIME);

			// ɨ����ɽ�������״̬
			TIMER_AddTask(TIMER_ID_BLE_LOCK_PAIR,
							(BLE_OUTFIT_PAIR_SCAN_TIME+1)*1000,
							BLE_PairEnterState,
							BLE_PAIR_CONNECT,
							1,
							ACTION_MODE_DO_AT_ONCE);
			break;

		// ����״̬
		case BLE_PAIR_CONNECT:

			if (!bleHostProtocolCB.pair.scanFlag)
			{
				BLE_PairEnterState(BLE_PAIR_FAILURE);
				break;
			}
			
			// ������������
			BLE_HOST_UART_SendConnectBleDevice(bleHostProtocolCB.pair.device, bleHostProtocolCB.pair.macBuff);
			
			// ���ӳ�ʱ����תʧ�ܴ���
			TIMER_AddTask(TIMER_ID_BLE_LOCK_PAIR,
							3000,
							BLE_PairEnterState,
							BLE_PAIR_FAILURE,
							1,
							ACTION_MODE_DO_AT_ONCE);
			break;

		// ��Գɹ�״̬
		case BLE_PAIR_SUCCEED:
			
			switch (bleHostProtocolCB.pair.device)
			{
				case BLE_HELMET_DEVICE:

					// �������mac��ַ
					strcpy((char *)PARAM_GetHelmetMacBuff(), (char *)bleHostProtocolCB.pair.macBuff);
					PARAM_SetHelmetPairState(TRUE);
					break;

				case BLE_LOCK_DEVICE:

					// �������mac��ַ
					strcpy((char *)PARAM_GetLockMacBuff(), (char *)bleHostProtocolCB.pair.macBuff);
					PARAM_SetLockPairState(TRUE);
					break;
					
				default:
					break;
			}

			NVM_SetDirtyFlag(TRUE);

			BLE_PairEnterState(BLE_PAIR_STOP);
			break;

		// ���ʧ��״̬
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

		// ���ֹͣ״̬
		case BLE_PAIR_STOP:

			TIMER_KillTask(TIMER_ID_BLE_HELMET_PAIR);
			TIMER_KillTask(TIMER_ID_BLE_LOCK_PAIR);
			
			bleHostProtocolCB.pair.count = 0;
			bleHostProtocolCB.pair.scanFlag = FALSE;
			bleHostProtocolCB.pair.mode = BLE_NORMAL_MODE;

			BLE_PairEnterState(BLE_PAIR_STANDBY);

			// ����豸�Ͽ����Ӿͼ�����ָ��ȥ����
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

// ״̬������
void BLE_PairProcess(void)
{
	// ģ�鷢������ͽ��������
	BLE_SendAndRevCmdHandle();

	switch(bleHostProtocolCB.pair.state)
	{
		// ����״̬
		case BLE_PAIR_STANDBY:
			
			switch (bleHostProtocolCB.pair.mode)
			{
				// ����ģʽ
				case BLE_NORMAL_MODE:
					break;

				// ���ģʽ
				case BLE_PAIR_MODE:
					
					BLE_PairEnterState(BLE_PAIR_SCAN);
					break;

				default:
					break;
			}
			break;
			
		// ɨ��״̬
		case BLE_PAIR_SCAN:

			if (BLE_PAIR_MODE != bleHostProtocolCB.pair.mode)
			{
				BLE_PairEnterState(BLE_PAIR_STOP);
			}
			break;

		// ����״̬
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

// �л������ģʽ
BOOL BLE_SwitchPairMode(BLE_PAIR_DEVICE device)
{
	if (BLE_PAIR_STANDBY != bleHostProtocolCB.pair.state)
	{
		return FALSE;
	}

	switch (device)
	{
		case BLE_HELMET_DEVICE:

			// ������״̬���������
			if (PARAM_GetHelmetConnectState())
			{
				return FALSE;
			}

			// �������״̬��ȡ��֮ǰ�����
			PARAM_SetHelmetPairState(FALSE);
			break;

		case BLE_LOCK_DEVICE:

			// ������״̬���������
			if (PARAM_GetLockConnectState())
			{
				return FALSE;
			}

			// �������״̬��ȡ��֮ǰ�����
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

// ȡ�����
void BLE_PairStop(void)
{
	BLE_PairEnterState(BLE_PAIR_STOP);
}


