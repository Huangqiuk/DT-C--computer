#include "common.h"
#include "timer.h"
#include "uartDrive.h"
#include "uartProtocol.h"
#include "state.h"
#include "spiFlash.h"
#include "ArmUart.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "uartProtocol3.h"
#include "DtaUartProtocol.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB *pCB);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB *pCB);

// UART����֡����������
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB *pCB);

// Э��㷢�ʹ������
void UART_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// ������������
void UART_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result);

// ���������޽��
void UART_PROTOCOL_SendCmdNoResult(uint8 cmdWord);

//==================================================================================
// �ٶ�ƽ���˲�����
void UART_PROTOCOL_CALLBACK_SpeedFilterProcess(uint32 param);

// ������������
void UART_PROTOCOL_SendLdoV(uint8 cmdWord, uint32 result);

// ȫ�ֱ�������
UART_PROTOCOL_CB uartProtocolCB;

// ��������������������������������������������������������������������������������������

// Э���ʼ��
void UART_PROTOCOL_Init(void)
{
	// Э������ݽṹ��ʼ��
	UART_PROTOCOL_DataStructInit(&uartProtocolCB);

	// ��������ע�����ݽ��սӿ�
	// UART_DRIVE_RegisterDataSendService(UART_PROTOCOL_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	UART_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);

	// ע������������ʱ��
	//	TIMER_AddTask(TIMER_ID_PROTOCOL_55_PARAM_TX,
	//					300,
	//					UART_PROTOCOL_SendCmdAging,
	//					TRUE,
	//					TIMER_LOOP_FOREVER,
	//					ACTION_MODE_ADD_TO_QUEUE);
}

// UARTЭ�����̴���
void UART_PROTOCOL_Process(void)
{
	// 55����Э��
	//  UART����FIFO����������
	UART_PROTOCOL_RxFIFOProcess(&uartProtocolCB);

	// UART���������������
	UART_PROTOCOL_CmdFrameProcess(&uartProtocolCB);

	// UARTЭ��㷢�ʹ������
	UART_PROTOCOL_TxStateProcess();

	// 3A����Э��
	// UART����FIFO����������
	UART_PROTOCOL3_RxFIFOProcess(&uartProtocolCB3);

	// UART���������������
	UART_PROTOCOL3_CmdFrameProcess(&uartProtocolCB3); // km5sЭ�飬��app������config

	// UARTЭ��㷢�ʹ������
	UART_PROTOCOL3_TxStateProcess();

    // DTA����Э��
	//  UART����FIFO����������
	DTA_UART_PROTOCOL_RxFIFOProcess(&dtaUartProtocolCB);

	// UART���������������
	DTA_UART_PROTOCOL_CmdFrameProcess(&dtaUartProtocolCB);

	// UARTЭ��㷢�ʹ������
	DTA_UART_PROTOCOL_TxStateProcess();    
}

// ��������֡�������������
void UART_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = uartProtocolCB.tx.head;
	uint16 end = uartProtocolCB.tx.end;
	UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];

	// ���ͻ������������������
	if ((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// ��β����֡�������˳�
	if (pCmdFrame->length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}
	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void UART_PROTOCOL_TxAddFrame(void)
{
	uint16 cc = 0;
	uint16 i = 0;
	uint16 head = uartProtocolCB.tx.head;
	uint16 end = uartProtocolCB.tx.end;
	UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// ���ͻ������������������
	if ((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// ����֡���Ȳ��㣬������������ݣ��˳�
	if (UART_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length) // ��ȥ"У���"1���ֽ�
	{
		pCmdFrame->length = 0;

		return;
	}

	// ��β����֡�������˳�
	if (length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] = length - 3; // �������ݳ���

	for (i = 0; i < length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = ~cc;

	uartProtocolCB.tx.end++;
	uartProtocolCB.tx.end %= UART_PROTOCOL_TX_QUEUE_SIZE;

	// pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

void UART_PROTOCOL_TxAddFrame_3A(void)
{
	uint16 checkSum = 0;
	uint16 i = 0;
	uint16 head = uartProtocolCB3.tx.head;
	uint16 end = uartProtocolCB3.tx.end;
	UART_PROTOCOL3_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// ���ͻ������������������
	if ((end + 1) % UART_PROTOCOL3_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// ����֡���Ȳ��㣬������������ݣ��˳�
	if (UART_PROTOCOL3_CMD_FRAME_LENGTH_MIN - 4 > length) // ��ȥ"У���L��У���H��������ʶ0xD��������ʶOxA"4���ֽ�
	{
		pCmdFrame->length = 0;

		return;
	}

	// ��β����֡�������˳�
	if ((length >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX) || (length + 2 >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX))
	{
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	pCmdFrame->buff[UART_PROTOCOL3_CMD_LENGTH_INDEX] = length - 4; // �������ݳ��ȣ���ȥ"����ͷ���豸��ַ�������֡����ݳ���"4���ֽ�
	for (i = 1; i < length; i++)
	{
		checkSum += pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);		   // ���ֽ���ǰ
	pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF); // ���ֽ��ں�

	// ������ʶ
	pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
	pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

	uartProtocolCB3.tx.end++;
	uartProtocolCB3.tx.end %= UART_PROTOCOL3_TX_QUEUE_SIZE;
}

// ���ݽṹ��ʼ��
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB *pCB)
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
	for (i = 0; i < UART_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end = 0;
	for (i = 0; i < UART_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
	uint16 end = uartProtocolCB.rxFIFO.end;
	uint16 head = uartProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;

	// ��������
	rxdata = *pData;

	// һ���������������������
	if ((end + 1) % UART_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// һ��������δ��������
	else
	{
		// �����յ������ݷŵ���ʱ��������
		uartProtocolCB.rxFIFO.buff[end] = rxdata;
		uartProtocolCB.rxFIFO.end++;
		uartProtocolCB.rxFIFO.end %= UART_PROTOCOL_RX_FIFO_SIZE;
	}
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
	uartProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB *pCB)
{
	UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return FALSE;
	}

	// ��ʱ������Ϊ�գ��������
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	if (0 == pCmdFrame->length)
	{
		return FALSE;
	}

	// ���
	pCB->rx.end++;
	pCB->rx.end %= UART_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

	return TRUE;
}

// Э��㷢�ʹ������
void UART_PROTOCOL_TxStateProcess(void)
{
	uint16 head = uartProtocolCB.tx.head;
	uint16 end = uartProtocolCB.tx.end;
	uint16 length = uartProtocolCB.tx.cmdQueue[head].length;
	uint8 *pCmd = uartProtocolCB.tx.cmdQueue[head].buff;
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
	uartProtocolCB.tx.head++;
	uartProtocolCB.tx.head %= UART_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB *pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
	uint16 length = 0;
	uint8 currentData = 0;

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}

	// һ��������Ϊ�գ��˳�
	if (head == end)
	{
		return;
	}

	// ��ȡ��ʱ������ָ��
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	// ȡ����ǰҪ������ֽ�
	currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];

	// ��ʱ����������Ϊ0ʱ���������ֽ�
	if (0 == pCmdFrame->length)
	{
		// ����ͷ����ɾ����ǰ�ֽڲ��˳�
		if (UART_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if ((pCB->rx.end + 1) % UART_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
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
		pCmdFrame->buff[pCmdFrame->length++] = currentData;
		pCB->rxFIFO.currentProcessIndex++;
		pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if (pCmdFrame->length >= UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			UART_PROTOCOL_StopRxTimeOutCheck();
#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0; // 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// һֱȡ��ĩβ
		while (end != pCB->rxFIFO.currentProcessIndex)
		{
			// ȡ����ǰҪ������ֽ�
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			// ������δ������������գ���������ӵ���ʱ��������
			pCmdFrame->buff[pCmdFrame->length++] = currentData;
			pCB->rxFIFO.currentProcessIndex++;
			pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������

			// �����ж�����֡��С���ȣ�һ�����������������ٰ���8���ֽ�: ����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����˲���8���ֽڵıض�������
			if (pCmdFrame->length < UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
			if (pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] > UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				UART_PROTOCOL_StopRxTimeOutCheck();
#endif

				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����Ϊ����֡ʵ�ʳ���
			length = pCmdFrame->length;
			if (length < pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] + UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if (!UART_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				UART_PROTOCOL_StopRxTimeOutCheck();
#endif

				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			UART_PROTOCOL_StopRxTimeOutCheck();
#endif

			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			UART_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
	uint8 cc = 0;
	uint16 i = 0;

	if (NULL == pCmdFrame)
	{
		return FALSE;
	}

	// ������ͷ��ʼ����У����֮ǰ��һ���ֽڣ����ν����������
	for (i = 0; i < pCmdFrame->length - 1; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}

	cc = ~cc;

	// �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
	if (pCmdFrame->buff[pCmdFrame->length - 1] != cc)
	{
		return FALSE;
	}

	return TRUE;
}

void setDutConfig(uint32 temp)
{
	/*����dut������Ϣ*/
	uint8 configs[100] = {0};
	uint8 i;

	SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�

	// DUT�ϵ�
	DUTCTRL_PowerOnOff(1);

	// �������ͷ
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);

	// ����豸��ַ
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// ���������
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0

	// ������ݳ���
	UART_PROTOCOL3_TxAddData(54);

	// д����
	// UART_PROTOCOL3_TxAddData(1);//��������1��������
	for (i = 0; i < 54; i++)
	{
		UART_PROTOCOL3_TxAddData(configs[i]);
	}
	// ��Ӽ������������������������
	UART_PROTOCOL3_TxAddFrame();
}

void uartProtocol_SendOnePacket(uint32 flashAddr, uint32 addr)
{
	uint8 addr1, addr2, addr3, addr4;
	uint8 appUpdateOnePacket[150] = {0};
	uint8 appUpdateOnePacket_i = 0;
	addr1 = (addr & 0xFF000000) >> 24;
	addr2 = (addr & 0x00FF0000) >> 16;
	addr3 = (addr & 0x0000FF00) >> 8;
	addr4 = (addr & 0x000000FF);
	SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + addr, 128); // ���߶�ȡ128�ֽ�

	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD); // ͷ
	if (dut_info.appUpFlag)
	{
		UART_PROTOCOL_TxAddData(UART_CMD_UP_APP_UP); // ����0x06
	}
	else if (dut_info.uiUpFlag)
	{
		UART_PROTOCOL_TxAddData(UART_CMD_DUT_UI_DATA_WRITE); // ����0x14
	}
	UART_PROTOCOL_TxAddData(0x00);													   // ������ʱΪ0
	UART_PROTOCOL_TxAddData(addr1);													   // ��ӵ�ַ
	UART_PROTOCOL_TxAddData(addr2);													   // ��ӵ�ַ
	UART_PROTOCOL_TxAddData(addr3);													   // ��ӵ�ַ
	UART_PROTOCOL_TxAddData(addr4);													   // ��ӵ�ַ
	for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 128; appUpdateOnePacket_i++) // �������
	{
		UART_PROTOCOL_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i]);
	}
	UART_PROTOCOL_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
}

void uartProtocol_SendOnePacket_Hex(uint32 flashAddr)
{
	uint8 appUpdateOnePacket[150] = {0};
	uint8 appUpdateOnePacket_i = 0;															 // app��ʼ��ַ
	SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + dut_info.currentAppSize * 116, 116); // ���߶�ȡ 4+112 ���ֽ�
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);										 // �������ͷ;//ͷ
	UART_PROTOCOL_TxAddData(UART_CMD_DUT_APP_WRITE_FLASH);									 // ����
	UART_PROTOCOL_TxAddData(0x00);															 // �����ʱ���ݳ���
	UART_PROTOCOL_TxAddData(0);																 // ����Ƿ����
	UART_PROTOCOL_TxAddData(0);																 // �����Կ
	UART_PROTOCOL_TxAddData(appUpdateOnePacket[0]);											 // ��ӵ�ַ
	UART_PROTOCOL_TxAddData(appUpdateOnePacket[1]);											 // ��ӵ�ַ
	UART_PROTOCOL_TxAddData(appUpdateOnePacket[2]);											 // ��ӵ�ַ
	UART_PROTOCOL_TxAddData(appUpdateOnePacket[3]);											 // ��ӵ�ַ

	for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 112; appUpdateOnePacket_i++) // �������
	{
		UART_PROTOCOL_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i + 4]);
	}
	UART_PROTOCOL_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
}

// UART����֡����������
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB *pCB)
{
	UART_PROTOCOL_CMD cmd = UART_PROTOCOL_CMD_NULL;
	UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	// �ҵı���
	uint8 eraResual = 0;

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}

	// ����֡������Ϊ�գ��˳�
	if (pCB->rx.head == pCB->rx.end)
	{
		return;
	}

	// ��ȡ��ǰҪ���������ָ֡��
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.head];

	// ����ͷ�Ƿ����˳�
	if (UART_PROTOCOL_CMD_HEAD != pCmdFrame->buff[UART_PROTOCOL_CMD_HEAD_INDEX])
	{
		// ɾ������֡
		pCB->rx.head++;
		pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (UART_PROTOCOL_CMD)pCmdFrame->buff[UART_PROTOCOL_CMD_CMD_INDEX];
	switch (dut_info.ID)
	{
	case DUT_TYPE_NULL:
		break;

	case DUT_TYPE_CM: // ����ͨ������
	
		switch (cmd)
		{
		case UART_PROTOCOL_CMD_NULL: // = 0				// ������
			break;

		case UART_CMD_UP_PROJECT_APPLY: // 0x01			//DUT����ģʽ����,bc280ul
			// ����������빤��ģʽ
			if (dut_info.bootUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_BOOT_ENTRY);
			}
			else if (!dut_info.dutPowerOnAllow)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_UP_ALLOW);
			}
			else
			{
				/*���������Ǳ��Զ���������ģʽ*/
			}
			break;

		case UART_CMD_UP_PROJECT_READY: // 0x03			//DUT����ģʽ׼������
			// ѡ������
//			if (dut_info.bootUpFlag)
//			{
//				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_BOOT_ENTRY);
//			}
            if (dut_info.appUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR);
			}            
			else if (dut_info.uiUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR);
			}
			else if (dut_info.configUpFlag)
			{
				STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
			}  
			break;

		case UART_CMD_DUT_UI_DATA_ERASE_ACK: // 0x13		//DUT_UI��д�ɹ�
			// ��ȡDUT_UI�������
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			// ����ʧ��
			if (0 == eraResual)
			{
				// ����ʧ���ٲ���һ��
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR);
				break;
			}
			// �����ɹ�
			dut_info.currentUiSize = 0;
			STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET);
			break;

		case UART_CMD_DUT_UI_DATA_WRITE_RES: // 0x15			//DUT_UIд����
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			if (0 == eraResual) // ����Ǳ�д��ʧ�ܾ��˳�
			{
				dut_info.currentUiSize--;
			}

			if (dut_info.currentUiSize < (dut_info.uiSize + 1))
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET);
			}
			else
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_UI_UP_SUCCESS);
			}
			break;

		case UART_CMD_DUT_APP_ERASE_FLASH: // 0x24		//DUT_APP��д
			break;

		case UART_CMD_DUT_APP_ERASE_RESULT: // 0x25			//DUT_APP��д�����ѯ
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];
			// ����ʧ��
			if (0 == eraResual)
			{
				// ����ʧ���ٲ���һ��
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR);
				break;
			}

			// ���͵�һ֡����
			dut_info.currentAppSize = 0;
			STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET);
			break;

		case UART_CMD_DUT_APP_WRITE_FLASH: // 0x26		//DUT_APPд��
			break;

		case UART_CMD_DUT_APP_WRITE_FLASH_RES: // 0x27		//DUT_APPд����
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			if (0 == eraResual) // ����Ǳ�д��ʧ�ܾ��˳�
			{
				dut_info.currentAppSize--;
			}
			if (dut_info.currentAppSize < dut_info.appSize)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET);
			}
			else
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_APP_UP_SUCCESS);
			}
			break;

		case UART_CMD_UP_UPDATA_FINISH_RESULT: // 0x2B  			//DUTд����ȷ��
			STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_ITEM_FINISH);
			break;

		default:
			break;
		}
		break;
		
	case DUT_TYPE_HUAXIN: // ��о΢�ش�������
		// ִ������֡
		switch (cmd)
		{
		// �յ��Ǳ��͵�ECO����
		case UART_CMD_UP_PROJECT_ALLOW:	   // 02
			if (!dut_info.dutPowerOnAllow) // �������Ͳ�Ӧ��
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_UP_ALLOW);
			}
			break;

		// �Ǳ��Ӧ׼������
		case UART_CMD_UP_PROJECT_READY: // 03

			/*�жϻ����Ƿ�Ҫ�����ı�־λ*/
			if (dut_info.appUpFlag)
			{
				// ���Ǳ��Ͳ���app������
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_EAR);
			}
			else if (dut_info.uiUpFlag)
			{
				// ���Ǳ��Ͳ���ui������
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR);
			}
			else if (dut_info.configUpFlag)
			{
				STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
			}
			break;

		case UART_CMD_UP_APP_EAR: // 05
			// ��ȡ�Ǳ�������
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			// ����ʧ��
			if (0 == eraResual)
			{
				// ����ʧ���ٲ���һ��
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_EAR);
				break;
			}
			// �����ɹ�
			STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET);
			break;

		case UART_CMD_UP_APP_UP: // 06,appд��
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];
			if (0 == eraResual) // ����Ǳ�д��ʧ��
			{
				dut_info.currentAppSize--;
			}

			if (dut_info.currentAppSize < dut_info.appSize)
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_PACKET);
			}
			else
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_APP_UP_SUCCESS);
			}

			break;

		case UART_CMD_UP_APP_UP_OVER: // 07���������������ʼ������һ��
			STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_ITEM_FINISH);

			break;

		case UART_CMD_DUT_UI_DATA_ERASE: // 0x12,ui����

			break;

		case UART_CMD_DUT_UI_DATA_ERASE_ACK: // 0x13,ui����Ӧ��
			// ��ȡDUT_UI�������
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			// ����ʧ��
			if (0 == eraResual)
			{
				// ����ʧ���ٲ���һ��
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR);
				break;
			}
			// �����ɹ�
			dut_info.currentUiSize = 0;
			STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET);
			break;
		case UART_CMD_DUT_UI_DATA_WRITE: // 0x14,uiд��

			break;
		case UART_CMD_DUT_UI_DATA_WRITE_RES: // 0x15,uiд����

			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			if (0 == eraResual) // ����Ǳ�д��ʧ�ܾ��˳�
			{
				dut_info.currentUiSize--;
			}

			if (dut_info.currentUiSize < (dut_info.uiSize + 1))
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_PACKET);
			}
			else
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_UI_UP_SUCCESS);
			}
			break;
		default:
			break;
		}

		break;

	// ���ڶ���������
	case DUT_TYPE_SEG:
		switch (cmd)
		{
		// �յ��Ǳ��͵�ECO����
		case UART_CMD_UP_PROJECT_ALLOW: // 02
			// ����������빤��ģʽ
			STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_UP_ALLOW);
			break;
		case UART_CMD_UP_PROJECT_READY: // 03
			// ����app����ָ��
			if (dut_info.appUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR);
			}
			else if (dut_info.configUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH);
			}
			break;

		// dut���������ѯ
		case UART_CMD_UP_APP_EAR:
			// ��ȡ�Ǳ�������
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			// ����ʧ��
			if (0 == eraResual)
			{
				// ����ʧ���ٲ���һ��
				STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR);
				break;
			}
			// �����ɹ�
			// ���͵�һ������
			STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET);

			break;
		case UART_CMD_UP_APP_UP: // 06,app����д��
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			if (0 == eraResual) // ����Ǳ�д��ʧ��
			{
				break;
			}
			else
			{
				if (dut_info.currentAppSize < dut_info.appSize)
				{
					STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_SEND_APP_PACKET); // ����app���ݰ�
				}
				else
				{
					STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS);
				}
			}
			break;
		case UART_CMD_UP_APP_UP_OVER: // 07��app�������
			STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH);
			break;
		default:
			break;
		}
		break;
		
	// ������������
	case DUT_TYPE_KAIYANG:
		// ִ������֡
		switch (cmd)
		{
		// �յ��Ǳ��͵�ECO����
		case UART_CMD_UP_PROJECT_APPLY: // 01
			if (!dut_info.dutPowerOnAllow)
			{
				if (dut_info.uiUpFlag || dut_info.appUpFlag)
				{
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_UP_ALLOW);
				}
				else if (dut_info.configUpFlag)
				{
					STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
				}
				dut_info.dutPowerOnAllow = TRUE; // ���ڽ���eco����
			}
			break;

		// �Ǳ��Ӧ׼������
		case UART_CMD_UP_PROJECT_ALLOW: // 02
			if (dut_info.appUpFlag)
			{
				dut_info.currentAppSize = 0;
				STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE);
				TIMER_AddTask(TIMER_ID_RECONNECTION, 
							1000, 
							STATE_SwitchStep, 
							STEP_KAIYANG_UART_UPGRADE_SEND_APP_AGAIN, 
							TIMER_LOOP_FOREVER, 
							ACTION_MODE_ADD_TO_QUEUE);
			}
			else if (dut_info.uiUpFlag)
			{
				dut_info.currentUiSize = 0;
				STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE);
				TIMER_AddTask(TIMER_ID_RECONNECTION, 
							1000, 
							STATE_SwitchStep, 
							STEP_KAIYANG_UART_UPGRADE_SEND_UI_AGAIN, 
							TIMER_LOOP_FOREVER, 
							ACTION_MODE_ADD_TO_QUEUE);
			}
			break;

		case UART_CMD_UP_PROJECT_READY: // 03
			TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);	// ι��
			if (dut_info.appUpFlag)
			{
				if (dut_info.currentAppSize < dut_info.appSize - 1)
				{
					if (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX])
					{
						dut_info.currentAppSize++;
					}
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE);
				}
				else
				{
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_APP_UP_SUCCESS);
				}
			}
			else if (dut_info.uiUpFlag)
			{
				if (dut_info.currentUiSize < dut_info.uiSize - 1)
				{
					if (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX])
					{
						dut_info.currentUiSize++;
					}
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE);
				}
				else
				{
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_APP_UP_SUCCESS);
				}
			}

			break;
		case UART_ECO_CMD_ECO_JUMP_APP: // 04
			if (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX])
			{
				// �����ɹ�
				if (dut_info.appUpFlag)
				{
					dut_info.appUpSuccesss = TRUE;
					dut_info.appUpFlag = FALSE;
				}
				else if (dut_info.uiUpFlag)
				{
					dut_info.uiUpSuccesss = TRUE;
					dut_info.uiUpFlag = FALSE;
				}
			}
			else // ����ʧ��
			{
				if (dut_info.appUpFlag)
				{
					dut_info.appUpFaile = TRUE;
					dut_info.appUpFlag = FALSE;
				}
				else if (dut_info.uiUpFlag)
				{
					dut_info.appUpFaile = TRUE;
					dut_info.uiUpFlag = FALSE;
				}
			}
			TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
			STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_ITEM_FINISH);
			break;

		default:
			break;
		}
		break;
		
	default:
		break;
	}

	// ɾ������֡
	pCB->rx.head++;
	pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
}

// ��������ظ�����һ������
void UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
	UART_PROTOCOL_TxAddData(0x55);
	UART_PROTOCOL_TxAddData(ackCmd);
	UART_PROTOCOL_TxAddData(0x01);
	UART_PROTOCOL_TxAddData(ackParam);
	UART_PROTOCOL_TxAddFrame();
}

// RXͨѶ��ʱ����-����
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	pCmdFrame = &uartProtocolCB.rx.cmdQueue[uartProtocolCB.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0; // 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	uartProtocolCB.rxFIFO.head++;
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

// 	UART���߳�ʱ������
void UART_PROTOCOL_CALLBACK_UartBusError(uint32 param)
{
}

// �ϱ�д���ò������
void UART_PROTOCOL_ReportWriteParamResult(uint32 param)
{
	// �������ͷ
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

	// ����豸��ַ
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// ���������
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM_RESULT);

	// ������ݳ���
	UART_PROTOCOL_TxAddData(1);

	// д����
	UART_PROTOCOL_TxAddData(param);

	// ��Ӽ������������������������
	UART_PROTOCOL_TxAddFrame();
}

// ������������
void UART_PROTOCOL_SendLdoV(uint8 cmdWord, uint32 result)
{
	// �������ͷ
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

	// ����豸��ַ
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// ���������
	UART_PROTOCOL_TxAddData(cmdWord);

	// ������ݳ���
	UART_PROTOCOL_TxAddData(4);

	UART_PROTOCOL_TxAddData((result >> 24) & 0xFF);
	UART_PROTOCOL_TxAddData((result >> 16) & 0xFF);
	UART_PROTOCOL_TxAddData((result >> 8) & 0xFF);
	UART_PROTOCOL_TxAddData((result >> 0) & 0xFF);

	UART_PROTOCOL_TxAddFrame();
}

// ������������
void UART_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
	// �������ͷ
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

	// ����豸��ַ
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// ���������
	UART_PROTOCOL_TxAddData(cmdWord);

	// ������ݳ���
	UART_PROTOCOL_TxAddData(1);

	UART_PROTOCOL_TxAddData(result);

	UART_PROTOCOL_TxAddFrame();
}

// ���������޽��
void UART_PROTOCOL_SendCmdNoResult(uint8 cmdWord)
{
	// �������ͷ
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

	// ����豸��ַ
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// ���������
	UART_PROTOCOL_TxAddData(cmdWord);

	UART_PROTOCOL_TxAddData(0); // ���ݳ���

	UART_PROTOCOL_TxAddFrame();
}
