#ifndef __AVO_PROTOCOL_H__
#define __AVO_PROTOCOL_H__

#include "common.h"

////==========与协议强相关，根据不同协议进行修改======================================
//// UART总线通讯故障时间
//#define UART_PROTOCOL_COMMUNICATION_TIME_OUT      10000UL // 单位:ms
//#define UART_PROTOCOL_CMD_SEND_TIME                   500     // 命令发送周期

//=====================================================================================================
#define AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE       0       // 通讯单向接收超时检测功能开关:0禁止，1使能

#define AVO_PROTOCOL_RX_QUEUE_SIZE 3 // 接收命令队列尺寸
#define AVO_PROTOCOL_TX_QUEUE_SIZE 3 // 发送命令队列尺寸

#define AVO_PROTOCOL_CMD_HEAD1                      0x55        // 命令头
#define AVO_PROTOCOL_CMD_HEAD2                      0x00        // 命令头1
#define AVO_PROTOCOL_CMD_HEAD3                      0xAA        // 命令头2

#define AVO_PROTOCOL_CMD_PROTOCOL_VERSION           0x01    // 协议版本
#define AVO_PROTOCOL_CMD_DEVICE_ADDR                0x10    // 设备号
#define AVO_PROTOCOL_CMD_NONHEAD                    0xFF    // 非命令头
#define AVO_PROTOCOL_HEAD_BYTE                      3   // 命令头字节数
#define AVO_PROTOCOL_CHECK_BYTE                     1   // 校验字节

#define AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 160 // 最大发送命令帧长度
#define AVO_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 160 // 最大接收命令帧长度

#define AVO_PROTOCOL_RX_FIFO_SIZE 200            // 接收一级缓冲区大小
#define AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN 6      // 命令帧最小长度，包含:5个命令头、协议版本、设备号、命令字、2个数据长度、校验码

// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define AVO_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / AVO_UART_BAUD_RATE * 10 * AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)      // 单位:ms
#define AVO_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / AVO_UART_BAUD_RATE * 10 * AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // 单位:ms

// UART发送协议定义
typedef enum
{
    AVO_PROTOCOL_CMD_HEAD1_INDEX = 0,                 // 帧头1索引
    AVO_PROTOCOL_CMD_HEAD2_INDEX,                     // 帧头2索引
    AVO_PROTOCOL_CMD_HEAD3_INDEX,                     // 帧头3索引
    AVO_PROTOCOL_CMD_INDEX,                           /* 命令字索引 */
    AVO_PROTOCOL_CMD_LENGTH_INDEX,                    /* 数据长度 */
    AVO_PROTOCOL_CMD_BOARD_INDEX,                     /* 目标板卡号 （板卡编号）*/

    AVO_PROTOCOL_CMD_DATA1_INDEX,                     // 数据索引(测量类型)
    AVO_PROTOCOL_CMD_DATA2_INDEX,                     // 数据索引(测量通道)
    AVO_PROTOCOL_CMD_DATA3_INDEX,                     // 数据索引(测量结果byte1)
    AVO_PROTOCOL_CMD_DATA4_INDEX,                     // 数据索引(测量结果byte2)
    AVO_PROTOCOL_CMD_DATA5_INDEX,                     // 数据索引(测量结果byte3)
    AVO_PROTOCOL_CMD_DATA6_INDEX,                     // 数据索引(测量结果byte4)
    AVO_PROTOCOL_CMD_DATA7_INDEX,                     // 数据索引
    AVO_PROTOCOL_CMD_DATA8_INDEX,                     // 数据索引
    AVO_PROTOCOL_CMD_DATA9_INDEX,                     // 数据索引
    AVO_PROTOCOL_CMD_DATA10_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA11_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA12_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA13_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA14_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA15_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA16_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA17_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA18_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA19_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA20_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA21_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA22_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA23_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_DATA24_INDEX,                    // 数据索引
    AVO_PROTOCOL_CMD_INDEX_MAX

} AVO_PROTOCOL_DATE_FRAME;


/*单板编号*/
typedef enum
{
    BOARD_ARM = 0X00,  /*ARM板编号*/
    BOARD_MOM = 0X01,  /*母版板编号*/
    BOARD_COM,         /*接口板板编号*/
    BOARD_PRO,         /*烧录卡板编号*/
    BOARD_AVO,         /*万用表板编号*/
    BOARD_MAX
} AVO_PROTOCOL_BDNUM;

// 被控命令
typedef enum
{
    AVO_CMD_NULL        = 0xFF,

    //万用表测量命令
    AVO_CMD_AVOMETER = 0xb0,
    AVO_CMD_AVOMETER_ACK = 0xb0,
    // 万用表复位命令
    AVO_CMD_AVORESET = 0xb1,
    AVO_CMD_AVORESET_ACK = 0xb1,

    // 表头档位通信命令
    //AVO_CMD_MEMET_CAPTURE_GET_GEARS = 0x50,           // 查询表头档位命令
    //AVO_CMD_MEMET_CAPTURE_GET_GEARS_ACK = 0x50,       // 查询表头档位命令应答

    AVO_PROTOCOL_CMD_MAX                                 // 总命令数
} AVO_PROTOCOL_CMD;


// 万用表测试档位枚举
typedef enum
{

    AVO_GEARS_DCV = 0x01,
    AVO_GEARS_ACV,
    AVO_GEARS_DCMA,
    AVO_GEARS_ACMA,
    AVO_GEARS_OHM,
    AVO_GEARS_DCA,
    AVO_GEARS_ACA,
    AVO_GEARS_MAX,
} AVO_GEARS_E;

typedef enum
{
    MEASURE_DCV   = 1, //直流电压（V）
    MEASURE_ACV,       //交流电压（V）
    MEASURE_OHM,       //电阻（Ω）
    MEASURE_DCMA,      //直流电流（mA）
    MEASURE_ACMA,      //交流电流（mA）
    MEASURE_DCA,       //直流电流（A）
    MEASURE_ACA,       //交流电流（A）
} AVO_MEASURE_TYPE;

// UART_RX命令帧定义
typedef struct
{
    uint16 deviceID;
    uint8 buff[AVO_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
    uint16 length;                                    // 命令帧有效数据个数
} AVO_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
    uint16 deviceID;
    uint8 buff[AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
    uint16 length;                                    // 命令帧有效数据个数
} AVO_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
    // 一级接收缓冲区
    struct
    {
        volatile uint8 buff[AVO_PROTOCOL_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex; // 当前待处理的字节的位置下标
    } rxFIFO;

    // 接收帧缓冲区数据结构
    struct
    {
        AVO_PROTOCOL_RX_CMD_FRAME cmdQueue[AVO_PROTOCOL_RX_QUEUE_SIZE];
        uint16 head; // 队列头索引
        uint16 end;  // 队列尾索引
    } rx;

    // 发送帧缓冲区数据结构
    struct
    {
        AVO_PROTOCOL_TX_CMD_FRAME cmdQueue[AVO_PROTOCOL_TX_QUEUE_SIZE];
        uint16 head;  // 队列头索引
        uint16 end;   // 队列尾索引
        uint16 index; // 当前待发送数据在命令帧中的索引号
        BOOL txBusy;  // 发送请求，为TRUE时，从待发送队列中取出一个BYTE放入发送寄存器
    } tx;

    // 发送数据接口
    BOOL(*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

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

    uint32 ageFlag;
    BOOL isTimeCheck;
} AVO_PROTOCOL_CB;

extern AVO_PROTOCOL_CB AVOProtocolCB;

/******************************************************************************
 * 【外部接口声明】
 ******************************************************************************/

// 协议初始化
void AVO_PROTOCOL_Init(void);

// 协议层过程处理
void AVO_PROTOCOL_Process(void);

// 向发送命令帧队列中添加数据
void AVO_PROTOCOL_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void AVO_PROTOCOL_TxAddFrame(void);

// 下发测量类型命令, 通道默认为0
void AVO_PROTOCOL_Send_Mesuretype(uint32 mesuretype);

// 下发测量类型和测量通道命令
void AVO_PROTOCOL_Send_Type_Chl(uint32 mesuretype, uint32 mesurechl);

// 下发复位命令
void AVO_PROTOCOL_Send_Reset(void);
#endif

/******************************************************************************
 * 【外部数据获取】
 ******************************************************************************/

typedef union
{
    uint8 data[4];
    float result;
} MEASURE_RESULT;

typedef union
{
   uint8_t data[4];
   int INT;
} OHM;

extern MEASURE_RESULT measure;
extern uint8 headlight_cnt ;
extern uint8 gnd_cnt ;
extern OHM ohm;
