#ifndef __BLE_HOST_PROTOCOL_H__
#define __BLE_HOST_PROTOCOL_H__

#include "common.h"

//=====================================================================================================
#define BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// 通讯单向接收超时检测功能开关:0禁止，1使能
#define BLE_HOST_PROTOCOL_TXRX_TIME_OUT_CHECK_ENABLE	0		// 通讯双向接收超时检测功能开关:0禁止，1使能

#define BLE_HOST_PROTOCOL_RX_QUEUE_SIZE					8		// 接收命令队列尺寸
#define BLE_HOST_PROTOCOL_TX_QUEUE_SIZE					8		// 发送命令队列尺寸

#define BLE_HOST_PROTOCOL_CMD_HEAD						0x55	// 命令头

#define BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		150		// 最大发送命令帧长度
#define BLE_HOST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		150		// 最大接收命令帧长度
#define BLE_HOST_PROTOCOL_RX_FIFO_SIZE					300		// 接收一级缓冲区大小
#define BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN			4		// 命令帧最小长度，包含:命令头、命令字、数据长度、校验和

// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define BLE_HOST_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/BLE_UART_BAUD_RATE*10*BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// 单位:ms
#define BLE_HOST_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/BLE_UART_BAUD_RATE*10*BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// 单位:ms

// 用来区分头盔和按键下发的命令
#define BLE_OUTFIT_HELMET_ID							0x01
#define BLE_OUTFIT_LOCK_ID								0x02

#define BLE_OUTFIT_CONNECT_TIME							1000	// 连接间隔
#define BLE_TIME_OUT					            	6000	// 超时时间

#define BLE_OUTFIT_PAIR_SCAN_TIME						5		// 配对扫描时间s
#define BLE_OUTFIT_PAIR_COUNT							-1		// 配对尝试次数
#define BLE_SEND_CMD_TIMEOUT							1000	// 蓝牙发送命令超时时间

// UART发送协议定义	
typedef enum
{
	BLE_HOST_PROTOCOL_CMD_HEAD_INDEX = 0,					// 帧头索引
	BLE_HOST_PROTOCOL_CMD_CMD_INDEX, 						// 命令字索引
	BLE_HOST_PROTOCOL_CMD_LENGTH_INDEX,						// 数据长度索引

	BLE_HOST_PROTOCOL_CMD_DATA1_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA2_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA3_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA4_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA5_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA6_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA7_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA8_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA9_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA10_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA11_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA12_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA13_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA14_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA15_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA16_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA17_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA18_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA19_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA20_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA21_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA22_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA23_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA24_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA25_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA26_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA27_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA28_INDEX,						// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA29_INDEX, 					// 数据索引
	BLE_HOST_PROTOCOL_CMD_DATA30_INDEX, 					// 数据索引

	BLE_HOST_PROTOCOL_CMD_INDEX_MAX
}BLE_HOST_PROTOCOL_DATE_FRAME;

// 下行命令
typedef enum{
	
	BLE_OUTFIT_PROTOCOL_CMD_HELMET = 0xB1,				// 头盔控制命令
	BLE_OUTFIT_PROTOCOL_CMD_HELMET_STATE = 0xB2,		// 头盔状态
	BLE_OUTFIT_PROTOCOL_CMD_HELMET_PHONE = 0xB3,		// 头盔来电提醒命令
	
	BLE_HOST_CMD_MAX,

}BLE_HOST_PROTOCOL_CMD;

// 蓝牙配对状态
typedef enum
{
	BLE_PAIR_INIT = 0,									// 初始化状态
	BLE_PAIR_STANDBY,									// 待机状态
	BLE_PAIR_SCAN,										// 扫描状态
	BLE_PAIR_CONNECT,									// 连接状态
	BLE_PAIR_SUCCEED,									// 配对成功状态
	BLE_PAIR_FAILURE,									// 配对失败状态
	BLE_PAIR_STOP,										// 配对停止状态
	
	BLE_PAIR_MAX,
}BLE_PAIR_STATE;

typedef enum
{
	BLE_MUSIC_PLAY = 1,								// 播放
	BLE_MUSIC_PAUSE = 2,							// 暂停
	BLE_MUSIC_UP = 3,								// 上一曲
	BLE_MUSIC_NEXT = 4,								// 下一曲	
	BLE_MUSIC_ADD = 8,								// 音量+
	BLE_MUSIC_DEC = 9,								// 音量-
	
	BLE_MUSIC_MAX_MODE,
}BLE_MUSIC_MODE;

typedef enum
{
	BLE_NORMAL_MODE = 0,
	BLE_PAIR_MODE,

	BLE_MAX_MODE,
}BLE_MODE;

typedef enum
{
	BLE_HELMET_DEVICE = 0,
	BLE_LOCK_DEVICE,
}BLE_PAIR_DEVICE;

typedef enum
{
	BLE_SEND_CMD_IDE = 0,							// 发送命令空闲
	BLE_SEND_CMD_SEND,								// 发送命令 发送
	BLE_SEND_CMD_WAIT,								// 发送命令 等待结果
	BLE_SEND_CMD_TIMOUT,							// 发送命令 等待超时
	BLE_SEND_CMD_DELAY,								// 延时等待
	BLE_SEND_CMD_STEP_MAX							
}BLE_SEND_CMD_STEP;

typedef enum
{
	BLE_NO_RCV_DATA = 0,
	BLE_RCV_KEYWORD,
	
	BLE_RCV_MAX
}BLE_RCV_CMD_FRAME_MODE_E;

// UART_RX命令帧定义
typedef struct
{
	uint16  deviceID;
	uint8	buff[BLE_HOST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];		// 命令帧缓冲区
	uint16	length; 										// 命令帧有效数据个数
}BLE_HOST_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
	uint16  deviceID;
	uint8	buff[BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];		// 命令帧缓冲区
	uint16	length; 										// 命令帧有效数据个数
}BLE_HOST_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
	// 一级接收缓冲区
	struct
	{
		volatile uint8	buff[BLE_HOST_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;											// 当前待处理的字节的位置下标
	}rxFIFO;
	
	// 接收帧缓冲区数据结构
	struct{
		BLE_HOST_PROTOCOL_RX_CMD_FRAME	cmdQueue[BLE_HOST_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;												// 队列头索引
		uint16			end;												// 队列尾索引
	}rx;

	// 发送帧缓冲区数据结构
	struct{
		BLE_HOST_PROTOCOL_TX_CMD_FRAME	cmdQueue[BLE_HOST_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;														// 队列头索引
		uint16	end;														// 队列尾索引
		uint16	index;														// 当前待发送数据在命令帧中的索引号
		BOOL	txBusy;														// 发送请求，为TRUE时，从待发送队列中取出一个BYTE放入发送寄存器
	}tx;

	// 发送数据接口
	BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	uint32 ageFlag;
	BOOL isTimeCheck;
	
	// TTM命令发送帧缓冲区数据结构
	struct{
		BLE_HOST_PROTOCOL_TX_CMD_FRAME	cmdQueue[BLE_HOST_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;														// 队列头索引
		uint16	end;														// 队列尾索引
		uint16	index;														// 当前待发送数据在命令帧中的索引号
	}ttmTx;

	struct
	{
		BLE_PAIR_STATE state;												// 当前系统状态
		BLE_PAIR_STATE preState;											// 上一个状态
		BLE_PAIR_DEVICE device;												// 配对设备
		uint8 macBuff[20];													// 配对mac地址
		uint8 count;														// 尝试次数
		BOOL scanFlag;														// 扫描设备标志
		BLE_MODE mode;														// 模式选择
	}pair;
	
	// 发送和接收判定数据结构
	struct{
		BLE_SEND_CMD_STEP sendCmdStep;
		uint32 timeout;
		BOOL Flag;	
	}sendCmd;
	
}BLE_HOST_PROTOCOL_CB;

extern BLE_HOST_PROTOCOL_CB bleHostProtocolCB;		
extern uint8 connectFlag ;
/******************************************************************************
* 【外部接口声明】
******************************************************************************/

// 协议初始化
void BLE_HOST_PROTOCOL_Init(void);

// 协议层过程处理
void BLE_HOST_PROTOCOL_Process(void);

// 向发送命令帧队列中添加数据
void BLE_HOST_PROTOCOL_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void BLE_HOST_PROTOCOL_TxAddFrame(void);

// 添加TTM命令
void BLE_HOST_PROTOCOL_TxAddTtmCmd(uint8 data[], uint16 length, uint16 id);

// 解析蓝牙收到的TTM命令
void BLE_HOST_UART_TTM_AnalysisCmd(uint8 buff[], uint8 length);

// 状态机处理
void BLE_PairProcess(void);

// 切换成配对模式
BOOL BLE_SwitchPairMode(BLE_PAIR_DEVICE device);

// 取消配对
void BLE_PairStop(void);

// 上电断开头盔连接
void BLE_HOST_UART_SendCmdDisconnectHelmet(uint32 param);

// 连接头盔
void BLE_HOST_UART_SendCmdConnectHelmet(uint32 param);

// 上电断开无线按键连接
void BLE_HOST_UART_SendCmdDisconnectKey(uint32 param);

// 连接无线按键
void BLE_HOST_UART_SendCmdConnectKey(uint32 param);

// 定时器回调发送设备连接指令
void  BLE_HOST_UART_CALLBALL_OutfitConnect(uint32 param);

// 左转向灯控制
void BLE_HOST_PROTOCOL_SendCmdLeftTurnLight(uint32 param);

// 右转向灯控制
void BLE_HOST_PROTOCOL_SendCmdRightTurnLight(uint32 param);

// 多媒体控制功能
void BLE_HOST_PROTOCOL_SendCmdControlMp3(uint32 param);

// 来电控制功能
void BLE_HOST_PROTOCOL_SendCmdControlPhone(uint32 param);

// 头盔信息查询
void BLE_HOST_PROTOCOL_SendCmdGetHeadInfoPhone(uint32 param);

// 电子锁控制
void BLE_HOST_PROTOCOL_SendCmdGetKey(uint32 param);

// 打开电子锁
void BLE_HOST_PROTOCOL_SendCmdLockOpen(uint32 param);

// 断开连接
void BLE_DISCONNECT(uint32 param);

// 发送复位命令
void BLE_RESET(uint32 param);
#endif

