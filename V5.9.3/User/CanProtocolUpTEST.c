#include "common.h"
#include "timer.h"
#include "Param.h"
#include "state.h"
#include "spiflash.h"
#include "CanProtocolUpTest.h"
#include "spiflash.h"
#include "iap.h"
#include "DutInfo.h"

/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void CAN_TEST_PROTOCOL_DataStructInit(CAN_TEST_PROTOCOL_CB* pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
// void CAN_TEST_PROTOCOL_MacProcess(uint32 standarID, uint8* pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_TEST_PROTOCOL_RxFIFOProcess(CAN_TEST_PROTOCOL_CB* pCB);

// UART����֡����������
void CAN_TEST_PROTOCOL_CmdFrameProcess(CAN_TEST_PROTOCOL_CB* pCB);

// �Դ��������֡����У�飬����У����
BOOL CAN_TEST_PROTOCOL_CheckSUM(CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_TEST_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_TEST_PROTOCOL_CB* pCB);

// ͨѶ��ʱ����-����
void CAN_TEST_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void CAN_TEST_PROTOCOL_StopRxTimeOutCheck(void);

// Э��㷢�ʹ������
void CAN_TEST_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void CAN_TEST_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length));

// ����ͨѶ��ʱ�ж�����
void CAN_TEST_PROTOCOL_StartTimeoutCheckTask(void);

// UART���߳�ʱ������
void CAN_TEST_PROTOCOL_CALLBACK_UartBusError(uint32 param);

// ��������ظ���������CRC
void CAN_TEST_PROTOCOL_SendCmdWithCrc(uint8 ackCmd, uint8 ackParam, uint32 Crc);

uint32 SPI_FlashReadForCrc32(uint32 startAddr, uint32 datalen);

// ȫ�ֱ�������
CAN_TEST_PROTOCOL_CB canTestProtocolCB;

// ��������������������������������������������������������������������������������������
// ����ͨѶ��ʱ�ж�����
void CAN_TEST_PROTOCOL_StartTimeoutCheckTask(void)
{

}

// Э���ʼ��
void CAN_TEST_PROTOCOL_Init(void)
{
	// Э������ݽṹ��ʼ��
	CAN_TEST_PROTOCOL_DataStructInit(&canTestProtocolCB);

	// ��������ע�����ݽ��սӿ�
//	CAN_DRIVE_RegisterDataSendService(CAN_TEST_PROTOCOL_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	CAN_TEST_PROTOCOL_RegisterDataSendService(CAN_DRIVE_AddTxArray);
}

// UARTЭ�����̴���
void CAN_TEST_PROTOCOL_Process(void)
{
	// UART����FIFO����������
	CAN_TEST_PROTOCOL_RxFIFOProcess(&canTestProtocolCB);

	// UART���������������
	CAN_TEST_PROTOCOL_CmdFrameProcess(&canTestProtocolCB);
	
	// UARTЭ��㷢�ʹ������
	CAN_TEST_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void CAN_TEST_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = canTestProtocolCB.tx.head;
	uint16 end =  canTestProtocolCB.tx.end;
	CAN_TEST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &canTestProtocolCB.tx.cmdQueue[canTestProtocolCB.tx.end];

	// ���ͻ������������������
	if((end + 1) % CAN_TEST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ��β����֡�������˳�
	if(pCmdFrame->length >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void CAN_TEST_PROTOCOL_TxAddFrame(void)
{
	uint16 checkSum = 0;
	uint16 i = 0;
	uint16 head = canTestProtocolCB.tx.head;
	uint16 end  = canTestProtocolCB.tx.end;
	CAN_TEST_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &canTestProtocolCB.tx.cmdQueue[canTestProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;
    
    pCmdFrame->deviceID = 1;

	// ���ͻ������������������
	if((end + 1) % CAN_TEST_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ����֡���Ȳ��㣬������������ݣ��˳�
	if(CAN_TEST_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length)	// ��ȥ"У����"1���ֽ�
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// ��β����֡�������˳�
	if((length >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	|| (length + 2 >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= CAN_TEST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
	{
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_LENGTH_INDEX] = length - 3;	// �������ݳ��ȣ���ȥ"����ͷ�������֡����ݳ���"4���ֽ�
	
    for(i = 0; i < length; i++)
	{
		checkSum ^= pCmdFrame->buff[i];
	}
    
	// У����
	pCmdFrame->buff[pCmdFrame->length++] = checkSum;

	canTestProtocolCB.tx.end ++;
	canTestProtocolCB.tx.end %= CAN_TEST_PROTOCOL_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void CAN_TEST_PROTOCOL_DataStructInit(CAN_TEST_PROTOCOL_CB* pCB)
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
	for(i = 0; i < CAN_TEST_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<CAN_TEST_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_TEST_PROTOCOL_MacProcess(uint32 standarID, uint8* pData, uint16 length)
{
	uint16 end = canTestProtocolCB.rxFIFO.end;
	uint16 head = canTestProtocolCB.rxFIFO.head;
	uint16 i = 0;
	uint8 *rxdata = pData;
	
	//���������жӣ���ӡ���
	// һ���������������������
	if ((end + 1) % CAN_TEST_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	
	for (i = 0; i < length; i++)
	{
		// �����ֽڶ�ȡ��������FIFO��
		canTestProtocolCB.rxFIFO.buff[canTestProtocolCB.rxFIFO.end] = *rxdata++;
		
		canTestProtocolCB.rxFIFO.end++;
		
		// һ���������������������
		if ((canTestProtocolCB.rxFIFO.end + 1) % CAN_TEST_PROTOCOL_RX_FIFO_SIZE == head)
		{
			break;
		}
		
		canTestProtocolCB.rxFIFO.end %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
	}
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void CAN_TEST_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length))
{		
	canTestProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_TEST_PROTOCOL_ConfirmTempCmdFrameBuff(CAN_TEST_PROTOCOL_CB* pCB)
{
	CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
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
	pCB->rx.end %= CAN_TEST_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
	
	return TRUE;
}

// Э��㷢�ʹ������
void CAN_TEST_PROTOCOL_TxStateProcess(void)
{
	uint16 head = canTestProtocolCB.tx.head;
	uint16 end =  canTestProtocolCB.tx.end;
	uint16 length = canTestProtocolCB.tx.cmdQueue[head].length;
	uint8* pCmd = canTestProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = canTestProtocolCB.tx.cmdQueue[head].deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == canTestProtocolCB.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������		
	if (!(*canTestProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	canTestProtocolCB.tx.cmdQueue[head].length = 0;
	canTestProtocolCB.tx.head ++;
	canTestProtocolCB.tx.head %= CAN_TEST_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_TEST_PROTOCOL_RxFIFOProcess(CAN_TEST_PROTOCOL_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
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
		if(CAN_TEST_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if((pCB->rx.end + 1)%CAN_TEST_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// ���UARTͨѶ��ʱʱ������-2016.1.5����
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_CAN_COMMUNICATION_TIME_OUT,
						CAN_TEST_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
						CAN_TEST_PROTOCOL_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if(pCmdFrame->length >= CAN_TEST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			CAN_TEST_PROTOCOL_StopRxTimeOutCheck();
#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0;	// 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
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
			pCB->rxFIFO.currentProcessIndex %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������
			
			// �����ж�����֡��С���ȣ�һ�����������������ٰ���8���ֽ�: ����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����˲���8���ֽڵıض�������
			if(pCmdFrame->length < CAN_TEST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
			if(pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_LENGTH_INDEX] > CAN_TEST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX-CAN_TEST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				CAN_TEST_PROTOCOL_StopRxTimeOutCheck();
#endif
			
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����Ϊ����֡ʵ�ʳ���
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_LENGTH_INDEX] + CAN_TEST_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if(!CAN_TEST_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				CAN_TEST_PROTOCOL_StopRxTimeOutCheck();
#endif
				
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			CAN_TEST_PROTOCOL_StopRxTimeOutCheck();
#endif
			
			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			CAN_TEST_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// �Դ��������֡����У�飬����У����
BOOL CAN_TEST_PROTOCOL_CheckSUM(CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame)
{
	uint16 checkSum = 0;
	uint16 sumTemp;
	uint16 i = 0;
    
    uint8 length = pCmdFrame->length;
	
	if(NULL == pCmdFrame)
	{
		return FALSE;
	}

	// ����ʼ����ʼ����У����֮ǰ��һ���ֽڣ����ν����������
	for(i = 0; i < pCmdFrame->length - 1; i++)
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
void CAN_TEST_PROTOCOL_CmdFrameProcess(CAN_TEST_PROTOCOL_CB* pCB)
{
	CAN_TEST_PROTOCOL_CMD cmd = CAN_TEST_PROTOCOL_CMD_NULL;
	CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
    
    uint8 eraResual;
    uint32 temp = 0;
    uint16 length = 0;	
    
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
	if(CAN_TEST_PROTOCOL_CMD_HEAD != pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_HEAD_INDEX])
	{
		// ɾ������֡
		pCB->rx.head ++;
		pCB->rx.head %= CAN_TEST_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (CAN_TEST_PROTOCOL_CMD)pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_INDEX];

	// ִ������֡
	switch(cmd)
	{
		// ���������ִ��
		case CAN_TEST_PROTOCOL_CMD_NULL:
			break;

        case CAN_TEST_PROTOCOL_CMD_BOOT_ERASE_FLASH: 			        // DUT_BOOT����
            eraResual = pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_DATA1_INDEX];
            
            // ����ʧ��
            if (0 == eraResual)
            {
                // ����ʧ���ٲ���һ��
                STATE_SwitchStep(STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_EAR);
                break;
            }

            // ����BOOT����
            dut_info.currentBootSize = 0;
            TIMER_KillTask(TIMER_ID_UPGRADE_DUT_BOOT);
            STATE_SwitchStep(STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_PACKET);                
            break;
            
        case CAN_TEST_PROTOCOL_CMD_BOOT_WRITE_FLASH: 				    // DUT_BOOTд��
            eraResual = pCmdFrame->buff[CAN_TEST_PROTOCOL_CMD_DATA1_INDEX];

            if (0 == eraResual) // ����Ǳ�д��ʧ�ܾ��˳�
            {
                dut_info.currentBootSize--;
            }
            if (dut_info.currentBootSize < dut_info.bootSize)
            {
                STATE_SwitchStep(STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_PACKET);
            }
            else
            {
                STATE_SwitchStep(STEP_CM_CAN_BC_UPGRADE_BOOT_UP_SUCCESS);
            }                
            break;
            
        case CAN_TEST_PROTOCOL_CMD_BOOT_UPDATA_FINISH: 		            	// DUT_BOOTд�����
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ITEM_FINISH);
            break;    
                    
        default:
            break;
	}

	// ����UART����ͨѶ��ʱ�ж�
	CAN_TEST_PROTOCOL_StartTimeoutCheckTask();
	
	// ɾ������֡
	pCB->rx.head ++;
	pCB->rx.head %= CAN_TEST_PROTOCOL_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if CAN_TEST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void CAN_TEST_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	CAN_TEST_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &canTestProtocolCB.rx.cmdQueue[canTestProtocolCB.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0;	// 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	canTestProtocolCB.rxFIFO.head ++;
	canTestProtocolCB.rxFIFO.head %= CAN_TEST_PROTOCOL_RX_FIFO_SIZE;
	canTestProtocolCB.rxFIFO.currentProcessIndex = canTestProtocolCB.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void CAN_TEST_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_CAN_COMMUNICATION_TIME_OUT);
}
#endif


// 	UART���߳�ʱ������
void CAN_TEST_PROTOCOL_CALLBACK_UartBusError(uint32 param)
{

}

// ������������
void CAN_TEST_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
	// �������ͷ
	CAN_TEST_PROTOCOL_TxAddData(CAN_TEST_PROTOCOL_CMD_HEAD);

	// ���������
	CAN_TEST_PROTOCOL_TxAddData(cmdWord);

	// ������ݳ���
	CAN_TEST_PROTOCOL_TxAddData(1);
	
	CAN_TEST_PROTOCOL_TxAddData(result);
	
	CAN_TEST_PROTOCOL_TxAddFrame();
}

// ���������޽��
void CAN_TEST_PROTOCOL_SendCmdNoResult(uint8 cmdWord)
{
	// �������ͷ
	CAN_TEST_PROTOCOL_TxAddData(CAN_TEST_PROTOCOL_CMD_HEAD);

	// ���������
	CAN_TEST_PROTOCOL_TxAddData(cmdWord);
	
	CAN_TEST_PROTOCOL_TxAddData(0);		// ���ݳ���
	
	CAN_TEST_PROTOCOL_TxAddFrame();
}

// ��������ظ���������CRC
void CAN_TEST_PROTOCOL_SendCmdWithCrc(uint8 ackCmd, uint8 ackParam, uint32 Crc)
{
    CAN_TEST_PROTOCOL_TxAddData(CAN_TEST_PROTOCOL_CMD_HEAD);
    CAN_TEST_PROTOCOL_TxAddData(ackCmd);
    CAN_TEST_PROTOCOL_TxAddData(5);
    CAN_TEST_PROTOCOL_TxAddData(ackParam);
    CAN_TEST_PROTOCOL_TxAddData((Crc >> 24) & 0xFF);
    CAN_TEST_PROTOCOL_TxAddData((Crc >> 16) & 0xFF);
    CAN_TEST_PROTOCOL_TxAddData((Crc >> 8) & 0xFF);
    CAN_TEST_PROTOCOL_TxAddData(Crc & 0xFF);  
    CAN_TEST_PROTOCOL_TxAddFrame();
}

// ������������
void CAN_TEST_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_TEST_PROTOCOL_CMD cmd, uint32 flashAddr, uint32 addr)
{
	uint8 i = 0;
    
	switch (upDataType)
	{
        case DUT_FILE_TYPE_HEX:							 // hex
            dut_info.reconnectionRepeatContent[0] = 121; // �ܳ���
            dut_info.reconnectionRepeatContent[1] = CAN_TEST_PROTOCOL_CMD_HEAD;
            dut_info.reconnectionRepeatContent[2] = cmd;
            dut_info.reconnectionRepeatContent[3] = 0;											  // ���ݳ���
            dut_info.reconnectionRepeatContent[4] = 0;											  // ����Ƿ����
            dut_info.reconnectionRepeatContent[5] = 0;											  // �����Կ
            SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[6], flashAddr + (addr), 116); // ���߶�ȡ 4+112 ���ֽ�
            for (i = 0; i < 121; i++)															  // �������
            {
                CAN_TEST_PROTOCOL_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
            }
            break;

        case DUT_FILE_TYPE_BIN:							 // bin
            dut_info.reconnectionRepeatContent[0] = 135; // �ܳ���
            dut_info.reconnectionRepeatContent[1] = CAN_TEST_PROTOCOL_CMD_HEAD;
            dut_info.reconnectionRepeatContent[2] = cmd;
            dut_info.reconnectionRepeatContent[3] = 0;											  // ���ݳ���
            dut_info.reconnectionRepeatContent[4] = ((addr) & 0xFF000000) >> 24;				  // ���ݳ���
            dut_info.reconnectionRepeatContent[5] = ((addr) & 0x00FF0000) >> 16;				  // ���ݳ���
            dut_info.reconnectionRepeatContent[6] = ((addr) & 0x0000FF00) >> 8;					  // ���ݳ���
            dut_info.reconnectionRepeatContent[7] = ((addr) & 0x000000FF);						  // ���ݳ���
            SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[8], flashAddr + (addr), 128); // ���߶�ȡ128�ֽ�
            for (i = 0; i < 135; i++)															  // �������
            {
                CAN_TEST_PROTOCOL_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
            }
            break;
            
        default:
            break;
	}
	CAN_TEST_PROTOCOL_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
}
