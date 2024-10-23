/********************************************************************************************************
        BigStone_V9.2协议
********************************************************************************************************/
#ifndef     __UART_PROTOCOL4_H__
#define     __UART_PROTOCOL4_H__

#include "common.h"

#define SMOOTH_BASE_TIME                            100     // 平滑基准时间，单位:ms

//==========与协议强相关，根据不同协议进行修改======================================
// UART总线通讯故障时间
#define UART_PROTOCOL4_COMMUNICATION_TIME_OUT       10000UL // 单位:ms

// P档位
#define PROTOCOL_ASSIST_P                           ASSIST_P

#define UART_PROTOCOL4_CMD_SEND_TIME                500     // 命令发送时间，单位:ms

//=====================================================================================================
#define UART_PROTOCOL4_RX_TIME_OUT_CHECK_ENABLE     1       // 通讯单向接收超时检测功能开关:0禁止，1使能

#define UART_PROTOCOL4_RX_QUEUE_SIZE                3       // 接收命令队列尺寸
#define UART_PROTOCOL4_TX_QUEUE_SIZE                3      // 发送命令队列尺寸

#define UART_PROTOCOL4_CMD_HEAD                     0x3A    // 命令头

#define UART_PROTOCOL4_RX_CMD_HMI_CTRL_RUN_LENGTH_MAX           150      // 该命令帧最大接收长度
#define UART_PROTOCOL4_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX    150     // 该命令帧最大接收长度
#define UART_PROTOCOL4_RX_CMD_READ_CONTROL_PARAM_LENGTH_MAX     150       // 该命令帧最大接收长度

#define UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX      150     // 最大发送命令帧长度
#define UART_PROTOCOL4_RX_CMD_FRAME_LENGTH_MAX      150     // 最大接收命令帧长度

#define UART_PROTOCOL4_RX_FIFO_SIZE                 200     // 接收一级缓冲区大小
#define UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN         8       // 命令帧最小长度，包含:起始符、异或调整值、错误信息、异或校验


// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define UART_PROTOCOL4_BUS_UNIDIRECTIONAL_TIME_OUT  (uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL4_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)        // 单位:ms
#define UART_PROTOCOL4_BUS_BIDIRECTIONAL_TIME_OUT   (uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL4_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)    // 单位:ms

// UART发送协议定义
typedef enum
{

    UART_PROTOCOL4_CMD_HEAD_INDEX = 0,                  // 帧头索引
    UART_PROTOCOL4_CMD_DEVICE_ADDR_INDEX,               // 设备地址
    UART_PROTOCOL4_CMD_CMD_INDEX,                       // 命令字索引
    UART_PROTOCOL4_CMD_LENGTH_INDEX,                    // 数据长度

    UART_PROTOCOL4_CMD_DATA1_INDEX,                     // 数据索引
    UART_PROTOCOL4_CMD_DATA2_INDEX,                     // 数据索引
    UART_PROTOCOL4_CMD_DATA3_INDEX,                     // 数据索引
    UART_PROTOCOL4_CMD_DATA4_INDEX,                     // 数据索引
    UART_PROTOCOL4_CMD_DATA5_INDEX,                     // 数据索引
    UART_PROTOCOL4_CMD_DATA6_INDEX,                     // 数据索引
    UART_PROTOCOL4_CMD_DATA7_INDEX,                     // 数据索引
    UART_PROTOCOL4_CMD_DATA8_INDEX,                     // 数据索引
    UART_PROTOCOL4_CMD_DATA9_INDEX,                     // 数据索引
    UART_PROTOCOL4_CMD_DATA10_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA11_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA12_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA13_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA14_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA15_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA16_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA17_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA18_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA19_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA20_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA21_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA22_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA23_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA24_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA25_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA26_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA27_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA28_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA29_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA30_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA31_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA32_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA33_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA34_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA35_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA36_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA37_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA38_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA39_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA40_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA41_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA42_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA43_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA44_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA45_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA46_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA47_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA48_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA49_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA50_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA51_INDEX,                    // 数据索引
    UART_PROTOCOL4_CMD_DATA52_INDEX,                    // 数据索引

    UART_PROTOCOL4_CMD_INDEX_MAX


} UART_PROTOCOL4_DATE_FRAME;

// 被控命令
typedef enum
{
    UART_PROTOCOL4_CMD_NULL = 0,                                // 空命令

    UART_PROTOCOL4_CMD_CONFIG_SET = 0x53,                       // 参数设置
    UART_PROTOCOL4_CMD_WRITE_CONTROL_PARAM = 0xC0,              // 写控制器参数
    UART_PROTOCOL4_CMD_WRITE_CONFIG_RESULT = 0xC1,              // 写控制器参数结果应答
    UART_PROTOCOL4_CMD_READ_CONTROL_PARAM = 0xC2,               // 读取控制器参数
    UART_PROTOCOL4_CMD_READ_CONTROL_PARAM_REPORT = 0xC3,        // 控制器参数上报
    UART_PROTOCOL4_CMD_PROTOCOL_SWITCCH = 0xAB,                 // 协议切换命令
    UART_PROTOCOL4_CMD_NEW_WRITE_CONTROL_PARAM = 0x50,          // 写入配置参数

    UART_PROTOCOL4_CMD_MAX                                      // 总命令数
} UART_PROTOCOL4_CMD;

// UART_RX命令帧定义
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL4_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX]; // 命令帧缓冲区
    uint16  length;                                         // 命令帧有效数据个数
} UART_PROTOCOL4_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX];   // 命令帧缓冲区
    uint16  length;                                         // 命令帧有效数据个数
} UART_PROTOCOL4_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
    // 一级接收缓冲区
    struct
    {
        volatile uint8  buff[UART_PROTOCOL4_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex;                 // 当前待处理的字节的位置下标

        uint16 curCmdFrameLength;                   // 当前命令帧长度
    } rxFIFO;

    // 接收帧缓冲区数据结构
    struct
    {
        UART_PROTOCOL4_RX_CMD_FRAME cmdQueue[UART_PROTOCOL4_RX_QUEUE_SIZE];
        uint16          head;                       // 队列头索引
        uint16          end;                        // 队列尾索引
    } rx;

    // 发送帧缓冲区数据结构
    struct
    {
        UART_PROTOCOL4_TX_CMD_FRAME cmdQueue[UART_PROTOCOL4_TX_QUEUE_SIZE];
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
} UART_PROTOCOL4_CB;

extern UART_PROTOCOL4_CB uartProtocolCB4;

/******************************************************************************
* 【外部接口声明】
******************************************************************************/

// 协议初始化
void UART_PROTOCOL4_Init(void);

// 向发送命令帧队列中添加数据
void UART_PROTOCOL4_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void UART_PROTOCOL4_TxAddFrame(void);

// 协议层过程处理
void UART_PROTOCOL_Process(void);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void UART_PROTOCOL4_RxFIFOProcess(UART_PROTOCOL4_CB *pCB);

// UART命令帧缓冲区处理
void UART_PROTOCOL4_CmdFrameProcess(UART_PROTOCOL4_CB *pCB);

// 协议层发送处理过程
void UART_PROTOCOL4_TxStateProcess(void);

//==================================================================================
// 设置下发请求标志，按键设置参数也是调用此接口
void UART_PROTOCOL4_SetTxAtOnceRequest(uint32 param);
/*
// 错误代码类型定义
typedef enum
{
    ERROR_TYPE_NO_ERROR = 0,                    // 无错误
    ERROR_TYPE_CURRENT_ERROR = 0x21,            // 电流异常
    ERROR_TYPE_THROTTLE_ERROR = 0x22,           // 转把故障
    ERROR_TYPE_MOTOR_PHASE_ERROR = 0x23,        // 电机相线故障
    ERROR_TYPE_HALL_SENSOR_ERROR = 0x24,        // 电机霍尔信号故障
    ERROR_TYPE_BRAKE_ERROR = 0x25,              // 刹车故障
    ERROR_TYPE_LOW_VOLTAGE_ERROR = 0x26,        // 欠压故障
    ERROR_TYPE_COMMUNICATION_TIME_OUT = 0x30,   // 通讯超时

}ERROR_TYPE_E;
*/
// ■■ 码表下发至驱动器的系统参数 ■■
typedef struct
{
    uint8 assitLevel : 4;                   // 档位值，0-9
    uint8 isPushModeOn : 1;                 // 6km/h推行功能，0:表示关闭；1:表示打开
    uint8 reserveByte1Bit65 : 2;
    uint8 isLightOn : 1;                    // 灯光控制

    uint8 wheelSizeCode : 3;                // 轮径代码
    uint8 speedLimitCode : 5;               // 限速值代码

} UART_PROTOCOL4_SET_PARAM_CB;
#define UART_PROTOCOL4_SET_PARAM_SIZE   (sizeof(UART_PROTOCOL4_SET_PARAM_CB))

// ■■ 码表下发至驱动器的实时参数 ■■
typedef struct
{
    uint8 assitLevel : 4;                   // 档位值，0-9
    uint8 isPushModeOn : 1;                 // 6km/h推行功能，0:表示关闭；1:表示打开
    uint8 reserveByte1Bit65 : 2;
    uint8 isLightOn : 1;                    // 灯光控制

    uint8 wheelSizeCode : 3;                // 轮径代码
    uint8 speedLimitCode : 5;               // 限速值代码
} UART_PROTOCOL4_RUNNING_PARAM_CB;
#define UART_PROTOCOL4_RUNNING_PARAM_SIZE   (sizeof(UART_PROTOCOL4_RUNNING_PARAM_CB))

// ■■ 驱动器上报至码表的参数 ■■
typedef struct
{
    uint8 batteryVoltageLevel;              // 电压电量

    uint8 batteryCurrent;                   // 当前电池电流，单位:1/3A

    uint8 oneCycleTimeH;                    // 当前一圈时间，高字节
    uint8 oneCycleTimeL;                    // 当前一圈时间，低字节

    uint8 xorAdjVal;                        // 异或调整值

    uint8 ucErrorCode;                      // 错误代码

} UART_PROTOCOL4_RX_PARAM_CB;
#define UART_PROTOCOL4_RX_PARAM_SIZE            (sizeof(UART_PROTOCOL4_RX_PARAM_CB))


// 码表与控制器数据交互结构体定义
typedef struct
{
    // 设置驱动器参数
    union
    {
        uint8 buff[UART_PROTOCOL4_SET_PARAM_SIZE];
        UART_PROTOCOL4_SET_PARAM_CB param;
    } set;

    // 驱动器运行参数
    union
    {
        uint8 buff[UART_PROTOCOL4_RUNNING_PARAM_SIZE];
        UART_PROTOCOL4_RUNNING_PARAM_CB param;
    } running;

    // 驱动器上报至码表的参数
    union
    {
        uint8 buff[UART_PROTOCOL4_RX_PARAM_SIZE];
        UART_PROTOCOL4_RX_PARAM_CB param;
    } rx;
} UART_PROTOCOL4_PARAM_CB;

extern UART_PROTOCOL4_PARAM_CB hmiDriveCB3;


#endif


