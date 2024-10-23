#include "common.h"
#include "timer.h"
#include "BleHostUart.h"
#include "stringOperation.h"
#include "Param.h"
#include "BleHostProtocol.h"

/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/
// UART��ʼ��
void BLE_HOST_UART_HwInit(uint32 baud);

// ���ݽṹ��ʼ��
void BLE_HOST_UART_DataStructureInit(BLE_HOST_UART_CB* pCB);

// ���ʹ���
void BLE_HOST_UART_TxProcess(BLE_HOST_UART_CB* pCB);

// �����ж��ֽڷ���
void BLE_HOST_UART_IR_StartSendData(uint8 data);

//  ��������״̬�жϺ���
void BLE_HOST_UART_BleConnectState(uint32 param);

// ��������һ���ֽ�����
//void BLE_HOST_UART_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void UART2_Handler(void);

// ȫ�ֱ�������
BLE_HOST_UART_CB bleHostUartCB;

// UART��ʼ��
void BLE_HOST_UART_HwInit(uint32 baud)
{
 	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTM, PIN7, PORTM_PIN7_UART2_TX, 0);	//GPIOM.7����ΪUART0�������
	PORT_Init(PORTM, PIN6, PORTM_PIN6_UART2_RX, 1);	//GPIOM.6����ΪUART0��������
	
 	UART_initStruct.Baudrate = baud;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThreshold = 0;			// ֻ����һ���ֽھͽ����ж�
	UART_initStruct.RXThresholdIEn = 1;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;
	UART_initStruct.TimeoutIEn = 1;				// ���ճ�ʱ�ж�Ҳ����
 	UART_Init(BLE_HOST_UART_TYPE_DEF, &UART_initStruct);
	UART_Open(BLE_HOST_UART_TYPE_DEF);
}	

// ���ݽṹ��ʼ��
void BLE_HOST_UART_DataStructureInit(BLE_HOST_UART_CB* pCB)
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
	for(i = 0; i < BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}	

	bleHostUartCB.rcvBleMacOK = FALSE;
}

// ���ʹ���
void BLE_HOST_UART_TxProcess(BLE_HOST_UART_CB* pCB)
{
	// �жϷ�ʽ
#if (BLE_HOST_UART_TX_MODE == BLE_HOST_UART_INTERRUPT_TX_MODE)
	uint16 index = pCB->tx.index;							// ��ǰ�������ݵ�������
	uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length;	// ��ǰ���͵�����֡�ĳ���
	uint16 head = pCB->tx.head;								// ��������֡����ͷ������
	uint16 end = pCB->tx.end;								// ��������֡����β������

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}
	
	// ����Ϊ�գ�������
	if (head == end)
	{
		return;
	}

	// ����æ���˳�
	if (pCB->tx.txBusy)
	{
		return;
	}

	// ����ִ�е����˵�����зǿա���
	
	// ��ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
	if (index < length)
	{		
		BLE_HOST_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		// ���÷���æ״̬
		pCB->tx.txBusy = TRUE;
	}

// ������ʽʱ��������
#else
	uint16 index = pCB->tx.index;
	uint16 head = pCB->tx.head;
	uint16 end = pCB->tx.end;

	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}

	// ����Ϊ�գ�������
	if(head == end)
	{
		return;
	}
	
	// ��ǰ����֡δ������ʱ����������
	while(index < pCB->tx.cmdQueue[head].length)
	{
		// һֱ��䷢��
		BLE_HOST_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

// �����ж��ֽڷ���
void BLE_HOST_UART_IR_StartSendData(uint8 data)
{
	// ����һ���ֽ�
	UART_WriteByte(BLE_HOST_UART_TYPE_DEF, data);

	// �򿪷�������ж�
	UART_INTTXDoneEn(BLE_HOST_UART_TYPE_DEF);	
}


// ��������һ���ֽ�����
void BLE_HOST_UART_BC_SendData(uint8 data)
{	
	// �������
	UART_WriteByte(BLE_HOST_UART_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while (UART_IsTXBusy(BLE_HOST_UART_TYPE_DEF));	
}

/**************************************************************
 * @brief  void BLE_HOST_UART_BleConnectState(void);
 * @input  ��������״̬�жϺ���
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
uint8 bleHostRxBuff[200] = {0};
uint8 bleHostIndex = 0;
void BLE_HOST_UART_BleConnectState(uint32 param)
{	
	// ����TTM����
	BLE_HOST_UART_TTM_AnalysisCmd(bleHostRxBuff, bleHostIndex);

	bleHostIndex = 0;
}

/**************************************************************
 * @brief  void UART2_Handler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
void UART2_Handler(void)
{
	// �ж�DR�Ƿ������ݣ��жϽ���
	if (UART_INTRXThresholdStat(BLE_HOST_UART_TYPE_DEF) || UART_INTTimeoutStat(BLE_HOST_UART_TYPE_DEF))
	{
		uint8 rxdata = 0x00;
		uint32_t chr;
		
		// ��������
		UART_ReadByte(BLE_HOST_UART_TYPE_DEF, &chr);
		rxdata = chr & 0xFF;
		
		if (NULL != bleHostUartCB.receiveDataThrowService)
		{			
			(*bleHostUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}
		
//=================================================================
		// ���������ж�
		if ((bleHostIndex + 1)< sizeof(bleHostRxBuff)/sizeof(bleHostRxBuff[0]))
		{
			bleHostRxBuff[bleHostIndex++] = rxdata;
			bleHostRxBuff[bleHostIndex] = '\0';
		}

		TIMER_AddTask(TIMER_ID_BLE_HOST_CONNECT_STATE,
					10,
					BLE_HOST_UART_BleConnectState,
					0,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
	}


// �ж�ģʽ����
#if (BLE_HOST_UART_TX_MODE == BLE_HOST_UART_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if (UART_INTTXDoneStat(BLE_HOST_UART_TYPE_DEF))
	{
		uint16 head = bleHostUartCB.tx.head;
		uint16 end;
		uint16 index = bleHostUartCB.tx.index;
		uint8 txdata = 0x00;

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if (index < bleHostUartCB.tx.cmdQueue[head].length)
		{
			txdata = bleHostUartCB.tx.cmdQueue[head].buff[bleHostUartCB.tx.index++];
			
			// �������
			UART_WriteByte(BLE_HOST_UART_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			bleHostUartCB.tx.cmdQueue[head].length = 0;
			bleHostUartCB.tx.head ++;
			bleHostUartCB.tx.head %= BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE;
			bleHostUartCB.tx.index = 0;

			head = bleHostUartCB.tx.head;
			end = bleHostUartCB.tx.end;
			
			// ����֡���зǿգ�����������һ������֡
			if (head != end)
			{
				txdata = bleHostUartCB.tx.cmdQueue[head].buff[bleHostUartCB.tx.index++];

				// �������
				UART_WriteByte(BLE_HOST_UART_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ�������ж�
				UART_INTTXDoneDis(BLE_HOST_UART_TYPE_DEF);
				
				bleHostUartCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif
}


// UART��ʼ��
void BLE_HOST_UART_Init(void)
{
	// ���ڳ�ʼ��
	BLE_HOST_UART_HwInit(BLE_HOST_UART_BAUD_RATE);

	// UART���ݽṹ��ʼ��
	BLE_HOST_UART_DataStructureInit(&bleHostUartCB);
}

// UARTģ�鴦�����
void BLE_HOST_UART_Process(void)
{
	// ���ʹ���
	BLE_HOST_UART_TxProcess(&bleHostUartCB);
}

// ע�������׳��ӿڷ���
void BLE_HOST_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	bleHostUartCB.receiveDataThrowService = service;
}

// ���ͻ����������һ������������
BOOL BLE_HOST_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = bleHostUartCB.tx.head;
	uint16 end = bleHostUartCB.tx.end;
	
	// ��������
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// ���ͻ������������������
	if((end + 1) % BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	bleHostUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		bleHostUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	bleHostUartCB.tx.cmdQueue[end].length = length;
	
	// ���ͻ��ζ��и���λ��
	bleHostUartCB.tx.end ++;
	bleHostUartCB.tx.end %= BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE;
	bleHostUartCB.tx.cmdQueue[bleHostUartCB.tx.end].length = 0;

	return TRUE;
}



