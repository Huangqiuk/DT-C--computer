#include "common.h"
#include "system.h"
#include "timer.h"
#include "uartDrive.h"
#include "LimeUartProtocol.h"
#include "iap.h"
#include "param.h"
#include "state.h"
#include "DutInfo.h"
#include "spiFlash.h"

/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void LIME_UART_PROTOCOL_DataStructInit(LIME_UART_PROTOCOL_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void LIME_UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void LIME_UART_PROTOCOL_RxFIFOProcess(LIME_UART_PROTOCOL_CB *pCB);

// UART����֡����������
void LIME_UART_PROTOCOL_CmdFrameProcess(LIME_UART_PROTOCOL_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL LIME_UART_PROTOCOL_CheckSUM(LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL LIME_UART_PROTOCOL_ConfirmTempCmdFrameBuff(LIME_UART_PROTOCOL_CB *pCB);

// ͨѶ��ʱ����-����
void LIME_UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void LIME_UART_PROTOCOL_StopRxTimeOutCheck(void);

// Э��㷢�ʹ������
void LIME_UART_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void LIME_UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// ������������·����������־
void LIME_UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param);

// UART���߳�ʱ������
void LIME_UART_PROTOCOL_CALLBACK_UartBusError(uint32 param);

//=========================================================================
void LIME_UART_PROTOCOL_SendCmdTwoBytesDataAck(uint8 *pBuf, uint8 length1, uint8 length2);
void LIME_UART_PROTOCOL_SendCmdOneByteDataAck(uint8 *pBuf, uint8 length1);
void LIME_UART_PROTOCOL_SendCmdNoDataAck(uint8 *pBuf);
void LIME_UART_PROTOCOL_SendCmdAppVersion(uint8 *pBuf);

// �汾��������
void LIME_UART_PROTOCOL_SendCmdVersion(uint8 versionCmd, uint8 *pBuf);

// ��ȡFLASH�ظ�
void LIME_UART_PROTOCOL_SendCmdReadFlashRespond(uint32 length, uint8 *pBuf);

// �ϵ緢��ƥ������
void LIME_UART_PROTOCOL_SendSnMatching(uint8 length, uint8 *Sn);

// ȫ�ֱ�������
LIME_UART_PROTOCOL_CB uartProtocolCB4;


// ��������������������������������������������������������������������������������������

uint32 uartProtocolOutTimeDelay[6] = {30 * 60 * 1000,       // ������
                                      30 * 60 * 1000,         // LOGO
                                      30 * 60 * 1000,         // ��ʾ
                                      1000,                   // ��������
                                      1000,                   // �쳣������ʾ
                                      30 * 60 * 1000,
                                     };// ����

// use the crc8
uint8 UART_GetCRCValue(uint8 *ptr, uint16 len)
{
    uint8 crc;
    uint8 i;

    crc = 0;

    while (len--)
    {
        crc ^= *ptr++;

        for (i = 0; i < 8; i++)
        {
            if (crc & 0x01)
            {
                crc = (crc >> 1) ^ 0x8C;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

// Э���ʼ��
void LIME_UART_PROTOCOL_Init(void)
{
    // Э������ݽṹ��ʼ��
    LIME_UART_PROTOCOL_DataStructInit(&uartProtocolCB4);

    // ��������ע�����ݽ��սӿ�
    UART_DRIVE_RegisterDataSendService(LIME_UART_PROTOCOL_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    LIME_UART_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);
}

// UARTЭ�����̴���
void LIME_UART_PROTOCOL_Process(void)
{
    // UART����FIFO����������
    LIME_UART_PROTOCOL_RxFIFOProcess(&uartProtocolCB4);

    // UART���������������
    LIME_UART_PROTOCOL_CmdFrameProcess(&uartProtocolCB4);

    // UARTЭ��㷢�ʹ������
    LIME_UART_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void LIME_UART_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end =  uartProtocolCB4.tx.end;
    LIME_UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB4.tx.cmdQueue[uartProtocolCB4.tx.end];

    // ���ͻ������������������
    if ((end + 1) % LIME_UART_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void LIME_UART_PROTOCOL_TxAddFrame(void)
{
    uint8 cc = 0;
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end  = uartProtocolCB4.tx.end;
    LIME_UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB4.tx.cmdQueue[uartProtocolCB4.tx.end];
    uint16 length = pCmdFrame->length;

    // ���ͻ������������������
    if ((end + 1) % LIME_UART_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ����֡���Ȳ��㣬������������ݣ��˳�
    if (LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length) // ��ȥ"У����"1���ֽ�
    {
        pCmdFrame->length = 0;

        return;
    }

    // ��β����֡�������˳�
    if (length >= LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
    // �������ݳ��ȣ���Ҫ��ȥ10=(5ͬ��ͷ+1Э��汾+1�豸��+1������+2���ݳ���)
    pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_LENGTH_H_INDEX] = ((length - 10) & 0xFF00) >> 8;
    pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_LENGTH_L_INDEX] = ((length - 10) & 0x00FF);

    cc = UART_GetCRCValue(&pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - LIME_UART_PROTOCOL_HEAD_BYTE));
    pCmdFrame->buff[length] = cc;
    pCmdFrame->length ++;

    uartProtocolCB4.tx.end ++;
    uartProtocolCB4.tx.end %= LIME_UART_PROTOCOL_TX_QUEUE_SIZE;
    //pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void LIME_UART_PROTOCOL_DataStructInit(LIME_UART_PROTOCOL_CB *pCB)
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
    for (i = 0; i < LIME_UART_PROTOCOL_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end  = 0;
    for (i = 0; i < LIME_UART_PROTOCOL_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void LIME_UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB4.rxFIFO.end;
    uint16 head = uartProtocolCB4.rxFIFO.head;
    uint8 rxdata = 0x00;

    // ��������
    rxdata = *pData;

    // һ���������������������
    if ((end + 1) % LIME_UART_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // һ��������δ��������
    else
    {
        // �����յ������ݷŵ���ʱ��������
        uartProtocolCB4.rxFIFO.buff[end] = rxdata;
        uartProtocolCB4.rxFIFO.end ++;
        uartProtocolCB4.rxFIFO.end %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
    }
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void LIME_UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB4.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL LIME_UART_PROTOCOL_ConfirmTempCmdFrameBuff(LIME_UART_PROTOCOL_CB *pCB)
{
    LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end ++;
    pCB->rx.end %= LIME_UART_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0;   // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void LIME_UART_PROTOCOL_TxStateProcess(void)
{
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end =  uartProtocolCB4.tx.end;
    uint16 length = uartProtocolCB4.tx.cmdQueue[head].length;
    uint8 *pCmd = uartProtocolCB4.tx.cmdQueue[head].buff;
    uint16 localDeviceID = uartProtocolCB4.tx.cmdQueue[head].deviceID;

    // ���ͻ�����Ϊ�գ�˵��������
    if (head == end)
    {
        return;
    }

    // ���ͺ���û��ע��ֱ�ӷ���
    if (NULL == uartProtocolCB4.sendDataThrowService)
    {
        return;
    }

    // Э�����������Ҫ���͵�������
    if (!(*uartProtocolCB4.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // ���ͻ��ζ��и���λ��
    uartProtocolCB4.tx.cmdQueue[head].length = 0;
    uartProtocolCB4.tx.head ++;
    uartProtocolCB4.tx.head %= LIME_UART_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void LIME_UART_PROTOCOL_RxFIFOProcess(LIME_UART_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (LIME_UART_PROTOCOL_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head ++;
            pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % LIME_UART_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
        {
            return;
        }

        // ���UARTͨѶ��ʱʱ������-2016.1.5����
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
        TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
                      LIME_UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
                      LIME_UART_PROTOCOL_CALLBACK_RxTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
#endif

        // ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
        pCmdFrame->buff[pCmdFrame->length++] = currentData;
        pCB->rxFIFO.currentProcessIndex ++;
        pCB->rxFIFO.currentProcessIndex %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= LIME_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            LIME_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0;  // 2016.1.5����
            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head ++;
            pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex ++;
            pCB->rxFIFO.currentProcessIndex %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;

            // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������

            // �����ж�����֡��С���ȣ�һ�����������������ٰ���10���ֽ�,��˲���8���ֽڵıض�������
            if (pCmdFrame->length < LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            length = ((uint16)pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_LENGTH_H_INDEX] << 8) + pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_LENGTH_L_INDEX];
            if (length > LIME_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                LIME_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head ++;
                pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У�飬������������ֵ���ֵ�ϣ���������֡��С���ȣ���Ϊ����֡ʵ�ʳ���
            if (pCmdFrame->length < (length + LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN))
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!LIME_UART_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                LIME_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head ++;
                pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            LIME_UART_PROTOCOL_StopRxTimeOutCheck();
#endif
            length = pCmdFrame->length;
            // ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            LIME_UART_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL LIME_UART_PROTOCOL_CheckSUM(LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint8 cc = 0;

    if (NULL == pCmdFrame)
    {
        return FALSE;
    }

    // ��Э��汾��ʼ����У����֮ǰ��һ���ֽڣ����ν���CRC8����
    cc = UART_GetCRCValue(&pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - (LIME_UART_PROTOCOL_HEAD_BYTE + LIME_UART_PROTOCOL_CHECK_BYTE)));

    // �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
    if (pCmdFrame->buff[pCmdFrame->length - 1] != cc)
    {
        return FALSE;
    }

    return TRUE;

}

// �ϱ�����ִ�н��
void LIME_UART_PROTOCOL_SendCmdResult(uint32 cmd, uint32 param)
{
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD1);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD2);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD3);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD4);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD5);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_DEVICE_ADDR);
    LIME_UART_PROTOCOL_TxAddData(cmd);
    LIME_UART_PROTOCOL_TxAddData(0);
    LIME_UART_PROTOCOL_TxAddData(2);

    // д���ֽ���
    LIME_UART_PROTOCOL_TxAddData(1);

    // д����
    LIME_UART_PROTOCOL_TxAddData((uint8)param);

    LIME_UART_PROTOCOL_TxAddFrame();
}


// �ϱ�����Ӧ��
void LIME_UART_PROTOCOL_SendCmdAck(uint32 cmd)
{
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD1);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD2);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD3);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD4);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD5);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_DEVICE_ADDR);
    LIME_UART_PROTOCOL_TxAddData(cmd);
    LIME_UART_PROTOCOL_TxAddData(0);
    LIME_UART_PROTOCOL_TxAddData(0);

    LIME_UART_PROTOCOL_TxAddFrame();
}

// �ϱ�ģ��д����
void LIME_UART_PROTOCOL_SendCmdWriteResult(uint32 cmd, uint16 byteNum, uint32 exeResoult)
{
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD1);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD2);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD3);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD4);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD5);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_DEVICE_ADDR);
    LIME_UART_PROTOCOL_TxAddData(cmd);
    LIME_UART_PROTOCOL_TxAddData(0);
    LIME_UART_PROTOCOL_TxAddData(2);

    // д���ֽ���
    LIME_UART_PROTOCOL_TxAddData(byteNum / 256);
    LIME_UART_PROTOCOL_TxAddData(byteNum % 256);

    // д����
    LIME_UART_PROTOCOL_TxAddData((uint8)exeResoult);

    LIME_UART_PROTOCOL_TxAddFrame();
}

// �������ʾ
void LIME_UART_PROTOCOL_Seg(uint32 cmd, uint16 d1, uint16 d2)
{
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD1);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD2);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD3);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD4);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD5);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_DEVICE_ADDR);
    LIME_UART_PROTOCOL_TxAddData(cmd);
    LIME_UART_PROTOCOL_TxAddData(0);
    LIME_UART_PROTOCOL_TxAddData(2);

    // д���ֽ���
    LIME_UART_PROTOCOL_TxAddData(d1);
    LIME_UART_PROTOCOL_TxAddData(d2);

    LIME_UART_PROTOCOL_TxAddFrame();
}



// UART����֡����������
void LIME_UART_PROTOCOL_CmdFrameProcess(LIME_UART_PROTOCOL_CB *pCB)
{
    LIME_UART_PROTOCOL_CMD cmd = LIME_UART_PROTOCOL_CMD_NULL;
    LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
    uint8 eraResual = 0;
    uint8 i;
    uint8 length1;

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
    if (LIME_UART_PROTOCOL_CMD_HEAD != pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_HEAD1_INDEX])
    {
        // ɾ������֡
        pCB->rx.head ++;
        pCB->rx.head %= LIME_UART_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (LIME_UART_PROTOCOL_CMD)pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_CMD_INDEX];
    switch (dut_info.ID)
    {
    case DUT_TYPE_NULL:
        break;

    case DUT_TYPE_LIME: // ����ͨ������
        // ִ������֡
        switch (cmd)
        {
        // �յ��Ǳ��͵�ECO����
        case LIME_UART_PROTOCOL_ECO_CMD_REQUEST: // 01
            // ����������빤��ģʽ
            STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST);
            break;

        // �Ǳ��Ӧ׼������
        case LIME_UART_PROTOCOL_ECO_CMD_READY:  // 02
            // ����app����ָ��
            STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR);
            break;

        // dut���������ѯ
        case LIME_UART_PROTOCOL_ECO_CMD_APP_ERASE:  //04
            // ��ȡ�Ǳ�������
            eraResual = pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_DATA2_INDEX];

            // ����ʧ��
            if (0 == eraResual)
            {
                // ����ʧ���ٲ���һ��
                STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR);
                break;
            }
            // �����ɹ�
            // ���͵�һ������
            STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET);

            break;
        case LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE: // 05,app����д��
           // eraResual = pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_DATA2_INDEX];
					 eraResual = pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_DATA3_INDEX];
            if (0 == eraResual) // ����Ǳ�д��ʧ��
            {
                break;
            }
            else
            {
                if (dut_info.currentAppSize < dut_info.appSize)
                {
                    STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_SEND_APP_PACKET); // ����app���ݰ�
                }
                else
                {
                    STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_APP_UP_SUCCESS);
                }
            }
            break;

        case LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE_FINISH: // 06��app�������
            STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_ITEM_FINISH);
            break;

            break;

        default:
            break;
        }
        break;
    default:
        break;
    }

    // ��ͨѶ���ص�LOGO����
//  TIMER_AddTask(TIMER_ID_STATE_CONTROL,
//                  timeOut,
//                  LIME_UART_PROTOCOL_CALLBACK_UartBusError,
//                  TRUE,
//                  1,
//                  ACTION_MODE_ADD_TO_QUEUE);

    // ɾ������֡
    pCB->rx.head ++;
    pCB->rx.head %= LIME_UART_PROTOCOL_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void LIME_UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
    LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    pCmdFrame = &uartProtocolCB4.rx.cmdQueue[uartProtocolCB4.rx.end];

    // ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
    pCmdFrame->length = 0;  // 2016.1.6����
    // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
    uartProtocolCB4.rxFIFO.head ++;
    uartProtocolCB4.rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
    uartProtocolCB4.rxFIFO.currentProcessIndex = uartProtocolCB4.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void LIME_UART_PROTOCOL_StopRxTimeOutCheck(void)
{
    TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

// ������������·����������־
void LIME_UART_PROTOCOL_SetTxAtOnceRequest(uint32 param)
{
    uartProtocolCB4.txAtOnceRequest = (BOOL)param;
}

// ������������·����������־
void LIME_UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param)
{
    uartProtocolCB4.txPeriodRequest = (BOOL)param;
}

// ���������ּ��ظ�һ���ֽ�
void LIME_UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD1);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD2);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD3);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD4);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD5);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_DEVICE_ADDR);
    LIME_UART_PROTOCOL_TxAddData((uint8)ackCmd);
    LIME_UART_PROTOCOL_TxAddData(0);
    LIME_UART_PROTOCOL_TxAddData(2);
    LIME_UART_PROTOCOL_TxAddData(1);
    LIME_UART_PROTOCOL_TxAddData(ackParam);

    LIME_UART_PROTOCOL_TxAddFrame();
}

// ����������
void LIME_UART_PROTOCOL_SendCmd(uint32 sndCmd)
{
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD1);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD2);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD3);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD4);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD5);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_DEVICE_ADDR);
    LIME_UART_PROTOCOL_TxAddData((uint8)sndCmd);
    LIME_UART_PROTOCOL_TxAddData(0);
    LIME_UART_PROTOCOL_TxAddData(0);
    LIME_UART_PROTOCOL_TxAddFrame();
}

void LIME_Protocol_SendOnePacket(uint32 flashAddr, uint32 addr)
{
    uint8 addr1, addr2, addr3, addr4;
    uint8 appUpdateOnePacket[150] = {0};
    uint8 appUpdateOnePacket_i = 0;
    addr1 = (addr & 0xFF000000) >> 24;
    addr2 = (addr & 0x00FF0000) >> 16;
    addr3 = (addr & 0x0000FF00) >> 8;
    addr4 = (addr & 0x000000FF);
    SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + addr, 128); // ���߶�ȡ128�ֽ�

    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD1);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD2);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD3);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD4);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD5);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_DEVICE_ADDR);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE);

    LIME_UART_PROTOCOL_TxAddData(0x00);                                                    // ������ʱΪ0
		LIME_UART_PROTOCOL_TxAddData(0x87);                                                    // ������ʱΪ0

 		LIME_UART_PROTOCOL_TxAddData(0x04);                                                    // 4����ַ����   
    LIME_UART_PROTOCOL_TxAddData(addr1);                                                       // ��ӵ�ַ
    LIME_UART_PROTOCOL_TxAddData(addr2);                                                       // ��ӵ�ַ
    LIME_UART_PROTOCOL_TxAddData(addr3);                                                       // ��ӵ�ַ
    LIME_UART_PROTOCOL_TxAddData(addr4);                                                       // ��ӵ�ַ
		
 		LIME_UART_PROTOCOL_TxAddData(0x00);                                                    // һ���ֽ�����   
  	LIME_UART_PROTOCOL_TxAddData(0x80);                                                    // һ���ֽ�����      		
    for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 128; appUpdateOnePacket_i++) // �������
    {
        LIME_UART_PROTOCOL_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i]);
    }
    LIME_UART_PROTOCOL_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
}
