#ifndef __POWER_PROTOCOL_H__
#define __POWER_PROTOCOL_H__

#include "common.h"

//=====================================================================================================

#define POWER_PROTOCOL_RX_QUEUE_SIZE 3 // 接收命令队列尺寸
#define POWER_PROTOCOL_TX_QUEUE_SIZE 3 // 发送命令队列尺寸

#define POWER_PROTOCOL_CMD_HEAD 0x3A // 命令头
#define POWER_PROTOCOL_CMD_DEVICE_ADDR 0x1A // 设备地址

#define POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 150 // 最大发送命令帧长度
#define POWER_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 150 // 最大接收命令帧长度
#define POWER_PROTOCOL_RX_FIFO_SIZE 200			 // 接收一级缓冲区大小
#define POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN 8		 // 命令帧最小长度，包含:命令头、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA

// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define POWER_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / POWER_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)	   // 单位:ms
#define POWER_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / POWER_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // 单位:ms
// UART发送协议定义
typedef enum
{
	POWER_PROTOCOL_CMD_HEAD_INDEX = 0, // 帧头索引
	POWER_PROTOCOL_CMD_DEVICE_ADDR_INDEX,  // 设备地址索引
	POWER_PROTOCOL_CMD_CMD_INDEX,		 // 命令字索引
	POWER_PROTOCOL_CMD_LENGTH_INDEX,	 // 数据长度索引

	POWER_PROTOCOL_CMD_DATA1_INDEX,  // 数据索引
	POWER_PROTOCOL_CMD_DATA2_INDEX,  // 数据索引
	POWER_PROTOCOL_CMD_DATA3_INDEX,  // 数据索引
	POWER_PROTOCOL_CMD_DATA4_INDEX,  // 数据索引
	POWER_PROTOCOL_CMD_DATA5_INDEX,  // 数据索引
	POWER_PROTOCOL_CMD_DATA6_INDEX,  // 数据索引
	POWER_PROTOCOL_CMD_DATA7_INDEX,  // 数据索引
	POWER_PROTOCOL_CMD_DATA8_INDEX,  // 数据索引
	POWER_PROTOCOL_CMD_DATA9_INDEX,  // 数据索引
	POWER_PROTOCOL_CMD_DATA10_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA11_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA12_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA13_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA14_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA15_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA16_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA17_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA18_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA19_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA20_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA21_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA22_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA23_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA24_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA25_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA26_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA27_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA28_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA29_INDEX, // 数据索引
	POWER_PROTOCOL_CMD_DATA30_INDEX, // 数据索引

	POWER_PROTOCOL_CMD_INDEX_MAX
} POWER_PROTOCOL_DATE_FRAME;

// 下行命令
typedef enum
{

	POWER_PROTOCOL_CMD_NULL = 0,						      // 空命令
	POWER_PROTOCOL_CMD_CHECK_EXTERNAL_POWER = 0x01,           // 检测是否有外接电源接入
	POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_VOLTAGE = 0x02,       // 仪表供电电压获取
	POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_CURRENT = 0x03,       // 仪表供电电流获取
	POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE = 0x04,// 调整DUT供电电压
	POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH = 0x05,        // DUT电源控制
	POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT = 0x06,              // 蓝牙 MAC 地址读取
	
    // IAP指令
    POWER_PROTOCOL_CMD_GET_SOFTWARE_INFO = 0x0A,             // 获取POWER软件信息   
	POWER_PROTOCOL_CMD_ECO_APP2_ERASE = 0x0B, 				 // APP2数据擦除
	POWER_PROTOCOL_CMD_ECO_APP2_WRITE = 0x0C, 				 // APP2数据写入
	POWER_PROTOCOL_CMD_ECO_APP_WRITE_FINISH = 0x0D, 		 // APP2数据完成    
	POWER_PROTOCOL_CMD_RST = 0x0E,						     // 重启命令
    POWER_PROTOCOL_CMD_UP_FINISH = 0x0F,					 // 升级成功上报

	POWER_PROTOCOL_CMD_MAX									  // 总命令数

} POWER_PROTOCOL_CMD;

// UART_RX命令帧定义
typedef struct
{
	uint16 deviceID;
	uint8 buff[POWER_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
	uint16 length;									  // 命令帧有效数据个数
} POWER_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
	uint16 deviceID;
	uint8 buff[POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
	uint16 length;									  // 命令帧有效数据个数
} POWER_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
	// 一级接收缓冲区
	struct
	{
		volatile uint8 buff[POWER_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16 head;
		volatile uint16 end;
		uint16 currentProcessIndex; // 当前待处理的字节的位置下标
	} rxFIFO;

	// 接收帧缓冲区数据结构
	struct
	{
		POWER_PROTOCOL_RX_CMD_FRAME cmdQueue[POWER_PROTOCOL_RX_QUEUE_SIZE];
		uint16 head; // 队列头索引
		uint16 end;	 // 队列尾索引
	} rx;

	// 发送帧缓冲区数据结构
	struct
	{
		POWER_PROTOCOL_TX_CMD_FRAME cmdQueue[POWER_PROTOCOL_TX_QUEUE_SIZE];
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
} POWER_PROTOCOL_CB;

extern POWER_PROTOCOL_CB POWERProtocolCB;

/******************************************************************************
 * 【外部接口声明】
 ******************************************************************************/

// 协议初始化
void POWER_PROTOCOL_Init(void);

// 协议层过程处理
void POWER_PROTOCOL_Process(void);

// 向发送命令帧队列中添加数据
void POWER_PROTOCOL_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void POWER_PROTOCOL_TxAddFrame(void);

// 发送命令回复
void POWER_PROTOCOL_SendCmdAck(uint8 ackCmd);

// 发送带应答的回复
void POWER_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

/******************************************************************************
 * 【外部数据获取】
 ******************************************************************************/
extern uint8 switch_cnt;

typedef union
{
    uint8_t value[4];
    uint32_t CURRENT;
} VOUT_CURRENT;

#endif
