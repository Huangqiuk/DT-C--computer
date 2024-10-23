#include "common.h"
#include "timer.h"
#include "delay.h"
#include "uartDrive.h"
#include "uartProtocol.h"
#include "param.h"
#include "state.h"
#include "iap.h"
#include "powerCtl.h"
#include "stringOperation.h"
#include "System.h"

/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB *pCB);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB *pCB);

// UART����֡����������
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB *pCB);

// ͨѶ��ʱ����-����
void UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void UART_PROTOCOL_StopRxTimeOutCheck(void);

// Э��㷢�ʹ������
void UART_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// ����ͨѶ��ʱ�ж�����
// void UART_PROTOCOL_StartTimeoutCheckTask(void);

// UART���߳�ʱ������
void UART_PROTOCOL_CALLBACK_UartBusError(uint32 param);

//========================================================================
// ��������ظ�
void UART_PROTOCOL_SendCmdAck(uint8 ackCmd);

// ���ʹ�Ӧ��Ļظ�
void UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// ����App�汾
void UART_PROTOCOL_SendCmdAppVersion(uint32 param);

//==================================================================================
// ȫ�ֱ�������
UART_PROTOCOL_CB uartProtocolCB;
VOUT_CURRENT vout_Current;
POWERSUPPLYVOLTAGE PowerSupplyVoltage;
uint8 uumacBuff[20];

// Э���ʼ��
void UART_PROTOCOL_Init(void)
{
    // Э������ݽṹ��ʼ��
    UART_PROTOCOL_DataStructInit(&uartProtocolCB);

    // ��������ע�����ݽ��սӿ�
    UART_DRIVE_RegisterDataSendService(UART_PROTOCOL_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    UART_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);
}


// UARTЭ�����̴���
void UART_PROTOCOL_Process(void)
{
    // UART����FIFO����������
    UART_PROTOCOL_RxFIFOProcess(&uartProtocolCB);

    // UART���������������
    UART_PROTOCOL_CmdFrameProcess(&uartProtocolCB);

    // UARTЭ��㷢�ʹ������
    UART_PROTOCOL_TxStateProcess();

}

// ��������֡�������������
void UART_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB.tx.head;
    uint16 end =  uartProtocolCB.tx.end;
    UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];

    // ���ͻ������������������
    if ((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void UART_PROTOCOL_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = uartProtocolCB.tx.head;
    uint16 end  = uartProtocolCB.tx.end;
    UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];
    uint16 length = pCmdFrame->length;

    // ���ͻ������������������
    if ((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ����֡���Ȳ��㣬������������ݣ��˳�
    if (UART_PROTOCOL_CMD_FRAME_LENGTH_MIN - 4 > length) // ��ȥ"У���L��У���H��������ʶ0xD��������ʶOxA"4���ֽ�
    {
        pCmdFrame->length = 0;

        return;
    }

    // ��β����֡�������˳�
    if ((length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
            || (length + 2 >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
    {
        pCmdFrame->length = 0;

        return;
    }

    // �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
    pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] = length - 4;   // �������ݳ��ȣ���ȥ"����ͷ���豸��ַ�������֡����ݳ���"4���ֽ�
    for (i = 1; i < length; i++)
    {
        checkSum += pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);         // ���ֽ���ǰ
    pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF);  // ���ֽ��ں�

    // ������ʶ
    pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
    pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

    uartProtocolCB.tx.end ++;
    uartProtocolCB.tx.end %= UART_PROTOCOL_TX_QUEUE_SIZE;
    //pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB *pCB)
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
    for (i = 0; i < UART_PROTOCOL_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end  = 0;
    for (i = 0; i < UART_PROTOCOL_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB.rxFIFO.end;
    uint16 head = uartProtocolCB.rxFIFO.head;
    uint8 rxdata = 0x00;

    // ��������
    rxdata = *pData;

    // һ���������������������
    if ((end + 1) % UART_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // һ��������δ��������
    else
    {
        // �����յ������ݷŵ���ʱ��������
        uartProtocolCB.rxFIFO.buff[end] = rxdata;
        uartProtocolCB.rxFIFO.end ++;
        uartProtocolCB.rxFIFO.end %= UART_PROTOCOL_RX_FIFO_SIZE;
    }
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB *pCB)
{
    UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= UART_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0;   // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void UART_PROTOCOL_TxStateProcess(void)
{
    uint16 head = uartProtocolCB.tx.head;
    uint16 end =  uartProtocolCB.tx.end;
    uint16 length = uartProtocolCB.tx.cmdQueue[head].length;
    uint8 *pCmd = uartProtocolCB.tx.cmdQueue[head].buff;
    uint16 localDeviceID = uartProtocolCB.tx.cmdQueue[head].deviceID;

    // ���ͻ�����Ϊ�գ�˵��������
    if (head == end)
    {
        return;
    }

    // ���ͺ���û��ע��ֱ�ӷ���
    if (NULL == uartProtocolCB.sendDataThrowService)
    {
        return;
    }

    // Э�����������Ҫ���͵�������
    if (!(*uartProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // ���ͻ��ζ��и���λ��
    uartProtocolCB.tx.cmdQueue[head].length = 0;
    uartProtocolCB.tx.head ++;
    uartProtocolCB.tx.head %= UART_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (UART_PROTOCOL_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head ++;
            pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % UART_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex ++;
        pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0;  // 2016.1.5����
            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head ++;
            pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;

            // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������

            // �����ж�����֡��С���ȣ�һ�����������������ٰ���8���ֽ�: ����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����˲���8���ֽڵıض�������
            if (pCmdFrame->length < UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            if (pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] > UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head ++;
                pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����Ϊ����֡ʵ�ʳ���
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] + UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!UART_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head ++;
                pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
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
            UART_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
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
    sumTemp = pCmdFrame->buff[pCmdFrame->length - 3];   // ���ֽ�
    sumTemp <<= 8;
    sumTemp += pCmdFrame->buff[pCmdFrame->length - 4];  // ���ֽ�

    // �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
    if (sumTemp != checkSum)
    {
        return FALSE;
    }

    return TRUE;
}

// UART����֡����������
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB *pCB)
{
    UART_PROTOCOL_CMD cmd = UART_ECO_CMD_NULL;
    UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    BOOL    writeRes;
    uint8   i;
    uint32  dataByteNum;
    uint32  u32TempVal;
    //uint8   tempBuf[50];

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
    if (UART_PROTOCOL_CMD_HEAD != pCmdFrame->buff[UART_PROTOCOL_CMD_HEAD_INDEX])
    {
        // ɾ������֡
        pCB->rx.head ++;
        pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (UART_PROTOCOL_CMD)pCmdFrame->buff[UART_PROTOCOL_CMD_CMD_INDEX];

    // ִ������֡
    switch (cmd)
    {
//        // ���������ִ��
//        case UART_ECO_CMD_NULL:
//            break;

//        // ��������
//        case UART_ECO_CMD_RST:
//            TIMER_AddTask(TIMER_ID_JUMP,
//                          100,
//                          SYSTEM_Rst,
//                          0,
//                          1,
//                          ACTION_MODE_ADD_TO_QUEUE);

//            // ����Ӧ��
//            UART_PROTOCOL_SendCmdAck(UART_ECO_CMD_RST);
//            break;

//        // ECO����
//        case UART_ECO_CMD_ECO_APPLY:

//            // Ϊ 1 ʱ��׼ͣ��ECO������ECOģʽ
//            // �رն�ʱ��������ECOģʽ
//            TIMER_KillTask(TIMER_ID_ECO_SEND);
//            TIMER_KillTask(TIMER_ID_JUMP);

//            // �ظ�ECO����
//            UART_PROTOCOL_SendCmdAck(UART_ECO_CMD_ECO_READY);
//            break;

//        // APP����汾���
//        case UART_ECO_CMD_ECO_JUMP_APP:

//            // 100ms ��ʱ����APP (��֤�ظ��������תAPP)
//            TIMER_AddTask(TIMER_ID_JUMP,
//                          100,
//                          IAP_JumpToAppFun,
//                          0,
//                          1,
//                          ACTION_MODE_ADD_TO_QUEUE);

//            // �ظ�����
//            UART_PROTOCOL_SendCmdParamAck(UART_ECO_CMD_ECO_JUMP_APP, 0x03);
//            break;

//        // APP���ݲ���
//        case UART_ECO_CMD_ECO_APP_ERASE:
//            writeRes = IAP_EraseAPPArea();
//            UART_PROTOCOL_SendCmdParamAck(UART_ECO_CMD_ECO_APP_ERASE, writeRes);
//            break;

//        // APP����д��
//        case UART_ECO_CMD_ECO_APP_WRITE:

//            // ������ݳ��Ȳ��Ϸ�
//            if (4 > pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX])
//            {
//                break;
//            }
//            // �ڶ���Ϊ�̼��ֽ���
//            dataByteNum = pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] - 4;

//            // ����̼���ַ
//            for (u32TempVal = 0, i = 0; i < 4; i++)
//            {
//                u32TempVal <<= 8;
//                u32TempVal |= pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX + i];
//            }

//            // д���������ݵ�flash����
//            writeRes = IAP_WriteAppBin(IAP_FLASH_APP_ADDR + u32TempVal, &pCmdFrame->buff[UART_PROTOCOL_CMD_DATA5_INDEX], dataByteNum);

//            // ��������Ӧ��
//            UART_PROTOCOL_SendCmdParamAck(UART_ECO_CMD_ECO_APP_WRITE, writeRes);
//            break;

//        // APP�������
//        case UART_ECO_CMD_ECO_APP_WRITE_FINISH:
//            UART_PROTOCOL_SendCmdAck(UART_ECO_CMD_ECO_APP_WRITE_FINISH);
//            break;

        default:
            break;
    }

    // ����UART����ͨѶ��ʱ�ж�
    //UART_PROTOCOL_StartTimeoutCheckTask();

    // ɾ������֡
    pCB->rx.head ++;
    pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
    UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    pCmdFrame = &uartProtocolCB.rx.cmdQueue[uartProtocolCB.rx.end];

    // ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
    pCmdFrame->length = 0;  // 2016.1.6����
    // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
    uartProtocolCB.rxFIFO.head ++;
    uartProtocolCB.rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
    uartProtocolCB.rxFIFO.currentProcessIndex = uartProtocolCB.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void UART_PROTOCOL_StopRxTimeOutCheck(void)
{
    TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif


////    UART���߳�ʱ������
//void UART_PROTOCOL_CALLBACK_UartBusError(uint32 param)
//{
//  PARAM_SetErrorCode(ERROR_TYPE_COMMUNICATION_TIME_OUT);
//  PARAM_SetBatteryDataSrc(BATTERY_DATA_SRC_ADC);
//
//  uartProtocolCB.speedFilter.realSpeed = 0;
//  uartProtocolCB.speedFilter.proSpeed = 0;
//  uartProtocolCB.speedFilter.difSpeed = 0;
//}


// �ϱ����
void UART_PROTOCOL_ReportWriteParamResult(uint32 cmd, uint32 param)
{
    // �������ͷ
    UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

    // ����豸��ַ
    UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

    // ���������
    UART_PROTOCOL_TxAddData(cmd);

    // ������ݳ���
    UART_PROTOCOL_TxAddData(1);

    // д����
    UART_PROTOCOL_TxAddData(param);

    // ��Ӽ������������������������
    UART_PROTOCOL_TxAddFrame();
}

// �ϱ����
void UART_PROTOCOL_Report_POWER_SUPPLY(uint32 cmd, uint32 param)
{
    // �������ͷ
    UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

    // ����豸��ַ
    UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

    // ���������
    UART_PROTOCOL_TxAddData(cmd);

    // ������ݳ���
    UART_PROTOCOL_TxAddData(4);

    // д����
    UART_PROTOCOL_TxAddData(param);

    // ��Ӽ������������������������
    UART_PROTOCOL_TxAddFrame();
}

// ��������ظ�
void UART_PROTOCOL_SendCmdAck(uint8 ackCmd)
{
    UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);
    UART_PROTOCOL_TxAddData(ackCmd);
    UART_PROTOCOL_TxAddData(0x00);
    UART_PROTOCOL_TxAddFrame();
}

// ���ʹ�Ӧ��Ļظ�
void UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
    UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);
    UART_PROTOCOL_TxAddData(ackCmd);
    UART_PROTOCOL_TxAddData(0x00);

    UART_PROTOCOL_TxAddData(ackParam);
    UART_PROTOCOL_TxAddFrame();
}

// ����ECO����
void UART_PROTOCOL_SendEco(uint32 param)
{
    uint8 len = BOOT_VERSION_LENGTH;
    uint8 pBuf[] = BOOT_VERSION;
    uint8 i;

    UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);
    UART_PROTOCOL_TxAddData(UART_ECO_CMD_ECO_APPLY);
    UART_PROTOCOL_TxAddData(0x00);

    // BOOT����
    UART_PROTOCOL_TxAddData(len);
    // BOOT�汾
    if (len < 32)
    {
        // BOOT�汾
        for (i = 0; i < len; i++)
        {
            UART_PROTOCOL_TxAddData(pBuf[i]);
        }
        // �������
        for (i = 0; i < (32 - len); i++)
        {
            UART_PROTOCOL_TxAddData(0xFF);
        }
    }
    else
    {
        // BOOT�汾
        for (i = 0; i < 32; i++)
        {
            UART_PROTOCOL_TxAddData(pBuf[i]);
        }
    }

    // CPU �ͺ��ֽ���
    UART_PROTOCOL_TxAddData(0x3);

    UART_PROTOCOL_TxAddFrame();
}
