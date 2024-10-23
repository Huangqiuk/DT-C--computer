#include "common.h"
#include "ArmUart.h"
#include "ArmProtocol.h"
#include "timer.h"
// #include "param.h"
#include "State.h"
#include "Spiflash.h"
#include "system.h"
#include "iap.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "CanProtocolUpGB.h"
#include "state.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void ARM_PROTOCOL_DataStructInit(ARM_PROTOCOL_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void ARM_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void ARM_PROTOCOL_RxFIFOProcess(ARM_PROTOCOL_CB *pCB);

// UART����֡����������
void ARM_PROTOCOL_CmdFrameProcess(ARM_PROTOCOL_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL ARM_PROTOCOL_CheckSUM(ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL ARM_PROTOCOL_ConfirmTempCmdFrameBuff(ARM_PROTOCOL_CB *pCB);

// Э��㷢�ʹ������
void ARM_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void ARM_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// ��������ظ�
void ARM_PROTOCOL_SendCmdAck(uint8 ackCmd);

// ���ʹ�Ӧ��Ļظ�
void ARM_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// �Ǳ�״̬��Ϣ�ظ�
void ARM_PROTOCOL_SendCmdStatusAck(void);

//=======================================================================================
// ȫ�ֱ�������
ARM_PROTOCOL_CB armProtocolCB;

// Э���ʼ��
void ARM_PROTOCOL_Init(void)
{
	// Э������ݽṹ��ʼ��
	ARM_PROTOCOL_DataStructInit(&armProtocolCB);

	// ��������ע�����ݽ��սӿ�
	ARM_UART_RegisterDataSendService(ARM_PROTOCOL_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	ARM_PROTOCOL_RegisterDataSendService(ARM_UART_AddTxArray);
}

// ARMЭ�����̴���
void ARM_PROTOCOL_Process(void)
{
	// ARM����FIFO����������
	ARM_PROTOCOL_RxFIFOProcess(&armProtocolCB);

	// ARM���������������
	ARM_PROTOCOL_CmdFrameProcess(&armProtocolCB);

	// ARMЭ��㷢�ʹ������
	ARM_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void ARM_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = armProtocolCB.tx.head;
	uint16 end = armProtocolCB.tx.end;
	ARM_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &armProtocolCB.tx.cmdQueue[armProtocolCB.tx.end];

	// ���ͻ������������������
	if ((end + 1) % ARM_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// ��β����֡�������˳�
	if (pCmdFrame->length >= ARM_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void ARM_PROTOCOL_TxAddFrame(void)
{
	uint16 cc = 0;
	uint16 i = 0;
	uint16 head = armProtocolCB.tx.head;
	uint16 end = armProtocolCB.tx.end;
	ARM_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &armProtocolCB.tx.cmdQueue[armProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// ���ͻ������������������
	if ((end + 1) % ARM_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// ����֡���Ȳ��㣬������������ݣ��˳�
	if (ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length) // ��ȥ"У���"1���ֽ�
	{
		pCmdFrame->length = 0;

		return;
	}

	// ��β����֡�������˳�
	if (length >= ARM_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] = length - 3; // �������ݳ���

	for (i = 0; i < length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = ~cc;

	armProtocolCB.tx.end++;
	armProtocolCB.tx.end %= ARM_PROTOCOL_TX_QUEUE_SIZE;
	// pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void ARM_PROTOCOL_DataStructInit(ARM_PROTOCOL_CB *pCB)
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
	for (i = 0; i < ARM_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end = 0;
	for (i = 0; i < ARM_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}

	pCB->isTimeCheck = FALSE;
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void ARM_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
	uint16 end = armProtocolCB.rxFIFO.end;
	uint16 head = armProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;

	// ��������
	rxdata = *pData;

	// һ���������������������
	if ((end + 1) % ARM_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// һ��������δ��������
	else
	{
		// �����յ������ݷŵ���ʱ��������
		armProtocolCB.rxFIFO.buff[end] = rxdata;
		armProtocolCB.rxFIFO.end++;
		armProtocolCB.rxFIFO.end %= ARM_PROTOCOL_RX_FIFO_SIZE;
	}
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void ARM_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
	armProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL ARM_PROTOCOL_ConfirmTempCmdFrameBuff(ARM_PROTOCOL_CB *pCB)
{
	ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
	pCB->rx.end %= ARM_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

	return TRUE;
}

// Э��㷢�ʹ������
void ARM_PROTOCOL_TxStateProcess(void)
{
	uint16 head = armProtocolCB.tx.head;
	uint16 end = armProtocolCB.tx.end;
	uint16 length = armProtocolCB.tx.cmdQueue[head].length;
	uint8 *pCmd = armProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = armProtocolCB.tx.cmdQueue[head].deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == armProtocolCB.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������
	if (!(*armProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	armProtocolCB.tx.cmdQueue[head].length = 0;
	armProtocolCB.tx.head++;
	armProtocolCB.tx.head %= ARM_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void ARM_PROTOCOL_RxFIFOProcess(ARM_PROTOCOL_CB *pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
		if (ARM_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if ((pCB->rx.end + 1) % ARM_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// ���UARTͨѶ��ʱʱ������-2016.1.5����
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
					  BLE_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
					  BLE_PROTOCOL_CALLBACK_RxTimeOut,
					  0,
					  1,
					  ACTION_MODE_ADD_TO_QUEUE);
#endif

		// ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
		pCmdFrame->buff[pCmdFrame->length++] = currentData;
		pCB->rxFIFO.currentProcessIndex++;
		pCB->rxFIFO.currentProcessIndex %= ARM_PROTOCOL_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if (pCmdFrame->length >= ARM_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0; // 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
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
			pCB->rxFIFO.currentProcessIndex %= ARM_PROTOCOL_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ���������� ����

			// �����ж�����֡��С���ȣ�һ�����������������ٰ���4���ֽ�: ����֡��С���ȣ�����:����ͷ�������֡����ݳ��ȡ�У��ͣ���˲���4���ֽڵıض�������
			if (pCmdFrame->length < ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
			if (pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] > (ARM_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN))
			{
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// ����֡����У��
			length = pCmdFrame->length;
			if (length < pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] + ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if (!ARM_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			BLE_PROTOCOL_StopRxTimeOutCheck();
#endif
			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			ARM_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// �Դ��������֡����У�飬����У����
BOOL ARM_PROTOCOL_CheckSUM(ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
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

void dutProgressReport(uint32 temp)
{
	uint8 i;

	if (dut_info.appUpFlag)
	{
		dut_info.dutProgress[3] = (dut_info.currentAppSize * 100) / dut_info.appSize;
	}
	else if (dut_info.uiUpFlag)
	{
		dut_info.dutProgress[5] = (dut_info.currentUiSize * 100) / dut_info.uiSize;
	}
	else if (dut_info.configUpFlag)
	{
		// ������Ϣ����ֻ��0%��100%��
	}
	else
	{
		// �������������
		TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
		TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);
		dut_info.configUpFaile = FALSE;
		dut_info.appUpFaile = FALSE;
		dut_info.uiUpFaile = FALSE;
	}

	if (dut_info.appUpFaile) // app����ʧ��
	{
		dut_info.appUpFaile = FALSE;
		dut_info.dutProgress[3] = 102;
		dut_info.currentAppSize = 0;
		TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
		dut_info.appUpFlag = FALSE;
	}
	
	if (dut_info.uiUpFaile) // ui����ʧ��
	{
		dut_info.dutProgress[5] = 102;
		dut_info.currentUiSize = 0;
		TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
		TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION); // �߱�ĳ�ʱ���
		dut_info.uiUpFaile = FALSE;
		dut_info.uiUpFlag = FALSE;
	}	
	
	if (dut_info.configUpFaile) // config����ʧ��
	{
		dut_info.dutProgress[7] = 102;
		TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
		dut_info.configUpFaile = FALSE;
		dut_info.configUpFlag = FALSE;
	}
	
	if (dut_info.appUpSuccesss)
	{
		dut_info.dutProgress[3] = 100;
		dut_info.currentAppSize = 0;
		dut_info.appUpSuccesss = FALSE;
	}

	if (dut_info.uiUpSuccesss)
	{
		dut_info.dutProgress[5] = 100;
		dut_info.currentUiSize = 0;
		dut_info.uiUpSuccesss = FALSE;
	}

	if (dut_info.configUpSuccesss)
	{
		dut_info.dutProgress[7] = 100;
		dut_info.configUpSuccesss = FALSE;
	}
	ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);		   // �������ͷ
	ARM_PROTOCOL_TxAddData(UART_ARM_CMD_TOOL_DUT_PROCESS); // ���������0x15
	ARM_PROTOCOL_TxAddData(0x00);						   // ���ݳ�����ʱΪ0
	for (i = 0; i < 8; i++)
	{
		ARM_PROTOCOL_TxAddData(dut_info.dutProgress[i]);
	}
	ARM_PROTOCOL_TxAddFrame(); // �������ݳ��Ⱥ����У��
}

// ���������
void setUpItem(uint8 upItem, uint8 ctrlState)
{
	dut_info.appUpFlag = FALSE;
	dut_info.uiUpFlag = FALSE;
	dut_info.configUpFlag = FALSE;
	// ���״̬
	dut_info.currentAppSize = 0;
	dut_info.currentUiSize = 0;
	if (1 == ctrlState)
	{
		if (0x02 == (upItem & 0x02)) // ����app
		{
			dut_info.appUpFlag = TRUE;
		}

		if (0x04 == (upItem & 0x04)) // ����ui
		{
			dut_info.uiUpFlag = TRUE;
		}

		if (0x08 == (upItem & 0x08)) // ����config
		{
			dut_info.configUpFlag = TRUE;
		}
	}
}

// ���ý����ϱ���Ϣ
void setDutProgressReportInfo()
{
	dut_info.dutProgress[0] = 0;   // boot
	dut_info.dutProgress[1] = 101; // boot
	dut_info.dutProgress[2] = 1;   // app
	dut_info.dutProgress[4] = 2;   // ui
	dut_info.dutProgress[6] = 3;   // config

	if (!dut_info.appUpFlag)
	{
		dut_info.dutProgress[3] = 101;
	}
	else
	{
		dut_info.dutProgress[3] = 0;
	}

	if (!dut_info.uiUpFlag)
	{
		dut_info.dutProgress[5] = 101;
	}
	else
	{
		dut_info.dutProgress[5] = 0;
	}

	if (!dut_info.configUpFlag)
	{
		dut_info.dutProgress[7] = 101;
	}
	else
	{
		dut_info.dutProgress[7] = 0;
	}

	// �����ϱ�
	TIMER_AddTask(TIMER_ID_SET_DUT_PROGRESS_REPORT,
				  300,
				  dutProgressReport,
				  TRUE,
				  TIMER_LOOP_FOREVER,
				  ACTION_MODE_ADD_TO_QUEUE);
}

// ����������Ϣ���벻ͬ��״̬��
void enterState()
{
	if (dut_info.appUpFlag || dut_info.uiUpFlag || dut_info.configUpFlag)
	{
		switch (dut_info.dutBusType)
		{
		case 0: // ����
			switch (dut_info.ID)
			{
			case DUT_TYPE_SEG:
				// ��������ܡ�����������
				STATE_EnterState(STATE_CM_UART_SEG_UPGRADE);
				break;
			case DUT_TYPE_CM:
				// ����ͨ�ò�������
				STATE_EnterState(STATE_CM_UART_BC_UPGRADE);
				break;
			case DUT_TYPE_HUAXIN:
				// ���뻪о΢������
				STATE_EnterState(STATE_HUAXIN_UART_BC_UPGRADE);
				break;
			case DUT_TYPE_KAIYANG:
				// ���뿪������
				STATE_EnterState(STATE_KAIYANG_UART_BC_UPGRADE);
				break;
			default:
				break;
			}

			break;

		case 1: // can
			switch (dut_info.ID)
			{
			case DUT_TYPE_GB:
				// ����߱�����
				STATE_EnterState(STATE_GAOBIAO_CAN_UPGRADE);
				break;

			case DUT_TYPE_SEG:
				// ��������ܡ�����������
				STATE_EnterState(STATE_SEG_CAN_UPGRADE);
				break;

			case DUT_TYPE_HUAXIN:
				// ��о΢������
				STATE_EnterState(STATE_HUAXIN_CAN_UPGRADE);
				break;

			case DUT_TYPE_KAIYANG:
				// ���뿪������
				STATE_EnterState(STATE_KAIYANG_CAN_UPGRADE);
				break;

			default:
				// ����ͨ��can����
				STATE_EnterState(STATE_CM_CAN_UPGRADE);
				break;
			}
			break;

		default:
			break;
		}
	}
	else
	{
		STATE_EnterState(STATE_STANDBY);
	}
}

// ARM����֡����������
void ARM_PROTOCOL_CmdFrameProcess(ARM_PROTOCOL_CB *pCB)
{
	ARM_PROTOCOL_CMD cmd = ARM_CMD_MAX;
	ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	BOOL writeRes;

	// �ҵı���
	uint8 versionBoot[3] = {0};
	uint8 versionApp[3] = {0};

	uint8 upItem = 0;
	uint8 clearItem = 0;
	uint8 dutAll[50] = {0};
	uint8 dutName_i = 0;
	uint8 packetSize = 0;
	uint8 addr1 = 0;
	uint8 addr2 = 0;
	uint8 addr3 = 0;
	uint8 addr4 = 0;
	uint32 writeAddr = 0;
	static uint32 allPacketSize = 0;
	static uint8 allPacket[4] = {0};
	uint8 dataLen = 0;
	uint8 endItem = 0;
	uint8 ctrlState = 0;
	uint8 dut_info_len = 0;
	uint8 size[4] = {0};
	static uint32 currPacketNum = 0xFFFF;

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
	if (ARM_PROTOCOL_CMD_HEAD != pCmdFrame->buff[ARM_PROTOCOL_CMD_HEAD_INDEX])
	{
		// ɾ������֡
		pCB->rx.head++;
		pCB->rx.head %= ARM_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (ARM_PROTOCOL_CMD)pCmdFrame->buff[ARM_PROTOCOL_CMD_CMD_INDEX];

	// ִ������֡
	switch (cmd)
	{
	// ��ȡ��¼��������Ϣ
	case UART_ARM_CMD_GET_TOOL_INFO:										 // 0x10,
		SPI_FLASH_ReadArray(versionBoot, SPI_FLASH_BOOT_VERSION_ADDEESS, 3); // ��ȡBOOT�汾
		SPI_FLASH_ReadArray(versionApp, SPI_FLASH_APP_VERSION_ADDEESS, 3);	 // ��ȡAPP�汾
		ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);						 // �������ͷ
		ARM_PROTOCOL_TxAddData(UART_ARM_CMD_GET_TOOL_INFO);					 // ���������
		ARM_PROTOCOL_TxAddData(0x00);										 // ���ݳ�����ʱΪ0
		ARM_PROTOCOL_TxAddData(versionBoot[0]);								 // ���BOOT�汾
		ARM_PROTOCOL_TxAddData(versionBoot[1]);
		ARM_PROTOCOL_TxAddData(versionBoot[2]);
		ARM_PROTOCOL_TxAddData(versionApp[0]); // ���APP�汾
		ARM_PROTOCOL_TxAddData(versionApp[1]);
		ARM_PROTOCOL_TxAddData(versionApp[2]);
		ARM_PROTOCOL_TxAddFrame(); // �������ݳ��Ⱥ����У��
		break;

	// ��¼������������ʼ
	case UART_ARM_CMD_TOOL_UP_BEGIN: // 0x11,
		// ����Ƭ��flash�����Ҫ1s��
		SPI_FLASH_EraseRoom(SPI_FLASH_TOOL_APP_ADDEESS, 256 * 1024); // 256k
		// ������Ϣ����׿��
		allPacketSize = 0;
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_UP_BEGIN, TRUE);
		break;

	// ��¼����������д��
	case UART_ARM_CMD_TOOL_WRITE_DATA: // 0x12,
		// ��Ƭ��flashд�밲׿�����͵�����
		packetSize = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 4;
		addr1 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		addr2 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];
		addr3 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX];
		addr4 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX];
		writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);

		// ָ��Ҫд������ݻ�������ָ��pBuffer��һ����ʾд���ַ���޷���32λ����WriteAddr���Լ�һ����ʾҪд����ֽ������޷���16λ����NumByteToWrite��
		writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_TOOL_APP_ADDEESS + writeAddr, packetSize);

		// ������Ϣ����׿��
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_WRITE_DATA, writeRes);
		allPacketSize++;
		break;

	// ��¼��������������
	case UART_ARM_CMD_TOOL_UP_END: // 0x13,
		// ���������ɹ�Ӧ��
		// ����������־λ��־
		allPacket[0] = allPacketSize >> 24;
		allPacket[1] = allPacketSize >> 16;
		allPacket[2] = allPacketSize >> 8;
		allPacket[3] = allPacketSize;

		SPI_FLASH_EraseSector(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS); // ������־λ�洢��
		SPI_FLASH_EraseSector(SPI_FLASH_TOOL_APP_DATA_SIZE);	   // ������С�洢��

		SPI_FLASH_WriteByte(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS, 0xAA);
		SPI_FLASH_WriteArray(allPacket, SPI_FLASH_TOOL_APP_DATA_SIZE, 4);

		ARM_PROTOCOL_SendCmdAck(UART_ARM_CMD_TOOL_UP_END);

		/*100ms����ת��BOOT����ȡ��־λ��Ȼ���滻�滻����APP��*/
		TIMER_AddTask(TIMER_ID_TOOL_APP_TO_BOOT,
					  100,
					  IAP_JumpToAppFun,
					  IAP_GD32_FLASH_BASE,
					  1,
					  ACTION_MODE_ADD_TO_QUEUE);
		break;

	// ����DUT��������
	case UART_ARM_CMD_TOOL_DUT_UP: // 0x14,
		upItem = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		ctrlState = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];

		// ����Ҫ��������
		setUpItem(upItem, ctrlState);

		// ���ý����ϱ���Ϣ
		setDutProgressReportInfo();

		// ����������Ϣ�����벻ͬ��״̬��
		enterState();

		// Ӧ��
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_DUT_UP, TRUE);
		break;

	// DUT���������ϱ�
	case UART_ARM_CMD_TOOL_DUT_PROCESS: // 0x15,
		break;

	// �������������
	case UART_ARM_CMD_TOOL_CLEAR_BUFF: // 0x16,

		clearItem = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		if (0x01 == (clearItem & 0x01))
		{
			/*����Ƭ���dut_boot��*/
			dut_info.bootSize = 0;
			SPI_FLASH_EraseRoom(SPI_FLASH_DUT_BOOT_ADDEESS, 128 * 1024); // 128k
		}
		if (0x02 == (clearItem & 0x02))
		{
			/*����Ƭ���dut_app��*/
			dut_info.appSize = 0;
			SPI_FLASH_EraseRoom(SPI_FLASH_DUT_APP_ADDEESS, 1024 * 1024); // 1M
		}
		if (0x04 == (clearItem & 0x04))
		{
			/*����Ƭ���dut_ui��*/
			dut_info.uiSize = 0;
			SPI_FLASH_EraseRoom(SPI_FLASH_DUT_UI_ADDEESS, 14 * 1024 * 1024); // 14M
		}
		if (0x08 == (clearItem & 0x08))
		{
			/*����Ƭ���dut_config��*/
			SPI_FLASH_EraseSector(SPI_FLASH_DUT_CONFIG_ADDEESS); // 4k
		}
		SPI_FLASH_EraseSector(SPI_FLASH_DUT_INFO); // ����dut������Ϣ
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_CLEAR_BUFF, TRUE);

		break;

	// ��¼��д��DUT������Ϣ
	case UART_ARM_CMD_TOOL_SET_DUT_INFO: // 0x17,
		dut_info_len = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 6;

		for (dutName_i = 0; dutName_i < dut_info_len; dutName_i++)
		{
			dutAll[dutName_i] = pCmdFrame->buff[dutName_i + 6 + 3];
		}
		dutAll[20] = dut_info_len;									// ����
		dutAll[21] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX]; // ��ѹ
		dutAll[22] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX]; // ͨѶ����
		dutAll[23] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX]; // bootType
		dutAll[24] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX]; // appType
		dutAll[25] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX]; // uiType
		dutAll[26] = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA6_INDEX]; // configType

		/* ��������Ϣд�뵽Ƭ��flash */
		SPI_FLASH_EraseSector(SPI_FLASH_DUT_INFO);
		writeRes = SPI_FLASH_WriteWithCheck(dutAll, SPI_FLASH_DUT_INFO, 30);

		if (writeRes)
		{
			DutInfoUpdata(); // ����dutinfo����
			if (dut_info.voltage)// ���µ�ѹ����
			{
				DutVol_24();
			}
			else
			{
				DutVol_12();
			}
		}
		/* ������Ϣ����׿�� */
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_TOOL_SET_DUT_INFO, writeRes);
		break;

	// ��ȡ��¼����¼��DUT������Ϣ
	case UART_ARM_CMD_TOOL_GET_DUT_INFO: // 0x18
		SPI_FLASH_ReadArray(dutAll, SPI_FLASH_DUT_INFO, 30);
		if (0xFF == dutAll[20]) // ����Ϊ��
		{
			dutAll[20] = 10;
		}
		ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);			 // �������ͷ
		ARM_PROTOCOL_TxAddData(UART_ARM_CMD_TOOL_GET_DUT_INFO);	 // ���������
		ARM_PROTOCOL_TxAddData(0x00);							 // ���ݳ�����ʱΪ0
		ARM_PROTOCOL_TxAddData(dutAll[21]);						 // ��ӵ�ѹ
		ARM_PROTOCOL_TxAddData(dutAll[22]);						 // ���ͨѶ����
		ARM_PROTOCOL_TxAddData(dutAll[23]);						 // bootType
		ARM_PROTOCOL_TxAddData(dutAll[24]);						 // appType
		ARM_PROTOCOL_TxAddData(dutAll[25]);						 // uiType
		ARM_PROTOCOL_TxAddData(dutAll[26]);						 // configType
		for (dutName_i = 0; dutName_i < dutAll[20]; dutName_i++) // ��ӻ�����Ϣ
		{
			ARM_PROTOCOL_TxAddData(dutAll[dutName_i]);
		}
		ARM_PROTOCOL_TxAddFrame(); // �������ݳ��Ⱥ����У��
		break;

	// 0x19:���ù������к�
	case UART_ARM_CMD_SET_TOOL_RANK:
		SPI_FLASH_EraseSector(SPI_FLASH_TOOL_RANK_ADDEESS); // ������Ŵ洢��
		dut_info.toolRank = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		SPI_FLASH_WriteByte(SPI_FLASH_TOOL_RANK_ADDEESS, dut_info.toolRank); // д�����
		ARM_PROTOCOL_SendCmdParamAck(UART_ARM_CMD_SET_TOOL_RANK, 0x01);		 // �������ݳ��Ⱥ����У��
		break;

	// 0x20:��ȡ�������к�
	case UART_ARM_CMD_GET_TOOL_RANK:
		dut_info.toolRank = SPI_FLASH_ReadByte(SPI_FLASH_TOOL_RANK_ADDEESS);
		ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);		// �������ͷ
		ARM_PROTOCOL_TxAddData(UART_ARM_CMD_GET_TOOL_RANK); // ���������
		ARM_PROTOCOL_TxAddData(0x00);						// �����ʱ����
		ARM_PROTOCOL_TxAddData(dut_info.toolRank);
		ARM_PROTOCOL_TxAddFrame();
		break;

	// д��DUT������������=0x30
	case UART_DUT_CMD_UP_CONFIG:
		// ��ȡ������Ϣ��flash����Ȼ��д��DUT
		dataLen = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX];
		writeRes = SPI_FLASH_WriteWithCheck(&(pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX]), SPI_FLASH_DUT_CONFIG_ADDEESS, dataLen);
		// ������Ϣ����׿��
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_CONFIG, writeRes);
		break;

	case UART_DUT_CMD_UP_UI: // UI����д�루������=0x31��
		// ��ȡUI��flash����Ȼ��д��DUT
		packetSize = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 4;
		addr1 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		addr2 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];
		addr3 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX];
		addr4 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX];
		writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);
		if (currPacketNum != writeAddr)
		{
			dut_info.uiSize++;
			currPacketNum = writeAddr;
		}
		writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_DUT_UI_ADDEESS + writeAddr, packetSize);
		// ������Ϣ����׿��
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_UI, writeRes);
		break;

	// BOOT����д�루������=0x32��
	case UART_DUT_CMD_UP_BOOT:
		// ��ȡBOOT��flash����Ȼ��д��DUT
		packetSize = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 4;
		addr1 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		addr2 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];
		addr3 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX];
		addr4 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX];
		writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);
		if (currPacketNum != writeAddr)
		{
			dut_info.bootSize++;
			currPacketNum = writeAddr;
		}
		writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_DUT_BOOT_ADDEESS + writeAddr, packetSize);
		// ������Ϣ����׿��
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_BOOT, writeRes);
		break;

	// APP����д�루������=0x33��
	case UART_DUT_CMD_UP_APP:
		packetSize = pCmdFrame->buff[ARM_PROTOCOL_CMD_LENGTH_INDEX] - 4;
		addr1 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		addr2 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA2_INDEX];
		addr3 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA3_INDEX];
		addr4 = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA4_INDEX];
		writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);
		if (currPacketNum != writeAddr)
		{
			dut_info.appSize++;
			currPacketNum = writeAddr;
		}
		writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_DUT_APP_ADDEESS + writeAddr, packetSize);
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_APP, writeRes);
		break;

	// �����������������=0x34��
	case UART_DUT_CMD_UP_END:
		endItem = pCmdFrame->buff[ARM_PROTOCOL_CMD_DATA1_INDEX];
		switch (endItem)
		{
		case 0:
			SPI_FLASH_EraseSector(SPI_FLASH_DUT_BOOT_SIZE_ADDEESS);
			size[0] = dut_info.bootSize >> 24;
			size[1] = dut_info.bootSize >> 16;
			size[2] = dut_info.bootSize >> 8;
			size[3] = dut_info.bootSize;
			SPI_FLASH_WriteArray(size, SPI_FLASH_DUT_BOOT_SIZE_ADDEESS, 4);
			break;
		case 1:
			SPI_FLASH_EraseSector(SPI_FLASH_DUT_APP_SIZE_ADDEESS);
			size[0] = dut_info.appSize >> 24;
			size[1] = dut_info.appSize >> 16;
			size[2] = dut_info.appSize >> 8;
			size[3] = dut_info.appSize;
			SPI_FLASH_WriteArray(size, SPI_FLASH_DUT_APP_SIZE_ADDEESS, 4);
			break;
		case 2:
			SPI_FLASH_EraseSector(SPI_FLASH_DUT_UI_SIZE_ADDEESS);
			size[0] = dut_info.uiSize >> 24;
			size[1] = dut_info.uiSize >> 16;
			size[2] = dut_info.uiSize >> 8;
			size[3] = dut_info.uiSize;
			SPI_FLASH_WriteArray(size, SPI_FLASH_DUT_UI_SIZE_ADDEESS, 4);
			break;
		}
		currPacketNum = 0xFFFF;
		// ������Ϣ����׿��
		ARM_PROTOCOL_SendCmdParamAck(UART_DUT_CMD_UP_END, endItem);
		break;

	default:
		break;
	}

	// ɾ������֡
	pCB->rx.head++;
	pCB->rx.head %= ARM_PROTOCOL_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void BLE_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	ARM_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	pCmdFrame = &armProtocolCB.rx.cmdQueue[armProtocolCB.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0; // 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	armProtocolCB.rxFIFO.head++;
	armProtocolCB.rxFIFO.head %= ARM_PROTOCOL_RX_FIFO_SIZE;
	armProtocolCB.rxFIFO.currentProcessIndex = armProtocolCB.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void BLE_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

#if BLE_PROTOCOL_TXRX_TIME_OUT_CHECK_ENABLE
// TXRXͨѶ��ʱ����-˫��
void BLE_PROTOCOL_CALLBACK_TxRxTimeOut(uint32 param)
{
}

// ֹͣTxRXͨѶ��ʱ�������
void BLE_PROTOCOL_StopTxRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_TXRX_TIME_OUT_CONTROL);
}
#endif

// ��������ظ�
void ARM_PROTOCOL_SendCmdAck(uint8 ackCmd)
{
	ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);
	ARM_PROTOCOL_TxAddData(ackCmd);
	ARM_PROTOCOL_TxAddData(0x00);
	ARM_PROTOCOL_TxAddFrame();
}

// ��������ظ�����һ������
void ARM_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{

	ARM_PROTOCOL_TxAddData(ARM_PROTOCOL_CMD_HEAD);
	ARM_PROTOCOL_TxAddData(ackCmd);
	ARM_PROTOCOL_TxAddData(0x01);

	ARM_PROTOCOL_TxAddData(ackParam);
	ARM_PROTOCOL_TxAddFrame();
}

// ���ͻ�ȡMAC��ַ
void ARM_PROTOCOL_SendGetMacAddr(uint32 param)
{
	uint8 bleMacStr[] = "TTM:MAC-?";

	// ʹ��BLE����
	ARM_BRTS_TX_REQUEST();

	// ��������������ӵ�����㻺����
	ARM_UART_AddTxArray(0xFFFF, bleMacStr, sizeof(bleMacStr));
}
