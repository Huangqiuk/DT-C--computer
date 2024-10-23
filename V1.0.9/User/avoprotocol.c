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
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void AVO_PROTOCOL_DataStructInit(AVO_PROTOCOL_CB *pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void AVO_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void AVO_PROTOCOL_RxFIFOProcess(AVO_PROTOCOL_CB *pCB);

// UART命令帧缓冲区处理
void AVO_PROTOCOL_CmdFrameProcess(AVO_PROTOCOL_CB *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL AVO_PROTOCOL_CheckSUM(AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL AVO_PROTOCOL_ConfirmTempCmdFrameBuff(AVO_PROTOCOL_CB *pCB);

// 协议层发送处理过程
void AVO_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void AVO_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// 发送命令回复
void AVO_PROTOCOL_SendCmdAck(uint8 ackCmd);

// 发送带应答的回复
void AVO_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// 仪表状态信息回复
void AVO_PROTOCOL_SendCmdStatusAck(void);

//=======================================================================================
// 全局变量定义
AVO_PROTOCOL_CB AVOProtocolCB;
MEASURE_RESULT measure;
OHM ohm;
uint8 headlight_cnt = 0;
uint8 gnd_cnt = 0;

// 万用表测试显示
char avoTestMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "Meas Completed",
};

// 协议初始化
void AVO_PROTOCOL_Init(void)
{
    // 协议层数据结构初始化
    AVO_PROTOCOL_DataStructInit(&AVOProtocolCB);

    // 向驱动层注册数据接收接口
    AVO_UART_RegisterDataSendService(AVO_PROTOCOL_MacProcess);

    // 向驱动层注册数据发送接口
    AVO_PROTOCOL_RegisterDataSendService(AVO_UART_AddTxArray);
}

// AVO协议层过程处理
void AVO_PROTOCOL_Process(void)
{
    // AVO接收FIFO缓冲区处理
    AVO_PROTOCOL_RxFIFOProcess(&AVOProtocolCB);

    // AVO接收命令缓冲区处理
    AVO_PROTOCOL_CmdFrameProcess(&AVOProtocolCB);

    // AVO协议层发送处理过程
    AVO_PROTOCOL_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void AVO_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = AVOProtocolCB.tx.head;
    uint16 end = AVOProtocolCB.tx.end;
    AVO_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &AVOProtocolCB.tx.cmdQueue[AVOProtocolCB.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % AVO_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void AVO_PROTOCOL_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = AVOProtocolCB.tx.head;
    uint16 end = AVOProtocolCB.tx.end;
    AVO_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &AVOProtocolCB.tx.cmdQueue[AVOProtocolCB.tx.end];
    uint16 length = pCmdFrame->length;

    // 发送缓冲区已满，不予接收
    if ((end + 1) % AVO_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 命令帧长度不足，清除已填充的数据，退出
    if (AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length) // 减去"校验和"1个字节
    {
        pCmdFrame->length = 0;

        return;
    }

    // 队尾命令帧已满，退出
    if (length >= AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
    // 重设数据长度,需要减去10=(3同步头1命令字+1数据长度)
    pCmdFrame->buff[AVO_PROTOCOL_CMD_LENGTH_INDEX] = length - 5;

    //cc = UART_GetCRCValue(&pCmdFrame->buff[UART_PROTOCOL_CMD_VERSION_INDEX], (pCmdFrame->length - UART_PROTOCOL_HEAD_BYTE));
    /* 异或取反计算校验码 */
    for (i = 0; i < pCmdFrame->length; i++)
    {
        checkSum ^= pCmdFrame->buff[i];
    }
    checkSum = ~checkSum;

    pCmdFrame->buff[length] = checkSum;
    pCmdFrame->length ++;

    AVOProtocolCB.tx.end++;
    AVOProtocolCB.tx.end %= AVO_PROTOCOL_TX_QUEUE_SIZE;
    // pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

// 数据结构初始化
void AVO_PROTOCOL_DataStructInit(AVO_PROTOCOL_CB *pCB)
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

// UART报文接收处理函数(注意根据具体模块修改)
void AVO_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = AVOProtocolCB.rxFIFO.end;
    uint16 head = AVOProtocolCB.rxFIFO.head;
    uint8 rxdata = 0x00;

    // 接收数据
    rxdata = *pData;

    // 一级缓冲区已满，不予接收
    if ((end + 1) % AVO_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // 一级缓冲区未满，接收
    else
    {
        // 将接收到的数据放到临时缓冲区中
        AVOProtocolCB.rxFIFO.buff[end] = rxdata;
        AVOProtocolCB.rxFIFO.end++;
        AVOProtocolCB.rxFIFO.end %= AVO_PROTOCOL_RX_FIFO_SIZE;
    }
}

// UART协议层向驱动层注册数据发送接口
void AVO_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    AVOProtocolCB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL AVO_PROTOCOL_ConfirmTempCmdFrameBuff(AVO_PROTOCOL_CB *pCB)
{
    AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= AVO_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

    return TRUE;
}

// 协议层发送处理过程
void AVO_PROTOCOL_TxStateProcess(void)
{
    uint16 head = AVOProtocolCB.tx.head;
    uint16 end = AVOProtocolCB.tx.end;
    uint16 length = AVOProtocolCB.tx.cmdQueue[head].length;
    uint8 *pCmd = AVOProtocolCB.tx.cmdQueue[head].buff;
    uint16 localDeviceID = AVOProtocolCB.tx.cmdQueue[head].deviceID;

    // 发送缓冲区为空，说明无数据
    if (head == end)
    {
        return;
    }

    // 发送函数没有注册直接返回
    if (NULL == AVOProtocolCB.sendDataThrowService)
    {
        return;
    }

    // 协议层有数据需要发送到驱动层
    if (!(*AVOProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // 发送环形队列更新位置
    AVOProtocolCB.tx.cmdQueue[head].length = 0;
    AVOProtocolCB.tx.head++;
    AVOProtocolCB.tx.head %= AVO_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void AVO_PROTOCOL_RxFIFOProcess(AVO_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (AVO_PROTOCOL_CMD_HEAD1 != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // 命令头正确，但无临时缓冲区可用，退出
        if ((pCB->rx.end + 1) % AVO_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
        {
            return;
        }

        // 添加UART通讯超时时间设置-2016.1.5增加
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
        TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
                      AVO_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
                      AVO_PROTOCOL_CALLBACK_RxTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
#endif

        // 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
        pCmdFrame->buff[pCmdFrame->length++] = currentData;
        pCB->rxFIFO.currentProcessIndex++;
        pCB->rxFIFO.currentProcessIndex %= AVO_PROTOCOL_RX_FIFO_SIZE;
    }
    // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
    else
    {
        // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
        if (pCmdFrame->length >= AVO_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
            pCmdFrame->length = 0; // 2016.1.5增加
            // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= AVO_PROTOCOL_RX_FIFO_SIZE;

            // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正 ■■

            // 首先判断命令帧最小长度，一个完整的命令字至少包括8个字节: 命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，因此不足8个字节的必定不完整
            if (pCmdFrame->length < AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // 继续接收
                continue;
            }

            // 命令帧长度数值越界，说明当前命令帧错误，停止接收
            if (pCmdFrame->buff[AVO_PROTOCOL_CMD_LENGTH_INDEX] > (AVO_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN))
            {
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                AVO_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // 命令帧长度校验
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[AVO_PROTOCOL_CMD_LENGTH_INDEX] + AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // 长度要求不一致，说明未接收完毕，退出继续
                continue;
            }

            // 命令帧长度OK，则进行校验，失败时删除命令头
            if (!AVO_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                AVO_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            AVO_PROTOCOL_StopRxTimeOutCheck();
#endif
            // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            AVO_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// 对传入的命令帧进行校验，返回校验结果
BOOL AVO_PROTOCOL_CheckSUM(AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint8_t checkSum = 0;
    uint8_t sumTemp;
    uint16_t i = 0;

    /* 参数合法性检验 */
    if (NULL == pCmdFrame)
    {
        return FALSE;
    }

    /* 从设备地址开始，到校验码之前的一个字节，依次进行异或运算 */
    for (i = 0;  i < pCmdFrame->length - 1; i++)
    {
        checkSum ^= pCmdFrame->buff[i];
    }

    /* 异或和取反 */
    checkSum = ~checkSum ;

    /* 获取命令帧中校验码 */
    sumTemp = pCmdFrame->buff[pCmdFrame->length - 1];

    /* 判断计算得到的校验码与命令帧中的校验码是否相同 */
    if (sumTemp != checkSum)
    {
        return FALSE;
    }

    return TRUE;
}

// 上报命令执行结果
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

    // 写入字节数
    AVO_PROTOCOL_TxAddData(1);

    // 写入结果
    AVO_PROTOCOL_TxAddData((uint8)param);

    AVO_PROTOCOL_TxAddFrame();
}

// 下发测量类型命令, 通道默认为0
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

// 下发复位命令
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

// 下发测量类型和测量通道命令
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
    __set_FAULTMASK(1); // 关闭所有中断
    NVIC_SystemReset(); // 复位
}

// AVO命令帧缓冲区处理
void AVO_PROTOCOL_CmdFrameProcess(AVO_PROTOCOL_CB *pCB)
{
    AVO_PROTOCOL_CMD cmd = AVO_CMD_NULL;
    AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
    AVO_MEASURE_TYPE mesureType;

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
    if (AVO_PROTOCOL_CMD_HEAD1 != pCmdFrame->buff[AVO_PROTOCOL_CMD_HEAD1_INDEX])
    {
        // 删除命令帧
        pCB->rx.head ++;
        pCB->rx.head %= AVO_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // 命令头合法，则提取命令
    cmd = (AVO_PROTOCOL_CMD)pCmdFrame->buff[AVO_PROTOCOL_CMD_INDEX];

    // 执行命令帧
    switch (cmd)
    {
        case AVO_CMD_NULL:
            break;

        case AVO_CMD_AVOMETER_ACK:
            mesureType = (AVO_MEASURE_TYPE)pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA1_INDEX];

            switch (mesureType)
            {
                // 直流电压（V）
                case MEASURE_DCV :
                    measure.data[0] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA3_INDEX];
                    measure.data[1] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA4_INDEX];
                    measure.data[2] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA5_INDEX];
                    measure.data[3] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA6_INDEX];
                    measure.result = measure.result * 1000.0f;

                    // 所有测试使能引脚复位
                    AVO_PIN_Reset();

                    // LCD显示
                    Vertical_Scrolling_Display(avoTestMsgBuff, 4, 0);

                    switch (dut_info.dutBusType)
                    {
                        case 0: // 串口
                            switch (dut_info.test)
                            {
                                case TEST_TYPE_NULL:
                                    break;

                                case TEST_TYPE_UART:
                                    STATE_SwitchStep(STEP_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                // 大灯测试
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

                                // 近光灯
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

                                // 远光灯
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

                                // 左转向灯
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

                                // 右转向灯
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

                                // (小米)大灯测试
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
                                    
                                // (小米)油门测试
                                case TEST_TYPE_XM_THROTTLE:
                                    STATE_SwitchStep(STEP_THROTTLE_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    break;

                                // (小米)刹把测试
                                case TEST_TYPE_XM_BRAKE:
                                    STATE_SwitchStep(STEP_BRAKE_UART_TEST_CHECK_VOLTAGE_VALUE);
                                    break;
                                    
                                // (小米)左转向灯
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

                                // (小米)右转向灯
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

                                // 大灯测试
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

                                // 近光灯
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

                                // 远光灯
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

                                // 左转向灯
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

                                // 右转向灯
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

                // 电阻（Ω）
                case MEASURE_OHM :
                    ohm.data[0] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA3_INDEX];
                    ohm.data[1] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA4_INDEX];
                    ohm.data[2] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA5_INDEX];
                    ohm.data[3] = pCmdFrame->buff[AVO_PROTOCOL_CMD_DATA6_INDEX];
                    ohm.INT = ohm.INT;

                    // 所有测试使能引脚复位
                    AVO_PIN_Reset();

                    // LCD显示
                    Vertical_Scrolling_Display(avoTestMsgBuff, 4, 0);

                    switch (dut_info.dutBusType)
                    {
                        case 0: // 串口
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

    // 删除命令帧
    pCB->rx.head++;
    pCB->rx.head %= AVO_PROTOCOL_RX_QUEUE_SIZE;
}

// RX通讯超时处理-单向
#if AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void AVO_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
    AVO_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    pCmdFrame = &AVOProtocolCB.rx.cmdQueue[AVOProtocolCB.rx.end];

    // 超时错误，将命令帧长度清零，即认为抛弃该命令帧
    pCmdFrame->length = 0; // 2016.1.6增加
    // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
    AVOProtocolCB.rxFIFO.head++;
    AVOProtocolCB.rxFIFO.head %= AVO_PROTOCOL_RX_FIFO_SIZE;
    AVOProtocolCB.rxFIFO.currentProcessIndex = AVOProtocolCB.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void UART_PROTOCOL_StopRxTimeOutCheck(void)
{
    TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

//// 码表设置立刻下发数据请求标志
//void UART_PROTOCOL_SetTxAtOnceRequest(uint32 param)
//{
//    AVOProtocolCB.txAtOnceRequest = (BOOL)param;
//}

//// 码表设置周期下发数据请求标志
//void UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param)
//{
//    AVOProtocolCB.txPeriodRequest = (BOOL)param;
//}


