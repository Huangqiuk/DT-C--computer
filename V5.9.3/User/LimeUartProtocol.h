/********************************************************************************************************
		KM5S与控制器通讯协议
********************************************************************************************************/
#ifndef 	__LIME_UART_PROTOCOL_H__
#define 	__LIME_UART_PROTOCOL_H__

#include "common.h"

#define SMOOTH_BASE_TIME							100		// 平滑基准时间，单位:ms

//==========与协议强相关，根据不同协议进行修改======================================
// UART总线通讯故障时间
#define LIME_UART_PROTOCOL_COMMUNICATION_TIME_OUT		10000UL	// 单位:ms

// P档位
#define PROTOCOL_ASSIST_P							ASSIST_P

#define LIME_UART_PROTOCOL_CMD_SEND_TIME					500		// 命令发送周期

//=====================================================================================================
#define LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// 通讯单向接收超时检测功能开关:0禁止，1使能

#define LIME_UART_PROTOCOL_RX_QUEUE_SIZE					4		// 接收命令队列尺寸
#define LIME_UART_PROTOCOL_TX_QUEUE_SIZE					4		// 发送命令队列尺寸

#define LIME_UART_PROTOCOL_CMD_HEAD						'L'		// 命令头
#define LIME_UART_PROTOCOL_CMD_HEAD1						'L'		// 命令头1
#define LIME_UART_PROTOCOL_CMD_HEAD2						'B'		// 命令头2
#define LIME_UART_PROTOCOL_CMD_HEAD3						'D'		// 命令头3
#define LIME_UART_PROTOCOL_CMD_HEAD4						'C'		// 命令头4
#define LIME_UART_PROTOCOL_CMD_HEAD5						'P'		// 命令头5
#define LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION			0x01	// 协议版本
#define LIME_UART_PROTOCOL_CMD_DEVICE_ADDR				0x10	// 设备号
#define LIME_UART_PROTOCOL_CMD_NONHEAD 					0xFF	// 非命令头
#define LIME_UART_PROTOCOL_HEAD_BYTE 					5	// 命令头字节数
#define LIME_UART_PROTOCOL_CHECK_BYTE 					1	// 校验字节数

#define LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		160		// 最大发送命令帧长度
#define LIME_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		160		// 最大接收命令帧长度

#define LIME_UART_PROTOCOL_RX_FIFO_SIZE					200		// 接收一级缓冲区大小
#define LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN			11		// 命令帧最小长度，包含:5个命令头、协议版本、设备号、命令字、2个数据长度、校验码


// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define LIME_UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// 单位:ms
#define LIME_UART_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// 单位:ms

// UART发送协议定义	
typedef enum
{
	LIME_UART_PROTOCOL_CMD_HEAD1_INDEX = 0,					// 帧头1索引
	LIME_UART_PROTOCOL_CMD_HEAD2_INDEX,						// 帧头2索引
	LIME_UART_PROTOCOL_CMD_HEAD3_INDEX,						// 帧头3索引
	LIME_UART_PROTOCOL_CMD_HEAD4_INDEX,						// 帧头4索引
	LIME_UART_PROTOCOL_CMD_HEAD5_INDEX,						// 帧头5索引
	LIME_UART_PROTOCOL_CMD_VERSION_INDEX,							// 协议版本索引
	LIME_UART_PROTOCOL_CMD_DEVICE_ADDR_INDEX,				// 设备地址
	LIME_UART_PROTOCOL_CMD_CMD_INDEX, 						// 命令字索引
	LIME_UART_PROTOCOL_CMD_LENGTH_H_INDEX,					// 数据长度H
	LIME_UART_PROTOCOL_CMD_LENGTH_L_INDEX,					// 数据长度L

	LIME_UART_PROTOCOL_CMD_DATA1_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA2_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA3_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA4_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA5_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA6_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA7_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA8_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA9_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA10_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA11_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA12_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA13_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA14_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA15_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA16_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA17_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA18_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA19_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA20_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA21_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA22_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA23_INDEX,						// 数据索引
	LIME_UART_PROTOCOL_CMD_DATA24_INDEX,						// 数据索引

	LIME_UART_PROTOCOL_CMD_INDEX_MAX
}LIME_UART_PROTOCOL_DATE_FRAME;

// 被控命令
typedef enum{
	LIME_UART_PROTOCOL_CMD_NULL = 0,								// 空命令

	// ECO模式命令
	LIME_UART_PROTOCOL_ECO_CMD_REQUEST = 0x01,					// ECO请求
	LIME_UART_PROTOCOL_ECO_CMD_READY = 0x02,						// ECO模式准备就绪
	LIME_UART_PROTOCOL_ECO_CMD_FACTORY_RST = 0x03,				// 恢复出厂设置
	LIME_UART_PROTOCOL_ECO_CMD_APP_ERASE = 0x04,					// APP数据擦除
	LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE = 0x05, 				// APP数据写入
	LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE_FINISH = 0x06, 			// APP数据写入完成

	// 运行模式命令
	LIME_UART_PROTOCOL_RUN_CMD_RST = 0x10,						// 仪表复位命令
	LIME_UART_PROTOCOL_RUN_CMD_SEG_DISPLAY = 0x11,				// 数码管显示
	LIME_UART_PROTOCOL_RUN_CMD_BRIGHTNESS = 0x12,				// 亮度调节
	LIME_UART_PROTOCOL_RUN_CMD_APP_VERSION = 0x16,				// APP软件版本
	
	LIME_UART_PROTOCOL_RUN_CMD_LIGHT_BELT = 0x1B,				// 灯带控制
	LIME_UART_PROTOCOL_RUN_CMD_READ_SN = 0x1C,					// 读取SN
	LIME_UART_PROTOCOL_RUN_CMD_BOOT_VERSION = 0x1D,				// 读取BOOT版本
	LIME_UART_PROTOCOL_RUN_CMD_HW_VERSION = 0x1E,				// 读取硬件版本
	LIME_UART_PROTOCOL_RUN_CMD_READ_LIME_SN = 0x1F,				// 读取LIMESN
	LIME_UART_PROTOCOL_RUN_CMD_MATCHING = 0x31,					// 配对
	LIME_UART_PROTOCOL_RUN_CMD_WRITE_CARD_ID = 0x32,				// 写卡ID

	// 特权命令
	LIME_UART_PROTOCOL_PRIVILEGE_READ_FLASH_DATA = 0xA4,			// FLASH数据读取
	LIME_UART_PROTOCOL_PRIVILEGE_WRITE_FLASH_DATA = 0xA5,		// FLASH数据写入	
	LIME_UART_PROTOCOL_PRIVILEGE_READ_FLAG_DATA = 0xA6,			// FLAG 数据读取
	LIME_UART_PROTOCOL_PRIVILEGE_WRITE_FLAG_DATA = 0xA7,			// FLAG 数据写入
	LIME_UART_PROTOCOL_PRIVILEGE_WRITE_VERSION_DATA = 0xA8,		// 版本类型写入

	// 数码管显示
	LIME_UART_PROTOCOL_SEG_DISPLAY_SEG_TEST = 0xB0,				// 数码管跑马灯
	LIME_UART_PROTOCOL_SEG_DISPLAY_SEG_CONTROL = 0xB1,			// 数码管控制	
	
	LIME_UART_PROTOCOL_CMD_MAX									// 总命令数
}LIME_UART_PROTOCOL_CMD;


// UART_RX命令帧定义
typedef struct
{
	uint16  deviceID;
	uint8	buff[LIME_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];	// 命令帧缓冲区
	uint16	length; 										// 命令帧有效数据个数
}LIME_UART_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
	uint16  deviceID;
	uint8	buff[LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];	// 命令帧缓冲区
	uint16	length; 										// 命令帧有效数据个数
}LIME_UART_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
	// 一级接收缓冲区
	struct
	{
		volatile uint8	buff[LIME_UART_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;					// 当前待处理的字节的位置下标
	}rxFIFO;
	
	// 接收帧缓冲区数据结构
	struct{
		LIME_UART_PROTOCOL_RX_CMD_FRAME	cmdQueue[LIME_UART_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;						// 队列头索引
		uint16			end;						// 队列尾索引
	}rx;

	// 发送帧缓冲区数据结构
	struct{
		LIME_UART_PROTOCOL_TX_CMD_FRAME	cmdQueue[LIME_UART_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;						// 队列头索引
		uint16	end;						// 队列尾索引
		uint16	index;						// 当前待发送数据在命令帧中的索引号
		BOOL	txBusy;						// 发送请求，为TRUE时，从待发送队列中取出一个BYTE放入发送寄存器
	}tx;

	// 发送数据接口
	BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	BOOL txPeriodRequest;		// 码表周期性发送数据请求
	BOOL txAtOnceRequest;		// 码表马上发送数据请求

	BOOL paramSetOK;

	// 平滑算法数据结构
	struct{
		uint16 realSpeed;
		uint16 proSpeed;
		uint16 difSpeed;
	}speedFilter;
}LIME_UART_PROTOCOL_CB;

extern LIME_UART_PROTOCOL_CB uartProtocolCB4;		

/******************************************************************************
* 【外部接口声明】
******************************************************************************/

// 协议初始化
void LIME_UART_PROTOCOL_Init(void);

// 协议层过程处理
void LIME_UART_PROTOCOL_Process(void);

// 向发送命令帧队列中添加数据
void LIME_UART_PROTOCOL_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void LIME_UART_PROTOCOL_TxAddFrame(void);

//==================================================================================
// 设置下发请求标志，按键设置参数也是调用此接口
void LIME_UART_PROTOCOL_SetTxAtOnceRequest(uint32 param);

void LIME_UART_PROTOCOL_SendCmdLedAck(void);
void UART_LED_SetControlMode(const uint8* buff);

// 发送命令字
void LIME_UART_PROTOCOL_SendCmd(uint32 sndCmd);

// 发送ECO请求
void LIME_UART_PROTOCOL_SendCmdEco(uint32 param);

// 读取FLASH回复
void LIME_UART_PROTOCOL_SendCmdReadFlagCmd(uint32* pBuf);

// 发送卡ID
void LIME_UART_PROTOCOL_SendCardId(uint8 cardStyle, uint8 cardIdLen, uint8 *cardId);

// 发送命令字及回复一个字节
void LIME_UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

void LIME_Protocol_SendOnePacket(uint32 flashAddr, uint32 addr);
#endif


