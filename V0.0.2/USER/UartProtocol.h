/********************************************************************************************************
        KM5S与控制器通讯协议
********************************************************************************************************/
#ifndef     __UART_PROTOCOL_H__
#define     __UART_PROTOCOL_H__

#include "common.h"

#define VOLTAGE_ERROR                 700   // 接收的误差值，单位:mv 
#define CURRENT_ERROR                 700   // 接收的误差值，单位:ma  

// 电压值
#define VOLTAGE_5V 5000
#define VOLTAGE_9V 9000
#define VOLTAGE_12V 12000
#define VOLTAGE_24V 24000
#define VOLTAGE_36V 36000
#define VOLTAGE_48V 48000
#define VOLTAGE_60V 60000
#define VOLTAGE_72V 72000

//==========与协议强相关，根据不同协议进行修改======================================
// UART总线通讯故障时间
#define UART_PROTOCOL_COMMUNICATION_TIME_OUT        10000UL // 单位:ms

// P档位
#define PROTOCOL_ASSIST_P                           ASSIST_P

#define UART_PROTOCOL_CMD_SEND_TIME                 500     // 命令发送周期

//=====================================================================================================
#define UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE      0       // 通讯单向接收超时检测功能开关:0禁止，1使能

#define UART_PROTOCOL_RX_QUEUE_SIZE                 3       // 接收命令队列尺寸
#define UART_PROTOCOL_TX_QUEUE_SIZE                 3       // 发送命令队列尺寸

#define UART_PROTOCOL_CMD_HEAD                      0x3A    // 命令头
#define UART_PROTOCOL_CMD_DEVICE_ADDR               0x1A    // 设备地址

#define UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX       80      // 最大发送命令帧长度
#define UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX       80      // 最大接收命令帧长度
#define UART_PROTOCOL_RX_FIFO_SIZE                  200     // 接收一级缓冲区大小
#define UART_PROTOCOL_CMD_FRAME_LENGTH_MIN          8       // 命令帧最小长度，包含:命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA


// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT   (uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)       // 单位:ms
#define UART_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT    (uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)   // 单位:ms

// UART发送协议定义
typedef enum
{
    UART_PROTOCOL_CMD_HEAD_INDEX = 0,                   // 帧头索引
    UART_PROTOCOL_CMD_DEVICE_ADDR_INDEX,                // 设备地址
    UART_PROTOCOL_CMD_CMD_INDEX,                        // 命令字索引
    UART_PROTOCOL_CMD_LENGTH_INDEX,                     // 数据长度

    UART_PROTOCOL_CMD_DATA1_INDEX,                      // 数据索引
    UART_PROTOCOL_CMD_DATA2_INDEX,                      // 数据索引
    UART_PROTOCOL_CMD_DATA3_INDEX,                      // 数据索引
    UART_PROTOCOL_CMD_DATA4_INDEX,                      // 数据索引
    UART_PROTOCOL_CMD_DATA5_INDEX,                      // 数据索引
    UART_PROTOCOL_CMD_DATA6_INDEX,                      // 数据索引
    UART_PROTOCOL_CMD_DATA7_INDEX,                      // 数据索引
    UART_PROTOCOL_CMD_DATA8_INDEX,                      // 数据索引
    UART_PROTOCOL_CMD_DATA9_INDEX,                      // 数据索引
    UART_PROTOCOL_CMD_DATA10_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA11_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA12_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA13_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA14_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA15_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA16_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA17_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA18_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA19_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA20_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA21_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA22_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA23_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA24_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA25_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA26_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA27_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA28_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA29_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA30_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA31_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA32_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA33_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA34_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA35_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA36_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA37_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA38_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA39_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA40_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA41_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA42_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA43_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA44_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA45_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA46_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA47_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA48_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA49_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA50_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA51_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA52_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA53_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA54_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA55_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA56_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA57_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA58_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA59_INDEX,                     // 数据索引
    UART_PROTOCOL_CMD_DATA60_INDEX,                     // 数据索引

    UART_PROTOCOL_CMD_INDEX_MAX
} UART_PROTOCOL_DATE_FRAME;

// 被控命令
typedef enum
{
	UART_ECO_CMD_NULL = 0,							 
	UART_ECO_CMD_RST = 0x01,						// 重启命令
	UART_ECO_CMD_ECO_APPLY = 0x02, 					// ECO申请
	UART_ECO_CMD_ECO_READY = 0x03, 					// ECO就绪
	UART_ECO_CMD_ECO_JUMP_APP = 0x04, 				// APP镜像版本变更
	UART_ECO_CMD_ECO_APP_ERASE = 0x05, 				// APP数据擦除
	UART_ECO_CMD_ECO_APP_WRITE = 0x06, 				// APP数据写入
	UART_ECO_CMD_ECO_APP_WRITE_FINISH = 0x07, 		// APP数据完成

    UART_ECO_PROTOCOL_CMD_MAX                                   // 总命令数
} UART_PROTOCOL_CMD;

// UART_RX命令帧定义
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];    // 命令帧缓冲区
    uint16  length;                                         // 命令帧有效数据个数
} UART_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];    // 命令帧缓冲区
    uint16  length;                                         // 命令帧有效数据个数
} UART_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
    // 一级接收缓冲区
    struct
    {
        volatile uint8  buff[UART_PROTOCOL_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex;                 // 当前待处理的字节的位置下标
    } rxFIFO;

    // 接收帧缓冲区数据结构
    struct
    {
        UART_PROTOCOL_RX_CMD_FRAME  cmdQueue[UART_PROTOCOL_RX_QUEUE_SIZE];
        uint16          head;                       // 队列头索引
        uint16          end;                        // 队列尾索引
    } rx;

    // 发送帧缓冲区数据结构
    struct
    {
        UART_PROTOCOL_TX_CMD_FRAME  cmdQueue[UART_PROTOCOL_TX_QUEUE_SIZE];
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
    BOOL readMcuUid;

    // 平滑算法数据结构
    struct
    {
        uint16 realSpeed;
        uint16 proSpeed;
        uint16 difSpeed;
    } speedFilter;
} UART_PROTOCOL_CB;

typedef union
{
    uint8_t value[4];
    uint32_t raw_data;
} POWERSUPPLYVOLTAGE;

typedef union
{
    uint8_t value[4];
    uint32_t CURRENT;
} VOUT_CURRENT;

extern UART_PROTOCOL_CB uartProtocolCB;
extern uint8 uumacBuff[20];

/******************************************************************************
* 【外部接口声明】
******************************************************************************/

// 协议初始化
void UART_PROTOCOL_Init(void);

// 协议层过程处理
void UART_PROTOCOL_Process(void);

// 向发送命令帧队列中添加数据
void UART_PROTOCOL_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void UART_PROTOCOL_TxAddFrame(void);

// 发送老化累加次数
void UART_PROTOCOL_SendCmdAging(uint32 param);

void UART_PROTOCOL_SendMcuUid(uint32 param);

// UART报文接收处理函数(注意根据具体模块修改)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

//==================================================================================
// 发送ECO请求
void UART_PROTOCOL_SendEco(uint32 param);

#define UART_PROTOCOL_RX_PARAM_SIZE         (sizeof(UART_PROTOCOL_RX_PARAM_CB))

#endif
