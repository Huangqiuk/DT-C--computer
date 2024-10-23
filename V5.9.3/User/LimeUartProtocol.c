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
* 【内部接口声明】
******************************************************************************/

// 数据结构初始化
void LIME_UART_PROTOCOL_DataStructInit(LIME_UART_PROTOCOL_CB *pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void LIME_UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void LIME_UART_PROTOCOL_RxFIFOProcess(LIME_UART_PROTOCOL_CB *pCB);

// UART命令帧缓冲区处理
void LIME_UART_PROTOCOL_CmdFrameProcess(LIME_UART_PROTOCOL_CB *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL LIME_UART_PROTOCOL_CheckSUM(LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL LIME_UART_PROTOCOL_ConfirmTempCmdFrameBuff(LIME_UART_PROTOCOL_CB *pCB);

// 通讯超时处理-单向
void LIME_UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// 停止RX通讯超时检测任务
void LIME_UART_PROTOCOL_StopRxTimeOutCheck(void);

// 协议层发送处理过程
void LIME_UART_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void LIME_UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// 码表设置周期下发数据请求标志
void LIME_UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param);

// UART总线超时错误处理
void LIME_UART_PROTOCOL_CALLBACK_UartBusError(uint32 param);

//=========================================================================
void LIME_UART_PROTOCOL_SendCmdTwoBytesDataAck(uint8 *pBuf, uint8 length1, uint8 length2);
void LIME_UART_PROTOCOL_SendCmdOneByteDataAck(uint8 *pBuf, uint8 length1);
void LIME_UART_PROTOCOL_SendCmdNoDataAck(uint8 *pBuf);
void LIME_UART_PROTOCOL_SendCmdAppVersion(uint8 *pBuf);

// 版本发送命令
void LIME_UART_PROTOCOL_SendCmdVersion(uint8 versionCmd, uint8 *pBuf);

// 读取FLASH回复
void LIME_UART_PROTOCOL_SendCmdReadFlashRespond(uint32 length, uint8 *pBuf);

// 上电发送匹配命令
void LIME_UART_PROTOCOL_SendSnMatching(uint8 length, uint8 *Sn);

// 全局变量定义
LIME_UART_PROTOCOL_CB uartProtocolCB4;


// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

uint32 uartProtocolOutTimeDelay[6] = {30 * 60 * 1000,       // 空命令
                                      30 * 60 * 1000,         // LOGO
                                      30 * 60 * 1000,         // 提示
                                      1000,                   // 正常骑行
                                      1000,                   // 异常骑行提示
                                      30 * 60 * 1000,
                                     };// 升级

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

// 协议初始化
void LIME_UART_PROTOCOL_Init(void)
{
    // 协议层数据结构初始化
    LIME_UART_PROTOCOL_DataStructInit(&uartProtocolCB4);

    // 向驱动层注册数据接收接口
    UART_DRIVE_RegisterDataSendService(LIME_UART_PROTOCOL_MacProcess);

    // 向驱动层注册数据发送接口
    LIME_UART_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);
}

// UART协议层过程处理
void LIME_UART_PROTOCOL_Process(void)
{
    // UART接收FIFO缓冲区处理
    LIME_UART_PROTOCOL_RxFIFOProcess(&uartProtocolCB4);

    // UART接收命令缓冲区处理
    LIME_UART_PROTOCOL_CmdFrameProcess(&uartProtocolCB4);

    // UART协议层发送处理过程
    LIME_UART_PROTOCOL_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void LIME_UART_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end =  uartProtocolCB4.tx.end;
    LIME_UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB4.tx.cmdQueue[uartProtocolCB4.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % LIME_UART_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length ++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void LIME_UART_PROTOCOL_TxAddFrame(void)
{
    uint8 cc = 0;
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end  = uartProtocolCB4.tx.end;
    LIME_UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB4.tx.cmdQueue[uartProtocolCB4.tx.end];
    uint16 length = pCmdFrame->length;

    // 发送缓冲区已满，不予接收
    if ((end + 1) % LIME_UART_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 命令帧长度不足，清除已填充的数据，退出
    if (LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length) // 减去"校验码"1个字节
    {
        pCmdFrame->length = 0;

        return;
    }

    // 队尾命令帧已满，退出
    if (length >= LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
    // 重设数据长度，需要减去10=(5同步头+1协议版本+1设备号+1命令字+2数据长度)
    pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_LENGTH_H_INDEX] = ((length - 10) & 0xFF00) >> 8;
    pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_LENGTH_L_INDEX] = ((length - 10) & 0x00FF);

    cc = UART_GetCRCValue(&pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - LIME_UART_PROTOCOL_HEAD_BYTE));
    pCmdFrame->buff[length] = cc;
    pCmdFrame->length ++;

    uartProtocolCB4.tx.end ++;
    uartProtocolCB4.tx.end %= LIME_UART_PROTOCOL_TX_QUEUE_SIZE;
    //pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

// 数据结构初始化
void LIME_UART_PROTOCOL_DataStructInit(LIME_UART_PROTOCOL_CB *pCB)
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

// UART报文接收处理函数(注意根据具体模块修改)
void LIME_UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB4.rxFIFO.end;
    uint16 head = uartProtocolCB4.rxFIFO.head;
    uint8 rxdata = 0x00;

    // 接收数据
    rxdata = *pData;

    // 一级缓冲区已满，不予接收
    if ((end + 1) % LIME_UART_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // 一级缓冲区未满，接收
    else
    {
        // 将接收到的数据放到临时缓冲区中
        uartProtocolCB4.rxFIFO.buff[end] = rxdata;
        uartProtocolCB4.rxFIFO.end ++;
        uartProtocolCB4.rxFIFO.end %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
    }
}

// UART协议层向驱动层注册数据发送接口
void LIME_UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB4.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL LIME_UART_PROTOCOL_ConfirmTempCmdFrameBuff(LIME_UART_PROTOCOL_CB *pCB)
{
    LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end ++;
    pCB->rx.end %= LIME_UART_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0;   // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

    return TRUE;
}

// 协议层发送处理过程
void LIME_UART_PROTOCOL_TxStateProcess(void)
{
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end =  uartProtocolCB4.tx.end;
    uint16 length = uartProtocolCB4.tx.cmdQueue[head].length;
    uint8 *pCmd = uartProtocolCB4.tx.cmdQueue[head].buff;
    uint16 localDeviceID = uartProtocolCB4.tx.cmdQueue[head].deviceID;

    // 发送缓冲区为空，说明无数据
    if (head == end)
    {
        return;
    }

    // 发送函数没有注册直接返回
    if (NULL == uartProtocolCB4.sendDataThrowService)
    {
        return;
    }

    // 协议层有数据需要发送到驱动层
    if (!(*uartProtocolCB4.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // 发送环形队列更新位置
    uartProtocolCB4.tx.cmdQueue[head].length = 0;
    uartProtocolCB4.tx.head ++;
    uartProtocolCB4.tx.head %= LIME_UART_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void LIME_UART_PROTOCOL_RxFIFOProcess(LIME_UART_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (LIME_UART_PROTOCOL_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head ++;
            pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // 命令头正确，但无临时缓冲区可用，退出
        if ((pCB->rx.end + 1) % LIME_UART_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
        {
            return;
        }

        // 添加UART通讯超时时间设置-2016.1.5增加
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
        TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
                      LIME_UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
                      LIME_UART_PROTOCOL_CALLBACK_RxTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
#endif

        // 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
        pCmdFrame->buff[pCmdFrame->length++] = currentData;
        pCB->rxFIFO.currentProcessIndex ++;
        pCB->rxFIFO.currentProcessIndex %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
    }
    // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
    else
    {
        // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
        if (pCmdFrame->length >= LIME_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            LIME_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
            pCmdFrame->length = 0;  // 2016.1.5增加
            // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
            pCB->rxFIFO.head ++;
            pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex ++;
            pCB->rxFIFO.currentProcessIndex %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;

            // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■

            // 首先判断命令帧最小长度，一个完整的命令字至少包括10个字节,因此不足8个字节的必定不完整
            if (pCmdFrame->length < LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // 继续接收
                continue;
            }

            // 命令帧长度数值越界，说明当前命令帧错误，停止接收
            length = ((uint16)pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_LENGTH_H_INDEX] << 8) + pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_LENGTH_L_INDEX];
            if (length > LIME_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                LIME_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head ++;
                pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // 命令帧长度校验，在命令长度描述字的数值上，增加命令帧最小长度，即为命令帧实际长度
            if (pCmdFrame->length < (length + LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN))
            {
                // 长度要求不一致，说明未接收完毕，退出继续
                continue;
            }

            // 命令帧长度OK，则进行校验，失败时删除命令头
            if (!LIME_UART_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                LIME_UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head ++;
                pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            LIME_UART_PROTOCOL_StopRxTimeOutCheck();
#endif
            length = pCmdFrame->length;
            // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            LIME_UART_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// 对传入的命令帧进行校验，返回校验结果
BOOL LIME_UART_PROTOCOL_CheckSUM(LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint8 cc = 0;

    if (NULL == pCmdFrame)
    {
        return FALSE;
    }

    // 从协议版本开始，到校验码之前的一个字节，依次进行CRC8运算
    cc = UART_GetCRCValue(&pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - (LIME_UART_PROTOCOL_HEAD_BYTE + LIME_UART_PROTOCOL_CHECK_BYTE)));

    // 判断计算得到的校验码与命令帧中的校验码是否相同
    if (pCmdFrame->buff[pCmdFrame->length - 1] != cc)
    {
        return FALSE;
    }

    return TRUE;

}

// 上报命令执行结果
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

    // 写入字节数
    LIME_UART_PROTOCOL_TxAddData(1);

    // 写入结果
    LIME_UART_PROTOCOL_TxAddData((uint8)param);

    LIME_UART_PROTOCOL_TxAddFrame();
}


// 上报命令应答
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

// 上报模块写入结果
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

    // 写入字节数
    LIME_UART_PROTOCOL_TxAddData(byteNum / 256);
    LIME_UART_PROTOCOL_TxAddData(byteNum % 256);

    // 写入结果
    LIME_UART_PROTOCOL_TxAddData((uint8)exeResoult);

    LIME_UART_PROTOCOL_TxAddFrame();
}

// 数码管显示
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

    // 写入字节数
    LIME_UART_PROTOCOL_TxAddData(d1);
    LIME_UART_PROTOCOL_TxAddData(d2);

    LIME_UART_PROTOCOL_TxAddFrame();
}



// UART命令帧缓冲区处理
void LIME_UART_PROTOCOL_CmdFrameProcess(LIME_UART_PROTOCOL_CB *pCB)
{
    LIME_UART_PROTOCOL_CMD cmd = LIME_UART_PROTOCOL_CMD_NULL;
    LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
    uint8 eraResual = 0;
    uint8 i;
    uint8 length1;

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
    if (LIME_UART_PROTOCOL_CMD_HEAD != pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_HEAD1_INDEX])
    {
        // 删除命令帧
        pCB->rx.head ++;
        pCB->rx.head %= LIME_UART_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // 命令头合法，则提取命令
    cmd = (LIME_UART_PROTOCOL_CMD)pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_CMD_INDEX];
    switch (dut_info.ID)
    {
    case DUT_TYPE_NULL:
        break;

    case DUT_TYPE_LIME: // 串口通用升级
        // 执行命令帧
        switch (cmd)
        {
        // 收到仪表发送的ECO请求
        case LIME_UART_PROTOCOL_ECO_CMD_REQUEST: // 01
            // 发送允许进入工程模式
            STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST);
            break;

        // 仪表回应准备就绪
        case LIME_UART_PROTOCOL_ECO_CMD_READY:  // 02
            // 发送app擦除指令
            STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR);
            break;

        // dut擦除结果查询
        case LIME_UART_PROTOCOL_ECO_CMD_APP_ERASE:  //04
            // 提取仪表擦除结果
            eraResual = pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_DATA2_INDEX];

            // 擦除失败
            if (0 == eraResual)
            {
                // 擦除失败再擦除一次
                STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR);
                break;
            }
            // 擦除成功
            // 发送第一包数据
            STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET);

            break;
        case LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE: // 05,app数据写入
           // eraResual = pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_DATA2_INDEX];
					 eraResual = pCmdFrame->buff[LIME_UART_PROTOCOL_CMD_DATA3_INDEX];
            if (0 == eraResual) // 如果仪表写入失败
            {
                break;
            }
            else
            {
                if (dut_info.currentAppSize < dut_info.appSize)
                {
                    STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_SEND_APP_PACKET); // 发送app数据包
                }
                else
                {
                    STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_APP_UP_SUCCESS);
                }
            }
            break;

        case LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE_FINISH: // 06，app升级完毕
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

    // 无通讯，回到LOGO界面
//  TIMER_AddTask(TIMER_ID_STATE_CONTROL,
//                  timeOut,
//                  LIME_UART_PROTOCOL_CALLBACK_UartBusError,
//                  TRUE,
//                  1,
//                  ACTION_MODE_ADD_TO_QUEUE);

    // 删除命令帧
    pCB->rx.head ++;
    pCB->rx.head %= LIME_UART_PROTOCOL_RX_QUEUE_SIZE;
}

// RX通讯超时处理-单向
#if LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void LIME_UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
    LIME_UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    pCmdFrame = &uartProtocolCB4.rx.cmdQueue[uartProtocolCB4.rx.end];

    // 超时错误，将命令帧长度清零，即认为抛弃该命令帧
    pCmdFrame->length = 0;  // 2016.1.6增加
    // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
    uartProtocolCB4.rxFIFO.head ++;
    uartProtocolCB4.rxFIFO.head %= LIME_UART_PROTOCOL_RX_FIFO_SIZE;
    uartProtocolCB4.rxFIFO.currentProcessIndex = uartProtocolCB4.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void LIME_UART_PROTOCOL_StopRxTimeOutCheck(void)
{
    TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

// 码表设置立刻下发数据请求标志
void LIME_UART_PROTOCOL_SetTxAtOnceRequest(uint32 param)
{
    uartProtocolCB4.txAtOnceRequest = (BOOL)param;
}

// 码表设置周期下发数据请求标志
void LIME_UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param)
{
    uartProtocolCB4.txPeriodRequest = (BOOL)param;
}

// 发送命令字及回复一个字节
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

// 发送命令字
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
    SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + addr, 128); // 工具读取128字节

    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD1);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD2);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD3);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD4);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_HEAD5);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_CMD_DEVICE_ADDR);
    LIME_UART_PROTOCOL_TxAddData(LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE);

    LIME_UART_PROTOCOL_TxAddData(0x00);                                                    // 长度暂时为0
		LIME_UART_PROTOCOL_TxAddData(0x87);                                                    // 长度暂时为0

 		LIME_UART_PROTOCOL_TxAddData(0x04);                                                    // 4个地址数据   
    LIME_UART_PROTOCOL_TxAddData(addr1);                                                       // 添加地址
    LIME_UART_PROTOCOL_TxAddData(addr2);                                                       // 添加地址
    LIME_UART_PROTOCOL_TxAddData(addr3);                                                       // 添加地址
    LIME_UART_PROTOCOL_TxAddData(addr4);                                                       // 添加地址
		
 		LIME_UART_PROTOCOL_TxAddData(0x00);                                                    // 一个字节数据   
  	LIME_UART_PROTOCOL_TxAddData(0x80);                                                    // 一个字节数据      		
    for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 128; appUpdateOnePacket_i++) // 添加数据
    {
        LIME_UART_PROTOCOL_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i]);
    }
    LIME_UART_PROTOCOL_TxAddFrame(); // 调整帧格式,修改长度和添加校验
}
