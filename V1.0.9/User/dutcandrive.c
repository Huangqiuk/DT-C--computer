/*********************************************************
 * 头文件区
 *********************************************************/
// can通用头文件
#include "CanProtocolUpDT.h"
#include "DutCanProtocol.h"
#include "param.h"
#include "DutInfo.h"
// 设备头文件（根据需求添加与删除）
#include "system.h"
#include "timer.h"
#include "spiflash.h"
#include "iap.h"
#include "gd32f10x_can.h"
/*********************************************************
 * 变量定义区
 *********************************************************/
CAN_CB sysCanCB;              // CAN通信数据结构
CAN_TX_STATE_CB canTxStateCB; // 发送状态机控制对象

/*********************************************************
 * 内部函数申明区
 *********************************************************/
/************ CAN底层函数 *************************/
// 数据结构初始化
void CAN_DataStructureInit(CAN_CB *pCB);

// CAN 报文接收处理函数(注意根据具体模块修改)
void CAN_MacProcess(uint16 standarID, uint8 *data, uint8 length);

// 一级报文接收缓冲区处理函数，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_RxFIFOProcess(CAN_CB *pCB);

// CAN命令帧缓冲区处理
void CAN_CmdFrameProcess(CAN_CB *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_CheckSum(uint8 *buff, uint32 len);

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_AddSum(uint8 *buff, uint32 len);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_ConfirmTempCmdFrameBuff(CAN_CB *pCB, uint8 sourceSelect);

/************ CAN状态机函数 **************************/
// 填充与发送处理
BOOL CAN_Tx_FillAndSend(CAN_CB *pCB);

// 发送成功启动下一帧
BOOL CAN_Tx_NextFram(CAN_CB *pCB);

// CAN发送状态机初始化
void CAN_TxStateInit(void);

// CAN发送状态迁移
void CAN_TxEnterState(uint32 state);

// CAN发送状态机处理
void CAN_TxStateProcess(void);

/************ CAN底层函数 **************************/

// 工程模式准备就绪报告
void IAP_CTRL_CAN_SendCmdProjectReady(uint8 param);

// 擦除FLASH结果上报
void IAP_CTRL_CAN_SendCmdEraseFlashResult(uint8 param);

// IAP数据写入结果上报
void IAP_CTRL_CAN_SendCmdWriteFlashResult(uint8 param);

// 查空结果上报
void IAP_CTRL_CAN_SendCmdCheckFlashBlankResult(uint8 param);

// 系统升级结束确认
void IAP_CTRL_CAN_SendCmdUpdataFinishResult(uint8 param);

// 发送UI数据擦除结果
void IAP_CTRL_CAN_SendCmdUIEraseResultReport(uint8 param);

// 发送UI数据写入结果
void IAP_CTRL_CAN_SendCmdUIWriteDataResultReport(uint8 param);

void CAN_TEST(uint32 temp)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK);
    IAP_CTRL_CAN_TxAddData(2); // 数据长度
    IAP_CTRL_CAN_TxAddData(55);
    IAP_CTRL_CAN_TxAddData(55);
    IAP_CTRL_CAN_TxAddFrame();
}
/*********************************************************
 * 函数定义区
 *********************************************************/

// CAN 初始化
void IAP_CTRL_CAN_Init(uint16 bund)
{
    // 初始化硬件配置
    CAN_HW_Init(bund);

    // 初始化结构体
    CAN_DataStructureInit(&sysCanCB);

    // CAN发送状态机初始化
    CAN_TxStateInit();
}

// 数据结构初始化
void CAN_DataStructureInit(CAN_CB *pCB)
{
    uint8 i;
    uint8 j;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return;
    }

    pCB->tx.txBusy = FALSE;
    pCB->tx.index = 0;
    pCB->tx.head = 0;
    pCB->tx.end = 0;
    for (i = 0; i < CAN_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    for (i = 0; i < CAN_DEVICE_SOURCE_ID_MAX; i++)
    {
        pCB->rxFIFO.rxFIFOEachNode[i].head = 0;
        pCB->rxFIFO.rxFIFOEachNode[i].end = 0;
        pCB->rxFIFO.rxFIFOEachNode[i].currentProcessIndex = 0;
    }

    for (i = 0; i < CAN_DEVICE_SOURCE_ID_MAX; i++)
    {
        for (j = 0; j < CAN_RX_QUEUE_SIZE; j++)
        {
            pCB->rx.rxEachNode[i].cmdQueue[j].length = 0;
        }
        pCB->rx.rxEachNode[i].head = 0;
        pCB->rx.rxEachNode[i].end = 0;
    }
}

// 向发送命令帧队列中添加数据
void IAP_CTRL_CAN_TxAddData(uint8 data)
{
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;
    CAN_CMD_FRAME *pCmdFrame = &sysCanCB.tx.cmdQueue[sysCanCB.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % CAN_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= CAN_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// 发送命令帧处理,自动扶正帧长度以及检验码
void IAP_CTRL_CAN_TxAddFrame(void)
{
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;
    uint8 *buff = sysCanCB.tx.cmdQueue[end].buff;
    uint16 length = sysCanCB.tx.cmdQueue[end].length;
    uint8 cmdHead = buff[CAN_CMD_HEAD_INDEX]; // 帧头

    // 发送缓冲区已满，不予接收
    if ((end + 1) % CAN_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 命令头错误
    if (CAN_CMD_HEAD != cmdHead)
    {
        // 把帧数据索引放在最开始位置
        sysCanCB.tx.cmdQueue[end].length = 0;
        return;
    }

    // 命令头正确，但数据长度不对(数据长度减去CRC的长度)
    if ((CAN_CMD_FRAME_LENGTH_MAX - CAN_CHECK_BYTE_SIZE < length) || (CAN_CMD_FRAME_LENGTH_MIN - CAN_CHECK_BYTE_SIZE > length))
    {
        // 把帧数据索引放在最开始位置
        sysCanCB.tx.cmdQueue[end].length = 0;
        buff[CAN_CMD_HEAD_INDEX] = CAN_CMD_NONHEAD; // 避免数据没更新时，再次进入
        return;
    }

    // 执行到此，命令头正确，数据长度正确,发送缓冲区未满
    // 长度扶正
    buff[CAN_CMD_LENGTH_INDEX] = length - CAN_CMD_FRAME_LENGTH_MIN + CAN_CHECK_BYTE_SIZE;

    // 填充CRC校验码
    CAN_AddSum(buff, length);

    // 将数据长度填到待发送缓冲器
    sysCanCB.tx.cmdQueue[end].length = length + CAN_CHECK_BYTE_SIZE;

    // 发送环形队列更新位置
    sysCanCB.tx.end++;
    sysCanCB.tx.end %= CAN_TX_QUEUE_SIZE;
    sysCanCB.tx.cmdQueue[sysCanCB.tx.end].length = 0; // 2015.12.2修改
}

// 填充与发送处理:TRUE,填充与发送；FALSE，环形队列向前滑移.
BOOL CAN_Tx_FillAndSend(CAN_CB *pCB)
{
    uint16 index = pCB->tx.index;                          // 当前发送数据的索引号
    uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length; // 当前发送的命令帧的长度
    uint16 head = pCB->tx.head;                            // 发送命令帧队列头索引号
    uint16 end = pCB->tx.end;                              // 发送命令帧队列尾索引号
    CAN_CMD_FRAME *pCmd = &pCB->tx.cmdQueue[pCB->tx.head]; // 取当前发送缓冲区地址
    can_trasnmit_message_struct canTxMsgCB;
    uint16 standardID = 0x000;
    uint8 i; // 内部赋值用
    uint32 extendedID;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return FALSE;
    }

    // 队列为空，不处理
    if (head == end)
    {
        return FALSE;
    }

    // ■■执行到这里，说明队列非空■■

    // 当前命令帧未发送完时，根据长度的变化取出命令帧中的数据放到发送寄存器中
    if (index < length)
    {
        // 配置为标准帧
        // 配置为数据帧
        canTxMsgCB.tx_ft = CAN_FT_DATA;
        switch (dut_info.ID)
        {
						case DUT_TYPE_GB:
						case DUT_TYPE_KAIYANG:
							// 扩展帧
							canTxMsgCB.tx_ff = CAN_FF_EXTENDED;
							extendedID = pCmd->deviceID;
							canTxMsgCB.tx_efid = extendedID;
							break;
						default:
							// 标准帧
							canTxMsgCB.tx_ff = CAN_FF_STANDARD;
							standardID = (uint16)CAN_DEVICE_ID_HMI; // id：001
							canTxMsgCB.tx_sfid = standardID;
							break;
        }
        // ■■标准数据帧CAN消息格式■■

        // 填充 DLC，最大8字节.当前长度>8字节，先发8字节。
        if ((length - index) >= CAN_ONCEMESSAGE_MAX_SIZE)
        {
            canTxMsgCB.tx_dlen = CAN_ONCEMESSAGE_MAX_SIZE;
        }
        else
        {
            canTxMsgCB.tx_dlen = length - index;
        }

        // 填充数据
        for (i = 0; i < canTxMsgCB.tx_dlen; i++)
        {
            canTxMsgCB.tx_data[i] = pCmd->buff[index + i];
        }

        // 发送数据
        if (can_message_transmit(CAN0, &canTxMsgCB) == CAN_TRANSMIT_NOMAILBOX)
        {
            return FALSE;
        }

        // 开启邮箱空中断
        can_interrupt_enable(CAN0, CAN_INT_TME);

        return TRUE;
    }
    // 当前命令帧发送完时，删除之
    else
    {
        pCB->tx.cmdQueue[head].length = 0;
        pCB->tx.head++;
        pCB->tx.head %= CAN_TX_QUEUE_SIZE;
        pCB->tx.index = 0;

        // 进入CAN消息发送待机状态
        CAN_TxEnterState(CAN_TX_STATE_STANDBY);

        return FALSE;
    }
}

// 发送成功启动下一帧:TRUE,启动下一帧；FALSE，发送完成.
BOOL CAN_Tx_NextFram(CAN_CB *pCB)
{
    uint16 index = pCB->tx.index;                          // 当前发送数据的索引号
    uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length; // 当前发送的命令帧的长度
    uint8 txBnDLC = 0;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return FALSE;
    }

    // 判断上一次成功发送的字节数.
    if (length - index >= CAN_ONCEMESSAGE_MAX_SIZE)
    {
        txBnDLC = CAN_ONCEMESSAGE_MAX_SIZE;
    }
    else
    {
        txBnDLC = length - index;
    }
    pCB->tx.index += txBnDLC;

    // 启动下一帧
    if (pCB->tx.index < length)
    {
        // 进入CAN消息下一帧发送状态
        return TRUE;
    }
    else
    {
        // 发送完成
        return FALSE;
    }
}

// CAN 报文接收处理函数(注意根据具体模块修改)
void CAN_MacProcess(uint16 standarID, uint8 *data, uint8 length)
{
    uint16 end;
    uint16 head;
    uint8 i;
    int8 sourceBuffSelect = CAN_DEVICE_SOURCE_ID_MAX; // 节点缓存区编号选择

    // 参数合法性判断
    if (NULL == data)
    {
        return;
    }

    // 标准帧判断
    if (standarID > 0x7FF)
    {
        return;
    }

    // 取出目标节点
    //  destinationID = standarID & 0x07FF;

    // 不是发给自身ID的数据直接丢掉
    //  if (CAN_DEVICE_ID_HMI != destinationID)
    //  {
    //      return;
    //  }

    // 已经使能
    if (CAN_RX_DEVICE_PUC)
    {
        sourceBuffSelect = CAN_RX_DEVICE_PUC - 1;
    }

    // 没有合适的源节点
    if ((sourceBuffSelect >= CAN_DEVICE_SOURCE_ID_MAX) || (sourceBuffSelect < 0))
    {
        return;
    }

    // 取相应缓存区结构体数据
    end = sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end;
    head = sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].head;

    // ■■环形列队，入队■■
    //  一级缓冲区已满，不予接收
    if ((end + 1) % CAN_RX_FIFO_SIZE == head)
    {
        return;
    }

    for (i = 0; i < length; i++)
    {
        // 单个字节读取，并放入FIFO中
        sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].buff[sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end] = *data++;

        sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end++;

        // 一级缓冲区已满，不予接收
        if ((sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end + 1) % CAN_RX_FIFO_SIZE == head)
        {
            break;
        }

        sysCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end %= CAN_RX_FIFO_SIZE;
    }
}

// 一级报文接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_RxFIFOProcess(CAN_CB *pCB)
{
    uint16 end;
    uint16 head;
    CAN_CMD_FRAME *pCmdFrame = NULL;
    CAN_RX_FIFO *pRxFIFO = NULL;
    uint16 length = 0;
    uint8 currentData = 0;
    uint8 sourceBuffSelect;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return;
    }

    // 循环判断是否有数据需要处理
    for (sourceBuffSelect = 0; sourceBuffSelect < CAN_DEVICE_SOURCE_ID_MAX; sourceBuffSelect++)
    {
        // 获取接收一级缓冲区节点临时缓冲区指针
        pRxFIFO = &pCB->rxFIFO.rxFIFOEachNode[sourceBuffSelect];

        end = pRxFIFO->end;
        head = pRxFIFO->head;

        // 一级缓冲区为空，继续
        if (head == end)
        {
            continue;
        }

        // 获取接收命令临时缓冲区指针
        pCmdFrame = &pCB->rx.rxEachNode[sourceBuffSelect].cmdQueue[pCB->rx.rxEachNode[sourceBuffSelect].end];

        // 取出当前要处理的字节
        currentData = pRxFIFO->buff[pRxFIFO->currentProcessIndex];

        // 临时缓冲区长度为0时，搜索首字节
        if (0 == pCmdFrame->length)
        {
            // 命令头错误，删除当前字节并退出
            if (CAN_CMD_HEAD != currentData)
            {
                pRxFIFO->head++;
                pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                continue;
            }

            // 命令头正确，但无命令临时缓冲区可用，退出
            if ((pCB->rx.rxEachNode[sourceBuffSelect].end + 1) % CAN_RX_QUEUE_SIZE == pCB->rx.rxEachNode[sourceBuffSelect].head)
            {
                continue;
            }

            // 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
            pCmdFrame->buff[pCmdFrame->length++] = currentData;
            pRxFIFO->currentProcessIndex++;
            pRxFIFO->currentProcessIndex %= CAN_RX_FIFO_SIZE;
        }
        // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
        else
        {
            // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
            if (pCmdFrame->length >= CAN_CMD_FRAME_LENGTH_MAX)
            {
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pRxFIFO->head++;
                pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                continue;
            }

            // 一直取到末尾
            while (end != pRxFIFO->currentProcessIndex)
            {
                // 取出当前要处理的字节
                currentData = pRxFIFO->buff[pRxFIFO->currentProcessIndex];
                // 缓冲区未溢出，正常接收，将数据添加到临时缓冲区中
                pCmdFrame->buff[pCmdFrame->length++] = currentData;
                pRxFIFO->currentProcessIndex++;
                pRxFIFO->currentProcessIndex %= CAN_RX_FIFO_SIZE;

                // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■

                // 首先判断命令帧最小长度，一个完整的命令字至少包括4个字节: 命令头(1Byte)+命令字(1Byte)+数据长度(1Byte)+校验码(1Byte) ，因此不足4个字节的必定不完整
                if (pCmdFrame->length < CAN_CMD_FRAME_LENGTH_MIN)
                {
                    // 继续接收
                    continue;
                }

                // 命令帧长度数值越界，说明当前命令帧错误，停止接收
                if (pCmdFrame->buff[CAN_CMD_LENGTH_INDEX] > (CAN_CMD_FRAME_LENGTH_MAX - CAN_CMD_FRAME_LENGTH_MIN))
                {
                    // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                    pCmdFrame->length = 0;
                    // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                    pRxFIFO->head++;
                    pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                    pRxFIFO->currentProcessIndex = pRxFIFO->head;

                    break;
                }

                // 命令帧长度校验，在命令长度描述字的数值上，增加命令头、命令字、...、数据长度、校验码，即为命令帧实际长度
                length = pCmdFrame->length;
                if (length < (pCmdFrame->buff[CAN_CMD_LENGTH_INDEX] + CAN_CMD_FRAME_LENGTH_MIN))
                {
                    // 长度要求不一致，说明未接收完毕，继续
                    continue;
                }

                // 命令帧长度OK，则进行校验，失败时删除命令头
                if (!CAN_CheckSum(pCmdFrame->buff, pCmdFrame->length))
                {
                    // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                    pCmdFrame->length = 0;
                    // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                    pRxFIFO->head++;
                    pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                    pRxFIFO->currentProcessIndex = pRxFIFO->head;

                    break;
                }

                // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
                pRxFIFO->head += length;
                pRxFIFO->head %= CAN_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                CAN_ConfirmTempCmdFrameBuff(pCB, sourceBuffSelect);

                break;
            }
        }
    }

    return;
}


// CAN命令帧缓冲区处理
void CAN_CmdFrameProcess(CAN_CB *pCB)
{
    CAN_CMD cmd = IAP_CTRL_CAN_CMD_EMPTY;
    CAN_CMD_FRAME *pCmdFrame = NULL;
    uint8 sourceBuffSelect = CAN_DEVICE_SOURCE_ID_MAX;
    volatile uint32 temp = 0;
    volatile uint16 length;
    volatile uint8 errStatus;
    volatile uint16 i;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return;
    }

    // 循环判断是否有数据需要处理
    for (sourceBuffSelect = 0; sourceBuffSelect < CAN_DEVICE_SOURCE_ID_MAX; sourceBuffSelect++)
    {
        // 命令帧缓冲区为空，退出
        if (pCB->rx.rxEachNode[sourceBuffSelect].head == pCB->rx.rxEachNode[sourceBuffSelect].end)
        {
            continue;
        }

        // 获取当前要处理的命令帧指针
        pCmdFrame = &pCB->rx.rxEachNode[sourceBuffSelect].cmdQueue[pCB->rx.rxEachNode[sourceBuffSelect].head];

        // 命令头非法，退出
        if (CAN_CMD_HEAD != pCmdFrame->buff[CAN_CMD_HEAD_INDEX])
        {
            // 删除命令帧
            pCB->rx.rxEachNode[sourceBuffSelect].head++;
            pCB->rx.rxEachNode[sourceBuffSelect].head %= CAN_RX_QUEUE_SIZE;
            continue;
        }

        // 命令头合法，则提取命令
        cmd = (CAN_CMD)(pCmdFrame->buff[CAN_CMD_CMD_INDEX]);

        switch (cmd)
        {
        case DUT_TYPE_NULL:
            break;



        default:
            break;
        }
        // 删除命令帧
        pCB->rx.rxEachNode[sourceBuffSelect].head++;
        pCB->rx.rxEachNode[sourceBuffSelect].head %= CAN_RX_QUEUE_SIZE;
    }
}

// CAN处理测试过程函数
void CAN_PROTOCOL_Process_DT(void)
{
    // CAN接收FIFO缓冲区处理
    //CAN_RxFIFOProcess(&sysCanCB);

    // CAN接收命令缓冲区处理
    //CAN_CmdFrameProcess(&sysCanCB);

    // CAN发送状态机处理过程
    CAN_TxStateProcess();
}

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_CheckSum(uint8 *buff, uint32 len)
{
    uint8 cc = 0;
    uint16 i = 0;

    if (NULL == buff)
    {
        return FALSE;
    }

    // 从命令头开始，到校验码之前的一个字节，依次进行异或运算
    for (i = 0; i < (len - CAN_CHECK_BYTE_SIZE); i++)
    {
        cc ^= buff[i];
    }

    cc = ~cc;

    // 判断计算得到的校验码与命令帧中的校验码是否相同
    if (buff[len - 1] != cc)
    {
        return FALSE;
    }

    return TRUE;
}

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_AddSum(uint8 *buff, uint32 len)
{
    uint8 cc = 0;
    uint16 i = 0;

    if (NULL == buff)
    {
        return FALSE;
    }

    // 从命令头开始，到校验码之前的一个字节，依次进行异或运算
    for (i = 0; i < len; i++)
    {
        cc ^= buff[i];
    }

    cc = ~cc;

    // 添加校验码
    buff[len] = cc;

    return TRUE;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_ConfirmTempCmdFrameBuff(CAN_CB *pCB, uint8 sourceSelect)
{
    CAN_CMD_FRAME *pCmdFrame = NULL;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return FALSE;
    }

    // 临时缓冲区为空，不予添加
    pCmdFrame = &pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end];

    if (0 == pCmdFrame->length)
    {
        return FALSE;
    }

    // 添加
    pCB->rx.rxEachNode[sourceSelect].end++;
    pCB->rx.rxEachNode[sourceSelect].end %= CAN_RX_QUEUE_SIZE;
    // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区
    pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end].length = 0;

    return TRUE;
}

// CANTX状态机初始化
void CAN_TxStateInit(void)
{
    CAN_TxEnterState(CAN_TX_STATE_STANDBY);
}

// CANTX状态迁移函数
void CAN_TxEnterState(uint32 state)
{
    // 让当前的状态成为历史
    canTxStateCB.preState = canTxStateCB.state;

    // 设置新的状态
    canTxStateCB.state = (CAN_TX_STATE_E)state;

    switch (state)
    {
    // ■■状态的入口处理■■
    case CAN_TX_STATE_NULL:

        break;

    // ■■入口状态的入口处理■■
    case CAN_TX_STATE_ENTRY:

        break;

    // ■■待机状态状态的入口处理■■
    case CAN_TX_STATE_STANDBY:

        break;

    // ■■CAN消息发送中■■
    case CAN_TX_STATE_SENDING:

        break;

    default:
        break;
    }
}

// 向发送缓冲区中添加一条待发送序列
BOOL CAN_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length)
{
    uint16 i;
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;

    // 参数检验
    if ((NULL == pArray) || (0 == length))
    {
        return FALSE;
    }

    // 发送缓冲区已满，不予接收
    if ((end + 1) % CAN_TX_QUEUE_SIZE == head)
    {
        return FALSE;
    }

    sysCanCB.tx.cmdQueue[end].deviceID = id;

    for (i = 0; i < length; i++)
    {
        sysCanCB.tx.cmdQueue[end].buff[i] = *pArray++;
    }
    sysCanCB.tx.cmdQueue[end].length = length;

    // 发送环形队列更新位置
    sysCanCB.tx.end++;
    sysCanCB.tx.end %= CAN_TX_QUEUE_SIZE;
    sysCanCB.tx.cmdQueue[sysCanCB.tx.end].length = 0;

    return TRUE;
}

// CANTX过程处理函数
void CAN_TxStateProcess(void)
{
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;

    switch (canTxStateCB.state)
    {
    // ■■状态的入口处理■■
    case CAN_TX_STATE_NULL:

        break;

    // ■■入口状态的入口处理■■
    case CAN_TX_STATE_ENTRY:

        break;

    // ■■待机状态状态的入口处理■■
    case CAN_TX_STATE_STANDBY:
        // 检测是否有Tx请求
        if (head != end)
        {
            // 调用此函数填充与启动发送can消息
            if (CAN_Tx_FillAndSend(&sysCanCB))
            {
                // 进入CAN消息发送中状态
                CAN_TxEnterState(CAN_TX_STATE_SENDING);
            }
            else
            {
                // 进入CAN消息发送待机状态
                CAN_TxEnterState(CAN_TX_STATE_STANDBY);
            }
        }
        else
        {
            // 否则什么也不处理
        }

        break;

    // ■■CAN消息发送中■■
    case CAN_TX_STATE_SENDING:

        break;

    default:
        break;
    }
}

// 功能函数：CAN初始化
void CAN_HW_Init(uint16 bund)
{
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;

    // 时钟分配与使能
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOB);
    // rcu_periph_clock_enable(RCU_GPIOA); // 后加
    rcu_periph_clock_enable(RCU_AF);

    // 配置CAN的接收中断
    nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 1, 0);

    // 配置CAN邮箱空中断
    nvic_irq_enable(USBD_HP_CAN0_TX_IRQn, 2, 0);

    // CAN管脚重映射到PB8-9
      gpio_pin_remap_config(GPIO_CAN_PARTIAL_REMAP,ENABLE);

    /* configure CAN0 GPIO */
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_INIT_STRUCT, &can_filter);

    /* initialize CAN register */
    can_deinit(CAN0);

    /* initialize CAN parameters */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.no_auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;

    // 波特率计算：36MHz / (1+6+1) / 36 = 125K bit
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_6TQ;// 6
    can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;// 1
    if (CAN_BAUD_RATE_125K == bund)
    {
        can_parameter.prescaler = 36;
    }
    else if (CAN_BAUD_RATE_250K == bund)
    {
        can_parameter.prescaler = 18;
    }
    else if (CAN_BAUD_RATE_500K == bund)
    {
        can_parameter.prescaler = 9;
    }
    else
    {
        can_parameter.prescaler = 36;
    }

    /* initialize CAN */
    can_init(CAN0, &can_parameter);

    /* initialize filter */
    can_filter.filter_number = 1;
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;

    can_filter_init(&can_filter);

    /* enable can receive FIFO0 not empty interrupt */
    can_interrupt_enable(CAN0, CAN_INT_RFNE0);
}

// ISR for CAN RX Interrupt
void USBD_LP_CAN0_RX0_IRQHandler(void)
{
    can_receive_message_struct rxMessage1;

    // 读出FIFO中所有的数据
    while (can_receive_message_length_get(CAN0, CAN_FIFO0))
    {
        can_message_receive(CAN0, CAN_FIFO0, &rxMessage1);

        //switch (dut_info.ID)
        //{
        //case DUT_TYPE_GB:
        //  CAN_MacProcess(rxMessage1.rx_efid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
        //  CAN_PROTOCOL_MacProcess(rxMessage1.rx_efid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
        //  break;
        //default:
        CAN_PROTOCOL1_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen); // 升级config
        //  CAN_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);           // 55升级协议
        //  break;
        //}
    }
}

// ISR for CAN TX Interrupt
void USBD_HP_CAN0_TX_IRQHandler(void)
{
    uint16 head = sysCanCB.tx.head;
    uint16 end = sysCanCB.tx.end;

    // 清空邮箱空中断标志位
    can_interrupt_disable(CAN0, CAN_INT_TME);

    // 检测是否有Tx请求
    if (head != end)
    {
        // Tx成功，判断是否有下一帧
        CAN_Tx_NextFram(&sysCanCB);

        // 进入CAN消息下一帧发送状态
        CAN_Tx_FillAndSend(&sysCanCB);
    }
}

//===============================================================================
// 详细命令设计
// ==============================================================================

// 发送命令不带应答
void IAP_CTRL_CAN_SendCmdNoAck(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddData(0);
    IAP_CTRL_CAN_TxAddData(1); // 段码屏工程允许
    IAP_CTRL_CAN_TxAddFrame();
}

// 发送升级数据
void IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_CMD cmd, uint32 flashAddr, uint32 addr)
{
    uint8 i = 0;
    switch (upDataType)
    {
    case DUT_FILE_TYPE_HEX:                          // hex
        dut_info.reconnectionRepeatContent[0] = 121; // 总长度
        dut_info.reconnectionRepeatContent[1] = CAN_CMD_HEAD;
        dut_info.reconnectionRepeatContent[2] = cmd;
        dut_info.reconnectionRepeatContent[3] = 0;                                            // 数据长度
        dut_info.reconnectionRepeatContent[4] = 0;                                            // 添加是否加密
        dut_info.reconnectionRepeatContent[5] = 0;                                            // 添加密钥
        SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[6], flashAddr + (addr), 116); // 工具读取 4+112 个字节
        for (i = 0; i < 121; i++)                                                             // 添加数据
        {
            IAP_CTRL_CAN_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
        }
        break;

    case DUT_FILE_TYPE_BIN:                          // bin
        dut_info.reconnectionRepeatContent[0] = 135; // 总长度
        dut_info.reconnectionRepeatContent[1] = CAN_CMD_HEAD;
        dut_info.reconnectionRepeatContent[2] = cmd;
        dut_info.reconnectionRepeatContent[3] = 0;                                            // 数据长度
        dut_info.reconnectionRepeatContent[4] = ((addr) & 0xFF000000) >> 24;                  // 数据长度
        dut_info.reconnectionRepeatContent[5] = ((addr) & 0x00FF0000) >> 16;                  // 数据长度
        dut_info.reconnectionRepeatContent[6] = ((addr) & 0x0000FF00) >> 8;                   // 数据长度
        dut_info.reconnectionRepeatContent[7] = ((addr) & 0x000000FF);                        // 数据长度
        SPI_FLASH_ReadArray(&dut_info.reconnectionRepeatContent[8], flashAddr + (addr), 128); // 工具读取128字节
        for (i = 0; i < 135; i++)                                                             // 添加数据
        {
            IAP_CTRL_CAN_TxAddData(dut_info.reconnectionRepeatContent[i + 1]);
        }
        break;
    default:
        break;
    }

    IAP_CTRL_CAN_TxAddFrame(); // 调整帧格式,修改长度和添加校验
}

// 工程模式启用
void IAP_CTRL_CAN_SendCmdProjectApply(uint8 *param)
{
    uint8 len = strlen((char *)param);
    uint8 i;

    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_PROJECT_APPLY);
    IAP_CTRL_CAN_TxAddData(len); // 数据长度

    for (i = 0; i < len; i++)
    {
        IAP_CTRL_CAN_TxAddData(param[i]);
    }

    IAP_CTRL_CAN_TxAddFrame();
}

// 工程模式准备就绪报告
void IAP_CTRL_CAN_SendCmdProjectReady(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_PROJECT_READY);
    IAP_CTRL_CAN_TxAddData(1); // 数据长度
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddFrame();
}

// 擦除FLASH结果上报
void IAP_CTRL_CAN_SendCmdEraseFlashResult(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_IAP_ERASE_FLASH_RESULT);
    IAP_CTRL_CAN_TxAddData(1); // 数据长度
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddFrame();
}

// IAP数据写入结果上报
void IAP_CTRL_CAN_SendCmdWriteFlashResult(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_IAP_WRITE_FLASH_RESULT);
    IAP_CTRL_CAN_TxAddData(1); // 数据长度
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddFrame();
}

// 查空结果上报
void IAP_CTRL_CAN_SendCmdCheckFlashBlankResult(uint8 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_CHECK_FLASH_BLANK_RESULT);
    IAP_CTRL_CAN_TxAddData(1); // 数据长度
    IAP_CTRL_CAN_TxAddData(param);
    IAP_CTRL_CAN_TxAddFrame();
}

//// 系统升级结束确认
void IAP_CTRL_CAN_SendCmdUpdataFinishResult(uint8 param)
{
  IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
  IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_UPDATA_FINISH_RESULT);
  IAP_CTRL_CAN_TxAddData(0); // 数据长度
  IAP_CTRL_CAN_TxAddFrame();
}

//// 发送UI数据擦除结果
//void IAP_CTRL_CAN_SendCmdUIEraseResultReport(uint8 param)
//{
//  IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
//  IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_UI_DATA_ERASE_ACK);
//  IAP_CTRL_CAN_TxAddData(1); // 数据长度
//  IAP_CTRL_CAN_TxAddData(param);
//  IAP_CTRL_CAN_TxAddFrame();
//}

//// 发送UI数据写入结果
//void IAP_CTRL_CAN_SendCmdUIWriteDataResultReport(uint8 param)
//{
//  IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
//  IAP_CTRL_CAN_TxAddData(IAP_CTRL_CAN_CMD_UP_UI_DATA_WRITE_RESULT);
//  IAP_CTRL_CAN_TxAddData(1); // 数据长度
//  IAP_CTRL_CAN_TxAddData(param);
//  IAP_CTRL_CAN_TxAddFrame();
//}

// 发送命令测试
void IAP_CTRL_CAN_SendCmdTest(uint32 param)
{
    IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
    IAP_CTRL_CAN_TxAddData(1);
    IAP_CTRL_CAN_TxAddData(0);
    IAP_CTRL_CAN_TxAddData(1); // 段码屏工程允许
    IAP_CTRL_CAN_TxAddFrame();
}
