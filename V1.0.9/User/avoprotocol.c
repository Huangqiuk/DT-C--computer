#include "AvoDrive.h"
#include "AvoProtocol.h"
#include "common.h"
#include "timer.h"
#include "Spiflash.h"
#include "system.h"
#include "iap.h"
#include "AvoPin.h"
#include "StsProtocol.h"
#include "param.h"
#include "DutUartProtocol.h"
#include "CanProtocol_3A.h"
#include "state.h"
#include "DutInfo.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void AVO_PROTOCOL_DataStructInit(AVO_PROTOCOL_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void AVO_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void AVO_PROTOCOL_RxFIFOProcess(AVO_PROTOCOL_CB *pCB);

// UART����֡����������
void AVO_PROTOCOL_CmdFrameProcess(AVO_PROTOCOL_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL AVO_PROTOCOL_CheckSUM(AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL AVO_PROTOCOL_ConfirmTempCmdFrameBuff(AVO_PROTOCOL_CB *pCB);

// Э��㷢�ʹ������
void AVO_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void AVO_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// ��������ظ�
void AVO_PROTOCOL_SendCmdAck(uint8 ackCmd);

// ���ʹ�Ӧ��Ļظ�
void AVO_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// �Ǳ�״̬��Ϣ�ظ�
void AVO_PROTOCOL_SendCmdStatusAck(void);

//=======================================================================================
// ȫ�ֱ�������
AVO_PROTOCOL_CB AVOProtocolCB;
MEASURE_RESULT measure;
OHM ohm;
uint8 headlight_cnt = 0;
uint8 gnd_cnt = 0;

// ���ñ������ʾ
char avoTestMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "Meas Completed",
};

// Э���ʼ��
void AVO_PROTOCOL_Init(void)
{
    // Э������ݽṹ��ʼ��
    AVO_PROTOCOL_DataStructInit(&AVOProtocolCB);

    // ��������ע�����ݽ��սӿ�
    AVO_UART_RegisterDataSendService(AVO_PROTOCOL_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    AVO_PROTOCOL_RegisterDataSendService(AVO_UART_AddTxArray);
}

// AVOЭ�����̴���
void AVO_PROTOCOL_Process(void)
{
    // AVO����FIFO����������
    AVO_PROTOCOL_RxFIFOProcess(&AVOProtocolCB);

    // AVO���������������
    AVO_PROTOCOL_CmdFrameProcess(&AVOProtocolCB);

    // AVOЭ��㷢�ʹ������
    AVO_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void AVO_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = AVOProtocolCB.tx.head;
    uint16 end = AVOProtocolCB.tx.end;
    AVO_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &AVOProtocolCB.tx.cmdQueue[AVOProtocolCB.tx.end];

    // ���ͻ������������������
    if ((end + 1) % AVO_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void AVO_PROTOCOL_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = AVOProtocolCB.tx.head;
    uint16 end = AVOProtocolCB.tx.end;
    AVO_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &AVOProtocolCB.tx.cmdQueue[AVOProtocolCB.tx.end];
    uint16 length = pCmdFrame->length;

    // ���ͻ������������������
    if ((end + 1) % AVO_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ����֡���Ȳ��㣬������������ݣ��˳�
    if (AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length) // ��ȥ"У���"1���ֽ�
    {
        pCmdFrame->length = 0;

        return;
    }

    // ��β����֡�������˳�
    if (length >= AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
    // �������ݳ���,��Ҫ��ȥ10=(3ͬ��ͷ1������+1���ݳ���)
    pCmdFrame->buff[AVO_PROTOCOL_CMD_LENGTH_INDEX] = length - 5;

    //cc = UART_GetCRCValue(&pCmdFrame->buff[UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - UART_PROTOCOL_HEAD_BYTE));
    /* ���ȡ������У���� */
    for (i = 0; i < pCmdFrame->length; i++)
    {
        checkSum ^= pCmdFrame->buff[i];
    }
    checkSum = ~checkSum;

    pCmdFrame->buff[length] = checkSum;
    pCmdFrame->length ++;

    AVOProtocolCB.tx.end++;
    AVOProtocolCB.tx.end %= AVO_PROTOCOL_TX_QUEUE_SIZE;
    // pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void AVO_PROTOCOL_DataStructInit(AVO_PROTOCOL_CB *pCB)
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
    for (i = 0; i < AVO_PROTOCOL_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < AVO_PROTOCOL_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }

    //pCB->isTimeCheck = FALSE;
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void AVO_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = AVOProtocolCB.rxFIFO.end;
    uint16 head = AVOProtocolCB.rxFIFO.head;
    uint8 rxdata = 0x00;

    // ��������
    rxdata = *pData;

    // һ���������������������
    if ((end + 1) % AVO_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // һ��������δ��������
    else
    {
        // �����յ������ݷŵ���ʱ��������
        AVOProtocolCB.rxFIFO.buff[end] = rxdata;
        AVOProtocolCB.rxFIFO.end++;
        AVOProtocolCB.rxFIFO.end %= AVO_PROTOCOL_RX_FIFO_SIZE;
    }
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void AVO_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    AVOProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL AVO_PROTOCOL_ConfirmTempCmdFrameBuff(AVO_PROTOCOL_CB *pCB)
{
    AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= AVO_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void AVO_PROTOCOL_TxStateProcess(void)
{
    uint16 head = AVOProtocolCB.tx.head;
    uint16 end = AVOProtocolCB.tx.end;
    uint16 length = AVOProtocolCB.tx.cmdQueue[head].length;
    uint8 *pCmd = AVOProtocolCB.tx.cmdQueue[head].buff;
    uint16 localDeviceID = AVOProtocolCB.tx.cmdQueue[head].deviceID;

    // ���ͻ�����Ϊ�գ�˵��������
    if (head == end)
    {
        return;
    }

    // ���ͺ���û��ע��ֱ�ӷ���
    if (NULL == AVOProtocolCB.sendDataThrowService)
    {
        return;
    }

    // Э�����������Ҫ���͵�������
    if (!(*AVOProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // ���ͻ��ζ��и���λ��
    AVOProtocolCB.tx.cmdQueue[head].length = 0;
    AVOProtocolCB.tx.head++;
    AVOProtocolCB.tx.head %= AVO_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void AVO_PROTOCOL_RxFIFOProcess(AVO_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (AVO_PROTOCOL_CMD_HEAD1 != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % AVO_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
        {
            return;
        }

        // ���UARTͨѶ��ʱʱ������-2016.1.5����
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
        TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
                      AVO_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
                      AVO_PROTOCOL_CALLBACK_RxTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
#endif

        // ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
        pCmdFrame->buff[pCmdFrame->length++] = currentData;
        pCB->rxFIFO.currentProcessIndex++;
        pCB->rxFIFO.currentProcessIndex %= AVO_PROTOCOL_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= AVO_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0; // 2016.1.5����
            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= AVO_PROTOCOL_RX_FIFO_SIZE;

            // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ���������� ����

            // �����ж�����֡��С���ȣ�һ�����������������ٰ���8���ֽ�: ����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����˲���8���ֽڵıض�������
            if (pCmdFrame->length < AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            if (pCmdFrame->buff[AVO_PROTOCOL_CMD_LENGTH_INDEX] > (AVO_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN))
            {
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                AVO_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У��
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[AVO_PROTOCOL_CMD_LENGTH_INDEX] + AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!AVO_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                AVO_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            AVO_PROTOCOL_StopRxTimeOutCheck();
#endif
            // ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            AVO_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL AVO_PROTOCOL_CheckSUM(AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint8_t checkSum = 0;
    uint8_t sumTemp;
    uint16_t i = 0;

    /* �����Ϸ��Լ��� */
    if (NULL == pCmdFrame)
    {
        return FALSE;
    }

    /* ���豸��ַ��ʼ����У����֮ǰ��һ���ֽڣ����ν���������� */
    for (i = 0;  i < pCmdFrame->length - 1; i++)
    {
        checkSum ^= pCmdFrame->buff[i];
    }

    /* ����ȡ�� */
    checkSum = ~checkSum ;

    /* ��ȡ����֡��У���� */
    sumTemp = pCmdFrame->buff[pCmdFrame->length - 1];

    /* �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ */
    if (sumTemp != checkSum)
    {
        return FALSE;
    }

    return TRUE;
}

// �ϱ�����ִ�н��
void AVO_PROTOCOL_SendCmdResult(uint32 cmd, uint32 param)
{
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD1);
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD2);
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD3);

    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_PROTOCOL_VERSION);
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_DEVICE_ADDR);
    AVO_PROTOCOL_TxAddData(cmd);
    AVO_PROTOCOL_TxAddData(0);
    AVO_PROTOCOL_TxAddData(2);

    // д���ֽ���
    AVO_PROTOCOL_TxAddData(1);

    // д����
    AVO_PROTOCOL_TxAddData((uint8)param);

    AVO_PROTOCOL_TxAddFrame();
}

// �·�������������, ͨ��Ĭ��Ϊ0
void AVO_PROTOCOL_Send_Mesuretype(uint32 mesuretype)
{
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD1);
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD2);
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD3);
    AVO_PROTOCOL_TxAddData(AVO_CMD_AVOMETER);
    // Length
    AVO_PROTOCOL_TxAddData(0x03);
    // Target board number
    AVO_PROTOCOL_TxAddData(BOARD_AVO);
    AVO_PROTOCOL_TxAddData(mesuretype);
    AVO_PROTOCOL_TxAddData(0);
    // Assemble command frame
    AVO_PROTOCOL_TxAddFrame();
}

// �·���λ����
void AVO_PROTOCOL_Send_Reset(void)
{
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD1);
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD2);
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD3);
    AVO_PROTOCOL_TxAddData(AVO_CMD_AVORESET);
    // Length
    AVO_PROTOCOL_TxAddData(0x01);
    // Target board number
    AVO_PROTOCOL_TxAddData(BOARD_AVO);
    // Assemble command frame
    AVO_PROTOCOL_TxAddFrame();
}

// �·��������ͺͲ���ͨ������
void AVO_PROTOCOL_Send_Type_Chl(uint32 mesuretype, uint32 mesurechl)
{
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD1);
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD2);
    AVO_PROTOCOL_TxAddData(AVO_PROTOCOL_CMD_HEAD3);
    AVO_PROTOCOL_TxAddData(AVO_CMD_AVOMETER);
    // Length
    AVO_PROTOCOL_TxAddData(0x03);
    // Target board number
    AVO_PROTOCOL_TxAddData(BOARD_AVO);
    AVO_PROTOCOL_TxAddData(mesuretype);
    AVO_PROTOCOL_TxAddData(mesurechl);
    // Assemble command frame
    AVO_PROTOCOL_TxAddFrame();

}

void SysRest(uint32 param)
{
    __set_FAULTMASK(1); // �ر������ж�
    NVIC_SystemReset(); // ��λ
}

// AVO����֡����������
void AVO_PROTOCOL_CmdFrameProcess(AVO_PROTOCOL_CB *pCB)
{
    AVO_PROTOCOL_CMD cmd = AVO_CMD_NULL;
    AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
    AVO_MEASURE_TYPE mesureType;

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
    if (AVO_PROTOCOL_CMD_HEAD1 != pCmdFrame->buff[AVO_PROTOCOL_CMD_HEAD1_INDEX])
    {
        // ɾ������֡
        pCB->rx.head ++;
        pCB->rx.head %= AVO_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (AVO_PROTOCOL_CMD)pCmdFrame->buff[AVO_PROTOCOL_CMD_INDEX];

    // ִ������֡
    switch (cmd)
    {
        case AVO_CMD_NULL:
            break;

        case AVO_CMD_AVOMETER_ACK:
            mesureType = (AVO_MEASURE_TYPE)pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA1_INDEX];

            switch (mesureType)
            {
                // ֱ����ѹ��V��
                case MEASURE_DCV :
                    measure.data[0] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA3_INDEX];
                    measure.data[1] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA4_INDEX];
                    measure.data[2] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA5_INDEX];
                    measure.data[3] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA6_INDEX];
                    measure.result = measure.result * 1000.0f;

                    // ���в���ʹ�����Ÿ�λ
                    AVO_PIN_Reset();

                    // LCD��ʾ
                    Vertical_Scrolling_Display(avoTestMsgBuff, 4, 0);

                    switch (dut_info.dutBusType)
                    {
                        case 0: // ����
                            switch (dut_info.test)
                            {
                                case TEST_TYPE_NULL:
                                    break;

                                case TEST_TYPE_UART:
                                    STATE_SwitchStep(STEP_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                // ��Ʋ���
                                case TEST_TYPE_HEADLIGHT:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                // �����
                                case TEST_TYPE_LBEAM:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LBEAM_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LBEAM_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                // Զ���
                                case TEST_TYPE_HBEAM:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HBEAM_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HBEAM_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                // ��ת���
                                case TEST_TYPE_LEFT_TURN_SIGNAL:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                // ��ת���
                                case TEST_TYPE_RIGHT_TURN_SIGNAL:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                case TEST_TYPE_THROTTLE:
                                    STATE_SwitchStep(STEP_THROTTLE_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                case TEST_TYPE_BRAKE:
                                    STATE_SwitchStep(STEP_BRAKE_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                case TEST_TYPE_VLK:
                                    STATE_SwitchStep(STEP_VLK_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                // (С��)��Ʋ���
                                case TEST_TYPE_XM_HEADLIGHT:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;
                                    
                                // (С��)���Ų���
                                case TEST_TYPE_XM_THROTTLE:
                                    STATE_SwitchStep(STEP_THROTTLE_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                // (С��)ɲ�Ѳ���
                                case TEST_TYPE_XM_BRAKE:
                                    STATE_SwitchStep(STEP_BRAKE_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    break;
                                    
                                // (С��)��ת���
                                case TEST_TYPE_XM_LEFT_TURN_SIGNAL:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    
//                                    if (3 == headlight_cnt)
//                                    {
//                                        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2);
//                                        headlight_cnt = 0;
//                                    }                                    
                                    break;

                                // (С��)��ת���
                                case TEST_TYPE_XM_RIGHT_TURN_SIGNAL:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    
//                                    if (3 == headlight_cnt)
//                                    {
//                                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2);
//                                    }  
//                                    
//                                    if (4 == headlight_cnt)
//                                    {
//                                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE3);
//                                        headlight_cnt = 0;
//                                    }                                        
                                    break;                                    
                                    
                                default:
                                    break;
                                }

                            break;

                        case 1: // can
                            switch (dut_info.test)
                            {
                                case TEST_TYPE_NULL:
                                    break;

                                // ��Ʋ���
                                case TEST_TYPE_HEADLIGHT:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                // �����
                                case TEST_TYPE_LBEAM:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                // Զ���
                                case TEST_TYPE_HBEAM:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                // ��ת���
                                case TEST_TYPE_LEFT_TURN_SIGNAL:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                // ��ת���
                                case TEST_TYPE_RIGHT_TURN_SIGNAL:
                                    headlight_cnt++;
                                    if (1 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    }

                                    if (2 == headlight_cnt)
                                    {
                                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE);
                                        headlight_cnt = 0;
                                    }
                                    break;

                                case TEST_TYPE_THROTTLE:
                                    STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                case TEST_TYPE_BRAKE:
                                    STATE_SwitchStep(STEP_BRAKE_CAN_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                case TEST_TYPE_VLK:
                                    STATE_SwitchStep(STEP_VLK_CAN_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                default:
                                    break;
                            }

                            break;
                        }

                    break;

                // ���裨����
                case MEASURE_OHM :
                    ohm.data[0] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA3_INDEX];
                    ohm.data[1] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA4_INDEX];
                    ohm.data[2] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA5_INDEX];
                    ohm.data[3] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA6_INDEX];
                    ohm.INT = ohm.INT;

                    // ���в���ʹ�����Ÿ�λ
                    AVO_PIN_Reset();

                    // LCD��ʾ
                    Vertical_Scrolling_Display(avoTestMsgBuff, 4, 0);

                    switch (dut_info.dutBusType)
                    {
                        case 0: // ����
                            switch (dut_info.test)
                            {
                                case TEST_TYPE_NULL:
                                    break;

                                case TEST_TYPE_GND:
                                    gnd_cnt++;
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_THROTTLE_BRAKE))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE);
                                    }
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_BRAKE))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_BRAKE_GND_VALUE);
                                    }
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_THROTTLE))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE);
                                    }
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_DERAILLEUR))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_DERAILLEUR_GND_VALUE);
                                    }                                    
                                    if ((2 == gnd_cnt) && (dut_info.gnd == GND_TYPE_THROTTLE_BRAKE))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_BRAKE_GND_VALUE);
                                        gnd_cnt = 0;
                                    }
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_BRAKE_DERAILLEUR))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_BRAKE_GND_VALUE);
                                    }                                        
                                    if ((2 == gnd_cnt) && (dut_info.gnd == GND_TYPE_BRAKE_DERAILLEUR))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_DERAILLEUR_GND_VALUE);
                                        gnd_cnt = 0;
                                    }                                    
                                    break;

                                case TEST_TYPE_THROTTLE:
                                    STATE_SwitchStep(STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE);
                                    break;

                                case TEST_TYPE_BRAKE:
                                    STATE_SwitchStep(STEP_GND_TEST_CHECK_BRAKE_GND_VALUE);
                                    break;

                                default:
                                    break;
                            }
                            break;

                        case 1: // can
                            switch (dut_info.test)
                            {
                                case TEST_TYPE_NULL:
                                    break;

                                case TEST_TYPE_GND:
                                    gnd_cnt++;
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_THROTTLE_BRAKE))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE);
                                    }
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_BRAKE))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_BRAKE_GND_VALUE);
                                    }
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_THROTTLE))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE);
                                    }
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_DERAILLEUR))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_DERAILLEUR_GND_VALUE);
                                    }                                    
                                    if ((2 == gnd_cnt) && (dut_info.gnd == GND_TYPE_THROTTLE_BRAKE))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_BRAKE_GND_VALUE);
                                        gnd_cnt = 0;
                                    }
                                    if ((1 == gnd_cnt) && (dut_info.gnd == GND_TYPE_BRAKE_DERAILLEUR))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_BRAKE_GND_VALUE);
                                    }                                        
                                    if ((2 == gnd_cnt) && (dut_info.gnd == GND_TYPE_BRAKE_DERAILLEUR))
                                    {
                                        STATE_SwitchStep(STEP_GND_TEST_CHECK_DERAILLEUR_GND_VALUE);
                                        gnd_cnt = 0;
                                    }                                    
                                    break;

                                case TEST_TYPE_THROTTLE:
                                    STATE_SwitchStep(STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE);
                                    break;

                                case TEST_TYPE_BRAKE:
                                    STATE_SwitchStep(STEP_GND_TEST_CHECK_BRAKE_GND_VALUE);
                                    break;

                                default:
                                    break;
                            }
                            break;
                        }
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
    pCB->rx.head %= AVO_PROTOCOL_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void AVO_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
    AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    pCmdFrame = &AVOProtocolCB.rx.cmdQueue[AVOProtocolCB.rx.end];

    // ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
    pCmdFrame->length = 0; // 2016.1.6����
    // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
    AVOProtocolCB.rxFIFO.head++;
    AVOProtocolCB.rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
    AVOProtocolCB.rxFIFO.currentProcessIndex = AVOProtocolCB.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void UART_PROTOCOL_StopRxTimeOutCheck(void)
{
    TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

//// ������������·����������־
//void UART_PROTOCOL_SetTxAtOnceRequest(uint32 param)
//{
//    AVOProtocolCB.txAtOnceRequest = (BOOL)param;
//}

//// ������������·����������־
//void UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param)
//{
//    AVOProtocolCB.txPeriodRequest = (BOOL)param;
//}


