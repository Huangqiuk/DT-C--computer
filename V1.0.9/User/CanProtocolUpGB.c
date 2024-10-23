#include "common.h"
#include "timer.h"
#include "CanProtocolUpDT.h"
#include "canProtocolUpGB.h"
#include "spiflash.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "state.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

CAN_PROTOCOL_PARAM_RX_CB canRxDriveCB;

// ���ݽṹ��ʼ��
void CAN_PROTOCOL_DataStructInit(CAN_PROTOCOL_CB *pCB);

// CAN ���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_PROTOCOL_MacProcess(uint32 standarID, uint8 *pData, uint8 length);

// һ�����Ľ��ջ���������������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_PROTOCOL_RxFIFOProcess(CAN_PROTOCOL_CB *pCB);

// CAN����֡����������
void CAN_PROTOCOL_CmdFrameProcess(CAN_PROTOCOL_RX *pCB);

// �Դ��������֡����У�飬����У����
BOOL CAN_PROTOCOL_CheckSum(uint8 *buff, uint32 len);

// �Դ��������֡����У�飬����У����
BOOL CAN_PROTOCOL_AddSum(uint8 *buff, uint32 len);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_PROTOCOL_CB *pCB, uint8 sourceSelect);

// Э��㷢�ʹ������
void CAN_PROTOCOL_TxStateProcess(void);

// CANЭ�����������ע�����ݷ��ͽӿ�
void CAN_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length));

//===============================================================================================================

// ������������·����������־
void CAN_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param);

// ���������ͨѶʱ����
void CAN_PROTOCOL_DataCommunicationSequenceProcess(void);

// 	CAN	����BMS��ʱ������
void CAN_PROTOCOL_CALLBACK_BmsCanBusError(uint32 param);

// 	CAN	����ECU��ʱ������
void CAN_PROTOCOL_CALLBACK_EcuCanBusError(uint32 param);

// ����ͨѶ��ʱ�ж�����
void CAN_PROTOCOL_StartTimeoutCheckTask(void);

// CAN���߳�ʱ������
void CAN_PROTOCOL_CALLBACK_CanBusError(uint32 param);

//==================================================================================
// ƽ���������
void CAN_PROTOCOL_CALLBACK_SmoothMechanismProcess(uint32 param);

// ����Ϳ��Ƶ��ָ��D0 ģʽ D1 ��λ D2 ��� ���� D3 ���������	100ms
void CMD_CAN_HmiToDriverSendData(uint32 param);

// ת���
void CMD_CAN_HmiTurnToDriverSendData(uint32 param);

// ���ò���״̬ 0x182A1828
void CMD_CAN_HmiFactoryAck(uint32 param);

// ������������
void CAN_SendRidingParam(uint32 param);

// ��λ����
void CAN_SendAssistNameParam(uint32 param);

// ������Ϣ
void CAN_SendErrorInfoParam(uint32 param);

// �汾��Ϣ
void CAN_SendHmiVersionParam(uint32 param);

// �Ǳ����к�
void CAN_SendHmiSnParam(uint32 param);

//=======================================================================================
// ȫ�ֱ�������
CAN_PROTOCOL_CB canProtocolCB;
CAN_PROTOCOL_RX canProtocolRX;

// CANЭ����ʼ��
void CAN_PROTOCOL_Init(void)
{
	// Э������ݽṹ��ʼ��
	CAN_PROTOCOL_DataStructInit(&canProtocolCB);

	// ��������ע�����ݽ��սӿ�
	//	CAN_DRIVE_RegisterDataSendService(CAN_PROTOCOL_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	CAN_PROTOCOL_RegisterDataSendService(CAN_DRIVE_AddTxArray);

	// ������ϣ����buff
	memset(canProtocolCB.sendBuff, 0xFF, 200);
	//	TIMER_AddTask(TIMER_ID_CAN_TEST,
	//					100,
	//					CAN_UP_RESET,
	//					1,
	//					TIMER_LOOP_FOREVER,
	//					ACTION_MODE_ADD_TO_QUEUE);
	//	CAN_UP_RESET(1);
}

// ��������֡���������֡Դ��֡Ŀ��
void CAN_PROTOCOL_TxAddNewFrame(uint8 priority, uint16 pgn, uint8 destinationID, uint8 sourceID)
{
	CAN_PROTOCOL_CMD_FRAME *pCmdFrame = &canProtocolCB.tx.cmdQueue[canProtocolCB.tx.end];

	// ֡Դ��֡Ŀ�ĳ�����Χ
	if ((sourceID > 0xFF) || (destinationID > 0xFF))
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	// pCmdFrame->deviceID = ((sourceID << 4) | destinationID);

	pCmdFrame->EXTID.sDet.nHeard = 0;				   // 3bit:000
	pCmdFrame->EXTID.sDet.priority = priority;		   // 3bit:���ȼ�
	pCmdFrame->EXTID.sDet.pgn = pgn;				   // 10bit:pgn
	pCmdFrame->EXTID.sDet.destination = destinationID; // 8bit:Ŀ�ĵ�ַ
	pCmdFrame->EXTID.sDet.source = sourceID;		   // 8bit:Դ��ַ
}

// ��������֡�������������
void CAN_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = canProtocolCB.tx.head;
	uint16 end = canProtocolCB.tx.end;
	CAN_PROTOCOL_CMD_FRAME *pCmdFrame = &canProtocolCB.tx.cmdQueue[canProtocolCB.tx.end];

	// ���ͻ������������������
	if ((end + 1) % CAN_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// ��β����֡�������˳�
	if (pCmdFrame->length >= CAN_PROTOCOL_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length++;
}

// ��������֡����,�Զ�����֡�����Լ�������
void CAN_PROTOCOL_TxAddFrame(void)
{
	uint16 head = canProtocolCB.tx.head;
	uint16 end = canProtocolCB.tx.end;

	// ���ͻ������������������
	if ((end + 1) % CAN_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	canProtocolCB.tx.end++;
	canProtocolCB.tx.end %= CAN_PROTOCOL_TX_QUEUE_SIZE;
	canProtocolCB.tx.cmdQueue[canProtocolCB.tx.end].length = 0; // 2015.12.2�޸�
}

// CANЭ�����̴���
void CAN_PROTOCOL_Process_GB(void)
{
	// CAN����FIFO����������
	// CAN_PROTOCOL_RxFIFOProcess(&canProtocolCB);

	// CAN���������������
	CAN_PROTOCOL_CmdFrameProcess(&canProtocolRX);

	// CANЭ��㷢�ʹ������
	CAN_PROTOCOL_TxStateProcess();
}

// ���ݽṹ��ʼ��
void CAN_PROTOCOL_DataStructInit(CAN_PROTOCOL_CB *pCB)
{
	uint16 i;
	uint16 j;

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}

	pCB->tx.txBusy = FALSE;
	pCB->tx.index = 0;
	pCB->tx.head = 0;
	pCB->tx.end = 0;
	for (i = 0; i < CAN_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	for (i = 0; i < CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX; i++)
	{
		pCB->rxFIFO.rxFIFOEachNode[i].head = 0;
		pCB->rxFIFO.rxFIFOEachNode[i].end = 0;
		pCB->rxFIFO.rxFIFOEachNode[i].currentProcessIndex = 0;
	}

	for (i = 0; i < CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX; i++)
	{
		for (j = 0; j < CAN_PROTOCOL_RX_QUEUE_SIZE; j++)
		{
			//		pCB->rx.rxEachNode[i].cmdQueue[j].length = 0;
		}
		pCB->rx.rxEachNode[i].head = 0;
		pCB->rx.rxEachNode[i].end = 0;
	}

	memset(canProtocolCB.sendBuff, 0xFF, 128);
	canProtocolCB.tripData.distance = 0;
	canProtocolCB.tripData.times = 0;
	canProtocolCB.hmiState = 0;
	canProtocolCB.UPDATA1 = TRUE;
	canProtocolCB.UPDATA = TRUE;
}

// CAN ���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_PROTOCOL_MacProcess(uint32 standarID, uint8 *pData, uint8 length)
{
	uint8 i = 0;

	canProtocolRX.cmdQueue[canProtocolRX.end].uExtId.ExtId = standarID;
	for (i = 0; i < length; i++)
	{
		canProtocolRX.cmdQueue[canProtocolRX.end].Data[i] = *pData++;
	}
	canProtocolRX.cmdQueue[canProtocolRX.end].DLC = length;
	canProtocolRX.end++;
	canProtocolRX.end %= CAN_PROTOCOL_RX_QUEUE_SIZE;
}

// һ�����Ľ��ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_PROTOCOL_RxFIFOProcess(CAN_PROTOCOL_CB *pCB)
{
}

// CAN����֡����������
void CAN_PROTOCOL_CmdFrameProcess(CAN_PROTOCOL_RX *pCB)
{
	if (pCB->head == pCB->end)
	{
		return;
	}

	switch (pCB->cmdQueue[pCB->head].uExtId.ExtId)
	{
	case 0x1801FF28: // �Ǳ��״̬
	
		// ���������Ǵ�app����boot��ʼ����
		if (!canProtocolCB.UPDATA1) // ������־λ
		{
			// 01��ʾ��ǰ��app
			if (0x01 == (pCB->cmdQueue[pCB->head].Data[0] & 0x01))
			{
				// �����Ҫ����������boot
				if (!canProtocolCB.UPDATA)
				{
					canProtocolCB.UPDATA = TRUE;
					// �л�������boot����
					STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_SEND_BOOT_CMD);
				}
			}
			else // ��boot�У����͵�һ�����������ݰ���������ʼ
			{
				canProtocolCB.UPDATA1 = TRUE;
				STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY);
			}
		}
		break;

	case 0x18ECF927: // ���Ͷ������
					 // 0xEF��һ��132Byte���ݣ�4����ַ��128�����ݡ�0x13����ǰ���ݰ��Ǳ������ɣ�
		if ((0xEF == pCB->cmdQueue[pCB->head].Data[6]) && (0x13 == pCB->cmdQueue[pCB->head].Data[0]))
		{
			// 0x01��У��ͨ����������һ�����ݰ�
			if (0x01 == pCB->cmdQueue[pCB->head].Data[4])
			{
				//				CAN_UP_DATA(1);
				STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY);
			}
			else
			{
				// У�鲻ͨ��
				STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WRITE_ERROR);
			}
		}
		// ����PGN���������������ݣ�0x11���Ǳ������߷�������
		else if ((0xEF == pCB->cmdQueue[pCB->head].Data[6]) && (0x11 == pCB->cmdQueue[pCB->head].Data[0]))
		{
			// STATE_PGN_SEND_2�����߷�������
			STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_SEND_DATA);
			// CAN_PGN_RequestSendData(STATE_PGN_SEND_2, pCB->cmdQueue[pCB->head].Data[6], 0x27, 0xF9);
		}

		break;

	default:
		break;
	}

	pCB->head++;
	pCB->head %= CAN_PROTOCOL_RX_QUEUE_SIZE;
}

// �Դ��������֡����У�飬����У����
BOOL CAN_PROTOCOL_CheckSum(uint8 *buff, uint32 len)
{
	uint8 cc = 0;
	uint16 i = 0;

	if (NULL == buff)
	{
		return FALSE;
	}

	// ������γ��ȿ�ʼ(�ڵ�ID�Ѿ��ڽ���ǰȥ���ˣ�����iΪ3)����У����֮ǰ��һ���ֽڣ����ν����������
	for (i = CAN_PROTOCOL_RX_CMD_LENGTH_INDEX; i < (len - CAN_PROTOCOL_CHECK_BYTE - CAN_PROTOCOL_TAIL_BYTE); i++)
	{
		cc ^= buff[i];
	}

	// �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
	if (buff[len - 2] != cc)
	{
		return FALSE;
	}

	return TRUE;
}

// �Դ��������֡����У�飬����У����
BOOL CAN_PROTOCOL_AddSum(uint8 *buff, uint32 len)
{
	uint8 cc = 0;
	uint16 i = 0;

	if (NULL == buff)
	{
		return FALSE;
	}

	// ������γ��ȿ�ʼ(�ڵ�ID�Ѿ����������ˣ�����iΪ3)����У����֮ǰ��һ���ֽڣ����ν����������
	for (i = CAN_PROTOCOL_TX_CMD_LENGTH_INDEX; i < len; i++)
	{
		cc ^= buff[i];
	}

	// ���У����
	buff[len] = cc;

	return TRUE;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_PROTOCOL_CB *pCB, uint8 sourceSelect)
{
	CAN_PROTOCOL_CMD_FRAME *pCmdFrame = NULL;

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return FALSE;
	}

	// ��ʱ������Ϊ�գ��������
	//	pCmdFrame = &pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end];

	if (0 == pCmdFrame->length)
	{
		return FALSE;
	}

	// ���
	pCB->rx.rxEachNode[sourceSelect].end++;
	pCB->rx.rxEachNode[sourceSelect].end %= CAN_PROTOCOL_RX_QUEUE_SIZE;
	// ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
	//	pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end].length = 0;

	return TRUE;
}

// Э��㷢�ʹ������
void CAN_PROTOCOL_TxStateProcess(void)
{
	uint16 head = canProtocolCB.tx.head;
	uint16 end = canProtocolCB.tx.end;
	uint16 length = canProtocolCB.tx.cmdQueue[head].length;
	uint8 *pCmd = canProtocolCB.tx.cmdQueue[head].buff;
	uint32 localDeviceID = canProtocolCB.tx.cmdQueue[head].EXTID.deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == canProtocolCB.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������
	if (!(*canProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	canProtocolCB.tx.cmdQueue[head].length = 0;
	canProtocolCB.tx.head++;
	canProtocolCB.tx.head %= CAN_PROTOCOL_TX_QUEUE_SIZE;
}

// CANЭ�����������ע�����ݷ��ͽӿ�
void CAN_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length))
{
	canProtocolCB.sendDataThrowService = service;
}

// state : ��ǰ״̬   DestAddress:Ŀ�ĵ�ַ   SourceAddress:Դ��ַ
void CAN_PGN_RequestSendData(STATE_PGN state, uint8 PGN, uint8 DestAddress, uint8 SourceAddress)
{
	uint16 dataLength = 0, dataNumber = 0;
	uint8 number = 1;

	canProtocolCB.preState = canProtocolCB.state;
	canProtocolCB.state = state;

	switch (PGN)
	{
	// ���������ݴ�С
	case 0xEF:

		// ���͵��ֽ���
		dataLength = 132;

		// ���͵��ܰ���
		dataNumber = 19;

		break;

	// �·��ܳɱ���
	case 0xED:

		// ���͵��ֽ���
		dataLength = 43;

		// ���͵��ܰ���
		dataNumber = 7;
		break;

	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
		// ���͵��ֽ���
		dataLength = 23;

		// ���͵��ܰ���
		dataNumber = 4;
		break;
	default:
		break;
	}

	switch (state)
	{
	case STATE_PGN_NULL:

		break;

	// ����PGN���ݵ�һ��:������
	case STATE_PGN_SEND_1:

		CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEC, DestAddress, SourceAddress);

		CAN_PROTOCOL_TxAddData(0x10);

		CAN_PROTOCOL_TxAddData(dataLength & 0xFF);

		CAN_PROTOCOL_TxAddData((dataLength >> 8) & 0xFF);

		CAN_PROTOCOL_TxAddData(dataNumber);

		CAN_PROTOCOL_TxAddData(0xFF);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(PGN);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
		break;

	// ����PGN���ݵڶ���:Ŀ�ĵ�ַ��Ӧ������·���Ӧ������
	case STATE_PGN_SEND_2:

		for (number = 1; number <= dataNumber; number++)
		{
			CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEB, DestAddress, SourceAddress);

			CAN_PROTOCOL_TxAddData(number);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 0]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 1]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 2]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 3]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 4]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 5]);

			CAN_PROTOCOL_TxAddData(canProtocolCB.sendBuff[((number - 1) * 7) + 6]);

			CAN_PROTOCOL_TxAddFrame();
		}

		break;
	default:
		break;
	}
}

// state : ��ǰ״̬   DestAddress:Ŀ�ĵ�ַ   SourceAddress:Դ��ַ
void CAN_PGN_RequestGetData(STATE_PGN state, uint8 PGN, uint8 DestAddress, uint8 SourceAddress)
{
	uint16 dataLength = 0, dataNumber = 0;

	canProtocolCB.preState = canProtocolCB.state;
	canProtocolCB.state = state;

	switch (PGN)
	{
	// �������ݰ���С�·�
	case 0x50:
		// ���͵��ֽ���
		dataLength = 28;

		// ���͵��ܰ���
		if (dataLength % 7)
		{
			dataNumber = (dataLength / 7) + 1;
		}
		else
		{
			dataNumber = dataLength / 7;
		}

		break;

	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
		// ���͵��ֽ���
		dataLength = 23;

		// ���͵��ܰ���
		if (dataLength % 7)
		{
			dataNumber = (dataLength / 7) + 1;
		}
		else
		{
			dataNumber = dataLength / 7;
		}
		break;

	default:
		break;
	}

	switch (state)
	{
	case STATE_PGN_NULL:

		break;

	// ����PGN���� ��һ��:��������PGN  EA00
	case STATE_PGN_GET_1:

		CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEA, DestAddress, SourceAddress);

		CAN_PROTOCOL_TxAddData(0x00);

		CAN_PROTOCOL_TxAddData(PGN);

		CAN_PROTOCOL_TxAddData(0x00);

		CAN_PROTOCOL_TxAddFrame();
		break;

	// ����PGN���� �ڶ���:��������PGN  EC00
	case STATE_PGN_GET_2:

		CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEC, DestAddress, SourceAddress);

		CAN_PROTOCOL_TxAddData(0x11); // ׼����

		CAN_PROTOCOL_TxAddData(dataNumber);

		CAN_PROTOCOL_TxAddData(0x01);

		CAN_PROTOCOL_TxAddData(0xFF);

		CAN_PROTOCOL_TxAddData(0xFF);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(PGN);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
		break;

	// ����PGN���� ������:��������PGN  EC00
	case STATE_PGN_GET_3:

		CAN_PROTOCOL_TxAddNewFrame(0x06, 0xEC, DestAddress, SourceAddress);

		CAN_PROTOCOL_TxAddData(0x13); // ����Ӧ��

		CAN_PROTOCOL_TxAddData(dataLength & 0xFF); // 	�ֽ���

		CAN_PROTOCOL_TxAddData((dataLength >> 8) & 0xFF);

		CAN_PROTOCOL_TxAddData(dataNumber); // 	����

		CAN_PROTOCOL_TxAddData(0x01); // ����У��ɹ�

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(PGN);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
		break;

	default:
		break;
	}
}

void CAN_UP_DATA(uint32 param)
{
	dut_info.currentUiSize++;
	// ����������
	if (dut_info.currentUiSize > dut_info.uiSize)
	{
		STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_FINISH);
		return;
	}
	// ����buff��ӵ�ַ���
	canProtocolCB.sendBuff[0] = ((dut_info.currentUiSize >> 24) & 0xFF);
	canProtocolCB.sendBuff[1] = ((dut_info.currentUiSize >> 16) & 0xFF);
	canProtocolCB.sendBuff[2] = ((dut_info.currentUiSize >> 8) & 0xFF);
	canProtocolCB.sendBuff[3] = (dut_info.currentUiSize & 0xFF);
	// ����buff�������
	SPI_FLASH_ReadArray(&canProtocolCB.sendBuff[4], (SPI_FLASH_DUT_UI_ADDEESS + (dut_info.currentUiSize - 1) * 128), 128);
	// STATE_PGN_SEND_1�����������Ǳ��������
	CAN_PGN_RequestSendData(STATE_PGN_SEND_1, 0xEF, 0x27, 0xF9);
}

void CAN_UP_RESET(uint32 param)
{
	if (1 == param)
	{
		CAN_PROTOCOL_TxAddNewFrame(0x06, 0x01, 0xFF, 0xF9);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
	}
	else if (2 == param)
	{
		CAN_PROTOCOL_TxAddNewFrame(0x06, 0x01, 0xFF, 0xF9);

		CAN_PROTOCOL_TxAddData(0x40);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0x02);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddData(0);

		CAN_PROTOCOL_TxAddFrame();
	}
}
