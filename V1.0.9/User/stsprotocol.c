#include "common.h"
#include "StsDrive.h"
#include "StsProtocol.h"
#include "DutUartDrive.h"
#include "DutUartProtocol.h"
#include "CanProtocolUpDT.h"
#include "CanProtocol_3A.h"
#include "timer.h"
#include "param.h"
#include "State.h"
#include "Spiflash.h"
#include "system.h"
#include "iap.h"
#include "state.h"
#include "AvoPin.h"
#include "AvoProtocol.h"
#include "PowerProtocol.h"
#include "DutInfo.h"
#include "CanProtocolTest.h"
#include "UartprotocolXM.h"
#include "adc.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void STS_PROTOCOL_DataStructInit(STS_PROTOCOL_CB *pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void STS_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void STS_PROTOCOL_RxFIFOProcess(STS_PROTOCOL_CB *pCB);

// UART����֡����������
void STS_PROTOCOL_CmdFrameProcess(STS_PROTOCOL_CB *pCB);

// �Դ��������֡����У�飬����У����
BOOL STS_PROTOCOL_CheckSUM(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL STS_PROTOCOL_ConfirmTempCmdFrameBuff(STS_PROTOCOL_CB *pCB);

// Э��㷢�ʹ������
void STS_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void STS_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// ѡ��Э��д���־������
void SELECT_PROTOCOL_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam);

// ѡ��Э�鷢������
void SELECT_PROTOCOL_SendCmdAck(uint8 param);

// ѡ��Э�鷢�������һ������
void SELECT_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);
//===============================================================================================================

// ��������ظ�
void STS_PROTOCOL_SendCmdAck(uint8 ackCmd);

// ���ʹ�Ӧ��Ļظ�
void STS_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// �Ǳ�״̬��Ϣ�ظ�
void STS_PROTOCOL_SendCmdStatusAck(void);

// DUTͨѶ��ʱ����
void DutTimeOut(uint32 param);

//=======================================================================================
// ȫ�ֱ�������
STS_PROTOCOL_CB STSProtocolCB;
CALIBRATION calibration;
uint8 stationNumber;
BOOL offFlag;
char dutverBuff[100] = {0};
uint8 dutverType = 0;
char progressStr[3];
BOOL showFlag = TRUE;
BOOL shutdownFlag;
BOOL dutFlag = TRUE;
BOOL verwriteFlag;
BOOL verreadFlag;
uint8 paramLength[150] = {0};
uint8 paramNumber[150] = {0};
uint8 paramContent[50][50] = {0};
uint8 resultArray[150];  // ���ڴ洢�����һά����
uint8 resultIndex = 0;
uint8 verifiedBuff[150];
uint8 verifiedIndex = 0;
uint8 configs[300] = {0};
uint8 noLenght = 0;
BOOL testFlag = FALSE;
uint32 powerPacketSize = 0;
uint8 verBle[20] = {0};
uint8 keyValue = 0;

//uint8 segBuff[72][15] =
//{
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
//    { 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00},
//    { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00},
//    { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00},
//    { 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10},
//    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},

//};

// DTA������ʾ
char dtaUpMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "Erasure APP Data",
    "Writing APP Data",
    "Updating DTA ",
    "Set Param Succeed",
};

// DUT������ʾ
char dutUpMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "Writing APP Data",
    "Updating DUT ",
    "Up Progress ",
    "DUT Up Completed",
};

// ������ʾ
char verTestMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "Sta Verifica",
    "Ver Verifica",
    "LCD",
    "KEY",
};

char showTestMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "SHOW",
    "FLASH Veri",
    "BT",
    "USB",
};

char volTestMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "VOL",
    "UART",
    "VLK",
    "Passing Through",
};

char deTestMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "DERAILLEUR",
};

// Э���ʼ��
void STS_PROTOCOL_Init(void)
{
    // Э������ݽṹ��ʼ��
    STS_PROTOCOL_DataStructInit(&STSProtocolCB);

    // ��������ע�����ݽ��սӿ�
    STS_UART_RegisterDataSendService(STS_PROTOCOL_MacProcess);

    // ��������ע�����ݷ��ͽӿ�
    STS_PROTOCOL_RegisterDataSendService(STS_UART_AddTxArray);
}

// STSЭ�����̴���
void STS_PROTOCOL_Process(void)
{
    // STS����FIFO����������
    STS_PROTOCOL_RxFIFOProcess(&STSProtocolCB);

    // STS���������������
    STS_PROTOCOL_CmdFrameProcess(&STSProtocolCB);

    // STSЭ��㷢�ʹ������
    STS_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void STS_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = STSProtocolCB.tx.head;
    uint16 end = STSProtocolCB.tx.end;
    STS_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &STSProtocolCB.tx.cmdQueue[STSProtocolCB.tx.end];

    // ���ͻ������������������
    if ((end + 1) % STS_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if (pCmdFrame->length >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // ������ӵ�֡ĩβ��������֡����
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void STS_PROTOCOL_TxAddFrame(void)
{
    uint16 cc = 0;
    uint16 i = 0;
    uint16 head = STSProtocolCB.tx.head;
    uint16 end = STSProtocolCB.tx.end;
    STS_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &STSProtocolCB.tx.cmdQueue[STSProtocolCB.tx.end];
    uint16 length = pCmdFrame->length;

    // ���ͻ������������������
    if ((end + 1) % STS_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // ��β����֡�������˳�
    if ((length >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
            || (length + 2 >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
    {
        pCmdFrame->length = 0;

        return;
    }

    // ��β����֡�������˳�
    if (length >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
    pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] = length - 3;   // �������ݳ��ȣ���ȥ"����ͷ�������֡����ݳ���"4���ֽ�

    for (i = 0; i < length; i++)
    {
        cc ^= pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = cc ;

    STSProtocolCB.tx.end++;
    STSProtocolCB.tx.end %= STS_PROTOCOL_TX_QUEUE_SIZE;
    // pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}

// ���ݽṹ��ʼ��
void STS_PROTOCOL_DataStructInit(STS_PROTOCOL_CB *pCB)
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
    for (i = 0; i < STS_PROTOCOL_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < STS_PROTOCOL_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }

//  pCB->isTimeCheck = FALSE;
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void STS_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = STSProtocolCB.rxFIFO.end;
    uint16 head = STSProtocolCB.rxFIFO.head;
    uint8 rxdata = 0x00;

    // ��������
    rxdata = *pData;

    if (dut_info.passThroughControl)
    {
        if (dut_info.dutBusType)
        {
            CAN_TEST_DRIVE_AddTxArray(standarID, pData, length);
            testFlag = TRUE;
        }
        else
        {
            UART_DRIVE_AddTxArray(standarID, pData, length);
        }
    }

    // һ���������������������
    if ((end + 1) % STS_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // һ��������δ��������
    else
    {
        // �����յ������ݷŵ���ʱ��������
        STSProtocolCB.rxFIFO.buff[end] = rxdata;
        STSProtocolCB.rxFIFO.end++;
        STSProtocolCB.rxFIFO.end %= STS_PROTOCOL_RX_FIFO_SIZE;
    }
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void STS_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    STSProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL STS_PROTOCOL_ConfirmTempCmdFrameBuff(STS_PROTOCOL_CB *pCB)
{
    STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= STS_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������

    return TRUE;
}

// Э��㷢�ʹ������
void STS_PROTOCOL_TxStateProcess(void)
{
    uint16 head = STSProtocolCB.tx.head;
    uint16 end = STSProtocolCB.tx.end;
    uint16 length = STSProtocolCB.tx.cmdQueue[head].length;
    uint8 *pCmd = STSProtocolCB.tx.cmdQueue[head].buff;
    uint16 localDeviceID = STSProtocolCB.tx.cmdQueue[head].deviceID;

    // ���ͻ�����Ϊ�գ�˵��������
    if (head == end)
    {
        return;
    }

    // ���ͺ���û��ע��ֱ�ӷ���
    if (NULL == STSProtocolCB.sendDataThrowService)
    {
        return;
    }

    // Э�����������Ҫ���͵�������
    if (!(*STSProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // ���ͻ��ζ��и���λ��
    STSProtocolCB.tx.cmdQueue[head].length = 0;
    STSProtocolCB.tx.head++;
    STSProtocolCB.tx.head %= STS_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void STS_PROTOCOL_RxFIFOProcess(STS_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (STS_PROTOCOL_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // ����ͷ��ȷ��������ʱ���������ã��˳�
        if ((pCB->rx.end + 1) % STS_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex %= STS_PROTOCOL_RX_FIFO_SIZE;
    }
    // �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
    else
    {
        // ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
        if (pCmdFrame->length >= STS_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

            // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
            pCmdFrame->length = 0; // 2016.1.5����

            // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= STS_PROTOCOL_RX_FIFO_SIZE;

            // ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ���������� ����

            // �����ж�����֡��С���ȣ�һ�����������������ٰ���4���ֽ�: ����ͷ�������֡����ݳ���
            if (pCmdFrame->length < STS_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ��������
                continue;
            }

            // ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
            if (pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] > (STS_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - STS_PROTOCOL_CMD_FRAME_LENGTH_MIN))
            {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;

                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // ����֡����У��
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] + STS_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // ����Ҫ��һ�£�˵��δ������ϣ��˳�����
                continue;
            }

            // ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
            if (!STS_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // ֹͣRXͨѶ��ʱ���
                BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

                // У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
                pCmdFrame->length = 0;

                // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // ֹͣRXͨѶ��ʱ���
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif
            // ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            STS_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// �Դ��������֡����У�飬����У����
BOOL STS_PROTOCOL_CheckSUM(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint8 cc = 0;
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

void dutProgressReport(uint32 temp)
{
    uint8 i;

    if (dut_info.appUpFlag)
    {
        dut_info.dutProgress[3] = (dut_info.currentAppSize * 100) / dut_info.appSize;
    }
    else if (dut_info.uiUpFlag)
    {
        dut_info.dutProgress[5] = (dut_info.currentUiSize * 100) / dut_info.uiSize;
    }
    else if (dut_info.configUpFlag)
    {
        // ������Ϣ����ֻ��0%��100%��
    }
    else
    {
        // �������������
        TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);
        dut_info.configUpFaile = FALSE;
        dut_info.appUpFaile = FALSE;
        dut_info.uiUpFaile = FALSE;
    }

    if (dut_info.appUpFaile) // app����ʧ��
    {
        dut_info.appUpFaile = FALSE;
        dut_info.dutProgress[3] = 102;
        dut_info.currentAppSize = 0;
        TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
        TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_LCD_REPORT);
        dut_info.appUpFlag = FALSE;
    }

    if (dut_info.configUpFaile) // config����ʧ��
    {
        dut_info.dutProgress[7] = 102;
        TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
        TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_LCD_REPORT);
        dut_info.configUpFaile = FALSE;
        dut_info.configUpFlag = FALSE;
    }

    if (dut_info.appUpSuccesss)
    {
        dut_info.dutProgress[3] = 100;
        dut_info.currentAppSize = 0;
        dut_info.appUpSuccesss = FALSE;
    }

    if (dut_info.configUpSuccesss)
    {
        dut_info.dutProgress[7] = 100;
        dut_info.configUpSuccesss = FALSE;
    }
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);         // �������ͷ
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_TOOL_DUT_PROCESS); // ���������
    STS_PROTOCOL_TxAddData(0x00);                          // ���ݳ�����ʱΪ0
    for (i = 0; i < 8; i++)
    {
        STS_PROTOCOL_TxAddData(dut_info.dutProgress[i]);
    }

    STS_PROTOCOL_TxAddFrame(); // �������ݳ��Ⱥ����У��

    // LCD��ʾ
    if (dut_info.appUpFlag)
    {
        sprintf(progressStr, "%d", dut_info.dutProgress[3]); // ��dut_info.dutProgress[3]ת��Ϊ�ַ���
        addFormattedString(2, 12, progressStr, dutUpMsgBuff); // ��ת������ַ������ݸ�addFormattedString����
        addString(2, 15, "%", dutUpMsgBuff);
    }

    if (dut_info.configUpFlag)
    {
        sprintf(progressStr, "%d", dut_info.dutProgress[7]); // ��dut_info.dutProgress[3]ת��Ϊ�ַ���
        addFormattedString(2, 12, progressStr, dutUpMsgBuff); // ��ת������ַ������ݸ�addFormattedString����
        addString(2, 15, "%", dutUpMsgBuff);
    }
}

// LCD�����ϱ�
void lcdProgressReport(uint32 temp)
{
    if ((dut_info.dutProgress[3] > 0) && (dut_info.dutProgress[3] < 100))
    {
        Vertical_Scrolling_Display(dutUpMsgBuff, 4, 2);
    }
    else if (100 == dut_info.dutProgress[3])
    {
        Vertical_Scrolling_Display(dutUpMsgBuff, 4, 3);
        TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_LCD_REPORT);
    }
}

// ���������
void setUpItem(uint8 upItem, uint8 ctrlState)
{
    dut_info.appUpFlag = FALSE;
    dut_info.uiUpFlag = FALSE;
    dut_info.configUpFlag = FALSE;

    // ���״̬
    dut_info.currentAppSize = 0;
    dut_info.currentUiSize = 0;
    if (1 == ctrlState)
    {
        if (0x02 == (upItem & 0x02)) // ����app
        {
            dut_info.appUpFlag = TRUE;
            testFlag = FALSE;
        }

        if (0x08 == (upItem & 0x08)) // ����config
        {
            dut_info.configUpFlag = TRUE;
            testFlag = TRUE;
        }
    }
}

// ���ý����ϱ���Ϣ
void setDutProgressReportInfo()
{
    dut_info.dutProgress[0] = 0;   // boot
    dut_info.dutProgress[1] = 101; // boot
    dut_info.dutProgress[2] = 1;   // app
    dut_info.dutProgress[4] = 2;   // ui
    dut_info.dutProgress[6] = 3;   // config

    if (!dut_info.appUpFlag)
    {
        dut_info.dutProgress[3] = 101;
    }
    else
    {
        dut_info.dutProgress[3] = 0;
    }

    if (!dut_info.uiUpFlag)
    {
        dut_info.dutProgress[5] = 101;
    }
    else
    {
        dut_info.dutProgress[5] = 0;
    }

    if (!dut_info.configUpFlag)
    {
        dut_info.dutProgress[7] = 101;
    }
    else
    {
        dut_info.dutProgress[7] = 0;
    }

    // �ϱ�STS
    TIMER_AddTask(TIMER_ID_SET_DUT_PROGRESS_REPORT,
                  300,
                  dutProgressReport,
                  TRUE,
                  TIMER_LOOP_FOREVER,
                  ACTION_MODE_ADD_TO_QUEUE);

    // LCD��ʾ
    TIMER_AddTask(TIMER_ID_SET_DUT_PROGRESS_LCD_REPORT,
                  8000,
                  lcdProgressReport,
                  TRUE,
                  TIMER_LOOP_FOREVER,
                  ACTION_MODE_ADD_TO_QUEUE);

}


// ����ͨѶ��ʽ���벻ͬ��״̬��
void enterState(void)
{
    switch (dut_info.dutBusType)
    {
    case 0: // ����

        switch (dut_info.test)
        {
        // ����UART����
        case TEST_TYPE_UART:
            STATE_EnterState(STATE_UART_TEST);
            break;

        // ����GND����
        case TEST_TYPE_GND:
            STATE_EnterState(STATE_UART_GND_TEST);
            break;

        // �����Ʋ���
        case TEST_TYPE_HEADLIGHT:
            STATE_EnterState(STATE_UART_HEADLIGHT_TEST);
            break;

        // �������Ʋ���
        case TEST_TYPE_LBEAM:
            STATE_EnterState(STATE_UART_LBEAM_TEST);
            break;

        // ����Զ��Ʋ���
        case TEST_TYPE_HBEAM:
            STATE_EnterState(STATE_UART_HBEAM_TEST);
            break;

        // ������ת��Ʋ���
        case TEST_TYPE_LEFT_TURN_SIGNAL:
            STATE_EnterState(STATE_UART_LEFT_TURN_SIGNAL_TEST);
            break;

        // ������ת��Ʋ���
        case TEST_TYPE_RIGHT_TURN_SIGNAL:
            STATE_EnterState(STATE_UART_RIGHT_TURN_SIGNAL_TEST);
            break;

        // �������Ų���
        case TEST_TYPE_THROTTLE:
            STATE_EnterState(STATE_UART_THROTTLE_TEST);
            break;

        // ����ɲ�Ѳ���
        case TEST_TYPE_BRAKE:
            STATE_EnterState(STATE_UART_BRAKE_TEST);
            break;

        // ����VLK����
        case TEST_TYPE_VLK:
            STATE_EnterState(STATE_UART_VLK_TEST);
            break;

        // �����ѹУ׼����
        case TEST_TYPE_VOLTAGE:
            STATE_EnterState(STATE_UART_VOLTAGE_TEST);
            break;

        // ����(С��)��Ʋ���
        case TEST_TYPE_XM_HEADLIGHT:
            STATE_EnterState(STATE_UART_XM_HEADLIGHT_TEST);
            break;

        // ����(С��)��ת��Ʋ���
        case TEST_TYPE_XM_LEFT_TURN_SIGNAL:
            STATE_EnterState(STATE_UART_XM_LEFT_TURN_SIGNAL_TEST);
            break;

        // ����(С��)��ת��Ʋ���
        case TEST_TYPE_XM_RIGHT_TURN_SIGNAL:
            STATE_EnterState(STATE_UART_XM_RIGHT_TURN_SIGNAL_TEST);
            break;

        // ����(С��)���Ų���
        case TEST_TYPE_XM_THROTTLE:
            STATE_EnterState(STATE_UART_XM_THROTTLE_TEST);
            break;

        // ����(С��)ɲ�Ѳ���
        case TEST_TYPE_XM_BRAKE:
            STATE_EnterState(STATE_UART_XM_BRAKE_TEST);
            break;

        default:
            break;
        }
        break;

    case 1: // CAN
        switch (dut_info.test)
        {
        // ����GND����
        case TEST_TYPE_GND:
            STATE_EnterState(STATE_UART_GND_TEST);
            break;

        // �����Ʋ���
        case TEST_TYPE_HEADLIGHT:
            STATE_EnterState(STATE_CAN_HEADLIGHT_TEST);
            break;

        // ������ת��Ʋ���
        case TEST_TYPE_LEFT_TURN_SIGNAL:
            STATE_EnterState(STATE_CAN_LEFT_TURN_SIGNAL_TEST);
            break;

        // ������ת��Ʋ���
        case TEST_TYPE_RIGHT_TURN_SIGNAL:
            STATE_EnterState(STATE_CAN_RIGHT_TURN_SIGNAL_TEST);
            break;

        // �������Ų���
        case TEST_TYPE_THROTTLE:
            STATE_EnterState(STATE_CAN_THROTTLE_TEST);
            break;

        // ����ɲ�Ѳ���
        case TEST_TYPE_BRAKE:
            STATE_EnterState(STATE_CAN_BRAKE_TEST);
            break;

        // ����VLK����
        case TEST_TYPE_VLK:
            STATE_EnterState(STATE_CAN_VLK_TEST);
            break;

        // �����ѹУ׼����
        case TEST_TYPE_VOLTAGE:
            STATE_EnterState(STATE_CAN_VOLTAGE_TEST);
            break;

        default:
            break;
        }
        break;

    default:
        STATE_EnterState(STATE_STANDBY);
        break;
    }

}

// ����������Ϣ���벻ͬ��״̬��
void enterUpState()
{
    if (dut_info.appUpFlag || dut_info.configUpFlag)
    {
        switch (dut_info.dutBusType)
        {
        case 0: // ����
            switch (dut_info.ID)
            {
            case DUT_TYPE_SEG:
                // ��������ܡ�����������
                STATE_EnterState(STATE_CM_UART_SEG_UPGRADE);
                break;

            case DUT_TYPE_CM:
                // ����ͨ�ò�������
                STATE_EnterState(STATE_CM_UART_BC_UPGRADE);
                break;

            case DUT_TYPE_HUAXIN:
                // ���뻪о΢������
                STATE_EnterState(STATE_HUAXIN_UART_BC_UPGRADE);
                break;

            case DUT_TYPE_KAIYANG:
                // ���뿪������
                STATE_EnterState(STATE_KAIYANG_UART_BC_UPGRADE);
                break;

            case DUT_TYPE_LIME:
                // ����LIME����
                STATE_EnterState(STATE_LIME_UART_BC_UPGRADE);
                break;

            default:
                break;
            }

            break;

        case 1: // can
            switch (dut_info.ID)
            {
            case DUT_TYPE_GB:
                // ����߱�����
                STATE_EnterState(STATE_GAOBIAO_CAN_UPGRADE);
                break;

            case DUT_TYPE_SEG:
                // ��������ܡ�����������
                STATE_EnterState(STATE_SEG_CAN_UPGRADE);
                break;

            case DUT_TYPE_HUAXIN:
                // ��о΢������
                STATE_EnterState(STATE_HUAXIN_CAN_UPGRADE);
                break;

            case DUT_TYPE_KAIYANG:
                // ���뿪������
                STATE_EnterState(STATE_KAIYANG_CAN_UPGRADE);
                break;

            case DUT_TYPE_SPARROW:
                // ����sparrow����
                STATE_EnterState(STATE_SPARROW_CAN_UPGRADE);
                break;

            default:
                // ����ͨ��can����
                STATE_EnterState(STATE_CM_CAN_UPGRADE);
                break;
            }
            break;

        default:
            break;
        }
    }
    else
    {
        STATE_EnterState(STATE_STANDBY);
    }
}

// STS����֡����������
void STS_PROTOCOL_CmdFrameProcess(STS_PROTOCOL_CB *pCB)
{
    STS_PROTOCOL_CMD cmd = STS_CMD_MAX;
    STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    BOOL writeRes;

    uint8 versionBoot[3] = {0};
    uint8 versionApp[3] = {0};
    uint8 packetSize = 0;
    uint8 addr1 = 0;
    uint8 addr2 = 0;
    uint8 addr3 = 0;
    uint8 addr4 = 0;
    uint32 writeAddr = 0;
    static uint32 allPacketSize = 0;
//    static uint32 powerPacketSize = 0;
    static uint8 allPacket[4] = {0};
    uint8 communication_Type = 0;
    uint8 baudRate_value = 0;
    uint16 can_baudRate = 0;
    uint32 uart_baudRate = 0;
    uint8 frame_Type = 0;
    uint8 test_Item = 0;
    uint16 load_voltage = 0;
    uint16 voltage_error = 0;
    uint8 load_current = 0;
    uint16 current_error = 0;
    uint16 real_load_voltage = 0;
    uint16 real_load_current = 0;
    uint32 voltageTarget = 0;
    uint16 voltageTarget_error = 0;
    uint8 upItem = 0;
    uint8 clearItem = 0;
    uint8 dutAll[50] = {0};
    uint8 dutName_i = 0;
    uint8 dataLen = 0;
    uint8 endItem = 0;
    uint8 ctrlState = 0;
    uint8 dut_info_len = 0;
    uint8 size[4] = {0};
    static uint32 currPacketNum = 0xFFFF;
    uint8_t infoLen = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    uint8 noNumber[43] = {0};  // 42�����ò���
    uint8 k = 0;
    uint8 lenght = 0;
    uint8 keyNumber = 0;
    uint8 cmdLength = 0;

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
    if (STS_PROTOCOL_CMD_HEAD != pCmdFrame->buff[STS_PROTOCOL_CMD_HEAD_INDEX])
    {
        // ɾ������֡
        pCB->rx.head++;
        pCB->rx.head %= STS_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // ����ͷ�Ϸ�������ȡ����
    cmd = (STS_PROTOCOL_CMD)pCmdFrame->buff[STS_PROTOCOL_CMD_CMD_INDEX];

    // ִ������֡
    switch (cmd)
    {
    // ������
    case STS_PROTOCOL_CMD_NULL:
        break;

    // DTA����ϱ�
    case STS_PROTOCOL_CMD_REPORT_OF_IDENTITY:
        TIMER_KillTask(TIMER_ID_REPORT_OF_IDENTITY);
        break;

    // ��վ��־У�飨������=0x01��
    case STS_PROTOCOL_CMD_CHECK_STATION_FLAG:

        // ��ȡվλ���
        stationNumber = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        offFlag = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];

        // д�봦��
        if (0x01 == (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX] & 0x01))
        {
            // д���־������
            SELECT_PROTOCOL_WriteFlag(stationNumber, offFlag);

            // д�볬ʱ��ʱ��
            TIMER_AddTask(TIMER_ID_WRITE_FLAG_TIMEOUT,
                          3000,
                          WriteFlagTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
        }

        // �Ա�У��
        if (0x02 == (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX] & 0x02))
        {
            // ��ȡ��־������
            SELECT_PROTOCOL_SendCmdAck(DUT_PROTOCOL_CMD_READ_FLAG_DATA);

            // ��ʱ��ʱ��
            TIMER_AddTask(TIMER_ID_WRITE_FLAG_TIMEOUT,
                          3000,
                          WriteFlagTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
        }

        // LCD��ʾ
        Vertical_Scrolling_Display(verTestMsgBuff, 4, 0);
        break;

    // �汾��ϢУ�����������=0x02��
    case STS_PROTOCOL_CMD_CHECK_VERSION_INFO:

        // �汾����
        dutverType = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];

        // ��Ϣ����
        infoLen = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];

        // ������Ϣ
        strcpy(dutverBuff, (const char *)&pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX]);
        dutverBuff[infoLen] = 0;

        // д��У��
        if (0x01 == (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX] & 0x01))
        {
            // ��־
            verwriteFlag = TRUE;

            // д��汾������Ϣ
            if (dut_info.dutBusType)
            {
                // CAN
                CAN_PROTOCOL_TEST_TxAddData(DUT_PROTOCOL_CMD_HEAD);
                CAN_PROTOCOL_TEST_TxAddData(DUT_PROTOCOL_CMD_WRITE_VERSION_TYPE_DATA);
                CAN_PROTOCOL_TEST_TxAddData(infoLen + 2);
                CAN_PROTOCOL_TEST_TxAddData(dutverType);
                CAN_PROTOCOL_TEST_TxAddData(infoLen);
                for (i = 0; i < infoLen; i++)
                {
                    CAN_PROTOCOL_TEST_TxAddData(pCmdFrame->buff[6 + i]);
                }
                CAN_PROTOCOL_TEST_TxAddFrame();

            }
            else
            {
                // UART
                if (dut_info.commProt)
                {
                    if (dutverType == 0)
                    {
                        dutverType = 1;
                    }
                    else if (dutverType == 1)
                    {
                        dutverType = 2;
                    }
                    else if (dutverType == 2)
                    {
                        dutverType = 3;
                    }

                    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
                    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
                    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_WRITE_VERSION_TYPE);
                    UART_PROTOCOL_XM_TxAddData(infoLen + 2);
                    UART_PROTOCOL_XM_TxAddData(dutverType);

                    if (dutverType == 3)
                    {
                        if (UART_BAUD_RATE_19200 == dut_info.uart_baudRate)
                        {
                            UART_PROTOCOL_XM_TxAddData(infoLen + 1);
                        }
                        else
                        {
                            UART_PROTOCOL_XM_TxAddData(infoLen);
                        }
                        UART_PROTOCOL_XM_TxAddData(infoLen + 1);
                    }
                    else
                    {
                        UART_PROTOCOL_XM_TxAddData(infoLen);
                    }

                    for (i = 0; i < infoLen; i++)
                    {
                        UART_PROTOCOL_XM_TxAddData(pCmdFrame->buff[6 + i]);
                    }
                    UART_PROTOCOL_XM_TxAddFrame();
                }
                else
                {
                    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_HEAD);
                    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_WRITE_VERSION_TYPE_DATA);
                    DUT_PROTOCOL_TxAddData(infoLen + 2);
                    DUT_PROTOCOL_TxAddData(dutverType);
                    DUT_PROTOCOL_TxAddData(infoLen);

                    for (i = 0; i < infoLen; i++)
                    {
                        DUT_PROTOCOL_TxAddData(pCmdFrame->buff[6 + i]);
                    }
                    DUT_PROTOCOL_TxAddFrame();
                }
            }
        }

        // ��ȡ
        if (0x02 == (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX] & 0x02))
        {
            // ��־
            verreadFlag = TRUE;

            // ��ȡ�汾������Ϣ
            SELECT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_READ_VERSION_TYPE_INFO, dutverType);
        }

        // �汾������ʱ��ʱ��
        TIMER_AddTask(TIMER_ID_OPERATE_VER_TIMEOUT,
                      3000,
                      OperateVerTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);

        // LCD��ʾ
        Vertical_Scrolling_Display(verTestMsgBuff, 4, 1);
        break;

    // ��ȡDTA�����Ϣ
    case STS_PROTOCOL_CMD_GET_SOFTWARE_INFO:                                 // 0x30,
        SPI_FLASH_ReadArray(versionBoot, SPI_FLASH_BOOT_VERSION_ADDEESS, 3); // ��ȡBOOT�汾
        SPI_FLASH_ReadArray(versionApp, SPI_FLASH_APP_VERSION_ADDEESS, 3);   // ��ȡAPP�汾
        STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);                       // �������ͷ
        STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_GET_SOFTWARE_INFO);              // ���������
        STS_PROTOCOL_TxAddData(0x00);                                        // ���ݳ�����ʱΪ0
        STS_PROTOCOL_TxAddData(versionBoot[0]);                              // ���BOOT�汾
        STS_PROTOCOL_TxAddData(versionBoot[1]);
        STS_PROTOCOL_TxAddData(versionBoot[2]);
        STS_PROTOCOL_TxAddData(versionApp[0]); // ���APP�汾
        STS_PROTOCOL_TxAddData(versionApp[1]);
        STS_PROTOCOL_TxAddData(versionApp[2]);
        STS_PROTOCOL_TxAddFrame(); // �������ݳ��Ⱥ����У��
        break;

    // DTA����������ʼ
    case STS_PROTOCOL_CMD_CLEAR_APP:
        SPI_FLASH_EraseRoom(SPI_FLASH_TOOL_APP_ADDEESS, 256 * 1024);     // 256k
        allPacketSize = 0;
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CLEAR_APP, TRUE);

        // LCD��ʾ
        Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 0);
        break;

    // DTA�����ļ�����д��
    case STS_PROTOCOL_CMD_TOOL_UP_APP:

        // ��Ƭ��flashд��STS���͵�����
        packetSize = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] - 4;
        addr1 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
        addr2 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        addr3 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
        addr4 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX];
        writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);
        writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_TOOL_APP_ADDEESS + writeAddr, packetSize);

        // ������Ϣ��STS
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_UP_APP, TRUE);
        allPacketSize++;

        if (showFlag)
        {
            // LCD��ʾ
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 1);
            showFlag = FALSE;
        }
        break;

    // DTA��������
    case STS_PROTOCOL_CMD_TOOL_UP_END:

        // ���������ɹ�Ӧ��
        // ����������־λ��־
        allPacket[0] = allPacketSize >> 24;
        allPacket[1] = allPacketSize >> 16;
        allPacket[2] = allPacketSize >> 8;
        allPacket[3] = allPacketSize;

        SPI_FLASH_EraseSector(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS); // ������־λ�洢��
        SPI_FLASH_EraseSector(SPI_FLASH_TOOL_APP_DATA_SIZE);       // ������С�洢��

        SPI_FLASH_WriteByte(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS, 0xAA);
        SPI_FLASH_WriteArray(allPacket, SPI_FLASH_TOOL_APP_DATA_SIZE, 4);

        STS_PROTOCOL_SendCmdAck(STS_PROTOCOL_CMD_TOOL_UP_END);

        /*100ms����ת��BOOT����ȡ��־λ��Ȼ���滻�滻����APP��*/
        TIMER_AddTask(TIMER_ID_TOOL_APP_TO_BOOT,
                      100,
                      IAP_JumpToAppFun,
                      IAP_GD32_FLASH_BASE,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);

        // LCD��ʾ
        Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 2);
        showFlag = TRUE;
        break;

    // ����DUTͨѶ����
    case STS_PROTOCOL_CMD_TOOL_SET_DUT_COMM_PARAM:
        cmdLength = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];
        baudRate_value = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        online_detection_cnt = 0;

        // ���ò�����
        switch (baudRate_value)
        {
        case 0x01:
            uart_baudRate = UART_BAUD_RATE_1200;
            can_baudRate = CAN_BAUD_RATE_100K;
            break;

        case 0x02:
            uart_baudRate = UART_BAUD_RATE_2400;
            can_baudRate = CAN_BAUD_RATE_125K;
            break;

        case 0x03:
            uart_baudRate = UART_BAUD_RATE_4800;
            can_baudRate = CAN_BAUD_RATE_150K;
            break;

        case 0x04:
            uart_baudRate = UART_BAUD_RATE_9600;
            can_baudRate = CAN_BAUD_RATE_200K;
            break;

        case 0x05:
            uart_baudRate = UART_BAUD_RATE_14400;
            can_baudRate = CAN_BAUD_RATE_250K;
            break;

        case 0x06:
            uart_baudRate = UART_BAUD_RATE_19200;
            can_baudRate = CAN_BAUD_RATE_300K;
            break;

        case 0x07:
            uart_baudRate = UART_BAUD_RATE_38400;
            can_baudRate = CAN_BAUD_RATE_400K;
            break;

        case 0x08:
            uart_baudRate = UART_BAUD_RATE_43000;
            can_baudRate = CAN_BAUD_RATE_500K;
            break;

        case 0x09:
            uart_baudRate = UART_BAUD_RATE_57600;
            can_baudRate = CAN_BAUD_RATE_600K;
            break;

        case 0x0A:
            uart_baudRate = UART_BAUD_RATE_76800;
            can_baudRate = CAN_BAUD_RATE_900K;
            break;

        case 0x0B:
            uart_baudRate = UART_BAUD_RATE_115200;
            break;

        case 0x0C:
            uart_baudRate = UART_BAUD_RATE_128000;
            break;

        default:
            break;
        }

        dut_info.uart_baudRate = uart_baudRate;
        dut_info.can_baudRate  = can_baudRate;

        // ����֡����
        frame_Type = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];

//            switch (frame_Type)
//            {
//                case 0x01:
//                    dut_info.can = CAN_STANDARD;
//                    break;

//                case 0x02:
//                    dut_info.can = CAN_EXTENDED;
//                    break;

//                default:
//                    break;
//            }

        // ����ͨѶЭ��  0:DTAЭ��   1��С��Э��
        if ((3 < cmdLength) && (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX]))
        {
            dut_info.commProt = TRUE;
        }
        else
        {
            dut_info.commProt = FALSE;
        }

        // ����ͨѶ��ʽ����ʼ��
        communication_Type = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];

        // ��Դʹ�ܡ��˷��߱�VLK����
        if (4 < cmdLength)
        {
            dut_info.VLKFlag = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX];
            _5VOUT_EN(dut_info.VLKFlag);
        }

        switch (communication_Type)
        {
        // 3.3V����ͨѶ
        case 0x01:
            CHANGE_Init();
            _5V_CHANGE_OFF();
            VCCB_EN_ON();
            UART_DRIVE_InitSelect(uart_baudRate);        // UART�������ʼ��
            DUT_PROTOCOL_Init();                         // UARTЭ����ʼ��
            UART_PROTOCOL_XM_Init();
            dut_info.dutBusType = FALSE;

            // LCD��ʾ
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // �ϱ�ִ�н��
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 5V����ͨѶ
        case 0x02:
            CHANGE_Init();
            _5V_CHANGE_ON();
            VCCB_EN_ON();
            UART_DRIVE_InitSelect(uart_baudRate);        // UART�������ʼ��
            DUT_PROTOCOL_Init();                         // UARTЭ����ʼ��
            UART_PROTOCOL_XM_Init();
            dut_info.dutBusType = FALSE;

            // LCD��ʾ
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // �ϱ�ִ�н��
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // CANͨѶ
        case 0x03:
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate);       // CAN�������ʼ��
            dut_info.dutBusType = TRUE;

            // LCD��ʾ
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // �ϱ�ִ�н��
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        default:
            break;
        }
        dut_info.passThroughControl = FALSE;
        break;

    case STS_PROTOCOL_CMD_MULTIPLE_TEST:     // ���յ��������ָ�� 0xDF
        test_Item = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];  // ��ȡ��������
        testFlag = TRUE;

        // ��ȡĿ���ѹֵ
        voltageTarget = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        voltageTarget = voltageTarget << 8;
        voltageTarget = voltageTarget | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
        voltageTarget = voltageTarget << 8;
        voltageTarget = voltageTarget | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX];
        voltageTarget = voltageTarget << 8;
        voltageTarget = voltageTarget | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX];

        // ��ȡ��ѹ���ֵ
        voltageTarget_error = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA6_INDEX];
        voltageTarget_error = voltageTarget_error << 8;
        voltageTarget_error = voltageTarget_error | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA7_INDEX];

        // ����õ�ѹ��Χ
        dut_info.voltageMax = voltageTarget + voltageTarget_error;
        dut_info.voltageMin = voltageTarget - voltageTarget_error;

        // ��Ʋ���
        if (0x01 == (test_Item & 0x01))
        {
            if (dut_info.commProt)
            {
                dut_info.test = TEST_TYPE_XM_HEADLIGHT;
                enterState();
            }
            else
            {
                dut_info.test = TEST_TYPE_HEADLIGHT;
                enterState();
            }

        }

        // ��ת��Ʋ���
        if (0x02 == (test_Item & 0x02))
        {
            if (dut_info.commProt)
            {
                dut_info.test = TEST_TYPE_XM_LEFT_TURN_SIGNAL;
                enterState();
            }
            else
            {
                dut_info.test = TEST_TYPE_LEFT_TURN_SIGNAL;
                enterState();
            }
        }

        // ��ת��Ʋ���
        if (0x03 == (test_Item & 0x03))
        {
            if (dut_info.commProt)
            {
                dut_info.test = TEST_TYPE_XM_RIGHT_TURN_SIGNAL;
                enterState();
            }
            else
            {
                dut_info.test = TEST_TYPE_RIGHT_TURN_SIGNAL;
                enterState();
            }            ;
        }

        // ����Ʋ���
        if (0x04 == (test_Item & 0x04))
        {
            dut_info.test = TEST_TYPE_LBEAM;
            enterState();
        }

        // Զ��Ʋ���
        if (0x05 == (test_Item & 0x05))
        {
            dut_info.test = TEST_TYPE_HBEAM;
            enterState();
        }

        // ���Ų���
        if (0x06 == (test_Item & 0x06))
        {
            if (dut_info.commProt)
            {
                dut_info.test = TEST_TYPE_XM_THROTTLE;
                enterState();
            }
            else
            {
                dut_info.test = TEST_TYPE_THROTTLE;
                enterState();
            }
        }

        // ɲ�Ѳ���
        if (0x07 == (test_Item & 0x07))
        {
            if (dut_info.commProt)
            {
                dut_info.test = TEST_TYPE_XM_BRAKE;
                enterState();
            }
            else
            {
                dut_info.test = TEST_TYPE_BRAKE;
                enterState();
            }
        }

        // VLK����
        if (0x08 == (test_Item & 0x08))
        {
            dut_info.test = TEST_TYPE_VLK;
            enterState();

            // LCD��ʾ
            Vertical_Scrolling_Display(volTestMsgBuff, 4, 2);
        }

        // ���ӱ��ٲ���
        if (0x09 == (test_Item & 0x09))
        {
            dut_info.test = TEST_TYPE_DERAILLEUR;
            enterState();

            // LCD��ʾ
            Vertical_Scrolling_Display(deTestMsgBuff, 4, 0);
        }
        break;

    // GND����
    case STS_PROTOCOL_CMD_GND_TEST: // 0x04

        // ����ɲ�Ѻ͵��ӱ���GND������
        if (5 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_BRAKE_DERAILLEUR;
            enterState();
        }

        // ���õ��ӱ���GND������
        if (4 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_DERAILLEUR;
            enterState();
        }

        // ����ɲ�ѡ�����GND������
        if (3 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_THROTTLE_BRAKE;
            enterState();
        }

        // ����ɲ��GND������
        if (2 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_BRAKE;
            enterState();
        }

        // ��������GND������
        if (1 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_THROTTLE;
            enterState();
        }


        break;

    // USB�����ԣ��������ã�
    case STS_PROTOCOL_CMD_USB_CHARGE_TEST: // 0x14

        // LCD��ʾ
        Vertical_Scrolling_Display(showTestMsgBuff, 4, 3);

        // ��ȡ���ص�ѹ
        load_voltage = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
        load_voltage = load_voltage << 8;
        load_voltage = load_voltage | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];

        // ��ȡ��ѹ���
        voltage_error = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
        voltage_error = voltage_error << 8;
        voltage_error = voltage_error | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX];

        // ��ȡ���ص���
        load_current = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX];

        // ��ȡ�������
        current_error = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA6_INDEX];
        current_error = current_error << 8;
        current_error = current_error | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA7_INDEX];

        // ����USB����
        // 500MA
        if (0x00 == load_current)
        {
            //����USB����
            DUT_USB_EN1_OFF();
            DUT_USB_EN3_OFF();
            DUT_USB_EN2_ON();
            Delayms(400);

//                // ͨ��mcu��ȡUSB����ѹ
//                real_load_voltage = USB_ADC_Read();
            real_load_voltage = USB_ADC_Read();

            // ʵ�ʲ�õĸ��ص�ѹ���������ķ�Χ��
            if (((load_voltage - voltage_error) < real_load_voltage) && (real_load_voltage < (load_voltage + voltage_error)))
            {
//                    // ͨ��mcu��ȡUSB������
                real_load_current = USB_CURRENT_Read();
//real_load_current = adcCB.usbcurrent.voltage;

                // ʵ�ʲ�õĸ��ص������������ķ�Χ��
                if (((500 - current_error) < real_load_current) && (real_load_current < (500 + current_error)))
                {
                    // �ϱ�STS����ͨ��
                    Clear_All_Lines();
                    Display_Centered(0, "USB Charging ");
                    Display_Centered(1, "Test Pass");
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, TRUE);
                }
                else
                {
                    // ����ʧ��
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, FALSE);
                }
            }
            else
            {
                // ����ʧ��
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, FALSE);
            }
        }

        // 1A
        if (0x01 == (load_current & 0x01))    // 1A
        {
            //����USB����
            DUT_USB_EN1_ON();
            DUT_USB_EN2_OFF();
            DUT_USB_EN3_OFF();
            Delayms(400);
            
            // ͨ��mcu��ȡUSB����ѹ
            real_load_voltage = USB_ADC_Read();

            // ʵ�ʲ�õĸ��ص�ѹ���������ķ�Χ��
            if (((load_voltage - voltage_error) < real_load_voltage) && (real_load_voltage < (load_voltage + voltage_error)))
            {
                // ͨ��mcu��ȡUSB������
                real_load_current = USB_CURRENT_Read();

                // ʵ�ʲ�õĸ��ص������������ķ�Χ��
                if (((1000 - current_error) < real_load_current) && (real_load_current < (1000 + current_error)))
                {
                    Clear_All_Lines();
                    Display_Centered(0, "USB Charging ");
                    Display_Centered(1, "Test Pass");

                    // �ϱ�STS����ͨ��
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, TRUE);
                }
                else
                {
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, FALSE);
                }
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, FALSE);
            }

        }

        // 2A
        if (0x02 == (load_current & 0x02))    // 2A
        {
            //����USB����
            DUT_USB_EN1_ON();
            DUT_USB_EN2_ON();
            DUT_USB_EN3_ON();
            Delayms(400);
            // ͨ��mcu��ȡUSB����ѹ
            real_load_voltage = USB_ADC_Read();

            // ʵ�ʲ�õĸ��ص�ѹ���������ķ�Χ��
            if (((load_voltage - voltage_error) < real_load_voltage) && (real_load_voltage < (load_voltage + voltage_error)))
            {

                // ͨ��mcu��ȡUSB������
                real_load_current = USB_CURRENT_Read();

                // ʵ�ʲ�õĸ��ص������������ķ�Χ��
                if (((2000 - current_error) < real_load_current) && (real_load_current < (2000 + current_error)))
                {
                    Clear_All_Lines();
                    Display_Centered(0, "USB Charging ");
                    Display_Centered(1, "Test Pass");

                    // �ϱ�STS����ͨ��
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, TRUE);
                }
                else
                {
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, FALSE);
                }
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, FALSE);
            }
        }
        break;

    // ��ѹУ׼���༶У׼��
    case STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION: // 0xDD

        // LCD��ʾ
        Vertical_Scrolling_Display(volTestMsgBuff, 3, 0);

        // ��ȡУ׼����
        dut_info.cal_number = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];

        // һ��У׼
        if (0x01 == dut_info.cal_number)
        {
            calibration.data[0] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
            dut_info.test = TEST_TYPE_VOLTAGE;
            enterState();
            dut_info.cali_cnt = 1;
        }

        // ����У׼
        if (0x02 == dut_info.cal_number)
        {
            calibration.data[0] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
            calibration.data[1] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_VOLTAGE;
            enterState();
            dut_info.cali_cnt = 2;
        }

        // ����У׼
        if (0x03 == dut_info.cal_number)
        {
            calibration.data[0] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
            calibration.data[1] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            calibration.data[2] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
            dut_info.test = TEST_TYPE_VOLTAGE;
            enterState();
            dut_info.cali_cnt = 3;
        }
        break;

    // �����Ǳ����ѹ
    case STS_PROTOCOL_CMD_ADJUST_DUT_VOLTAGE:      // 0x16
        dut_info.adjustState = TRUE;

        // ֪ͨ��Դ�����DUT�����ѹ
        POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX]);
        break;

    // д��־������
    case STS_PROTOCOL_CMD_WRITE_FLAG_DATA:

        // �ر�VLK����
        VLK_PW_EN_OFF();
        UART_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
        Clear_All_Lines();
        Display_Centered(0, "VLK CLOSE");
        break;

    // UART����
    case STS_PROTOCOL_CMD_UART_TEST:      // 0x19 - UART��������
        dut_info.test = TEST_TYPE_UART;
        enterState();

        // LCD��ʾ
        Vertical_Scrolling_Display(volTestMsgBuff, 3, 1);
        break;

    // Flash У�����
    case STS_PROTOCOL_CMD_FLASH_CHECK_TEST:
        TIMER_ChangeTime(TIMER_ID_ONLINE_DETECT, 7000);

        if (dut_info.dutBusType)
        {
            CAN_TEST_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
        }
        else
        {
            UART_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
        }
        break;

    // �Ǳ��Դ����
    case STS_PROTOCOL_CMD_CONTROL_DUT_POWER:
        dut_info.powerOnFlag = TRUE;

        // ������Դ
        if (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
        }

        // �رյ�Դ
        if (!pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            // �ر�vlk����
//                VLK_PW_EN_OFF();
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
        }
        break;

    // ��ȡWAKE��ѹ
    case STS_PROTOCOL_CMD_OBTAIN_WAKE_UP_VOL:
        STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
        STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_OBTAIN_WAKE_UP_VOL);
        STS_PROTOCOL_TxAddData(0x02);
        STS_PROTOCOL_TxAddData(KEY_ADC_Read() >> 8);
        STS_PROTOCOL_TxAddData(KEY_ADC_Read() & 0xFF);
        STS_PROTOCOL_TxAddFrame();
        break;

    // �ʶ���/�������ʾ����
    case STS_PROTOCOL_CMD_DISPLAY_TEST:

        if (dut_info.commProt)
        {
            // ֻȡ����͸��
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_LED);  // �������ʾָ��
            UART_PROTOCOL_XM_TxAddData(0);                             // ���ݳ���

            for (i = 0; i < pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] - 1; i++) // ��ȥ����ģʽһ���ֽ�
            {
                UART_PROTOCOL_XM_TxAddData(pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX + 1 + i]);
            }

            UART_PROTOCOL_XM_TxAddFrame();

//                UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
//                UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
//                UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_LED);  // �������ʾָ��

//                for(i = 0 ; i< 14 ;i++)
//                {
//                    UART_PROTOCOL_XM_TxAddData();
//                }
//                UART_PROTOCOL_XM_TxAddFrame();
        }
        else
        {
            if (dut_info.dutBusType)
            {
                CAN_TEST_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
                testFlag = TRUE;
            }
            else
            {
                UART_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
            }
        }
        break;

    // ���������Ʋ���
    case STS_PROTOCOL_CMD_BUZZER_TEST:

        if (dut_info.commProt)
        {
            UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_TEST_BEEP);
        }
        else
        {
            if (dut_info.dutBusType)
            {
                CAN_TEST_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
                testFlag = TRUE;
            }
            else
            {
                UART_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
            }
        }
        break;

    // ��������
    case STS_PROTOCOL_CMD_BLE_TEST:

        // ����4lite
        if (dut_info.commProt)
        {
            for (i = 0 ; i < pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX]; i++)
            {
                verBle[i] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX + i];
            }

            // �Ȳ�ѯ�����̼��汾
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_BLE_VERSION);  // ��ȡOOB����ָ��
            UART_PROTOCOL_XM_TxAddData(0x00);                                   // ���ݳ���
            UART_PROTOCOL_XM_TxAddFrame();
        }
        else
        {
            if (dut_info.dutBusType)
            {
                CAN_TEST_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
                testFlag = TRUE;
            }
            else
            {
                UART_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
            }
        }
        break;

    // ��������
    case STS_PROTOCOL_CMD_KEY_TEST:

        dut_info.buttonSimulationFlag = FALSE;

        // ����4lite
        if (dut_info.commProt)
        {
            // ��������
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_KEY);  // ��ȡOOB����ָ��
            UART_PROTOCOL_XM_TxAddData(0x00);         // ���ݳ���
            UART_PROTOCOL_XM_TxAddFrame();
        }
        else
        {
            if (dut_info.dutBusType)
            {
                CAN_TEST_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
                testFlag = TRUE;
            }
            else
            {
                UART_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
            }
        }
        break;

    // ģ�ⰴ������
    case STS_PROTOCOL_CMD_SIMULATION_KEY_TEST:

        dut_info.buttonSimulationFlag = TRUE;

        // ��������
        // ����4lite
        if (dut_info.commProt)
        {
            // ��������
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_KEY);  // ��ȡOOB����ָ��
            UART_PROTOCOL_XM_TxAddData(0x00);         // ���ݳ���
            UART_PROTOCOL_XM_TxAddFrame();
        }
        else
        {
            if (dut_info.dutBusType)
            {
                CAN_PROTOCOL_TEST_SendCmdAck(CAN_PROTOCOL_TEST_CMD_KEY_TEST);
                testFlag = TRUE;
            }
            else
            {
                DUT_PROTOCOL_SendCmdAck(DUT_PROTOCOL_CMD_KEY_TEST);
            }
        }

        keyNumber = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
        keyValue  = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];

        // ģ�ⰴ������
        switch (keyNumber)
        {
        case KEY_NUMBER_ZERO:
            KEY_TEST_EN0_ON();
            Delayms(100);
            KEY_TEST_EN0_OFF();
            break;

        case KEY_NUMBER_ONE:
            KEY_TEST_EN1_ON();
            Delayms(100);
            KEY_TEST_EN1_OFF();
            break;

        case KEY_NUMBER_TWO:
            KEY_TEST_EN2_ON();
            Delayms(100);
            KEY_TEST_EN2_OFF();
            break;

        case KEY_NUMBER_THREE:
            KEY_TEST_EN3_ON();
            Delayms(100);
            KEY_TEST_EN3_OFF();
            break;

        case KEY_NUMBER_FOUR:
            KEY_TEST_EN4_ON();
            Delayms(100);
            KEY_TEST_EN4_OFF();
            break;

        case KEY_NUMBER_FIVE:
            KEY_TEST_EN5_ON();
            Delayms(100);
            KEY_TEST_EN5_OFF();
            break;

        case KEY_NUMBER_SIX:
            KEY_TEST_EN6_ON();
            Delayms(100);
            KEY_TEST_EN6_OFF();
            break;

        case KEY_NUMBER_SEVEN:
            KEY_TEST_EN7_ON();
            Delayms(100);
            KEY_TEST_EN7_OFF();
            break;

        default:
            // ������Ч�İ������
            break;
        }
        break;

    // ��ȡDUT�������
    case STS_PROTOCOL_CMD_OBTAIN_DUT_CURRENT:
        POWER_PROTOCOL_SendCmdAck(POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_CURRENT);
        break;

//        // ����DUT��������
//        case STS_PROTOCOL_CMD_TOOL_DUT_UP:

//            // �ر����߼��
//            TIMER_KillTask(TIMER_ID_ONLINE_DETECT);

//            upItem = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
//            ctrlState = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];

//            // ����Ҫ��������
//            setUpItem(upItem, ctrlState);

//            // ���ý����ϱ���Ϣ
//            setDutProgressReportInfo();

//            // ����������Ϣ�����벻ͬ��״̬��
//            enterUpState();

//            // Ӧ��
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_DUT_UP, TRUE);

//            // LCD��ʾ
//            Vertical_Scrolling_Display(dutUpMsgBuff, 4, 1);
//            break;

//        // 0x41 - DUT���������ϱ�
//        case STS_PROTOCOL_CMD_TOOL_DUT_PROCESS:
//            break;

//        // 0x42 - �������������
//        case STS_PROTOCOL_CMD_TOOL_CLEAR_BUFF:
//            clearItem = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
//            if (0x01 == (clearItem & 0x01))
//            {
//                /*����Ƭ���dut_boot��*/
//                dut_info.bootSize = 0;
//                SPI_FLASH_EraseRoom(SPI_FLASH_DUT_BOOT_ADDEESS, 128 * 1024); // 128k
//            }
//            if (0x02 == (clearItem & 0x02))
//            {
//                /*����Ƭ���dut_app��*/
//                dut_info.appSize = 0;
//                SPI_FLASH_EraseRoom(SPI_FLASH_DUT_APP_ADDEESS, 1024 * 1024); // 1M
//            }
//            if (0x08 == (clearItem & 0x08))
//            {
//                /*����Ƭ���dut_config��*/
//                SPI_FLASH_EraseSector(SPI_FLASH_DUT_CONFIG_ADDEESS); // 4k
//            }
//            SPI_FLASH_EraseSector(SPI_FLASH_DUT_INFO); // ����dut������Ϣ
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_CLEAR_BUFF, TRUE);
//            break;

//        // 0x43 - DTAд��DUT������Ϣ
//        case STS_PROTOCOL_CMD_TOOL_SET_DUT_INFO:
//            dut_info_len = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] - 3;

//            for (dutName_i = 0; dutName_i < dut_info_len; dutName_i++)
//            {
//                dutAll[dutName_i] = pCmdFrame->buff[dutName_i + 3 + 3];
//            }
//            dutAll[20] = dut_info_len;                                  // ����
//            //        dutAll[21] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX]; // ��ѹ
//            //        dutAll[22] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX]; // ͨѶ����
//            dutAll[23] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX]; // bootType
//            dutAll[24] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX]; // appType
//            dutAll[25] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX]; // uiType
//            dutAll[26] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX]; // configType

//            /* ��������Ϣд�뵽Ƭ��flash */
//            SPI_FLASH_EraseSector(SPI_FLASH_DUT_INFO);
//            writeRes = SPI_FLASH_WriteWithCheck(dutAll, SPI_FLASH_DUT_INFO, 30);

//            if (writeRes)
//            {
//                DutInfoUpdata(); // ����dutinfo����
//            }
//            /* ������Ϣ��STS */
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_DUT_INFO, writeRes);
//            break;

//        case STS_PROTOCOL_CMD_TOOL_GET_DUT_INFO: // 0x44 - ��ȡDTA��¼��DUT������Ϣ
//            SPI_FLASH_ReadArray(dutAll, SPI_FLASH_DUT_INFO, 30);
//            if (0xFF == dutAll[20]) // ����Ϊ��
//            {
//                dutAll[20] = 10;
//            }
//            STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);           // �������ͷ
//            STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_TOOL_GET_DUT_INFO);  // ���������
//            STS_PROTOCOL_TxAddData(24);                              // ���ݳ�����ʱΪ0
//            STS_PROTOCOL_TxAddData(dutAll[23]);                      // bootType
//            STS_PROTOCOL_TxAddData(dutAll[24]);                      // appType
//            STS_PROTOCOL_TxAddData(dutAll[25]);                      // uiType
//            STS_PROTOCOL_TxAddData(dutAll[26]);                      // configType
//            for (dutName_i = 0; dutName_i < dutAll[20]; dutName_i++) // ��ӻ�����Ϣ
//            {
//                STS_PROTOCOL_TxAddData(dutAll[dutName_i]);
//            }
//            STS_PROTOCOL_TxAddFrame(); // �������ݳ��Ⱥ����У��
//            break;

//        case STS_PROTOCOL_CMD_UP_CONFIG: // 0x50 - д��DUT���ò���
//            SPI_FLASH_EraseSector(SPI_FLASH_DUT_CONFIG_ADDEESS); // 4k

//            // ��ȡ������Ϣ��flash����Ȼ��д��DUT
//            dataLen = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];
//            writeRes = SPI_FLASH_WriteWithCheck(&(pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX]), SPI_FLASH_DUT_CONFIG_ADDEESS, dataLen + 1);

//            // ������Ϣ��STS
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UP_CONFIG, writeRes);
//            break;

//        case STS_PROTOCOL_CMD_UP_BOOT:   // 0x51 - BOOT����д��

//            // ��ȡBOOT��flash����Ȼ��д��DUT
//            packetSize = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] - 4;
//            addr1 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
//            addr2 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
//            addr3 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
//            addr4 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX];
//            writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);

//            if (currPacketNum != writeAddr)
//            {
//                dut_info.bootSize++;
//                currPacketNum = writeAddr;
//            }

//            writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_DUT_BOOT_ADDEESS + writeAddr, packetSize);

//            // ������Ϣ��STS
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UP_BOOT, writeRes);
//            break;

//        case STS_PROTOCOL_CMD_UP_APP:    // 0x52 - APP����д��
//            packetSize = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] - 4;
//            addr1 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
//            addr2 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
//            addr3 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
//            addr4 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX];
//            writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);

//            if (currPacketNum != writeAddr)
//            {
//                dut_info.appSize++;
//                currPacketNum = writeAddr;
//            }

//            writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_DUT_APP_ADDEESS + writeAddr, packetSize);
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UP_APP, writeRes);

//            if (dutFlag)
//            {
//                // LCD��ʾ
//                Vertical_Scrolling_Display(dutUpMsgBuff, 4, 0);
//                dutFlag = FALSE;
//            }
//            break;

//        case STS_PROTOCOL_CMD_UP_END:    // 0x53 - ���������
//            endItem = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];

//            switch (endItem)
//            {
//                case 0:
//                    SPI_FLASH_EraseSector(SPI_FLASH_DUT_BOOT_SIZE_ADDEESS);
//                    size[0] = dut_info.bootSize >> 24;
//                    size[1] = dut_info.bootSize >> 16;
//                    size[2] = dut_info.bootSize >> 8;
//                    size[3] = dut_info.bootSize;
//                    SPI_FLASH_WriteArray(size, SPI_FLASH_DUT_BOOT_SIZE_ADDEESS, 4);
//                    break;
//                case 1:
//                    SPI_FLASH_EraseSector(SPI_FLASH_DUT_APP_SIZE_ADDEESS);
//                    size[0] = dut_info.appSize >> 24;
//                    size[1] = dut_info.appSize >> 16;
//                    size[2] = dut_info.appSize >> 8;
//                    size[3] = dut_info.appSize;
//                    SPI_FLASH_WriteArray(size, SPI_FLASH_DUT_APP_SIZE_ADDEESS, 4);
//                    break;
//                case 2:
//                    SPI_FLASH_EraseSector(SPI_FLASH_DUT_UI_SIZE_ADDEESS);
//                    size[0] = dut_info.uiSize >> 24;
//                    size[1] = dut_info.uiSize >> 16;
//                    size[2] = dut_info.uiSize >> 8;
//                    size[3] = dut_info.uiSize;
//                    SPI_FLASH_WriteArray(size, SPI_FLASH_DUT_UI_SIZE_ADDEESS, 4);
//                    break;
//            }
//            currPacketNum = 0xFFFF;

//            // ������Ϣ��STS
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UP_END, endItem);

//            // LCD��ʾ
//            Vertical_Scrolling_Display(dutUpMsgBuff, 4, 3);
//            dutFlag = TRUE;
//            break;

//        // ���ò�д����
//        case STS_PROTOCOL_CMD_SET_NOT_WRITTEN:

//            // ��ȡ��������
//            memset(configs, 0, sizeof(configs));
//            memset(resultArray, 0, sizeof(resultArray));
//            memset(verifiedBuff, 0, sizeof(verifiedBuff));
//            memset(paramLength, 0, sizeof(paramLength));
//            memset(paramNumber, 0, sizeof(paramNumber));
//            resultIndex = 0;
//            verifiedIndex = 0;

//            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 170);

//            // ��ȡ��д�������
//            noLenght = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];
//            for (i = 0; i < noLenght ; i++)
//            {
//                noNumber[i] = pCmdFrame->buff[3 + i];
//            }

//            // ���ò�д�����ȡ��д�������
//            if (noLenght)
//            {
//                // �Բ������д�����ȡ��������
//                for (i = 0; i < configs[0] ;)
//                {
//                    paramLength[i] = configs[i + 1];   // ��������
//                    paramNumber[i] = configs[i + 2];   // �������

//                    for (k = 0; k < noLenght; k++)
//                    {
//                        if (noNumber[k] != configs[i + 2])
//                        {
//                            // ƴ�����ò���
//                            for (j = 0 ; j < paramLength[i]; j++)
//                            {
//                                paramContent[paramNumber[i]][j] = configs[i + 2 + j + 1];

//                                // ��������ӵ��������
//                                resultArray[resultIndex++] = paramContent[paramNumber[i]][j];
//                            }
//                        }
//                        i = i + 1 + paramLength[i] + 1;
//                    }
//                }

//                // ��ȡ��У��buff��ȥ��ϵͳʱ��
//                for (k = 0; k < noLenght; k++)
//                {
//                    // ��д��ϵͳ��ʱ��
//                    if (noNumber[k] != 3)
//                    {
//                        for (i = 0; i < resultIndex ; i++)
//                        {
//                            if (i == 2)
//                            {
//                                i = i + 5;
//                            }
//                            verifiedBuff[verifiedIndex++] = resultArray[i];
//                        }
//                    }

//                    // ����д��ϵͳʱ��
//                    if (noNumber[k] == 3)
//                    {
//                        for (i = 0; i < resultIndex ; i++)
//                        {
//                            verifiedBuff[verifiedIndex++] = resultArray[i];
//                        }
//                    }
//                }
//            }

//            // ������
//            if (!noLenght)
//            {
//                for (i = 0; i < configs[0] ;)
//                {
//                    paramLength[i] = configs[i + 1];
//                    paramNumber[i] = configs[i + 2];
//                    for (j = 0 ; j < paramLength[i]; j++)
//                    {
//                        paramContent[paramNumber[i]][j] = configs[i + 2 + j + 1];

//                        // ��������ӵ��������
//                        resultArray[resultIndex++] = paramContent[paramNumber[i]][j];
//                    }
//                    i = i + 1 + paramLength[i] + 1;
//                }

//                // ��ȡ��У��buff��ȥ��ϵͳʱ��
//                for (i = 0; i < resultIndex ; i++)
//                {
//                    if (i == 2)
//                    {
//                        i = i + 5;
//                    }
//                    verifiedBuff[verifiedIndex++] = resultArray[i];
//                }
//            }

//            // ������Ϣ��STS
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_SET_NOT_WRITTEN, TRUE);
//            break;

    // ��ȡPOWER�����Ϣ
    case STS_PROTOCOL_CMD_POWER_GET_SOFTWARE_INFO:
        POWER_PROTOCOL_SendCmdAck(POWER_PROTOCOL_CMD_GET_SOFTWARE_INFO);
        break;

    // POWER-APP2���ݲ���
    case STS_PROTOCOL_CMD_POWER_UP_APP_ERASE:
        powerPacketSize = 0;
        POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
        POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
        POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_ECO_APP2_ERASE);
        POWER_PROTOCOL_TxAddData(0);
        POWER_PROTOCOL_TxAddFrame();
        break;

    // POWER-APP2����д��
    case STS_PROTOCOL_CMD_POWER_UP_APP_WRITE:
        lenght = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];
        addr1 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
        addr2 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        addr3 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
        addr4 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX];
        writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);

        if (currPacketNum != writeAddr)
        {
            powerPacketSize++;
            currPacketNum = writeAddr;
        }

        POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
        POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
        POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_ECO_APP2_WRITE);
        POWER_PROTOCOL_TxAddData(lenght);

        for (i = 0; i < lenght; i++)
        {
            POWER_PROTOCOL_TxAddData(pCmdFrame->buff[3 + i]);
        }

        POWER_PROTOCOL_TxAddFrame();
        break;

    // POWER-APP����д�����
    case STS_PROTOCOL_CMD_POWER_UP_APP_WRITE_FINISH:
        POWER_PROTOCOL_SendCmdAck(POWER_PROTOCOL_CMD_ECO_APP_WRITE_FINISH);
        break;

    // ����͸��DUTͨѶ����
    case STS_PROTOCOL_CMD_TOOL_SET_TRAN_DUT_COMM_PARAM:
        cmdLength = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];
        baudRate_value = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        online_detection_cnt = 0;

        // ���ò�����
        switch (baudRate_value)
        {
        case 0x01:
            uart_baudRate = UART_BAUD_RATE_1200;
            can_baudRate = CAN_BAUD_RATE_100K;
            break;

        case 0x02:
            uart_baudRate = UART_BAUD_RATE_2400;
            can_baudRate = CAN_BAUD_RATE_125K;
            break;

        case 0x03:
            uart_baudRate = UART_BAUD_RATE_4800;
            can_baudRate = CAN_BAUD_RATE_150K;
            break;

        case 0x04:
            uart_baudRate = UART_BAUD_RATE_9600;
            can_baudRate = CAN_BAUD_RATE_200K;
            break;

        case 0x05:
            uart_baudRate = UART_BAUD_RATE_14400;
            can_baudRate = CAN_BAUD_RATE_250K;
            break;

        case 0x06:
            uart_baudRate = UART_BAUD_RATE_19200;
            can_baudRate = CAN_BAUD_RATE_300K;
            break;

        case 0x07:
            uart_baudRate = UART_BAUD_RATE_38400;
            can_baudRate = CAN_BAUD_RATE_400K;
            break;

        case 0x08:
            uart_baudRate = UART_BAUD_RATE_43000;
            can_baudRate = CAN_BAUD_RATE_500K;
            break;

        case 0x09:
            uart_baudRate = UART_BAUD_RATE_57600;
            can_baudRate = CAN_BAUD_RATE_600K;
            break;

        case 0x0A:
            uart_baudRate = UART_BAUD_RATE_76800;
            can_baudRate = CAN_BAUD_RATE_900K;
            break;

        case 0x0B:
            uart_baudRate = UART_BAUD_RATE_115200;
            break;

        case 0x0C:
            uart_baudRate = UART_BAUD_RATE_128000;
            break;

        default:
            break;
        }

        dut_info.uart_baudRate = uart_baudRate;
        dut_info.can_baudRate  = can_baudRate;

        // ����֡����
        frame_Type = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];

//            switch (frame_Type)
//            {
//                case 0x01:
//                    dut_info.can = CAN_STANDARD;
//                    break;

//                case 0x02:
//                    dut_info.can = CAN_EXTENDED;
//                    break;

//                default:
//                    break;
//            }

        // ����ͨѶЭ��  0:DTAЭ��   1��С��Э��
//           if((3 < cmdLength)&&(pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX]))
//           {
//               dut_info.commProt = TRUE;
//           }
//           else
//           {
//               dut_info.commProt = FALSE;
//           }

        // ����ͨѶ��ʽ����ʼ��
        communication_Type = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];

//            // ��Դʹ�ܡ��˷��߱�VLK����
//           if(4 < cmdLength)
//           {
//                dut_info.VLKFlag = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX];
//                _5VOUT_EN(dut_info.VLKFlag);
//           }

        switch (communication_Type)
        {
        // 3.3V����ͨѶ
        case 0x01:
            CHANGE_Init();
            _5V_CHANGE_OFF();
            VCCB_EN_ON();
            UART_DRIVE_InitSelect(uart_baudRate);        // UART�������ʼ��
            UART_PROTOCOL_Init();        // 55����Э��
            UART_PROTOCOL4_Init();
            dut_info.dutBusType = FALSE;

            // LCD��ʾ
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // �ϱ�ִ�н��
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_TRAN_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 5V����ͨѶ
        case 0x02:
            CHANGE_Init();
            _5V_CHANGE_ON();
            VCCB_EN_ON();
            UART_DRIVE_InitSelect(uart_baudRate);        // UART�������ʼ��
            dut_info.dutBusType = FALSE;
            UART_PROTOCOL4_Init();

            // LCD��ʾ
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // �ϱ�ִ�н��
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_TRAN_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // CANͨѶ
        case 0x03:
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate);       // CAN�������ʼ��

            CAN_PROTOCOL1_Init();  // ͨ�����ó�ʼ��

            dut_info.dutBusType = TRUE;

            // LCD��ʾ
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // �ϱ�ִ�н��
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_TRAN_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        default:
            break;
        }
        break;

    // ͸��DUTָ�����
    case STS_PROTOCOL_CMD_TRAN_DUT_INSTRUCTION_CONTROL:
        if (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.passThroughControl = TRUE;
        }
        else
        {
            dut_info.passThroughControl = FALSE;
        }

        // �ϱ�ִ�н��
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TRAN_DUT_INSTRUCTION_CONTROL, TRUE);
        break;

    default:
        if (dut_info.dutBusType)
        {
            CAN_TEST_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
            testFlag = TRUE;
        }
        else
        {
            UART_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
        }

        // LCD��ʾ
        Vertical_Scrolling_Display(volTestMsgBuff, 4, 3);
        break;
    }

    // ɾ������֡
    pCB->rx.head++;
    pCB->rx.head %= STS_PROTOCOL_RX_QUEUE_SIZE;
}


// RXͨѶ��ʱ����-����
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void BLE_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
    STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    pCmdFrame = &STSProtocolCB.rx.cmdQueue[STSProtocolCB.rx.end];

    // ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
    pCmdFrame->length = 0; // 2016.1.6����
    // ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
    STSProtocolCB.rxFIFO.head++;
    STSProtocolCB.rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
    STSProtocolCB.rxFIFO.currentProcessIndex = STSProtocolCB.rxFIFO.head;
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
void STS_PROTOCOL_SendCmdAck(uint8 ackCmd)
{
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
    STS_PROTOCOL_TxAddData(ackCmd);
    STS_PROTOCOL_TxAddData(0x00);
    STS_PROTOCOL_TxAddFrame();
}

// ��������ظ�����һ������
void STS_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
    STS_PROTOCOL_TxAddData(ackCmd);
    STS_PROTOCOL_TxAddData(0x01);
    STS_PROTOCOL_TxAddData(ackParam);
    STS_PROTOCOL_TxAddFrame();
}

// ��������ظ�������������
void STS_PROTOCOL_SendCmdParamTwoAck(uint8 ackCmd, uint8 ackParam, uint8 twoParam)
{
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
    STS_PROTOCOL_TxAddData(ackCmd);
    STS_PROTOCOL_TxAddData(0x02);
    STS_PROTOCOL_TxAddData(ackParam);
    STS_PROTOCOL_TxAddData(twoParam);
    STS_PROTOCOL_TxAddFrame();
}

// DUTͨѶ��ʱ����
void DutTimeOut(uint32 param)
{
    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION, SHUTDOWN);
}

// д���־��ʱ����
void WriteFlagTimeOut(uint32 param)
{
    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
}

// �汾������ʱ����
void OperateVerTimeOut(uint32 param)
{
    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_VERSION_INFO, FALSE);
}

// ����ϱ�
void Report_Identity(uint32 param)
{
    STS_PROTOCOL_SendCmdAck(STS_PROTOCOL_CMD_REPORT_OF_IDENTITY);
}

// ѡ��Э��д���־������
void SELECT_PROTOCOL_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam)
{
    if (dut_info.commProt)
    {
//        if (placeParam == 2)
//        {
//            placeParam = 3;
//        }
//        else if (placeParam == 0)
//        {
//            placeParam = 1;
//        }
//        else if (placeParam == 1)
//        {
//            placeParam = 2;
//        }
        UART_PROTOCOL_XM_SendCmdTwoParamAck(UART_PROTOCOL_XM_CMD_WRITE_FLAG_DATA, placeParam, 2);
    }
    else
    {
        if (dut_info.dutBusType)
        {
            CAN_PROTOCOL_TEST_WriteFlag(placeParam, shutdownFlagParam);
        }
        else
        {
            DUT_PROTOCOL_WriteFlag(placeParam, shutdownFlagParam);
        }
    }
}

// ѡ��Э�鷢������
void SELECT_PROTOCOL_SendCmdAck(uint8 paramm)
{
    if (dut_info.commProt)
    {
        UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_READ_FLAG_DATA);
    }
    else
    {
        if (dut_info.dutBusType)
        {
            CAN_PROTOCOL_TEST_SendCmdAck(paramm);
        }
        else
        {
            DUT_PROTOCOL_SendCmdAck(paramm);
        }
    }
}

// ���������һ������
void SELECT_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
    if (dut_info.commProt)
    {
        if (ackParam == 0)
        {
            ackParam = 1;
        }
        else if (ackParam == 1)
        {
            ackParam = 2;
        }
        else if (ackParam == 2)
        {
            ackParam = 3;
        }
        UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_READ_VERSION_TYPE, ackParam);
    }
    else
    {
        if (dut_info.dutBusType)
        {
            CAN_PROTOCOL_TEST_SendCmdParamAck(ackCmd, ackParam);
        }
        else
        {
            DUT_PROTOCOL_SendCmdParamAck(ackCmd, ackParam);
        }
    }
}

