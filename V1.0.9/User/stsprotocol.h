#ifndef __STS_PROTOCOL_H__
#define __STS_PROTOCOL_H__

#include "common.h"

//=====================================================================================================
#define ERROR_VOL                 1000  // 单位：mv

#define STARTUP                   0x01

#define POWER_ON_SUCCESS          1
#define THROTTLE_GND_TEST_FAILURE 1
#define BRAKING_GND_TEST_FAILURE  2
#define VLK_TEST_FAILURE          3
#define GND_TEST_TIMEOUT          4
#define SHUTDOWN                  0

#define STS_PROTOCOL_RX_QUEUE_SIZE 3 // 接收命令队列尺寸
#define STS_PROTOCOL_TX_QUEUE_SIZE 3 // 发送命令队列尺寸

#define STS_PROTOCOL_CMD_HEAD 0x55// 命令头

#define STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 150 // 最大发送命令帧长度
#define STS_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 150 // 最大接收命令帧长度
#define STS_PROTOCOL_RX_FIFO_SIZE 200            // 接收一级缓冲区大小
#define STS_PROTOCOL_CMD_FRAME_LENGTH_MIN 4      // 命令帧最小长度，包含:命令头、命令字、数据长度

// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define STS_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / STS_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)      // 单位:ms
#define STS_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / STS_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // 单位:ms

// UART发送协议定义
typedef enum
{
    STS_PROTOCOL_CMD_HEAD_INDEX = 0, // 帧头索引
    STS_PROTOCOL_CMD_CMD_INDEX,      // 命令字索引
    STS_PROTOCOL_CMD_LENGTH_INDEX,   // 数据长度索引

    STS_PROTOCOL_CMD_DATA1_INDEX,  // 数据索引
    STS_PROTOCOL_CMD_DATA2_INDEX,  // 数据索引
    STS_PROTOCOL_CMD_DATA3_INDEX,  // 数据索引
    STS_PROTOCOL_CMD_DATA4_INDEX,  // 数据索引
    STS_PROTOCOL_CMD_DATA5_INDEX,  // 数据索引
    STS_PROTOCOL_CMD_DATA6_INDEX,  // 数据索引
    STS_PROTOCOL_CMD_DATA7_INDEX,  // 数据索引
    STS_PROTOCOL_CMD_DATA8_INDEX,  // 数据索引
    STS_PROTOCOL_CMD_DATA9_INDEX,  // 数据索引
    STS_PROTOCOL_CMD_DATA10_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA11_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA12_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA13_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA14_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA15_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA16_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA17_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA18_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA19_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA20_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA21_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA22_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA23_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA24_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA25_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA26_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA27_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA28_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA29_INDEX, // 数据索引
    STS_PROTOCOL_CMD_DATA30_INDEX, // 数据索引

    STS_PROTOCOL_CMD_INDEX_MAX
} STS_PROTOCOL_DATE_FRAME;

// 下行命令
typedef enum
{

    STS_PROTOCOL_CMD_NULL = 0,

    STS_PROTOCOL_CMD_REPORT_OF_IDENTITY = 0x34,            // DTA身份上报（命令字=0x34）
    STS_PROTOCOL_CMD_CHECK_STATION_FLAG = 0x01,            // 过站标志校验
    STS_PROTOCOL_CMD_CHECK_VERSION_INFO = 0x02,            // 版本信息校验命令
    STS_PROTOCOL_CMD_GET_SOFTWARE_INFO = 0x30,             // 获取DTA软件信息
    STS_PROTOCOL_CMD_CLEAR_APP = 0x31,                     // 擦除APP命令
    STS_PROTOCOL_CMD_TOOL_UP_APP = 0x32,                   // APP数据写入
    STS_PROTOCOL_CMD_TOOL_UP_END = 0x33,                   // APP升级结束
    STS_PROTOCOL_CMD_TOOL_SET_DUT_COMM_PARAM = 0x45,       // 设置DUT通讯参数
    STS_PROTOCOL_CMD_MULTIPLE_TEST = 0x0B,                 // 多项测试

    STS_PROTOCOL_CMD_GND_TEST = 0x04,                      // 上电测试
    STS_PROTOCOL_CMD_KEY_TEST = 0x05,                      // 按键测试
    STS_PROTOCOL_CMD_USB_CHARGE_TEST = 0x14,               // USB充电测试（负载设置）
    STS_PROTOCOL_CMD_LIGHT_SENSING_TEST = 0x0E,            // 光敏测试
    STS_PROTOCOL_CMD_DISPLAY_TEST = 0x06,                  // 笔段屏/数码管显示测试
    STS_PROTOCOL_CMD_BUZZER_TEST = 0x07,                   // 蜂鸣器控制测试（命令字=0x07）
    
    STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION = 0x15,           // 电压校准（多级校准）
    STS_PROTOCOL_CMD_ONLINE_DETECTION = 0x18,              // 在线检测（启动及上电状态）
    STS_PROTOCOL_CMD_UART_TEST = 0x19,                     // UART测试
    STS_PROTOCOL_CMD_BLE_TEST = 0x0C,                      // 蓝牙测试
    STS_PROTOCOL_CMD_ADJUST_DUT_VOLTAGE = 0x16,            // 调整DUT供电电压
    STS_PROTOCOL_CMD_WRITE_FLAG_DATA = 0xA7,               // 写标志区数据
    STS_PROTOCOL_CMD_EXECUTIVE_DIRECTOR = 0x1C,            // 启动脚本
    STS_PROTOCOL_CMD_FLASH_CHECK_TEST = 0x09,              // Flash 校验测试
    STS_PROTOCOL_CMD_CONTROL_DUT_POWER = 0x1A,             // DUT电源控制
    STS_PROTOCOL_CMD_OBTAIN_WAKE_UP_VOL = 0x1D,            // 获取WAKE电压  
    STS_PROTOCOL_CMD_SIMULATION_KEY_TEST = 0x1E,            // 模拟按键测试    
    STS_PROTOCOL_CMD_OBTAIN_DUT_CURRENT = 0x1F,            // 获取DUT供电电流
    
    STS_PROTOCOL_CMD_TOOL_DUT_UP = 0x40,                   // 升级DUT控制命令
    STS_PROTOCOL_CMD_TOOL_DUT_PROCESS = 0x41,              // DUT升级进度上报
    STS_PROTOCOL_CMD_TOOL_CLEAR_BUFF = 0x42,               // 清空升级缓冲区
    STS_PROTOCOL_CMD_TOOL_SET_DUT_INFO = 0x43,             // DTA写入DUT机型信息
    STS_PROTOCOL_CMD_TOOL_GET_DUT_INFO = 0x44,             // 获取DTA记录的DUT机型信息
    STS_PROTOCOL_CMD_UP_CONFIG = 0x50,                     // 写入DUT配置参数
    STS_PROTOCOL_CMD_UP_BOOT = 0x51,                       // BOOT数据写入
    STS_PROTOCOL_CMD_UP_APP = 0x52,                        // APP数据写入
    STS_PROTOCOL_CMD_UP_END = 0x53,                        // 升级项结束
    STS_PROTOCOL_CMD_SET_NOT_WRITTEN = 0x46,               // 设置不写入项
    STS_PROTOCOL_CMD_POWER_GET_SOFTWARE_INFO = 0x35,       // 获取POWER软件信息
    STS_PROTOCOL_CMD_POWER_UP_APP_ERASE= 0x36,             // POWER-APP数据擦除
    STS_PROTOCOL_CMD_POWER_UP_APP_WRITE = 0x37,            // POWER-APP数据写入
    STS_PROTOCOL_CMD_POWER_UP_APP_WRITE_FINISH = 0x38,     // POWER-APP数据写入完成
    STS_PROTOCOL_CMD_POWER_UP_FINISH = 0x39,               // 升级POWER-APP成功应答

    STS_PROTOCOL_CMD_TOOL_SET_TRAN_DUT_COMM_PARAM = 0x47,       // 设置透传DUT通讯参数
    STS_PROTOCOL_CMD_TRAN_DUT_INSTRUCTION_CONTROL = 0x48,       // 透传DUT指令控制   
    
    STS_CMD_MAX
} STS_PROTOCOL_CMD;

// UART_RX命令帧定义
typedef struct
{
    uint16 deviceID;
    uint8 buff[STS_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
    uint16 length;                                    // 命令帧有效数据个数
} STS_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
    uint16 deviceID;
    uint8 buff[STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
    uint16 length;                                    // 命令帧有效数据个数
} STS_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
    // 一级接收缓冲区
    struct
    {
        volatile uint8 buff[STS_PROTOCOL_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex; // 当前待处理的字节的位置下标
    } rxFIFO;

    // 接收帧缓冲区数据结构
    struct
    {
        STS_PROTOCOL_RX_CMD_FRAME cmdQueue[STS_PROTOCOL_RX_QUEUE_SIZE];
        uint16 head; // 队列头索引
        uint16 end;  // 队列尾索引
    } rx;

    // 发送帧缓冲区数据结构
    struct
    {
        STS_PROTOCOL_TX_CMD_FRAME cmdQueue[STS_PROTOCOL_TX_QUEUE_SIZE];
        uint16 head;  // 队列头索引
        uint16 end;   // 队列尾索引
        uint16 index; // 当前待发送数据在命令帧中的索引号
        BOOL txBusy;  // 发送请求，为TRUE时，从待发送队列中取出一个BYTE放入发送寄存器
    } tx;

    // 发送数据接口
    BOOL(*sendDataThrowService)
    (uint16 id, uint8 *pData, uint16 length);

    uint32 ageFlag;
    BOOL isTimeCheck;
} STS_PROTOCOL_CB;

// 电压校准值
typedef struct
{
    uint8 number;
    uint8 data[3];
} CALIBRATION;

extern STS_PROTOCOL_CB STSProtocolCB;

/******************************************************************************
 * 【外部接口声明】
 ******************************************************************************/

// 协议初始化
void STS_PROTOCOL_Init(void);

// 协议层过程处理
void STS_PROTOCOL_Process(void);

// 向发送命令帧队列中添加数据
void STS_PROTOCOL_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void STS_PROTOCOL_TxAddFrame(void);

// 将来自STS的数据帧通过UART转发给DUT EG：StsRxFrame_ToDut_Uart_Transmit（pCmdFrame）
void StsRxFrame_ToDut_Uart_Transmit(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

void StsRxFrame_ToDut_Can_Transmit(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// 发送命令回复，带一个参数
void STS_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// 发送命令给DUT，带一个参数
void DUT_Serial_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

void StsRxFrame_ToDut_Transmit(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// DUT通讯超时处理
void DutTimeOut(uint32 param);

// 写入标志超时处理
void WriteFlagTimeOut(uint32 param);

void OperateVerTimeOut(uint32 param);

void enterState(void);

// 身份上报
void Report_Identity(uint32 param);

// 发送命令回复
void STS_PROTOCOL_SendCmdAck(uint8 ackCmd);

// 发送命令回复，带两个参数
void STS_PROTOCOL_SendCmdParamTwoAck(uint8 ackCmd, uint8 ackParam, uint8 twoParam);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
extern CALIBRATION calibration;
extern BOOL STS_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length);
extern uint8 stationNumber;
extern char dutverBuff[100];
extern uint8_t dutverType;
extern BOOL verwriteFlag;
extern BOOL verreadFlag;
extern uint8 resultArray[150];  // 用于存储结果的一维数组
extern uint8 verifiedBuff[150];
extern uint8 verifiedIndex;
extern uint8 configs[300];
extern uint8 noLenght ;
extern uint8 noNumber ;
extern BOOL testFlag ;
extern uint8 verBle[20];
extern uint8 keyValue;

#endif
