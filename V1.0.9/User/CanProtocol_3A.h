/********************************************************************************************************
        KM5S与控制器通讯协议
********************************************************************************************************/
#ifndef __CAN_PROTOCOL1_H__
#define __CAN_PROTOCOL1_H__

#include "common.h"

//==========与协议强相关，根据不同协议进行修改======================================
// UART总线通讯故障时间
#define CAN_PROTOCOL1_COMMUNICATION_TIME_OUT 10000UL // 单位:ms

//=====================================================================================================
#define CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE 0 // 通讯单向接收超时检测功能开关:0禁止，1使能

#define CAN_PROTOCOL1_RX_QUEUE_SIZE 5 // 接收命令队列尺寸
#define CAN_PROTOCOL1_TX_QUEUE_SIZE 5 // 发送命令队列尺寸

#define CAN_PROTOCOL1_CMD_HEAD 0x3A        // 命令头
#define CAN_PROTOCOL1_CMD_DEVICE_ADDR 0x1A // 设备地址

#define CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX 150 // 最大发送命令帧长度
#define CAN_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX 150 // 最大接收命令帧长度
#define CAN_PROTOCOL1_RX_FIFO_SIZE 200            // 接收一级缓冲区大小
#define CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN 8      // 命令帧最小长度，包含:命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA

// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define CAN_PROTOCOL1_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / UART_DRIVE_BAUD_RATE * 10 * CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)      // 单位:ms
#define CAN_PROTOCOL1_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / UART_DRIVE_BAUD_RATE * 10 * CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // 单位:ms

// UART发送协议定义
typedef enum
{
    CAN_PROTOCOL1_CMD_HEAD_INDEX = 0,    // 帧头索引
    CAN_PROTOCOL1_CMD_DEVICE_ADDR_INDEX, // 设备地址
    CAN_PROTOCOL1_CMD_CMD_INDEX,         // 命令字索引
    CAN_PROTOCOL1_CMD_LENGTH_INDEX,      // 数据长度

    CAN_PROTOCOL1_CMD_DATA1_INDEX,  // 数据索引
    CAN_PROTOCOL1_CMD_DATA2_INDEX,  // 数据索引
    CAN_PROTOCOL1_CMD_DATA3_INDEX,  // 数据索引
    CAN_PROTOCOL1_CMD_DATA4_INDEX,  // 数据索引
    CAN_PROTOCOL1_CMD_DATA5_INDEX,  // 数据索引
    CAN_PROTOCOL1_CMD_DATA6_INDEX,  // 数据索引
    CAN_PROTOCOL1_CMD_DATA7_INDEX,  // 数据索引
    CAN_PROTOCOL1_CMD_DATA8_INDEX,  // 数据索引
    CAN_PROTOCOL1_CMD_DATA9_INDEX,  // 数据索引
    CAN_PROTOCOL1_CMD_DATA10_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA11_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA12_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA13_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA14_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA15_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA16_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA17_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA18_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA19_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA20_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA21_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA22_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA23_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA24_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA25_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA26_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA27_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA28_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA29_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA30_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA31_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA32_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA33_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA34_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA35_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA36_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA37_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA38_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA39_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA40_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA41_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA42_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA43_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA44_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA45_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA46_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA47_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA48_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA49_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA50_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA51_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA52_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA53_INDEX, // 数据索引
    CAN_PROTOCOL1_CMD_DATA54_INDEX, // 数据索引

    CAN_PROTOCOL1_CMD_INDEX_MAX
} CAN_PROTOCOL1_DATE_FRAME;

// 被控命令
typedef enum
{
    CAN_PROTOCOL1_CMD_NULL = 0, // 空命令

    CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM = 0xC0,        // 写控制器参数
    CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM_RESULT = 0xC1, // 写控制器参数结果应答
    CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM = 0xC2,               // 读取控制器参数
    CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM_REPORT = 0xC3,        // 控制器参数上报
    CAN_PROTOCOL1_CMD_NEW_WRITE_CONTROL_PARAM = 0x50,          // 写入配置参数
	CAN_PROTOCOL1_CMD_TEST_LCD = 0xAA,						   // LCD颜色显示控制

    CAN_PROTOCOL1_CMD_MAX // 总命令数
} CAN_PROTOCOL1_CMD;

// UART_RX命令帧定义
typedef struct
{
    uint16 deviceID;
    uint8 buff[CAN_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
    uint16 length;                                     // 命令帧有效数据个数
} CAN_PROTOCOL1_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
    uint16 deviceID;
    uint8 buff[CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
    uint16 length;                                     // 命令帧有效数据个数
} CAN_PROTOCOL1_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
    // 一级接收缓冲区
    struct
    {
        volatile uint8 buff[CAN_PROTOCOL1_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex; // 当前待处理的字节的位置下标
    } rxFIFO;

    // 接收帧缓冲区数据结构
    struct
    {
        CAN_PROTOCOL1_RX_CMD_FRAME cmdQueue[CAN_PROTOCOL1_RX_QUEUE_SIZE];
        uint16 head; // 队列头索引
        uint16 end;  // 队列尾索引
    } rx;

    // 发送帧缓冲区数据结构
    struct
    {
        CAN_PROTOCOL1_TX_CMD_FRAME cmdQueue[CAN_PROTOCOL1_TX_QUEUE_SIZE];
        uint16 head;  // 队列头索引
        uint16 end;   // 队列尾索引
        uint16 index; // 当前待发送数据在命令帧中的索引号
        BOOL txBusy;  // 发送请求，为TRUE时，从待发送队列中取出一个BYTE放入发送寄存器
    } tx;

    // 发送数据接口
    BOOL (*sendDataThrowService)
    (uint32 id, uint8 *pData, uint8 length);

} CAN_PROTOCOL1_CB;

extern CAN_PROTOCOL1_CB canProtocol1CB;

/******************************************************************************
 * 【外部接口声明】
 ******************************************************************************/
// 协议初始化
void CAN_PROTOCOL1_Init(void);

// 协议层过程处理
void CAN_PROTOCOL_Process_3A(void);

// 向发送命令帧队列中添加数据
void CAN_PROTOCOL1_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void CAN_PROTOCOL1_TxAddFrame(void);

// UART报文接收处理函数(注意根据具体模块修改)
void CAN_PROTOCOL1_MacProcess(uint32 standarID, uint8 *pData, uint16 length);

// 向发送缓冲区中添加一条待发送序列
BOOL CAN_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length);

extern void CAN_HW_Init(uint16 bund);

// 升级成功变换屏幕颜色
void CAN_PROTOCOL1_SendCmdChangeColor(uint32 param);

#endif
