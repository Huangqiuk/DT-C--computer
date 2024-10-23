/********************************************************************************************************
        BigStone_V9.2协议
********************************************************************************************************/
#ifndef     __DUTUART_PROTOCOL_H__
#define     __DUTUART_PROTOCOL_H__

#include "common.h"

#define SMOOTH_BASE_TIME                            100     // 平滑基准时间，单位:ms

//==========与协议强相关，根据不同协议进行修改======================================
// UART总线通讯故障时间
#define DUT_PROTOCOL_COMMUNICATION_TIME_OUT     10000UL // 单位:ms

#define LEFT_TURN_SIGNAL       0
#define RIGHT_TURN_SIGNAL      1
#define LBEAM                  2
#define HBEAM                  3

#define DUT_PROTOCOL_CMD_SEND_TIME              500     // 命令发送时间，单位:ms

//=====================================================================================================
#define DUT_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE       1       // 通讯单向接收超时检测功能开关:0禁止，1使能

#define DUT_PROTOCOL_RX_QUEUE_SIZE              5       // 接收命令队列尺寸
#define DUT_PROTOCOL_TX_QUEUE_SIZE              5       // 发送命令队列尺寸

#define DUT_PROTOCOL_CMD_HEAD                       0x55    // 命令头

#define DUT_PROTOCOL_RX_CMD_HMI_CTRL_RUN_LENGTH_MAX         8       // 该命令帧最大接收长度
#define DUT_PROTOCOL_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX  80      // 该命令帧最大接收长度
#define DUT_PROTOCOL_RX_CMD_READ_CONTROL_PARAM_LENGTH_MAX       4       // 该命令帧最大接收长度

#define DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX        150     // 最大发送命令帧长度
#define DUT_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX        150     // 最大接收命令帧长度

#define DUT_PROTOCOL_RX_FIFO_SIZE                   200     // 接收一级缓冲区大小
#define DUT_PROTOCOL_CMD_FRAME_LENGTH_MIN           4       // 命令帧最小长度，包含:起始符、异或调整值、错误信息、异或校验


// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define DUT_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT    (uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*DUT_PROTOCOL_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)      // 单位:ms
#define DUT_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*DUT_PROTOCOL_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)  // 单位:ms

// UART发送协议定义
typedef enum
{

    DUT_PROTOCOL_CMD_HEAD_INDEX = 0,                // 帧头索引
    DUT_PROTOCOL_CMD_CMD_INDEX,                     // 命令字索引
    DUT_PROTOCOL_CMD_LENGTH_INDEX,                  // 数据长度

    DUT_PROTOCOL_CMD_DATA1_INDEX,                       // 数据索引
    DUT_PROTOCOL_CMD_DATA2_INDEX,                       // 数据索引
    DUT_PROTOCOL_CMD_DATA3_INDEX,                       // 数据索引
    DUT_PROTOCOL_CMD_DATA4_INDEX,                       // 数据索引
    DUT_PROTOCOL_CMD_DATA5_INDEX,                       // 数据索引
    DUT_PROTOCOL_CMD_DATA6_INDEX,                       // 数据索引
    DUT_PROTOCOL_CMD_DATA7_INDEX,                       // 数据索引
    DUT_PROTOCOL_CMD_DATA8_INDEX,                       // 数据索引
    DUT_PROTOCOL_CMD_DATA9_INDEX,                       // 数据索引
    DUT_PROTOCOL_CMD_DATA10_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA11_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA12_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA13_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA14_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA15_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA16_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA17_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA18_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA19_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA20_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA21_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA22_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA23_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA24_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA25_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA26_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA27_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA28_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA29_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA30_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA31_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA32_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA33_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA34_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA35_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA36_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA37_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA38_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA39_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA40_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA41_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA42_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA43_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA44_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA45_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA46_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA47_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA48_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA49_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA50_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA51_INDEX,                  // 数据索引
    DUT_PROTOCOL_CMD_DATA52_INDEX,                  // 数据索引

    DUT_PROTOCOL_CMD_INDEX_MAX


} DUT_PROTOCOL_DATE_FRAME;

// 被控命令
typedef enum
{
    DUT_PROTOCOL_CMD_NULL = 0,                              // 空命令

    DUT_PROTOCOL_CMD_HEADLIGHT_CONTROL = 0xEA,              // 大灯控制
    DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD = 0xEB,          // 获取油门/刹车的模拟数字值
    DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL = 0xEC,            // 转向灯控制
    DUT_PROTOCOL_CMD_ONLINE_DETECTION = 0xED,               // 在线检测
    DUT_PROTOCOL_CMD_GET_PHOTORESISTOR_VALUE = 0x90,        // 获取光敏传感器的数值
    DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION = 0x91,            // 进行电压校准
    DUT_PROTOCOL_CMD_KEY_TEST = 0x05,                       // 按键测试

    DUT_PROTOCOL_CMD_READ_FLAG_DATA = 0xA6,               // 读取标志区数据
    DUT_PROTOCOL_CMD_WRITE_FLAG_DATA = 0xA7,              // 写入标志区数据
    DUT_PROTOCOL_CMD_WRITE_VERSION_TYPE_DATA = 0xA8,      // 写入版本类型数据
    DUT_PROTOCOL_CMD_READ_VERSION_TYPE_INFO = 0xA9,       // 读取版本类型信息

    DUT_PROTOCOL_CMD_BLUETOOTH_MAC_ADDRESS_READ = 0x0C,   // 蓝牙测试
    DUT_PROTOCOL_CMD_TEST_LCD = 0x03,                     // LCD颜色测试（命令字=0x03）
    DUT_PROTOCOL_CMD_FLASH_CHECK_TEST = 0x09,             // Flash 校验测试

    DUT_PROTOCOL_CMD_MAX                                  // 总命令数
} DUT_PROTOCOL_CMD;

// UART_RX命令帧定义
typedef struct
{
    uint16  deviceID;
    uint8   buff[DUT_PROTOCOL_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX];   // 命令帧缓冲区
    uint16  length;                                         // 命令帧有效数据个数
} DUT_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
    uint16  deviceID;
    uint8   buff[DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
    uint16  length;                                         // 命令帧有效数据个数
} DUT_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
    // 一级接收缓冲区
    struct
    {
        volatile uint8  buff[DUT_PROTOCOL_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex;                 // 当前待处理的字节的位置下标

        uint16 curCmdFrameLength;                   // 当前命令帧长度
    } rxFIFO;

    // 接收帧缓冲区数据结构
    struct
    {
        DUT_PROTOCOL_RX_CMD_FRAME   cmdQueue[DUT_PROTOCOL_RX_QUEUE_SIZE];
        uint16          head;                       // 队列头索引
        uint16          end;                        // 队列尾索引
    } rx;

    // 发送帧缓冲区数据结构
    struct
    {
        DUT_PROTOCOL_TX_CMD_FRAME   cmdQueue[DUT_PROTOCOL_TX_QUEUE_SIZE];
        uint16  head;                       // 队列头索引
        uint16  end;                        // 队列尾索引
        uint16  index;                      // 当前待发送数据在命令帧中的索引号
        BOOL    txBusy;                     // 发送请求，为TRUE时，从待发送队列中取出一个BYTE放入发送寄存器
    } tx;

    // 发送数据接口
    BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

    BOOL txPeriodRequest;       // 码表周期性发送数据请求
    BOOL txAtOnceRequest;       // 码表马上发送数据请求

    BOOL paramSetOK;

    // 平滑算法数据结构
    struct
    {
        uint16 realSpeed;
        uint16 proSpeed;
        uint16 difSpeed;
    } speedFilter;
} DUT_PROTOCOL_CB;

extern DUT_PROTOCOL_CB uartProtocolCB3;
extern BOOL write_success_Flag;
/******************************************************************************
* 【外部接口声明】
******************************************************************************/

// 协议初始化
void DUT_PROTOCOL_Init(void);

// 向发送命令帧队列中添加数据
void DUT_PROTOCOL_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void DUT_PROTOCOL_TxAddFrame(void);

// UART协议层过程处理
void DUT_UART_PROTOCOL_Process(void);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void DUT_PROTOCOL_RxFIFOProcess(DUT_PROTOCOL_CB *pCB);

// UART命令帧缓冲区处理
void DUT_PROTOCOL_CmdFrameProcess(DUT_PROTOCOL_CB *pCB);

// 协议层发送处理过程
void DUT_PROTOCOL_TxStateProcess(void);

// 发送命令回复
void DUT_PROTOCOL_SendCmdAck(uint8 ackCmd);

// 发送命令回复，带一个参数
void DUT_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// 发送命令回复，带两个个参数
void DUT_PROTOCOL_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam);

// 将来自DUT的数据帧通过UART转发给STS
void DutRxFrame_ToSts_Transmit(DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// 上报在线状态
void DUT_PROTOCOL_SendOnlineStart(uint32 param);

// 写入标志区数据
void DUT_PROTOCOL_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam);

// UART报文接收处理函数(注意根据具体模块修改)
void DUT_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);
////////////////////////////////////////////////////////////////////////////////

// 油门
typedef union
{
    uint16 data[2];
    uint16 value;
} THROTTLE;

// 刹车
typedef union
{
    uint16 data[2];
    uint16 value;
} BRAKE;

// 电子变速
typedef union
{
    uint16 data[2];
    uint16 value;
} DERAILLEUR;

extern uint8 light_cnt;
extern uint8 left_light_cnt ;
extern uint8 right_light_cnt ;
extern uint8 cali_cnt;
extern uint32 online_detection_cnt ;
extern THROTTLE throttle;
extern BRAKE brake;
#endif


