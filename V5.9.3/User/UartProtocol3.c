#include "common.h"
#include "timer.h"
#include "uartDrive.h"
#include "uartProtocol3.h"
#include "uartProtocol.h"
#include "state.h"
#include "spiflash.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "DtaUartProtocol.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void UART_PROTOCOL3_DataStructInit(UART_PROTOCOL3_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL3_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL3_CheckSUM(UART_PROTOCOL3_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL3_ConfirmTempCmdFrameBuff(UART_PROTOCOL3_CB *pCB);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL3_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// �ϱ�д���ò������
void UART_PROTOCOL3_ReportWriteParamResult(uint32 param); // ���õ�

void UART_PROTOCOL3_22(uint32 param); // ����

// ȫ�ֱ�������
UART_PROTOCOL3_CB uartProtocolCB3;

//UART_PROTOCOL3_PARAM_CB hmiDriveCB3;

// Э���ʼ��
void UART_PROTOCOL3_Init(void)
{
    // Э������ݽṹ��ʼ��
    UART_PROTOCOL3_DataStructInit(&uartProtocolCB3);

    // ��������ע�����ݽ��սӿ�
    UART_DRIVE_RegisterDataSendService(UART_PROTOCOL3_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    UART_PROTOCOL3_RegisterDataSendService(UART_DRIVE_AddTxArray);

    //  TIMER_AddTask(TIMER_ID_PROTOCOL_3A_PARAM_TX,
    //                  200,
    //                  UART_PROTOCOL3_22,
    //                  TRUE,
    //                  TIMER_LOOP_FOREVER,
    //                  ACTION_MODE_ADD_TO_QUEUE);
}

// ��������֡�������������
void UART_PROTOCOL3_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB3.tx.head;
    uint16 end = uartProtocolCB3.tx.end;
    UART_PROTOCOL3_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB3.tx.end];

    // ���ͻ������������������
    if ((end + 1) % UART_PROTOCOL3_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void UART_PROTOCOL3_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = uartProtocolCB3.tx.head;
    uint16 end = uartProtocolCB3.tx.end;
    UART_PROTOCOL3_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB3.tx.end];
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
    pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);        // ���ֽ���ǰ
    pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF); // ���ֽ��ں�

    // ������ʶ
    pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
    pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

    uartProtocolCB3.tx.end++;
    uartProtocolCB3.tx.end %= UART_PROTOCOL3_TX_QUEUE_SIZE;
}

// ���ݽṹ��ʼ��
void UART_PROTOCOL3_DataStructInit(UART_PROTOCOL3_CB *pCB)
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
    for (i = 0; i < UART_PROTOCOL3_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < UART_PROTOCOL3_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL3_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB3.rxFIFO.end;
    uint16 head = uartProtocolCB3.rxFIFO.head;
    uint8 rxdata = 0x00;

    // ��������
    rxdata = *pData;

    // һ���������������������
    if ((end + 1) % UART_PROTOCOL3_RX_FIFO_SIZE == head)
    {
        return;
    }
    // һ��������δ��������
    else
    {
        // �����յ������ݷŵ���ʱ��������
        uartProtocolCB3.rxFIFO.buff[end] = rxdata;
        uartProtocolCB3.rxFIFO.end++;
        uartProtocolCB3.rxFIFO.end %= UART_PROTOCOL3_RX_FIFO_SIZE;
    }

    // ����KM5SЭ�����
    UART_PROTOCOL_MacProcess(standarID, pData, length);
    DTA_UART_PROTOCOL_MacProcess(standarID, pData, length);
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL3_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB3.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL3_ConfirmTempCmdFrameBuff(UART_PROTOCOL3_CB *pCB)
{
    UART_PROTOCOL3_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= UART_PROTOCOL3_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void UART_PROTOCOL3_TxStateProcess(void)
{
    uint16 head = uartProtocolCB3.tx.head;
    uint16 end = uartProtocolCB3.tx.end;
    uint16 length = uartProtocolCB3.tx.cmdQueue[head].length;
    uint8 *pCmd = uartProtocolCB3.tx.cmdQueue[head].buff;
    uint16 localDeviceID = uartProtocolCB3.tx.cmdQueue[head].deviceID;

    // ���ͻ�����Ϊ�գ�˵��������
    if (head == end)
    {
        return;
    }

    // ���ͺ���û��ע��ֱ�ӷ���
    if (NULL == uartProtocolCB3.sendDataThrowService)
    {
        return;
    }

    // Э�����������Ҫ���͵�������
    if (!(*uartProtocolCB3.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // ���ͻ��ζ��и���λ��
    uartProtocolCB3.tx.cmdQueue[head].length = 0;
    uartProtocolCB3.tx.head++;
    uartProtocolCB3.tx.head %= UART_PROTOCOL3_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL3_RxFIFOProcess(UART_PROTOCOL3_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    UART_PROTOCOL3_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (UART_PROTOCOL3_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL3_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % UART_PROTOCOL3_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL3_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= UART_PROTOCOL3_RX_CMD_FRAME_LENGTH_MAX)
        {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0; // 2016.1.5����
            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL3_RX_FIFO_SIZE;
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
            if (pCmdFrame->length < UART_PROTOCOL3_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            if (pCmdFrame->buff[UART_PROTOCOL3_CMD_LENGTH_INDEX] > UART_PROTOCOL3_RX_CMD_FRAME_LENGTH_MAX - UART_PROTOCOL3_CMD_FRAME_LENGTH_MIN)
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL3_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����Ϊ����֡ʵ�ʳ���
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[UART_PROTOCOL3_CMD_LENGTH_INDEX] + UART_PROTOCOL3_CMD_FRAME_LENGTH_MIN)
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!UART_PROTOCOL3_CheckSUM(pCmdFrame))
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL3_RX_FIFO_SIZE;
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
            UART_PROTOCOL3_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL3_CheckSUM(UART_PROTOCOL3_RX_CMD_FRAME *pCmdFrame)
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
void UART_PROTOCOL3_CmdFrameProcess(UART_PROTOCOL3_CB *pCB)
{
    UART_PROTOCOL3_CMD cmd = UART_PROTOCOL3_CMD_NULL;
    UART_PROTOCOL3_RX_CMD_FRAME *pCmdFrame = NULL;

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
    if (UART_PROTOCOL3_CMD_HEAD != pCmdFrame->buff[UART_PROTOCOL3_CMD_HEAD_INDEX])
    {
        // ɾ������֡
        pCB->rx.head++;
        pCB->rx.head %= UART_PROTOCOL3_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (UART_PROTOCOL3_CMD)pCmdFrame->buff[UART_PROTOCOL3_CMD_CMD_INDEX];

    // ��ΪIOT����
    if(UART_PROTOCOL3_IOT_DEVICE_ADDR == pCmdFrame->buff[UART_PROTOCOL3_CMD_DEVICE_ADDR_INDEX])
    {
        // ִ������֡
        switch (cmd)
        {
            // ���������ִ��
            case UART_PROTOCOL3_CMD_NULL: 
                break;

            // �����������0xF1��
            case UART_PROTOCOL3_CMD_UPDATE_REQUEST: 
                TIMER_KillTask(TIMER_ID_SEND_UP_APP_REQUEST);
            
                //   0���豸�ܾ�������1���豸ͬ�������� 
                if(pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX])
                {
                    STATE_SwitchStep(STEP_IOT_UART_UPGRADE_SEND_APP_EAR);                    
                }
                else
                {
                    // ������ʱ�������������ʧ��
                    STATE_SwitchStep(STEP_IOT_UART_UPGRADE_COMMUNICATION_TIME_OUT);                 
                }
                break;

            // �����̼����0xF2��
            case UART_PROTOCOL3_CMD_UPDATE_START: 
            
                //   ����������Ӧ���������������������洢�ռ䣬�򷵻�0��
                if(0 == pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX])
                {
                    // ������ʱ�������������ʧ��                
                    STATE_SwitchStep(STEP_IOT_UART_UPGRADE_COMMUNICATION_TIME_OUT);                    
                }
                else
                {
                    STATE_SwitchStep(STEP_IOT_UART_UPGRADE_SEND_FIRST_APP_PACKET);                 
                }
                break;

            // ���ݰ�д�����0xF3��
            case UART_PROTOCOL3_CMD_WRITE_DATA: 

                if (0 == pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX])
                {
                    if (0 < dut_info.currentAppSize)
                    {
                        dut_info.currentAppSize--;
                    }
                    break;
                }
                if (dut_info.currentAppSize < dut_info.appSize)
                {
                    STATE_SwitchStep(STEP_IOT_UART_UPGRADE_SEND_APP_PACKET);
                }
                else
                {
                    STATE_SwitchStep(STEP_IOT_UART_UPGRADE_APP_UP_SUCCESS);
                }
                break;

            // ���ݰ�д��������0xF4��0����������ɣ�CRCδͨ��У�飻1����������ɣ�CRCͨ��У��
            case UART_PROTOCOL3_CMD_WRITE_COMPLETE: 
            
                if(0 == pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX])
                {
                    // ������ʱ�������������ʧ��                
                    STATE_SwitchStep(STEP_IOT_UART_UPGRADE_COMMUNICATION_TIME_OUT);                    
                }
                else
                {
                    STATE_SwitchStep(STEP_IOT_UART_UPGRADE_ITEM_FINISH);                 
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
            case UART_PROTOCOL3_CMD_CONFIG_SET: // 0x53
                break;

            // Э���л�
            case UART_PROTOCOL3_CMD_PROTOCOL_SWITCCH:
    //            TIMER_KillTask(TIMER_ID_SET_DUT_UI_VER);

                if (!writeUiFlag)
                {
                    TIMER_KillTask(TIMER_ID_PROTOCOL_SWITCCH);

                    // Э���л��ɹ�
                    if (pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX])
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

                        // ��������
                        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                                      500,
                                      STATE_SwitchStep,
                                      STEP_UART_SEND_SET_CONFIG,
                                      TIMER_LOOP_FOREVER,
                                      ACTION_MODE_ADD_TO_QUEUE);
                    }
                }
                break;

            // д��汾��Ϣ
            case UART_PROTOCOL3_CMD_VERSION_TYPE_WRITE:
            TIMER_KillTask(TIMER_ID_SET_DUT_UI_VER);
                if ((8 ==  pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX]) && (TRUE == pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA2_INDEX]))
                {
                    STATE_SwitchStep(STEP_CM_UART_BC_READ_UI_VER);
                }
                else
                {
                    dut_info.uiUpFaile = TRUE; // ui����ʧ��
                    STATE_EnterState(STATE_STANDBY);
                }
                break;

            // ��ȡ�汾��Ϣ
            case UART_PROTOCOL3_CMD_VERSION_TYPE_READ:
            
                if (8 ==  pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX])
                {
                    // У��
                    for (i = 0; i < pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA2_INDEX]; i++)
                    {
                        if (uiVerifiedBuff[i] != pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA3_INDEX + i])
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
                        STATE_SwitchStep(STEP_CM_UART_BC_WRITE_UI_VER_SUCCESS);
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

            // dut_configд��ɹ�
            case UART_PROTOCOL3_CMD_WRITE_CONFIG_RESULT: // 0xc1
                TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);
                STATE_SwitchStep(STEP_UART_READ_CONFIG);
                break;

            // У�����ò���
            case UART_PROTOCOL3_CMD_READ_CONTROL_PARAM_REPORT: // 0xc3

                for (i = 0; i < pCmdFrame->buff[UART_PROTOCOL3_CMD_LENGTH_INDEX]; i++)
                {
                    if (verifiedBuff[i] != pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX + i])
                    {
                        checkMark = FALSE;  // ���ֲ�ƥ�䣬���ñ�־Ϊ��
                        break;              // ����ѭ��
                    }
                    else
                    {
                        checkMark = TRUE;   // ����Ԫ�ض�ƥ�䣬���ñ�־Ϊ��
                    }
                }

                // д�����óɹ���д���־λ
                if (checkMark)
                {
                    STATE_SwitchStep(STEP_UART_DCD_FLAG_WRITE);
                }
                else
                {
                    dut_info.configUpFaile = TRUE; // config����ʧ��
                    STATE_EnterState(STATE_STANDBY);
                }
                break;

            // д��DCD��λ��־λ
            case UART_PROTOCOL3_CMD_DCD_FLAG_WRITE:   
            
                // У���־������
                if (pCmdFrame->buff[UART_PROTOCOL3_CMD_DATA1_INDEX])
                {
                    STATE_SwitchStep(STEP_UART_DCD_FLAG_READ);
                }
                else
                {
                    dut_info.configUpFaile = TRUE; // config����ʧ��
                    STATE_EnterState(STATE_STANDBY);
                }         
              break;

            // ��ȡDCD��λ��־λ
            case UART_PROTOCOL3_CMD_DCD_FLAG_READ:
            
                 // ��ʼ�� + �豸��ַ + ������ + ���ݳ��� + ��־�����ݳ��� + ������ +����N + У���� + ������
                 // ��ȡDCD��־�����ݣ�վλ��� * ��ÿ����־λ�ĸ����ݣ� + ����һ����־λǰ������(������ǰ���������)�� + �ڼ�����־λ��
                for (i = 0; i < 4; i++)
                {
                    buffFlag[i] = pCmdFrame->buff[(5 * 4) + 5 + i];
                }

                // У���־������
                if ((buffFlag[0] == 0x00) && (buffFlag[1] == 0x00) && (buffFlag[2] == 0x00) && (buffFlag[3] == 0x02))
                {
                    STATE_SwitchStep(STEP_UART_SET_CONFIG_SUCCESS);
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
    pCB->rx.head %= UART_PROTOCOL3_RX_QUEUE_SIZE;
}

// ���������̼�����
void UART_PROTOCOL3_SendUpAppReuqest(uint32 param)
{
    UART_PROTOCOL3_SendCmdWithResult(UART_PROTOCOL3_CMD_UPDATE_REQUEST, 9); 
}

// ������������
void UART_PROTOCOL3_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
	// �������ͷ
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);

    if(DUT_TYPE_IOT == dut_info.ID)
    {
        // ����豸��ַ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_IOT_DEVICE_ADDR);    
    }
    else
    {
        // ����豸��ַ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);    
    }

	// ���������
	UART_PROTOCOL3_TxAddData(cmdWord);

	// ������ݳ���
	UART_PROTOCOL3_TxAddData(1);

	UART_PROTOCOL3_TxAddData(result);

	UART_PROTOCOL3_TxAddFrame();
}

// ���������޽��
void UART_PROTOCOL3_SendCmdNoResult(uint8 cmdWord)
{
	// �������ͷ
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
    
    if(DUT_TYPE_IOT == dut_info.ID)
    {
        // ����豸��ַ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_IOT_DEVICE_ADDR);    
    }
    else
    {
        // ����豸��ַ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);    
    }

	// ���������
	UART_PROTOCOL3_TxAddData(cmdWord);

	UART_PROTOCOL3_TxAddData(0); // ���ݳ���

	UART_PROTOCOL3_TxAddFrame();
}

// ���������̼�����
void UART_PROTOCOL3_SendCmdUpApp(void)
{
	// �������ͷ
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
    
    // ����豸��ַ
    UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_IOT_DEVICE_ADDR);    

	// ���������
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_UPDATE_START); // F2
	UART_PROTOCOL3_TxAddData(0); // ���ݳ��� 
	UART_PROTOCOL3_TxAddData(9); // �豸���� IOT-9
	UART_PROTOCOL3_TxAddData(dut_info.iotCrc8); // CRC8У����
	UART_PROTOCOL3_TxAddData(((dut_info.iotAppUpDataLen) & 0xFF000000) >> 24); // �ļ�����
	UART_PROTOCOL3_TxAddData(((dut_info.iotAppUpDataLen) & 0xFF000000) >> 16); // �ļ�����
	UART_PROTOCOL3_TxAddData(((dut_info.iotAppUpDataLen) & 0xFF000000) >> 8); // �ļ�����
	UART_PROTOCOL3_TxAddData((dut_info.iotAppUpDataLen) & 0xFF000000); // �ļ�����    
	UART_PROTOCOL3_TxAddFrame();
}

// д��IOT APP����
void UART_PROTOCOL3_SendOnePacket_Bin(uint32 flashAddr, uint32 addr)
{
	uint8 addr1, addr2, addr3, addr4;
	uint8 appUpdateOnePacket[150] = {0};
	uint8 appUpdateOnePacket_i = 0;
	addr1 = (addr & 0xFF000000) >> 24;
	addr2 = (addr & 0x00FF0000) >> 16;
	addr3 = (addr & 0x0000FF00) >> 8;
	addr4 = (addr & 0x000000FF);
	SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + addr, 128); // ���߶�ȡ128�ֽ�

	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);                                     // ͷ
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_IOT_DEVICE_ADDR);                              // �豸��
    UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_WRITE_DATA);                               // ������
	UART_PROTOCOL3_TxAddData(0x00);													       // ������ʱΪ0
	UART_PROTOCOL3_TxAddData(addr1);													   // ��ӵ�ַ
	UART_PROTOCOL3_TxAddData(addr2);													   // ��ӵ�ַ
	UART_PROTOCOL3_TxAddData(addr3);													   // ��ӵ�ַ
	UART_PROTOCOL3_TxAddData(addr4);													   // ��ӵ�ַ
	UART_PROTOCOL3_TxAddData(128);													       // ���ݰ�����
    
	for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 128; appUpdateOnePacket_i++) // �������
	{
		UART_PROTOCOL3_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i]);
	}
	UART_PROTOCOL3_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
}
