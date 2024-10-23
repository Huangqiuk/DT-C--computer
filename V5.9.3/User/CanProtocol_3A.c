#include "common.h"
#include "timer.h"
#include "CanProtocolUpDT.h"
#include "CanProtocol_3A.h"
#include "state.h"
#include "spiflash.h"
#include "dutCtl.h"
#include "DutInfo.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void CAN_PROTOCOL1_DataStructInit(CAN_PROTOCOL1_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_PROTOCOL1_MacProcess(uint32 standarID, uint8 *pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_PROTOCOL1_RxFIFOProcess(CAN_PROTOCOL1_CB *pCB);

// UART����֡����������
void CAN_PROTOCOL1_CmdFrameProcess(CAN_PROTOCOL1_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL CAN_PROTOCOL1_CheckSUM(CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_PROTOCOL1_ConfirmTempCmdFrameBuff(CAN_PROTOCOL1_CB *pCB);

// ͨѶ��ʱ����-����
void CAN_PROTOCOL1_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void CAN_PROTOCOL1_StopRxTimeOutCheck(void);

// Э��㷢�ʹ������
void CAN_PROTOCOL1_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void CAN_PROTOCOL1_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length));

// ����ͨѶ��ʱ�ж�����
void CAN_PROTOCOL1_StartTimeoutCheckTask(void);

// UART���߳�ʱ������
void CAN_PROTOCOL1_CALLBACK_UartBusError(uint32 param);

// �ϱ�д���ò������
void CAN_PROTOCOL1_ReportWriteParamResult(uint32 param);

// �ϱ����ò���
void CAN_PROTOCOL1_ReportConfigureParam(uint32 param);

// ������������
void CAN_PROTOCOL1_SendCmdWithResult(uint8 cmdWord, uint8 result);

// ���������޽��
void CAN_PROTOCOL1_SendCmdNoResult(uint8 cmdWord);

// ������������
void CAN_PROTOCOL1_SendLdoV(uint8 cmdWord, uint32 result);

// ���Ͱ���ֵ
void CAN_PROTOCOL1_SendCmdCheckKeyValue(uint32 param);

// ȫ�ֱ�������
CAN_PROTOCOL1_CB canProtocol1CB;

// ��������������������������������������������������������������������������������������
// ����ͨѶ��ʱ�ж�����
void CAN_PROTOCOL1_StartTimeoutCheckTask(void)
{
}

void can_test(uint32 temp)
{
    CAN_PROTOCOL1_SendCmdWithResult(0x88, FALSE);
}

// Э���ʼ��
void CAN_PROTOCOL1_Init(void)
{
    // Э������ݽṹ��ʼ��
    CAN_PROTOCOL1_DataStructInit(&canProtocol1CB);

    // ��������ע�����ݽ��սӿ�
    // CAN_DRIVE_RegisterDataSendService(CAN_PROTOCOL1_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    CAN_PROTOCOL1_RegisterDataSendService(CAN_DRIVE_AddTxArray);
    //
    //  TIMER_AddTask(TIMER_ID_CAN_TEST,
    //              1500,
    //              can_test,
    //              TRUE,
    //              4,
    //              ACTION_MODE_ADD_TO_QUEUE);
}

// UARTЭ�����̴���
void CAN_PROTOCOL_Process_3A(void)
{
    // UART����FIFO����������
    CAN_PROTOCOL1_RxFIFOProcess(&canProtocol1CB);

    // UART���������������
    CAN_PROTOCOL1_CmdFrameProcess(&canProtocol1CB);

    // UARTЭ��㷢�ʹ������
    CAN_PROTOCOL1_TxStateProcess();
}

// ��������֡�������������
void CAN_PROTOCOL1_TxAddData(uint8 data)
{
    uint16 head = canProtocol1CB.tx.head;
    uint16 end = canProtocol1CB.tx.end;
    CAN_PROTOCOL1_TX_CMD_FRAME *pCmdFrame = &canProtocol1CB.tx.cmdQueue[canProtocol1CB.tx.end];

    // ���ͻ������������������
    if ((end + 1) % CAN_PROTOCOL1_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void CAN_PROTOCOL1_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = canProtocol1CB.tx.head;
    uint16 end = canProtocol1CB.tx.end;
    CAN_PROTOCOL1_TX_CMD_FRAME *pCmdFrame = &canProtocol1CB.tx.cmdQueue[canProtocol1CB.tx.end];
    uint16 length = pCmdFrame->length;

    // ���ͻ������������������
    if ((end + 1) % CAN_PROTOCOL1_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ����֡���Ȳ��㣬������������ݣ��˳�
    if (CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN - 4 > length) // ��ȥ"У���L��У���H��������ʶ0xD��������ʶOxA"4���ֽ�
    {
        pCmdFrame->length = 0;

        return;
    }

    // ��β����֡�������˳�
    if ((length >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX) || (length + 2 >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX))
    {
        return;
    }

    // �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
    pCmdFrame->buff[CAN_PROTOCOL1_CMD_LENGTH_INDEX] = length - 4; // �������ݳ��ȣ���ȥ"����ͷ���豸��ַ�������֡����ݳ���"4���ֽ�
    for (i = 1; i < length; i++)
    {
        checkSum += pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);        // ���ֽ���ǰ
    pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF); // ���ֽ��ں�

    // ������ʶ
    pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
    pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

    canProtocol1CB.tx.end++;
    canProtocol1CB.tx.end %= CAN_PROTOCOL1_TX_QUEUE_SIZE;
    // pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void CAN_PROTOCOL1_DataStructInit(CAN_PROTOCOL1_CB *pCB)
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
    for (i = 0; i < CAN_PROTOCOL1_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < CAN_PROTOCOL1_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_PROTOCOL1_MacProcess(uint32 standarID, uint8 *pData, uint16 length)
{
    uint16 end = canProtocol1CB.rxFIFO.end;
    uint16 head = canProtocol1CB.rxFIFO.head;
    uint16 i = 0;
    uint8 *rxdata = pData;

    // ���������жӣ���ӡ���
    //  һ���������������������
    if ((end + 1) % CAN_PROTOCOL1_RX_FIFO_SIZE == head)
    {
        return;
    }

    for (i = 0; i < length; i++)
    {
        // �����ֽڶ�ȡ��������FIFO��
        canProtocol1CB.rxFIFO.buff[canProtocol1CB.rxFIFO.end] = *rxdata++;

        canProtocol1CB.rxFIFO.end++;

        // һ���������������������
        if ((canProtocol1CB.rxFIFO.end + 1) % CAN_PROTOCOL1_RX_FIFO_SIZE == head)
        {
            break;
        }

        canProtocol1CB.rxFIFO.end %= CAN_PROTOCOL1_RX_FIFO_SIZE;
    }
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void CAN_PROTOCOL1_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length))
{
    canProtocol1CB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_PROTOCOL1_ConfirmTempCmdFrameBuff(CAN_PROTOCOL1_CB *pCB)
{
    CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= CAN_PROTOCOL1_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void CAN_PROTOCOL1_TxStateProcess(void)
{
    uint16 head = canProtocol1CB.tx.head;
    uint16 end = canProtocol1CB.tx.end;
    uint16 length = canProtocol1CB.tx.cmdQueue[head].length;
    uint8 *pCmd = canProtocol1CB.tx.cmdQueue[head].buff;
    uint16 localDeviceID = canProtocol1CB.tx.cmdQueue[head].deviceID;

    // ���ͻ�����Ϊ�գ�˵��������
    if (head == end)
    {
        return;
    }

    // ���ͺ���û��ע��ֱ�ӷ���
    if (NULL == canProtocol1CB.sendDataThrowService)
    {
        return;
    }

    // Э�����������Ҫ���͵�������
    if (!(*canProtocol1CB.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // ���ͻ��ζ��и���λ��
    canProtocol1CB.tx.cmdQueue[head].length = 0;
    canProtocol1CB.tx.head++;
    canProtocol1CB.tx.head %= CAN_PROTOCOL1_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_PROTOCOL1_RxFIFOProcess(CAN_PROTOCOL1_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (CAN_PROTOCOL1_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % CAN_PROTOCOL1_RX_QUEUE_SIZE == pCB->rx.head)
        {
            return;
        }

        // ���UARTͨѶ��ʱʱ������-2016.1.5����
#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
        TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
                      CAN_PROTOCOL1_BUS_UNIDIRECTIONAL_TIME_OUT,
                      CAN_PROTOCOL1_CALLBACK_RxTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
#endif

        // ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
        pCmdFrame->buff[pCmdFrame->length++] = currentData;
        pCB->rxFIFO.currentProcessIndex++;
        pCB->rxFIFO.currentProcessIndex %= CAN_PROTOCOL1_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= CAN_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX)
        {
#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            CAN_PROTOCOL1_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0; // 2016.1.5����
            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= CAN_PROTOCOL1_RX_FIFO_SIZE;

            // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������

            // �����ж�����֡��С���ȣ�һ�����������������ٰ���8���ֽ�: ����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����˲���8���ֽڵıض�������
            if (pCmdFrame->length < CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            if (pCmdFrame->buff[CAN_PROTOCOL1_CMD_LENGTH_INDEX] > CAN_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX - CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN)
            {
#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                CAN_PROTOCOL1_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����Ϊ����֡ʵ�ʳ���
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[CAN_PROTOCOL1_CMD_LENGTH_INDEX] + CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN)
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!CAN_PROTOCOL1_CheckSUM(pCmdFrame))
            {
#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                CAN_PROTOCOL1_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            CAN_PROTOCOL1_StopRxTimeOutCheck();
#endif

            // ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            CAN_PROTOCOL1_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL CAN_PROTOCOL1_CheckSUM(CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame)
{
    uint16 checkSum = 0;
    uint16 sumTemp;
    uint16 i = 0;

    if (NULL == pCmdFrame)
    {
        return FALSE;
    }

    // ���豸��ַ��ʼ����У����֮ǰ��һ���ֽڣ����ν����ۼ�����
    for (i = 1; i < pCmdFrame->length - 4; i++)
    {
        checkSum += pCmdFrame->buff[i];
    }

    // �ۼӺͣ����ֽ���ǰ�����ֽ��ں�
    sumTemp = pCmdFrame->buff[pCmdFrame->length - 3]; // ���ֽ�
    sumTemp <<= 8;
    sumTemp += pCmdFrame->buff[pCmdFrame->length - 4]; // ���ֽ�

    // �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
    if (sumTemp != checkSum)
    {
        return FALSE;
    }

    return TRUE;
}


// UART����֡����������
void CAN_PROTOCOL1_CmdFrameProcess(CAN_PROTOCOL1_CB *pCB)
{
    CAN_PROTOCOL1_CMD cmd = CAN_PROTOCOL1_CMD_NULL;
    CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame = NULL;

    BOOL checkMark;
    uint8_t i = 0;
    uint8 buffFlag[4] = {0xFF};

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
    if (CAN_PROTOCOL1_CMD_HEAD != pCmdFrame->buff[CAN_PROTOCOL1_CMD_HEAD_INDEX])
    {
        // ɾ������֡
        pCB->rx.head++;
        pCB->rx.head %= CAN_PROTOCOL1_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (CAN_PROTOCOL1_CMD)pCmdFrame->buff[CAN_PROTOCOL1_CMD_CMD_INDEX];
    
    if(CAN_PROTOCOL1_IOT_DEVICE_ADDR == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DEVICE_ADDR_INDEX])
    {
        // ִ������֡
        switch (cmd)
        {
            // ���������ִ��
            case CAN_PROTOCOL1_CMD_NULL: 
                break;

            // �����������0xF1��
            case CAN_PROTOCOL1_CMD_UPDATE_REQUEST: 
                TIMER_KillTask(TIMER_ID_SEND_UP_APP_REQUEST);
            
                //   0���豸�ܾ�������1���豸ͬ�������� 
                if(pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_SEND_APP_EAR);                    
                }
                else
                {
                    // ������ʱ�������������ʧ��
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_COMMUNICATION_TIME_OUT);                 
                }
                break;

            // �����̼����0xF2��
            case CAN_PROTOCOL1_CMD_UPDATE_START: 
            
                //   ����������Ӧ���������������������洢�ռ䣬�򷵻�0��
                if(0 == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    // ������ʱ�������������ʧ��                
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_COMMUNICATION_TIME_OUT);                    
                }
                else
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_SEND_FIRST_APP_PACKET);                 
                }
                break;

            // ���ݰ�д�����0xF3��
            case CAN_PROTOCOL1_CMD_WRITE_DATA: 

                if (1 == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    if (0 < dut_info.currentAppSize)
                    {
                        dut_info.currentAppSize--;
                    }
                    break;
                }
                if (dut_info.currentAppSize < dut_info.appSize)
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_SEND_APP_PACKET);
                }
                else
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_APP_UP_SUCCESS);
                }
                break;

            // ���ݰ�д��������0xF4��0����������ɣ�CRCδͨ��У�飻1����������ɣ�CRCͨ��У��
            case CAN_PROTOCOL1_CMD_WRITE_COMPLETE: 
            
                if(0 == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    // ������ʱ�������������ʧ��                
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_COMMUNICATION_TIME_OUT);                    
                }
                else
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_ITEM_FINISH);                 
                }
                break;
                
            default:
                break;
        }         
    }
    else
    {
        // ִ������֡
        switch (cmd)
        {
            // ���������ִ��
            case CAN_PROTOCOL1_CMD_NULL:
                break;

            // д��汾��Ϣ
            case CAN_PROTOCOL1_CMD_VERSION_TYPE_WRITE:
                TIMER_KillTask(TIMER_ID_SET_DUT_UI_VER);
                writeUiVerFlag = FALSE;
                if ((8 ==  pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX]) && (TRUE == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA2_INDEX]))
                {
                    TIMER_KillTask(TIMER_ID_SET_DUT_UI_VER);
                    STATE_SwitchStep(STEP_CM_CAN_READ_UI_VER);
                }
                else
                {
                    dut_info.uiUpFaile = TRUE; // ui����ʧ��
                    STATE_EnterState(STATE_STANDBY);
                }
                break;

            // ��ȡ�汾��Ϣ
            case CAN_PROTOCOL1_CMD_VERSION_TYPE_READ:
                if (8 ==  pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    // У��
                    for (i = 0; i < pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA2_INDEX]; i++)
                    {
                        if (uiVerifiedBuff[i] != pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA3_INDEX + i])
                        {
                            checkMark = FALSE;  // ���ֲ�ƥ�䣬���ñ�־Ϊ��
                            break;              // ����ѭ��
                        }
                        else
                        {
                            checkMark = TRUE;   // ����Ԫ�ض�ƥ�䣬���ñ�־Ϊ��
                        }
                    }

                    if (checkMark)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_WRITE_UI_VER_SUCCESS);
                    }
                    else
                    {
                        dut_info.uiUpFaile = TRUE; // ui����ʧ��
                        STATE_EnterState(STATE_STANDBY);
                    }
                }
                else
                {
                    dut_info.uiUpFaile = TRUE; // ui����ʧ��
                    STATE_EnterState(STATE_STANDBY);
                }
                break;

            // 0xC1,д������ϢӦ��
            case CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM_RESULT:
            
                // ��Ӧ�����������Ϣ�����ɹ�
                STATE_SwitchStep(STEP_CAN_READ_CONFIG);
                break;

            // 0xc3,��ȡ���ò��������ò���У��
            case CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM_REPORT:

                // У��
                for (i = 0; i < pCmdFrame->buff[CAN_PROTOCOL1_CMD_LENGTH_INDEX]; i++)
                {
                    if (verifiedBuff[i] != pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX + i])
                    {
                        checkMark = FALSE;  // ���ֲ�ƥ�䣬���ñ�־Ϊ��
                        break;              // ����ѭ��
                    }
                    else
                    {
                        checkMark = TRUE;   // ����Ԫ�ض�ƥ�䣬���ñ�־Ϊ��
                    }
                }

                if (checkMark)
                {
    //                STATE_SwitchStep(STEP_CAN_SET_CONFIG_SUCCESS);
                    STATE_SwitchStep(STEP_CAN_DCD_FLAG_WRITE);
                }
                else
                {
                    dut_info.configUpFaile = TRUE; // config����ʧ��
                    STATE_EnterState(STATE_STANDBY);
                }
                break;

            // д��DCD��λ��־λ
            case CAN_PROTOCOL1_CMD_DCD_FLAG_WRITE:   
            
                // У���־������
                if (pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    STATE_SwitchStep(STEP_CAN_DCD_FLAG_READ);
                }
                else
                {
                    dut_info.configUpFaile = TRUE; // config����ʧ��
                    STATE_EnterState(STATE_STANDBY);
                }         
              break;

            // ��ȡDCD��λ��־λ
            case CAN_PROTOCOL1_CMD_DCD_FLAG_READ:
            
                 // ��ʼ�� + �豸��ַ + ������ + ���ݳ��� + ��־�����ݳ��� + ������ +����N + У���� + ������
                 // ��ȡDCD��־�����ݣ�վλ��� * ��ÿ����־λ�ĸ����ݣ� + ����һ����־λǰ������(������ǰ���������)�� + �ڼ�����־λ��
                for (i = 0; i < 4; i++)
                {
                    buffFlag[i] = pCmdFrame->buff[(5 * 4) + 5 + i];
                }

                // У���־������
                if ((buffFlag[0] == 0x00) && (buffFlag[1] == 0x00) && (buffFlag[2] == 0x00) && (buffFlag[3] == 0x02))
                {
                    STATE_SwitchStep(STEP_CAN_SET_CONFIG_SUCCESS);
                }
                else
                {
                    dut_info.configUpFaile = TRUE; // config����ʧ��
                    STATE_EnterState(STATE_STANDBY);
                }              
              break;

            default:
                break;
        }    
    }
    

    // ɾ������֡
    pCB->rx.head++;
    pCB->rx.head %= CAN_PROTOCOL1_RX_QUEUE_SIZE;
}

//  UART���߳�ʱ������
void CAN_PROTOCOL1_CALLBACK_UartBusError(uint32 param)
{
}

// �ϱ�д���ò������
void CAN_PROTOCOL1_ReportWriteParamResult(uint32 param)
{
    // �������ͷ
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);

    // ����豸��ַ
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);

    // ���������
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM_RESULT);

    // ������ݳ���
    CAN_PROTOCOL1_TxAddData(1);

    // д����
    CAN_PROTOCOL1_TxAddData(param);

    // ��Ӽ������������������������
    CAN_PROTOCOL1_TxAddFrame();
}

// ������������
void CAN_PROTOCOL1_SendLdoV(uint8 cmdWord, uint32 result)
{
    // �������ͷ
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);

    if(DUT_TYPE_IOT == dut_info.ID)
    {
        // ����豸��ַ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);    
    }
    else
    {
        // ����豸��ַ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);    
    }

    // ���������
    CAN_PROTOCOL1_TxAddData(cmdWord);

    // ������ݳ���
    CAN_PROTOCOL1_TxAddData(4);

    CAN_PROTOCOL1_TxAddData((result >> 24) & 0xFF);
    CAN_PROTOCOL1_TxAddData((result >> 16) & 0xFF);
    CAN_PROTOCOL1_TxAddData((result >> 8) & 0xFF);
    CAN_PROTOCOL1_TxAddData((result >> 0) & 0xFF);

    CAN_PROTOCOL1_TxAddFrame();
}

// ������������
void CAN_PROTOCOL1_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
    // �������ͷ
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);

    if(DUT_TYPE_IOT == dut_info.ID)
    {
        // ����豸��ַ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);    
    }
    else
    {
        // ����豸��ַ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);    
    }

    // ���������
    CAN_PROTOCOL1_TxAddData(cmdWord);

    // ������ݳ���
    CAN_PROTOCOL1_TxAddData(1);

    CAN_PROTOCOL1_TxAddData(result);

    CAN_PROTOCOL1_TxAddFrame();
}

// ���������޽��
void CAN_PROTOCOL1_SendCmdNoResult(uint8 cmdWord)
{
    // �������ͷ
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);

    if(DUT_TYPE_IOT == dut_info.ID)
    {
        // ����豸��ַ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);    
    }
    else
    {
        // ����豸��ַ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);    
    }
    
    // ���������
    CAN_PROTOCOL1_TxAddData(cmdWord);

    CAN_PROTOCOL1_TxAddData(0); // ���ݳ���

    CAN_PROTOCOL1_TxAddFrame();
}

// ���������̼�����
void CAN_PROTOCOL1_SendUpAppReuqest(uint32 param)
{
    CAN_PROTOCOL1_SendCmdWithResult(CAN_PROTOCOL1_CMD_UPDATE_REQUEST, 9); 
}

// ���������̼�����
void CAN_PROTOCOL1_SendCmdUpApp(void)
{
	// �������ͷ
	CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
    
    // ����豸��ַ
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);    

	// ���������
	CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_UPDATE_START); // F2
	CAN_PROTOCOL1_TxAddData(0); // ���ݳ��� 
	CAN_PROTOCOL1_TxAddData(9); // �豸���� IOT-9
	CAN_PROTOCOL1_TxAddData(dut_info.iotCrc8); // CRC8У����
	CAN_PROTOCOL1_TxAddData(((dut_info.iotAppUpDataLen) & 0xFF000000) >> 24); // �ļ�����
	CAN_PROTOCOL1_TxAddData(((dut_info.iotAppUpDataLen) & 0x00FF0000) >> 16); // �ļ�����
	CAN_PROTOCOL1_TxAddData(((dut_info.iotAppUpDataLen) & 0x0000FF00) >> 8); // �ļ�����
	CAN_PROTOCOL1_TxAddData((dut_info.iotAppUpDataLen) & 0x000000FF); // �ļ�����    
	CAN_PROTOCOL1_TxAddFrame();
}

// д��IOT APP����
void CAN_PROTOCOL1_SendOnePacket_Bin(uint32 flashAddr, uint32 addr)
{
	uint8 addr1, addr2, addr3, addr4;
	uint8 appUpdateOnePacket[150] = {0};
	uint8 appUpdateOnePacket_i = 0;
	addr1 = (addr & 0xFF000000) >> 24;
	addr2 = (addr & 0x00FF0000) >> 16;
	addr3 = (addr & 0x0000FF00) >> 8;
	addr4 = (addr & 0x000000FF);
	SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + addr, 128); // ���߶�ȡ128�ֽ�

	CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                                     // ͷ
	CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);                              // �豸��
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_DATA);                               // ������
	CAN_PROTOCOL1_TxAddData(0x00);													       // ������ʱΪ0
	CAN_PROTOCOL1_TxAddData(addr1);													   // ��ӵ�ַ
	CAN_PROTOCOL1_TxAddData(addr2);													   // ��ӵ�ַ
	CAN_PROTOCOL1_TxAddData(addr3);													   // ��ӵ�ַ
	CAN_PROTOCOL1_TxAddData(addr4);													   // ��ӵ�ַ
	CAN_PROTOCOL1_TxAddData(128);													       // ���ݰ�����
    
	for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 128; appUpdateOnePacket_i++) // �������
	{
		CAN_PROTOCOL1_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i]);
	}
	CAN_PROTOCOL1_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
}

