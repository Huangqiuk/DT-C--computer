#include "common.h"
#include "PowerDrive.h"
#include "PowerProtocol.h"
#include "timer.h"
#include "DutUartProtocol.h"
#include "param.h"
#include "State.h"
#include "Spiflash.h"
#include "system.h"
#include "iap.h"
#include "state.h"
#include "StsProtocol.h"
#include "AvoPin.h"
#include "DutInfo.h"

/******************************************************************************
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void POWER_PROTOCOL_DataStructInit(POWER_PROTOCOL_CB *pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void POWER_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void POWER_PROTOCOL_RxFIFOProcess(POWER_PROTOCOL_CB *pCB);

// UART命令帧缓冲区处理
void POWER_PROTOCOL_CmdFrameProcess(POWER_PROTOCOL_CB *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL POWER_PROTOCOL_CheckSUM(POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL POWER_PROTOCOL_ConfirmTempCmdFrameBuff(POWER_PROTOCOL_CB *pCB);

// 协议层发送处理过程
void POWER_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void POWER_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================

// 仪表状态信息回复
void POWER_PROTOCOL_SendCmdStatusAck(void);

//=======================================================================================
// 全局变量定义
POWER_PROTOCOL_CB POWERProtocolCB;
VOUT_CURRENT vout_Current;
uint8 power_cnt = 0;
uint8 switch_cnt = 0;

// 协议初始化
void POWER_PROTOCOL_Init(void)
{
    // 协议层数据结构初始化
    POWER_PROTOCOL_DataStructInit(&POWERProtocolCB);

    // 向驱动层注册数据接收接口
    POWER_UART_RegisterDataSendService(POWER_PROTOCOL_MacProcess);

    // 向驱动层注册数据发送接口
    POWER_PROTOCOL_RegisterDataSendService(POWER_UART_AddTxArray);
}

// POWER协议层过程处理
void POWER_PROTOCOL_Process(void)
{
    // POWER接收FIFO缓冲区处理
    POWER_PROTOCOL_RxFIFOProcess(&POWERProtocolCB);

    // POWER接收命令缓冲区处理
    POWER_PROTOCOL_CmdFrameProcess(&POWERProtocolCB);

    // POWER协议层发送处理过程
    POWER_PROTOCOL_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void POWER_PROTOCOL_TxAddData(uint8 data)
{
    uint16 head = POWERProtocolCB.tx.head;
    uint16 end = POWERProtocolCB.tx.end;
    POWER_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &POWERProtocolCB.tx.cmdQueue[POWERProtocolCB.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % POWER_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void POWER_PROTOCOL_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = POWERProtocolCB.tx.head;
    uint16 end = POWERProtocolCB.tx.end;
    POWER_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &POWERProtocolCB.tx.cmdQueue[POWERProtocolCB.tx.end];
    uint16 length = pCmdFrame->length;

    // 发送缓冲区已满，不予接收
    if ((end + 1) % POWER_PROTOCOL_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 命令帧长度不足，清除已填充的数据，退出
    if (POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN - 4 > length)  // 减去"校验和L、校验和H、结束标识0xD、结束标识OxA"4个字节
    {
        pCmdFrame->length = 0;

        return;
    }

    // 队尾命令帧已满，退出
    if ((length >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
            || (length + 2 >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX))
    {
        pCmdFrame->length = 0;

        return;
    }

    // 队尾命令帧已满，退出
    if (length >= POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
    pCmdFrame->buff[POWER_PROTOCOL_CMD_LENGTH_INDEX] = length - 4;   // 重设数据长度，减去"命令头、设备地址、命令字、数据长度"4个字节
    for (i = 1; i < length; i++)
    {
        checkSum += pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);         // 低字节在前
    pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF);  // 高字节在后

    // 结束标识
    pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
    pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

    POWERProtocolCB.tx.end++;
    POWERProtocolCB.tx.end %= POWER_PROTOCOL_TX_QUEUE_SIZE;
    // pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

// 数据结构初始化
void POWER_PROTOCOL_DataStructInit(POWER_PROTOCOL_CB *pCB)
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
    for (i = 0; i < POWER_PROTOCOL_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < POWER_PROTOCOL_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }

    pCB->isTimeCheck = FALSE;
}

// UART报文接收处理函数(注意根据具体模块修改)
void POWER_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = POWERProtocolCB.rxFIFO.end;
    uint16 head = POWERProtocolCB.rxFIFO.head;
    uint8 rxdata = 0x00;

    // 接收数据
    rxdata = *pData;

    // 一级缓冲区已满，不予接收
    if ((end + 1) % POWER_PROTOCOL_RX_FIFO_SIZE == head)
    {
        return;
    }
    // 一级缓冲区未满，接收
    else
    {
        // 将接收到的数据放到临时缓冲区中
        POWERProtocolCB.rxFIFO.buff[end] = rxdata;
        POWERProtocolCB.rxFIFO.end++;
        POWERProtocolCB.rxFIFO.end %= POWER_PROTOCOL_RX_FIFO_SIZE;
    }
}

// UART协议层向驱动层注册数据发送接口
void POWER_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    POWERProtocolCB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL POWER_PROTOCOL_ConfirmTempCmdFrameBuff(POWER_PROTOCOL_CB *pCB)
{
    POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= POWER_PROTOCOL_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

    return TRUE;
}

// 协议层发送处理过程
void POWER_PROTOCOL_TxStateProcess(void)
{
    uint16 head = POWERProtocolCB.tx.head;
    uint16 end = POWERProtocolCB.tx.end;
    uint16 length = POWERProtocolCB.tx.cmdQueue[head].length;
    uint8 *pCmd = POWERProtocolCB.tx.cmdQueue[head].buff;
    uint16 localDeviceID = POWERProtocolCB.tx.cmdQueue[head].deviceID;

    // 发送缓冲区为空，说明无数据
    if (head == end)
    {
        return;
    }

    // 发送函数没有注册直接返回
    if (NULL == POWERProtocolCB.sendDataThrowService)
    {
        return;
    }

    // 协议层有数据需要发送到驱动层
    if (!(*POWERProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // 发送环形队列更新位置
    POWERProtocolCB.tx.cmdQueue[head].length = 0;
    POWERProtocolCB.tx.head++;
    POWERProtocolCB.tx.head %= POWER_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void POWER_PROTOCOL_RxFIFOProcess(POWER_PROTOCOL_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (POWER_PROTOCOL_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // 命令头正确，但无临时缓冲区可用，退出
        if ((pCB->rx.end + 1) % POWER_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex %= POWER_PROTOCOL_RX_FIFO_SIZE;
    }
    // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
    else
    {
        // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
        if (pCmdFrame->length >= POWER_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
        {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
            pCmdFrame->length = 0; // 2016.1.5增加
            // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= POWER_PROTOCOL_RX_FIFO_SIZE;

            // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正 ■■

            // 首先判断命令帧最小长度，一个完整的命令字至少包括8个字节: 命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，因此不足8个字节的必定不完整
            if (pCmdFrame->length < POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // 继续接收
                continue;
            }

            // 命令帧长度数值越界，说明当前命令帧错误，停止接收
            if (pCmdFrame->buff[POWER_PROTOCOL_CMD_LENGTH_INDEX] > (POWER_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN))
            {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // 命令帧长度校验
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[POWER_PROTOCOL_CMD_LENGTH_INDEX] + POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN)
            {
                // 长度要求不一致，说明未接收完毕，退出继续
                continue;
            }

            // 命令帧长度OK，则进行校验，失败时删除命令头
            if (!POWER_PROTOCOL_CheckSUM(pCmdFrame))
            {
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            BLE_PROTOCOL_StopRxTimeOutCheck();
#endif
            // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            POWER_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// 对传入的命令帧进行校验，返回校验结果
BOOL POWER_PROTOCOL_CheckSUM(POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
    uint16 checkSum = 0;
    uint16 sumTemp;
    uint16 i = 0;

    if (NULL == pCmdFrame)
    {
        return FALSE;
    }

    // 从设备地址开始，到校验码之前的一个字节，依次进行累加运算
    for (i = 1; i < pCmdFrame->length - 4; i++)
    {
        checkSum += pCmdFrame->buff[i];
    }

    // 累加和，低字节在前，高字节在后
    sumTemp = pCmdFrame->buff[pCmdFrame->length - 3]; // 高字节
    sumTemp <<= 8;
    sumTemp += pCmdFrame->buff[pCmdFrame->length - 4]; // 低字节

    // 判断计算得到的校验码与命令帧中的校验码是否相同
    if (sumTemp != checkSum)
    {
        return FALSE;
    }

    return TRUE;

}

// POWER命令帧缓冲区处理
void POWER_PROTOCOL_CmdFrameProcess(POWER_PROTOCOL_CB *pCB)
{
    POWER_PROTOCOL_CMD cmd = POWER_PROTOCOL_CMD_NULL;
    POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
    uint8 calibration_result = 0;
    uint16 i = 0;
    // 我的变量

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
    if (POWER_PROTOCOL_CMD_HEAD != pCmdFrame->buff[POWER_PROTOCOL_CMD_HEAD_INDEX])
    {
        // 删除命令帧
        pCB->rx.head++;
        pCB->rx.head %= POWER_PROTOCOL_RX_QUEUE_SIZE;
        return;
    }

    // 命令头合法，则提取命令
    cmd = (POWER_PROTOCOL_CMD)pCmdFrame->buff[POWER_PROTOCOL_CMD_CMD_INDEX];

    // 执行命令帧
    switch (cmd)
    {
        case POWER_PROTOCOL_CMD_NULL:                                   // 空命令
            break;

        case POWER_PROTOCOL_CMD_CHECK_EXTERNAL_POWER:                   // 检测是否有外接电源接入（命令字=0x01）
            break;

        case POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_VOLTAGE:               // 仪表供电电压获取（命令字=0x02）
            break;

        case POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_CURRENT:               // 仪表供电电流获取（命令字=0x03）
            vout_Current.value[0] = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX];
            vout_Current.value[1] = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA2_INDEX];
            vout_Current.value[2] = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA3_INDEX];
            vout_Current.value[3] = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA4_INDEX];
            vout_Current.CURRENT = vout_Current.CURRENT;
            break;

        case POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE:        // 调整DUT供电电压（命令字=0x04）
            calibration_result = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX];
            power_cnt++;

            // 一级校准
            if ((calibration_result) && (dut_info.cali_cnt == 1))
            {
                if (power_cnt == 1)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 2)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_COMPLETE);
                    power_cnt = 0;
                }

            }

            // 二级校准
            if ((calibration_result) && (dut_info.cali_cnt == 2))
            {

                if (power_cnt == 1)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 2)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 3)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_COMPLETE);
                    power_cnt = 0;
                }

            }

            // 三级校准
            if ((calibration_result) && (dut_info.cali_cnt == 3))
            {

                if (power_cnt == 1)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 2)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 3)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION);
                }
                if (power_cnt == 4)
                {
                    STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_COMPLETE);
                    power_cnt = 0;
                }
            }

            // 调整DUT供电电压
            if ((calibration_result) && (dut_info.adjustState))
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ADJUST_DUT_VOLTAGE, TRUE);
                dut_info.adjustState = FALSE;
                power_cnt = 0;

                // vlk供电
                VLK_PW_EN_OFF();
            }

            // 上报结果
            if ((!calibration_result) && (dut_info.adjustState == FALSE))
            {
                Clear_All_Lines();
                Display_Centered(0, "Calibration");
                Display_Centered(1, "Power adjustment");
                Display_Centered(2, "failure");
                power_cnt = 0;
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, FALSE);
            }
            break;

        // DUT电源控制
        case POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH:
            switch_cnt++;

            calibration_result = pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX];

            // DUT电源控制
            if ((switch_cnt) && (dut_info.powerOnFlag))
            {
                switch_cnt = 0;
                dut_info.powerOnFlag = FALSE;
                
//                if(dut_info.passThroughControl)
//                {
//                     break;
//                }
//                else
//                {
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CONTROL_DUT_POWER, TRUE);                
//                }
            }

            // 仪表升级APP重启
            if ((1 == switch_cnt) && (resetFlag == TRUE)  && (configResetFlag == FALSE))
            {
                Delayms(1000); // 延时1s,充分放电

                switch (dut_info.ID)
                {
                case DUT_TYPE_CM:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_RESTART);
                    }
                    break;

                case DUT_TYPE_SEG:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_RESTART);
                    }
                    break;

                default:
                    break;
                }
            }

            if ((2 == switch_cnt) && (resetFlag == TRUE)  && (configResetFlag == FALSE))
            {
                switch (dut_info.ID)
                {
                case DUT_TYPE_CM:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
                    }
                    break;

                case DUT_TYPE_SEG:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
                    }
                    break;

                default:
                    break;
                }
                switch_cnt = 0;
                resetFlag = FALSE;
            }

            // 仪表升级成功重启
            if ((appResetFlag) && (switch_cnt))
            {
                Delayms(300); // 延时300ms，充分放电

                // vlk供电
                VLK_PW_EN_ON();

                // 开启仪表电源
                POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);

                switch (dut_info.ID)
                {
                case DUT_TYPE_CM:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
                    }
                    break;

                case DUT_TYPE_SEG:
                    //                        if (dut_info.dutBusType)
                    //                        {
                    //                            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    //                        }
                    //                        else
                    //                        {
                    STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
                    //                        }
                    break;

                default:
                    break;
                }
            }
            if ((appResetFlag) && (2 == switch_cnt))
            {
                switch (dut_info.ID)
                {
                case DUT_TYPE_CM:
                    if (dut_info.dutBusType)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_YELLOW);
                    }
                    else
                    {
                        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
                    }
                    break;

                case DUT_TYPE_SEG:
                    //                        if (dut_info.dutBusType)
                    //                        {
                    //                            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_RESTART);
                    //                        }
                    //                        else
                    //                        {
                    STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH);
                    //                        }
                    break;

                default:
                    break;
                }
                appResetFlag = FALSE;
                switch_cnt = 0;
            }

            // 仪表升级配置重启
            if ((1 == switch_cnt) && (resetFlag == FALSE) && (configResetFlag == TRUE))
            {
                Delayms(1000); // 延时1s，充分放电

                if (dut_info.dutBusType)
                {
                    STATE_SwitchStep(STEP_CAN_CONFIG_RESET);
                }
                else
                {
                    STATE_SwitchStep(STEP_UART_CONFIG_RESET);
                }
            }

            if ((2 == switch_cnt) && (resetFlag == FALSE) && (configResetFlag == TRUE))
            {
                if (dut_info.dutBusType)
                {
                    STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

                    // 发送配置
                    TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                                  500,
                                  STATE_SwitchStep,
                                  STEP_CAN_SEND_SET_CONFIG,
                                  TIMER_LOOP_FOREVER,
                                  ACTION_MODE_ADD_TO_QUEUE);
                }
                else
                {
                    TIMER_AddTask(TIMER_ID_PROTOCOL_SWITCCH,
                                  500,
                                  STATE_SwitchStep,
                                  STEP_UART_PROTOCOL_SWITCCH,
                                  TIMER_LOOP_FOREVER,
                                  ACTION_MODE_ADD_TO_QUEUE);
                }

                switch_cnt = 0;
                configResetFlag = FALSE;
            }
            break;

        // 蓝牙测试
        case POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT:

            // 测试成功
            if (pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX])
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                STS_PROTOCOL_TxAddData(0x07);
                STS_PROTOCOL_TxAddData(2);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA2_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA3_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA4_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA5_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA6_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA7_INDEX]);
                
                for(i = 0; i < 20 ;i++)
                {
                  STS_PROTOCOL_TxAddData(dut_info.bleVerBuff[i]);
                }
                
                STS_PROTOCOL_TxAddFrame();
            }

            // 无法连接测试失败
            if (!pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX])
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                STS_PROTOCOL_TxAddData(0x07);
                STS_PROTOCOL_TxAddData(0);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA2_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA3_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA4_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA5_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA6_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA7_INDEX]);
                
                for(i = 0; i < 20 ;i++)
                {
                  STS_PROTOCOL_TxAddData(dut_info.bleVerBuff[i]);
                }
                STS_PROTOCOL_TxAddFrame();
            }
            break;

        // 获取POWER软件信息
        case POWER_PROTOCOL_CMD_GET_SOFTWARE_INFO:
            STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
            STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_POWER_GET_SOFTWARE_INFO);
            STS_PROTOCOL_TxAddData(6);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA2_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA3_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA4_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA5_INDEX]);
            STS_PROTOCOL_TxAddData(pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA6_INDEX]);
            STS_PROTOCOL_TxAddFrame();
            break;

        // APP2数据擦除
        case POWER_PROTOCOL_CMD_ECO_APP2_ERASE:

            // 上报擦除结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_POWER_UP_APP_ERASE, pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            break;

        // APP2数据写入
        case POWER_PROTOCOL_CMD_ECO_APP2_WRITE:

            // 上报写入结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_POWER_UP_APP_WRITE, pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            break;

        // APP2数据完成
        case POWER_PROTOCOL_CMD_ECO_APP_WRITE_FINISH:

            // 上报写入结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_POWER_UP_APP_WRITE_FINISH, pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            break;

        // 重启命令
        case POWER_PROTOCOL_CMD_RST:

            // 上报重启结果
            STS_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_RST, pCmdFrame->buff[POWER_PROTOCOL_CMD_DATA1_INDEX]);
            break;

        // 升级成功上报
        case POWER_PROTOCOL_CMD_UP_FINISH:

            // 上报升级结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_POWER_UP_FINISH, 1);
            POWER_PROTOCOL_SendCmdAck(POWER_PROTOCOL_CMD_UP_FINISH);
            break;

        default:
            break;
        }

    // 删除命令帧
    pCB->rx.head++;
    pCB->rx.head %= POWER_PROTOCOL_RX_QUEUE_SIZE;
}

// RX通讯超时处理-单向
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void BLE_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
    POWER_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

    pCmdFrame = &POWERProtocolCB.rx.cmdQueue[POWERProtocolCB.rx.end];

    // 超时错误，将命令帧长度清零，即认为抛弃该命令帧
    pCmdFrame->length = 0; // 2016.1.6增加
    // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
    POWERProtocolCB.rxFIFO.head++;
    POWERProtocolCB.rxFIFO.head %= POWER_PROTOCOL_RX_FIFO_SIZE;
    POWERProtocolCB.rxFIFO.currentProcessIndex = POWERProtocolCB.rxFIFO.head;
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
void POWER_PROTOCOL_SendCmdAck(uint8 ackCmd)
{
    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
    POWER_PROTOCOL_TxAddData(ackCmd);
    POWER_PROTOCOL_TxAddData(0x00);
    POWER_PROTOCOL_TxAddFrame();
}

// 发送命令回复，带一个参数
void POWER_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
    POWER_PROTOCOL_TxAddData(ackCmd);
    POWER_PROTOCOL_TxAddData(0x01);
    POWER_PROTOCOL_TxAddData(ackParam);
    POWER_PROTOCOL_TxAddFrame();
}


