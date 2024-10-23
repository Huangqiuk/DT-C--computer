/*********************************************************
 * ͷ�ļ���
 *********************************************************/
// canͨ��ͷ�ļ�
#include "CanProtocolTest.h"

// �豸ͷ�ļ����������������ɾ����
#include "system.h"
#include "timer.h"
#include "spiflash.h"
#include "iap.h"
#include "gd32f10x_can.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "canProtocol_3A.h"
#include "state.h"
#include "StsProtocol.h"
#include "PowerProtocol.h"
#include "stringOperation.h"
#include "DutUartProtocol.h"
#include "CanProtocolUpDT.h"

/*********************************************************
 * ����������
 *********************************************************/
CAN_TEST_CB testCanCB;            // CANͨ�����ݽṹ
CAN_TEST_TX_STATE_CB canTestTxStateCB; // ����״̬�����ƶ���

/*********************************************************
 * �ڲ�����������
 *********************************************************/
/************ CAN�ײ㺯�� *************************/

// ���ݽṹ��ʼ��
void CAN_TEST_DataStructureInit(CAN_TEST_CB *pCB);

// CAN ���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_TEST_MacProcess(uint16 standarID, uint8 *data, uint8 length);

// һ�����Ľ��ջ���������������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_TEST_RXFIFOProcess(CAN_TEST_CB *pCB);

// CAN����֡����������
void CAN_TEST_CmdFrameProcess(CAN_TEST_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL CAN_TEST_CheckSum(uint8 *buff, uint32 len);

// �Դ��������֡����У�飬����У����
BOOL CAN_TEST_AddSum(uint8 *buff, uint32 len);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_TEST_ConfirmTempCmdFrameBuff(CAN_TEST_CB *pCB, uint8 sourceSelect);

/************ CAN״̬������ **************************/
// ����뷢�ʹ���
BOOL CAN_TEST_Tx_FillAndSend(CAN_TEST_CB *pCB);

// ���ͳɹ�������һ֡
BOOL CAN_TEST_Tx_NextFram(CAN_TEST_CB *pCB);

// CAN����״̬����ʼ��
void CAN_TEST_TxStateInit(void);

// CAN����״̬Ǩ��
void CAN_TEST_TxEnterState(uint32 state);

// CAN����״̬������
void CAN_TEST_TxStateProcess(void);

/************ CAN�ײ㺯�� **************************/

// ����ģʽ׼����������
void CAN_PROTOCOL_TEST_SendCmdProjectReady(uint8 param);

// ����FLASH����ϱ�
void CAN_PROTOCOL_TEST_SendCmdEraseFlashResult(uint8 param);

// IAP����д�����ϱ�
void CAN_PROTOCOL_TEST_SendCmdWriteFlashResult(uint8 param);

// ��ս���ϱ�
void CAN_PROTOCOL_TEST_SendCmdCheckFlashBlankResult(uint8 param);

// ϵͳ��������ȷ��
void CAN_PROTOCOL_TEST_SendCmdUpdataFinishResult(uint8 param);

// ����UI���ݲ������
void CAN_PROTOCOL_TEST_SendCmdUIEraseResultReport(uint8 param);

// ����UI����д����
void CAN_PROTOCOL_TEST_SendCmdUIWriteDataResultReport(uint8 param);

/*********************************************************
 * ȫ�ֱ�������
 *********************************************************/
extern uint8 cali_cnt;
//CAN_PROTOCOL1_CB canProtocol1CB;
uint8 can_light_cnt = 0;
uint8 can_left_light_cnt = 0;
uint8 can_right_light_cnt = 0;
uint8 can_cnt = 0;
uint8 can_cali_cnt = 0;
uint8 can_brake_cnt = 0;
uint8 can_throttleAd_cnt = 0;
uint8 can_derailleurAd_cnt = 0;

// ͸��
char ptUpMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "P-T Completion",
    "",
    "",
    "",
};

/*********************************************************
 * ����������
 *********************************************************/
// CAN ��ʼ��
void CAN_PROTOCOL_TEST_Init(uint16 bund)
{
    // ��ʼ��Ӳ������
    CAN_TEST_HW_Init(bund);

    // ��ʼ���ṹ��
    CAN_TEST_DataStructureInit(&testCanCB);

    // CAN����״̬����ʼ��
    CAN_TEST_TxStateInit();

    //  TIMER_AddTask(TIMER_ID_CAN_TEST,//����
    //                          500,
    //                          CAN_TEST,
    //                          TRUE,
    //                          -1,
    //                          ACTION_MODE_ADD_TO_QUEUE);
}

// ���ݽṹ��ʼ��
void CAN_TEST_DataStructureInit(CAN_TEST_CB *pCB)
{
    uint8 i;
    uint8 j;

    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return;
    }

    pCB->tx.txBusy = FALSE;
    pCB->tx.index = 0;
    pCB->tx.head = 0;
    pCB->tx.end = 0;
    for (i = 0; i < CAN_TEST_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    for (i = 0; i < CAN_TEST_DEVICE_SOURCE_ID_MAX; i++)
    {
        pCB->rxFIFO.rxFIFOEachNode[i].head = 0;
        pCB->rxFIFO.rxFIFOEachNode[i].end = 0;
        pCB->rxFIFO.rxFIFOEachNode[i].currentProcessIndex = 0;
    }

    for (i = 0; i < CAN_TEST_DEVICE_SOURCE_ID_MAX; i++)
    {
        for (j = 0; j < CAN_TEST_RX_QUEUE_SIZE; j++)
        {
            pCB->rx.rxEachNode[i].cmdQueue[j].length = 0;
        }
        pCB->rx.rxEachNode[i].head = 0;
        pCB->rx.rxEachNode[i].end = 0;
    }
}

// ��������֡�������������
void CAN_PROTOCOL_TEST_TxAddData(uint8 data)
{
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;
    CAN_PROTOCOL_TEST_FRAME *pCmdFrame = &testCanCB.tx.cmdQueue[testCanCB.tx.end];

    // ���ͻ������������������
    if ((end + 1) % CAN_TEST_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ��������֡����,�Զ�����֡�����Լ�������
void CAN_PROTOCOL_TEST_TxAddFrame(void)
{
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;
    uint8 *buff = testCanCB.tx.cmdQueue[end].buff;
    uint16 length = testCanCB.tx.cmdQueue[end].length;
    uint8 cmdHead = buff[CAN_PROTOCOL_TEST_HEAD_INDEX]; // ֡ͷ

    // ���ͻ������������������
    if ((end + 1) % CAN_TEST_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ����ͷ����
    if (CAN_PROTOCOL_TEST_HEAD != cmdHead)
    {
        // ��֡�������������ʼλ��
        testCanCB.tx.cmdQueue[end].length = 0;
        return;
    }

    // ����ͷ��ȷ�������ݳ��Ȳ���(���ݳ��ȼ�ȥCRC�ĳ���)
    if ((CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX - CAN_CHECK_BYTE_SIZE < length) || (CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN - CAN_CHECK_BYTE_SIZE > length))
    {
        // ��֡�������������ʼλ��
        testCanCB.tx.cmdQueue[end].length = 0;
        buff[CAN_PROTOCOL_TEST_HEAD_INDEX] = CAN_PROTOCOL_TEST_NONHEAD; // ��������û����ʱ���ٴν���
        return;
    }

    // ִ�е��ˣ�����ͷ��ȷ�����ݳ�����ȷ,���ͻ�����δ��
    // ���ȷ���
    buff[CAN_PROTOCOL_TEST_LENGTH_INDEX] = length - CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN + CAN_CHECK_BYTE_SIZE;

    // ���CRCУ����
    CAN_TEST_AddSum(buff, length);

    // �����ݳ���������ͻ�����
    testCanCB.tx.cmdQueue[end].length = length + CAN_CHECK_BYTE_SIZE;

    // ���ͻ��ζ��и���λ��
    testCanCB.tx.end++;
    testCanCB.tx.end %= CAN_TEST_TX_QUEUE_SIZE;
    testCanCB.tx.cmdQueue[testCanCB.tx.end].length = 0; // 2015.12.2�޸�
}

// ����뷢�ʹ���:TRUE,����뷢�ͣ�FALSE�����ζ�����ǰ����.
BOOL CAN_TEST_Tx_FillAndSend(CAN_TEST_CB *pCB)
{
    uint16 index = pCB->tx.index;                          // ��ǰ�������ݵ�������
    uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length; // ��ǰ���͵�����֡�ĳ���
    uint16 head = pCB->tx.head;                            // ��������֡����ͷ������
    uint16 end = pCB->tx.end;                              // ��������֡����β������
    CAN_PROTOCOL_TEST_FRAME *pCmd = &pCB->tx.cmdQueue[pCB->tx.head]; // ȡ��ǰ���ͻ�������ַ
    can_trasnmit_message_struct canTxMsgCB;
    uint16 standardID = 0x000;
    uint8 i; // �ڲ���ֵ��
    uint32 extendedID;

    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return FALSE;
    }

    // ����Ϊ�գ�������
    if (head == end)
    {
        return FALSE;
    }

    // ����ִ�е����˵�����зǿա���

    // ��ǰ����֡δ������ʱ�����ݳ��ȵı仯ȡ������֡�е����ݷŵ����ͼĴ�����
    if (index < length)
    {
        // ����Ϊ��׼֡
        // ����Ϊ����֡
        canTxMsgCB.tx_ft = CAN_FT_DATA;

        if(testFlag)
        {
            // ��չ֡
            canTxMsgCB.tx_ff = CAN_FF_EXTENDED;
//            extendedID = pCmd->deviceID;
            extendedID = 0x1801FFF4;
            canTxMsgCB.tx_efid = extendedID;
        }
        else
        {
            switch (dut_info.ID)
            {
                case DUT_TYPE_GB:
                case DUT_TYPE_KAIYANG:

                    // ��չ֡
                    canTxMsgCB.tx_ff = CAN_FF_EXTENDED;
                    extendedID = pCmd->deviceID;
                    canTxMsgCB.tx_efid = extendedID;
                    break;

                default:
                    // ��׼֡
                    canTxMsgCB.tx_ff = CAN_FF_STANDARD;
                    standardID = (uint16)CAN_TEST_DEVICE_ID_HMI; // id��001
                    canTxMsgCB.tx_sfid = standardID;
                    break;
            }
         }
        // ������׼����֡CAN��Ϣ��ʽ����

        // ��� DLC�����8�ֽ�.��ǰ����>8�ֽڣ��ȷ�8�ֽڡ�
        if ((length - index) >= CAN_ONCEMESSAGE_MAX_SIZE)
        {
            canTxMsgCB.tx_dlen = CAN_ONCEMESSAGE_MAX_SIZE;
        }
        else
        {
            canTxMsgCB.tx_dlen = length - index;
        }

        // �������
        for (i = 0; i < canTxMsgCB.tx_dlen; i++)
        {
            canTxMsgCB.tx_data[i] = pCmd->buff[index + i];
        }

        // ��������
        if (can_message_transmit(CAN0, &canTxMsgCB) == CAN_TRANSMIT_NOMAILBOX)
        {
            return FALSE;
        }

        // ����������ж�
        can_interrupt_enable(CAN0, CAN_INT_TME);

        return TRUE;
    }
    // ��ǰ����֡������ʱ��ɾ��֮
    else
    {
        pCB->tx.cmdQueue[head].length = 0;
        pCB->tx.head++;
        pCB->tx.head %= CAN_TEST_TX_QUEUE_SIZE;
        pCB->tx.index = 0;

        // ����CAN��Ϣ���ʹ���״̬
        CAN_TEST_TxEnterState(CAN_TEST_TX_STATE_STANDBY);

        return FALSE;
    }
}

// ���ͳɹ�������һ֡:TRUE,������һ֡��FALSE���������.
BOOL CAN_TEST_Tx_NextFram(CAN_TEST_CB *pCB)
{
    uint16 index = pCB->tx.index;                          // ��ǰ�������ݵ�������
    uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length; // ��ǰ���͵�����֡�ĳ���
    uint8 txBnDLC = 0;

    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return FALSE;
    }

    // �ж���һ�γɹ����͵��ֽ���.
    if (length - index >= CAN_ONCEMESSAGE_MAX_SIZE)
    {
        txBnDLC = CAN_ONCEMESSAGE_MAX_SIZE;
    }
    else
    {
        txBnDLC = length - index;
    }
    pCB->tx.index += txBnDLC;

    // ������һ֡
    if (pCB->tx.index < length)
    {
        // ����CAN��Ϣ��һ֡����״̬
        return TRUE;
    }
    else
    {
        // �������
        return FALSE;
    }
}

// CAN ���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_TEST_MacProcess(uint16 standarID, uint8 *data, uint8 length)
{
    uint16 end;
    uint16 head;
    uint8 i;
    int8 sourceBuffSelect = 0; // �ڵ㻺�������ѡ��

    // �����Ϸ����ж�
    if (NULL == data)
    {
        return;
    }

    // ��׼֡�ж�
//    if (standarID > 0x7FF)
//    {
//        return;
//    }

    // ȡ��Ŀ��ڵ�
    //  destinationID = standarID & 0x07FF;

    // ���Ƿ�������ID������ֱ�Ӷ���
    //  if (CAN_TEST_DEVICE_ID_HMI != destinationID)
    //  {
    //      return;
    //  }

    // �Ѿ�ʹ��
//    if (CAN_TEST_RX_DEVICE_PUC)
//    {
//        sourceBuffSelect = CAN_TEST_RX_DEVICE_PUC - 1;
//    }

//    // û�к��ʵ�Դ�ڵ�
//    if ((sourceBuffSelect >= CAN_TEST_DEVICE_SOURCE_ID_MAX) || (sourceBuffSelect < 0))
//    {
//        return;
//    }

    // ȡ��Ӧ�������ṹ������
    end = testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end;
    head = testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].head;

    // ���������жӣ���ӡ���
    //  һ���������������������
//    if ((end + 1) % CAN_TEST_RX_FIFO_SIZE == head)
//    {
//        return;
//    }

    for (i = 0; i < length; i++)
    {
        // �����ֽڶ�ȡ��������FIFO��
        testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].buff[testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end] = *data++;

        testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end++;

        // һ���������������������
        if ((testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end + 1) % CAN_TEST_RX_FIFO_SIZE == head)
        {
            break;
        }

        testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end %= CAN_TEST_RX_FIFO_SIZE;
    }
}

// һ�����Ľ��ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_TEST_RxFIFOProcess(CAN_TEST_CB *pCB)
{
    uint16 end;
    uint16 head;
    CAN_PROTOCOL_TEST_FRAME *pCmdFrame = NULL;
    CAN_TEST_RX_FIFO *pRxFIFO = NULL;
    uint16 length = 0;
    uint8 currentData = 0;
    uint8 sourceBuffSelect = 0;

    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return;
    }

    // ѭ���ж��Ƿ���������Ҫ����
//    for (sourceBuffSelect = 0; sourceBuffSelect < CAN_TEST_DEVICE_SOURCE_ID_MAX; sourceBuffSelect++)
//    {
        // ��ȡ����һ���������ڵ���ʱ������ָ��
        pRxFIFO = &pCB->rxFIFO.rxFIFOEachNode[sourceBuffSelect];

        end = pRxFIFO->end;
        head = pRxFIFO->head;

        // һ��������Ϊ�գ�����
        if (head == end)
        {
            return;
        }

        // ��ȡ����������ʱ������ָ��
        pCmdFrame = &pCB->rx.rxEachNode[sourceBuffSelect].cmdQueue[pCB->rx.rxEachNode[sourceBuffSelect].end];

        // ȡ����ǰҪ������ֽ�
        currentData = pRxFIFO->buff[pRxFIFO->currentProcessIndex];

        // ��ʱ����������Ϊ0ʱ���������ֽ�
        if (0 == pCmdFrame->length)
        {
            // ����ͷ����ɾ����ǰ�ֽڲ��˳�
            if (CAN_PROTOCOL_TEST_HEAD != currentData)
            {
                pRxFIFO->head++;
                pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                return;
            }

            // ����ͷ��ȷ������������ʱ���������ã��˳�
            if ((pCB->rx.rxEachNode[sourceBuffSelect].end + 1) % CAN_TEST_RX_QUEUE_SIZE == pCB->rx.rxEachNode[sourceBuffSelect].head)
            {
                return;
            }

            // ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
            pCmdFrame->buff[pCmdFrame->length++] = currentData;
            pRxFIFO->currentProcessIndex++;
            pRxFIFO->currentProcessIndex %= CAN_TEST_RX_FIFO_SIZE;
        }
        // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
        else
        {
            // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
            if (pCmdFrame->length >= CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX)
            {
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pRxFIFO->head++;
                pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                return;
            }

            // һֱȡ��ĩβ
            while (end != pRxFIFO->currentProcessIndex)
            {
                // ȡ����ǰҪ������ֽ�
                currentData = pRxFIFO->buff[pRxFIFO->currentProcessIndex];

                // ������δ������������գ���������ӵ���ʱ��������
                pCmdFrame->buff[pCmdFrame->length++] = currentData;
                pRxFIFO->currentProcessIndex++;
                pRxFIFO->currentProcessIndex %= CAN_TEST_RX_FIFO_SIZE;

                // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������

                // �����ж�����֡��С���ȣ�һ�����������������ٰ���4���ֽ�: ����ͷ(1Byte)+������(1Byte)+���ݳ���(1Byte)+У����(1Byte) ����˲���4���ֽڵıض�������
                if (pCmdFrame->length < CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN)
                {
                    // ��������
                    continue;
                }

                // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
                if (pCmdFrame->buff[CAN_PROTOCOL_TEST_LENGTH_INDEX] > (CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX - CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN))
                {
                    // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                    pCmdFrame->length = 0;

                    // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                    pRxFIFO->head++;
                    pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                    pRxFIFO->currentProcessIndex = pRxFIFO->head;

                    break;
                }

                // ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ�������֡�...�����ݳ��ȡ�У���룬��Ϊ����֡ʵ�ʳ���
                length = pCmdFrame->length;
                if (length < (pCmdFrame->buff[CAN_PROTOCOL_TEST_LENGTH_INDEX] + CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN))
                {
                    // ����Ҫ��һ�£�˵��δ������ϣ�����
                    continue;
                }

                // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
                if (!CAN_TEST_CheckSum(pCmdFrame->buff, pCmdFrame->length))
                {
                    // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                    pCmdFrame->length = 0;

                    // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                    pRxFIFO->head++;
                    pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                    pRxFIFO->currentProcessIndex = pRxFIFO->head;

                    break;
                }

                // ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
                pRxFIFO->head += length;
                pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                CAN_TEST_ConfirmTempCmdFrameBuff(pCB, sourceBuffSelect);
                break;
            }
        }
//    }
    return;
}


// CAN����֡����������
void CAN_TEST_CmdFrameProcess(CAN_TEST_CB *pCB)
{
    CAN_PROTOCOL_TEST cmd = CAN_PROTOCOL_TEST_CMD_NULL;
    CAN_PROTOCOL_TEST_FRAME *pCmdFrame = NULL;
    uint8 sourceBuffSelect = CAN_TEST_DEVICE_SOURCE_ID_MAX;
    volatile uint32 temp = 0;
    volatile uint16 length;
    volatile uint8 errStatus;
    volatile uint16 i;
    uint8 buffFlag[4] = {0xFF};
    uint8_t verType = 0;
    uint8_t infoLen = 0;
    char  verBuff[100] = {0};
    uint8 calibration_result = 0;
    uint8 switch_state = 0;
    uint8 Peripheral_type = 0;
    uint8 turn_item = 0;
    uint8 turn_state = 0;
    BOOL testResult = 0;


    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return;
    }

    // ѭ���ж��Ƿ���������Ҫ����
    for (sourceBuffSelect = 0; sourceBuffSelect < CAN_TEST_DEVICE_SOURCE_ID_MAX; sourceBuffSelect++)
    {
        // ����֡������Ϊ�գ��˳�
        if (pCB->rx.rxEachNode[sourceBuffSelect].head == pCB->rx.rxEachNode[sourceBuffSelect].end)
        {
            continue;
        }

        // ��ȡ��ǰҪ���������ָ֡��
        pCmdFrame = &pCB->rx.rxEachNode[sourceBuffSelect].cmdQueue[pCB->rx.rxEachNode[sourceBuffSelect].head];

        // ����ͷ�Ƿ����˳�
        if (CAN_PROTOCOL_TEST_HEAD != pCmdFrame->buff[CAN_PROTOCOL_TEST_HEAD_INDEX])
        {
            // ɾ������֡
            pCB->rx.rxEachNode[sourceBuffSelect].head++;
            pCB->rx.rxEachNode[sourceBuffSelect].head %= CAN_TEST_RX_QUEUE_SIZE;
            continue;
        }

        // ����ͷ�Ϸ�������ȡ����
        cmd = (CAN_PROTOCOL_TEST)(pCmdFrame->buff[CAN_PROTOCOL_TEST_CMD_INDEX]);

        switch (cmd)
        {
            // ������
            case CAN_PROTOCOL_TEST_CMD_NULL:
                break;

            // ��ƿ���
            case CAN_PROTOCOL_TEST_CMD_HEADLIGHT_CONTROL:
                switch_state = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];   // ִ�н��

                // ִ��ʧ��
                if (0x00 == (switch_state & 0x00))
                {
    #if DEBUG_ENABLE
                    Clear_All_Lines();
                    Display_Centered(1, "Headlights fail to turn on");
    #endif
                    can_light_cnt = 0;
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                }

                // ִ�гɹ�
                if (0x01 == (switch_state & 0x01))
                {
                    can_light_cnt++;
                    if (can_light_cnt == 1)
                    {
                        STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_OPEN);
                    }
                    if (can_light_cnt == 2)
                    {
                        STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_COMPLETE);
                        can_light_cnt = 0;
                    }
                }
                break;

            // ��ȡ����/ɲ����ģ������ֵ
            case CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD:

                // ��ȡADֵ���ұȶ�
                Peripheral_type = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX];   // ��������

                if (0x01 == (Peripheral_type & 0x01))  // ����
                {
                    dut_info.throttleAd = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX] << 8;
                    dut_info.throttleAd = dut_info.throttleAd | pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX];
                    can_throttleAd_cnt++;
                    switch (can_throttleAd_cnt)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_CHECK_DAC1_VALUE);
                            break;
                            
                        case 2:
                            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_CHECK_DAC3_VALUE);
                            break;
                            
                        case 3:
                            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_CHECK_DAC5_VALUE);
                            can_throttleAd_cnt = 0;
                            break;
                            
                        default:
                            break;
                    }
                }

                if (0x02 == (Peripheral_type & 0x02))  // ɲ��
                {
                    dut_info.brakeAd = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX] << 8;
                    dut_info.brakeAd = dut_info.brakeAd | pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX];
                    can_brake_cnt++;
                    
                    switch (can_brake_cnt)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_CHECK_DAC1_VALUE);
                            break;
                            
                        case 2:
                            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_CHECK_DAC3_VALUE);
                            break;
                            
                        case 3:
                            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_CHECK_DAC5_VALUE);
                            can_brake_cnt = 0;
                            break;
                            
                        default:
                            break;
                    }
                }

                // ���ӱ���
                if (0x03 == (Peripheral_type & 0x03))
                {
                    // ��ȡɲ��ADֵ
                    dut_info.derailleurAd = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX] << 8;
                    dut_info.derailleurAd = dut_info.derailleurAd | pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX];
                    can_derailleurAd_cnt++;
                    
                    switch (can_derailleurAd_cnt)
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
                            can_derailleurAd_cnt = 0;
                            break;

                        default:
                            break;
                    }
                }
                break;

            // ת��ƿ���
            case CAN_PROTOCOL_TEST_CMD_TURN_SIGNAL_CONTROL:
                turn_item = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX];  // ת�������
                turn_state = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX]; // ִ�н��

                // ִ��ʧ��
                if (0x00 == (switch_state & 0x00))
                {
    #if DEBUG_ENABLE
                    Clear_All_Lines();
                    Display_Centered(1, "TURN SIGNAL fail to turn on");
    #endif
                    can_left_light_cnt = 0;
                    can_right_light_cnt = 0;
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                }

                // ��ת��ƴ򿪳ɹ�
                if ((turn_item == 0) && (turn_state == 1))
                {
                    can_left_light_cnt++;
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_OPEN);
                    if (can_left_light_cnt == 2)
                    {
                        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_COMPLETE);
                        can_left_light_cnt = 0;
                    }
                }

                // ��ת��ƴ򿪳ɹ�
                if ((turn_item == 1) && (turn_state == 1))
                {
                    can_right_light_cnt++;
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_OPEN);
                    if (can_right_light_cnt == 2)
                    {
                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_COMPLETE);
                        can_right_light_cnt = 0;
                    }
                }
                break;

            // ���߼��
            case CAN_PROTOCOL_TEST_CMD_ONLINE_DETECTION:
                online_detection_cnt++;            
                TIMER_ResetTimer(TIMER_ID_ONLINE_DETECT);
                
                if(1 == online_detection_cnt)
                {
                   // �ϱ��Ǳ�����״̬
                   STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION,TRUE);
                }
                break;

            // ��ȡ��������������ֵ
            case CAN_PROTOCOL_TEST_CMD_GET_PHOTORESISTOR_VALUE:
                break;

            // ���е�ѹУ׼
            case CAN_PROTOCOL_TEST_CMD_VOLTAGE_CALIBRATION:
                cali_cnt++;
                calibration_result = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];

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

            // ��ȡ��־������
            case CAN_PROTOCOL_TEST_CMD_READ_FLAG_DATA:
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
            case CAN_PROTOCOL_TEST_CMD_WRITE_FLAG_DATA:

                // �ر�д�볬ʱ��ʱ��
                TIMER_ResetTimer(TIMER_ID_WRITE_FLAG_TIMEOUT);

                // �յ�д��ɹ��ٶ�ȡ��������У��
                if (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
                {
                    CAN_PROTOCOL_TEST_SendCmdAck(CAN_PROTOCOL_TEST_CMD_READ_FLAG_DATA);
                }
                else
                {
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
                }
                break;

            // д��汾��������
            case CAN_PROTOCOL_TEST_CMD_WRITE_VERSION_TYPE_DATA:

                // �رն�ʱ��
                TIMER_ResetTimer(TIMER_ID_OPERATE_VER_TIMEOUT);

                // ��ȡ����
                verType =  pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX];

                // ������ȷ����ִ�гɹ����ϱ�STS
                if ((dutverType == verType) && (pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX]))
                {
                    // ��ȡ�汾������Ϣ
                    CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_READ_VERSION_TYPE_INFO, dutverType);
                }
                else
                {
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_VERSION_INFO, FALSE);
                }
                break;

            // ��ȡ�汾������Ϣ
            case CAN_PROTOCOL_TEST_CMD_READ_VERSION_TYPE_INFO:
                TIMER_KillTask(TIMER_ID_OPERATE_VER_TIMEOUT);

                // ��ȡ����
                verType =  pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX];

                // ��ȡ��Ϣ����
                infoLen = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX];

                // ��ȡ������Ϣ
                strcpy(verBuff, (const char *)&pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX]);
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
            case CAN_PROTOCOL_TEST_CMD_BLUETOOTH_MAC_ADDRESS_READ:

                // �汾�ȶԳɹ���MAC��ַ������Դ������
                if (pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX])
                {
                    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
                    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
                    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT);
                    POWER_PROTOCOL_TxAddData(12);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA4_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA5_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA6_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA7_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA8_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA9_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA10_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA11_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA12_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA13_INDEX]);
                    POWER_PROTOCOL_TxAddFrame();
                }

                // �汾�ȶ�ʧ���ϱ�STS
                if (0 == pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX])
                {
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                    STS_PROTOCOL_TxAddData(7);
                    STS_PROTOCOL_TxAddData(1);
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA4_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA5_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA6_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA7_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA8_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA9_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA10_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA11_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA12_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA13_INDEX]) & 0x0F)); 
                    
                    for(i = 0; i < 20 ;i++)
                    {
                      STS_PROTOCOL_TxAddData(dut_info.bleVerBuff[i]);
                    }
                
                    STS_PROTOCOL_TxAddFrame();
                }
                break;

            // LCD ��ɫ���ԣ�������=0x03��
            case CAN_PROTOCOL_TEST_CMD_TEST_LCD:
                STS_PROTOCOL_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_TEST_LCD, TRUE);
                break;

            // Flash У�����
            case CAN_PROTOCOL_TEST_CMD_FLASH_CHECK_TEST:
                TIMER_ChangeTime(TIMER_ID_ONLINE_DETECT, 3000);
                STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
                break;
                
             // ��������
            case CAN_PROTOCOL_TEST_CMD_KEY_TEST:
             
                // ����ģ�����
                if(dut_info.buttonSimulationFlag)
                {
                    if(keyValue == pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX])
                    {
                         testResult = TRUE;
                    }
                    else
                    {
                         testResult = FALSE;
                    }
                    STS_PROTOCOL_SendCmdParamTwoAck(STS_PROTOCOL_CMD_SIMULATION_KEY_TEST, testResult, pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX]);                
                }
                // ������������
                else
                {
                    STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);                
                }
                break;               
                
            default:
                testFlag = FALSE;
                STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);

                // LCD��ʾ
                Vertical_Scrolling_Display(ptUpMsgBuff, 4, 0);
                break;
        }
        // ɾ������֡
        pCB->rx.rxEachNode[sourceBuffSelect].head++;
        pCB->rx.rxEachNode[sourceBuffSelect].head %= CAN_TEST_RX_QUEUE_SIZE;
    }
}

// CAN������Թ��̺���
void CAN_PROTOCOL_Process_Test(void)
{
    // CAN����FIFO����������
    CAN_TEST_RxFIFOProcess(&testCanCB);

    // CAN���������������
    CAN_TEST_CmdFrameProcess(&testCanCB);

    // CAN����״̬���������
    CAN_TEST_TxStateProcess();
}

// �Դ��������֡����У�飬����У����
BOOL CAN_TEST_CheckSum(uint8 *buff, uint32 len)
{
    uint8 cc = 0;
    uint16 i = 0;

    if (NULL == buff)
    {
        return FALSE;
    }

    // ������ͷ��ʼ����У����֮ǰ��һ���ֽڣ����ν����������
    for (i = 0; i < (len - CAN_CHECK_BYTE_SIZE); i++)
    {
        cc ^= buff[i];
    }

//  cc = ~cc;

    // �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
    if (buff[len - 1] != cc)
    {
        return FALSE;
    }

    return TRUE;
}

// �Դ��������֡����У�飬����У����
BOOL CAN_TEST_AddSum(uint8 *buff, uint32 len)
{
    uint8 cc = 0;
    uint16 i = 0;

    if (NULL == buff)
    {
        return FALSE;
    }

    // ������ͷ��ʼ����У����֮ǰ��һ���ֽڣ����ν����������
    for (i = 0; i < len; i++)
    {
        cc ^= buff[i];
    }

//  cc = ~cc;

    // ���У����
    buff[len] = cc;

    return TRUE;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_TEST_ConfirmTempCmdFrameBuff(CAN_TEST_CB *pCB, uint8 sourceSelect)
{
    CAN_PROTOCOL_TEST_FRAME *pCmdFrame = NULL;

    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return FALSE;
    }

    // ��ʱ������Ϊ�գ��������
    pCmdFrame = &pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end];

    if (0 == pCmdFrame->length)
    {
        return FALSE;
    }

    // ���
    pCB->rx.rxEachNode[sourceSelect].end++;
    pCB->rx.rxEachNode[sourceSelect].end %= CAN_TEST_RX_QUEUE_SIZE;
    // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
    pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end].length = 0;

    return TRUE;
}

// CANTX״̬����ʼ��
void CAN_TEST_TxStateInit(void)
{
    CAN_TEST_TxEnterState(CAN_TEST_TX_STATE_STANDBY);
}

// CANTX״̬Ǩ�ƺ���
void CAN_TEST_TxEnterState(uint32 state)
{
    // �õ�ǰ��״̬��Ϊ��ʷ
    canTestTxStateCB.preState = canTestTxStateCB.state;

    // �����µ�״̬
    canTestTxStateCB.state = (CAN_TEST_TX_STATE_E)state;

    switch (state)
    {
        // ����״̬����ڴ������
        case CAN_TEST_TX_STATE_NULL:

            break;

        // �������״̬����ڴ������
        case CAN_TEST_TX_STATE_ENTRY:

            break;

        // ��������״̬״̬����ڴ������
        case CAN_TEST_TX_STATE_STANDBY:

            break;

        // ����CAN��Ϣ�����С���
        case CAN_TEST_TX_STATE_SENDING:

            break;

        default:
            break;
    }
}

// ���ͻ����������һ������������
BOOL CAN_TEST_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length)
{
    uint16 i;
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;

    // ��������
    if ((NULL == pArray) || (0 == length))
    {
        return FALSE;
    }

    // ���ͻ������������������
    if ((end + 1) % CAN_TEST_TX_QUEUE_SIZE == head)
    {
        return FALSE;
    }

//    testCanCB.tx.cmdQueue[end].deviceID = id;
    testCanCB.tx.cmdQueue[end].deviceID = 0x1801FFF4;
    
    for (i = 0; i < length; i++)
    {
        testCanCB.tx.cmdQueue[end].buff[i] = *pArray++;
    }
    testCanCB.tx.cmdQueue[end].length = length;

    // ���ͻ��ζ��и���λ��
    testCanCB.tx.end++;
    testCanCB.tx.end %= CAN_TEST_TX_QUEUE_SIZE;
    testCanCB.tx.cmdQueue[testCanCB.tx.end].length = 0;

    return TRUE;
}

// CANTX���̴�����
void CAN_TEST_TxStateProcess(void)
{
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;

    switch (canTestTxStateCB.state)
    {
        // ����״̬����ڴ������
        case CAN_TEST_TX_STATE_NULL:

            break;

        // �������״̬����ڴ������
        case CAN_TEST_TX_STATE_ENTRY:

            break;

        // ��������״̬״̬����ڴ������
        case CAN_TEST_TX_STATE_STANDBY:
        
            // ����Ƿ���Tx����
            if (head != end)
            {
                // ���ô˺����������������can��Ϣ
                if (CAN_TEST_Tx_FillAndSend(&testCanCB))
                {
                    // ����CAN��Ϣ������״̬
                    CAN_TEST_TxEnterState(CAN_TEST_TX_STATE_SENDING);
                }
                else
                {
                    // ����CAN��Ϣ���ʹ���״̬
                    CAN_TEST_TxEnterState(CAN_TEST_TX_STATE_STANDBY);
                }
            }
            else
            {
                // ����ʲôҲ������
            }

            break;

        // ����CAN��Ϣ�����С���
        case CAN_TEST_TX_STATE_SENDING:

            break;

        default:
            break;
    }
}

// ���ܺ�����CAN��ʼ��
void CAN_TEST_HW_Init(uint16 bund)
{
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;

    // ʱ�ӷ�����ʹ��
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOB);
//  rcu_periph_clock_enable(RCU_GPIOA); // ���
    rcu_periph_clock_enable(RCU_AF);

    // ����CAN�Ľ����ж�
    nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 1, 0);

    // ����CAN������ж�
    nvic_irq_enable(USBD_HP_CAN0_TX_IRQn, 2, 0);

    // CAN�ܽ���ӳ�䵽PB8-9
    gpio_pin_remap_config(GPIO_CAN_PARTIAL_REMAP, ENABLE);

    /* configure CAN0 GPIO */
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_INIT_STRUCT, &can_filter);

    /* initialize CAN register */
    can_deinit(CAN0);

    /* initialize CAN parameters */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.no_auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;

    // �����ʼ��㣺36MHz / (1+6+1) / 36 = 125K bit
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_6TQ;// 6
    can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;// 1
    
    if (CAN_BAUD_RATE_125K == bund)
    {
        can_parameter.prescaler = 36;
    }
    else if (CAN_BAUD_RATE_250K == bund)
    {
        can_parameter.prescaler = 18;
    }
    else if (CAN_BAUD_RATE_500K == bund)
    {
        can_parameter.prescaler = 9;
    }
    else
    {
        can_parameter.prescaler = 36;
    }

    /* initialize CAN */
    can_init(CAN0, &can_parameter);

    /* initialize filter */
    can_filter.filter_number = 1;
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;

    can_filter_init(&can_filter);

    /* enable can receive FIFO0 not empty interrupt */
    can_interrupt_enable(CAN0, CAN_INT_RFNE0);
}

// ISR for CAN RX Interrupt
void USBD_LP_CAN0_RX0_IRQHandler(void)
{
  can_receive_message_struct rxMessage1;

  // ����FIFO�����е�����
  while (can_receive_message_length_get(CAN0, CAN_FIFO0))
  {
        if(dut_info.passThroughControl)           
        {
            dut_info.online_dete_cnt++;            
            TIMER_ResetTimer(TIMER_ID_ONLINE_DETECT);
            
//            // ��ֹ�ۼ�Խ��
//            if(2 < dut_info.online_dete_cnt)
//            {
//                dut_info.online_dete_cnt = 2;
//            }
            
            if(1 == dut_info.online_dete_cnt)
            {
               // �ϱ��Ǳ�����״̬
               STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION,TRUE);
            }
        }
        
      can_message_receive(CAN0, CAN_FIFO0, &rxMessage1);

      switch (dut_info.ID)
      {
          case DUT_TYPE_GB:
              CAN_MacProcess(rxMessage1.rx_efid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
              CAN_PROTOCOL_MacProcess(rxMessage1.rx_efid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
              CAN_TEST_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
              break;
              
          default:
              CAN_PROTOCOL1_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen); // ����config
              CAN_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);           // 55����Э��
              CAN_TEST_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
              break;
      }
  }
}

extern CAN_CB sysCanCB;	
//// ISR for CAN TX Interrupt
void USBD_HP_CAN0_TX_IRQHandler(void)
{
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;
    uint16 head1 = sysCanCB.tx.head;
	uint16 end1 = sysCanCB.tx.end;
    
    // ���������жϱ�־λ
    can_interrupt_disable(CAN0, CAN_INT_TME);

    // ����Ƿ���Tx����
    if (head != end)
    {
        // Tx�ɹ����ж��Ƿ�����һ֡
        CAN_TEST_Tx_NextFram(&testCanCB);

        // ����CAN��Ϣ��һ֡����״̬
        CAN_TEST_Tx_FillAndSend(&testCanCB);
    }

	// ����Ƿ���Tx����
	if (head1 != end1)
	{
		// Tx�ɹ����ж��Ƿ�����һ֡
		CAN_Tx_NextFram(&sysCanCB);

		// ����CAN��Ϣ��һ֡����״̬
		CAN_Tx_FillAndSend(&sysCanCB);
        
	}
}

//===============================================================================
// ��ϸ�������
// ==============================================================================

// ���������Ӧ��
void CAN_PROTOCOL_TEST_SendCmdAck(uint8 param)
{
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_HEAD);
    CAN_PROTOCOL_TEST_TxAddData(param);
    CAN_PROTOCOL_TEST_TxAddData(0);
    CAN_PROTOCOL_TEST_TxAddFrame();
}

// �����������
void CAN_PROTOCOL_TEST_SendCmdTest(uint32 param)
{
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_HEAD);
    CAN_PROTOCOL_TEST_TxAddData(1);
    CAN_PROTOCOL_TEST_TxAddData(0);
    CAN_PROTOCOL_TEST_TxAddData(1);
    CAN_PROTOCOL_TEST_TxAddFrame();
}

// д���־������
void CAN_PROTOCOL_TEST_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam)
{
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_HEAD);
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_CMD_WRITE_FLAG_DATA);
    CAN_PROTOCOL_TEST_TxAddData(0x06);

    CAN_PROTOCOL_TEST_TxAddData(placeParam);
    CAN_PROTOCOL_TEST_TxAddData(0x00);
    CAN_PROTOCOL_TEST_TxAddData(0x00);
    CAN_PROTOCOL_TEST_TxAddData(0x00);
    CAN_PROTOCOL_TEST_TxAddData(0x02);
    CAN_PROTOCOL_TEST_TxAddData(shutdownFlagParam);
    CAN_PROTOCOL_TEST_TxAddFrame();
}

// ���������Ӧ��
void CAN_PROTOCOL_TEST_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_HEAD);
    CAN_PROTOCOL_TEST_TxAddData(ackCmd);
    CAN_PROTOCOL_TEST_TxAddData(1);
    CAN_PROTOCOL_TEST_TxAddData(ackParam);
    CAN_PROTOCOL_TEST_TxAddFrame();
}
