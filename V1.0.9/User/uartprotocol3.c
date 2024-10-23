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
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void UART_PROTOCOL4_DataStructInit(UART_PROTOCOL4_CB *pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void UART_PROTOCOL4_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// 对传入的命令帧进行校验，返回校验结果
BOOL UART_PROTOCOL4_CheckSUM(UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL UART_PROTOCOL4_ConfirmTempCmdFrameBuff(UART_PROTOCOL4_CB *pCB);

// UART协议层向驱动层注册数据发送接口
void UART_PROTOCOL4_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// 上报写配置参数结果
void UART_PROTOCOL4_ReportWriteParamResult(uint32 param); // 有用到

void UART_PROTOCOL4_22(uint32 param); // 测试

// 全局变量定义
UART_PROTOCOL4_CB uartProtocolCB4;

UART_PROTOCOL4_PARAM_CB hmiDriveCB3;

// 协议初始化
void UART_PROTOCOL4_Init(void)
{
    // 协议层数据结构初始化
    UART_PROTOCOL4_DataStructInit(&uartProtocolCB4);

    // 向驱动层注册数据接收接口
    UART_DRIVE_RegisterDataSendService(UART_PROTOCOL4_MacProcess);

    // 向驱动层注册数据发送接口
    UART_PROTOCOL4_RegisterDataSendService(UART_DRIVE_AddTxArray);

    //  TIMER_AddTask(TIMER_ID_PROTOCOL_3A_PARAM_TX,
    //                  200,
    //                  UART_PROTOCOL4_22,
    //                  TRUE,
    //                  TIMER_LOOP_FOREVER,
    //                  ACTION_MODE_ADD_TO_QUEUE);
}

// 向发送命令帧队列中添加数据
void UART_PROTOCOL4_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end = uartProtocolCB4.tx.end;
    UART_PROTOCOL4_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB4.tx.cmdQueue[uartProtocolCB4.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % UART_PROTOCOL4_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void UART_PROTOCOL4_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end = uartProtocolCB4.tx.end;
    UART_PROTOCOL4_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB4.tx.cmdQueue[uartProtocolCB4.tx.end];
    uint16 length = pCmdFrame->length;

    // 发送缓冲区已满，不予接收
    if ((end + 1) % UART_PROTOCOL4_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 命令帧长度不足，清除已填充的数据，退出
    if (UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN - 4 > length) // 减去"校验和L、校验和H、结束标识0xD、结束标识OxA"4个字节
    {
        pCmdFrame->length = 0;

        return;
    }

    // 队尾命令帧已满，退出
    if ((length >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX) || (length + 2 >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX))
    {
        return;
    }

    // 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
    pCmdFrame->buff[UART_PROTOCOL4_CMD_LENGTH_INDEX] = length - 4; // 重设数据长度，减去"命令头、设备地址、命令字、数据长度"4个字节
    for (i = 1; i < length; i++)
    {
        checkSum += pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);        // 低字节在前
    pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF); // 高字节在后

    // 结束标识
    pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
    pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

    uartProtocolCB4.tx.end++;
    uartProtocolCB4.tx.end %= UART_PROTOCOL4_TX_QUEUE_SIZE;
}

// 数据结构初始化
void UART_PROTOCOL4_DataStructInit(UART_PROTOCOL4_CB *pCB)
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

// UART报文接收处理函数(注意根据具体模块修改)
void UART_PROTOCOL4_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB4.rxFIFO.end;
    uint16 head = uartProtocolCB4.rxFIFO.head;
    uint8 rxdata = 0x00;

    // 接收数据
    rxdata = *pData;

    // 借用KM5S协议解析
    UART_PROTOCOL_MacProcess(standarID, pData, length);
    UART_PROTOCOL_XM_MacProcess(standarID, pData, length);
    
    // 一级缓冲区已满，不予接收
    if ((end + 1) % UART_PROTOCOL4_RX_FIFO_SIZE == head)
    {
        return;
    }
    // 一级缓冲区未满，接收
    else
    {
        // 将接收到的数据放到临时缓冲区中
        uartProtocolCB4.rxFIFO.buff[end] = rxdata;
        uartProtocolCB4.rxFIFO.end++;
        uartProtocolCB4.rxFIFO.end %= UART_PROTOCOL4_RX_FIFO_SIZE;
    }
}

// UART协议层向驱动层注册数据发送接口
void UART_PROTOCOL4_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB4.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL UART_PROTOCOL4_ConfirmTempCmdFrameBuff(UART_PROTOCOL4_CB *pCB)
{
    UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= UART_PROTOCOL4_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

    return TRUE;
}

// 协议层发送处理过程
void UART_PROTOCOL4_TxStateProcess(void)
{
    uint16 head = uartProtocolCB4.tx.head;
    uint16 end = uartProtocolCB4.tx.end;
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
    uartProtocolCB4.tx.head++;
    uartProtocolCB4.tx.head %= UART_PROTOCOL4_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void UART_PROTOCOL4_RxFIFOProcess(UART_PROTOCOL4_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (UART_PROTOCOL4_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL4_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // 命令头正确，但无临时缓冲区可用，退出
        if ((pCB->rx.end + 1) % UART_PROTOCOL4_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL4_RX_FIFO_SIZE;
    }
    // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
    else
    {
        // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
        if (pCmdFrame->length >= UART_PROTOCOL4_RX_CMD_FRAME_LENGTH_MAX)
        {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
            pCmdFrame->length = 0; // 2016.1.5增加
            // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL4_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;

            // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■

            // 首先判断命令帧最小长度，一个完整的命令字至少包括8个字节: 命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，因此不足8个字节的必定不完整
            if (pCmdFrame->length < UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN)
            {
                // 继续接收
                continue;
            }

            // 命令帧长度数值越界，说明当前命令帧错误，停止接收
            if (pCmdFrame->buff[UART_PROTOCOL4_CMD_LENGTH_INDEX] > UART_PROTOCOL4_RX_CMD_FRAME_LENGTH_MAX - UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN)
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL4_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // 命令帧长度校验，在命令长度描述字的数值上，增加命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，即为命令帧实际长度
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[UART_PROTOCOL4_CMD_LENGTH_INDEX] + UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN)
            {
                // 长度要求不一致，说明未接收完毕，退出继续
                continue;
            }

            // 命令帧长度OK，则进行校验，失败时删除命令头
            if (!UART_PROTOCOL4_CheckSUM(pCmdFrame))
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL4_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            UART_PROTOCOL4_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// 对传入的命令帧进行校验，返回校验结果
BOOL UART_PROTOCOL4_CheckSUM(UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame)
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

// UART命令帧缓冲区处理
void UART_PROTOCOL4_CmdFrameProcess(UART_PROTOCOL4_CB *pCB)
{
    UART_PROTOCOL4_CMD cmd = UART_PROTOCOL4_CMD_NULL;
    UART_PROTOCOL4_RX_CMD_FRAME *pCmdFrame = NULL;
    
    uint8_t i = 0;
    BOOL checkMark;
    
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
    if (UART_PROTOCOL4_CMD_HEAD != pCmdFrame->buff[UART_PROTOCOL4_CMD_HEAD_INDEX])
    {
        // 删除命令帧
        pCB->rx.head++;
        pCB->rx.head %= UART_PROTOCOL4_RX_QUEUE_SIZE;
        return;
    }

    // 命令头合法，则提取命令
    cmd = (UART_PROTOCOL4_CMD)pCmdFrame->buff[UART_PROTOCOL4_CMD_CMD_INDEX];

    // 执行命令帧
    switch (cmd)
    {
        // 空命令，不予执行
        case UART_PROTOCOL4_CMD_NULL: // 0x53
            break;

//       // 协议切换
//       case UART_PROTOCOL4_CMD_PROTOCOL_SWITCCH:
//            TIMER_KillTask(TIMER_ID_PROTOCOL_SWITCCH);
//            
//            // 协议切换成功
//            if(pCmdFrame->buff[UART_PROTOCOL4_CMD_DATA1_INDEX])
//            {
//                STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
//                
//                // 发送配置
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
//            // dut_config升级成功
//            STATE_SwitchStep(STEP_UART_READ_CONFIG);
//            break;

//        case UART_PROTOCOL4_CMD_NEW_WRITE_CONTROL_PARAM: // 0x50
//            // dut_config升级成功
//            STATE_SwitchStep(STEP_UART_READ_CONFIG);
//            break;

//        // 校验配置参数
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

    // 删除命令帧
    pCB->rx.head++;
    pCB->rx.head %= UART_PROTOCOL4_RX_QUEUE_SIZE;
}
