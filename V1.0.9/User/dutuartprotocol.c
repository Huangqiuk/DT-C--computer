#include "common.h"
#include "timer.h"
#include "DutUartDrive.h"
#include "DutUartProtocol.h"
#include "StsProtocol.h"
#include "state.h"
#include "spiflash.h"
#include "AvoPin.h"
#include "AvoProtocol.h"
#include "param.h"
#include "DutInfo.h"
#include "PowerProtocol.h"
#include "stringOperation.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void DUT_PROTOCOL_DataStructInit(DUT_PROTOCOL_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void DUT_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// �Դ��������֡����У�飬����У����
BOOL DUT_PROTOCOL_CheckSUM(DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL DUT_PROTOCOL_ConfirmTempCmdFrameBuff(DUT_PROTOCOL_CB *pCB);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void DUT_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// �ϱ�д���ò������
void DUT_PROTOCOL_ReportWriteParamResult(uint32 param); // ���õ�

// ��������ظ�
void DUT_PROTOCOL_SendCmdAck(uint8 ackCmd);

// ��������ظ�����һ������
void DUT_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// ��������ظ���������������
void DUT_PROTOCOL_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam);

// UARTЭ�����̴���
void DUT_UART_PROTOCOL_Process(void);

// ȫ�ֱ�������
extern char ptUpMsgBuff[][MAX_ONE_LINES_LENGTH];

DUT_PROTOCOL_CB uartProtocolCB3;
THROTTLE throttle;
BRAKE brake;
DERAILLEUR derailleur;
uint8 light_cnt = 0;
uint8 cnt = 0;
uint8 cali_cnt = 0;
uint8 brake_cnt = 0;
uint8 derailleur_cnt = 0;
uint8 left_light_cnt = 0;
uint8 right_light_cnt = 0;
BOOL write_success_Flag;
uint32 online_detection_cnt = 0;

// Э���ʼ��
void DUT_PROTOCOL_Init(void)
{
    // Э������ݽṹ��ʼ��
    DUT_PROTOCOL_DataStructInit(&uartProtocolCB3);

    // ��������ע�����ݽ��սӿ�
    UART_DRIVE_RegisterDataSendService(DUT_PROTOCOL_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    DUT_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);
}

// UARTЭ�����̴���
void DUT_UART_PROTOCOL_Process(void)
{
    // UART����FIFO����������
    DUT_PROTOCOL_RxFIFOProcess(&uartProtocolCB3);

    // UART���������������
    DUT_PROTOCOL_CmdFrameProcess(&uartProtocolCB3);

    // UARTЭ��㷢�ʹ������
    DUT_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void DUT_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB3.tx.head;
    uint16 end = uartProtocolCB3.tx.end;
    DUT_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB3.tx.end];

    // ���ͻ������������������
    if ((end + 1) % DUT_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void DUT_PROTOCOL_TxAddFrame(void)
{
    uint16 cc = 0;
    uint16 i = 0;
    uint16 head = uartProtocolCB3.tx.head;
    uint16 end = uartProtocolCB3.tx.end;
    DUT_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB3.tx.end];
    uint16 length = pCmdFrame->length;

    // ���ͻ������������������
    if ((end + 1) % DUT_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if ((length >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
            || (length + 2 >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
    {
        pCmdFrame->length = 0;

        return;
    }

    // ��β����֡�������˳�
    if (length >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }


    // �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
    pCmdFrame->buff[DUT_PROTOCOL_CMD_LENGTH_INDEX] = length - 3; // �������ݳ��ȣ���ȥ"����ͷ���豸��ַ�������֡����ݳ���"4���ֽ�

    for (i = 0; i < length; i++)
    {
        cc ^= pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = cc ;


    uartProtocolCB3.tx.end++;
    uartProtocolCB3.tx.end %= DUT_PROTOCOL_TX_QUEUE_SIZE;
}

// ���ݽṹ��ʼ��
void DUT_PROTOCOL_DataStructInit(DUT_PROTOCOL_CB *pCB)
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
    for (i = 0; i < DUT_PROTOCOL_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < DUT_PROTOCOL_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void DUT_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB3.rxFIFO.end;
    uint16 head = uartProtocolCB3.rxFIFO.head;
    uint8 rxdata = 0x00;

    // ��������
    rxdata = *pData;

//    UART_PROTOCOL4_MacProcess(standarID, pData, length);
    
    // һ���������������������
    if ((end + 1) % DUT_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // һ��������δ��������
    else
    {
        // �����յ������ݷŵ���ʱ��������
        uartProtocolCB3.rxFIFO.buff[end] = rxdata;
        uartProtocolCB3.rxFIFO.end++;
        uartProtocolCB3.rxFIFO.end %= DUT_PROTOCOL_RX_FIFO_SIZE;
    }

    // ����KM5SЭ�����
    // DUT_PROTOCOL_MacProcess(standarID, pData, length);
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void DUT_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB3.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL DUT_PROTOCOL_ConfirmTempCmdFrameBuff(DUT_PROTOCOL_CB *pCB)
{
    DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= DUT_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void DUT_PROTOCOL_TxStateProcess(void)
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
    uartProtocolCB3.tx.head %= DUT_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void DUT_PROTOCOL_RxFIFOProcess(DUT_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (DUT_PROTOCOL_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % DUT_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex %= DUT_PROTOCOL_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= DUT_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0; // 2016.1.5����
            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= DUT_PROTOCOL_RX_FIFO_SIZE;

            // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������

            // �����ж�����֡��С���ȣ�һ�����������������ٰ���8���ֽ�: ����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����˲���8���ֽڵıض�������
            if (pCmdFrame->length < DUT_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            if (pCmdFrame->buff[DUT_PROTOCOL_CMD_LENGTH_INDEX] > DUT_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - DUT_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����Ϊ����֡ʵ�ʳ���
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[DUT_PROTOCOL_CMD_LENGTH_INDEX] + DUT_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!DUT_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            DUT_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL DUT_PROTOCOL_CheckSUM(DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint16 cc = 0;
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

// UART����֡����������
void DUT_PROTOCOL_CmdFrameProcess(DUT_PROTOCOL_CB *pCB)
{
    DUT_PROTOCOL_CMD cmd = DUT_PROTOCOL_CMD_NULL;
    DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    uint8 switch_state = 0;
    uint8 Peripheral_type = 0;
    uint8 calibration_result = 0;
    uint8 turn_item = 0;
    uint8 turn_state = 0;
    uint8 buffFlag[4] = {0xFF};
    uint8 i =  0;
    uint8_t verType = 0;
    uint8_t infoLen = 0;
    char  verBuff[100] = {0};
    BOOL testResult = 0;

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
    if (DUT_PROTOCOL_CMD_HEAD != pCmdFrame->buff[DUT_PROTOCOL_CMD_HEAD_INDEX])
    {
        // ɾ������֡
        pCB->rx.head++;
        pCB->rx.head %= DUT_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (DUT_PROTOCOL_CMD)pCmdFrame->buff[DUT_PROTOCOL_CMD_CMD_INDEX];

    // ִ������֡
    switch (cmd)
    {
        // ���������ִ��
        case DUT_PROTOCOL_CMD_NULL:
            break;

        // �յ����Ӧ��
        case DUT_PROTOCOL_CMD_HEADLIGHT_CONTROL:
            switch_state = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];   // ִ�н��
            light_cnt++;

            // ִ��ʧ��
            if (0x00 == switch_state)
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }

            // ִ�гɹ�
            if (0x01 == (switch_state & 0x01))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_OPEN);
                }
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }
            break;

        case DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD:                      // ��ȡ����/ɲ����ģ������ֵ
            Peripheral_type = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];   // ��������

            // ����
            if (0x01 == (Peripheral_type & 0x01))
            {
                // ��ȡ����ADֵ
                throttle.value = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX] << 8;
                throttle.value = throttle.value | pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX];
                dut_info.throttleAd = throttle.value;
                cnt++;
                
                switch (cnt)
                {
                    case 0:
                        break;

                    case 1:
                        STATE_SwitchStep(STEP_THROTTLE_UART_TEST_CHECK_DAC1_VALUE);
                        break;

                    case 2:
                        STATE_SwitchStep(STEP_THROTTLE_UART_TEST_CHECK_DAC3_VALUE);
                        break;

                    case 3:
                        STATE_SwitchStep(STEP_THROTTLE_UART_TEST_CHECK_DAC5_VALUE);
                        cnt = 0;
                        break;

                    default:
                        break;
                }
            }

            // ɲ��
            if (0x02 == (Peripheral_type & 0x02))
            {
                // ��ȡɲ��ADֵ
                brake.value = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX] << 8;
                brake.value = brake.value | pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX];
                dut_info.brakeAd = brake.value;
                brake_cnt++;
                
                switch (brake_cnt)
                {
                    case 0:
                        break;

                    case 1:
                        STATE_SwitchStep(STEP_BRAKE_UART_TEST_CHECK_DAC1_VALUE);
                        break;

                    case 2:
                        STATE_SwitchStep(STEP_BRAKE_UART_TEST_CHECK_DAC3_VALUE);
                        break;

                    case 3:
                        STATE_SwitchStep(STEP_BRAKE_UART_TEST_CHECK_DAC5_VALUE);
                        brake_cnt = 0;
                        break;

                    default:
                        break;
                }
            }

            // ���ӱ���
            if (0x03 == (Peripheral_type & 0x03))
            {
                // ��ȡɲ��ADֵ
                derailleur.value = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX] << 8;
                derailleur.value = derailleur.value | pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX];
                dut_info.derailleurAd = derailleur.value;
                derailleur_cnt++;
                
                switch (derailleur_cnt)
                {
                    case 0:
                        break;

                    case 1:
                        STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_CHECK_DAC1_VALUE);
                        break;

                    case 2:
                        STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_CHECK_DAC3_VALUE);
                        break;

                    case 3:
                        STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_CHECK_DAC5_VALUE);
                        derailleur_cnt = 0;
                        break;

                    default:
                        break;
                }
            }
            
            break;

        // ת��ƿ���
        case DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL:
            turn_item = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];   // ת�������
            turn_state = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX];  // ִ�н��
            light_cnt++;

            // �����
            if ((turn_state) && (dut_info.test == TEST_TYPE_LBEAM) && (LBEAM == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_LBEAM_UART_TEST_OPEN);
                }
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_LBEAM_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }

            // Զ���
            if ((turn_state) && (dut_info.test == TEST_TYPE_HBEAM) && (HBEAM == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_HBEAM_UART_TEST_OPEN);
                }
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_HBEAM_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }

            // ��ת���
            if ((turn_state) && (dut_info.test == TEST_TYPE_LEFT_TURN_SIGNAL) && (LEFT_TURN_SIGNAL == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN);
                }
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }

            // ��ת���
            if ((turn_state) && (dut_info.test == TEST_TYPE_RIGHT_TURN_SIGNAL) && (RIGHT_TURN_SIGNAL == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN);
                }
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }

            // ִ��ʧ��
            if (!switch_state)
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(1, "Fail to turn on");
    #endif
                left_light_cnt = 0;
                right_light_cnt = 0;
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        // �յ��Ǳ��У׼���
        case DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION:
            cali_cnt++;
            calibration_result = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];

            // һ��У׼
            if ((calibration_result) && (dut_info.cali_cnt == 1))
            {
                STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_RESTORE_VOLTAGE_CALIBRATION);
                cali_cnt = 0;
            }

            // ����У׼
            if ((calibration_result) && (dut_info.cali_cnt == 2))
            {
                if (cali_cnt == 1)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_POWER_BOARD);
                }
                if (cali_cnt == 2)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_RESTORE_VOLTAGE_CALIBRATION);
                    cali_cnt = 0;
                }
            }

            // ����У׼
            if ((calibration_result) && (dut_info.cali_cnt == 3))
            {
                if (cali_cnt == 1)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_POWER_BOARD);
                }
                if (cali_cnt == 2)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_POWER_BOARD);
                }
                if (cali_cnt == 3)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_RESTORE_VOLTAGE_CALIBRATION);
                }
            }

            // У׼ʧ��
            if (!calibration_result)
            {
                cali_cnt = 0;
                Clear_All_Lines();
                Display_Centered(0, "Calibration");
                Display_Centered(1, "Fail");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        // ���߼��
        case DUT_PROTOCOL_CMD_ONLINE_DETECTION:
            online_detection_cnt++;            
            TIMER_ResetTimer(TIMER_ID_ONLINE_DETECT);
            
            if(1 == online_detection_cnt)
            {
               // �ϱ��Ǳ�����״̬
               STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION,TRUE);
            }
            break;

        // ��ȡ��־������
        case DUT_PROTOCOL_CMD_READ_FLAG_DATA:
            TIMER_KillTask(TIMER_ID_WRITE_FLAG_TIMEOUT);

            // ��ȡ��־������
            for (i = 0; i < 4; i++)
            {
                buffFlag[i] = pCmdFrame->buff[(stationNumber * 4) + 4 + i];
            }

            // У���־������
            if ((buffFlag[0] == 0x00) && (buffFlag[1] == 0x00) && (buffFlag[2] == 0x00) && (buffFlag[3] == 0x02))
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, TRUE);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
            }
            break;

        // д���־������
        case DUT_PROTOCOL_CMD_WRITE_FLAG_DATA:

            // �ر�д�볬ʱ��ʱ��
            TIMER_ResetTimer(TIMER_ID_WRITE_FLAG_TIMEOUT);

            // �յ�д��ɹ��ٶ�ȡ��������У��
            if (pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX])
            {
                DUT_PROTOCOL_SendCmdAck(DUT_PROTOCOL_CMD_READ_FLAG_DATA);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
            }
            break;

        // д��汾��������
        case DUT_PROTOCOL_CMD_WRITE_VERSION_TYPE_DATA:

            // �رն�ʱ��
            TIMER_ResetTimer(TIMER_ID_OPERATE_VER_TIMEOUT);

            // ��ȡ����
            verType =  pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];

            // ������ȷ����ִ�гɹ����ϱ�STS
            if ((dutverType == verType) && (pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX]))
            {
                // ��ȡ�汾������Ϣ
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_READ_VERSION_TYPE_INFO, dutverType);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_VERSION_INFO, FALSE);
            }
            break;

        // ��ȡ�汾������Ϣ
        case DUT_PROTOCOL_CMD_READ_VERSION_TYPE_INFO:
            TIMER_KillTask(TIMER_ID_OPERATE_VER_TIMEOUT);

            // ��ȡ����
            verType =  pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];

            // ��ȡ��Ϣ����
            infoLen = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX];

            // ��ȡ������Ϣ
            strcpy(verBuff, (const char *)&pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX]);
            verBuff[infoLen] = 0;

            // ֻ��ȡ
            if (verreadFlag)
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                STS_PROTOCOL_TxAddData(0x00);       // ���ݳ���
                STS_PROTOCOL_TxAddData(1);          // У���ȡ���
                STS_PROTOCOL_TxAddData(verType);    // �汾����
                STS_PROTOCOL_TxAddData(infoLen);    // ��Ϣ����

                // ������Ϣ
                for (i = 0; i < infoLen; i++)
                {
                    STS_PROTOCOL_TxAddData(pCmdFrame->buff[5 + i]);
                }
                STS_PROTOCOL_TxAddFrame();     // �汾����
                verreadFlag = FALSE;
            }

            // д��Ա�
            if (verwriteFlag)
            {
                // �汾�Ա�
                if ((dutverType == verType) && (strstr((const char *)dutverBuff, (char *)verBuff) != NULL))
                {
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                    STS_PROTOCOL_TxAddData(0x00);       // ���ݳ���
                    STS_PROTOCOL_TxAddData(1);          // У���ȡ���
                    STS_PROTOCOL_TxAddData(verType);    // �汾����
                    STS_PROTOCOL_TxAddData(infoLen);    // ��Ϣ����

                    // ������Ϣ
                    for (i = 0; i < infoLen; i++)
                    {
                        STS_PROTOCOL_TxAddData(pCmdFrame->buff[5 + i]);
                    }
                    STS_PROTOCOL_TxAddFrame();     // �汾����
                }
                else
                {
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                    STS_PROTOCOL_TxAddData(0x00);       // ���ݳ���
                    STS_PROTOCOL_TxAddData(0);          // У���ȡ���
                    STS_PROTOCOL_TxAddData(verType);    // �汾����
                    STS_PROTOCOL_TxAddData(infoLen);    // ��Ϣ����

                    // ������Ϣ
                    for (i = 0; i < infoLen; i++)
                    {
                        STS_PROTOCOL_TxAddData(pCmdFrame->buff[5 + i]);
                    }
                    STS_PROTOCOL_TxAddFrame();     // �汾����
                }
                verwriteFlag = FALSE;
            }

            break;

        // ��������
        case DUT_PROTOCOL_CMD_BLUETOOTH_MAC_ADDRESS_READ:
            
            // �������汾���Ƶ�������
            strcpy(dut_info.bleVerBuff, &pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA14_INDEX]);
            
            // ��У���븳Ϊ0
            dut_info.bleVerBuff[pCmdFrame->buff[DUT_PROTOCOL_CMD_LENGTH_INDEX] - 13] =  0;

            // �汾�ȶԳɹ���MAC��ַ������Դ������
            if (pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX])
            {
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT);
                POWER_PROTOCOL_TxAddData(12);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA4_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA5_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA6_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA7_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA8_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA9_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA10_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA11_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA12_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA13_INDEX]);                
                POWER_PROTOCOL_TxAddFrame();
            }

            // �汾�ȶ�ʧ���ϱ�STS
            if (0 == pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX])
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                STS_PROTOCOL_TxAddData(7);
                STS_PROTOCOL_TxAddData(1);
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA4_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA5_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA6_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA7_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA8_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA9_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA10_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA11_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA12_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA13_INDEX]) & 0x0F)); 
                
                for(i = 0; i < 20 ;i++)
                {
                  STS_PROTOCOL_TxAddData(dut_info.bleVerBuff[i]);
                }
                              
                STS_PROTOCOL_TxAddFrame();
            }
            break;

        // LCD ��ɫ���ԣ�������=0x03��
        case DUT_PROTOCOL_CMD_TEST_LCD:
            STS_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_TEST_LCD, TRUE);
            break;

        // Flash У�����
        case DUT_PROTOCOL_CMD_FLASH_CHECK_TEST:
            TIMER_ChangeTime(TIMER_ID_ONLINE_DETECT, 3000);
            STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
            break;

        // ��������
        case DUT_PROTOCOL_CMD_KEY_TEST:
        
            // ����ģ�����
            if(dut_info.buttonSimulationFlag)
            {
                if(keyValue == pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX])
                {
                     testResult = TRUE;
                }
                else
                {
                     testResult = FALSE;
                }
                STS_PROTOCOL_SendCmdParamTwoAck(STS_PROTOCOL_CMD_SIMULATION_KEY_TEST, testResult, pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA12_INDEX]);                
            }
            // ������������
            else
            {
                STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);         

                // LCD��ʾ
                Vertical_Scrolling_Display(ptUpMsgBuff, 4, 0);                
            }
            break;
            
        default:
            STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);

            // LCD��ʾ
            Vertical_Scrolling_Display(ptUpMsgBuff, 4, 0);     
            break;
    }

    // ɾ������֡
    pCB->rx.head++;
    pCB->rx.head %= DUT_PROTOCOL_RX_QUEUE_SIZE;
}

// ��������ظ�
void DUT_PROTOCOL_SendCmdAck(uint8 ackCmd)
{
    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_HEAD);
    DUT_PROTOCOL_TxAddData(ackCmd);
    DUT_PROTOCOL_TxAddData(0x00);         // ���ݳ���
    DUT_PROTOCOL_TxAddFrame();
}

// ��������ظ�����һ������
void DUT_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{

    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_HEAD);
    DUT_PROTOCOL_TxAddData(ackCmd);
    DUT_PROTOCOL_TxAddData(0x01);

    DUT_PROTOCOL_TxAddData(ackParam);
    DUT_PROTOCOL_TxAddFrame();
}

// ��������ظ���������������
void DUT_PROTOCOL_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam)
{
    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_HEAD);
    DUT_PROTOCOL_TxAddData(ackCmd);
    DUT_PROTOCOL_TxAddData(0x02);

    DUT_PROTOCOL_TxAddData(ackParam);
    DUT_PROTOCOL_TxAddData(two_ackParam);
    DUT_PROTOCOL_TxAddFrame();
}

// �ϱ�����״̬
//void DUT_PROTOCOL_SendOnlineStart(uint32 param)
//{
//    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION, SHUTDOWN);
//}

// д���־������
void DUT_PROTOCOL_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam)
{

    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_HEAD);
    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_WRITE_FLAG_DATA);
    DUT_PROTOCOL_TxAddData(0x06);

    DUT_PROTOCOL_TxAddData(placeParam);
    DUT_PROTOCOL_TxAddData(0x00);
    DUT_PROTOCOL_TxAddData(0x00);
    DUT_PROTOCOL_TxAddData(0x00);
    DUT_PROTOCOL_TxAddData(0x02);
    DUT_PROTOCOL_TxAddData(shutdownFlagParam);
    DUT_PROTOCOL_TxAddFrame();
}

