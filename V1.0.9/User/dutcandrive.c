/*********************************************************
 * ͷ�ļ���
 *********************************************************/
// canͨ��ͷ�ļ�
#include "CanProtocolUpDT.h"
#include "DutCanProtocol.h"
#include "param.h"
#include "DutInfo.h"
// �豸ͷ�ļ����������������ɾ����
#include "system.h"
#include "timer.h"
#include "spiflash.h"
#include "iap.h"
#include "gd32f10x_can.h"
/*********************************************************
 * ����������
 *********************************************************/
CAN_CB sysCanCB;              // CANͨ�����ݽṹ
CAN_TX_STATE_CB canTxStateCB; // ����״̬�����ƶ���

/*********************************************************
 * �ڲ�����������
 *********************************************************/
/************ CAN�ײ㺯�� *************************/
// ���ݽṹ��ʼ��
void CAN_DataStructureInit(CAN_CB *pCB);

// CAN ���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_MacProcess(uint16 standarID, uint8 *data, uint8 length);

// һ�����Ľ��ջ���������������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_RxFIFOProcess(CAN_CB *pCB);

// CAN����֡����������
void CAN_CmdFrameProcess(CAN_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL CAN_CheckSum(uint8 *buff, uint32 len);

// �Դ��������֡����У�飬����У����
BOOL CAN_AddSum(uint8 *buff, uint32 len);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_ConfirmTempCmdFrameBuff(CAN_CB *pCB, uint8 sourceSelect);

/************ CAN״̬������ **************************/
// ����뷢�ʹ���
BOOL CAN_Tx_FillAndSend(CAN_CB *pCB);

// ���ͳɹ�������һ֡
BOOL CAN_Tx_NextFram(CAN_CB *pCB);

// CAN����״̬����ʼ��
void CAN_TxStateInit(void);

// CAN����״̬Ǩ��
void CAN_TxEnterState(uint32 state);

// CAN����״̬������
void CAN_TxStateProcess(void);

/************ CAN�ײ㺯�� **************************/

// ����ģʽ׼����������
void IAP_CTRL_CAN_SendCmdProjectReady(uint8 param);

// ����FLASH����ϱ�
void IAP_CTRL_CAN_SendCmdEraseFlashResult(uint8 param);

// IAP����д�����ϱ�
void IAP_CTRL_CAN_SendCmdWriteFlashResult(uint8 param);

// ��ս���ϱ�
void IAP_CTRL_CAN_SendCmdCheckFlashBlankResult(uint8 param);

// ϵͳ��������ȷ��
void IAP_CTRL_CAN_SendCmdUpdataFinishResult(uint8 param);

// ����UI���ݲ������
void IAP_CTRL_CAN_SendCmdUIEraseResultReport(uint8 param);

// ����UI����д����
void IAP_CTRL_CAN_SendCmdUIWriteDataResultReport(uint8 param);

void CAN_TEST(uint32 temp)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK);
    IAP_CTRL_CAN_TxAddData(2); // ���ݳ���
    IAP_CTRL_CAN_TxAddData(55);
    IAP_CTRL_CAN_TxAddData(55);
    IAP_CTRL_CAN_TxAddFrame();
}
/*********************************************************
 * ����������
 *********************************************************/

// CAN ��ʼ��
void IAP_CTRL_CAN_Init(uint16 bund)
{
    // ��ʼ��Ӳ������
    CAN_HW_Init(bund);

    // ��ʼ���ṹ��
    CAN_DataStructureInit(&sysCanCB);

    // CAN����״̬����ʼ��
    CAN_TxStateInit();
}

// ���ݽṹ��ʼ��
void CAN_DataStructureInit(CAN_CB *pCB)
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
    for (i = 0; i < CAN_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    for (i = 0; i < CAN_DEVICE_SOURCE_ID_MAX; i++)
    {
        pCB->rxFIFO.rxFIFOEachNode[i].head = 0;
        pCB->rxFIFO.rxFIFOEachNode[i].end = 0;
        pCB->rxFIFO.rxFIFOEachNode[i].currentProcessIndex = 0;
    }

    for (i = 0; i < CAN_DEVICE_SOURCE_ID_MAX; i++)
    {
        for (j = 0; j < CAN_RX_QUEUE_SIZE; j++)
        {
            pCB->rx.rxEachNode[i].cmdQueue[j].length = 0;
        }
        pCB->rx.rxEachNode[i].head = 0;
        pCB->rx.rxEachNode[i].end = 0;
    }
}

// ��������֡�������������
void IAP_CTRL_CAN_TxAddData(uint8 data)
{
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;
    CAN_CMD_FRAME *pCmdFrame = &sysCanCB.tx.cmdQueue[sysCanCB.tx.end];

    // ���ͻ������������������
    if ((end + 1) % CAN_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= CAN_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ��������֡����,�Զ�����֡�����Լ�������
void IAP_CTRL_CAN_TxAddFrame(void)
{
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;
    uint8 *buff = sysCanCB.tx.cmdQueue[end].buff;
    uint16 length = sysCanCB.tx.cmdQueue[end].length;
    uint8 cmdHead = buff[CAN_CMD_HEAD_INDEX]; // ֡ͷ

    // ���ͻ������������������
    if ((end + 1) % CAN_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ����ͷ����
    if (CAN_CMD_HEAD != cmdHead)
    {
        // ��֡�������������ʼλ��
        sysCanCB.tx.cmdQueue[end].length = 0;
        return;
    }

    // ����ͷ��ȷ�������ݳ��Ȳ���(���ݳ��ȼ�ȥCRC�ĳ���)
    if ((CAN_CMD_FRAME_LENGTH_MAX - CAN_CHECK_BYTE_SIZE < length) || (CAN_CMD_FRAME_LENGTH_MIN - CAN_CHECK_BYTE_SIZE > length))
    {
        // ��֡�������������ʼλ��
        sysCanCB.tx.cmdQueue[end].length = 0;
        buff[CAN_CMD_HEAD_INDEX] = CAN_CMD_NONHEAD; // ��������û����ʱ���ٴν���
        return;
    }

    // ִ�е��ˣ�����ͷ��ȷ�����ݳ�����ȷ,���ͻ�����δ��
    // ���ȷ���
    buff[CAN_CMD_LENGTH_INDEX] = length - CAN_CMD_FRAME_LENGTH_MIN + CAN_CHECK_BYTE_SIZE;

    // ���CRCУ����
    CAN_AddSum(buff, length);

    // �����ݳ���������ͻ�����
    sysCanCB.tx.cmdQueue[end].length = length + CAN_CHECK_BYTE_SIZE;

    // ���ͻ��ζ��и���λ��
    sysCanCB.tx.end++;
    sysCanCB.tx.end %= CAN_TX_QUEUE_SIZE;
    sysCanCB.tx.cmdQueue[sysCanCB.tx.end].length = 0; // 2015.12.2�޸�
}

// ����뷢�ʹ���:TRUE,����뷢�ͣ�FALSE�����ζ�����ǰ����.
BOOL CAN_Tx_FillAndSend(CAN_CB *pCB)
{
    uint16 index = pCB->tx.index;                          // ��ǰ�������ݵ�������
    uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length; // ��ǰ���͵�����֡�ĳ���
    uint16 head = pCB->tx.head;                            // ��������֡����ͷ������
    uint16 end = pCB->tx.end;                              // ��������֡����β������
    CAN_CMD_FRAME *pCmd = &pCB->tx.cmdQueue[pCB->tx.head]; // ȡ��ǰ���ͻ�������ַ
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
							standardID = (uint16)CAN_DEVICE_ID_HMI; // id��001
							canTxMsgCB.tx_sfid = standardID;
							break;
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
        pCB->tx.head %= CAN_TX_QUEUE_SIZE;
        pCB->tx.index = 0;

        // ����CAN��Ϣ���ʹ���״̬
        CAN_TxEnterState(CAN_TX_STATE_STANDBY);

        return FALSE;
    }
}

// ���ͳɹ�������һ֡:TRUE,������һ֡��FALSE���������.
BOOL CAN_Tx_NextFram(CAN_CB *pCB)
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
void CAN_MacProcess(uint16 standarID, uint8 *data, uint8 length)
{
    uint16 end;
    uint16 head;
    uint8 i;
    int8 sourceBuffSelect = CAN_DEVICE_SOURCE_ID_MAX; // �ڵ㻺�������ѡ��

    // �����Ϸ����ж�
    if (NULL == data)
    {
        return;
    }

    // ��׼֡�ж�
    if (standarID > 0x7FF)
    {
        return;
    }

    // ȡ��Ŀ��ڵ�
    //  destinationID = standarID & 0x07FF;

    // ���Ƿ�������ID������ֱ�Ӷ���
    //  if (CAN_DEVICE_ID_HMI != destinationID)
    //  {
    //      return;
    //  }

    // �Ѿ�ʹ��
    if (CAN_RX_DEVICE_PUC)
    {
        sourceBuffSelect = CAN_RX_DEVICE_PUC - 1;
    }

    // û�к��ʵ�Դ�ڵ�
    if ((sourceBuffSelect >= CAN_DEVICE_SOURCE_ID_MAX) || (sourceBuffSelect < 0))
    {
        return;
    }

    // ȡ��Ӧ�������ṹ������
    end = sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end;
    head = sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].head;

    // ���������жӣ���ӡ���
    //  һ���������������������
    if ((end + 1) % CAN_RX_FIFO_SIZE == head)
    {
        return;
    }

    for (i = 0; i < length; i++)
    {
        // �����ֽڶ�ȡ��������FIFO��
        sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].buff[sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end] = *data++;

        sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end++;

        // һ���������������������
        if ((sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end + 1) % CAN_RX_FIFO_SIZE == head)
        {
            break;
        }

        sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end %= CAN_RX_FIFO_SIZE;
    }
}

// һ�����Ľ��ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void CAN_RxFIFOProcess(CAN_CB *pCB)
{
    uint16 end;
    uint16 head;
    CAN_CMD_FRAME *pCmdFrame = NULL;
    CAN_RX_FIFO *pRxFIFO = NULL;
    uint16 length = 0;
    uint8 currentData = 0;
    uint8 sourceBuffSelect;

    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return;
    }

    // ѭ���ж��Ƿ���������Ҫ����
    for (sourceBuffSelect = 0; sourceBuffSelect < CAN_DEVICE_SOURCE_ID_MAX; sourceBuffSelect++)
    {
        // ��ȡ����һ���������ڵ���ʱ������ָ��
        pRxFIFO = &pCB->rxFIFO.rxFIFOEachNode[sourceBuffSelect];

        end = pRxFIFO->end;
        head = pRxFIFO->head;

        // һ��������Ϊ�գ�����
        if (head == end)
        {
            continue;
        }

        // ��ȡ����������ʱ������ָ��
        pCmdFrame = &pCB->rx.rxEachNode[sourceBuffSelect].cmdQueue[pCB->rx.rxEachNode[sourceBuffSelect].end];

        // ȡ����ǰҪ������ֽ�
        currentData = pRxFIFO->buff[pRxFIFO->currentProcessIndex];

        // ��ʱ����������Ϊ0ʱ���������ֽ�
        if (0 == pCmdFrame->length)
        {
            // ����ͷ����ɾ����ǰ�ֽڲ��˳�
            if (CAN_CMD_HEAD != currentData)
            {
                pRxFIFO->head++;
                pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                continue;
            }

            // ����ͷ��ȷ������������ʱ���������ã��˳�
            if ((pCB->rx.rxEachNode[sourceBuffSelect].end + 1) % CAN_RX_QUEUE_SIZE == pCB->rx.rxEachNode[sourceBuffSelect].head)
            {
                continue;
            }

            // ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
            pCmdFrame->buff[pCmdFrame->length++] = currentData;
            pRxFIFO->currentProcessIndex++;
            pRxFIFO->currentProcessIndex %= CAN_RX_FIFO_SIZE;
        }
        // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
        else
        {
            // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
            if (pCmdFrame->length >= CAN_CMD_FRAME_LENGTH_MAX)
            {
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pRxFIFO->head++;
                pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                continue;
            }

            // һֱȡ��ĩβ
            while (end != pRxFIFO->currentProcessIndex)
            {
                // ȡ����ǰҪ������ֽ�
                currentData = pRxFIFO->buff[pRxFIFO->currentProcessIndex];
                // ������δ������������գ���������ӵ���ʱ��������
                pCmdFrame->buff[pCmdFrame->length++] = currentData;
                pRxFIFO->currentProcessIndex++;
                pRxFIFO->currentProcessIndex %= CAN_RX_FIFO_SIZE;

                // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������

                // �����ж�����֡��С���ȣ�һ�����������������ٰ���4���ֽ�: ����ͷ(1Byte)+������(1Byte)+���ݳ���(1Byte)+У����(1Byte) ����˲���4���ֽڵıض�������
                if (pCmdFrame->length < CAN_CMD_FRAME_LENGTH_MIN)
                {
                    // ��������
                    continue;
                }

                // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
                if (pCmdFrame->buff[CAN_CMD_LENGTH_INDEX] > (CAN_CMD_FRAME_LENGTH_MAX - CAN_CMD_FRAME_LENGTH_MIN))
                {
                    // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                    pCmdFrame->length = 0;
                    // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                    pRxFIFO->head++;
                    pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                    pRxFIFO->currentProcessIndex = pRxFIFO->head;

                    break;
                }

                // ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ�������֡�...�����ݳ��ȡ�У���룬��Ϊ����֡ʵ�ʳ���
                length = pCmdFrame->length;
                if (length < (pCmdFrame->buff[CAN_CMD_LENGTH_INDEX] + CAN_CMD_FRAME_LENGTH_MIN))
                {
                    // ����Ҫ��һ�£�˵��δ������ϣ�����
                    continue;
                }

                // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
                if (!CAN_CheckSum(pCmdFrame->buff, pCmdFrame->length))
                {
                    // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                    pCmdFrame->length = 0;
                    // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                    pRxFIFO->head++;
                    pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                    pRxFIFO->currentProcessIndex = pRxFIFO->head;

                    break;
                }

                // ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
                pRxFIFO->head += length;
                pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                CAN_ConfirmTempCmdFrameBuff(pCB, sourceBuffSelect);

                break;
            }
        }
    }

    return;
}


// CAN����֡����������
void CAN_CmdFrameProcess(CAN_CB *pCB)
{
    CAN_CMD cmd = IAP_CTRL_CAN_CMD_EMPTY;
    CAN_CMD_FRAME *pCmdFrame = NULL;
    uint8 sourceBuffSelect = CAN_DEVICE_SOURCE_ID_MAX;
    volatile uint32 temp = 0;
    volatile uint16 length;
    volatile uint8 errStatus;
    volatile uint16 i;

    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return;
    }

    // ѭ���ж��Ƿ���������Ҫ����
    for (sourceBuffSelect = 0; sourceBuffSelect < CAN_DEVICE_SOURCE_ID_MAX; sourceBuffSelect++)
    {
        // ����֡������Ϊ�գ��˳�
        if (pCB->rx.rxEachNode[sourceBuffSelect].head == pCB->rx.rxEachNode[sourceBuffSelect].end)
        {
            continue;
        }

        // ��ȡ��ǰҪ���������ָ֡��
        pCmdFrame = &pCB->rx.rxEachNode[sourceBuffSelect].cmdQueue[pCB->rx.rxEachNode[sourceBuffSelect].head];

        // ����ͷ�Ƿ����˳�
        if (CAN_CMD_HEAD != pCmdFrame->buff[CAN_CMD_HEAD_INDEX])
        {
            // ɾ������֡
            pCB->rx.rxEachNode[sourceBuffSelect].head++;
            pCB->rx.rxEachNode[sourceBuffSelect].head %= CAN_RX_QUEUE_SIZE;
            continue;
        }

        // ����ͷ�Ϸ�������ȡ����
        cmd = (CAN_CMD)(pCmdFrame->buff[CAN_CMD_CMD_INDEX]);

        switch (cmd)
        {
        case DUT_TYPE_NULL:
            break;



        default:
            break;
        }
        // ɾ������֡
        pCB->rx.rxEachNode[sourceBuffSelect].head++;
        pCB->rx.rxEachNode[sourceBuffSelect].head %= CAN_RX_QUEUE_SIZE;
    }
}

// CAN������Թ��̺���
void CAN_PROTOCOL_Process_DT(void)
{
    // CAN����FIFO����������
    //CAN_RxFIFOProcess(&sysCanCB);

    // CAN���������������
    //CAN_CmdFrameProcess(&sysCanCB);

    // CAN����״̬���������
    CAN_TxStateProcess();
}

// �Դ��������֡����У�飬����У����
BOOL CAN_CheckSum(uint8 *buff, uint32 len)
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

    cc = ~cc;

    // �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
    if (buff[len - 1] != cc)
    {
        return FALSE;
    }

    return TRUE;
}

// �Դ��������֡����У�飬����У����
BOOL CAN_AddSum(uint8 *buff, uint32 len)
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

    cc = ~cc;

    // ���У����
    buff[len] = cc;

    return TRUE;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL CAN_ConfirmTempCmdFrameBuff(CAN_CB *pCB, uint8 sourceSelect)
{
    CAN_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.rxEachNode[sourceSelect].end %= CAN_RX_QUEUE_SIZE;
    // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
    pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end].length = 0;

    return TRUE;
}

// CANTX״̬����ʼ��
void CAN_TxStateInit(void)
{
    CAN_TxEnterState(CAN_TX_STATE_STANDBY);
}

// CANTX״̬Ǩ�ƺ���
void CAN_TxEnterState(uint32 state)
{
    // �õ�ǰ��״̬��Ϊ��ʷ
    canTxStateCB.preState = canTxStateCB.state;

    // �����µ�״̬
    canTxStateCB.state = (CAN_TX_STATE_E)state;

    switch (state)
    {
    // ����״̬����ڴ������
    case CAN_TX_STATE_NULL:

        break;

    // �������״̬����ڴ������
    case CAN_TX_STATE_ENTRY:

        break;

    // ��������״̬״̬����ڴ������
    case CAN_TX_STATE_STANDBY:

        break;

    // ����CAN��Ϣ�����С���
    case CAN_TX_STATE_SENDING:

        break;

    default:
        break;
    }
}

// ���ͻ����������һ������������
BOOL CAN_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length)
{
    uint16 i;
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;

    // ��������
    if ((NULL == pArray) || (0 == length))
    {
        return FALSE;
    }

    // ���ͻ������������������
    if ((end + 1) % CAN_TX_QUEUE_SIZE == head)
    {
        return FALSE;
    }

    sysCanCB.tx.cmdQueue[end].deviceID = id;

    for (i = 0; i < length; i++)
    {
        sysCanCB.tx.cmdQueue[end].buff[i] = *pArray++;
    }
    sysCanCB.tx.cmdQueue[end].length = length;

    // ���ͻ��ζ��и���λ��
    sysCanCB.tx.end++;
    sysCanCB.tx.end %= CAN_TX_QUEUE_SIZE;
    sysCanCB.tx.cmdQueue[sysCanCB.tx.end].length = 0;

    return TRUE;
}

// CANTX���̴�����
void CAN_TxStateProcess(void)
{
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;

    switch (canTxStateCB.state)
    {
    // ����״̬����ڴ������
    case CAN_TX_STATE_NULL:

        break;

    // �������״̬����ڴ������
    case CAN_TX_STATE_ENTRY:

        break;

    // ��������״̬״̬����ڴ������
    case CAN_TX_STATE_STANDBY:
        // ����Ƿ���Tx����
        if (head != end)
        {
            // ���ô˺����������������can��Ϣ
            if (CAN_Tx_FillAndSend(&sysCanCB))
            {
                // ����CAN��Ϣ������״̬
                CAN_TxEnterState(CAN_TX_STATE_SENDING);
            }
            else
            {
                // ����CAN��Ϣ���ʹ���״̬
                CAN_TxEnterState(CAN_TX_STATE_STANDBY);
            }
        }
        else
        {
            // ����ʲôҲ������
        }

        break;

    // ����CAN��Ϣ�����С���
    case CAN_TX_STATE_SENDING:

        break;

    default:
        break;
    }
}

// ���ܺ�����CAN��ʼ��
void CAN_HW_Init(uint16 bund)
{
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;

    // ʱ�ӷ�����ʹ��
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOB);
    // rcu_periph_clock_enable(RCU_GPIOA); // ���
    rcu_periph_clock_enable(RCU_AF);

    // ����CAN�Ľ����ж�
    nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 1, 0);

    // ����CAN������ж�
    nvic_irq_enable(USBD_HP_CAN0_TX_IRQn, 2, 0);

    // CAN�ܽ���ӳ�䵽PB8-9
      gpio_pin_remap_config(GPIO_CAN_PARTIAL_REMAP,ENABLE);

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
        can_message_receive(CAN0, CAN_FIFO0, &rxMessage1);

        //switch (dut_info.ID)
        //{
        //case DUT_TYPE_GB:
        //  CAN_MacProcess(rxMessage1.rx_efid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
        //  CAN_PROTOCOL_MacProcess(rxMessage1.rx_efid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
        //  break;
        //default:
        CAN_PROTOCOL1_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen); // ����config
        //  CAN_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);           // 55����Э��
        //  break;
        //}
    }
}

// ISR for CAN TX Interrupt
void USBD_HP_CAN0_TX_IRQHandler(void)
{
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;

    // ���������жϱ�־λ
    can_interrupt_disable(CAN0, CAN_INT_TME);

    // ����Ƿ���Tx����
    if (head != end)
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
void IAP_CTRL_CAN_SendCmdNoAck(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddData(0);
    IAP_CTRL_CAN_TxAddData(1); // ��������������
    IAP_CTRL_CAN_TxAddFrame();
}

// ������������
void IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_CMD cmd, uint32 flashAddr, uint32 addr)
{
    uint8 i = 0;
    switch (upDataType)
    {
    case DUT_FILE_TYPE_HEX:                          // hex
        dut_info.reconnectionRepeatContent[0] = 121; // �ܳ���
        dut_info.reconnectionRepeatContent[1] = CAN_CMD_HEAD;
        dut_info.reconnectionRepeatContent[2] = cmd;
        dut_info.reconnectionRepeatContent[3] = 0;                                            // ���ݳ���
        dut_info.reconnectionRepeatContent[4] = 0;                                            // ����Ƿ����
        dut_info.reconnectionRepeatContent[5] = 0;                                            // �����Կ
        SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[6], flashAddr + (addr), 116); // ���߶�ȡ 4+112 ���ֽ�
        for (i = 0; i < 121; i++)                                                             // �������
        {
            IAP_CTRL_CAN_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
        }
        break;

    case DUT_FILE_TYPE_BIN:                          // bin
        dut_info.reconnectionRepeatContent[0] = 135; // �ܳ���
        dut_info.reconnectionRepeatContent[1] = CAN_CMD_HEAD;
        dut_info.reconnectionRepeatContent[2] = cmd;
        dut_info.reconnectionRepeatContent[3] = 0;                                            // ���ݳ���
        dut_info.reconnectionRepeatContent[4] = ((addr) & 0xFF000000) >> 24;                  // ���ݳ���
        dut_info.reconnectionRepeatContent[5] = ((addr) & 0x00FF0000) >> 16;                  // ���ݳ���
        dut_info.reconnectionRepeatContent[6] = ((addr) & 0x0000FF00) >> 8;                   // ���ݳ���
        dut_info.reconnectionRepeatContent[7] = ((addr) & 0x000000FF);                        // ���ݳ���
        SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[8], flashAddr + (addr), 128); // ���߶�ȡ128�ֽ�
        for (i = 0; i < 135; i++)                                                             // �������
        {
            IAP_CTRL_CAN_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
        }
        break;
    default:
        break;
    }

    IAP_CTRL_CAN_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
}

// ����ģʽ����
void IAP_CTRL_CAN_SendCmdProjectApply(uint8 *param)
{
    uint8 len = strlen((char *)param);
    uint8 i;

    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_PROJECT_APPLY);
    IAP_CTRL_CAN_TxAddData(len); // ���ݳ���

    for (i = 0; i < len; i++)
    {
        IAP_CTRL_CAN_TxAddData(param[i]);
    }

    IAP_CTRL_CAN_TxAddFrame();
}

// ����ģʽ׼����������
void IAP_CTRL_CAN_SendCmdProjectReady(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_PROJECT_READY);
    IAP_CTRL_CAN_TxAddData(1); // ���ݳ���
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddFrame();
}

// ����FLASH����ϱ�
void IAP_CTRL_CAN_SendCmdEraseFlashResult(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_IAP_ERASE_FLASH_RESULT);
    IAP_CTRL_CAN_TxAddData(1); // ���ݳ���
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddFrame();
}

// IAP����д�����ϱ�
void IAP_CTRL_CAN_SendCmdWriteFlashResult(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_IAP_WRITE_FLASH_RESULT);
    IAP_CTRL_CAN_TxAddData(1); // ���ݳ���
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddFrame();
}

// ��ս���ϱ�
void IAP_CTRL_CAN_SendCmdCheckFlashBlankResult(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_CHECK_FLASH_BLANK_RESULT);
    IAP_CTRL_CAN_TxAddData(1); // ���ݳ���
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddFrame();
}

//// ϵͳ��������ȷ��
void IAP_CTRL_CAN_SendCmdUpdataFinishResult(uint8 param)
{
  IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
  IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_UPDATA_FINISH_RESULT);
  IAP_CTRL_CAN_TxAddData(0); // ���ݳ���
  IAP_CTRL_CAN_TxAddFrame();
}

//// ����UI���ݲ������
//void IAP_CTRL_CAN_SendCmdUIEraseResultReport(uint8 param)
//{
//  IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
//  IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_UI_DATA_ERASE_ACK);
//  IAP_CTRL_CAN_TxAddData(1); // ���ݳ���
//  IAP_CTRL_CAN_TxAddData(param);
//  IAP_CTRL_CAN_TxAddFrame();
//}

//// ����UI����д����
//void IAP_CTRL_CAN_SendCmdUIWriteDataResultReport(uint8 param)
//{
//  IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
//  IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_UI_DATA_WRITE_RESULT);
//  IAP_CTRL_CAN_TxAddData(1); // ���ݳ���
//  IAP_CTRL_CAN_TxAddData(param);
//  IAP_CTRL_CAN_TxAddFrame();
//}

// �����������
void IAP_CTRL_CAN_SendCmdTest(uint32 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(1);
    IAP_CTRL_CAN_TxAddData(0);
    IAP_CTRL_CAN_TxAddData(1); // ��������������
    IAP_CTRL_CAN_TxAddFrame();
}
