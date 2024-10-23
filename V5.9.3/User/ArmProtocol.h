#ifndef __ARM_PROTOCOL_H__
#define __ARM_PROTOCOL_H__

#include "common.h"

//=====================================================================================================

#define ARM_PROTOCOL_RX_QUEUE_SIZE 5 // 接收命令队列尺寸
#define ARM_PROTOCOL_TX_QUEUE_SIZE 5 // 发送命令队列尺寸

#define ARM_PROTOCOL_CMD_HEAD 0x55 // 命令头

#define ARM_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 150 // 最大发送命令帧长度
#define ARM_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 150 // 最大接收命令帧长度
#define ARM_PROTOCOL_RX_FIFO_SIZE 300			 // 接收一级缓冲区大小
#define ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN 4		 // 命令帧最小长度，包含:命令头、命令字、数据长度、校验和

// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define ARM_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / ARM_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)	   // 单位:ms
#define ARM_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / ARM_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // 单位:ms

// UART发送协议定义
typedef enum
{
	ARM_PROTOCOL_CMD_HEAD_INDEX = 0, // 帧头索引
	ARM_PROTOCOL_CMD_CMD_INDEX,		 // 命令字索引
	ARM_PROTOCOL_CMD_LENGTH_INDEX,	 // 数据长度索引

	ARM_PROTOCOL_CMD_DATA1_INDEX,  // 数据索引
	ARM_PROTOCOL_CMD_DATA2_INDEX,  // 数据索引
	ARM_PROTOCOL_CMD_DATA3_INDEX,  // 数据索引
	ARM_PROTOCOL_CMD_DATA4_INDEX,  // 数据索引
	ARM_PROTOCOL_CMD_DATA5_INDEX,  // 数据索引
	ARM_PROTOCOL_CMD_DATA6_INDEX,  // 数据索引
	ARM_PROTOCOL_CMD_DATA7_INDEX,  // 数据索引
	ARM_PROTOCOL_CMD_DATA8_INDEX,  // 数据索引
	ARM_PROTOCOL_CMD_DATA9_INDEX,  // 数据索引
	ARM_PROTOCOL_CMD_DATA10_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA11_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA12_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA13_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA14_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA15_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA16_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA17_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA18_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA19_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA20_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA21_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA22_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA23_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA24_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA25_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA26_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA27_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA28_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA29_INDEX, // 数据索引
	ARM_PROTOCOL_CMD_DATA30_INDEX, // 数据索引

	ARM_PROTOCOL_CMD_INDEX_MAX
} ARM_PROTOCOL_DATE_FRAME;

// 下行命令
typedef enum
{

	UART_ARM_CMD_GET_TOOL_INFO = 0x10,	   // 获取烧录器工具信息
	UART_ARM_CMD_TOOL_UP_BEGIN = 0x11,	   // 烧录器工具升级开始
	UART_ARM_CMD_TOOL_WRITE_DATA = 0x12,   // 烧录器工具数据写入
	UART_ARM_CMD_TOOL_UP_END = 0x13,	   // 烧录器工具升级结束
	UART_ARM_CMD_TOOL_DUT_UP = 0x14,	   // 升级DUT控制命令
	UART_ARM_CMD_TOOL_DUT_PROCESS = 0x15,  // DUT升级进度上报
	UART_ARM_CMD_TOOL_CLEAR_BUFF = 0x16,   // 清空升级缓冲区
	UART_ARM_CMD_TOOL_SET_DUT_INFO = 0x17, // 烧录器写入DUT机型信息
	UART_ARM_CMD_TOOL_GET_DUT_INFO = 0x18, // 获取烧录器记录的DUT机型信息
	UART_ARM_CMD_SET_TOOL_RANK = 0x19,	   // 写入烧录器工具序号信息
	UART_ARM_CMD_GET_TOOL_RANK = 0x1A,	   // 获取烧录器工具序号信息
	UART_ARM_CMD_PROMPT_THE_USER = 0x1B,   // 用户提示信息

	UART_DUT_CMD_UP_CONFIG = 0x30, // 写入DUT参数（命令字=0x30
	UART_DUT_CMD_UP_UI = 0x31,	   // UI数据写入（下行，命令字=0x31）
	UART_DUT_CMD_UP_BOOT = 0x32,   // BOOT数据写入（下行，命令字=0x32）
	UART_DUT_CMD_UP_APP = 0x33,	   // APP数据写入（命令字=0x33）
	UART_DUT_CMD_UP_END = 0x34,	   // 升级项结束（命令字=0x34）
	UART_DUT_CMD_UP_UI_VER = 0x35, // UI版本写入（命令字=0x35）

	ARM_CMD_MAX,

} ARM_PROTOCOL_CMD;

// UART_RX命令帧定义
typedef struct
{
	uint16 deviceID;
	uint8 buff[ARM_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
	uint16 length;									  // 命令帧有效数据个数
} ARM_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
	uint16 deviceID;
	uint8 buff[ARM_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
	uint16 length;									  // 命令帧有效数据个数
} ARM_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
	// 一级接收缓冲区
	struct
	{
		volatile uint8 buff[ARM_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16 head;
		volatile uint16 end;
		uint16 currentProcessIndex; // 当前待处理的字节的位置下标
	} rxFIFO;

	// 接收帧缓冲区数据结构
	struct
	{
		ARM_PROTOCOL_RX_CMD_FRAME cmdQueue[ARM_PROTOCOL_RX_QUEUE_SIZE];
		uint16 head; // 队列头索引
		uint16 end;	 // 队列尾索引
	} rx;

	// 发送帧缓冲区数据结构
	struct
	{
		ARM_PROTOCOL_TX_CMD_FRAME cmdQueue[ARM_PROTOCOL_TX_QUEUE_SIZE];
		uint16 head;  // 队列头索引
		uint16 end;	  // 队列尾索引
		uint16 index; // 当前待发送数据在命令帧中的索引号
		BOOL txBusy;  // 发送请求，为TRUE时，从待发送队列中取出一个BYTE放入发送寄存器
	} tx;

	// 发送数据接口
	BOOL(*sendDataThrowService)
	(uint16 id, uint8 *pData, uint16 length);

	uint32 ageFlag;
	BOOL isTimeCheck;
} ARM_PROTOCOL_CB;

extern ARM_PROTOCOL_CB armProtocolCB;

/******************************************************************************
 * 【外部接口声明】
 ******************************************************************************/

// 协议初始化
void ARM_PROTOCOL_Init(void);

// 协议层过程处理
void ARM_PROTOCOL_Process(void);

// 向发送命令帧队列中添加数据
void ARM_PROTOCOL_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void ARM_PROTOCOL_TxAddFrame(void);

#endif
