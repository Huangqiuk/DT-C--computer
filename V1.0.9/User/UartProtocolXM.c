#include "common.h"
#include "timer.h"
#include "DutUartDrive.h"
#include "UartprotocolXM.h"
#include "StsProtocol.h"
#include "state.h"
#include "spiflash.h"
#include "AvoPin.h"
#include "AvoProtocol.h"
#include "param.h"
#include "DutInfo.h"
#include "PowerProtocol.h"
#include "stringOperation.h"
#include "DutUartProtocol.h"
#include "uartProtocol.h"
#include "uartProtocol3.h"

/******************************************************************************
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void UART_PROTOCOL_XM_DataStructInit(UART_PROTOCOL_XM_CB *pCB);

// UART报文接收处理函数(注意根据具体模块修改)
void UART_PROTOCOL_XM_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

// 对传入的命令帧进行校验，返回校验结果
BOOL UART_PROTOCOL_XM_CheckSUM(UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL UART_PROTOCOL_XM_ConfirmTempCmdFrameBuff(UART_PROTOCOL_XM_CB *pCB);

// UART协议层向驱动层注册数据发送接口
void UART_PROTOCOL_XM_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// 上报写配置参数结果
void UART_PROTOCOL_XM_ReportWriteParamResult(uint32 param); // 有用到

// 发送命令回复
void UART_PROTOCOL_XM_SendCmdAck(uint8 ackCmd);

// 发送命令回复，带一个参数
void UART_PROTOCOL_XM_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// 发送命令回复，带两个个参数
void UART_PROTOCOL_XM_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam);

// UART协议层过程处理
void UART_PROTOCOL_XM_Process(void);

// 全局变量定义
extern char ptUpMsgBuff[][MAX_ONE_LINES_LENGTH];

UART_PROTOCOL_XM_CB uartProtocolCB5;
////uint8 light_cnt = 0;
//uint8 cnt = 0;
////uint8 cali_cnt = 0;
//uint8 brake_cnt = 0;
//uint8 left_light_cnt = 0;
//uint8 right_light_cnt = 0;
//BOOL write_success_Flag;
//uint32 online_detection_cnt = 0;
uint8 conVer[20];
uint8 verLenght = 0;

const uint16 crc16Tab[] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

uint16 calCrc16(unsigned char *buf, int len)
{
    int i;
    unsigned short cksum;

    cksum = 0;
    for (i = 0; i < len; i++)
    {
        cksum = crc16Tab[((cksum >> 8) ^ *buf++) & 0xFF] ^ (cksum << 8);
    }

    return cksum;
}

// 协议初始化
void UART_PROTOCOL_XM_Init(void)
{
    // 协议层数据结构初始化
    UART_PROTOCOL_XM_DataStructInit(&uartProtocolCB5);

    // 向驱动层注册数据接收接口
    UART_DRIVE_RegisterDataSendService(UART_PROTOCOL_XM_MacProcess);

    // 向驱动层注册数据发送接口
    UART_PROTOCOL_XM_RegisterDataSendService(UART_DRIVE_AddTxArray);
}

// UART协议层过程处理
void UART_PROTOCOL_XM_Process(void)
{
    // UART接收FIFO缓冲区处理
    UART_PROTOCOL_XM_RxFIFOProcess(&uartProtocolCB5);

    // UART接收命令缓冲区处理
    UART_PROTOCOL_XM_CmdFrameProcess(&uartProtocolCB5);

    // UART协议层发送处理过程
    UART_PROTOCOL_XM_TxStateProcess();
}

// 向发送命令帧队列中添加数据
void UART_PROTOCOL_XM_TxAddData(uint8 data)
{
    uint16 head = uartProtocolCB5.tx.head;
    uint16 end = uartProtocolCB5.tx.end;
    UART_PROTOCOL_XM_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB5.tx.cmdQueue[uartProtocolCB5.tx.end];

    // 发送缓冲区已满，不予接收
    if ((end + 1) % UART_PROTOCOL_XM_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 队尾命令帧已满，退出
    if (pCmdFrame->length >= UART_PROTOCOL_XM_TX_CMD_FRAME_LENGTH_MAX)
    {
        return;
    }

    // 数据添加到帧末尾，并更新帧长度
    pCmdFrame->buff[pCmdFrame->length] = data;
    pCmdFrame->length++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void UART_PROTOCOL_XM_TxAddFrame(void)
{
    uint16 checkSum = 0;
    uint16 i = 0;
    uint16 head = uartProtocolCB5.tx.head;
    uint16 end = uartProtocolCB5.tx.end;
    UART_PROTOCOL_XM_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB5.tx.cmdQueue[uartProtocolCB5.tx.end];
    uint16 length = pCmdFrame->length;

    // 发送缓冲区已满，不予接收
    if ((end + 1) % UART_PROTOCOL_XM_TX_QUEUE_SIZE == head)
    {
        return;
    }

    // 命令帧长度不足，清除已填充的数据，退出
    if (UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN - 2 > length) // 减去"CRC校验位H,L"2个字节
    {
        pCmdFrame->length = 0;

        return;
    }


    // 队尾命令帧已满，退出
    if ((length + 2 >= UART_PROTOCOL_XM_TX_CMD_FRAME_LENGTH_MAX)) // 留出2个字节给"校验和、结束码"
    {
        return;
    }

    // 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
    // 重设数据长度，需要减4=("1头+1设备号+1命令+1长度")
    pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX] = length - 4;    // 重设数据长度

    checkSum = calCrc16(pCmdFrame->buff, length);

    pCmdFrame->buff[pCmdFrame->length] = (uint8)((checkSum >> 8) & 0xFF);
    pCmdFrame->length ++;

    pCmdFrame->buff[pCmdFrame->length] = (uint8)(checkSum & 0xFF);
    pCmdFrame->length ++;

    uartProtocolCB5.tx.end ++;
    uartProtocolCB5.tx.end %= UART_PROTOCOL_XM_TX_QUEUE_SIZE;
}

// 数据结构初始化
void UART_PROTOCOL_XM_DataStructInit(UART_PROTOCOL_XM_CB *pCB)
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
    for (i = 0; i < UART_PROTOCOL_XM_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    pCB->rxFIFO.head = 0;
    pCB->rxFIFO.end = 0;
    pCB->rxFIFO.currentProcessIndex = 0;

    pCB->rx.head = 0;
    pCB->rx.end = 0;
    for (i = 0; i < UART_PROTOCOL_XM_RX_QUEUE_SIZE; i++)
    {
        pCB->rx.cmdQueue[i].length = 0;
    }
}

// UART报文接收处理函数(注意根据具体模块修改)
void UART_PROTOCOL_XM_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
    uint16 end = uartProtocolCB5.rxFIFO.end;
    uint16 head = uartProtocolCB5.rxFIFO.head;
    uint8 rxdata = 0x00;

    // 接收数据
    rxdata = *pData;

    // 借用KM5S协议解析
   UART_PROTOCOL_MacProcess(standarID, pData, length);
   DUT_PROTOCOL_MacProcess(standarID, pData, length);
//   UART_PROTOCOL4_MacProcess(standarID, pData, length);

    // 一级缓冲区已满，不予接收
    if ((end + 1) % UART_PROTOCOL_XM_RX_FIFO_SIZE == head)
    {
        return;
    }
    // 一级缓冲区未满，接收
    else
    {
        // 将接收到的数据放到临时缓冲区中
        uartProtocolCB5.rxFIFO.buff[end] = rxdata;
        uartProtocolCB5.rxFIFO.end++;
        uartProtocolCB5.rxFIFO.end %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
    }
}

// UART协议层向驱动层注册数据发送接口
void UART_PROTOCOL_XM_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
    uartProtocolCB5.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL UART_PROTOCOL_XM_ConfirmTempCmdFrameBuff(UART_PROTOCOL_XM_CB *pCB)
{
    UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame = NULL;

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
    pCB->rx.end %= UART_PROTOCOL_XM_RX_QUEUE_SIZE;
    pCB->rx.cmdQueue[pCB->rx.end].length = 0; // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

    return TRUE;
}

// 协议层发送处理过程
void UART_PROTOCOL_XM_TxStateProcess(void)
{
    uint16 head = uartProtocolCB5.tx.head;
    uint16 end = uartProtocolCB5.tx.end;
    uint16 length = uartProtocolCB5.tx.cmdQueue[head].length;
    uint8 *pCmd = uartProtocolCB5.tx.cmdQueue[head].buff;
    uint16 localDeviceID = uartProtocolCB5.tx.cmdQueue[head].deviceID;

    // 发送缓冲区为空，说明无数据
    if (head == end)
    {
        return;
    }

    // 发送函数没有注册直接返回
    if (NULL == uartProtocolCB5.sendDataThrowService)
    {
        return;
    }

    // 协议层有数据需要发送到驱动层
    if (!(*uartProtocolCB5.sendDataThrowService)(localDeviceID, pCmd, length))
    {
        return;
    }

    // 发送环形队列更新位置
    uartProtocolCB5.tx.cmdQueue[head].length = 0;
    uartProtocolCB5.tx.head++;
    uartProtocolCB5.tx.head %= UART_PROTOCOL_XM_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void UART_PROTOCOL_XM_RxFIFOProcess(UART_PROTOCOL_XM_CB *pCB)
{
    uint16 end = pCB->rxFIFO.end;
    uint16 head = pCB->rxFIFO.head;
    UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame = NULL;
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
        if (UART_PROTOCOL_XM_CMD_HEAD != currentData)
        {
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			// 取出当前要处理的字节
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			
			// 一级缓冲区为空，退出
			if(pCB->rxFIFO.head == pCB->rxFIFO.end)
			{
				return;
			}
        }

        // 命令头正确，但无临时缓冲区可用，退出
        if ((pCB->rx.end + 1) % UART_PROTOCOL_XM_RX_QUEUE_SIZE == pCB->rx.head)
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
        pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
    }
    // 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
    else
    {
        // 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
        if (pCmdFrame->length >= UART_PROTOCOL_XM_RX_CMD_FRAME_LENGTH_MAX)
        {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
            pCmdFrame->length = 0; // 2016.1.5增加
            // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
            pCB->rxFIFO.head++;
            pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
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
            pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_XM_RX_FIFO_SIZE;

            // ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■

            // 首先判断命令帧最小长度，一个完整的命令字至少包括6个字节: 命令帧最小长度，包含:命令头、设备号、命令字、帧长度、校验码L、校验码H，因此不足6个字节的必定不完整
            if (pCmdFrame->length < UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN)
            {
                // 继续接收
                continue;
            }

            // 命令帧长度数值越界，说明当前命令帧错误，停止接收
            if ((uint16)pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX] + UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN > UART_PROTOCOL_XM_RX_CMD_FRAME_LENGTH_MAX)
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;

                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

            // 命令帧长度校验，在命令长度描述字的数值上，增加命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，即为命令帧实际长度
            length = pCmdFrame->length;
            if (length < pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX] + UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN)
            {
                // 长度要求不一致，说明未接收完毕，退出继续
                continue;
            }

            // 命令帧长度OK，则进行校验，失败时删除命令头
            if (!UART_PROTOCOL_XM_CheckSUM(pCmdFrame))
            {
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
                // 停止RX通讯超时检测
                UART_PROTOCOL_StopRxTimeOutCheck();
#endif

                // 校验失败，将命令帧长度清零，即认为抛弃该命令帧
                pCmdFrame->length = 0;
                // 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
                pCB->rxFIFO.head++;
                pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
                pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

                return;
            }

#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
            // 停止RX通讯超时检测
            UART_PROTOCOL_StopRxTimeOutCheck();
#endif

            // 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
            pCB->rxFIFO.head += length;
            pCB->rxFIFO.head %= UART_PROTOCOL_XM_RX_FIFO_SIZE;
            pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
            UART_PROTOCOL_XM_ConfirmTempCmdFrameBuff(pCB);

            return;
        }
    }
}

// 对传入的命令帧进行校验，返回校验结果
BOOL UART_PROTOCOL_XM_CheckSUM(UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame)
{
    uint16 cc = 0;
    uint16 checkValue = 0;

    if (NULL == pCmdFrame)
    {
        return FALSE;
    }

    // CRC校验
    checkValue = calCrc16(pCmdFrame->buff, pCmdFrame->length - 2);

    cc = ((uint16)pCmdFrame->buff[pCmdFrame->length - 2] << 8) | (uint16)pCmdFrame->buff[pCmdFrame->length - 1];

    if (cc != checkValue)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL checkMark;
uint8_t infoLen = 0;

// UART命令帧缓冲区处理
void UART_PROTOCOL_XM_CmdFrameProcess(UART_PROTOCOL_XM_CB *pCB)
{
    UART_PROTOCOL_XM_CMD cmd = UART_PROTOCOL_XM_CMD_NULL;
    UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame = NULL;

    uint8 switch_state = 0;
    uint8 Peripheral_type = 0;
    uint8 calibration_result = 0;
    uint8 turn_item = 0;
    uint8 turn_state = 0;
    uint8 buffFlag[4] = {0xFF};
    uint8 i =  0;
    uint8_t verType = 0;
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
    if (UART_PROTOCOL_XM_CMD_HEAD != pCmdFrame->buff[UART_PROTOCOL_XM_CMD_HEAD_INDEX])
    {
        // 删除命令帧
        pCB->rx.head++;
        pCB->rx.head %= UART_PROTOCOL_XM_RX_QUEUE_SIZE;
        return;
    }

    // 命令头合法，则提取命令
    cmd = (UART_PROTOCOL_XM_CMD)pCmdFrame->buff[UART_PROTOCOL_XM_CMD_CMD_INDEX];

    // 执行命令帧
    switch (cmd)
    {
        // 空命令，不予执行
        case UART_PROTOCOL_XM_CMD_NULL:
            break;

        // 收到大灯应答
        case UART_PROTOCOL_XM_CMD_TEST_LIGHT:
            switch_state = pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX];   // 执行结果
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
            
        // 获取油门/刹车的模拟数字值
        case UART_PROTOCOL_XM_CMD_TEST_Throttle_Break:                      

            dut_info.throttleAd = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX];
            dut_info.brakeAd = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA2_INDEX];

            // 油门
            if (TEST_TYPE_XM_THROTTLE == dut_info.test)
            {
                // 获取油门AD值
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
            if (TEST_TYPE_XM_BRAKE == dut_info.test)
            {
                // 获取刹把AD值
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

            break;

        // 转向灯控制
        case UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT:
            turn_item = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX];   // 转向灯类型
            light_cnt++;

            // 左转向灯
            if ((dut_info.test == TEST_TYPE_XM_LEFT_TURN_SIGNAL) && (1 == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN);
                }
            }

            // 右转向灯
            if ((dut_info.test == TEST_TYPE_XM_RIGHT_TURN_SIGNAL) && (2 == turn_item))
            {
                if (light_cnt == 1)
                {
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN);
                }
            }

            // 关灯
            if ((dut_info.test == TEST_TYPE_XM_LEFT_TURN_SIGNAL) && (0 == turn_item))
            {
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }
            // 关灯
            if ((dut_info.test == TEST_TYPE_XM_RIGHT_TURN_SIGNAL) && (0 == turn_item))
            {
                if (light_cnt == 2)
                {
                    STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE);
                    light_cnt = 0;
                }
            }
            break;

        // 笔段屏/数码管显示测试
        case UART_PROTOCOL_XM_CMD_TEST_LED:
             STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_DISPLAY_TEST, TRUE);
            break;

        // 测试按键
        case UART_PROTOCOL_XM_CMD_TEST_KEY:
            if(keyValue == pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX])
            {
                 testResult = TRUE;
            }
            else
            {
                 testResult = FALSE;
            }
            STS_PROTOCOL_SendCmdParamTwoAck(STS_PROTOCOL_CMD_SIMULATION_KEY_TEST, testResult, pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX]);
            break;

        // 蜂鸣器测试
        case UART_PROTOCOL_XM_CMD_TEST_BEEP:
             STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_BUZZER_TEST, TRUE);
            break;

        // 在线检测
        case UART_PROTOCOL_XM_CMD_CONTORY_UP:
            online_detection_cnt++;
            TIMER_ResetTimer(TIMER_ID_ONLINE_DETECT);

            if(1 == online_detection_cnt)
            {
               // 上报仪表启动状态
               STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION,TRUE);
            }
            break;

        // 读取标志区数据
        case UART_PROTOCOL_XM_CMD_READ_FLAG_DATA:
            TIMER_KillTask(TIMER_ID_WRITE_FLAG_TIMEOUT);

            // 读取标志区数据
             buffFlag[0] = pCmdFrame->buff[stationNumber + 5];

            // 校验标志区数据
            if (buffFlag[0] == 2)
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, TRUE);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
            }
            break;

        // 写入标志区数据
        case UART_PROTOCOL_XM_CMD_WRITE_FLAG_DATA:

            // 关闭写入超时定时器
            TIMER_ResetTimer(TIMER_ID_WRITE_FLAG_TIMEOUT);

            // 收到写入成功再读取出来进行校验
            if (pCmdFrame->buff[STS_PROTOCOL_CMD_DATA1_INDEX])
            {
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_READ_FLAG_DATA);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_STATION_FLAG, FALSE);
            }
            break;

        // 写入版本类型数据
        case UART_PROTOCOL_XM_CMD_WRITE_VERSION_TYPE:

            // 关闭定时器
            TIMER_ResetTimer(TIMER_ID_OPERATE_VER_TIMEOUT);

            // 获取类型
            verType =  pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX];

            // 类型正确并且执行成功，上报STS
            if ((dutverType == verType) && (pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA2_INDEX]))
            {
                // 读取版本类型信息
                UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_READ_VERSION_TYPE, dutverType);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_CHECK_VERSION_INFO, FALSE);
            }
            break;

        // 读取版本类型信息
        case UART_PROTOCOL_XM_CMD_READ_VERSION_TYPE:
            TIMER_KillTask(TIMER_ID_OPERATE_VER_TIMEOUT);

            // 获取类型
            verType =  pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX];

            // 获取信息长度
            infoLen = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA2_INDEX];

            // 获取类型信息
            strcpy(verBuff, (const char *)&pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA3_INDEX]);                        
            
            verBuff[infoLen] = 0;
            if(3 == verType)
            {
                 verBuff[infoLen - 1] = 0;            
            }

            // 只读取
            if (verreadFlag)
            {
                if (verType == 1)
                {
                    verType = 0;
                }
                else if (verType == 2)
                {
                    verType = 1;
                }
                else if (verType == 3)
                {
                    verType = 2;
                }
                else if (verType == 4)
                {
                    verType = 4;
                }
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                STS_PROTOCOL_TxAddData(0x00);       // 数据长度
                STS_PROTOCOL_TxAddData(1);          // 校验读取结果
                STS_PROTOCOL_TxAddData(verType);    // 版本类型
                
                if(2 == verType)
                {
                    STS_PROTOCOL_TxAddData(infoLen - 1);    // 信息长度

                    // 类型信息
                    for (i = 0; i < (infoLen - 1); i++)
                    {
                        STS_PROTOCOL_TxAddData(verBuff[i]);
                    }                
                }
                else
                {
                    STS_PROTOCOL_TxAddData(infoLen);    // 信息长度

                    // 类型信息
                    for (i = 0; i < (infoLen); i++)
                    {
                        STS_PROTOCOL_TxAddData(verBuff[i]);
                    }                 
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
                    if (verType == 1)
                    {
                        verType = 0;
                    }
                    else if (verType == 2)
                    {
                        verType = 1;
                    }
                    else if (verType == 3)
                    {
                        verType = 2;
                    }
                    else if (verType == 4)
                    {
                        verType = 4;
                    }
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                    STS_PROTOCOL_TxAddData(0x00);       // 数据长度
                    STS_PROTOCOL_TxAddData(1);          // 校验读取结果
                    STS_PROTOCOL_TxAddData(verType);    // 版本类型
                    STS_PROTOCOL_TxAddData(infoLen);    // 信息长度

                    // 类型信息
                    for (i = 0; i < infoLen; i++)
                    {
                        STS_PROTOCOL_TxAddData(verBuff[i]);
                    }
                    STS_PROTOCOL_TxAddFrame();     // 版本类型
                }
                else
                {
                    if (verType == 1)
                    {
                        verType = 0;
                    }
                    else if (verType == 2)
                    {
                        verType = 1;
                    }
                    else if (verType == 3)
                    {
                        verType = 2;
                    }
                    else if (verType == 4)
                    {
                        verType = 4;
                    }
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                    STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_CHECK_VERSION_INFO);
                    STS_PROTOCOL_TxAddData(0x00);       // 数据长度
                    STS_PROTOCOL_TxAddData(0);          // 校验读取结果
                    STS_PROTOCOL_TxAddData(verType);    // 版本类型
                    STS_PROTOCOL_TxAddData(infoLen);    // 信息长度

                    // 类型信息
                    for (i = 0; i < infoLen; i++)
                    {
                        STS_PROTOCOL_TxAddData(verBuff[i]);
                    }
                    STS_PROTOCOL_TxAddFrame();     // 版本类型
                }
                verwriteFlag = FALSE;
            }

            break;

        // 蓝牙测试
        case UART_PROTOCOL_XM_CMD_GET_OOB_PARAM:

            // 版本比对成功将MAC地址发给电源板连接
            if (checkMark)
            {
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_HEAD);
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_DEVICE_ADDR);
                POWER_PROTOCOL_TxAddData(POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT);
                POWER_PROTOCOL_TxAddData(12);
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA17_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA17_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA18_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA18_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA19_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA19_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA20_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA20_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA21_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA21_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA22_INDEX]));
                POWER_PROTOCOL_TxAddData(HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA22_INDEX]));
                POWER_PROTOCOL_TxAddFrame();
            }

            // 版本比对失败上报STS
            if (!checkMark)
            {
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_HEAD);
                STS_PROTOCOL_TxAddData(STS_PROTOCOL_CMD_BLE_TEST);
                STS_PROTOCOL_TxAddData(7 + verLenght);
                STS_PROTOCOL_TxAddData(1);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA17_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA18_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA19_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA20_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA21_INDEX]);
                STS_PROTOCOL_TxAddData(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA22_INDEX]);
                
//                // 上报正确版本
//                for(i = 0 ; i < verLenght ; i++)
//                {
//                    STS_PROTOCOL_TxAddData(conVer[i]);
//                }
                
                STS_PROTOCOL_TxAddFrame();
            }
            break;

        // 获取蓝牙固件版本
        case UART_PROTOCOL_XM_CMD_TEST_BLE_VERSION:

            verLenght = pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX];
            
            // 存储转换后的MAC地址
            for (i = 0; i < pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX]; i++)
            {
                conVer[i * 2] = HexToCharHigh(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX + i]);
                conVer[i * 2 + 1] = HexToCharLow(pCmdFrame->buff[UART_PROTOCOL_XM_CMD_DATA1_INDEX + i]);
            }
            
            // 版本一致就查询MAC地址
            for (i = 0; i < pCmdFrame->buff[UART_PROTOCOL_XM_CMD_LENGTH_INDEX]; i++)
            {
                if (verBle[i] != conVer[i])
                {
                    checkMark = FALSE;  // 发现不匹配，设置标志为假
                    break;              // 跳出循环
                }
                else
                {
                    checkMark = TRUE;   // 所有元素都匹配，设置标志为真
                }                    
            }
                        
            // 查询MAC地址
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);                
            UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_GET_OOB_PARAM);
            UART_PROTOCOL_XM_TxAddData(0x00);         // 数据长度
            UART_PROTOCOL_XM_TxAddFrame(); 
            break;
            
        default:
            break;
    }

    // 删除命令帧
    pCB->rx.head++;
    pCB->rx.head %= UART_PROTOCOL_XM_RX_QUEUE_SIZE;
}

// 发送命令回复
void UART_PROTOCOL_XM_SendCmdAck(uint8 ackCmd)
{
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);

    UART_PROTOCOL_XM_TxAddData(ackCmd);
    UART_PROTOCOL_XM_TxAddData(0x00);         // 数据长度
    UART_PROTOCOL_XM_TxAddFrame();
}

// 发送命令回复，带一个参数
void UART_PROTOCOL_XM_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{

    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);

    UART_PROTOCOL_XM_TxAddData(ackCmd);
    UART_PROTOCOL_XM_TxAddData(0x01);

    UART_PROTOCOL_XM_TxAddData(ackParam);
    UART_PROTOCOL_XM_TxAddFrame();
}

// 发送命令回复，带两个个参数
void UART_PROTOCOL_XM_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam)
{
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
    UART_PROTOCOL_XM_TxAddData(ackCmd);
    UART_PROTOCOL_XM_TxAddData(0x02);

    UART_PROTOCOL_XM_TxAddData(ackParam);
    UART_PROTOCOL_XM_TxAddData(two_ackParam);
    UART_PROTOCOL_XM_TxAddFrame();
}

// 上报在线状态
//void UART_PROTOCOL_XM_SendOnlineStart(uint32 param)
//{
//    STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_ONLINE_DETECTION, SHUTDOWN);
//}

// 写入标志区数据
void UART_PROTOCOL_XM_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam)
{

    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_WRITE_FLAG_DATA);
    UART_PROTOCOL_XM_TxAddData(0x06);

    UART_PROTOCOL_XM_TxAddData(placeParam);
    UART_PROTOCOL_XM_TxAddData(0x00);
    UART_PROTOCOL_XM_TxAddData(0x00);
    UART_PROTOCOL_XM_TxAddData(0x00);
    UART_PROTOCOL_XM_TxAddData(0x02);
    UART_PROTOCOL_XM_TxAddData(shutdownFlagParam);
    UART_PROTOCOL_XM_TxAddFrame();
}

// 发送命令回复
void UART_PROTOCOL_XM_Test(uint32 param)
{
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_HEAD);
    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_DEVICE);

    UART_PROTOCOL_XM_TxAddData(UART_PROTOCOL_XM_CMD_GET_OOB_PARAM);
    UART_PROTOCOL_XM_TxAddData(0x00);         // 数据长度
    UART_PROTOCOL_XM_TxAddFrame();
}

