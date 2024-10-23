#include "common.h"
#include "timer.h"
#include "DutUartDrive.h"
#include "UartprotocolXM.h"
#include "StsProtocol.h"
#include "state.h"
#include "spiflash.h"
#include "AvoPin.h"
#include "AvoProtocol.h"
#include "param.h"
#include "DutInfo.h"
#include "PowerProtocol.h"
#include "stringOperation.h"
#include "DutUartProtocol.h"
#include "uartProtocol.h"
#include "uartProtocol3.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void UART_PROTOCOL_XM_DataStructInit(UART_PROTOCOL_XM_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL_XM_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL_XM_CheckSUM(UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL_XM_ConfirmTempCmdFrameBuff(UART_PROTOCOL_XM_CB *pCB);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL_XM_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// �ϱ�д���ò������
void UART_PROTOCOL_XM_ReportWriteParamResult(uint32 param); // ���õ�

// ��������ظ�
void UART_PROTOCOL_XM_SendCmdAck(uint8 ackCmd);

// ��������ظ�����һ������
void UART_PROTOCOL_XM_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// ��������ظ���������������
void UART_PROTOCOL_XM_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam);

// UARTЭ�����̴���
void UART_PROTOCOL_XM_Process(void);

// ȫ�ֱ�������
extern char ptUpMsgBuff[][MAX_ONE_LINES_LENGTH];

UART_PROTOCOL_XM_CB uartProtocolCB5;
////uint8 light_cnt = 0;
//uint8 cnt = 0;
////uint8 cali_cnt = 0;
//uint8 brake_cnt = 0;
//uint8 left_light_cnt = 0;
//uint8 right_light_cnt = 0;
//BOOL write_success_Flag;
//uint32 online_detection_cnt = 0;
uint8 conVer[20];
uint8 verLenght = 0;

const uint16 crc16Tab[] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

uint16 calCrc16(unsigned char *buf, int len)
{
    int i;
    unsigned short cksum;

    cksum = 0;
    for (i = 0; i < len; i++)
    {
        cksum = crc16Tab[((cksum >> 8) ^ *buf++) & 0xFF] ^ (cksum << 8);
    }

    return cksum;
}

// Э���ʼ��
void UART_PROTOCOL_XM_Init(void)
{
    // Э������ݽṹ��ʼ��
    UART_PROTOCOL_XM_DataStructInit(&uartProtocolCB5);

    // ��������ע�����ݽ��սӿ�
    UART_DRIVE_RegisterDataSendService(UART_PROTOCOL_XM_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    UART_PROTOCOL_XM_RegisterDataSendService(UART_DRIVE_AddTxArray);
}

// UARTЭ�����̴���
void UART_PROTOCOL_XM_Process(void)
{
    // UART����FIFO����������
    UART_PROTOCOL_XM_RxFIFOProcess(&uartProtocolCB5);

    // UART���������������
    UART_PROTOCOL_XM_CmdFrameProcess(&uartProtocolCB5);

    // UARTЭ��㷢�ʹ������
    UART_PROTOCOL_XM_TxStateProcess();
}

// ��������֡�������������
void UART_PROTOCOL_XM_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB5.tx.head;
    uint16 end = uartProtocolCB5.tx.end;
    UART_PROTOCOL_XM_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB5.tx.cmdQueue[uartProtocolCB5.tx.end];

    // ���ͻ������������������
    if ((end + 1) % UART_PROTOCOL_XM_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= UART_PROTOCOL_XM_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void UART_PROTOCOL_XM_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = uartProtocolCB5.tx.head;
    uint16 end = uartProtocolCB5.tx.end;
    UART_PROTOCOL_XM_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB5.tx.cmdQueue[uartProtocolCB5.tx.end];
    uint16 length = pCmdFrame->length;

    // ���ͻ������������������
    if ((end + 1) % UART_PROTOCOL_XM_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ����֡���Ȳ��㣬������������ݣ��˳�
    if (UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN - 2 > length) // ��ȥ"CRCУ��λH,L"2���ֽ�
    {
        pCmdFrame->length = 0;

        return;
    }


    // ��β����֡�������˳�
    if ((length + 2 >= UART_PROTOCOL_XM_TX_CMD_FRAME_LENGTH_MAX)) // ����2���ֽڸ�"У��͡�������"
    {
        return;
    }

    // �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
    // �������ݳ��ȣ���Ҫ��4=("1ͷ+1�豸��+1����+1����")
    pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX] = length - 4;    // �������ݳ���

    checkSum = calCrc16(pCmdFrame->buff, length);

    pCmdFrame->buff[pCmdFrame->length] = (uint8)((checkSum >> 8) & 0xFF);
    pCmdFrame->length ++;

    pCmdFrame->buff[pCmdFrame->length] = (uint8)(checkSum & 0xFF);
    pCmdFrame->length ++;

    uartProtocolCB5.tx.end ++;
    uartProtocolCB5.tx.end %= UART_PROTOCOL_XM_TX_QUEUE_SIZE;
}

// ���ݽṹ��ʼ��
void UART_PROTOCOL_XM_DataStructInit(UART_PROTOCOL_XM_CB *pCB)
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
    for (i = 0; i < UART_PROTOCOL_XM_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < UART_PROTOCOL_XM_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL_XM_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB5.rxFIFO.end;
    uint16 head = uartProtocolCB5.rxFIFO.head;
    uint8 rxdata = 0x00;

    // ��������
    rxdata = *pData;

    // ����KM5SЭ�����
   UART_PROTOCOL_MacProcess(standarID, pData, length);
   DUT_PROTOCOL_MacProcess(standarID, pData, length);
//   UART_PROTOCOL4_MacProcess(standarID, pData, length);

    // һ���������������������
    if ((end + 1) % UART_PROTOCOL_XM_RX_FIFO_SIZE == head)
    {
        return;
    }
    // һ��������δ��������
    else
    {
        // �����յ������ݷŵ���ʱ��������
        uartProtocolCB5.rxFIFO.buff[end] = rxdata;
        uartProtocolCB5.rxFIFO.end++;
        uartProtocolCB5.rxFIFO.end %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
    }
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void UART_PROTOCOL_XM_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB5.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL UART_PROTOCOL_XM_ConfirmTempCmdFrameBuff(UART_PROTOCOL_XM_CB *pCB)
{
    UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= UART_PROTOCOL_XM_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void UART_PROTOCOL_XM_TxStateProcess(void)
{
    uint16 head = uartProtocolCB5.tx.head;
    uint16 end = uartProtocolCB5.tx.end;
    uint16 length = uartProtocolCB5.tx.cmdQueue[head].length;
    uint8 *pCmd = uartProtocolCB5.tx.cmdQueue[head].buff;
    uint16 localDeviceID = uartProtocolCB5.tx.cmdQueue[head].deviceID;

    // ���ͻ�����Ϊ�գ�˵��������
    if (head == end)
    {
        return;
    }

    // ���ͺ���û��ע��ֱ�ӷ���
    if (NULL == uartProtocolCB5.sendDataThrowService)
    {
        return;
    }

    // Э�����������Ҫ���͵�������
    if (!(*uartProtocolCB5.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // ���ͻ��ζ��и���λ��
    uartProtocolCB5.tx.cmdQueue[head].length = 0;
    uartProtocolCB5.tx.head++;
    uartProtocolCB5.tx.head %= UART_PROTOCOL_XM_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL_XM_RxFIFOProcess(UART_PROTOCOL_XM_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (UART_PROTOCOL_XM_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			// ȡ����ǰҪ������ֽ�
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			
			// һ��������Ϊ�գ��˳�
			if(pCB->rxFIFO.head == pCB->rxFIFO.end)
			{
				return;
			}
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % UART_PROTOCOL_XM_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= UART_PROTOCOL_XM_RX_CMD_FRAME_LENGTH_MAX)
        {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0; // 2016.1.5����
            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_XM_RX_FIFO_SIZE;

            // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������

            // �����ж�����֡��С���ȣ�һ�����������������ٰ���6���ֽ�: ����֡��С���ȣ�����:����ͷ���豸�š������֡�֡���ȡ�У����L��У����H����˲���6���ֽڵıض�������
            if (pCmdFrame->length < UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            if ((uint16)pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX] + UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN > UART_PROTOCOL_XM_RX_CMD_FRAME_LENGTH_MAX)
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;

                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA����Ϊ����֡ʵ�ʳ���
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX] + UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN)
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!UART_PROTOCOL_XM_CheckSUM(pCmdFrame))
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;
                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            UART_PROTOCOL_XM_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL UART_PROTOCOL_XM_CheckSUM(UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame)
{
    uint16 cc = 0;
    uint16 checkValue = 0;

    if (NULL == pCmdFrame)
    {
        return FALSE;
    }

    // CRCУ��
    checkValue = calCrc16(pCmdFrame->buff, pCmdFrame->length - 2);

    cc = ((uint16)pCmdFrame->buff[pCmdFrame->length - 2] << 8) | (uint16)pCmdFrame->buff[pCmdFrame->length - 1];

    if (cc != checkValue)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL checkMark;
uint8_t infoLen = 0;

// UART����֡����������
void UART_PROTOCOL_XM_CmdFrameProcess(UART_PROTOCOL_XM_CB *pCB)
{
    UART_PROTOCOL_XM_CMD cmd = UART_PROTOCOL_XM_CMD_NULL;
    UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame = NULL;

    uint8 switch_state = 0;
    uint8 Peripheral_type = 0;
    uint8 calibration_result = 0;
    uint8 turn_item = 0;
    uint8 turn_state = 0;
    uint8 buffFlag[4] = {0xFF};
    uint8 i =  0;
    uint8_t verType = 0;
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
    if (UART_PROTOCOL_XM_CMD_HEAD != pCmdFrame->buff[UART_PROTOCOL_XM_CMD_HEAD_INDEX])
    {
        // ɾ������֡
        pCB->rx.head++;
        pCB->rx.head %= UART_PROTOCOL_XM_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (UART_PROTOCOL_XM_CMD)pCmdFrame->buff[UART_PROTOCOL_XM_CMD_CMD_INDEX];

    // ִ������֡
    switch (cmd)
    {
        // ���������ִ��
        case UART_PROTOCOL_XM_CMD_NULL:
            break;

        // �յ����Ӧ��
        case UART_PROTOCOL_XM_CMD_TEST_LIGHT:
            switch_state = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];   // ִ�н��
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
            
        // ��ȡ����/ɲ����ģ������ֵ
        case UART_PROTOCOL_XM_CMD_TEST_Throttle_Break:                      

            dut_info.throttleAd = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX];
            dut_info.brakeAd = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA2_INDEX];

            // ����
            if (TEST_TYPE_XM_THROTTLE == dut_info.test)
            {
                // ��ȡ����ADֵ
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
            if (TEST_TYPE_XM_BRAKE == dut_info.test)
            {
                // ��ȡɲ��ADֵ
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

            break;

        // ת��ƿ���
        case UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT:
            turn_item = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX];   // ת�������
            light_cnt++;

            // ��ת���
            if ((dut_info.test == TEST_TYPE_XM_LEFT_TURN_SIGNAL) && (1 == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN);
                }
            }

            // ��ת���
            if ((dut_info.test == TEST_TYPE_XM_RIGHT_TURN_SIGNAL) && (2 == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN);
                }
            }

            // �ص�
            if ((dut_info.test == TEST_TYPE_XM_LEFT_TURN_SIGNAL) && (0 == turn_item))
            {
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }
            // �ص�
            if ((dut_info.test == TEST_TYPE_XM_RIGHT_TURN_SIGNAL) && (0 == turn_item))
            {
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }
            break;

        // �ʶ���/�������ʾ����
        case UART_PROTOCOL_XM_CMD_TEST_LED:
             STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_DISPLAY_TEST, TRUE);
            break;

        // ���԰���
        case UART_PROTOCOL_XM_CMD_TEST_KEY:
            if(keyValue == pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX])
            {
                 testResult = TRUE;
            }
            else
            {
                 testResult = FALSE;
            }
            STS_PROTOCOL_SendCmdParamTwoAck(STS_PROTOCOL_CMD_SIMULATION_KEY_TEST, testResult, pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX]);
            break;

        // ����������
        case UART_PROTOCOL_XM_CMD_TEST_BEEP:
             STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_BUZZER_TEST, TRUE);
            break;

        // ���߼��
        case UART_PROTOCOL_XM_CMD_CONTORY_UP:
            online_detection_cnt++;
            TIMER_ResetTimer(TIMER_ID_ONLINE_DETECT);

            if(1 == online_detection_cnt)
            {
               // �ϱ��Ǳ�����״̬
               STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION,TRUE);
            }
            break;

        // ��ȡ��־������
        case UART_PROTOCOL_XM_CMD_READ_FLAG_DATA:
            TIMER_KillTask(TIMER_ID_WRITE_FLAG_TIMEOUT);

            // ��ȡ��־������
             buffFlag[0] = pCmdFrame->buff[stationNumber + 5];

            // У���־������
            if (buffFlag[0] == 2)
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, TRUE);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
            }
            break;

        // д���־������
        case UART_PROTOCOL_XM_CMD_WRITE_FLAG_DATA:

            // �ر�д�볬ʱ��ʱ��
            TIMER_ResetTimer(TIMER_ID_WRITE_FLAG_TIMEOUT);

            // �յ�д��ɹ��ٶ�ȡ��������У��
            if (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
            {
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_READ_FLAG_DATA);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
            }
            break;

        // д��汾��������
        case UART_PROTOCOL_XM_CMD_WRITE_VERSION_TYPE:

            // �رն�ʱ��
            TIMER_ResetTimer(TIMER_ID_OPERATE_VER_TIMEOUT);

            // ��ȡ����
            verType =  pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX];

            // ������ȷ����ִ�гɹ����ϱ�STS
            if ((dutverType == verType) && (pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA2_INDEX]))
            {
                // ��ȡ�汾������Ϣ
                UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_READ_VERSION_TYPE, dutverType);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_VERSION_INFO, FALSE);
            }
            break;

        // ��ȡ�汾������Ϣ
        case UART_PROTOCOL_XM_CMD_READ_VERSION_TYPE:
            TIMER_KillTask(TIMER_ID_OPERATE_VER_TIMEOUT);

            // ��ȡ����
            verType =  pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX];

            // ��ȡ��Ϣ����
            infoLen = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA2_INDEX];

            // ��ȡ������Ϣ
            strcpy(verBuff, (const char *)&pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA3_INDEX]);                        
            
            verBuff[infoLen] = 0;
            if(3 == verType)
            {
                 verBuff[infoLen - 1] = 0;            
            }

            // ֻ��ȡ
            if (verreadFlag)
            {
                if (verType == 1)
                {
                    verType = 0;
                }
                else if (verType == 2)
                {
                    verType = 1;
                }
                else if (verType == 3)
                {
                    verType = 2;
                }
                else if (verType == 4)
                {
                    verType = 4;
                }
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                STS_PROTOCOL_TxAddData(0x00);       // ���ݳ���
                STS_PROTOCOL_TxAddData(1);          // У���ȡ���
                STS_PROTOCOL_TxAddData(verType);    // �汾����
                
                if(2 == verType)
                {
                    STS_PROTOCOL_TxAddData(infoLen - 1);    // ��Ϣ����

                    // ������Ϣ
                    for (i = 0; i < (infoLen - 1); i++)
                    {
                        STS_PROTOCOL_TxAddData(verBuff[i]);
                    }                
                }
                else
                {
                    STS_PROTOCOL_TxAddData(infoLen);    // ��Ϣ����

                    // ������Ϣ
                    for (i = 0; i < (infoLen); i++)
                    {
                        STS_PROTOCOL_TxAddData(verBuff[i]);
                    }                 
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
                    if (verType == 1)
                    {
                        verType = 0;
                    }
                    else if (verType == 2)
                    {
                        verType = 1;
                    }
                    else if (verType == 3)
                    {
                        verType = 2;
                    }
                    else if (verType == 4)
                    {
                        verType = 4;
                    }
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                    STS_PROTOCOL_TxAddData(0x00);       // ���ݳ���
                    STS_PROTOCOL_TxAddData(1);          // У���ȡ���
                    STS_PROTOCOL_TxAddData(verType);    // �汾����
                    STS_PROTOCOL_TxAddData(infoLen);    // ��Ϣ����

                    // ������Ϣ
                    for (i = 0; i < infoLen; i++)
                    {
                        STS_PROTOCOL_TxAddData(verBuff[i]);
                    }
                    STS_PROTOCOL_TxAddFrame();     // �汾����
                }
                else
                {
                    if (verType == 1)
                    {
                        verType = 0;
                    }
                    else if (verType == 2)
                    {
                        verType = 1;
                    }
                    else if (verType == 3)
                    {
                        verType = 2;
                    }
                    else if (verType == 4)
                    {
                        verType = 4;
                    }
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                    STS_PROTOCOL_TxAddData(0x00);       // ���ݳ���
                    STS_PROTOCOL_TxAddData(0);          // У���ȡ���
                    STS_PROTOCOL_TxAddData(verType);    // �汾����
                    STS_PROTOCOL_TxAddData(infoLen);    // ��Ϣ����

                    // ������Ϣ
                    for (i = 0; i < infoLen; i++)
                    {
                        STS_PROTOCOL_TxAddData(verBuff[i]);
                    }
                    STS_PROTOCOL_TxAddFrame();     // �汾����
                }
                verwriteFlag = FALSE;
            }

            break;

        // ��������
        case UART_PROTOCOL_XM_CMD_GET_OOB_PARAM:

            // �汾�ȶԳɹ���MAC��ַ������Դ������
            if (checkMark)
            {
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT);
                POWER_PROTOCOL_TxAddData(12);
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA17_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA17_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA18_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA18_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA19_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA19_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA20_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA20_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA21_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA21_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA22_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA22_INDEX]));
                POWER_PROTOCOL_TxAddFrame();
            }

            // �汾�ȶ�ʧ���ϱ�STS
            if (!checkMark)
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                STS_PROTOCOL_TxAddData(7 + verLenght);
                STS_PROTOCOL_TxAddData(1);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA17_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA18_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA19_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA20_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA21_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA22_INDEX]);
                
//                // �ϱ���ȷ�汾
//                for(i = 0 ; i < verLenght ; i++)
//                {
//                    STS_PROTOCOL_TxAddData(conVer[i]);
//                }
                
                STS_PROTOCOL_TxAddFrame();
            }
            break;

        // ��ȡ�����̼��汾
        case UART_PROTOCOL_XM_CMD_TEST_BLE_VERSION:

            verLenght = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX];
            
            // �洢ת�����MAC��ַ
            for (i = 0; i < pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX]; i++)
            {
                conVer[i * 2] = HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX + i]);
                conVer[i * 2 + 1] = HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX + i]);
            }
            
            // �汾һ�¾Ͳ�ѯMAC��ַ
            for (i = 0; i < pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX]; i++)
            {
                if (verBle[i] != conVer[i])
                {
                    checkMark = FALSE;  // ���ֲ�ƥ�䣬���ñ�־Ϊ��
                    break;              // ����ѭ��
                }
                else
                {
                    checkMark = TRUE;   // ����Ԫ�ض�ƥ�䣬���ñ�־Ϊ��
                }                    
            }
                        
            // ��ѯMAC��ַ
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);                
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_GET_OOB_PARAM);
            UART_PROTOCOL_XM_TxAddData(0x00);         // ���ݳ���
            UART_PROTOCOL_XM_TxAddFrame(); 
            break;
            
        default:
            break;
    }

    // ɾ������֡
    pCB->rx.head++;
    pCB->rx.head %= UART_PROTOCOL_XM_RX_QUEUE_SIZE;
}

// ��������ظ�
void UART_PROTOCOL_XM_SendCmdAck(uint8 ackCmd)
{
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);

    UART_PROTOCOL_XM_TxAddData(ackCmd);
    UART_PROTOCOL_XM_TxAddData(0x00);         // ���ݳ���
    UART_PROTOCOL_XM_TxAddFrame();
}

// ��������ظ�����һ������
void UART_PROTOCOL_XM_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{

    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);

    UART_PROTOCOL_XM_TxAddData(ackCmd);
    UART_PROTOCOL_XM_TxAddData(0x01);

    UART_PROTOCOL_XM_TxAddData(ackParam);
    UART_PROTOCOL_XM_TxAddFrame();
}

// ��������ظ���������������
void UART_PROTOCOL_XM_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam)
{
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
    UART_PROTOCOL_XM_TxAddData(ackCmd);
    UART_PROTOCOL_XM_TxAddData(0x02);

    UART_PROTOCOL_XM_TxAddData(ackParam);
    UART_PROTOCOL_XM_TxAddData(two_ackParam);
    UART_PROTOCOL_XM_TxAddFrame();
}

// �ϱ�����״̬
//void UART_PROTOCOL_XM_SendOnlineStart(uint32 param)
//{
//    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION, SHUTDOWN);
//}

// д���־������
void UART_PROTOCOL_XM_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam)
{

    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_WRITE_FLAG_DATA);
    UART_PROTOCOL_XM_TxAddData(0x06);

    UART_PROTOCOL_XM_TxAddData(placeParam);
    UART_PROTOCOL_XM_TxAddData(0x00);
    UART_PROTOCOL_XM_TxAddData(0x00);
    UART_PROTOCOL_XM_TxAddData(0x00);
    UART_PROTOCOL_XM_TxAddData(0x02);
    UART_PROTOCOL_XM_TxAddData(shutdownFlagParam);
    UART_PROTOCOL_XM_TxAddFrame();
}

// ��������ظ�
void UART_PROTOCOL_XM_Test(uint32 param)
{
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);

    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_GET_OOB_PARAM);
    UART_PROTOCOL_XM_TxAddData(0x00);         // ���ݳ���
    UART_PROTOCOL_XM_TxAddFrame();
}

