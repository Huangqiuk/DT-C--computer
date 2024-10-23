/********************************************************************************************************
        BigStone_V9.2协议
********************************************************************************************************/
#ifndef     __UART_PROTOCOL_XM_H__
#define     __UART_PROTOCOL_XM_H__

#include "common.h"


#define SMOOTH_BASE_TIME                            100     // 平滑基准时间，单位:ms

//==========与协议强相关，根据不同协议进行修改======================================
// UART总线通讯故障时间
#define UART_PROTOCOL_XM_COMMUNICATION_TIME_OUT     10000UL // 单位:ms

#define UART_PROTOCOL_XM_CMD_SEND_TIME              50     // 命令发送时间，单位:ms

//=====================================================================================================
#define UART_PROTOCOL_XM_RX_TIME_OUT_CHECK_ENABLE       1       // 通讯单向接收超时检测功能开关:0禁止，1使能

#define UART_PROTOCOL_XM_RX_QUEUE_SIZE              5       // 接收命令队列尺寸
#define UART_PROTOCOL_XM_TX_QUEUE_SIZE              5      // 发送命令队列尺寸

#define UART_PROTOCOL_XM_CMD_HEAD                       0x5A    // 命令头
#define UART_PROTOCOL_XM_CMD_DEVICE					0x12	// 设备号
#define UART_PROTOCOL_XM_CMD_BMS_DEVICE				0x13	// 电池设备号

#define UART_PROTOCOL_XM_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX  80      // 该命令帧最大接收长度

#define UART_PROTOCOL_XM_TX_CMD_FRAME_LENGTH_MAX        150     // 最大发送命令帧长度
#define UART_PROTOCOL_XM_RX_CMD_FRAME_LENGTH_MAX        150     // 最大接收命令帧长度

#define UART_PROTOCOL_XM_RX_FIFO_SIZE                   200     // 接收一级缓冲区大小
#define UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN           6       // 命令帧最小长度，包含:起始符、异或调整值、错误信息、异或校验


// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define UART_PROTOCOL_XM_BUS_UNIDIRECTIONAL_TIME_OUT    (uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_XM_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)      // 单位:ms
#define UART_PROTOCOL_XM_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_XM_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)  // 单位:ms

// UART发送协议定义
typedef enum
{

    UART_PROTOCOL_XM_CMD_HEAD_INDEX = 0,                // 帧头索引
    UART_PROTOCOL_XM_CMD_DEVICE_INDEX,					// 设备号索引
    UART_PROTOCOL_XM_CMD_CMD_INDEX,                     // 命令字索引
    UART_PROTOCOL_XM_CMD_LENGTH_INDEX,                  // 数据长度

    UART_PROTOCOL_XM_CMD_DATA1_INDEX,                       // 数据索引
    UART_PROTOCOL_XM_CMD_DATA2_INDEX,                       // 数据索引
    UART_PROTOCOL_XM_CMD_DATA3_INDEX,                       // 数据索引
    UART_PROTOCOL_XM_CMD_DATA4_INDEX,                       // 数据索引
    UART_PROTOCOL_XM_CMD_DATA5_INDEX,                       // 数据索引
    UART_PROTOCOL_XM_CMD_DATA6_INDEX,                       // 数据索引
    UART_PROTOCOL_XM_CMD_DATA7_INDEX,                       // 数据索引
    UART_PROTOCOL_XM_CMD_DATA8_INDEX,                       // 数据索引
    UART_PROTOCOL_XM_CMD_DATA9_INDEX,                       // 数据索引
    UART_PROTOCOL_XM_CMD_DATA10_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA11_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA12_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA13_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA14_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA15_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA16_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA17_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA18_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA19_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA20_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA21_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA22_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA23_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA24_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA25_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA26_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA27_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA28_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA29_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA30_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA31_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA32_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA33_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA34_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA35_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA36_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA37_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA38_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA39_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA40_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA41_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA42_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA43_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA44_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA45_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA46_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA47_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA48_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA49_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA50_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA51_INDEX,                  // 数据索引
    UART_PROTOCOL_XM_CMD_DATA52_INDEX,                  // 数据索引

    UART_PROTOCOL_XM_CMD_INDEX_MAX


} UART_PROTOCOL_XM_DATE_FRAME;

// 被控命令
typedef enum
{
    UART_PROTOCOL_XM_CMD_NULL = 0,                              // 空命令

    // 用来在线检测
	UART_PROTOCOL_XM_CMD_CONTORY_UP = 0x20,						// 仪表上传电量速度数据

	// 特权命令
	UART_PROTOCOL_XM_CMD_READ_FLAG_DATA = 0x81,				    // 读标志区数据
	UART_PROTOCOL_XM_CMD_WRITE_FLAG_DATA = 0x82,				// 写标志区数据
	
	UART_PROTOCOL_XM_CMD_WRITE_VERSION_TYPE = 0x83,			    // 写入版本信息
	UART_PROTOCOL_XM_CMD_READ_VERSION_TYPE = 0x84,				// 读取版本信息

	// BIST 指令
	UART_PROTOCOL_XM_CMD_TEST_LED = 0x85,						// 数码管显示指令

	UART_PROTOCOL_XM_CMD_TEST_LIGHT = 0x86,					    // 开启大灯测试

	UART_PROTOCOL_XM_CMD_TEST_Throttle_Break = 0x87,			// 测试油门刹车

	UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT = 0x88,				// 测试转向灯
	
	UART_PROTOCOL_XM_CMD_TEST_KEY = 0x89,						// 测试按键
	
	UART_PROTOCOL_XM_CMD_TEST_BEEP = 0x9A,						// 蜂鸣器测试
	
	UART_PROTOCOL_XM_CMD_TEST_BLE_VERSION = 0x90,				// 获取蓝牙固件版本

	UART_PROTOCOL_XM_CMD_TEST_CONTROL_VERSION = 0x91,			// 获取控制器固件版本

	UART_PROTOCOL_XM_CMD_CLEAR_TOTAL_DISTANCE = 0x92,			// 总里程清除指令

	UART_PROTOCOL_XM_CMD_TEST_TOTAL_DISTANCE = 0x93,			// 获取骑行总路程

	UART_PROTOCOL_XM_CMD_TEST_RESET_FACTORY = 0x94,			    // 恢复出厂指令
	
	UART_PROTOCOL_XM_CMD_GET_OOB_PARAM = 0x95,					// 获取OOB参数指令
	
	UART_PROTOCOL_XM_CMD_GET_P_DATA_PARAM = 0x96,				// 获取生产日期参数指令
	
	UART_PROTOCOL_XM_CMD_GET_BIKE_SN_PARAM = 0x97,				// 获取车辆序列号参数指令
	
	UART_PROTOCOL_XM_CMD_GET_BMS_SN_PARAM = 0x98,				// 同步电池序信息指令

    UART_PROTOCOL_XM_CMD_MAX                                    // 总命令数
} UART_PROTOCOL_XM_CMD;

// UART_RX命令帧定义
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL_XM_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX];   // 命令帧缓冲区
    uint16  length;                                         // 命令帧有效数据个数
} UART_PROTOCOL_XM_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL_XM_TX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
    uint16  length;                                         // 命令帧有效数据个数
} UART_PROTOCOL_XM_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
    // 一级接收缓冲区
    struct
    {
        volatile uint8  buff[UART_PROTOCOL_XM_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex;                 // 当前待处理的字节的位置下标

        uint16 curCmdFrameLength;                   // 当前命令帧长度
    } rxFIFO;

    // 接收帧缓冲区数据结构
    struct
    {
        UART_PROTOCOL_XM_RX_CMD_FRAME   cmdQueue[UART_PROTOCOL_XM_RX_QUEUE_SIZE];
        uint16          head;                       // 队列头索引
        uint16          end;                        // 队列尾索引
    } rx;

    // 发送帧缓冲区数据结构
    struct
    {
        UART_PROTOCOL_XM_TX_CMD_FRAME   cmdQueue[UART_PROTOCOL_XM_TX_QUEUE_SIZE];
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
} UART_PROTOCOL_XM_CB;

extern UART_PROTOCOL_XM_CB uartProtocolCB5;
extern BOOL write_success_Flag;
/******************************************************************************
* 【外部接口声明】
******************************************************************************/

// 协议初始化
void UART_PROTOCOL_XM_Init(void);

// 向发送命令帧队列中添加数据
void UART_PROTOCOL_XM_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void UART_PROTOCOL_XM_TxAddFrame(void);

// UART协议层过程处理
void UART_PROTOCOL_XM_Process(void);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void UART_PROTOCOL_XM_RxFIFOProcess(UART_PROTOCOL_XM_CB *pCB);

// UART命令帧缓冲区处理
void UART_PROTOCOL_XM_CmdFrameProcess(UART_PROTOCOL_XM_CB *pCB);

// 协议层发送处理过程
void UART_PROTOCOL_XM_TxStateProcess(void);

// 发送命令回复
void UART_PROTOCOL_XM_SendCmdAck(uint8 ackCmd);

// 发送命令回复，带一个参数
void UART_PROTOCOL_XM_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// 发送命令回复，带两个个参数
void UART_PROTOCOL_XM_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam);

// 将来自DUT的数据帧通过UART转发给STS
//void DutRxFrame_ToSts_Transmit(UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame);

// 上报在线状态
void UART_PROTOCOL_XM_SendOnlineStart(uint32 param);

// 写入标志区数据
void UART_PROTOCOL_XM_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam);

void UART_PROTOCOL_XM_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

void UART_PROTOCOL_XM_Test(uint32 param);
////////////////////////////////////////////////////////////////////////////////

extern uint8 light_cnt;
extern uint8 left_light_cnt ;
extern uint8 right_light_cnt ;
extern uint8 cali_cnt;
extern uint32 online_detection_cnt ;
extern uint8 cnt;
extern uint8 brake_cnt;


#endif


