#include "common.h"
#include "timer.h"
#include "uartDrive.h"
#include "state.h"
#include "spiFlash.h"
#include "ArmUart.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "DtaUartProtocol.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void DTA_UART_PROTOCOL_DataStructInit(DTA_UART_PROTOCOL_CB *pCB);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void DTA_UART_PROTOCOL_RxFIFOProcess(DTA_UART_PROTOCOL_CB *pCB);

// UART����֡����������
void DTA_UART_PROTOCOL_CmdFrameProcess(DTA_UART_PROTOCOL_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL DTA_UART_PROTOCOL_CheckSUM(DTA_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL DTA_UART_PROTOCOL_ConfirmTempCmdFrameBuff(DTA_UART_PROTOCOL_CB *pCB);

// Э��㷢�ʹ������
void DTA_UART_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void DTA_UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// ������������
void DTA_UART_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result);

// ���������޽��
void DTA_UART_PROTOCOL_SendCmdNoResult(uint8 cmdWord);

//==================================================================================
// �ٶ�ƽ���˲�����
void DTA_UART_PROTOCOL_CALLBACK_SpeedFilterProcess(uint32 param);

// ������������
void DTA_UART_PROTOCOL_SendLdoV(uint8 cmdWord, uint32 result);

// ȫ�ֱ�������
DTA_UART_PROTOCOL_CB dtaUartProtocolCB;

// ��������������������������������������������������������������������������������������

// Э���ʼ��
void DTA_UART_PROTOCOL_Init(void)
{
	// Э������ݽṹ��ʼ��
	DTA_UART_PROTOCOL_DataStructInit(&dtaUartProtocolCB);

	// ��������ע�����ݽ��սӿ�
	// UART_DRIVE_RegisterDataSendService(DTA_UART_PROTOCOL_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	DTA_UART_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);
}

// UARTЭ�����̴���
void DTA_UART_PROTOCOL_Process(void)
{
	//  UART����FIFO����������
	DTA_UART_PROTOCOL_RxFIFOProcess(&dtaUartProtocolCB);

	// UART���������������
	DTA_UART_PROTOCOL_CmdFrameProcess(&dtaUartProtocolCB);

	// UARTЭ��㷢�ʹ������
	DTA_UART_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void DTA_UART_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = dtaUartProtocolCB.tx.head;
	uint16 end = dtaUartProtocolCB.tx.end;
	DTA_UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &dtaUartProtocolCB.tx.cmdQueue[dtaUartProtocolCB.tx.end];

	// ���ͻ������������������
	if ((end + 1) % DTA_UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// ��β����֡�������˳�
	if (pCmdFrame->length >= DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}
	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void DTA_UART_PROTOCOL_TxAddFrame(void)
{
	uint16 cc = 0;
	uint16 i = 0;
	uint16 head = dtaUartProtocolCB.tx.head;
	uint16 end = dtaUartProtocolCB.tx.end;
	DTA_UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &dtaUartProtocolCB.tx.cmdQueue[dtaUartProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// ���ͻ������������������
	if ((end + 1) % DTA_UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

    // ��β����֡�������˳�
    if ((length >= DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
            || (length + 2 >= DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
    {
        pCmdFrame->length = 0;

        return;
    }

	// ��β����֡�������˳�
	if (length >= DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	pCmdFrame->buff[DTA_UART_PROTOCOL_CMD_LENGTH_INDEX] = length - 3; // �������ݳ���

	for (i = 0; i < length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = cc;

	dtaUartProtocolCB.tx.end++;
	dtaUartProtocolCB.tx.end %= DTA_UART_PROTOCOL_TX_QUEUE_SIZE;

	// pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void DTA_UART_PROTOCOL_DataStructInit(DTA_UART_PROTOCOL_CB *pCB)
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
	for (i = 0; i < DTA_UART_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end = 0;
	for (i = 0; i < DTA_UART_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void DTA_UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
	uint16 end = dtaUartProtocolCB.rxFIFO.end;
	uint16 head = dtaUartProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;

	// ��������
	rxdata = *pData;

	// һ���������������������
	if ((end + 1) % DTA_UART_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// һ��������δ��������
	else
	{
		// �����յ������ݷŵ���ʱ��������
		dtaUartProtocolCB.rxFIFO.buff[end] = rxdata;
		dtaUartProtocolCB.rxFIFO.end++;
		dtaUartProtocolCB.rxFIFO.end %= DTA_UART_PROTOCOL_RX_FIFO_SIZE;
	}
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void DTA_UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
	dtaUartProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL DTA_UART_PROTOCOL_ConfirmTempCmdFrameBuff(DTA_UART_PROTOCOL_CB *pCB)
{
	DTA_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
	pCB->rx.end %= DTA_UART_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

	return TRUE;
}

// Э��㷢�ʹ������
void DTA_UART_PROTOCOL_TxStateProcess(void)
{
	uint16 head = dtaUartProtocolCB.tx.head;
	uint16 end = dtaUartProtocolCB.tx.end;
	uint16 length = dtaUartProtocolCB.tx.cmdQueue[head].length;
	uint8 *pCmd = dtaUartProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = dtaUartProtocolCB.tx.cmdQueue[head].deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == dtaUartProtocolCB.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������
	if (!(*dtaUartProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	dtaUartProtocolCB.tx.cmdQueue[head].length = 0;
	dtaUartProtocolCB.tx.head++;
	dtaUartProtocolCB.tx.head %= DTA_UART_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void DTA_UART_PROTOCOL_RxFIFOProcess(DTA_UART_PROTOCOL_CB *pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	DTA_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
		if (DTA_UART_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= DTA_UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if ((pCB->rx.end + 1) % DTA_UART_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// ���UARTͨѶ��ʱʱ������-2016.1.5����
#if DTA_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
					  DTA_UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
					  DTA_UART_PROTOCOL_CALLBACK_RxTimeOut,
					  0,
					  1,
					  ACTION_MODE_ADD_TO_QUEUE);
#endif

		// ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
		pCmdFrame->buff[pCmdFrame->length++] = currentData;
		pCB->rxFIFO.currentProcessIndex++;
		pCB->rxFIFO.currentProcessIndex %= DTA_UART_PROTOCOL_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if (pCmdFrame->length >= DTA_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if DTA_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			DTA_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0; // 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= DTA_UART_PROTOCOL_RX_FIFO_SIZE;
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
			pCB->rxFIFO.currentProcessIndex %= DTA_UART_PROTOCOL_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������

			// �����ж�����֡��С���ȣ�һ�����������������ٰ���8���ֽ�: ����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����˲���8���ֽڵıض�������
			if (pCmdFrame->length < DTA_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
			if (pCmdFrame->buff[DTA_UART_PROTOCOL_CMD_LENGTH_INDEX] > DTA_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - DTA_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
#if DTA_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				DTA_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= DTA_UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����Ϊ����֡ʵ�ʳ���
			length = pCmdFrame->length;
			if (length < pCmdFrame->buff[DTA_UART_PROTOCOL_CMD_LENGTH_INDEX] + DTA_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if (!DTA_UART_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if DTA_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				DTA_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= DTA_UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

#if DTA_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			DTA_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= DTA_UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			DTA_UART_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// �Դ��������֡����У�飬����У����
BOOL DTA_UART_PROTOCOL_CheckSUM(DTA_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
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

	// �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
	if (pCmdFrame->buff[pCmdFrame->length - 1] != cc)
	{
		return FALSE;
	}

	return TRUE;
}


void dtaUartProtocol_SendOnePacket(uint32 flashAddr, uint32 addr)
{
	uint8 addr1, addr2, addr3, addr4;
	uint8 appUpdateOnePacket[150] = {0};
	uint8 appUpdateOnePacket_i = 0;
    
	addr1 = (addr & 0xFF000000) >> 24;
	addr2 = (addr & 0x00FF0000) >> 16;
	addr3 = (addr & 0x0000FF00) >> 8;
	addr4 = (addr & 0x000000FF);
    
	SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + addr, 128); // ���߶�ȡ128�ֽ�

	DTA_UART_PROTOCOL_TxAddData(DTA_UART_PROTOCOL_CMD_HEAD); // ͷ
    DTA_UART_PROTOCOL_TxAddData(DTA_UART_CMD_DUT_BOOT_WRITE_FLASH);
	DTA_UART_PROTOCOL_TxAddData(0x00);													   // ������ʱΪ0
	DTA_UART_PROTOCOL_TxAddData(addr1);													   // ��ӵ�ַ
	DTA_UART_PROTOCOL_TxAddData(addr2);													   // ��ӵ�ַ
	DTA_UART_PROTOCOL_TxAddData(addr3);													   // ��ӵ�ַ
	DTA_UART_PROTOCOL_TxAddData(addr4);													   // ��ӵ�ַ
	for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 128; appUpdateOnePacket_i++) // �������
	{
		DTA_UART_PROTOCOL_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i]);
	}
	DTA_UART_PROTOCOL_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
}

void dtaUartProtocol_SendOnePacket_Hex(uint32 flashAddr)
{
	uint8 appUpdateOnePacket[150] = {0};
	uint8 appUpdateOnePacket_i = 0;		
    
    // app��ʼ��ַ
	SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + dut_info.currentBootSize * 116, 116); // ���߶�ȡ 4+112 ���ֽ�
	DTA_UART_PROTOCOL_TxAddData(DTA_UART_PROTOCOL_CMD_HEAD);										 // �������ͷ;//ͷ
	DTA_UART_PROTOCOL_TxAddData(DTA_UART_CMD_DUT_BOOT_WRITE_FLASH);									 // ����
	DTA_UART_PROTOCOL_TxAddData(0x00);															 // �����ʱ���ݳ���
	DTA_UART_PROTOCOL_TxAddData(0);																 // ����Ƿ����
	DTA_UART_PROTOCOL_TxAddData(0);																 // �����Կ
	DTA_UART_PROTOCOL_TxAddData(appUpdateOnePacket[0]);											 // ��ӵ�ַ
	DTA_UART_PROTOCOL_TxAddData(appUpdateOnePacket[1]);											 // ��ӵ�ַ
	DTA_UART_PROTOCOL_TxAddData(appUpdateOnePacket[2]);											 // ��ӵ�ַ
	DTA_UART_PROTOCOL_TxAddData(appUpdateOnePacket[3]);											 // ��ӵ�ַ

	for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 112; appUpdateOnePacket_i++) // �������
	{
		DTA_UART_PROTOCOL_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i + 4]);
	}
	DTA_UART_PROTOCOL_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
}

// UART����֡����������
void DTA_UART_PROTOCOL_CmdFrameProcess(DTA_UART_PROTOCOL_CB *pCB)
{
	DTA_UART_PROTOCOL_CMD cmd = DTA_UART_PROTOCOL_CMD_NULL;
	DTA_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
	if (DTA_UART_PROTOCOL_CMD_HEAD != pCmdFrame->buff[DTA_UART_PROTOCOL_CMD_HEAD_INDEX])
	{
		// ɾ������֡
		pCB->rx.head++;
		pCB->rx.head %= DTA_UART_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (DTA_UART_PROTOCOL_CMD)pCmdFrame->buff[DTA_UART_PROTOCOL_CMD_CMD_INDEX];
	
	switch (dut_info.ID)
	{
		case DUT_TYPE_NULL:
			break;

		case DUT_TYPE_CM: // ����ͨ������
		
			switch (cmd)
			{
				case DTA_UART_PROTOCOL_CMD_NULL: // = 0			         	// ������
					break;
                    
				case DTA_UART_CMD_DUT_BOOT_ERASE_FLASH: 			        // DUT_BOOT����
                    eraResual = pCmdFrame->buff[DTA_UART_PROTOCOL_CMD_DATA1_INDEX];
                    
                    // ����ʧ��
                    if (0 == eraResual)
                    {
                        // ����ʧ���ٲ���һ��
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_BOOT_EAR);
                        break;
                    }

                    // ����BOOT����
                    dut_info.currentBootSize = 0;
                    TIMER_KillTask(TIMER_ID_UPGRADE_DUT_BOOT);
                    STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_BOOT_PACKET);                
					break;
                    
				case DTA_UART_CMD_DUT_BOOT_WRITE_FLASH: 				    // DUT_BOOTд��
                    eraResual = pCmdFrame->buff[DTA_UART_PROTOCOL_CMD_DATA1_INDEX];

                    if (0 == eraResual) // ����Ǳ�д��ʧ�ܾ��˳�
                    {
                        dut_info.currentBootSize--;
                    }
                    if (dut_info.currentBootSize < dut_info.bootSize)
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_BOOT_PACKET);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_BOOT_UP_SUCCESS);
                    }                
					break;
                    
				case DTA_UART_CMD_DUT_UPDATA_FINISH: 		            	// DUT_BOOTд�����
                	STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_ITEM_FINISH);
					break;                    
			}
			break;

		default:
			break;
	}

	// ɾ������֡
	pCB->rx.head++;
	pCB->rx.head %= DTA_UART_PROTOCOL_RX_QUEUE_SIZE;
}

// ��������ظ�����һ������
void DTA_UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
	DTA_UART_PROTOCOL_TxAddData(0x55);
	DTA_UART_PROTOCOL_TxAddData(ackCmd);
	DTA_UART_PROTOCOL_TxAddData(0x01);
	DTA_UART_PROTOCOL_TxAddData(ackParam);
	DTA_UART_PROTOCOL_TxAddFrame();
}

// RXͨѶ��ʱ����-����
#if DTA_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void DTA_UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	DTA_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	pCmdFrame = &dtaUartProtocolCB.rx.cmdQueue[dtaUartProtocolCB.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0; // 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	dtaUartProtocolCB.rxFIFO.head++;
	dtaUartProtocolCB.rxFIFO.head %= DTA_UART_PROTOCOL_RX_FIFO_SIZE;
	dtaUartProtocolCB.rxFIFO.currentProcessIndex = dtaUartProtocolCB.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void DTA_UART_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

// ������������·����������־
void DTA_UART_PROTOCOL_SetTxAtOnceRequest(uint32 param)
{
	dtaUartProtocolCB.txAtOnceRequest = (BOOL)param;
}

// ������������·����������־
void DTA_UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param)
{
	dtaUartProtocolCB.txPeriodRequest = (BOOL)param;
}

// 	UART���߳�ʱ������
void DTA_UART_PROTOCOL_CALLBACK_UartBusError(uint32 param)
{
}

// ������������
void DTA_UART_PROTOCOL_SendLdoV(uint8 cmdWord, uint32 result)
{
	// �������ͷ
	DTA_UART_PROTOCOL_TxAddData(DTA_UART_PROTOCOL_CMD_HEAD);

	// ���������
	DTA_UART_PROTOCOL_TxAddData(cmdWord);

	// ������ݳ���
	DTA_UART_PROTOCOL_TxAddData(4);

	DTA_UART_PROTOCOL_TxAddData((result >> 24) & 0xFF);
	DTA_UART_PROTOCOL_TxAddData((result >> 16) & 0xFF);
	DTA_UART_PROTOCOL_TxAddData((result >> 8) & 0xFF);
	DTA_UART_PROTOCOL_TxAddData((result >> 0) & 0xFF);

	DTA_UART_PROTOCOL_TxAddFrame();
}

// ������������
void DTA_UART_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
	// �������ͷ
	DTA_UART_PROTOCOL_TxAddData(DTA_UART_PROTOCOL_CMD_HEAD);

	// ���������
	DTA_UART_PROTOCOL_TxAddData(cmdWord);

	// ������ݳ���
	DTA_UART_PROTOCOL_TxAddData(1);

	DTA_UART_PROTOCOL_TxAddData(result);

	DTA_UART_PROTOCOL_TxAddFrame();
}

// ���������޽��
void DTA_UART_PROTOCOL_SendCmdNoResult(uint8 cmdWord)
{
	// �������ͷ
	DTA_UART_PROTOCOL_TxAddData(DTA_UART_PROTOCOL_CMD_HEAD);

	// ���������
	DTA_UART_PROTOCOL_TxAddData(cmdWord);

	DTA_UART_PROTOCOL_TxAddData(0); // ���ݳ���

	DTA_UART_PROTOCOL_TxAddFrame();
}
