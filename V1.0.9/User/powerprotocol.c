#include "common.h"
#include "PowerDrive.h"
#include "PowerProtocol.h"
#include "timer.h"
#include "DutUartProtocol.h"
#include "param.h"
#include "State.h"
#include "Spiflash.h"
#include "system.h"
#include "iap.h"
#include "state.h"
#include "StsProtocol.h"
#include "AvoPin.h"
#include "DutInfo.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void POWER_PROTOCOL_DataStructInit(POWER_PROTOCOL_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void POWER_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void POWER_PROTOCOL_RxFIFOProcess(POWER_PROTOCOL_CB *pCB);

// UART����֡����������
void POWER_PROTOCOL_CmdFrameProcess(POWER_PROTOCOL_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL POWER_PROTOCOL_CheckSUM(POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL POWER_PROTOCOL_ConfirmTempCmdFrameBuff(POWER_PROTOCOL_CB *pCB);

// Э��㷢�ʹ������
void POWER_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void POWER_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================

// �Ǳ�״̬��Ϣ�ظ�
void POWER_PROTOCOL_SendCmdStatusAck(void);

//=======================================================================================
// ȫ�ֱ�������
POWER_PROTOCOL_CB POWERProtocolCB;
VOUT_CURRENT vout_Current;
uint8 power_cnt = 0;
uint8 switch_cnt = 0;

// Э���ʼ��
void POWER_PROTOCOL_Init(void)
{
    // Э������ݽṹ��ʼ��
    POWER_PROTOCOL_DataStructInit(&POWERProtocolCB);

    // ��������ע�����ݽ��սӿ�
    POWER_UART_RegisterDataSendService(POWER_PROTOCOL_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    POWER_PROTOCOL_RegisterDataSendService(POWER_UART_AddTxArray);
}

// POWERЭ�����̴���
void POWER_PROTOCOL_Process(void)
{
    // POWER����FIFO����������
    POWER_PROTOCOL_RxFIFOProcess(&POWERProtocolCB);

    // POWER���������������
    POWER_PROTOCOL_CmdFrameProcess(&POWERProtocolCB);

    // POWERЭ��㷢�ʹ������
    POWER_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void POWER_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = POWERProtocolCB.tx.head;
    uint16 end = POWERProtocolCB.tx.end;
    POWER_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &POWERProtocolCB.tx.cmdQueue[POWERProtocolCB.tx.end];

    // ���ͻ������������������
    if ((end + 1) % POWER_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void POWER_PROTOCOL_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = POWERProtocolCB.tx.head;
    uint16 end = POWERProtocolCB.tx.end;
    POWER_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &POWERProtocolCB.tx.cmdQueue[POWERProtocolCB.tx.end];
    uint16 length = pCmdFrame->length;

    // ���ͻ������������������
    if ((end + 1) % POWER_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ����֡���Ȳ��㣬������������ݣ��˳�
    if (POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN - 4 > length)  // ��ȥ"У���L��У���H��������ʶ0xD��������ʶOxA"4���ֽ�
    {
        pCmdFrame->length = 0;

        return;
    }

    // ��β����֡�������˳�
    if ((length >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
            || (length + 2 >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
    {
        pCmdFrame->length = 0;

        return;
    }

    // ��β����֡�������˳�
    if (length >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
    pCmdFrame->buff[POWER_PROTOCOL_CMD_LENGTH_INDEX] = length - 4;   // �������ݳ��ȣ���ȥ"����ͷ���豸��ַ�������֡����ݳ���"4���ֽ�
    for (i = 1; i < length; i++)
    {
        checkSum += pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);         // ���ֽ���ǰ
    pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF);  // ���ֽ��ں�

    // ������ʶ
    pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
    pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

    POWERProtocolCB.tx.end++;
    POWERProtocolCB.tx.end %= POWER_PROTOCOL_TX_QUEUE_SIZE;
    // pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void POWER_PROTOCOL_DataStructInit(POWER_PROTOCOL_CB *pCB)
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
    for (i = 0; i < POWER_PROTOCOL_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < POWER_PROTOCOL_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }

    pCB->isTimeCheck = FALSE;
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void POWER_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = POWERProtocolCB.rxFIFO.end;
    uint16 head = POWERProtocolCB.rxFIFO.head;
    uint8 rxdata = 0x00;

    // ��������
    rxdata = *pData;

    // һ���������������������
    if ((end + 1) % POWER_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // һ��������δ��������
    else
    {
        // �����յ������ݷŵ���ʱ��������
        POWERProtocolCB.rxFIFO.buff[end] = rxdata;
        POWERProtocolCB.rxFIFO.end++;
        POWERProtocolCB.rxFIFO.end %= POWER_PROTOCOL_RX_FIFO_SIZE;
    }
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void POWER_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    POWERProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL POWER_PROTOCOL_ConfirmTempCmdFrameBuff(POWER_PROTOCOL_CB *pCB)
{
    POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= POWER_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void POWER_PROTOCOL_TxStateProcess(void)
{
    uint16 head = POWERProtocolCB.tx.head;
    uint16 end = POWERProtocolCB.tx.end;
    uint16 length = POWERProtocolCB.tx.cmdQueue[head].length;
    uint8 *pCmd = POWERProtocolCB.tx.cmdQueue[head].buff;
    uint16 localDeviceID = POWERProtocolCB.tx.cmdQueue[head].deviceID;

    // ���ͻ�����Ϊ�գ�˵��������
    if (head == end)
    {
        return;
    }

    // ���ͺ���û��ע��ֱ�ӷ���
    if (NULL == POWERProtocolCB.sendDataThrowService)
    {
        return;
    }

    // Э�����������Ҫ���͵�������
    if (!(*POWERProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // ���ͻ��ζ��и���λ��
    POWERProtocolCB.tx.cmdQueue[head].length = 0;
    POWERProtocolCB.tx.head++;
    POWERProtocolCB.tx.head %= POWER_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void POWER_PROTOCOL_RxFIFOProcess(POWER_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (POWER_PROTOCOL_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % POWER_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex %= POWER_PROTOCOL_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= POWER_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0; // 2016.1.5����
            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= POWER_PROTOCOL_RX_FIFO_SIZE;

            // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ���������� ����

            // �����ж�����֡��С���ȣ�һ�����������������ٰ���8���ֽ�: ����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����˲���8���ֽڵıض�������
            if (pCmdFrame->length < POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            if (pCmdFrame->buff[POWER_PROTOCOL_CMD_LENGTH_INDEX] > (POWER_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN))
            {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У��
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[POWER_PROTOCOL_CMD_LENGTH_INDEX] + POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!POWER_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif
            // ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            POWER_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL POWER_PROTOCOL_CheckSUM(POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
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

// POWER����֡����������
void POWER_PROTOCOL_CmdFrameProcess(POWER_PROTOCOL_CB *pCB)
{
    POWER_PROTOCOL_CMD cmd = POWER_PROTOCOL_CMD_NULL;
    POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
    uint8 calibration_result = 0;
    uint16 i = 0;
    // �ҵı���

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
    if (POWER_PROTOCOL_CMD_HEAD != pCmdFrame->buff[POWER_PROTOCOL_CMD_HEAD_INDEX])
    {
        // ɾ������֡
        pCB->rx.head++;
        pCB->rx.head %= POWER_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (POWER_PROTOCOL_CMD)pCmdFrame->buff[POWER_PROTOCOL_CMD_CMD_INDEX];

    // ִ������֡
    switch (cmd)
    {
        case POWER_PROTOCOL_CMD_NULL:                                   // ������
            break;

        case POWER_PROTOCOL_CMD_CHECK_EXTERNAL_POWER:                   // ����Ƿ�����ӵ�Դ���루������=0x01��
            break;

        case POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_VOLTAGE:               // �Ǳ����ѹ��ȡ��������=0x02��
            break;

        case POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_CURRENT:               // �Ǳ��������ȡ��������=0x03��
            vout_Current.value[0] = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX];
            vout_Current.value[1] = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA2_INDEX];
            vout_Current.value[2] = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA3_INDEX];
            vout_Current.value[3] = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA4_INDEX];
            vout_Current.CURRENT = vout_Current.CURRENT;
            break;

        case POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE:        // ����DUT�����ѹ��������=0x04��
            calibration_result = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX];
            power_cnt++;

            // һ��У׼
            if ((calibration_result) && (dut_info.cali_cnt == 1))
            {
                if (power_cnt == 1)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 2)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_COMPLETE);
                    power_cnt = 0;
                }

            }

            // ����У׼
            if ((calibration_result) && (dut_info.cali_cnt == 2))
            {

                if (power_cnt == 1)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 2)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 3)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_COMPLETE);
                    power_cnt = 0;
                }

            }

            // ����У׼
            if ((calibration_result) && (dut_info.cali_cnt == 3))
            {

                if (power_cnt == 1)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 2)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 3)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 4)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_COMPLETE);
                    power_cnt = 0;
                }
            }

            // ����DUT�����ѹ
            if ((calibration_result) && (dut_info.adjustState))
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ADJUST_DUT_VOLTAGE, TRUE);
                dut_info.adjustState = FALSE;
                power_cnt = 0;

                // vlk����
                VLK_PW_EN_OFF();
            }

            // �ϱ����
            if ((!calibration_result) && (dut_info.adjustState == FALSE))
            {
                Clear_All_Lines();
                Display_Centered(0, "Calibration");
                Display_Centered(1, "Power adjustment");
                Display_Centered(2, "failure");
                power_cnt = 0;
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, FALSE);
            }
            break;

        // DUT��Դ����
        case POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH:
            switch_cnt++;

            calibration_result = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX];

            // DUT��Դ����
            if ((switch_cnt) && (dut_info.powerOnFlag))
            {
                switch_cnt = 0;
                dut_info.powerOnFlag = FALSE;
                
//                if(dut_info.passThroughControl)
//                {
//                     break;
//                }
//                else
//                {
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CONTROL_DUT_POWER, TRUE);                
//                }
            }

            // �Ǳ�����APP����
            if ((1 == switch_cnt) && (resetFlag == TRUE)  && (configResetFlag == FALSE))
            {
                Delayms(1000); // ��ʱ1s,��ַŵ�

                switch (dut_info.ID)
                {
                case DUT_TYPE_CM:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_RESTART);
                    }
                    break;

                case DUT_TYPE_SEG:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_RESTART);
                    }
                    break;

                default:
                    break;
                }
            }

            if ((2 == switch_cnt) && (resetFlag == TRUE)  && (configResetFlag == FALSE))
            {
                switch (dut_info.ID)
                {
                case DUT_TYPE_CM:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
                    }
                    break;

                case DUT_TYPE_SEG:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
                    }
                    break;

                default:
                    break;
                }
                switch_cnt = 0;
                resetFlag = FALSE;
            }

            // �Ǳ������ɹ�����
            if ((appResetFlag) && (switch_cnt))
            {
                Delayms(300); // ��ʱ300ms����ַŵ�

                // vlk����
                VLK_PW_EN_ON();

                // �����Ǳ��Դ
                POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);

                switch (dut_info.ID)
                {
                case DUT_TYPE_CM:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
                    }
                    break;

                case DUT_TYPE_SEG:
                    //                        if (dut_info.dutBusType)
                    //                        {
                    //                            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    //                        }
                    //                        else
                    //                        {
                    STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
                    //                        }
                    break;

                default:
                    break;
                }
            }
            if ((appResetFlag) && (2 == switch_cnt))
            {
                switch (dut_info.ID)
                {
                case DUT_TYPE_CM:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_YELLOW);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
                    }
                    break;

                case DUT_TYPE_SEG:
                    //                        if (dut_info.dutBusType)
                    //                        {
                    //                            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    //                        }
                    //                        else
                    //                        {
                    STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH);
                    //                        }
                    break;

                default:
                    break;
                }
                appResetFlag = FALSE;
                switch_cnt = 0;
            }

            // �Ǳ�������������
            if ((1 == switch_cnt) && (resetFlag == FALSE) && (configResetFlag == TRUE))
            {
                Delayms(1000); // ��ʱ1s����ַŵ�

                if (dut_info.dutBusType)
                {
                    STATE_SwitchStep(STEP_CAN_CONFIG_RESET);
                }
                else
                {
                    STATE_SwitchStep(STEP_UART_CONFIG_RESET);
                }
            }

            if ((2 == switch_cnt) && (resetFlag == FALSE) && (configResetFlag == TRUE))
            {
                if (dut_info.dutBusType)
                {
                    STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

                    // ��������
                    TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                                  500,
                                  STATE_SwitchStep,
                                  STEP_CAN_SEND_SET_CONFIG,
                                  TIMER_LOOP_FOREVER,
                                  ACTION_MODE_ADD_TO_QUEUE);
                }
                else
                {
                    TIMER_AddTask(TIMER_ID_PROTOCOL_SWITCCH,
                                  500,
                                  STATE_SwitchStep,
                                  STEP_UART_PROTOCOL_SWITCCH,
                                  TIMER_LOOP_FOREVER,
                                  ACTION_MODE_ADD_TO_QUEUE);
                }

                switch_cnt = 0;
                configResetFlag = FALSE;
            }
            break;

        // ��������
        case POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT:

            // ���Գɹ�
            if (pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX])
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                STS_PROTOCOL_TxAddData(0x07);
                STS_PROTOCOL_TxAddData(2);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA2_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA3_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA4_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA5_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA6_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA7_INDEX]);
                
                for(i = 0; i < 20 ;i++)
                {
                  STS_PROTOCOL_TxAddData(dut_info.bleVerBuff[i]);
                }
                
                STS_PROTOCOL_TxAddFrame();
            }

            // �޷����Ӳ���ʧ��
            if (!pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX])
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                STS_PROTOCOL_TxAddData(0x07);
                STS_PROTOCOL_TxAddData(0);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA2_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA3_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA4_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA5_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA6_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA7_INDEX]);
                
                for(i = 0; i < 20 ;i++)
                {
                  STS_PROTOCOL_TxAddData(dut_info.bleVerBuff[i]);
                }
                STS_PROTOCOL_TxAddFrame();
            }
            break;

        // ��ȡPOWER�����Ϣ
        case POWER_PROTOCOL_CMD_GET_SOFTWARE_INFO:
            STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
            STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_POWER_GET_SOFTWARE_INFO);
            STS_PROTOCOL_TxAddData(6);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA2_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA3_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA4_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA5_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA6_INDEX]);
            STS_PROTOCOL_TxAddFrame();
            break;

        // APP2���ݲ���
        case POWER_PROTOCOL_CMD_ECO_APP2_ERASE:

            // �ϱ��������
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_POWER_UP_APP_ERASE, pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            break;

        // APP2����д��
        case POWER_PROTOCOL_CMD_ECO_APP2_WRITE:

            // �ϱ�д����
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_POWER_UP_APP_WRITE, pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            break;

        // APP2�������
        case POWER_PROTOCOL_CMD_ECO_APP_WRITE_FINISH:

            // �ϱ�д����
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_POWER_UP_APP_WRITE_FINISH, pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            break;

        // ��������
        case POWER_PROTOCOL_CMD_RST:

            // �ϱ��������
            STS_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_RST, pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            break;

        // �����ɹ��ϱ�
        case POWER_PROTOCOL_CMD_UP_FINISH:

            // �ϱ��������
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_POWER_UP_FINISH, 1);
            POWER_PROTOCOL_SendCmdAck(POWER_PROTOCOL_CMD_UP_FINISH);
            break;

        default:
            break;
        }

    // ɾ������֡
    pCB->rx.head++;
    pCB->rx.head %= POWER_PROTOCOL_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void BLE_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
    POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    pCmdFrame = &POWERProtocolCB.rx.cmdQueue[POWERProtocolCB.rx.end];

    // ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
    pCmdFrame->length = 0; // 2016.1.6����
    // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
    POWERProtocolCB.rxFIFO.head++;
    POWERProtocolCB.rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
    POWERProtocolCB.rxFIFO.currentProcessIndex = POWERProtocolCB.rxFIFO.head;
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
void POWER_PROTOCOL_SendCmdAck(uint8 ackCmd)
{
    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
    POWER_PROTOCOL_TxAddData(ackCmd);
    POWER_PROTOCOL_TxAddData(0x00);
    POWER_PROTOCOL_TxAddFrame();
}

// ��������ظ�����һ������
void POWER_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
    POWER_PROTOCOL_TxAddData(ackCmd);
    POWER_PROTOCOL_TxAddData(0x01);
    POWER_PROTOCOL_TxAddData(ackParam);
    POWER_PROTOCOL_TxAddFrame();
}


