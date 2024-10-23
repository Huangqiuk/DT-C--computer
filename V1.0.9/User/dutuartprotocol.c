#include "common.h"
#include "timer.h"
#include "DutUartDrive.h"
#include "DutUartProtocol.h"
#include "StsProtocol.h"
#include "state.h"
#include "spiflash.h"
#include "AvoPin.h"
#include "AvoProtocol.h"
#include "param.h"
#include "DutInfo.h"
#include "PowerProtocol.h"
#include "stringOperation.h"

/******************************************************************************
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void DUT_PROTOCOL_DataStructInit(DUT_PROTOCOL_CB *pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void DUT_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// 对传入的命令帧进行校验，返回校验结果
BOOL DUT_PROTOCOL_CheckSUM(DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL DUT_PROTOCOL_ConfirmTempCmdFrameBuff(DUT_PROTOCOL_CB *pCB);

// UART协议层向驱动层注册数据发送接口
void DUT_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// 上报写配置参数结果
void DUT_PROTOCOL_ReportWriteParamResult(uint32 param); // 有用到

// 发送命令回复
void DUT_PROTOCOL_SendCmdAck(uint8 ackCmd);

// 发送命令回复，带一个参数
void DUT_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// 发送命令回复，带两个个参数
void DUT_PROTOCOL_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam);

// UART协议层过程处理
void DUT_UART_PROTOCOL_Process(void);

// 全局变量定义
extern char ptUpMsgBuff[][MAX_ONE_LINES_LENGTH];

DUT_PROTOCOL_CB uartProtocolCB3;
THROTTLE throttle;
BRAKE brake;
DERAILLEUR derailleur;
uint8 light_cnt = 0;
uint8 cnt = 0;
uint8 cali_cnt = 0;
uint8 brake_cnt = 0;
uint8 derailleur_cnt = 0;
uint8 left_light_cnt = 0;
uint8 right_light_cnt = 0;
BOOL write_success_Flag;
uint32 online_detection_cnt = 0;

// 协议初始化
void DUT_PROTOCOL_Init(void)
{
    // 协议层数据结构初始化
    DUT_PROTOCOL_DataStructInit(&uartProtocolCB3);

    // 向驱动层注册数据接收接口
    UART_DRIVE_RegisterDataSendService(DUT_PROTOCOL_MacProcess);

    // 向驱动层注册数据发送接口
    DUT_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);
}

// UART协议层过程处理
void DUT_UART_PROTOCOL_Process(void)
{
    // UART接收FIFO缓冲区处理
    DUT_PROTOCOL_RxFIFOProcess(&uartProtocolCB3);

    // UART接收命令缓冲区处理
    DUT_PROTOCOL_CmdFrameProcess(&uartProtocolCB3);

    // UART协议层发送处理过程
    DUT_PROTOCOL_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void DUT_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB3.tx.head;
    uint16 end = uartProtocolCB3.tx.end;
    DUT_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB3.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % DUT_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void DUT_PROTOCOL_TxAddFrame(void)
{
    uint16 cc = 0;
    uint16 i = 0;
    uint16 head = uartProtocolCB3.tx.head;
    uint16 end = uartProtocolCB3.tx.end;
    DUT_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB3.tx.end];
    uint16 length = pCmdFrame->length;

    // 发送缓冲区已满，不予接收
    if ((end + 1) % DUT_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if ((length >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
            || (length + 2 >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
    {
        pCmdFrame->length = 0;

        return;
    }

    // 队尾命令帧已满，退出
    if (length >= DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }


    // 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
    pCmdFrame->buff[DUT_PROTOCOL_CMD_LENGTH_INDEX] = length - 3; // 重设数据长度，减去"命令头、设备地址、命令字、数据长度"4个字节

    for (i = 0; i < length; i++)
    {
        cc ^= pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = cc ;


    uartProtocolCB3.tx.end++;
    uartProtocolCB3.tx.end %= DUT_PROTOCOL_TX_QUEUE_SIZE;
}

// 数据结构初始化
void DUT_PROTOCOL_DataStructInit(DUT_PROTOCOL_CB *pCB)
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
    for (i = 0; i < DUT_PROTOCOL_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < DUT_PROTOCOL_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART报文接收处理函数(注意根据具体模块修改)
void DUT_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB3.rxFIFO.end;
    uint16 head = uartProtocolCB3.rxFIFO.head;
    uint8 rxdata = 0x00;

    // 接收数据
    rxdata = *pData;

//    UART_PROTOCOL4_MacProcess(standarID, pData, length);
    
    // 一级缓冲区已满，不予接收
    if ((end + 1) % DUT_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // 一级缓冲区未满，接收
    else
    {
        // 将接收到的数据放到临时缓冲区中
        uartProtocolCB3.rxFIFO.buff[end] = rxdata;
        uartProtocolCB3.rxFIFO.end++;
        uartProtocolCB3.rxFIFO.end %= DUT_PROTOCOL_RX_FIFO_SIZE;
    }

    // 借用KM5S协议解析
    // DUT_PROTOCOL_MacProcess(standarID, pData, length);
}

// UART协议层向驱动层注册数据发送接口
void DUT_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB3.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL DUT_PROTOCOL_ConfirmTempCmdFrameBuff(DUT_PROTOCOL_CB *pCB)
{
    DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= DUT_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

    return TRUE;
}

// 协议层发送处理过程
void DUT_PROTOCOL_TxStateProcess(void)
{
    uint16 head = uartProtocolCB3.tx.head;
    uint16 end = uartProtocolCB3.tx.end;
    uint16 length = uartProtocolCB3.tx.cmdQueue[head].length;
    uint8 *pCmd = uartProtocolCB3.tx.cmdQueue[head].buff;
    uint16 localDeviceID = uartProtocolCB3.tx.cmdQueue[head].deviceID;

    // 发送缓冲区为空，说明无数据
    if (head == end)
    {
        return;
    }

    // 发送函数没有注册直接返回
    if (NULL == uartProtocolCB3.sendDataThrowService)
    {
        return;
    }

    // 协议层有数据需要发送到驱动层
    if (!(*uartProtocolCB3.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // 发送环形队列更新位置
    uartProtocolCB3.tx.cmdQueue[head].length = 0;
    uartProtocolCB3.tx.head++;
    uartProtocolCB3.tx.head %= DUT_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void DUT_PROTOCOL_RxFIFOProcess(DUT_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (DUT_PROTOCOL_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // 命令头正确，但无临时缓冲区可用，退出
        if ((pCB->rx.end + 1) % DUT_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
        {
            return;
        }

        // 添加UART通讯超时时间设置-2016.1.5增加
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
        TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
                      UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
                      UART_PROTOCOL_CALLBACK_RxTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
#endif

        // 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
        pCmdFrame->buff[pCmdFrame->length++] = currentData;
        pCB->rxFIFO.currentProcessIndex++;
        pCB->rxFIFO.currentProcessIndex %= DUT_PROTOCOL_RX_FIFO_SIZE;
    }
    // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
    else
    {
        // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
        if (pCmdFrame->length >= DUT_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
            pCmdFrame->length = 0; // 2016.1.5增加
            // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= DUT_PROTOCOL_RX_FIFO_SIZE;

            // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■

            // 首先判断命令帧最小长度，一个完整的命令字至少包括8个字节: 命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，因此不足8个字节的必定不完整
            if (pCmdFrame->length < DUT_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // 继续接收
                continue;
            }

            // 命令帧长度数值越界，说明当前命令帧错误，停止接收
            if (pCmdFrame->buff[DUT_PROTOCOL_CMD_LENGTH_INDEX] > DUT_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - DUT_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // 命令帧长度校验，在命令长度描述字的数值上，增加命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，即为命令帧实际长度
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[DUT_PROTOCOL_CMD_LENGTH_INDEX] + DUT_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // 长度要求不一致，说明未接收完毕，退出继续
                continue;
            }

            // 命令帧长度OK，则进行校验，失败时删除命令头
            if (!DUT_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= DUT_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            DUT_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// 对传入的命令帧进行校验，返回校验结果
BOOL DUT_PROTOCOL_CheckSUM(DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint16 cc = 0;
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

// UART命令帧缓冲区处理
void DUT_PROTOCOL_CmdFrameProcess(DUT_PROTOCOL_CB *pCB)
{
    DUT_PROTOCOL_CMD cmd = DUT_PROTOCOL_CMD_NULL;
    DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    uint8 switch_state = 0;
    uint8 Peripheral_type = 0;
    uint8 calibration_result = 0;
    uint8 turn_item = 0;
    uint8 turn_state = 0;
    uint8 buffFlag[4] = {0xFF};
    uint8 i =  0;
    uint8_t verType = 0;
    uint8_t infoLen = 0;
    char  verBuff[100] = {0};
    BOOL testResult = 0;

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
    if (DUT_PROTOCOL_CMD_HEAD != pCmdFrame->buff[DUT_PROTOCOL_CMD_HEAD_INDEX])
    {
        // 删除命令帧
        pCB->rx.head++;
        pCB->rx.head %= DUT_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // 命令头合法，则提取命令
    cmd = (DUT_PROTOCOL_CMD)pCmdFrame->buff[DUT_PROTOCOL_CMD_CMD_INDEX];

    // 执行命令帧
    switch (cmd)
    {
        // 空命令，不予执行
        case DUT_PROTOCOL_CMD_NULL:
            break;

        // 收到大灯应答
        case DUT_PROTOCOL_CMD_HEADLIGHT_CONTROL:
            switch_state = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];   // 执行结果
            light_cnt++;

            // 执行失败
            if (0x00 == switch_state)
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }

            // 执行成功
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

        case DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD:                      // 获取油门/刹车的模拟数字值
            Peripheral_type = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];   // 外设类型

            // 油门
            if (0x01 == (Peripheral_type & 0x01))
            {
                // 获取油门AD值
                throttle.value = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX] << 8;
                throttle.value = throttle.value | pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX];
                dut_info.throttleAd = throttle.value;
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

            // 刹把
            if (0x02 == (Peripheral_type & 0x02))
            {
                // 获取刹把AD值
                brake.value = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX] << 8;
                brake.value = brake.value | pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX];
                dut_info.brakeAd = brake.value;
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

            // 电子变速
            if (0x03 == (Peripheral_type & 0x03))
            {
                // 获取刹把AD值
                derailleur.value = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX] << 8;
                derailleur.value = derailleur.value | pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX];
                dut_info.derailleurAd = derailleur.value;
                derailleur_cnt++;
                
                switch (derailleur_cnt)
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
                        derailleur_cnt = 0;
                        break;

                    default:
                        break;
                }
            }
            
            break;

        // 转向灯控制
        case DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL:
            turn_item = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];   // 转向灯类型
            turn_state = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX];  // 执行结果
            light_cnt++;

            // 近光灯
            if ((turn_state) && (dut_info.test == TEST_TYPE_LBEAM) && (LBEAM == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_LBEAM_UART_TEST_OPEN);
                }
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_LBEAM_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }

            // 远光灯
            if ((turn_state) && (dut_info.test == TEST_TYPE_HBEAM) && (HBEAM == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_HBEAM_UART_TEST_OPEN);
                }
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_HBEAM_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }

            // 左转向灯
            if ((turn_state) && (dut_info.test == TEST_TYPE_LEFT_TURN_SIGNAL) && (LEFT_TURN_SIGNAL == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN);
                }
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }

            // 右转向灯
            if ((turn_state) && (dut_info.test == TEST_TYPE_RIGHT_TURN_SIGNAL) && (RIGHT_TURN_SIGNAL == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN);
                }
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }

            // 执行失败
            if (!switch_state)
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(1, "Fail to turn on");
    #endif
                left_light_cnt = 0;
                right_light_cnt = 0;
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        // 收到仪表的校准结果
        case DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION:
            cali_cnt++;
            calibration_result = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];

            // 一级校准
            if ((calibration_result) && (dut_info.cali_cnt == 1))
            {
                STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_RESTORE_VOLTAGE_CALIBRATION);
                cali_cnt = 0;
            }

            // 二级校准
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

            // 三级校准
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

            // 校准失败
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

        // 在线检测
        case DUT_PROTOCOL_CMD_ONLINE_DETECTION:
            online_detection_cnt++;            
            TIMER_ResetTimer(TIMER_ID_ONLINE_DETECT);
            
            if(1 == online_detection_cnt)
            {
               // 上报仪表启动状态
               STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION,TRUE);
            }
            break;

        // 读取标志区数据
        case DUT_PROTOCOL_CMD_READ_FLAG_DATA:
            TIMER_KillTask(TIMER_ID_WRITE_FLAG_TIMEOUT);

            // 读取标志区数据
            for (i = 0; i < 4; i++)
            {
                buffFlag[i] = pCmdFrame->buff[(stationNumber * 4) + 4 + i];
            }

            // 校验标志区数据
            if ((buffFlag[0] == 0x00) && (buffFlag[1] == 0x00) && (buffFlag[2] == 0x00) && (buffFlag[3] == 0x02))
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, TRUE);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
            }
            break;

        // 写入标志区数据
        case DUT_PROTOCOL_CMD_WRITE_FLAG_DATA:

            // 关闭写入超时定时器
            TIMER_ResetTimer(TIMER_ID_WRITE_FLAG_TIMEOUT);

            // 收到写入成功再读取出来进行校验
            if (pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX])
            {
                DUT_PROTOCOL_SendCmdAck(DUT_PROTOCOL_CMD_READ_FLAG_DATA);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
            }
            break;

        // 写入版本类型数据
        case DUT_PROTOCOL_CMD_WRITE_VERSION_TYPE_DATA:

            // 关闭定时器
            TIMER_ResetTimer(TIMER_ID_OPERATE_VER_TIMEOUT);

            // 获取类型
            verType =  pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];

            // 类型正确并且执行成功，上报STS
            if ((dutverType == verType) && (pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX]))
            {
                // 读取版本类型信息
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_READ_VERSION_TYPE_INFO, dutverType);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_VERSION_INFO, FALSE);
            }
            break;

        // 读取版本类型信息
        case DUT_PROTOCOL_CMD_READ_VERSION_TYPE_INFO:
            TIMER_KillTask(TIMER_ID_OPERATE_VER_TIMEOUT);

            // 获取类型
            verType =  pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX];

            // 获取信息长度
            infoLen = pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX];

            // 获取类型信息
            strcpy(verBuff, (const char *)&pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX]);
            verBuff[infoLen] = 0;

            // 只读取
            if (verreadFlag)
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                STS_PROTOCOL_TxAddData(0x00);       // 数据长度
                STS_PROTOCOL_TxAddData(1);          // 校验读取结果
                STS_PROTOCOL_TxAddData(verType);    // 版本类型
                STS_PROTOCOL_TxAddData(infoLen);    // 信息长度

                // 类型信息
                for (i = 0; i < infoLen; i++)
                {
                    STS_PROTOCOL_TxAddData(pCmdFrame->buff[5 + i]);
                }
                STS_PROTOCOL_TxAddFrame();     // 版本类型
                verreadFlag = FALSE;
            }

            // 写入对比
            if (verwriteFlag)
            {
                // 版本对比
                if ((dutverType == verType) && (strstr((const char *)dutverBuff, (char *)verBuff) != NULL))
                {
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                    STS_PROTOCOL_TxAddData(0x00);       // 数据长度
                    STS_PROTOCOL_TxAddData(1);          // 校验读取结果
                    STS_PROTOCOL_TxAddData(verType);    // 版本类型
                    STS_PROTOCOL_TxAddData(infoLen);    // 信息长度

                    // 类型信息
                    for (i = 0; i < infoLen; i++)
                    {
                        STS_PROTOCOL_TxAddData(pCmdFrame->buff[5 + i]);
                    }
                    STS_PROTOCOL_TxAddFrame();     // 版本类型
                }
                else
                {
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                    STS_PROTOCOL_TxAddData(0x00);       // 数据长度
                    STS_PROTOCOL_TxAddData(0);          // 校验读取结果
                    STS_PROTOCOL_TxAddData(verType);    // 版本类型
                    STS_PROTOCOL_TxAddData(infoLen);    // 信息长度

                    // 类型信息
                    for (i = 0; i < infoLen; i++)
                    {
                        STS_PROTOCOL_TxAddData(pCmdFrame->buff[5 + i]);
                    }
                    STS_PROTOCOL_TxAddFrame();     // 版本类型
                }
                verwriteFlag = FALSE;
            }

            break;

        // 蓝牙测试
        case DUT_PROTOCOL_CMD_BLUETOOTH_MAC_ADDRESS_READ:
            
            // 将蓝牙版本复制到缓冲区
            strcpy(dut_info.bleVerBuff, &pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA14_INDEX]);
            
            // 将校验码赋为0
            dut_info.bleVerBuff[pCmdFrame->buff[DUT_PROTOCOL_CMD_LENGTH_INDEX] - 13] =  0;

            // 版本比对成功将MAC地址发给电源板连接
            if (pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX])
            {
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT);
                POWER_PROTOCOL_TxAddData(12);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA4_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA5_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA6_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA7_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA8_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA9_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA10_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA11_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA12_INDEX]);
                POWER_PROTOCOL_TxAddData(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA13_INDEX]);                
                POWER_PROTOCOL_TxAddFrame();
            }

            // 版本比对失败上报STS
            if (0 == pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX])
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                STS_PROTOCOL_TxAddData(7);
                STS_PROTOCOL_TxAddData(1);
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA2_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA3_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA4_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA5_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA6_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA7_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA8_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA9_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA10_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA11_INDEX]) & 0x0F)); 
                STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA12_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA13_INDEX]) & 0x0F)); 
                
                for(i = 0; i < 20 ;i++)
                {
                  STS_PROTOCOL_TxAddData(dut_info.bleVerBuff[i]);
                }
                              
                STS_PROTOCOL_TxAddFrame();
            }
            break;

        // LCD 颜色测试（命令字=0x03）
        case DUT_PROTOCOL_CMD_TEST_LCD:
            STS_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_TEST_LCD, TRUE);
            break;

        // Flash 校验测试
        case DUT_PROTOCOL_CMD_FLASH_CHECK_TEST:
            TIMER_ChangeTime(TIMER_ID_ONLINE_DETECT, 3000);
            STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
            break;

        // 按键测试
        case DUT_PROTOCOL_CMD_KEY_TEST:
        
            // 按键模拟测试
            if(dut_info.buttonSimulationFlag)
            {
                if(keyValue == pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA1_INDEX])
                {
                     testResult = TRUE;
                }
                else
                {
                     testResult = FALSE;
                }
                STS_PROTOCOL_SendCmdParamTwoAck(STS_PROTOCOL_CMD_SIMULATION_KEY_TEST, testResult, pCmdFrame->buff[DUT_PROTOCOL_CMD_DATA12_INDEX]);                
            }
            // 按键正常测试
            else
            {
                STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);         

                // LCD显示
                Vertical_Scrolling_Display(ptUpMsgBuff, 4, 0);                
            }
            break;
            
        default:
            STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);

            // LCD显示
            Vertical_Scrolling_Display(ptUpMsgBuff, 4, 0);     
            break;
    }

    // 删除命令帧
    pCB->rx.head++;
    pCB->rx.head %= DUT_PROTOCOL_RX_QUEUE_SIZE;
}

// 发送命令回复
void DUT_PROTOCOL_SendCmdAck(uint8 ackCmd)
{
    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_HEAD);
    DUT_PROTOCOL_TxAddData(ackCmd);
    DUT_PROTOCOL_TxAddData(0x00);         // 数据长度
    DUT_PROTOCOL_TxAddFrame();
}

// 发送命令回复，带一个参数
void DUT_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{

    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_HEAD);
    DUT_PROTOCOL_TxAddData(ackCmd);
    DUT_PROTOCOL_TxAddData(0x01);

    DUT_PROTOCOL_TxAddData(ackParam);
    DUT_PROTOCOL_TxAddFrame();
}

// 发送命令回复，带两个个参数
void DUT_PROTOCOL_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam)
{
    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_HEAD);
    DUT_PROTOCOL_TxAddData(ackCmd);
    DUT_PROTOCOL_TxAddData(0x02);

    DUT_PROTOCOL_TxAddData(ackParam);
    DUT_PROTOCOL_TxAddData(two_ackParam);
    DUT_PROTOCOL_TxAddFrame();
}

// 上报在线状态
//void DUT_PROTOCOL_SendOnlineStart(uint32 param)
//{
//    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION, SHUTDOWN);
//}

// 写入标志区数据
void DUT_PROTOCOL_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam)
{

    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_HEAD);
    DUT_PROTOCOL_TxAddData(DUT_PROTOCOL_CMD_WRITE_FLAG_DATA);
    DUT_PROTOCOL_TxAddData(0x06);

    DUT_PROTOCOL_TxAddData(placeParam);
    DUT_PROTOCOL_TxAddData(0x00);
    DUT_PROTOCOL_TxAddData(0x00);
    DUT_PROTOCOL_TxAddData(0x00);
    DUT_PROTOCOL_TxAddData(0x02);
    DUT_PROTOCOL_TxAddData(shutdownFlagParam);
    DUT_PROTOCOL_TxAddFrame();
}

