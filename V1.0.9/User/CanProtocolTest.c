/*********************************************************
 * 头文件区
 *********************************************************/
// can通用头文件
#include "CanProtocolTest.h"

// 设备头文件（根据需求添加与删除）
#include "system.h"
#include "timer.h"
#include "spiflash.h"
#include "iap.h"
#include "gd32f10x_can.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "canProtocol_3A.h"
#include "state.h"
#include "StsProtocol.h"
#include "PowerProtocol.h"
#include "stringOperation.h"
#include "DutUartProtocol.h"
#include "CanProtocolUpDT.h"

/*********************************************************
 * 变量定义区
 *********************************************************/
CAN_TEST_CB testCanCB;            // CAN通信数据结构
CAN_TEST_TX_STATE_CB canTestTxStateCB; // 发送状态机控制对象

/*********************************************************
 * 内部函数申明区
 *********************************************************/
/************ CAN底层函数 *************************/

// 数据结构初始化
void CAN_TEST_DataStructureInit(CAN_TEST_CB *pCB);

// CAN 报文接收处理函数(注意根据具体模块修改)
void CAN_TEST_MacProcess(uint16 standarID, uint8 *data, uint8 length);

// 一级报文接收缓冲区处理函数，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_TEST_RXFIFOProcess(CAN_TEST_CB *pCB);

// CAN命令帧缓冲区处理
void CAN_TEST_CmdFrameProcess(CAN_TEST_CB *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_TEST_CheckSum(uint8 *buff, uint32 len);

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_TEST_AddSum(uint8 *buff, uint32 len);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_TEST_ConfirmTempCmdFrameBuff(CAN_TEST_CB *pCB, uint8 sourceSelect);

/************ CAN状态机函数 **************************/
// 填充与发送处理
BOOL CAN_TEST_Tx_FillAndSend(CAN_TEST_CB *pCB);

// 发送成功启动下一帧
BOOL CAN_TEST_Tx_NextFram(CAN_TEST_CB *pCB);

// CAN发送状态机初始化
void CAN_TEST_TxStateInit(void);

// CAN发送状态迁移
void CAN_TEST_TxEnterState(uint32 state);

// CAN发送状态机处理
void CAN_TEST_TxStateProcess(void);

/************ CAN底层函数 **************************/

// 工程模式准备就绪报告
void CAN_PROTOCOL_TEST_SendCmdProjectReady(uint8 param);

// 擦除FLASH结果上报
void CAN_PROTOCOL_TEST_SendCmdEraseFlashResult(uint8 param);

// IAP数据写入结果上报
void CAN_PROTOCOL_TEST_SendCmdWriteFlashResult(uint8 param);

// 查空结果上报
void CAN_PROTOCOL_TEST_SendCmdCheckFlashBlankResult(uint8 param);

// 系统升级结束确认
void CAN_PROTOCOL_TEST_SendCmdUpdataFinishResult(uint8 param);

// 发送UI数据擦除结果
void CAN_PROTOCOL_TEST_SendCmdUIEraseResultReport(uint8 param);

// 发送UI数据写入结果
void CAN_PROTOCOL_TEST_SendCmdUIWriteDataResultReport(uint8 param);

/*********************************************************
 * 全局变量定义
 *********************************************************/
extern uint8 cali_cnt;
//CAN_PROTOCOL1_CB canProtocol1CB;
uint8 can_light_cnt = 0;
uint8 can_left_light_cnt = 0;
uint8 can_right_light_cnt = 0;
uint8 can_cnt = 0;
uint8 can_cali_cnt = 0;
uint8 can_brake_cnt = 0;
uint8 can_throttleAd_cnt = 0;
uint8 can_derailleurAd_cnt = 0;

// 透传
char ptUpMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "P-T Completion",
    "",
    "",
    "",
};

/*********************************************************
 * 函数定义区
 *********************************************************/
// CAN 初始化
void CAN_PROTOCOL_TEST_Init(uint16 bund)
{
    // 初始化硬件配置
    CAN_TEST_HW_Init(bund);

    // 初始化结构体
    CAN_TEST_DataStructureInit(&testCanCB);

    // CAN发送状态机初始化
    CAN_TEST_TxStateInit();

    //  TIMER_AddTask(TIMER_ID_CAN_TEST,//测试
    //                          500,
    //                          CAN_TEST,
    //                          TRUE,
    //                          -1,
    //                          ACTION_MODE_ADD_TO_QUEUE);
}

// 数据结构初始化
void CAN_TEST_DataStructureInit(CAN_TEST_CB *pCB)
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
    for (i = 0; i < CAN_TEST_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    for (i = 0; i < CAN_TEST_DEVICE_SOURCE_ID_MAX; i++)
    {
        pCB->rxFIFO.rxFIFOEachNode[i].head = 0;
        pCB->rxFIFO.rxFIFOEachNode[i].end = 0;
        pCB->rxFIFO.rxFIFOEachNode[i].currentProcessIndex = 0;
    }

    for (i = 0; i < CAN_TEST_DEVICE_SOURCE_ID_MAX; i++)
    {
        for (j = 0; j < CAN_TEST_RX_QUEUE_SIZE; j++)
        {
            pCB->rx.rxEachNode[i].cmdQueue[j].length = 0;
        }
        pCB->rx.rxEachNode[i].head = 0;
        pCB->rx.rxEachNode[i].end = 0;
    }
}

// 向发送命令帧队列中添加数据
void CAN_PROTOCOL_TEST_TxAddData(uint8 data)
{
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;
    CAN_PROTOCOL_TEST_FRAME *pCmdFrame = &testCanCB.tx.cmdQueue[testCanCB.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % CAN_TEST_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// 发送命令帧处理,自动扶正帧长度以及检验码
void CAN_PROTOCOL_TEST_TxAddFrame(void)
{
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;
    uint8 *buff = testCanCB.tx.cmdQueue[end].buff;
    uint16 length = testCanCB.tx.cmdQueue[end].length;
    uint8 cmdHead = buff[CAN_PROTOCOL_TEST_HEAD_INDEX]; // 帧头

    // 发送缓冲区已满，不予接收
    if ((end + 1) % CAN_TEST_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 命令头错误
    if (CAN_PROTOCOL_TEST_HEAD != cmdHead)
    {
        // 把帧数据索引放在最开始位置
        testCanCB.tx.cmdQueue[end].length = 0;
        return;
    }

    // 命令头正确，但数据长度不对(数据长度减去CRC的长度)
    if ((CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX - CAN_CHECK_BYTE_SIZE < length) || (CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN - CAN_CHECK_BYTE_SIZE > length))
    {
        // 把帧数据索引放在最开始位置
        testCanCB.tx.cmdQueue[end].length = 0;
        buff[CAN_PROTOCOL_TEST_HEAD_INDEX] = CAN_PROTOCOL_TEST_NONHEAD; // 避免数据没更新时，再次进入
        return;
    }

    // 执行到此，命令头正确，数据长度正确,发送缓冲区未满
    // 长度扶正
    buff[CAN_PROTOCOL_TEST_LENGTH_INDEX] = length - CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN + CAN_CHECK_BYTE_SIZE;

    // 填充CRC校验码
    CAN_TEST_AddSum(buff, length);

    // 将数据长度填到待发送缓冲器
    testCanCB.tx.cmdQueue[end].length = length + CAN_CHECK_BYTE_SIZE;

    // 发送环形队列更新位置
    testCanCB.tx.end++;
    testCanCB.tx.end %= CAN_TEST_TX_QUEUE_SIZE;
    testCanCB.tx.cmdQueue[testCanCB.tx.end].length = 0; // 2015.12.2修改
}

// 填充与发送处理:TRUE,填充与发送；FALSE，环形队列向前滑移.
BOOL CAN_TEST_Tx_FillAndSend(CAN_TEST_CB *pCB)
{
    uint16 index = pCB->tx.index;                          // 当前发送数据的索引号
    uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length; // 当前发送的命令帧的长度
    uint16 head = pCB->tx.head;                            // 发送命令帧队列头索引号
    uint16 end = pCB->tx.end;                              // 发送命令帧队列尾索引号
    CAN_PROTOCOL_TEST_FRAME *pCmd = &pCB->tx.cmdQueue[pCB->tx.head]; // 取当前发送缓冲区地址
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

        if(testFlag)
        {
            // 扩展帧
            canTxMsgCB.tx_ff = CAN_FF_EXTENDED;
//            extendedID = pCmd->deviceID;
            extendedID = 0x1801FFF4;
            canTxMsgCB.tx_efid = extendedID;
        }
        else
        {
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
                    standardID = (uint16)CAN_TEST_DEVICE_ID_HMI; // id：001
                    canTxMsgCB.tx_sfid = standardID;
                    break;
            }
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
        pCB->tx.head %= CAN_TEST_TX_QUEUE_SIZE;
        pCB->tx.index = 0;

        // 进入CAN消息发送待机状态
        CAN_TEST_TxEnterState(CAN_TEST_TX_STATE_STANDBY);

        return FALSE;
    }
}

// 发送成功启动下一帧:TRUE,启动下一帧；FALSE，发送完成.
BOOL CAN_TEST_Tx_NextFram(CAN_TEST_CB *pCB)
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
void CAN_TEST_MacProcess(uint16 standarID, uint8 *data, uint8 length)
{
    uint16 end;
    uint16 head;
    uint8 i;
    int8 sourceBuffSelect = 0; // 节点缓存区编号选择

    // 参数合法性判断
    if (NULL == data)
    {
        return;
    }

    // 标准帧判断
//    if (standarID > 0x7FF)
//    {
//        return;
//    }

    // 取出目标节点
    //  destinationID = standarID & 0x07FF;

    // 不是发给自身ID的数据直接丢掉
    //  if (CAN_TEST_DEVICE_ID_HMI != destinationID)
    //  {
    //      return;
    //  }

    // 已经使能
//    if (CAN_TEST_RX_DEVICE_PUC)
//    {
//        sourceBuffSelect = CAN_TEST_RX_DEVICE_PUC - 1;
//    }

//    // 没有合适的源节点
//    if ((sourceBuffSelect >= CAN_TEST_DEVICE_SOURCE_ID_MAX) || (sourceBuffSelect < 0))
//    {
//        return;
//    }

    // 取相应缓存区结构体数据
    end = testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end;
    head = testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].head;

    // ■■环形列队，入队■■
    //  一级缓冲区已满，不予接收
//    if ((end + 1) % CAN_TEST_RX_FIFO_SIZE == head)
//    {
//        return;
//    }

    for (i = 0; i < length; i++)
    {
        // 单个字节读取，并放入FIFO中
        testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].buff[testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end] = *data++;

        testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end++;

        // 一级缓冲区已满，不予接收
        if ((testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end + 1) % CAN_TEST_RX_FIFO_SIZE == head)
        {
            break;
        }

        testCanCB.rxFIFO.rxFIFOEachNode[sourceBuffSelect].end %= CAN_TEST_RX_FIFO_SIZE;
    }
}

// 一级报文接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void CAN_TEST_RxFIFOProcess(CAN_TEST_CB *pCB)
{
    uint16 end;
    uint16 head;
    CAN_PROTOCOL_TEST_FRAME *pCmdFrame = NULL;
    CAN_TEST_RX_FIFO *pRxFIFO = NULL;
    uint16 length = 0;
    uint8 currentData = 0;
    uint8 sourceBuffSelect = 0;

    // 参数合法性检验
    if (NULL == pCB)
    {
        return;
    }

    // 循环判断是否有数据需要处理
//    for (sourceBuffSelect = 0; sourceBuffSelect < CAN_TEST_DEVICE_SOURCE_ID_MAX; sourceBuffSelect++)
//    {
        // 获取接收一级缓冲区节点临时缓冲区指针
        pRxFIFO = &pCB->rxFIFO.rxFIFOEachNode[sourceBuffSelect];

        end = pRxFIFO->end;
        head = pRxFIFO->head;

        // 一级缓冲区为空，继续
        if (head == end)
        {
            return;
        }

        // 获取接收命令临时缓冲区指针
        pCmdFrame = &pCB->rx.rxEachNode[sourceBuffSelect].cmdQueue[pCB->rx.rxEachNode[sourceBuffSelect].end];

        // 取出当前要处理的字节
        currentData = pRxFIFO->buff[pRxFIFO->currentProcessIndex];

        // 临时缓冲区长度为0时，搜索首字节
        if (0 == pCmdFrame->length)
        {
            // 命令头错误，删除当前字节并退出
            if (CAN_PROTOCOL_TEST_HEAD != currentData)
            {
                pRxFIFO->head++;
                pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                return;
            }

            // 命令头正确，但无命令临时缓冲区可用，退出
            if ((pCB->rx.rxEachNode[sourceBuffSelect].end + 1) % CAN_TEST_RX_QUEUE_SIZE == pCB->rx.rxEachNode[sourceBuffSelect].head)
            {
                return;
            }

            // 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
            pCmdFrame->buff[pCmdFrame->length++] = currentData;
            pRxFIFO->currentProcessIndex++;
            pRxFIFO->currentProcessIndex %= CAN_TEST_RX_FIFO_SIZE;
        }
        // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
        else
        {
            // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
            if (pCmdFrame->length >= CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX)
            {
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pRxFIFO->head++;
                pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                return;
            }

            // 一直取到末尾
            while (end != pRxFIFO->currentProcessIndex)
            {
                // 取出当前要处理的字节
                currentData = pRxFIFO->buff[pRxFIFO->currentProcessIndex];

                // 缓冲区未溢出，正常接收，将数据添加到临时缓冲区中
                pCmdFrame->buff[pCmdFrame->length++] = currentData;
                pRxFIFO->currentProcessIndex++;
                pRxFIFO->currentProcessIndex %= CAN_TEST_RX_FIFO_SIZE;

                // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■

                // 首先判断命令帧最小长度，一个完整的命令字至少包括4个字节: 命令头(1Byte)+命令字(1Byte)+数据长度(1Byte)+校验码(1Byte) ，因此不足4个字节的必定不完整
                if (pCmdFrame->length < CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN)
                {
                    // 继续接收
                    continue;
                }

                // 命令帧长度数值越界，说明当前命令帧错误，停止接收
                if (pCmdFrame->buff[CAN_PROTOCOL_TEST_LENGTH_INDEX] > (CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX - CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN))
                {
                    // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                    pCmdFrame->length = 0;

                    // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                    pRxFIFO->head++;
                    pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                    pRxFIFO->currentProcessIndex = pRxFIFO->head;

                    break;
                }

                // 命令帧长度校验，在命令长度描述字的数值上，增加命令头、命令字、...、数据长度、校验码，即为命令帧实际长度
                length = pCmdFrame->length;
                if (length < (pCmdFrame->buff[CAN_PROTOCOL_TEST_LENGTH_INDEX] + CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN))
                {
                    // 长度要求不一致，说明未接收完毕，继续
                    continue;
                }

                // 命令帧长度OK，则进行校验，失败时删除命令头
                if (!CAN_TEST_CheckSum(pCmdFrame->buff, pCmdFrame->length))
                {
                    // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                    pCmdFrame->length = 0;

                    // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                    pRxFIFO->head++;
                    pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                    pRxFIFO->currentProcessIndex = pRxFIFO->head;

                    break;
                }

                // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
                pRxFIFO->head += length;
                pRxFIFO->head %= CAN_TEST_RX_FIFO_SIZE;
                pRxFIFO->currentProcessIndex = pRxFIFO->head;

                CAN_TEST_ConfirmTempCmdFrameBuff(pCB, sourceBuffSelect);
                break;
            }
        }
//    }
    return;
}


// CAN命令帧缓冲区处理
void CAN_TEST_CmdFrameProcess(CAN_TEST_CB *pCB)
{
    CAN_PROTOCOL_TEST cmd = CAN_PROTOCOL_TEST_CMD_NULL;
    CAN_PROTOCOL_TEST_FRAME *pCmdFrame = NULL;
    uint8 sourceBuffSelect = CAN_TEST_DEVICE_SOURCE_ID_MAX;
    volatile uint32 temp = 0;
    volatile uint16 length;
    volatile uint8 errStatus;
    volatile uint16 i;
    uint8 buffFlag[4] = {0xFF};
    uint8_t verType = 0;
    uint8_t infoLen = 0;
    char  verBuff[100] = {0};
    uint8 calibration_result = 0;
    uint8 switch_state = 0;
    uint8 Peripheral_type = 0;
    uint8 turn_item = 0;
    uint8 turn_state = 0;
    BOOL testResult = 0;


    // 参数合法性检验
    if (NULL == pCB)
    {
        return;
    }

    // 循环判断是否有数据需要处理
    for (sourceBuffSelect = 0; sourceBuffSelect < CAN_TEST_DEVICE_SOURCE_ID_MAX; sourceBuffSelect++)
    {
        // 命令帧缓冲区为空，退出
        if (pCB->rx.rxEachNode[sourceBuffSelect].head == pCB->rx.rxEachNode[sourceBuffSelect].end)
        {
            continue;
        }

        // 获取当前要处理的命令帧指针
        pCmdFrame = &pCB->rx.rxEachNode[sourceBuffSelect].cmdQueue[pCB->rx.rxEachNode[sourceBuffSelect].head];

        // 命令头非法，退出
        if (CAN_PROTOCOL_TEST_HEAD != pCmdFrame->buff[CAN_PROTOCOL_TEST_HEAD_INDEX])
        {
            // 删除命令帧
            pCB->rx.rxEachNode[sourceBuffSelect].head++;
            pCB->rx.rxEachNode[sourceBuffSelect].head %= CAN_TEST_RX_QUEUE_SIZE;
            continue;
        }

        // 命令头合法，则提取命令
        cmd = (CAN_PROTOCOL_TEST)(pCmdFrame->buff[CAN_PROTOCOL_TEST_CMD_INDEX]);

        switch (cmd)
        {
            // 空命令
            case CAN_PROTOCOL_TEST_CMD_NULL:
                break;

            // 大灯控制
            case CAN_PROTOCOL_TEST_CMD_HEADLIGHT_CONTROL:
                switch_state = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];   // 执行结果

                // 执行失败
                if (0x00 == (switch_state & 0x00))
                {
    #if DEBUG_ENABLE
                    Clear_All_Lines();
                    Display_Centered(1, "Headlights fail to turn on");
    #endif
                    can_light_cnt = 0;
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                }

                // 执行成功
                if (0x01 == (switch_state & 0x01))
                {
                    can_light_cnt++;
                    if (can_light_cnt == 1)
                    {
                        STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_OPEN);
                    }
                    if (can_light_cnt == 2)
                    {
                        STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_COMPLETE);
                        can_light_cnt = 0;
                    }
                }
                break;

            // 获取油门/刹车的模拟数字值
            case CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD:

                // 获取AD值并且比对
                Peripheral_type = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX];   // 外设类型

                if (0x01 == (Peripheral_type & 0x01))  // 油门
                {
                    dut_info.throttleAd = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX] << 8;
                    dut_info.throttleAd = dut_info.throttleAd | pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX];
                    can_throttleAd_cnt++;
                    switch (can_throttleAd_cnt)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_CHECK_DAC1_VALUE);
                            break;
                            
                        case 2:
                            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_CHECK_DAC3_VALUE);
                            break;
                            
                        case 3:
                            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_CHECK_DAC5_VALUE);
                            can_throttleAd_cnt = 0;
                            break;
                            
                        default:
                            break;
                    }
                }

                if (0x02 == (Peripheral_type & 0x02))  // 刹把
                {
                    dut_info.brakeAd = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX] << 8;
                    dut_info.brakeAd = dut_info.brakeAd | pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX];
                    can_brake_cnt++;
                    
                    switch (can_brake_cnt)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_CHECK_DAC1_VALUE);
                            break;
                            
                        case 2:
                            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_CHECK_DAC3_VALUE);
                            break;
                            
                        case 3:
                            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_CHECK_DAC5_VALUE);
                            can_brake_cnt = 0;
                            break;
                            
                        default:
                            break;
                    }
                }

                // 电子变速
                if (0x03 == (Peripheral_type & 0x03))
                {
                    // 获取刹把AD值
                    dut_info.derailleurAd = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX] << 8;
                    dut_info.derailleurAd = dut_info.derailleurAd | pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX];
                    can_derailleurAd_cnt++;
                    
                    switch (can_derailleurAd_cnt)
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
                            can_derailleurAd_cnt = 0;
                            break;

                        default:
                            break;
                    }
                }
                break;

            // 转向灯控制
            case CAN_PROTOCOL_TEST_CMD_TURN_SIGNAL_CONTROL:
                turn_item = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX];  // 转向灯类型
                turn_state = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX]; // 执行结果

                // 执行失败
                if (0x00 == (switch_state & 0x00))
                {
    #if DEBUG_ENABLE
                    Clear_All_Lines();
                    Display_Centered(1, "TURN SIGNAL fail to turn on");
    #endif
                    can_left_light_cnt = 0;
                    can_right_light_cnt = 0;
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                }

                // 左转向灯打开成功
                if ((turn_item == 0) && (turn_state == 1))
                {
                    can_left_light_cnt++;
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_OPEN);
                    if (can_left_light_cnt == 2)
                    {
                        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_COMPLETE);
                        can_left_light_cnt = 0;
                    }
                }

                // 右转向灯打开成功
                if ((turn_item == 1) && (turn_state == 1))
                {
                    can_right_light_cnt++;
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_OPEN);
                    if (can_right_light_cnt == 2)
                    {
                        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_COMPLETE);
                        can_right_light_cnt = 0;
                    }
                }
                break;

            // 在线检测
            case CAN_PROTOCOL_TEST_CMD_ONLINE_DETECTION:
                online_detection_cnt++;            
                TIMER_ResetTimer(TIMER_ID_ONLINE_DETECT);
                
                if(1 == online_detection_cnt)
                {
                   // 上报仪表启动状态
                   STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION,TRUE);
                }
                break;

            // 获取光敏传感器的数值
            case CAN_PROTOCOL_TEST_CMD_GET_PHOTORESISTOR_VALUE:
                break;

            // 进行电压校准
            case CAN_PROTOCOL_TEST_CMD_VOLTAGE_CALIBRATION:
                cali_cnt++;
                calibration_result = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];

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

            // 读取标志区数据
            case CAN_PROTOCOL_TEST_CMD_READ_FLAG_DATA:
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
            case CAN_PROTOCOL_TEST_CMD_WRITE_FLAG_DATA:

                // 关闭写入超时定时器
                TIMER_ResetTimer(TIMER_ID_WRITE_FLAG_TIMEOUT);

                // 收到写入成功再读取出来进行校验
                if (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
                {
                    CAN_PROTOCOL_TEST_SendCmdAck(CAN_PROTOCOL_TEST_CMD_READ_FLAG_DATA);
                }
                else
                {
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
                }
                break;

            // 写入版本类型数据
            case CAN_PROTOCOL_TEST_CMD_WRITE_VERSION_TYPE_DATA:

                // 关闭定时器
                TIMER_ResetTimer(TIMER_ID_OPERATE_VER_TIMEOUT);

                // 获取类型
                verType =  pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX];

                // 类型正确并且执行成功，上报STS
                if ((dutverType == verType) && (pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX]))
                {
                    // 读取版本类型信息
                    CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_READ_VERSION_TYPE_INFO, dutverType);
                }
                else
                {
                    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_VERSION_INFO, FALSE);
                }
                break;

            // 读取版本类型信息
            case CAN_PROTOCOL_TEST_CMD_READ_VERSION_TYPE_INFO:
                TIMER_KillTask(TIMER_ID_OPERATE_VER_TIMEOUT);

                // 获取类型
                verType =  pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX];

                // 获取信息长度
                infoLen = pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX];

                // 获取类型信息
                strcpy(verBuff, (const char *)&pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX]);
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
            case CAN_PROTOCOL_TEST_CMD_BLUETOOTH_MAC_ADDRESS_READ:

                // 版本比对成功将MAC地址发给电源板连接
                if (pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX])
                {
                    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
                    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
                    POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT);
                    POWER_PROTOCOL_TxAddData(12);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA4_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA5_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA6_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA7_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA8_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA9_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA10_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA11_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA12_INDEX]);
                    POWER_PROTOCOL_TxAddData(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA13_INDEX]);
                    POWER_PROTOCOL_TxAddFrame();
                }

                // 版本比对失败上报STS
                if (0 == pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX])
                {
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                    STS_PROTOCOL_TxAddData(7);
                    STS_PROTOCOL_TxAddData(1);
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA2_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA3_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA4_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA5_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA6_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA7_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA8_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA9_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA10_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA11_INDEX]) & 0x0F)); 
                    STS_PROTOCOL_TxAddData((HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA12_INDEX]) << 4) | (HexToChar(pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA13_INDEX]) & 0x0F)); 
                    
                    for(i = 0; i < 20 ;i++)
                    {
                      STS_PROTOCOL_TxAddData(dut_info.bleVerBuff[i]);
                    }
                
                    STS_PROTOCOL_TxAddFrame();
                }
                break;

            // LCD 颜色测试（命令字=0x03）
            case CAN_PROTOCOL_TEST_CMD_TEST_LCD:
                STS_PROTOCOL_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_TEST_LCD, TRUE);
                break;

            // Flash 校验测试
            case CAN_PROTOCOL_TEST_CMD_FLASH_CHECK_TEST:
                TIMER_ChangeTime(TIMER_ID_ONLINE_DETECT, 3000);
                STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);
                break;
                
             // 按键测试
            case CAN_PROTOCOL_TEST_CMD_KEY_TEST:
             
                // 按键模拟测试
                if(dut_info.buttonSimulationFlag)
                {
                    if(keyValue == pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX])
                    {
                         testResult = TRUE;
                    }
                    else
                    {
                         testResult = FALSE;
                    }
                    STS_PROTOCOL_SendCmdParamTwoAck(STS_PROTOCOL_CMD_SIMULATION_KEY_TEST, testResult, pCmdFrame->buff[CAN_PROTOCOL_TEST_DATA1_INDEX]);                
                }
                // 按键正常测试
                else
                {
                    STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);                
                }
                break;               
                
            default:
                testFlag = FALSE;
                STS_UART_AddTxArray(0, pCmdFrame->buff, pCmdFrame->length);

                // LCD显示
                Vertical_Scrolling_Display(ptUpMsgBuff, 4, 0);
                break;
        }
        // 删除命令帧
        pCB->rx.rxEachNode[sourceBuffSelect].head++;
        pCB->rx.rxEachNode[sourceBuffSelect].head %= CAN_TEST_RX_QUEUE_SIZE;
    }
}

// CAN处理测试过程函数
void CAN_PROTOCOL_Process_Test(void)
{
    // CAN接收FIFO缓冲区处理
    CAN_TEST_RxFIFOProcess(&testCanCB);

    // CAN接收命令缓冲区处理
    CAN_TEST_CmdFrameProcess(&testCanCB);

    // CAN发送状态机处理过程
    CAN_TEST_TxStateProcess();
}

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_TEST_CheckSum(uint8 *buff, uint32 len)
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

//  cc = ~cc;

    // 判断计算得到的校验码与命令帧中的校验码是否相同
    if (buff[len - 1] != cc)
    {
        return FALSE;
    }

    return TRUE;
}

// 对传入的命令帧进行校验，返回校验结果
BOOL CAN_TEST_AddSum(uint8 *buff, uint32 len)
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

//  cc = ~cc;

    // 添加校验码
    buff[len] = cc;

    return TRUE;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL CAN_TEST_ConfirmTempCmdFrameBuff(CAN_TEST_CB *pCB, uint8 sourceSelect)
{
    CAN_PROTOCOL_TEST_FRAME *pCmdFrame = NULL;

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
    pCB->rx.rxEachNode[sourceSelect].end %= CAN_TEST_RX_QUEUE_SIZE;
    // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区
    pCB->rx.rxEachNode[sourceSelect].cmdQueue[pCB->rx.rxEachNode[sourceSelect].end].length = 0;

    return TRUE;
}

// CANTX状态机初始化
void CAN_TEST_TxStateInit(void)
{
    CAN_TEST_TxEnterState(CAN_TEST_TX_STATE_STANDBY);
}

// CANTX状态迁移函数
void CAN_TEST_TxEnterState(uint32 state)
{
    // 让当前的状态成为历史
    canTestTxStateCB.preState = canTestTxStateCB.state;

    // 设置新的状态
    canTestTxStateCB.state = (CAN_TEST_TX_STATE_E)state;

    switch (state)
    {
        // ■■状态的入口处理■■
        case CAN_TEST_TX_STATE_NULL:

            break;

        // ■■入口状态的入口处理■■
        case CAN_TEST_TX_STATE_ENTRY:

            break;

        // ■■待机状态状态的入口处理■■
        case CAN_TEST_TX_STATE_STANDBY:

            break;

        // ■■CAN消息发送中■■
        case CAN_TEST_TX_STATE_SENDING:

            break;

        default:
            break;
    }
}

// 向发送缓冲区中添加一条待发送序列
BOOL CAN_TEST_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length)
{
    uint16 i;
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;

    // 参数检验
    if ((NULL == pArray) || (0 == length))
    {
        return FALSE;
    }

    // 发送缓冲区已满，不予接收
    if ((end + 1) % CAN_TEST_TX_QUEUE_SIZE == head)
    {
        return FALSE;
    }

//    testCanCB.tx.cmdQueue[end].deviceID = id;
    testCanCB.tx.cmdQueue[end].deviceID = 0x1801FFF4;
    
    for (i = 0; i < length; i++)
    {
        testCanCB.tx.cmdQueue[end].buff[i] = *pArray++;
    }
    testCanCB.tx.cmdQueue[end].length = length;

    // 发送环形队列更新位置
    testCanCB.tx.end++;
    testCanCB.tx.end %= CAN_TEST_TX_QUEUE_SIZE;
    testCanCB.tx.cmdQueue[testCanCB.tx.end].length = 0;

    return TRUE;
}

// CANTX过程处理函数
void CAN_TEST_TxStateProcess(void)
{
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;

    switch (canTestTxStateCB.state)
    {
        // ■■状态的入口处理■■
        case CAN_TEST_TX_STATE_NULL:

            break;

        // ■■入口状态的入口处理■■
        case CAN_TEST_TX_STATE_ENTRY:

            break;

        // ■■待机状态状态的入口处理■■
        case CAN_TEST_TX_STATE_STANDBY:
        
            // 检测是否有Tx请求
            if (head != end)
            {
                // 调用此函数填充与启动发送can消息
                if (CAN_TEST_Tx_FillAndSend(&testCanCB))
                {
                    // 进入CAN消息发送中状态
                    CAN_TEST_TxEnterState(CAN_TEST_TX_STATE_SENDING);
                }
                else
                {
                    // 进入CAN消息发送待机状态
                    CAN_TEST_TxEnterState(CAN_TEST_TX_STATE_STANDBY);
                }
            }
            else
            {
                // 否则什么也不处理
            }

            break;

        // ■■CAN消息发送中■■
        case CAN_TEST_TX_STATE_SENDING:

            break;

        default:
            break;
    }
}

// 功能函数：CAN初始化
void CAN_TEST_HW_Init(uint16 bund)
{
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;

    // 时钟分配与使能
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOB);
//  rcu_periph_clock_enable(RCU_GPIOA); // 后加
    rcu_periph_clock_enable(RCU_AF);

    // 配置CAN的接收中断
    nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 1, 0);

    // 配置CAN邮箱空中断
    nvic_irq_enable(USBD_HP_CAN0_TX_IRQn, 2, 0);

    // CAN管脚重映射到PB8-9
    gpio_pin_remap_config(GPIO_CAN_PARTIAL_REMAP, ENABLE);

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
        if(dut_info.passThroughControl)           
        {
            dut_info.online_dete_cnt++;            
            TIMER_ResetTimer(TIMER_ID_ONLINE_DETECT);
            
//            // 防止累加越界
//            if(2 < dut_info.online_dete_cnt)
//            {
//                dut_info.online_dete_cnt = 2;
//            }
            
            if(1 == dut_info.online_dete_cnt)
            {
               // 上报仪表启动状态
               STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION,TRUE);
            }
        }
        
      can_message_receive(CAN0, CAN_FIFO0, &rxMessage1);

      switch (dut_info.ID)
      {
          case DUT_TYPE_GB:
              CAN_MacProcess(rxMessage1.rx_efid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
              CAN_PROTOCOL_MacProcess(rxMessage1.rx_efid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
              CAN_TEST_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
              break;
              
          default:
              CAN_PROTOCOL1_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen); // 升级config
              CAN_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);           // 55升级协议
              CAN_TEST_MacProcess(rxMessage1.rx_sfid, &rxMessage1.rx_data[0], rxMessage1.rx_dlen);
              break;
      }
  }
}

extern CAN_CB sysCanCB;	
//// ISR for CAN TX Interrupt
void USBD_HP_CAN0_TX_IRQHandler(void)
{
    uint16 head = testCanCB.tx.head;
    uint16 end = testCanCB.tx.end;
    uint16 head1 = sysCanCB.tx.head;
	uint16 end1 = sysCanCB.tx.end;
    
    // 清空邮箱空中断标志位
    can_interrupt_disable(CAN0, CAN_INT_TME);

    // 检测是否有Tx请求
    if (head != end)
    {
        // Tx成功，判断是否有下一帧
        CAN_TEST_Tx_NextFram(&testCanCB);

        // 进入CAN消息下一帧发送状态
        CAN_TEST_Tx_FillAndSend(&testCanCB);
    }

	// 检测是否有Tx请求
	if (head1 != end1)
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
void CAN_PROTOCOL_TEST_SendCmdAck(uint8 param)
{
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_HEAD);
    CAN_PROTOCOL_TEST_TxAddData(param);
    CAN_PROTOCOL_TEST_TxAddData(0);
    CAN_PROTOCOL_TEST_TxAddFrame();
}

// 发送命令测试
void CAN_PROTOCOL_TEST_SendCmdTest(uint32 param)
{
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_HEAD);
    CAN_PROTOCOL_TEST_TxAddData(1);
    CAN_PROTOCOL_TEST_TxAddData(0);
    CAN_PROTOCOL_TEST_TxAddData(1);
    CAN_PROTOCOL_TEST_TxAddFrame();
}

// 写入标志区数据
void CAN_PROTOCOL_TEST_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam)
{
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_HEAD);
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_CMD_WRITE_FLAG_DATA);
    CAN_PROTOCOL_TEST_TxAddData(0x06);

    CAN_PROTOCOL_TEST_TxAddData(placeParam);
    CAN_PROTOCOL_TEST_TxAddData(0x00);
    CAN_PROTOCOL_TEST_TxAddData(0x00);
    CAN_PROTOCOL_TEST_TxAddData(0x00);
    CAN_PROTOCOL_TEST_TxAddData(0x02);
    CAN_PROTOCOL_TEST_TxAddData(shutdownFlagParam);
    CAN_PROTOCOL_TEST_TxAddFrame();
}

// 发送命令带应答
void CAN_PROTOCOL_TEST_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
    CAN_PROTOCOL_TEST_TxAddData(CAN_PROTOCOL_TEST_HEAD);
    CAN_PROTOCOL_TEST_TxAddData(ackCmd);
    CAN_PROTOCOL_TEST_TxAddData(1);
    CAN_PROTOCOL_TEST_TxAddData(ackParam);
    CAN_PROTOCOL_TEST_TxAddFrame();
}
