#include "common.h"
#include "timer.h"
#include "CanProtocolUpDT.h"
#include "CanProtocol_3A.h"
#include "state.h"
#include "spiflash.h"
#include "dutCtl.h"
#include "DutInfo.h"

/******************************************************************************
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void CAN_PROTOCOL1_DataStructInit(CAN_PROTOCOL1_CB *pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void CAN_PROTOCOL1_MacProcess(uint32 standarID, uint8 *pData, uint16 length);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_PROTOCOL1_RxFIFOProcess(CAN_PROTOCOL1_CB *pCB);

// UART命令帧缓冲区处理
void CAN_PROTOCOL1_CmdFrameProcess(CAN_PROTOCOL1_CB *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_PROTOCOL1_CheckSUM(CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_PROTOCOL1_ConfirmTempCmdFrameBuff(CAN_PROTOCOL1_CB *pCB);

// 通讯超时处理-单向
void CAN_PROTOCOL1_CALLBACK_RxTimeOut(uint32 param);

// 停止RX通讯超时检测任务
void CAN_PROTOCOL1_StopRxTimeOutCheck(void);

// 协议层发送处理过程
void CAN_PROTOCOL1_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void CAN_PROTOCOL1_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length));

// 启动通讯超时判断任务
void CAN_PROTOCOL1_StartTimeoutCheckTask(void);

// UART总线超时错误处理
void CAN_PROTOCOL1_CALLBACK_UartBusError(uint32 param);

// 上报写配置参数结果
void CAN_PROTOCOL1_ReportWriteParamResult(uint32 param);

// 上报配置参数
void CAN_PROTOCOL1_ReportConfigureParam(uint32 param);

// 发送命令带结果
void CAN_PROTOCOL1_SendCmdWithResult(uint8 cmdWord, uint8 result);

// 发送命令无结果
void CAN_PROTOCOL1_SendCmdNoResult(uint8 cmdWord);

// 发送命令带结果
void CAN_PROTOCOL1_SendLdoV(uint8 cmdWord, uint32 result);

// 发送按键值
void CAN_PROTOCOL1_SendCmdCheckKeyValue(uint32 param);

// 全局变量定义
CAN_PROTOCOL1_CB canProtocol1CB;

// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// 启动通讯超时判断任务
void CAN_PROTOCOL1_StartTimeoutCheckTask(void)
{
}

void can_test(uint32 temp)
{
    CAN_PROTOCOL1_SendCmdWithResult(0x88, FALSE);
}

// 协议初始化
void CAN_PROTOCOL1_Init(void)
{
    // 协议层数据结构初始化
    CAN_PROTOCOL1_DataStructInit(&canProtocol1CB);

    // 向驱动层注册数据接收接口
    // CAN_DRIVE_RegisterDataSendService(CAN_PROTOCOL1_MacProcess);

    // 向驱动层注册数据发送接口
    CAN_PROTOCOL1_RegisterDataSendService(CAN_DRIVE_AddTxArray);
    //
    //  TIMER_AddTask(TIMER_ID_CAN_TEST,
    //              1500,
    //              can_test,
    //              TRUE,
    //              4,
    //              ACTION_MODE_ADD_TO_QUEUE);
}

// UART协议层过程处理
void CAN_PROTOCOL_Process_3A(void)
{
    // UART接收FIFO缓冲区处理
    CAN_PROTOCOL1_RxFIFOProcess(&canProtocol1CB);

    // UART接收命令缓冲区处理
    CAN_PROTOCOL1_CmdFrameProcess(&canProtocol1CB);

    // UART协议层发送处理过程
    CAN_PROTOCOL1_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void CAN_PROTOCOL1_TxAddData(uint8 data)
{
    uint16 head = canProtocol1CB.tx.head;
    uint16 end = canProtocol1CB.tx.end;
    CAN_PROTOCOL1_TX_CMD_FRAME *pCmdFrame = &canProtocol1CB.tx.cmdQueue[canProtocol1CB.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % CAN_PROTOCOL1_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void CAN_PROTOCOL1_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = canProtocol1CB.tx.head;
    uint16 end = canProtocol1CB.tx.end;
    CAN_PROTOCOL1_TX_CMD_FRAME *pCmdFrame = &canProtocol1CB.tx.cmdQueue[canProtocol1CB.tx.end];
    uint16 length = pCmdFrame->length;

    // 发送缓冲区已满，不予接收
    if ((end + 1) % CAN_PROTOCOL1_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 命令帧长度不足，清除已填充的数据，退出
    if (CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN - 4 > length) // 减去"校验和L、校验和H、结束标识0xD、结束标识OxA"4个字节
    {
        pCmdFrame->length = 0;

        return;
    }

    // 队尾命令帧已满，退出
    if ((length >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX) || (length + 2 >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX))
    {
        return;
    }

    // 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
    pCmdFrame->buff[CAN_PROTOCOL1_CMD_LENGTH_INDEX] = length - 4; // 重设数据长度，减去"命令头、设备地址、命令字、数据长度"4个字节
    for (i = 1; i < length; i++)
    {
        checkSum += pCmdFrame->buff[i];
    }
    pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);        // 低字节在前
    pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF); // 高字节在后

    // 结束标识
    pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
    pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

    canProtocol1CB.tx.end++;
    canProtocol1CB.tx.end %= CAN_PROTOCOL1_TX_QUEUE_SIZE;
    // pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

// 数据结构初始化
void CAN_PROTOCOL1_DataStructInit(CAN_PROTOCOL1_CB *pCB)
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
    for (i = 0; i < CAN_PROTOCOL1_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < CAN_PROTOCOL1_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART报文接收处理函数(注意根据具体模块修改)
void CAN_PROTOCOL1_MacProcess(uint32 standarID, uint8 *pData, uint16 length)
{
    uint16 end = canProtocol1CB.rxFIFO.end;
    uint16 head = canProtocol1CB.rxFIFO.head;
    uint16 i = 0;
    uint8 *rxdata = pData;

    // ■■环形列队，入队■■
    //  一级缓冲区已满，不予接收
    if ((end + 1) % CAN_PROTOCOL1_RX_FIFO_SIZE == head)
    {
        return;
    }

    for (i = 0; i < length; i++)
    {
        // 单个字节读取，并放入FIFO中
        canProtocol1CB.rxFIFO.buff[canProtocol1CB.rxFIFO.end] = *rxdata++;

        canProtocol1CB.rxFIFO.end++;

        // 一级缓冲区已满，不予接收
        if ((canProtocol1CB.rxFIFO.end + 1) % CAN_PROTOCOL1_RX_FIFO_SIZE == head)
        {
            break;
        }

        canProtocol1CB.rxFIFO.end %= CAN_PROTOCOL1_RX_FIFO_SIZE;
    }
}

// UART协议层向驱动层注册数据发送接口
void CAN_PROTOCOL1_RegisterDataSendService(BOOL (*service)(uint32 id, uint8 *pData, uint8 length))
{
    canProtocol1CB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_PROTOCOL1_ConfirmTempCmdFrameBuff(CAN_PROTOCOL1_CB *pCB)
{
    CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= CAN_PROTOCOL1_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

    return TRUE;
}

// 协议层发送处理过程
void CAN_PROTOCOL1_TxStateProcess(void)
{
    uint16 head = canProtocol1CB.tx.head;
    uint16 end = canProtocol1CB.tx.end;
    uint16 length = canProtocol1CB.tx.cmdQueue[head].length;
    uint8 *pCmd = canProtocol1CB.tx.cmdQueue[head].buff;
    uint16 localDeviceID = canProtocol1CB.tx.cmdQueue[head].deviceID;

    // 发送缓冲区为空，说明无数据
    if (head == end)
    {
        return;
    }

    // 发送函数没有注册直接返回
    if (NULL == canProtocol1CB.sendDataThrowService)
    {
        return;
    }

    // 协议层有数据需要发送到驱动层
    if (!(*canProtocol1CB.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // 发送环形队列更新位置
    canProtocol1CB.tx.cmdQueue[head].length = 0;
    canProtocol1CB.tx.head++;
    canProtocol1CB.tx.head %= CAN_PROTOCOL1_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_PROTOCOL1_RxFIFOProcess(CAN_PROTOCOL1_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (CAN_PROTOCOL1_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

            return;
        }

        // 命令头正确，但无临时缓冲区可用，退出
        if ((pCB->rx.end + 1) % CAN_PROTOCOL1_RX_QUEUE_SIZE == pCB->rx.head)
        {
            return;
        }

        // 添加UART通讯超时时间设置-2016.1.5增加
#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
        TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
                      CAN_PROTOCOL1_BUS_UNIDIRECTIONAL_TIME_OUT,
                      CAN_PROTOCOL1_CALLBACK_RxTimeOut,
                      0,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
#endif

        // 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
        pCmdFrame->buff[pCmdFrame->length++] = currentData;
        pCB->rxFIFO.currentProcessIndex++;
        pCB->rxFIFO.currentProcessIndex %= CAN_PROTOCOL1_RX_FIFO_SIZE;
    }
    // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
    else
    {
        // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
        if (pCmdFrame->length >= CAN_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX)
        {
#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            CAN_PROTOCOL1_StopRxTimeOutCheck();
#endif

            // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
            pCmdFrame->length = 0; // 2016.1.5增加
            // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= CAN_PROTOCOL1_RX_FIFO_SIZE;

            // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■

            // 首先判断命令帧最小长度，一个完整的命令字至少包括8个字节: 命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，因此不足8个字节的必定不完整
            if (pCmdFrame->length < CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN)
            {
                // 继续接收
                continue;
            }

            // 命令帧长度数值越界，说明当前命令帧错误，停止接收
            if (pCmdFrame->buff[CAN_PROTOCOL1_CMD_LENGTH_INDEX] > CAN_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX - CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN)
            {
#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                CAN_PROTOCOL1_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // 命令帧长度校验，在命令长度描述字的数值上，增加命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，即为命令帧实际长度
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[CAN_PROTOCOL1_CMD_LENGTH_INDEX] + CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN)
            {
                // 长度要求不一致，说明未接收完毕，退出继续
                continue;
            }

            // 命令帧长度OK，则进行校验，失败时删除命令头
            if (!CAN_PROTOCOL1_CheckSUM(pCmdFrame))
            {
#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                CAN_PROTOCOL1_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            CAN_PROTOCOL1_StopRxTimeOutCheck();
#endif

            // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= CAN_PROTOCOL1_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            CAN_PROTOCOL1_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_PROTOCOL1_CheckSUM(CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame)
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
void CAN_PROTOCOL1_CmdFrameProcess(CAN_PROTOCOL1_CB *pCB)
{
    CAN_PROTOCOL1_CMD cmd = CAN_PROTOCOL1_CMD_NULL;
    CAN_PROTOCOL1_RX_CMD_FRAME *pCmdFrame = NULL;

    BOOL checkMark;
    uint8_t i = 0;
    uint8 buffFlag[4] = {0xFF};

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
    if (CAN_PROTOCOL1_CMD_HEAD != pCmdFrame->buff[CAN_PROTOCOL1_CMD_HEAD_INDEX])
    {
        // 删除命令帧
        pCB->rx.head++;
        pCB->rx.head %= CAN_PROTOCOL1_RX_QUEUE_SIZE;
        return;
    }

    // 命令头合法，则提取命令
    cmd = (CAN_PROTOCOL1_CMD)pCmdFrame->buff[CAN_PROTOCOL1_CMD_CMD_INDEX];
    
    if(CAN_PROTOCOL1_IOT_DEVICE_ADDR == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DEVICE_ADDR_INDEX])
    {
        // 执行命令帧
        switch (cmd)
        {
            // 空命令，不予执行
            case CAN_PROTOCOL1_CMD_NULL: 
                break;

            // 升级请求命令（0xF1）
            case CAN_PROTOCOL1_CMD_UPDATE_REQUEST: 
                TIMER_KillTask(TIMER_ID_SEND_UP_APP_REQUEST);
            
                //   0：设备拒绝升级，1：设备同意升级。 
                if(pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_SEND_APP_EAR);                    
                }
                else
                {
                    // 跳到超时处理步骤代表升级失败
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_COMMUNICATION_TIME_OUT);                 
                }
                break;

            // 升级固件命令（0xF2）
            case CAN_PROTOCOL1_CMD_UPDATE_START: 
            
                //   该命令用于应答升级命令结果，若超过存储空间，则返回0。
                if(0 == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    // 跳到超时处理步骤代表升级失败                
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_COMMUNICATION_TIME_OUT);                    
                }
                else
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_SEND_FIRST_APP_PACKET);                 
                }
                break;

            // 数据包写入命令（0xF3）
            case CAN_PROTOCOL1_CMD_WRITE_DATA: 

                if (1 == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    if (0 < dut_info.currentAppSize)
                    {
                        dut_info.currentAppSize--;
                    }
                    break;
                }
                if (dut_info.currentAppSize < dut_info.appSize)
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_SEND_APP_PACKET);
                }
                else
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_APP_UP_SUCCESS);
                }
                break;

            // 数据包写入完成命令（0xF4）0：包传输完成，CRC未通过校验；1：包传输完成，CRC通过校验
            case CAN_PROTOCOL1_CMD_WRITE_COMPLETE: 
            
                if(0 == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    // 跳到超时处理步骤代表升级失败                
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_COMMUNICATION_TIME_OUT);                    
                }
                else
                {
                    STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_ITEM_FINISH);                 
                }
                break;
                
            default:
                break;
        }         
    }
    else
    {
        // 执行命令帧
        switch (cmd)
        {
            // 空命令，不予执行
            case CAN_PROTOCOL1_CMD_NULL:
                break;

            // 写入版本信息
            case CAN_PROTOCOL1_CMD_VERSION_TYPE_WRITE:
                TIMER_KillTask(TIMER_ID_SET_DUT_UI_VER);
                writeUiVerFlag = FALSE;
                if ((8 ==  pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX]) && (TRUE == pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA2_INDEX]))
                {
                    TIMER_KillTask(TIMER_ID_SET_DUT_UI_VER);
                    STATE_SwitchStep(STEP_CM_CAN_READ_UI_VER);
                }
                else
                {
                    dut_info.uiUpFaile = TRUE; // ui升级失败
                    STATE_EnterState(STATE_STANDBY);
                }
                break;

            // 读取版本信息
            case CAN_PROTOCOL1_CMD_VERSION_TYPE_READ:
                if (8 ==  pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    // 校验
                    for (i = 0; i < pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA2_INDEX]; i++)
                    {
                        if (uiVerifiedBuff[i] != pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA3_INDEX + i])
                        {
                            checkMark = FALSE;  // 发现不匹配，设置标志为假
                            break;              // 跳出循环
                        }
                        else
                        {
                            checkMark = TRUE;   // 所有元素都匹配，设置标志为真
                        }
                    }

                    if (checkMark)
                    {
                        STATE_SwitchStep(STEP_CM_CAN_WRITE_UI_VER_SUCCESS);
                    }
                    else
                    {
                        dut_info.uiUpFaile = TRUE; // ui升级失败
                        STATE_EnterState(STATE_STANDBY);
                    }
                }
                else
                {
                    dut_info.uiUpFaile = TRUE; // ui升级失败
                    STATE_EnterState(STATE_STANDBY);
                }
                break;

            // 0xC1,写配置信息应答
            case CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM_RESULT:
            
                // 有应答代表配置信息升级成功
                STATE_SwitchStep(STEP_CAN_READ_CONFIG);
                break;

            // 0xc3,读取配置参数，配置参数校验
            case CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM_REPORT:

                // 校验
                for (i = 0; i < pCmdFrame->buff[CAN_PROTOCOL1_CMD_LENGTH_INDEX]; i++)
                {
                    if (verifiedBuff[i] != pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX + i])
                    {
                        checkMark = FALSE;  // 发现不匹配，设置标志为假
                        break;              // 跳出循环
                    }
                    else
                    {
                        checkMark = TRUE;   // 所有元素都匹配，设置标志为真
                    }
                }

                if (checkMark)
                {
    //                STATE_SwitchStep(STEP_CAN_SET_CONFIG_SUCCESS);
                    STATE_SwitchStep(STEP_CAN_DCD_FLAG_WRITE);
                }
                else
                {
                    dut_info.configUpFaile = TRUE; // config升级失败
                    STATE_EnterState(STATE_STANDBY);
                }
                break;

            // 写入DCD工位标志位
            case CAN_PROTOCOL1_CMD_DCD_FLAG_WRITE:   
            
                // 校验标志区数据
                if (pCmdFrame->buff[CAN_PROTOCOL1_CMD_DATA1_INDEX])
                {
                    STATE_SwitchStep(STEP_CAN_DCD_FLAG_READ);
                }
                else
                {
                    dut_info.configUpFaile = TRUE; // config升级失败
                    STATE_EnterState(STATE_STANDBY);
                }         
              break;

            // 读取DCD工位标志位
            case CAN_PROTOCOL1_CMD_DCD_FLAG_READ:
            
                 // 起始符 + 设备地址 + 命令字 + 数据长度 + 标志区数据长度 + （数据 +）×N + 校验码 + 结束符
                 // 读取DCD标志区数据（站位编号 * （每个标志位四个数据） + （第一个标志位前的数据(即数据前有五个数据)） + 第几个标志位）
                for (i = 0; i < 4; i++)
                {
                    buffFlag[i] = pCmdFrame->buff[(5 * 4) + 5 + i];
                }

                // 校验标志区数据
                if ((buffFlag[0] == 0x00) && (buffFlag[1] == 0x00) && (buffFlag[2] == 0x00) && (buffFlag[3] == 0x02))
                {
                    STATE_SwitchStep(STEP_CAN_SET_CONFIG_SUCCESS);
                }
                else
                {
                    dut_info.configUpFaile = TRUE; // config升级失败
                    STATE_EnterState(STATE_STANDBY);
                }              
              break;

            default:
                break;
        }    
    }
    

    // 删除命令帧
    pCB->rx.head++;
    pCB->rx.head %= CAN_PROTOCOL1_RX_QUEUE_SIZE;
}

//  UART总线超时错误处理
void CAN_PROTOCOL1_CALLBACK_UartBusError(uint32 param)
{
}

// 上报写配置参数结果
void CAN_PROTOCOL1_ReportWriteParamResult(uint32 param)
{
    // 添加命令头
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);

    // 添加设备地址
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);

    // 添加命令字
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM_RESULT);

    // 添加数据长度
    CAN_PROTOCOL1_TxAddData(1);

    // 写入结果
    CAN_PROTOCOL1_TxAddData(param);

    // 添加检验和与结束符，并添加至发送
    CAN_PROTOCOL1_TxAddFrame();
}

// 发送命令带结果
void CAN_PROTOCOL1_SendLdoV(uint8 cmdWord, uint32 result)
{
    // 添加命令头
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);

    if(DUT_TYPE_IOT == dut_info.ID)
    {
        // 添加设备地址
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);    
    }
    else
    {
        // 添加设备地址
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);    
    }

    // 添加命令字
    CAN_PROTOCOL1_TxAddData(cmdWord);

    // 添加数据长度
    CAN_PROTOCOL1_TxAddData(4);

    CAN_PROTOCOL1_TxAddData((result >> 24) & 0xFF);
    CAN_PROTOCOL1_TxAddData((result >> 16) & 0xFF);
    CAN_PROTOCOL1_TxAddData((result >> 8) & 0xFF);
    CAN_PROTOCOL1_TxAddData((result >> 0) & 0xFF);

    CAN_PROTOCOL1_TxAddFrame();
}

// 发送命令带结果
void CAN_PROTOCOL1_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
    // 添加命令头
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);

    if(DUT_TYPE_IOT == dut_info.ID)
    {
        // 添加设备地址
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);    
    }
    else
    {
        // 添加设备地址
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);    
    }

    // 添加命令字
    CAN_PROTOCOL1_TxAddData(cmdWord);

    // 添加数据长度
    CAN_PROTOCOL1_TxAddData(1);

    CAN_PROTOCOL1_TxAddData(result);

    CAN_PROTOCOL1_TxAddFrame();
}

// 发送命令无结果
void CAN_PROTOCOL1_SendCmdNoResult(uint8 cmdWord)
{
    // 添加命令头
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);

    if(DUT_TYPE_IOT == dut_info.ID)
    {
        // 添加设备地址
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);    
    }
    else
    {
        // 添加设备地址
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);    
    }
    
    // 添加命令字
    CAN_PROTOCOL1_TxAddData(cmdWord);

    CAN_PROTOCOL1_TxAddData(0); // 数据长度

    CAN_PROTOCOL1_TxAddFrame();
}

// 发送升级固件请求
void CAN_PROTOCOL1_SendUpAppReuqest(uint32 param)
{
    CAN_PROTOCOL1_SendCmdWithResult(CAN_PROTOCOL1_CMD_UPDATE_REQUEST, 9); 
}

// 发送升级固件命令
void CAN_PROTOCOL1_SendCmdUpApp(void)
{
	// 添加命令头
	CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
    
    // 添加设备地址
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);    

	// 添加命令字
	CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_UPDATE_START); // F2
	CAN_PROTOCOL1_TxAddData(0); // 数据长度 
	CAN_PROTOCOL1_TxAddData(9); // 设备类型 IOT-9
	CAN_PROTOCOL1_TxAddData(dut_info.iotCrc8); // CRC8校验码
	CAN_PROTOCOL1_TxAddData(((dut_info.iotAppUpDataLen) & 0xFF000000) >> 24); // 文件长度
	CAN_PROTOCOL1_TxAddData(((dut_info.iotAppUpDataLen) & 0x00FF0000) >> 16); // 文件长度
	CAN_PROTOCOL1_TxAddData(((dut_info.iotAppUpDataLen) & 0x0000FF00) >> 8); // 文件长度
	CAN_PROTOCOL1_TxAddData((dut_info.iotAppUpDataLen) & 0x000000FF); // 文件长度    
	CAN_PROTOCOL1_TxAddFrame();
}

// 写入IOT APP数据
void CAN_PROTOCOL1_SendOnePacket_Bin(uint32 flashAddr, uint32 addr)
{
	uint8 addr1, addr2, addr3, addr4;
	uint8 appUpdateOnePacket[150] = {0};
	uint8 appUpdateOnePacket_i = 0;
	addr1 = (addr & 0xFF000000) >> 24;
	addr2 = (addr & 0x00FF0000) >> 16;
	addr3 = (addr & 0x0000FF00) >> 8;
	addr4 = (addr & 0x000000FF);
	SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + addr, 128); // 工具读取128字节

	CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                                     // 头
	CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_IOT_DEVICE_ADDR);                              // 设备号
    CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_DATA);                               // 命令字
	CAN_PROTOCOL1_TxAddData(0x00);													       // 长度暂时为0
	CAN_PROTOCOL1_TxAddData(addr1);													   // 添加地址
	CAN_PROTOCOL1_TxAddData(addr2);													   // 添加地址
	CAN_PROTOCOL1_TxAddData(addr3);													   // 添加地址
	CAN_PROTOCOL1_TxAddData(addr4);													   // 添加地址
	CAN_PROTOCOL1_TxAddData(128);													       // 数据包长度
    
	for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 128; appUpdateOnePacket_i++) // 添加数据
	{
		CAN_PROTOCOL1_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i]);
	}
	CAN_PROTOCOL1_TxAddFrame(); // 调整帧格式,修改长度和添加校验
}

