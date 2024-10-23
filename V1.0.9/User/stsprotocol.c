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
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void STS_PROTOCOL_DataStructInit(STS_PROTOCOL_CB *pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void STS_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void STS_PROTOCOL_RxFIFOProcess(STS_PROTOCOL_CB *pCB);

// UART命令帧缓冲区处理
void STS_PROTOCOL_CmdFrameProcess(STS_PROTOCOL_CB *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL STS_PROTOCOL_CheckSUM(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL STS_PROTOCOL_ConfirmTempCmdFrameBuff(STS_PROTOCOL_CB *pCB);

// 协议层发送处理过程
void STS_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void STS_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// 选择协议写入标志区数据
void SELECT_PROTOCOL_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam);

// 选择协议发送命令
void SELECT_PROTOCOL_SendCmdAck(uint8 param);

// 选择协议发送命令，带一个参数
void SELECT_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);
//===============================================================================================================

// 发送命令回复
void STS_PROTOCOL_SendCmdAck(uint8 ackCmd);

// 发送带应答的回复
void STS_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// 仪表状态信息回复
void STS_PROTOCOL_SendCmdStatusAck(void);

// DUT通讯超时处理
void DutTimeOut(uint32 param);

//=======================================================================================
// 全局变量定义
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
uint8 resultArray[150];  // 用于存储结果的一维数组
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

// DTA升级显示
char dtaUpMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "Erasure APP Data",
    "Writing APP Data",
    "Updating DTA ",
    "Set Param Succeed",
};

// DUT升级显示
char dutUpMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "Writing APP Data",
    "Updating DUT ",
    "Up Progress ",
    "DUT Up Completed",
};

// 测试显示
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

// 协议初始化
void STS_PROTOCOL_Init(void)
{
    // 协议层数据结构初始化
    STS_PROTOCOL_DataStructInit(&STSProtocolCB);

    // 向驱动层注册数据接收接口
    STS_UART_RegisterDataSendService(STS_PROTOCOL_MacProcess);

    // 向驱动层注册数据发送接口
    STS_PROTOCOL_RegisterDataSendService(STS_UART_AddTxArray);
}

// STS协议层过程处理
void STS_PROTOCOL_Process(void)
{
    // STS接收FIFO缓冲区处理
    STS_PROTOCOL_RxFIFOProcess(&STSProtocolCB);

    // STS接收命令缓冲区处理
    STS_PROTOCOL_CmdFrameProcess(&STSProtocolCB);

    // STS协议层发送处理过程
    STS_PROTOCOL_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void STS_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = STSProtocolCB.tx.head;
    uint16 end = STSProtocolCB.tx.end;
    STS_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &STSProtocolCB.tx.cmdQueue[STSProtocolCB.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % STS_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void STS_PROTOCOL_TxAddFrame(void)
{
    uint16 cc = 0;
    uint16 i = 0;
    uint16 head = STSProtocolCB.tx.head;
    uint16 end = STSProtocolCB.tx.end;
    STS_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &STSProtocolCB.tx.cmdQueue[STSProtocolCB.tx.end];
    uint16 length = pCmdFrame->length;

    // 发送缓冲区已满，不予接收
    if ((end + 1) % STS_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if ((length >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
            || (length + 2 >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
    {
        pCmdFrame->length = 0;

        return;
    }

    // 队尾命令帧已满，退出
    if (length >= STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
    pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] = length - 3;   // 重设数据长度，减去"命令头、命令字、数据长度"4个字节

    for (i = 0; i < length; i++)
    {
        cc ^= pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = cc ;

    STSProtocolCB.tx.end++;
    STSProtocolCB.tx.end %= STS_PROTOCOL_TX_QUEUE_SIZE;
    // pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

// 数据结构初始化
void STS_PROTOCOL_DataStructInit(STS_PROTOCOL_CB *pCB)
{
    uint16 i;

    // 参数合法性检验
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

// UART报文接收处理函数(注意根据具体模块修改)
void STS_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = STSProtocolCB.rxFIFO.end;
    uint16 head = STSProtocolCB.rxFIFO.head;
    uint8 rxdata = 0x00;

    // 接收数据
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

    // 一级缓冲区已满，不予接收
    if ((end + 1) % STS_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // 一级缓冲区未满，接收
    else
    {
        // 将接收到的数据放到临时缓冲区中
        STSProtocolCB.rxFIFO.buff[end] = rxdata;
        STSProtocolCB.rxFIFO.end++;
        STSProtocolCB.rxFIFO.end %= STS_PROTOCOL_RX_FIFO_SIZE;
    }
}

// UART协议层向驱动层注册数据发送接口
void STS_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    STSProtocolCB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL STS_PROTOCOL_ConfirmTempCmdFrameBuff(STS_PROTOCOL_CB *pCB)
{
    STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return FALSE;
    }

    // 临时缓冲区为空，不予添加
    pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
    if (0 == pCmdFrame->length)
    {
        return FALSE;
    }

    // 添加
    pCB->rx.end++;
    pCB->rx.end %= STS_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

    return TRUE;
}

// 协议层发送处理过程
void STS_PROTOCOL_TxStateProcess(void)
{
    uint16 head = STSProtocolCB.tx.head;
    uint16 end = STSProtocolCB.tx.end;
    uint16 length = STSProtocolCB.tx.cmdQueue[head].length;
    uint8 *pCmd = STSProtocolCB.tx.cmdQueue[head].buff;
    uint16 localDeviceID = STSProtocolCB.tx.cmdQueue[head].deviceID;

    // 发送缓冲区为空，说明无数据
    if (head == end)
    {
        return;
    }

    // 发送函数没有注册直接返回
    if (NULL == STSProtocolCB.sendDataThrowService)
    {
        return;
    }

    // 协议层有数据需要发送到驱动层
    if (!(*STSProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // 发送环形队列更新位置
    STSProtocolCB.tx.cmdQueue[head].length = 0;
    STSProtocolCB.tx.head++;
    STSProtocolCB.tx.head %= STS_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void STS_PROTOCOL_RxFIFOProcess(STS_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
    uint16 length = 0;
    uint8 currentData = 0;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return;
    }

    // 一级缓冲区为空，退出
    if (head == end)
    {
        return;
    }

    // 获取临时缓冲区指针
    pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];

    // 取出当前要处理的字节
    currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];

    // 临时缓冲区长度为0时，搜索首字节
    if (0 == pCmdFrame->length)
    {
        // 命令头错误，删除当前字节并退出
        if (STS_PROTOCOL_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // 命令头正确，但无临时缓冲区可用，退出
        if ((pCB->rx.end + 1) % STS_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
        {
            return;
        }

        // 添加UART通讯超时时间设置-2016.1.5增加
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
        TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
                      BLE_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
                      BLE_PROTOCOL_CALLBACK_RxTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
#endif

        // 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
        pCmdFrame->buff[pCmdFrame->length++] = currentData;
        pCB->rxFIFO.currentProcessIndex++;
        pCB->rxFIFO.currentProcessIndex %= STS_PROTOCOL_RX_FIFO_SIZE;
    }
    // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
    else
    {
        // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
        if (pCmdFrame->length >= STS_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
            pCmdFrame->length = 0; // 2016.1.5增加

            // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // 一直取到末尾
        while (end != pCB->rxFIFO.currentProcessIndex)
        {
            // 取出当前要处理的字节
            currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];

            // 缓冲区未溢出，正常接收，将数据添加到临时缓冲区中
            pCmdFrame->buff[pCmdFrame->length++] = currentData;
            pCB->rxFIFO.currentProcessIndex++;
            pCB->rxFIFO.currentProcessIndex %= STS_PROTOCOL_RX_FIFO_SIZE;

            // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正 ■■

            // 首先判断命令帧最小长度，一个完整的命令字至少包括4个字节: 命令头、命令字、数据长度
            if (pCmdFrame->length < STS_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // 继续接收
                continue;
            }

            // 命令帧长度数值越界，说明当前命令帧错误，停止接收
            if (pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] > (STS_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - STS_PROTOCOL_CMD_FRAME_LENGTH_MIN))
            {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;

                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // 命令帧长度校验
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] + STS_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // 长度要求不一致，说明未接收完毕，退出继续
                continue;
            }

            // 命令帧长度OK，则进行校验，失败时删除命令头
            if (!STS_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;

                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif
            // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            STS_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// 对传入的命令帧进行校验，返回校验结果
BOOL STS_PROTOCOL_CheckSUM(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint8 cc = 0;
    uint16 i = 0;

    if (NULL == pCmdFrame)
    {
        return FALSE;
    }

    // 从命令头开始，到校验码之前的一个字节，依次进行异或运算
    for (i = 0; i < pCmdFrame->length - 1; i++)
    {
        cc ^= pCmdFrame->buff[i];
    }

    // 判断计算得到的校验码与命令帧中的校验码是否相同
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
        // 配置信息进度只有0%和100%。
    }
    else
    {
        // 所有项升级完毕
        TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);
        dut_info.configUpFaile = FALSE;
        dut_info.appUpFaile = FALSE;
        dut_info.uiUpFaile = FALSE;
    }

    if (dut_info.appUpFaile) // app升级失败
    {
        dut_info.appUpFaile = FALSE;
        dut_info.dutProgress[3] = 102;
        dut_info.currentAppSize = 0;
        TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_REPORT);
        TIMER_KillTask(TIMER_ID_SET_DUT_PROGRESS_LCD_REPORT);
        dut_info.appUpFlag = FALSE;
    }

    if (dut_info.configUpFaile) // config升级失败
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
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);         // 添加命令头
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_TOOL_DUT_PROCESS); // 添加命令字
    STS_PROTOCOL_TxAddData(0x00);                          // 数据长度临时为0
    for (i = 0; i < 8; i++)
    {
        STS_PROTOCOL_TxAddData(dut_info.dutProgress[i]);
    }

    STS_PROTOCOL_TxAddFrame(); // 设置数据长度和添加校验

    // LCD显示
    if (dut_info.appUpFlag)
    {
        sprintf(progressStr, "%d", dut_info.dutProgress[3]); // 将dut_info.dutProgress[3]转换为字符串
        addFormattedString(2, 12, progressStr, dutUpMsgBuff); // 将转换后的字符串传递给addFormattedString函数
        addString(2, 15, "%", dutUpMsgBuff);
    }

    if (dut_info.configUpFlag)
    {
        sprintf(progressStr, "%d", dut_info.dutProgress[7]); // 将dut_info.dutProgress[3]转换为字符串
        addFormattedString(2, 12, progressStr, dutUpMsgBuff); // 将转换后的字符串传递给addFormattedString函数
        addString(2, 15, "%", dutUpMsgBuff);
    }
}

// LCD进度上报
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

// 升级项控制
void setUpItem(uint8 upItem, uint8 ctrlState)
{
    dut_info.appUpFlag = FALSE;
    dut_info.uiUpFlag = FALSE;
    dut_info.configUpFlag = FALSE;

    // 清除状态
    dut_info.currentAppSize = 0;
    dut_info.currentUiSize = 0;
    if (1 == ctrlState)
    {
        if (0x02 == (upItem & 0x02)) // 升级app
        {
            dut_info.appUpFlag = TRUE;
            testFlag = FALSE;
        }

        if (0x08 == (upItem & 0x08)) // 升级config
        {
            dut_info.configUpFlag = TRUE;
            testFlag = TRUE;
        }
    }
}

// 设置进度上报信息
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

    // 上报STS
    TIMER_AddTask(TIMER_ID_SET_DUT_PROGRESS_REPORT,
                  300,
                  dutProgressReport,
                  TRUE,
                  TIMER_LOOP_FOREVER,
                  ACTION_MODE_ADD_TO_QUEUE);

    // LCD显示
    TIMER_AddTask(TIMER_ID_SET_DUT_PROGRESS_LCD_REPORT,
                  8000,
                  lcdProgressReport,
                  TRUE,
                  TIMER_LOOP_FOREVER,
                  ACTION_MODE_ADD_TO_QUEUE);

}


// 根据通讯方式进入不同的状态机
void enterState(void)
{
    switch (dut_info.dutBusType)
    {
    case 0: // 串口

        switch (dut_info.test)
        {
        // 进入UART测试
        case TEST_TYPE_UART:
            STATE_EnterState(STATE_UART_TEST);
            break;

        // 进入GND测试
        case TEST_TYPE_GND:
            STATE_EnterState(STATE_UART_GND_TEST);
            break;

        // 进入大灯测试
        case TEST_TYPE_HEADLIGHT:
            STATE_EnterState(STATE_UART_HEADLIGHT_TEST);
            break;

        // 进入近光灯测试
        case TEST_TYPE_LBEAM:
            STATE_EnterState(STATE_UART_LBEAM_TEST);
            break;

        // 进入远光灯测试
        case TEST_TYPE_HBEAM:
            STATE_EnterState(STATE_UART_HBEAM_TEST);
            break;

        // 进入左转向灯测试
        case TEST_TYPE_LEFT_TURN_SIGNAL:
            STATE_EnterState(STATE_UART_LEFT_TURN_SIGNAL_TEST);
            break;

        // 进入右转向灯测试
        case TEST_TYPE_RIGHT_TURN_SIGNAL:
            STATE_EnterState(STATE_UART_RIGHT_TURN_SIGNAL_TEST);
            break;

        // 进入油门测试
        case TEST_TYPE_THROTTLE:
            STATE_EnterState(STATE_UART_THROTTLE_TEST);
            break;

        // 进入刹把测试
        case TEST_TYPE_BRAKE:
            STATE_EnterState(STATE_UART_BRAKE_TEST);
            break;

        // 进入VLK测试
        case TEST_TYPE_VLK:
            STATE_EnterState(STATE_UART_VLK_TEST);
            break;

        // 进入电压校准测试
        case TEST_TYPE_VOLTAGE:
            STATE_EnterState(STATE_UART_VOLTAGE_TEST);
            break;

        // 进入(小米)大灯测试
        case TEST_TYPE_XM_HEADLIGHT:
            STATE_EnterState(STATE_UART_XM_HEADLIGHT_TEST);
            break;

        // 进入(小米)左转向灯测试
        case TEST_TYPE_XM_LEFT_TURN_SIGNAL:
            STATE_EnterState(STATE_UART_XM_LEFT_TURN_SIGNAL_TEST);
            break;

        // 进入(小米)右转向灯测试
        case TEST_TYPE_XM_RIGHT_TURN_SIGNAL:
            STATE_EnterState(STATE_UART_XM_RIGHT_TURN_SIGNAL_TEST);
            break;

        // 进入(小米)油门测试
        case TEST_TYPE_XM_THROTTLE:
            STATE_EnterState(STATE_UART_XM_THROTTLE_TEST);
            break;

        // 进入(小米)刹把测试
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
        // 进入GND测试
        case TEST_TYPE_GND:
            STATE_EnterState(STATE_UART_GND_TEST);
            break;

        // 进入大灯测试
        case TEST_TYPE_HEADLIGHT:
            STATE_EnterState(STATE_CAN_HEADLIGHT_TEST);
            break;

        // 进入左转向灯测试
        case TEST_TYPE_LEFT_TURN_SIGNAL:
            STATE_EnterState(STATE_CAN_LEFT_TURN_SIGNAL_TEST);
            break;

        // 进入右转向灯测试
        case TEST_TYPE_RIGHT_TURN_SIGNAL:
            STATE_EnterState(STATE_CAN_RIGHT_TURN_SIGNAL_TEST);
            break;

        // 进入油门测试
        case TEST_TYPE_THROTTLE:
            STATE_EnterState(STATE_CAN_THROTTLE_TEST);
            break;

        // 进入刹把测试
        case TEST_TYPE_BRAKE:
            STATE_EnterState(STATE_CAN_BRAKE_TEST);
            break;

        // 进入VLK测试
        case TEST_TYPE_VLK:
            STATE_EnterState(STATE_CAN_VLK_TEST);
            break;

        // 进入电压校准测试
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

// 根据升级信息进入不同的状态机
void enterUpState()
{
    if (dut_info.appUpFlag || dut_info.configUpFlag)
    {
        switch (dut_info.dutBusType)
        {
        case 0: // 串口
            switch (dut_info.ID)
            {
            case DUT_TYPE_SEG:
                // 进入数码管、段码屏升级
                STATE_EnterState(STATE_CM_UART_SEG_UPGRADE);
                break;

            case DUT_TYPE_CM:
                // 进入通用彩屏升级
                STATE_EnterState(STATE_CM_UART_BC_UPGRADE);
                break;

            case DUT_TYPE_HUAXIN:
                // 进入华芯微特升级
                STATE_EnterState(STATE_HUAXIN_UART_BC_UPGRADE);
                break;

            case DUT_TYPE_KAIYANG:
                // 进入开阳升级
                STATE_EnterState(STATE_KAIYANG_UART_BC_UPGRADE);
                break;

            case DUT_TYPE_LIME:
                // 进入LIME升级
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
                // 进入高标升级
                STATE_EnterState(STATE_GAOBIAO_CAN_UPGRADE);
                break;

            case DUT_TYPE_SEG:
                // 进入数码管、段码屏升级
                STATE_EnterState(STATE_SEG_CAN_UPGRADE);
                break;

            case DUT_TYPE_HUAXIN:
                // 华芯微特升级
                STATE_EnterState(STATE_HUAXIN_CAN_UPGRADE);
                break;

            case DUT_TYPE_KAIYANG:
                // 进入开阳升级
                STATE_EnterState(STATE_KAIYANG_CAN_UPGRADE);
                break;

            case DUT_TYPE_SPARROW:
                // 进入sparrow升级
                STATE_EnterState(STATE_SPARROW_CAN_UPGRADE);
                break;

            default:
                // 进入通用can升级
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

// STS命令帧缓冲区处理
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
    uint8 noNumber[43] = {0};  // 42个配置参数
    uint8 k = 0;
    uint8 lenght = 0;
    uint8 keyNumber = 0;
    uint8 cmdLength = 0;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return;
    }

    // 命令帧缓冲区为空，退出
    if (pCB->rx.head == pCB->rx.end)
    {
        return;
    }

    // 获取当前要处理的命令帧指针
    pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.head];

    // 命令头非法，退出
    if (STS_PROTOCOL_CMD_HEAD != pCmdFrame->buff[STS_PROTOCOL_CMD_HEAD_INDEX])
    {
        // 删除命令帧
        pCB->rx.head++;
        pCB->rx.head %= STS_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // 命令头合法，则提取命令
    cmd = (STS_PROTOCOL_CMD)pCmdFrame->buff[STS_PROTOCOL_CMD_CMD_INDEX];

    // 执行命令帧
    switch (cmd)
    {
    // 空命令
    case STS_PROTOCOL_CMD_NULL:
        break;

    // DTA身份上报
    case STS_PROTOCOL_CMD_REPORT_OF_IDENTITY:
        TIMER_KillTask(TIMER_ID_REPORT_OF_IDENTITY);
        break;

    // 过站标志校验（命令字=0x01）
    case STS_PROTOCOL_CMD_CHECK_STATION_FLAG:

        // 获取站位编号
        stationNumber = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        offFlag = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];

        // 写入处理
        if (0x01 == (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX] & 0x01))
        {
            // 写入标志区数据
            SELECT_PROTOCOL_WriteFlag(stationNumber, offFlag);

            // 写入超时定时器
            TIMER_AddTask(TIMER_ID_WRITE_FLAG_TIMEOUT,
                          3000,
                          WriteFlagTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
        }

        // 对比校验
        if (0x02 == (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX] & 0x02))
        {
            // 读取标志区数据
            SELECT_PROTOCOL_SendCmdAck(DUT_PROTOCOL_CMD_READ_FLAG_DATA);

            // 超时定时器
            TIMER_AddTask(TIMER_ID_WRITE_FLAG_TIMEOUT,
                          3000,
                          WriteFlagTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
        }

        // LCD显示
        Vertical_Scrolling_Display(verTestMsgBuff, 4, 0);
        break;

    // 版本信息校验命令（命令字=0x02）
    case STS_PROTOCOL_CMD_CHECK_VERSION_INFO:

        // 版本类型
        dutverType = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];

        // 信息长度
        infoLen = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];

        // 类型信息
        strcpy(dutverBuff, (const char *)&pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX]);
        dutverBuff[infoLen] = 0;

        // 写入校验
        if (0x01 == (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX] & 0x01))
        {
            // 标志
            verwriteFlag = TRUE;

            // 写入版本类型信息
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

        // 读取
        if (0x02 == (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX] & 0x02))
        {
            // 标志
            verreadFlag = TRUE;

            // 读取版本类型信息
            SELECT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_READ_VERSION_TYPE_INFO, dutverType);
        }

        // 版本操作超时定时器
        TIMER_AddTask(TIMER_ID_OPERATE_VER_TIMEOUT,
                      3000,
                      OperateVerTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);

        // LCD显示
        Vertical_Scrolling_Display(verTestMsgBuff, 4, 1);
        break;

    // 获取DTA软件信息
    case STS_PROTOCOL_CMD_GET_SOFTWARE_INFO:                                 // 0x30,
        SPI_FLASH_ReadArray(versionBoot, SPI_FLASH_BOOT_VERSION_ADDEESS, 3); // 获取BOOT版本
        SPI_FLASH_ReadArray(versionApp, SPI_FLASH_APP_VERSION_ADDEESS, 3);   // 获取APP版本
        STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);                       // 添加命令头
        STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_GET_SOFTWARE_INFO);              // 添加命令字
        STS_PROTOCOL_TxAddData(0x00);                                        // 数据长度临时为0
        STS_PROTOCOL_TxAddData(versionBoot[0]);                              // 添加BOOT版本
        STS_PROTOCOL_TxAddData(versionBoot[1]);
        STS_PROTOCOL_TxAddData(versionBoot[2]);
        STS_PROTOCOL_TxAddData(versionApp[0]); // 添加APP版本
        STS_PROTOCOL_TxAddData(versionApp[1]);
        STS_PROTOCOL_TxAddData(versionApp[2]);
        STS_PROTOCOL_TxAddFrame(); // 设置数据长度和添加校验
        break;

    // DTA工具升级开始
    case STS_PROTOCOL_CMD_CLEAR_APP:
        SPI_FLASH_EraseRoom(SPI_FLASH_TOOL_APP_ADDEESS, 256 * 1024);     // 256k
        allPacketSize = 0;
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CLEAR_APP, TRUE);

        // LCD显示
        Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 0);
        break;

    // DTA升级文件数据写入
    case STS_PROTOCOL_CMD_TOOL_UP_APP:

        // 向片外flash写入STS发送的数据
        packetSize = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] - 4;
        addr1 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
        addr2 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        addr3 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
        addr4 = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX];
        writeAddr = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | (addr4);
        writeRes = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX], SPI_FLASH_TOOL_APP_ADDEESS + writeAddr, packetSize);

        // 返回信息给STS
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_UP_APP, TRUE);
        allPacketSize++;

        if (showFlag)
        {
            // LCD显示
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 1);
            showFlag = FALSE;
        }
        break;

    // DTA升级结束
    case STS_PROTOCOL_CMD_TOOL_UP_END:

        // 发送升级成功应答
        // 设置升级标志位标志
        allPacket[0] = allPacketSize >> 24;
        allPacket[1] = allPacketSize >> 16;
        allPacket[2] = allPacketSize >> 8;
        allPacket[3] = allPacketSize;

        SPI_FLASH_EraseSector(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS); // 擦除标志位存储区
        SPI_FLASH_EraseSector(SPI_FLASH_TOOL_APP_DATA_SIZE);       // 擦除大小存储区

        SPI_FLASH_WriteByte(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS, 0xAA);
        SPI_FLASH_WriteArray(allPacket, SPI_FLASH_TOOL_APP_DATA_SIZE, 4);

        STS_PROTOCOL_SendCmdAck(STS_PROTOCOL_CMD_TOOL_UP_END);

        /*100ms后跳转回BOOT，读取标志位，然后替换替换工具APP区*/
        TIMER_AddTask(TIMER_ID_TOOL_APP_TO_BOOT,
                      100,
                      IAP_JumpToAppFun,
                      IAP_GD32_FLASH_BASE,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);

        // LCD显示
        Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 2);
        showFlag = TRUE;
        break;

    // 设置DUT通讯参数
    case STS_PROTOCOL_CMD_TOOL_SET_DUT_COMM_PARAM:
        cmdLength = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];
        baudRate_value = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        online_detection_cnt = 0;

        // 设置波特率
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

        // 设置帧类型
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

        // 设置通讯协议  0:DTA协议   1：小米协议
        if ((3 < cmdLength) && (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX]))
        {
            dut_info.commProt = TRUE;
        }
        else
        {
            dut_info.commProt = FALSE;
        }

        // 设置通讯方式并初始化
        communication_Type = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];

        // 电源使能、八方高标VLK供电
        if (4 < cmdLength)
        {
            dut_info.VLKFlag = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX];
            _5VOUT_EN(dut_info.VLKFlag);
        }

        switch (communication_Type)
        {
        // 3.3V串口通讯
        case 0x01:
            CHANGE_Init();
            _5V_CHANGE_OFF();
            VCCB_EN_ON();
            UART_DRIVE_InitSelect(uart_baudRate);        // UART驱动层初始化
            DUT_PROTOCOL_Init();                         // UART协议层初始化
            UART_PROTOCOL_XM_Init();
            dut_info.dutBusType = FALSE;

            // LCD显示
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // 上报执行结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 5V串口通讯
        case 0x02:
            CHANGE_Init();
            _5V_CHANGE_ON();
            VCCB_EN_ON();
            UART_DRIVE_InitSelect(uart_baudRate);        // UART驱动层初始化
            DUT_PROTOCOL_Init();                         // UART协议层初始化
            UART_PROTOCOL_XM_Init();
            dut_info.dutBusType = FALSE;

            // LCD显示
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // 上报执行结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // CAN通讯
        case 0x03:
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate);       // CAN驱动层初始化
            dut_info.dutBusType = TRUE;

            // LCD显示
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // 上报执行结果
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

    case STS_PROTOCOL_CMD_MULTIPLE_TEST:     // 接收到多项测试指令 0xDF
        test_Item = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];  // 获取测试类型
        testFlag = TRUE;

        // 获取目标电压值
        voltageTarget = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        voltageTarget = voltageTarget << 8;
        voltageTarget = voltageTarget | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
        voltageTarget = voltageTarget << 8;
        voltageTarget = voltageTarget | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX];
        voltageTarget = voltageTarget << 8;
        voltageTarget = voltageTarget | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX];

        // 获取电压误差值
        voltageTarget_error = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA6_INDEX];
        voltageTarget_error = voltageTarget_error << 8;
        voltageTarget_error = voltageTarget_error | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA7_INDEX];

        // 换算得电压误差范围
        dut_info.voltageMax = voltageTarget + voltageTarget_error;
        dut_info.voltageMin = voltageTarget - voltageTarget_error;

        // 大灯测试
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

        // 左转向灯测试
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

        // 右转向灯测试
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

        // 近光灯测试
        if (0x04 == (test_Item & 0x04))
        {
            dut_info.test = TEST_TYPE_LBEAM;
            enterState();
        }

        // 远光灯测试
        if (0x05 == (test_Item & 0x05))
        {
            dut_info.test = TEST_TYPE_HBEAM;
            enterState();
        }

        // 油门测试
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

        // 刹把测试
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

        // VLK测试
        if (0x08 == (test_Item & 0x08))
        {
            dut_info.test = TEST_TYPE_VLK;
            enterState();

            // LCD显示
            Vertical_Scrolling_Display(volTestMsgBuff, 4, 2);
        }

        // 电子变速测试
        if (0x09 == (test_Item & 0x09))
        {
            dut_info.test = TEST_TYPE_DERAILLEUR;
            enterState();

            // LCD显示
            Vertical_Scrolling_Display(deTestMsgBuff, 4, 0);
        }
        break;

    // GND测试
    case STS_PROTOCOL_CMD_GND_TEST: // 0x04

        // 配置刹把和电子变速GND测试项
        if (5 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_BRAKE_DERAILLEUR;
            enterState();
        }

        // 配置电子变速GND测试项
        if (4 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_DERAILLEUR;
            enterState();
        }

        // 配置刹把、油门GND测试项
        if (3 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_THROTTLE_BRAKE;
            enterState();
        }

        // 配置刹把GND测试项
        if (2 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_BRAKE;
            enterState();
        }

        // 配置油门GND测试项
        if (1 == pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.timeout_Period = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_GND;
            dut_info.gnd = GND_TYPE_THROTTLE;
            enterState();
        }


        break;

    // USB充电测试（负载设置）
    case STS_PROTOCOL_CMD_USB_CHARGE_TEST: // 0x14

        // LCD显示
        Vertical_Scrolling_Display(showTestMsgBuff, 4, 3);

        // 获取负载电压
        load_voltage = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
        load_voltage = load_voltage << 8;
        load_voltage = load_voltage | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];

        // 获取电压误差
        voltage_error = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX];
        voltage_error = voltage_error << 8;
        voltage_error = voltage_error | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX];

        // 获取负载电流
        load_current = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX];

        // 获取电流误差
        current_error = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA6_INDEX];
        current_error = current_error << 8;
        current_error = current_error | pCmdFrame->buff[STS_PROTOCOL_CMD_DATA7_INDEX];

        // 调整USB负载
        // 500MA
        if (0x00 == load_current)
        {
            //调整USB负载
            DUT_USB_EN1_OFF();
            DUT_USB_EN3_OFF();
            DUT_USB_EN2_ON();
            Delayms(400);

//                // 通过mcu获取USB充电电压
//                real_load_voltage = USB_ADC_Read();
            real_load_voltage = USB_ADC_Read();

            // 实际测得的负载电压在误差允许的范围内
            if (((load_voltage - voltage_error) < real_load_voltage) && (real_load_voltage < (load_voltage + voltage_error)))
            {
//                    // 通过mcu获取USB充电电流
                real_load_current = USB_CURRENT_Read();
//real_load_current = adcCB.usbcurrent.voltage;

                // 实际测得的负载电流在误差允许的范围内
                if (((500 - current_error) < real_load_current) && (real_load_current < (500 + current_error)))
                {
                    // 上报STS测试通过
                    Clear_All_Lines();
                    Display_Centered(0, "USB Charging ");
                    Display_Centered(1, "Test Pass");
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, TRUE);
                }
                else
                {
                    // 测试失败
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, FALSE);
                }
            }
            else
            {
                // 测试失败
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_USB_CHARGE_TEST, FALSE);
            }
        }

        // 1A
        if (0x01 == (load_current & 0x01))    // 1A
        {
            //调整USB负载
            DUT_USB_EN1_ON();
            DUT_USB_EN2_OFF();
            DUT_USB_EN3_OFF();
            Delayms(400);
            
            // 通过mcu获取USB充电电压
            real_load_voltage = USB_ADC_Read();

            // 实际测得的负载电压在误差允许的范围内
            if (((load_voltage - voltage_error) < real_load_voltage) && (real_load_voltage < (load_voltage + voltage_error)))
            {
                // 通过mcu获取USB充电电流
                real_load_current = USB_CURRENT_Read();

                // 实际测得的负载电流在误差允许的范围内
                if (((1000 - current_error) < real_load_current) && (real_load_current < (1000 + current_error)))
                {
                    Clear_All_Lines();
                    Display_Centered(0, "USB Charging ");
                    Display_Centered(1, "Test Pass");

                    // 上报STS测试通过
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
            //调整USB负载
            DUT_USB_EN1_ON();
            DUT_USB_EN2_ON();
            DUT_USB_EN3_ON();
            Delayms(400);
            // 通过mcu获取USB充电电压
            real_load_voltage = USB_ADC_Read();

            // 实际测得的负载电压在误差允许的范围内
            if (((load_voltage - voltage_error) < real_load_voltage) && (real_load_voltage < (load_voltage + voltage_error)))
            {

                // 通过mcu获取USB充电电流
                real_load_current = USB_CURRENT_Read();

                // 实际测得的负载电流在误差允许的范围内
                if (((2000 - current_error) < real_load_current) && (real_load_current < (2000 + current_error)))
                {
                    Clear_All_Lines();
                    Display_Centered(0, "USB Charging ");
                    Display_Centered(1, "Test Pass");

                    // 上报STS测试通过
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

    // 电压校准（多级校准）
    case STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION: // 0xDD

        // LCD显示
        Vertical_Scrolling_Display(volTestMsgBuff, 3, 0);

        // 获取校准级别
        dut_info.cal_number = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];

        // 一级校准
        if (0x01 == dut_info.cal_number)
        {
            calibration.data[0] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
            dut_info.test = TEST_TYPE_VOLTAGE;
            enterState();
            dut_info.cali_cnt = 1;
        }

        // 二级校准
        if (0x02 == dut_info.cal_number)
        {
            calibration.data[0] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
            calibration.data[1] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
            dut_info.test = TEST_TYPE_VOLTAGE;
            enterState();
            dut_info.cali_cnt = 2;
        }

        // 三级校准
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

    // 调整仪表供电电压
    case STS_PROTOCOL_CMD_ADJUST_DUT_VOLTAGE:      // 0x16
        dut_info.adjustState = TRUE;

        // 通知电源板调整DUT供电电压
        POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX]);
        break;

    // 写标志区数据
    case STS_PROTOCOL_CMD_WRITE_FLAG_DATA:

        // 关闭VLK供电
        VLK_PW_EN_OFF();
        UART_DRIVE_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
        Clear_All_Lines();
        Display_Centered(0, "VLK CLOSE");
        break;

    // UART测试
    case STS_PROTOCOL_CMD_UART_TEST:      // 0x19 - UART测试命令
        dut_info.test = TEST_TYPE_UART;
        enterState();

        // LCD显示
        Vertical_Scrolling_Display(volTestMsgBuff, 3, 1);
        break;

    // Flash 校验测试
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

    // 仪表电源控制
    case STS_PROTOCOL_CMD_CONTROL_DUT_POWER:
        dut_info.powerOnFlag = TRUE;

        // 开启电源
        if (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
        }

        // 关闭电源
        if (!pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            // 关闭vlk供电
//                VLK_PW_EN_OFF();
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
        }
        break;

    // 获取WAKE电压
    case STS_PROTOCOL_CMD_OBTAIN_WAKE_UP_VOL:
        STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
        STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_OBTAIN_WAKE_UP_VOL);
        STS_PROTOCOL_TxAddData(0x02);
        STS_PROTOCOL_TxAddData(KEY_ADC_Read() >> 8);
        STS_PROTOCOL_TxAddData(KEY_ADC_Read() & 0xFF);
        STS_PROTOCOL_TxAddFrame();
        break;

    // 笔段屏/数码管显示测试
    case STS_PROTOCOL_CMD_DISPLAY_TEST:

        if (dut_info.commProt)
        {
            // 只取数据透传
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_LED);  // 数码管显示指令
            UART_PROTOCOL_XM_TxAddData(0);                             // 数据长度

            for (i = 0; i < pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] - 1; i++) // 减去测试模式一个字节
            {
                UART_PROTOCOL_XM_TxAddData(pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX + 1 + i]);
            }

            UART_PROTOCOL_XM_TxAddFrame();

//                UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
//                UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
//                UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_LED);  // 数码管显示指令

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

    // 蜂鸣器控制测试
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

    // 蓝牙测试
    case STS_PROTOCOL_CMD_BLE_TEST:

        // 若测4lite
        if (dut_info.commProt)
        {
            for (i = 0 ; i < pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX]; i++)
            {
                verBle[i] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX + i];
            }

            // 先查询蓝牙固件版本
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_BLE_VERSION);  // 获取OOB参数指令
            UART_PROTOCOL_XM_TxAddData(0x00);                                   // 数据长度
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

    // 按键测试
    case STS_PROTOCOL_CMD_KEY_TEST:

        dut_info.buttonSimulationFlag = FALSE;

        // 若测4lite
        if (dut_info.commProt)
        {
            // 按键测试
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_KEY);  // 获取OOB参数指令
            UART_PROTOCOL_XM_TxAddData(0x00);         // 数据长度
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

    // 模拟按键测试
    case STS_PROTOCOL_CMD_SIMULATION_KEY_TEST:

        dut_info.buttonSimulationFlag = TRUE;

        // 按键测试
        // 若测4lite
        if (dut_info.commProt)
        {
            // 按键测试
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_TEST_KEY);  // 获取OOB参数指令
            UART_PROTOCOL_XM_TxAddData(0x00);         // 数据长度
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

        // 模拟按键按下
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
            // 处理无效的按键编号
            break;
        }
        break;

    // 获取DUT供电电流
    case STS_PROTOCOL_CMD_OBTAIN_DUT_CURRENT:
        POWER_PROTOCOL_SendCmdAck(POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_CURRENT);
        break;

//        // 升级DUT控制命令
//        case STS_PROTOCOL_CMD_TOOL_DUT_UP:

//            // 关闭在线检测
//            TIMER_KillTask(TIMER_ID_ONLINE_DETECT);

//            upItem = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
//            ctrlState = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];

//            // 设置要升级的项
//            setUpItem(upItem, ctrlState);

//            // 设置进度上报信息
//            setDutProgressReportInfo();

//            // 根据升级信息，进入不同的状态机
//            enterUpState();

//            // 应答
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_DUT_UP, TRUE);

//            // LCD显示
//            Vertical_Scrolling_Display(dutUpMsgBuff, 4, 1);
//            break;

//        // 0x41 - DUT升级进度上报
//        case STS_PROTOCOL_CMD_TOOL_DUT_PROCESS:
//            break;

//        // 0x42 - 清空升级缓冲区
//        case STS_PROTOCOL_CMD_TOOL_CLEAR_BUFF:
//            clearItem = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];
//            if (0x01 == (clearItem & 0x01))
//            {
//                /*擦除片外的dut_boot区*/
//                dut_info.bootSize = 0;
//                SPI_FLASH_EraseRoom(SPI_FLASH_DUT_BOOT_ADDEESS, 128 * 1024); // 128k
//            }
//            if (0x02 == (clearItem & 0x02))
//            {
//                /*擦除片外的dut_app区*/
//                dut_info.appSize = 0;
//                SPI_FLASH_EraseRoom(SPI_FLASH_DUT_APP_ADDEESS, 1024 * 1024); // 1M
//            }
//            if (0x08 == (clearItem & 0x08))
//            {
//                /*擦除片外的dut_config区*/
//                SPI_FLASH_EraseSector(SPI_FLASH_DUT_CONFIG_ADDEESS); // 4k
//            }
//            SPI_FLASH_EraseSector(SPI_FLASH_DUT_INFO); // 擦除dut机型信息
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_CLEAR_BUFF, TRUE);
//            break;

//        // 0x43 - DTA写入DUT机型信息
//        case STS_PROTOCOL_CMD_TOOL_SET_DUT_INFO:
//            dut_info_len = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX] - 3;

//            for (dutName_i = 0; dutName_i < dut_info_len; dutName_i++)
//            {
//                dutAll[dutName_i] = pCmdFrame->buff[dutName_i + 3 + 3];
//            }
//            dutAll[20] = dut_info_len;                                  // 长度
//            //        dutAll[21] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX]; // 电压
//            //        dutAll[22] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX]; // 通讯类型
//            dutAll[23] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX]; // bootType
//            dutAll[24] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX]; // appType
//            dutAll[25] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA3_INDEX]; // uiType
//            dutAll[26] = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX]; // configType

//            /* 将机型信息写入到片外flash */
//            SPI_FLASH_EraseSector(SPI_FLASH_DUT_INFO);
//            writeRes = SPI_FLASH_WriteWithCheck(dutAll, SPI_FLASH_DUT_INFO, 30);

//            if (writeRes)
//            {
//                DutInfoUpdata(); // 更新dutinfo数据
//            }
//            /* 返回信息给STS */
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_DUT_INFO, writeRes);
//            break;

//        case STS_PROTOCOL_CMD_TOOL_GET_DUT_INFO: // 0x44 - 获取DTA记录的DUT机型信息
//            SPI_FLASH_ReadArray(dutAll, SPI_FLASH_DUT_INFO, 30);
//            if (0xFF == dutAll[20]) // 机型为空
//            {
//                dutAll[20] = 10;
//            }
//            STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);           // 添加命令头
//            STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_TOOL_GET_DUT_INFO);  // 添加命令字
//            STS_PROTOCOL_TxAddData(24);                              // 数据长度临时为0
//            STS_PROTOCOL_TxAddData(dutAll[23]);                      // bootType
//            STS_PROTOCOL_TxAddData(dutAll[24]);                      // appType
//            STS_PROTOCOL_TxAddData(dutAll[25]);                      // uiType
//            STS_PROTOCOL_TxAddData(dutAll[26]);                      // configType
//            for (dutName_i = 0; dutName_i < dutAll[20]; dutName_i++) // 添加机型信息
//            {
//                STS_PROTOCOL_TxAddData(dutAll[dutName_i]);
//            }
//            STS_PROTOCOL_TxAddFrame(); // 设置数据长度和添加校验
//            break;

//        case STS_PROTOCOL_CMD_UP_CONFIG: // 0x50 - 写入DUT配置参数
//            SPI_FLASH_EraseSector(SPI_FLASH_DUT_CONFIG_ADDEESS); // 4k

//            // 读取配置信息的flash区，然后写入DUT
//            dataLen = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];
//            writeRes = SPI_FLASH_WriteWithCheck(&(pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX]), SPI_FLASH_DUT_CONFIG_ADDEESS, dataLen + 1);

//            // 返回信息给STS
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UP_CONFIG, writeRes);
//            break;

//        case STS_PROTOCOL_CMD_UP_BOOT:   // 0x51 - BOOT数据写入

//            // 读取BOOT的flash区，然后写入DUT
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

//            // 返回信息给STS
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UP_BOOT, writeRes);
//            break;

//        case STS_PROTOCOL_CMD_UP_APP:    // 0x52 - APP数据写入
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
//                // LCD显示
//                Vertical_Scrolling_Display(dutUpMsgBuff, 4, 0);
//                dutFlag = FALSE;
//            }
//            break;

//        case STS_PROTOCOL_CMD_UP_END:    // 0x53 - 升级项结束
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

//            // 返回信息给STS
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UP_END, endItem);

//            // LCD显示
//            Vertical_Scrolling_Display(dutUpMsgBuff, 4, 3);
//            dutFlag = TRUE;
//            break;

//        // 设置不写入项
//        case STS_PROTOCOL_CMD_SET_NOT_WRITTEN:

//            // 读取配置数据
//            memset(configs, 0, sizeof(configs));
//            memset(resultArray, 0, sizeof(resultArray));
//            memset(verifiedBuff, 0, sizeof(verifiedBuff));
//            memset(paramLength, 0, sizeof(paramLength));
//            memset(paramNumber, 0, sizeof(paramNumber));
//            resultIndex = 0;
//            verifiedIndex = 0;

//            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 170);

//            // 获取不写入项序号
//            noLenght = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];
//            for (i = 0; i < noLenght ; i++)
//            {
//                noNumber[i] = pCmdFrame->buff[3 + i];
//            }

//            // 设置不写入项，获取不写入项序号
//            if (noLenght)
//            {
//                // 对参数进行处理，提取参数内容
//                for (i = 0; i < configs[0] ;)
//                {
//                    paramLength[i] = configs[i + 1];   // 参数长度
//                    paramNumber[i] = configs[i + 2];   // 参数序号

//                    for (k = 0; k < noLenght; k++)
//                    {
//                        if (noNumber[k] != configs[i + 2])
//                        {
//                            // 拼接配置参数
//                            for (j = 0 ; j < paramLength[i]; j++)
//                            {
//                                paramContent[paramNumber[i]][j] = configs[i + 2 + j + 1];

//                                // 将数据添加到结果数组
//                                resultArray[resultIndex++] = paramContent[paramNumber[i]][j];
//                            }
//                        }
//                        i = i + 1 + paramLength[i] + 1;
//                    }
//                }

//                // 获取待校验buff，去掉系统时间
//                for (k = 0; k < noLenght; k++)
//                {
//                    // 会写入系统的时间
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

//                    // 不会写入系统时间
//                    if (noNumber[k] == 3)
//                    {
//                        for (i = 0; i < resultIndex ; i++)
//                        {
//                            verifiedBuff[verifiedIndex++] = resultArray[i];
//                        }
//                    }
//                }
//            }

//            // 不设置
//            if (!noLenght)
//            {
//                for (i = 0; i < configs[0] ;)
//                {
//                    paramLength[i] = configs[i + 1];
//                    paramNumber[i] = configs[i + 2];
//                    for (j = 0 ; j < paramLength[i]; j++)
//                    {
//                        paramContent[paramNumber[i]][j] = configs[i + 2 + j + 1];

//                        // 将数据添加到结果数组
//                        resultArray[resultIndex++] = paramContent[paramNumber[i]][j];
//                    }
//                    i = i + 1 + paramLength[i] + 1;
//                }

//                // 获取待校验buff，去掉系统时间
//                for (i = 0; i < resultIndex ; i++)
//                {
//                    if (i == 2)
//                    {
//                        i = i + 5;
//                    }
//                    verifiedBuff[verifiedIndex++] = resultArray[i];
//                }
//            }

//            // 返回信息给STS
//            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_SET_NOT_WRITTEN, TRUE);
//            break;

    // 获取POWER软件信息
    case STS_PROTOCOL_CMD_POWER_GET_SOFTWARE_INFO:
        POWER_PROTOCOL_SendCmdAck(POWER_PROTOCOL_CMD_GET_SOFTWARE_INFO);
        break;

    // POWER-APP2数据擦除
    case STS_PROTOCOL_CMD_POWER_UP_APP_ERASE:
        powerPacketSize = 0;
        POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
        POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
        POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_ECO_APP2_ERASE);
        POWER_PROTOCOL_TxAddData(0);
        POWER_PROTOCOL_TxAddFrame();
        break;

    // POWER-APP2数据写入
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

    // POWER-APP数据写入完成
    case STS_PROTOCOL_CMD_POWER_UP_APP_WRITE_FINISH:
        POWER_PROTOCOL_SendCmdAck(POWER_PROTOCOL_CMD_ECO_APP_WRITE_FINISH);
        break;

    // 设置透传DUT通讯参数
    case STS_PROTOCOL_CMD_TOOL_SET_TRAN_DUT_COMM_PARAM:
        cmdLength = pCmdFrame->buff[STS_PROTOCOL_CMD_LENGTH_INDEX];
        baudRate_value = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA2_INDEX];
        online_detection_cnt = 0;

        // 设置波特率
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

        // 设置帧类型
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

        // 设置通讯协议  0:DTA协议   1：小米协议
//           if((3 < cmdLength)&&(pCmdFrame->buff[STS_PROTOCOL_CMD_DATA4_INDEX]))
//           {
//               dut_info.commProt = TRUE;
//           }
//           else
//           {
//               dut_info.commProt = FALSE;
//           }

        // 设置通讯方式并初始化
        communication_Type = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];

//            // 电源使能、八方高标VLK供电
//           if(4 < cmdLength)
//           {
//                dut_info.VLKFlag = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA5_INDEX];
//                _5VOUT_EN(dut_info.VLKFlag);
//           }

        switch (communication_Type)
        {
        // 3.3V串口通讯
        case 0x01:
            CHANGE_Init();
            _5V_CHANGE_OFF();
            VCCB_EN_ON();
            UART_DRIVE_InitSelect(uart_baudRate);        // UART驱动层初始化
            UART_PROTOCOL_Init();        // 55升级协议
            UART_PROTOCOL4_Init();
            dut_info.dutBusType = FALSE;

            // LCD显示
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // 上报执行结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_TRAN_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 5V串口通讯
        case 0x02:
            CHANGE_Init();
            _5V_CHANGE_ON();
            VCCB_EN_ON();
            UART_DRIVE_InitSelect(uart_baudRate);        // UART驱动层初始化
            dut_info.dutBusType = FALSE;
            UART_PROTOCOL4_Init();

            // LCD显示
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // 上报执行结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_TOOL_SET_TRAN_DUT_COMM_PARAM, TRUE);

            TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
                          3000,
                          DutTimeOut,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // CAN通讯
        case 0x03:
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate);       // CAN驱动层初始化

            CAN_PROTOCOL1_Init();  // 通用配置初始化

            dut_info.dutBusType = TRUE;

            // LCD显示
            Vertical_Scrolling_Display(dtaUpMsgBuff, 4, 3);

            // 上报执行结果
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

    // 透传DUT指令控制
    case STS_PROTOCOL_CMD_TRAN_DUT_INSTRUCTION_CONTROL:
        if (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
        {
            dut_info.passThroughControl = TRUE;
        }
        else
        {
            dut_info.passThroughControl = FALSE;
        }

        // 上报执行结果
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

        // LCD显示
        Vertical_Scrolling_Display(volTestMsgBuff, 4, 3);
        break;
    }

    // 删除命令帧
    pCB->rx.head++;
    pCB->rx.head %= STS_PROTOCOL_RX_QUEUE_SIZE;
}


// RX通讯超时处理-单向
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void BLE_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
    STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    pCmdFrame = &STSProtocolCB.rx.cmdQueue[STSProtocolCB.rx.end];

    // 超时错误，将命令帧长度清零，即认为抛弃该命令帧
    pCmdFrame->length = 0; // 2016.1.6增加
    // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
    STSProtocolCB.rxFIFO.head++;
    STSProtocolCB.rxFIFO.head %= STS_PROTOCOL_RX_FIFO_SIZE;
    STSProtocolCB.rxFIFO.currentProcessIndex = STSProtocolCB.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void BLE_PROTOCOL_StopRxTimeOutCheck(void)
{
    TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

#if BLE_PROTOCOL_TXRX_TIME_OUT_CHECK_ENABLE
// TXRX通讯超时处理-双向
void BLE_PROTOCOL_CALLBACK_TxRxTimeOut(uint32 param)
{
}

// 停止TxRX通讯超时检测任务
void BLE_PROTOCOL_StopTxRxTimeOutCheck(void)
{
    TIMER_KillTask(TIMER_ID_UART_TXRX_TIME_OUT_CONTROL);
}
#endif

// 发送命令回复
void STS_PROTOCOL_SendCmdAck(uint8 ackCmd)
{
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
    STS_PROTOCOL_TxAddData(ackCmd);
    STS_PROTOCOL_TxAddData(0x00);
    STS_PROTOCOL_TxAddFrame();
}

// 发送命令回复，带一个参数
void STS_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
    STS_PROTOCOL_TxAddData(ackCmd);
    STS_PROTOCOL_TxAddData(0x01);
    STS_PROTOCOL_TxAddData(ackParam);
    STS_PROTOCOL_TxAddFrame();
}

// 发送命令回复，带两个参数
void STS_PROTOCOL_SendCmdParamTwoAck(uint8 ackCmd, uint8 ackParam, uint8 twoParam)
{
    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
    STS_PROTOCOL_TxAddData(ackCmd);
    STS_PROTOCOL_TxAddData(0x02);
    STS_PROTOCOL_TxAddData(ackParam);
    STS_PROTOCOL_TxAddData(twoParam);
    STS_PROTOCOL_TxAddFrame();
}

// DUT通讯超时处理
void DutTimeOut(uint32 param)
{
    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION, SHUTDOWN);
}

// 写入标志超时处理
void WriteFlagTimeOut(uint32 param)
{
    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
}

// 版本操作超时处理
void OperateVerTimeOut(uint32 param)
{
    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_VERSION_INFO, FALSE);
}

// 身份上报
void Report_Identity(uint32 param)
{
    STS_PROTOCOL_SendCmdAck(STS_PROTOCOL_CMD_REPORT_OF_IDENTITY);
}

// 选择协议写入标志区数据
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

// 选择协议发送命令
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

// 发送命令，带一个参数
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

