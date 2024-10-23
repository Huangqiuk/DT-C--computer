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
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB* pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB* pCB);

// UART����֡����������
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB* pCB);

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB* pCB);

// ͨѶ��ʱ����-����
void UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void UART_PROTOCOL_StopRxTimeOutCheck(void);

// Э��㷢�ʹ������
void UART_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// ������������·����������־
void UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param);

// UART���߳�ʱ������
void UART_PROTOCOL_CALLBACK_UartBusError(uint32 param);

//=========================================================================
void UART_PROTOCOL_SendCmdTwoBytesDataAck(uint8* pBuf, uint8 length1, uint8 length2);
void UART_PROTOCOL_SendCmdOneByteDataAck(uint8* pBuf, uint8 length1);
void UART_PROTOCOL_SendCmdNoDataAck(uint8* pBuf);
void UART_PROTOCOL_SendCmdAppVersion(uint8* pBuf);

// �汾��������
void UART_PROTOCOL_SendCmdVersion(uint8 versionCmd, uint8* pBuf);

// ��ȡFLASH�ظ�
void UART_PROTOCOL_SendCmdReadFlashRespond(uint32 length, uint8* pBuf);

// �ϵ緢��ƥ������
void UART_PROTOCOL_SendSnMatching(uint8 length, uint8 *Sn);

// ȫ�ֱ�������
UART_PROTOCOL_CB uartProtocolCB;


// ��������������������������������������������������������������������������������������

uint32 uartProtocolOutTimeDelay[6] = {30*60*1000,			// ������
									30*60*1000,				// LOGO
									30*60*1000,				// ��ʾ
									1000,					// ��������
									1000,					// �쳣������ʾ
									30*60*1000,};// ����

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



// Э���ʼ��
void UART_PROTOCOL_Init(void)
{
	// Э������ݽṹ��ʼ��
	UART_PROTOCOL_DataStructInit(&uartProtocolCB);

	// ��������ע�����ݽ��սӿ�
	UART_DRIVE_RegisterDataSendService(UART_PROTOCOL_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	UART_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);
}

// UARTЭ�����̴���
void UART_PROTOCOL_Process(void)
{
	// UART����FIFO����������
	UART_PROTOCOL_RxFIFOProcess(&uartProtocolCB);

	// UART���������������
	UART_PROTOCOL_CmdFrameProcess(&uartProtocolCB);
	
	// UARTЭ��㷢�ʹ������
	UART_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void UART_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = uartProtocolCB.tx.head;
	uint16 end =  uartProtocolCB.tx.end;
	UART_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];

	// ���ͻ������������������
	if((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ��β����֡�������˳�
	if(pCmdFrame->length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void UART_PROTOCOL_TxAddFrame(void)
{
	
    uint16_t checkSum = 0;
	uint16_t i = 0;
	uint16 head = uartProtocolCB.tx.head;
	uint16 end  = uartProtocolCB.tx.end;
	UART_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// ���ͻ������������������
	if((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ����֡���Ȳ��㣬������������ݣ��˳�
	if(UART_PROTOCOL_CMD_FRAME_LENGTH_MIN-1 > length)	// ��ȥ"У����"1���ֽ�
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// ��β����֡�������˳�
	if(length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	// �������ݳ��ȣ���Ҫ��ȥ10=(3ͬ��ͷ1������+1���ݳ���)
	pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] = (length - 5);
	

	//cc = UART_GetCRCValue(&pCmdFrame->buff[UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - UART_PROTOCOL_HEAD_BYTE));
	/* ���ȡ������У���� */
    for(i = 0; i < pCmdFrame->length; i++)
	{
		checkSum ^= pCmdFrame->buff[i];
	}
	checkSum = ~checkSum;
	
	pCmdFrame->buff[length] = checkSum;
	pCmdFrame->length ++;

	uartProtocolCB.tx.end ++;
	uartProtocolCB.tx.end %= UART_PROTOCOL_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB* pCB)
{
	uint16 i;
	
	// �����Ϸ��Լ���
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

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length)
{
	uint16 end = uartProtocolCB.rxFIFO.end;
	uint16 head = uartProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;
	
	// ��������
	rxdata = *pData;

	// һ���������������������
	if((end + 1)%UART_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// һ��������δ�������� 
	else
	{
		// �����յ������ݷŵ���ʱ��������
		uartProtocolCB.rxFIFO.buff[end] = rxdata;
		uartProtocolCB.rxFIFO.end ++;
		uartProtocolCB.rxFIFO.end %= UART_PROTOCOL_RX_FIFO_SIZE;
	}	
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{		
	uartProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB* pCB)
{
	UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return bFALSE;
	}

	// ��ʱ������Ϊ�գ��������
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	if(0 == pCmdFrame->length)
	{
		return bFALSE;
	}

	// ���
	pCB->rx.end ++;
	pCB->rx.end %= UART_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
	
	return bTRUE;
}

// Э��㷢�ʹ������
void UART_PROTOCOL_TxStateProcess(void)
{
	uint16 head = uartProtocolCB.tx.head;
	uint16 end =  uartProtocolCB.tx.end;
	uint16 length = uartProtocolCB.tx.cmdQueue[head].length;
	uint8* pCmd = uartProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = uartProtocolCB.tx.cmdQueue[head].deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == uartProtocolCB.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������		
	if (!(*uartProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	uartProtocolCB.tx.cmdQueue[head].length = 0;
	uartProtocolCB.tx.head ++;
	uartProtocolCB.tx.head %= UART_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
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
		if(UART_PROTOCOL_CMD_HEAD1 != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if((pCB->rx.end + 1)%UART_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// ���UARTͨѶ��ʱʱ������-2016.1.5����
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
						UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
						UART_PROTOCOL_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if(pCmdFrame->length >= UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			UART_PROTOCOL_StopRxTimeOutCheck();
#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0;	// 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
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
			pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������
			
			// �����ж�����֡��С���ȣ�һ�����������������ٰ���10���ֽ�,��˲���8���ֽڵıض�������
			if(pCmdFrame->length < UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
			length = (uint16)pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] ;
			if(length > UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				UART_PROTOCOL_StopRxTimeOutCheck();
#endif
			
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// ����֡����У�飬������������ֵ���ֵ�ϣ���������֡��С���ȣ���Ϊ����֡ʵ�ʳ���
			if(pCmdFrame->length < (length + UART_PROTOCOL_CMD_FRAME_LENGTH_MIN))
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if(!UART_CheckSUM_(pCmdFrame))
			{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				UART_PROTOCOL_StopRxTimeOutCheck();
#endif
				
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			UART_PROTOCOL_StopRxTimeOutCheck();
#endif
			length = pCmdFrame->length;
			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			UART_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}
/***************************************************************
 * �� �� ����UART2_CheckSUM
 * ������Σ�UART2_RX_CMD_FRAME *pCmdFrame -> ���ڽ�������֡�ṹ��
 * �������Σ�None
 * �� �� ֵ��None
 * �����������Դ��������֡����У�飬����У����
***************************************************************/
static bool UART_CheckSUM_(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint8_t checkSum = 0;
	uint8_t sumTemp;
	uint16_t i = 0;
	
    /* �����Ϸ��Լ��� */
	if(NULL == pCmdFrame)
	{
		return false;
	}

	/* ���豸��ַ��ʼ����У����֮ǰ��һ���ֽڣ����ν���������� */
	for(i = 0;  i < pCmdFrame->length - 1; i++)
	{
		checkSum ^= pCmdFrame->buff[i];
	}
	
	/* ����ȡ�� */
	checkSum = ~checkSum ;

    /* ��ȡ����֡��У���� */
	sumTemp = pCmdFrame->buff[pCmdFrame->length - 1];
	
	/* �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ */
	if(sumTemp != checkSum)
	{
		return false;
	}
	
	return true;
}

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame)
{
	uint8 cc = 0;
	
	if(NULL == pCmdFrame)
	{
		return bFALSE;
	}

	// ��Э��汾��ʼ����У����֮ǰ��һ���ֽڣ����ν���CRC8����
	//cc = UART_GetCRCValue(&pCmdFrame->buff[UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - (UART_PROTOCOL_HEAD_BYTE + UART_PROTOCOL_CHECK_BYTE)));
     
	// �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
	if(pCmdFrame->buff[pCmdFrame->length-1] != cc)
	{
		return bFALSE;
	}
	
	return bTRUE;
	
}

// �ϱ�����ִ�н��
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

	// д���ֽ���
	UART_PROTOCOL_TxAddData(1);

	// д����
	UART_PROTOCOL_TxAddData((uint8)param);
	
	UART_PROTOCOL_TxAddFrame();
}


// �ϱ�����Ӧ��
void UART_PROTOCOL_SendCmdAck(uint32 cmd)
{
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD1);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD2);
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD3);
	UART_PROTOCOL_TxAddData(cmd);
	//����
	UART_PROTOCOL_TxAddData(0);
	//Ŀ�����
	UART_PROTOCOL_TxAddData(BOARD_ARM);
	// ��װ����֡
	UART_PROTOCOL_TxAddFrame();
}

// �豸�ϱ�����
void APP_ReportMsg(uint8_t cmd, uint8_t *msg, uint8_t length)
{		
	uint8 i = 0;
	
	//	����ͷ1
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD1);
	//	����ͷ2
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD2);
	//	����ͷ3
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD3);
	//	������
	UART_PROTOCOL_TxAddData(cmd);
	// 	���ȣ����Ȼ���UART2_TxAddData�������Զ�����
	UART_PROTOCOL_TxAddData(0);
	//	���ӷ���Ŀ�ĵ�����
	UART_PROTOCOL_TxAddData(BOARD_ARM);
	
	//	д������
	for(i = 0; i < length; i++)
	{
		UART_PROTOCOL_TxAddData(msg[i]);
	}
	
	// ��װ����֡
	UART_PROTOCOL_TxAddFrame();
	
	
}


// �ϱ�ģ��д����
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

	// д���ֽ���
	UART_PROTOCOL_TxAddData(byteNum/256);
	UART_PROTOCOL_TxAddData(byteNum%256);

	// д����
	UART_PROTOCOL_TxAddData((uint8)exeResoult);
	
	UART_PROTOCOL_TxAddFrame();
}


// ѯ�ʹ̼��汾
void Uart2_CmdAskVersionACK(void)
{	
	uint8_t aSendMsg[7];

	// APP�̼��汾
	aSendMsg[2] = (0);
	aSendMsg[1] = (0);
	aSendMsg[0] = (1);
	//// APP�̼����
	aSendMsg[3] = (1);
	//	boot�̼��汾
	aSendMsg[6] = (0);
	aSendMsg[5] = (0);
	aSendMsg[4] = (1);
			
	// �����ϱ�����
	APP_ReportMsg(UART_CMD_CALL, aSendMsg, 7);
}

extern LOCAL_PARAM_CB 	localPrame; 
// ѯ�ʹ̼��汾
void Uart2_CmdAskVerACK(void)
{	
	uint8_t aSendMsg[7];

	// APP�̼��汾
	aSendMsg[2] = ((localPrame.data.prame.AppVersion >> 16) & 0xff);
	aSendMsg[1] = ((localPrame.data.prame.AppVersion >> 8) & 0xff);
	aSendMsg[0] = ((localPrame.data.prame.AppVersion >> 0) & 0xff);
	//// APP�̼����
	aSendMsg[3] = (localPrame.data.prame.AppVersionNum);
	//	boot�̼��汾
	aSendMsg[6] = ((localPrame.data.prame.BootVersion >> 16) & 0xff);
	aSendMsg[5] = ((localPrame.data.prame.BootVersion >> 8) & 0xff);
	aSendMsg[4] = ((localPrame.data.prame.BootVersion >> 0) & 0xff);
			
	// �����ϱ�����
	APP_ReportMsg(UART_CMD_CALL, aSendMsg, 7);
}




/*******************************************************************************
* Function Name  : MesureType_SwitchToGears (AVO_MEASURE_TYPE MesureType )
* Description    : ��ͨѶЭ��Ĳ�������ת��Ϊ���ñ�����͡�(��Ҫ��Ϊ����PUCS���ñ���ϵ������߼�����)
* Input          : MesureType ����������
* Output         : None
* Return         : AVOMETER_GEARS_CMD ����ֵ����Ϊ0���ʾʧ��
*******************************************************************************/
AVOMETER_GEARS_CMD  MesureType_SwitchToGears (AVO_MEASURE_TYPE MesureType )
{
	   AVOMETER_GEARS_CMD  AVOMETER_GEARS=0;
       switch(MesureType)
		{	
			//ֱ����ѹ��V��					
			case MEASURE_DCV :					 				
			      AVOMETER_GEARS=AVOMETER_GEARS_DCV;
				  break;
			//������ѹ��V��
			case MEASURE_ACV :
				  AVOMETER_GEARS=AVOMETER_GEARS_ACV;
				  break;
			//���裨����
			case MEASURE_OHM :
				  AVOMETER_GEARS=AVOMETER_GEARS_OHM;
				  break;					     
			//ֱ��������mA��		 
			case MEASURE_DCMA:						 
				  AVOMETER_GEARS=AVOMETER_GEARS_DCMA;				
				  break;
			//����������mA��		 
			case MEASURE_ACMA:						 
				  AVOMETER_GEARS=AVOMETER_GEARS_ACMA;
				  break;
			//ֱ��������A��
			case MEASURE_DCA :
                  AVOMETER_GEARS=AVOMETER_GEARS_DCA;				
				  break;
			//����������A��		
			case MEASURE_ACA :	
                  AVOMETER_GEARS=AVOMETER_GEARS_ACA;				
				  break;					
		}
        return AVOMETER_GEARS;
}

void SysRest(uint32 param)
{
	__set_FAULTMASK(1); // �ر������ж�
    NVIC_SystemReset(); // ��λ
}


//��λָ��Ӧ��
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


// UART����֡����������
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB* pCB)
{
	UART_PROTOCOL_CMD cmd = UART_CMD_NULL;
	UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
	AVO_MEASURE_TYPE mesureType;
	AVOMETER_GEARS_CMD  AVO_GEARS=0;
	uint8 chl=0;
	uint8 res=0;
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
	if(UART_PROTOCOL_CMD_HEAD1 != pCmdFrame->buff[UART_PROTOCOL_CMD_HEAD1_INDEX])
	{
		// ɾ������֡
		pCB->rx.head ++;
		pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (UART_PROTOCOL_CMD)pCmdFrame->buff[UART_PROTOCOL_CMD_INDEX];
	
	
	// ִ������֡
	switch(cmd)
	{
		
		case UART_CMD_CALL:
			    Uart2_CmdAskVersionACK(); 
		        break;
		 case    UART_CMD_RES :
			    Uart2_CmdRstACK();   
				break;	
		// ���ñ��������
		case UART_CMD_AVOMETER:	
                // һ��ʼ�͸�λ���Լ̵���	20220420		
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
                    //ֱ����ѹ��V��					
					case MEASURE_DCV :
						  if(chl<RELAY_VIN_MAX)
						  {
							 COM_CHL_ON(); 						
							 VIN_RelayCtrl(chl,ON); 
						  }							
					      break;
					//������ѹ��V��
					case MEASURE_ACV :
						  if(chl<RELAY_VIN_MAX)
						  {
							 COM_CHL_ON(); 	
						     VIN_RelayCtrl(chl,ON);
						  }							  
					      break;
					//���裨����
					case MEASURE_OHM :
						  if(chl<RELAY_OHM_MAX)
						  {
						     COM_CHL_ON(); 	
						     OHM_RelayCtrl(chl,ON); 
						  }
						  break;	
                    //ֱ��������mA��		 
					case MEASURE_DCMA:						 
						 COM_CHL_ON(); 				
						  break;
					//����������mA��		 
					case MEASURE_ACMA:						 
						 COM_CHL_ON();
						  break;
					//ֱ��������A��
					case MEASURE_DCA :
						 COM_CHL_ON();				
						  break;
					//����������A��		
					case MEASURE_ACA :	
						 COM_CHL_ON(); 			
				          break;										
					default:
			              break;					
				}
				// ���õ�λ
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
				printf(" ��ѯ��ͷ��λ����\r\n");					
				
			    break;				
		// ���ñ�λ����
		case UART_CMD_AVORESET :
			    res=1;
			    RelayReset();
			    APP_ReportMsg(UART_CMD_AVORESET_ACK, &res, 1);
                break;
		
		// �㲥��λ����
        case UART_CMD_RESET_SOURCE:	
			    RelayReset();
		        break;		
		default:
			    break;
	}	
	// ɾ������֡
	pCB->rx.head ++;
	pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	UART_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &uartProtocolCB.rx.cmdQueue[uartProtocolCB.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0;	// 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	uartProtocolCB.rxFIFO.head ++;
	uartProtocolCB.rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
	uartProtocolCB.rxFIFO.currentProcessIndex = uartProtocolCB.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void UART_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

// ������������·����������־
void UART_PROTOCOL_SetTxAtOnceRequest(uint32 param)
{
	uartProtocolCB.txAtOnceRequest = (BOOL)param;
}

// ������������·����������־
void UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param)
{
	uartProtocolCB.txPeriodRequest = (BOOL)param;
}


//========================================================================



