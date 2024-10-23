#include "common.h"
#include "timer.h"
#include "DutUartDrive.h"
#include "uartProtocol3.h"
#include "uartProtocol.h"
#include "state.h"
#include "spiflash.h"
#include "DutInfo.h"
#include "StsProtocol.h"
#include "UartprotocolXM.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void UART_PROTOCOL4_DataStructInit(UART_PROTOCOL4_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL4_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL4_CheckSUM(UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL4_ConfirmTempCmdFrameBuff(UART_PROTOCOL4_CB *pCB);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL4_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// �ϱ�д���ò������
void UART_PROTOCOL4_ReportWriteParamResult(uint32 param); // ���õ�

void UART_PROTOCOL4_22(uint32 param); // ����

// ȫ�ֱ�������
UART_PROTOCOL4_CB uartProtocolCB4;

UART_PROTOCOL4_PARAM_CB hmiDriveCB3;

// Э���ʼ��
void UART_PROTOCOL4_Init(void)
{
    // Э������ݽṹ��ʼ��
    UART_PROTOCOL4_DataStructInit(&uartProtocolCB4);

    // ��������ע�����ݽ��սӿ�
    UART_DRIVE_RegisterDataSendService(UART_PROTOCOL4_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    UART_PROTOCOL4_RegisterDataSendService(UART_DRIVE_AddTxArray);

    //  TIMER_AddTask(TIMER_ID_PROTOCOL_3A_PARAM_TX,
    //                  200,
    //                  UART_PROTOCOL4_22,
    //                  TRUE,
    //                  TIMER_LOOP_FOREVER,
    //                  ACTION_MODE_ADD_TO_QUEUE);
}

// ��������֡�������������
void UART_PROTOCOL4_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end = uartProtocolCB4.tx.end;
    UART_PROTOCOL4_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB4.tx.cmdQueue[uartProtocolCB4.tx.end];

    // ���ͻ������������������
    if ((end + 1) % UART_PROTOCOL4_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void UART_PROTOCOL4_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end = uartProtocolCB4.tx.end;
    UART_PROTOCOL4_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB4.tx.cmdQueue[uartProtocolCB4.tx.end];
    uint16 length = pCmdFrame->length;

    // ���ͻ������������������
    if ((end + 1) % UART_PROTOCOL4_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ����֡���Ȳ��㣬������������ݣ��˳�
    if (UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN - 4 > length) // ��ȥ"У���L��У���H��������ʶ0xD��������ʶOxA"4���ֽ�
    {
        pCmdFrame->length = 0;

        return;
    }

    // ��β����֡�������˳�
    if ((length >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX) || (length + 2 >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX))
    {
        return;
    }

    // �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
    pCmdFrame->buff[UART_PROTOCOL4_CMD_LENGTH_INDEX] = length - 4; // �������ݳ��ȣ���ȥ"����ͷ���豸��ַ�������֡����ݳ���"4���ֽ�
    for (i = 1; i < length; i++)
    {
        checkSum += pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);        // ���ֽ���ǰ
    pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF); // ���ֽ��ں�

    // ������ʶ
    pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
    pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

    uartProtocolCB4.tx.end++;
    uartProtocolCB4.tx.end %= UART_PROTOCOL4_TX_QUEUE_SIZE;
}

// ���ݽṹ��ʼ��
void UART_PROTOCOL4_DataStructInit(UART_PROTOCOL4_CB *pCB)
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
    for (i = 0; i < UART_PROTOCOL4_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < UART_PROTOCOL4_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL4_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB4.rxFIFO.end;
    uint16 head = uartProtocolCB4.rxFIFO.head;
    uint8 rxdata = 0x00;

    // ��������
    rxdata = *pData;

    // ����KM5SЭ�����
    UART_PROTOCOL_MacProcess(standarID, pData, length);
    UART_PROTOCOL_XM_MacProcess(standarID, pData, length);
    
    // һ���������������������
    if ((end + 1) % UART_PROTOCOL4_RX_FIFO_SIZE == head)
    {
        return;
    }
    // һ��������δ��������
    else
    {
        // �����յ������ݷŵ���ʱ��������
        uartProtocolCB4.rxFIFO.buff[end] = rxdata;
        uartProtocolCB4.rxFIFO.end++;
        uartProtocolCB4.rxFIFO.end %= UART_PROTOCOL4_RX_FIFO_SIZE;
    }
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL4_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB4.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL4_ConfirmTempCmdFrameBuff(UART_PROTOCOL4_CB *pCB)
{
    UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= UART_PROTOCOL4_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void UART_PROTOCOL4_TxStateProcess(void)
{
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end = uartProtocolCB4.tx.end;
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
    uartProtocolCB4.tx.head++;
    uartProtocolCB4.tx.head %= UART_PROTOCOL4_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL4_RxFIFOProcess(UART_PROTOCOL4_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (UART_PROTOCOL4_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL4_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % UART_PROTOCOL4_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL4_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= UART_PROTOCOL4_RX_CMD_FRAME_LENGTH_MAX)
        {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0; // 2016.1.5����
            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL4_RX_FIFO_SIZE;
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
            if (pCmdFrame->length < UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            if (pCmdFrame->buff[UART_PROTOCOL4_CMD_LENGTH_INDEX] > UART_PROTOCOL4_RX_CMD_FRAME_LENGTH_MAX - UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN)
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL4_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����Ϊ����֡ʵ�ʳ���
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[UART_PROTOCOL4_CMD_LENGTH_INDEX] + UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN)
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!UART_PROTOCOL4_CheckSUM(pCmdFrame))
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL4_RX_FIFO_SIZE;
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
            UART_PROTOCOL4_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL4_CheckSUM(UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame)
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
void UART_PROTOCOL4_CmdFrameProcess(UART_PROTOCOL4_CB *pCB)
{
    UART_PROTOCOL4_CMD cmd = UART_PROTOCOL4_CMD_NULL;
    UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame = NULL;
    
    uint8_t i = 0;
    BOOL checkMark;
    
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
    if (UART_PROTOCOL4_CMD_HEAD != pCmdFrame->buff[UART_PROTOCOL4_CMD_HEAD_INDEX])
    {
        // ɾ������֡
        pCB->rx.head++;
        pCB->rx.head %= UART_PROTOCOL4_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (UART_PROTOCOL4_CMD)pCmdFrame->buff[UART_PROTOCOL4_CMD_CMD_INDEX];

    // ִ������֡
    switch (cmd)
    {
        // ���������ִ��
        case UART_PROTOCOL4_CMD_NULL: // 0x53
            break;

//       // Э���л�
//       case UART_PROTOCOL4_CMD_PROTOCOL_SWITCCH:
//            TIMER_KillTask(TIMER_ID_PROTOCOL_SWITCCH);
//            
//            // Э���л��ɹ�
//            if(pCmdFrame->buff[UART_PROTOCOL4_CMD_DATA1_INDEX])
//            {
//                STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
//                
//                // ��������
//                TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
//                              500,
//                              STATE_SwitchStep,
//                              STEP_UART_SEND_SET_CONFIG,
//                              TIMER_LOOP_FOREVER,
//                              ACTION_MODE_ADD_TO_QUEUE);
//            }             
//            break;
//            
//        case UART_PROTOCOL4_CMD_WRITE_CONFIG_RESULT: // 0xc1
//            // dut_config�����ɹ�
//            STATE_SwitchStep(STEP_UART_READ_CONFIG);
//            break;

//        case UART_PROTOCOL4_CMD_NEW_WRITE_CONTROL_PARAM: // 0x50
//            // dut_config�����ɹ�
//            STATE_SwitchStep(STEP_UART_READ_CONFIG);
//            break;

//        // У�����ò���
//        case UART_PROTOCOL4_CMD_READ_CONTROL_PARAM_REPORT: // 0xc3     
//            
//            for(i = 0; i< pCmdFrame->buff[UART_PROTOCOL4_CMD_LENGTH_INDEX]; i++)
//            {  
//               if(verifiedBuff[i] == pCmdFrame->buff[UART_PROTOCOL4_CMD_DATA1_INDEX + i] )
//               {
//                  checkMark = TRUE;
//               }
//               else
//               {
//                  checkMark = FALSE;
//               }
//            }
//            
//            if(checkMark)
//            {
//                STATE_SwitchStep(STEP_UART_SET_CONFIG_SUCCESS);
//            }
//            else
//            {
//                STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_ITEM_FINISH);
//            }

//            break;

        default:
            if(dut_info.passThroughControl)
            {
                STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);            
            }             
            break;
    }

    // ɾ������֡
    pCB->rx.head++;
    pCB->rx.head %= UART_PROTOCOL4_RX_QUEUE_SIZE;
}
