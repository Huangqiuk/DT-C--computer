/********************************************************************************************************
	天腾动力中置驱动系统通信协议V3r2_20170821
********************************************************************************************************/
#ifndef __CAN_PROTOCOL_UP_GB_H__
#define __CAN_PROTOCOL_UP_GB_H__

#include "common.h"

/*******************************************************************************************************
 * 【程序开发可修改接口】
 * 用户根据使用情况修改
 *******************************************************************************************************/
// CAN总线通讯故障时间
#define CAN_PROTOCOL_COMMUNICATION_TIME_OUT 30000UL // 单位:ms

// 定义模块本身编号
#define CAN_PROTOCOL_DEVICE_SELF_ID CAN_PROTOCOL_DEVICE_ID_HMI

// 源节点使能(顺序不能打乱,使能为TRUE，失能为FALSE)
#define CAN_PROTOCOL_RX_DEVICE_HMI FALSE
#define CAN_PROTOCOL_RX_DEVICE_BATTERY TRUE
#define CAN_PROTOCOL_RX_DEVICE_DRIVER TRUE
#define CAN_PROTOCOL_RX_DEVICE_PUC TRUE

/*******************************************************************************************************
 * 【 Can 高级应用层协议定义】
 * 驱动程序员根据协议修改(以下修改需要咨询驱动开发人员)
 *******************************************************************************************************/
// 定义最大源节点数(用户无须修改)
#define CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX (CAN_PROTOCOL_RX_DEVICE_HMI + CAN_PROTOCOL_RX_DEVICE_DRIVER + CAN_PROTOCOL_RX_DEVICE_BATTERY + CAN_PROTOCOL_RX_DEVICE_PUC)

#define CAN_PROTOCOL_RX_FIFO_SIZE 150  // 接收缓冲区尺寸
#define CAN_PROTOCOL_RX_QUEUE_SIZE 40 // 接收命令帧尺寸
#define CAN_PROTOCOL_TX_QUEUE_SIZE 30 // 发送命令帧尺寸

#define CAN_PROTOCOL_RX_CMD_FRAME_LENGTH_MIN 6 // 接收命令帧最小长度，包括6个字节: 命令头(3Byte) + 数据长度(1Byte) + 校验码(1Byte) + 帧尾(1Byte)
#define CAN_PROTOCOL_TX_CMD_FRAME_LENGTH_MIN 6 // 发送命令帧最小长度，包括6个字节: 命令头(3Byte) + 数据长度(1Byte) + 校验码(1Byte) + 帧尾(1Byte)
#define CAN_PROTOCOL_CMD_FRAME_LENGTH_MAX 8	   // 定义CAN最大的命令长度

#define CAN_PROTOCOL_CMD_HEAD 0x55	  // 命令头
#define CAN_PROTOCOL_CMD_TAIL 0xF0	  // 帧尾
#define CAN_PROTOCOL_CMD_NONHEAD 0xFF // 非命令头
#define CAN_PROTOCOL_CHECK_BYTE 0x01  // 校验字节数
#define CAN_PROTOCOL_TAIL_BYTE 0x01	  // 帧尾字节数

#define CAN_PROTOCOL_ONCE_MESSAGE_MAX_SIZE 0x08 // CAN总线链路层决定，最大发送8个字节

// CAN命令帧协议定义
typedef enum
{
	CAN_PROTOCOL_RX_CMD_HEAD1_INDEX = 0, // RX帧头索引1
	CAN_PROTOCOL_RX_CMD_HEAD2_INDEX,	 // RX帧头索引2
	CAN_PROTOCOL_RX_CMD_HEAD3_INDEX,	 // RX帧头索引3
	CAN_PROTOCOL_RX_CMD_LENGTH_INDEX,	 // RX数据长度
	CAN_PROTOCOL_RX_CMD_CMD_H_INDEX,	 // RX命令字索引高字节
	CAN_PROTOCOL_RX_CMD_CMD_L_INDEX,	 // RX命令字索引低字节

	CAN_PROTOCOL_RX_CMD_DATA1_INDEX, // RX数据索引
	CAN_PROTOCOL_RX_CMD_DATA2_INDEX, // RX数据索引
	CAN_PROTOCOL_RX_CMD_DATA3_INDEX, // RX数据索引
	CAN_PROTOCOL_RX_CMD_DATA4_INDEX, // RX数据索引
	CAN_PROTOCOL_RX_CMD_DATA5_INDEX, // RX数据索引
	CAN_PROTOCOL_RX_CMD_DATA6_INDEX, // RX数据索引
	CAN_PROTOCOL_RX_CMD_DATA7_INDEX, // RX数据索引
	CAN_PROTOCOL_RX_CMD_DATA8_INDEX, // RX数据索引
	CAN_PROTOCOL_RX_CMD_DATA9_INDEX, // RX数据索引

	//=======================================================================
	CAN_PROTOCOL_TX_CMD_HEAD1_INDEX = 0, // TX帧头索引1
	CAN_PROTOCOL_TX_CMD_HEAD2_INDEX,	 // TX帧头索引2
	CAN_PROTOCOL_TX_CMD_HEAD3_INDEX,	 // TX帧头索引3
	CAN_PROTOCOL_TX_CMD_LENGTH_INDEX,	 // TX数据长度
	CAN_PROTOCOL_TX_CMD_CMD_H_INDEX,	 // TX命令字索引高字节
	CAN_PROTOCOL_TX_CMD_CMD_L_INDEX,	 // TX命令字索引低字节

	CAN_PROTOCOL_TX_CMD_DATA1_INDEX, // TX数据索引
	CAN_PROTOCOL_TX_CMD_DATA2_INDEX, // TX数据索引
	CAN_PROTOCOL_TX_CMD_DATA3_INDEX, // TX数据索引
	CAN_PROTOCOL_TX_CMD_DATA4_INDEX, // TX数据索引
	CAN_PROTOCOL_TX_CMD_DATA5_INDEX, // TX数据索引
	CAN_PROTOCOL_TX_CMD_DATA6_INDEX, // TX数据索引
	CAN_PROTOCOL_TX_CMD_DATA7_INDEX, // TX数据索引
	CAN_PROTOCOL_TX_CMD_DATA8_INDEX, // TX数据索引
	CAN_PROTOCOL_TX_CMD_DATA9_INDEX, // TX数据索引

	CAN_PROTOCOL_CMD_INDEX_MAX
} CAN_PROTOCOL_DATE_FRAME;


// PGN 1939数据状态
typedef enum
{
	STATE_PGN_NULL = 0,

	STATE_PGN_SEND_1, // 请求发送
	STATE_PGN_SEND_2, // 允许发送
	STATE_PGN_SEND_3, // 发送结束
	STATE_PGN_SEND_4,

	STATE_PGN_GET_1,
	STATE_PGN_GET_2,
	STATE_PGN_GET_3,
	STATE_PGN_GET_4,
	STATE_PGN_GET_5,
	STATE_PGN_GET_6,
} STATE_PGN;

// ■■ 驱动器上报至码表的参数 ■■
typedef struct
{
	uint8 ucBatMcuState : 1; // 电池MCU状态
	uint8 reserved : 3;		 // 预留
	uint8 ucBatState : 4;	 // BMS状态

	uint8 ucBatPercent1; // 电池电量，单位:0.1%
	uint8 ucBatPercent2 : 2;

	uint8 ucBatHealthState1 : 6; // 电池健康状态，单位:0.1%
	uint8 ucBatHealthState2 : 4;

	uint8 ucBatTotalVol1 : 4; // 电池内总压，单位:0.1V
	uint8 ucBatTotalVol2;

	uint8 ucBatOutputlVol1; // 电池输出总压，单位:0.1V
	uint8 ucBatOutputlVol2 : 4;

	uint8 ucBatCurrent1 : 4; // 当前电池电流，单位:0.1A
	uint8 ucBatCurrent2;

} CAN_PROTOCOL_RX_BAT_PARAM_CB;
#define CAN_PROTOCOL_RX_BAT_PARAM_SIZE (sizeof(CAN_PROTOCOL_RX_BAT_PARAM_CB))

// ■■ 驱动器上报至码表的参数 ■■
typedef struct
{
	uint8 ucHmiMcuState : 1; // 仪表MCU程序状态
	uint8 ucBluMcuState : 1; // 仪表BLE程序状态
	uint8 reserved1 : 2;	 // 预留
	uint8 ucHmiState : 4;	 // 仪表状态

	uint8 ucHmiError1;
	uint8 ucHmiError2; // 仪表故障码

	uint8 ucHmiSetMode : 2; // 仪表设置模式
	uint8 ucHmiLight : 2;	// 仪表灯控状态
	uint8 ucHmiAssist : 4;	// 仪表助力模式请求

	uint8 ucBluKeyEnable : 2; // 蓝牙钥匙使能状态
	uint8 ucBluKeyUnLock : 2; // 蓝牙钥匙解锁状态
	uint8 ucHmiPowerCol : 2;  // 仪表上电控制
	uint8 reserved2 : 2;

	uint8 reserved3; // 预留
	uint8 bikeTypeL; // 整车车型L
	uint8 bikeTypeH; // 整车车型H
} CAN_PROTOCOL_TX_HMI_PARAM_CB;
#define CAN_PROTOCOL_TX_HMI_PARAM_SIZE (sizeof(CAN_PROTOCOL_TX_HMI_PARAM_CB))

// CAN命令帧定义
typedef struct
{
	union
	{
		uint32 deviceID;
		struct
		{
			uint32 source : 8;
			uint32 destination : 8;
			uint32 pgn : 10;
			uint32 priority : 3;
			uint32 nHeard : 3;
		} sDet;
	} EXTID;
	uint8 buff[CAN_PROTOCOL_CMD_FRAME_LENGTH_MAX]; // 命令帧缓冲区
	uint16 length;								   // 命令帧有效数据个数
} CAN_PROTOCOL_CMD_FRAME;

// 一级接收缓冲区(存放上层协议数据)
typedef struct
{
	volatile uint8 buff[CAN_PROTOCOL_RX_FIFO_SIZE];
	volatile uint16 head;
	volatile uint16 end;
	uint16 currentProcessIndex; // 当前待处理的字节的位置下标
} CAN_PROTOCOL_RX_FIFO;

// can一帧的数据内容
typedef struct
{
	uint32 StdId;
	union
	{
		uint32 ExtId;
		struct
		{
			uint32 nFn : 8;
			uint32 nAfn : 8;
			uint32 nCmd : 3;
			uint32 nDestId : 5;
			uint32 nSrcId : 5;
			uint32 nHeard : 3;
		} sDet;
	} uExtId;
	uint8 IDE;
	uint8 RTR;
	uint8 DLC;
	uint8 Data[8];
	uint8 FMI;
} CAN_RX_FRAME_MAP;

// 接收帧缓冲区数据结构(解析成标准CAN消息，并进行命令解析)
typedef struct
{
	CAN_RX_FRAME_MAP cmdQueue[CAN_PROTOCOL_RX_QUEUE_SIZE];
	uint16 head; // 队列头索引
	uint16 end;	 // 队列尾索引
} CAN_PROTOCOL_RX;

// CAN控制结构体定义
typedef struct
{
	// 一级接收缓冲区(存放上层协议数据)
	struct
	{
		CAN_PROTOCOL_RX_FIFO rxFIFOEachNode[CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX];
	} rxFIFO;

	// 接收帧缓冲区数据结构(解析成标准CAN消息，并进行命令解析)
	struct
	{
		CAN_PROTOCOL_RX rxEachNode[CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX];
	} rx;

	// 发送帧缓冲区数据结构(解析成标准CAN消息)
	struct
	{
		CAN_PROTOCOL_CMD_FRAME cmdQueue[CAN_PROTOCOL_TX_QUEUE_SIZE];
		uint16 head;  // 队列头索引
		uint16 end;	  // 队列尾索引
		uint16 index; // 当前待发送数据在命令帧中的索引号
		BOOL txBusy;  // 发送请求
	} tx;

	// 发送数据接口
	BOOL(*sendDataThrowService)
	(uint32 id, uint8 *pData, uint8 length);

	BOOL txPeriodRequest; // 码表周期性发送数据请求
	BOOL txAtOnceRequest; // 码表马上发送数据请求
	BOOL controlFlag;

	// 平滑算法数据结构
	struct
	{
		uint16 realSpeed;
		uint16 proSpeed;
	} smooth;

	struct
	{
		uint32 distance;
		uint32 times;
	} tripData;

	STATE_PGN state;		   // 当前系统状态
	STATE_PGN preState;		   // 上一个状态
	uint8 sendBuff[200];	   // 发送数据buff
	uint8 keyPasswordBuff[20]; // 加密KEY
	BOOL hmiState;
	// PGN接收处理
	struct
	{

		uint8 numbers;		// 接收包长度
		uint16 byteNumbers; // 字节数
		uint8 PGN;			// 当前接收的PGN

		uint8 dataBuff[200]; // 接收数据buff
	} PGN_RX;

	struct
	{
		uint16 magicNumber;
		uint8 funCode;
		uint8 data[40];
	} encodeData;

	BOOL UPDATA;
	BOOL UPDATA1;
} CAN_PROTOCOL_CB;

// 码表与控制器数据交互结构体定义
typedef struct
{
	// 驱动器上报至码表的参数
	union
	{
		uint8 buff[CAN_PROTOCOL_RX_BAT_PARAM_SIZE];
		CAN_PROTOCOL_RX_BAT_PARAM_CB param;
	} rxBat;

	// 仪表发送数据
	union
	{
		uint8 buff[CAN_PROTOCOL_TX_HMI_PARAM_SIZE];
		CAN_PROTOCOL_TX_HMI_PARAM_CB param;
	} tx;

} CAN_PROTOCOL_PARAM_RX_CB;

extern CAN_PROTOCOL_PARAM_RX_CB canRxDriveCB;
extern CAN_PROTOCOL_CB canProtocolCB;

//=======================================================================================
// 协议初始化
void CAN_PROTOCOL_Init(void);

// 协议层过程处理
void CAN_PROTOCOL_Process_GB(void);

// 向发送命令帧队列中添加帧源，帧目的
void CAN_PROTOCOL_TxAddNewFrame(uint8 priority, uint16 pgn, uint8 destinationID, uint8 sourceID);

// 向发送命令帧队列中添加数据
void CAN_PROTOCOL_TxAddData(uint8 data);

// CAN消息命令帧扶正接口
void CAN_PROTOCOL_TxAddFrame(void);

//==================================================================================
// 设置下发请求标志，按键设置参数也是调用此接口
void CAN_PROTOCOL_SetTxAtOnceRequest(uint32 param);

// state : 当前状态   DestAddress:目的地址   SourceAddress:源地址
void CAN_PGN_RequestSendData(STATE_PGN state, uint8 PGN, uint8 DestAddress, uint8 SourceAddress);

// state : 当前状态   DestAddress:目的地址   SourceAddress:源地址
void CAN_PGN_RequestGetData(STATE_PGN state, uint8 PGN, uint8 DestAddress, uint8 SourceAddress);

// 设备升级复位id  0x1803FF28
void CMD_CAN_HmiToDriverSendResetData(uint32 param);

// 档位助力曲线设置 0x184FEF28
void CMD_CAN_HmiStAssistParam(uint8 assist, uint8 param1, uint8 param2, uint8 param3);

// 轮径后轮设置 0x184FEFF9
void CMD_CAN_HmiSetWheelSizeParam(void);

// 开关机请求确认0x18221828
void CMD_CAN_HmiPowerOffParam(uint32 param);

// 设置参数状态 0x18041828
void CMD_CAN_HmiSettingStateParam(uint32 param);

// 工作状态 0x1825FF28
void CMD_CAN_HmiWorkStateParam(uint32 param);

// 主动发送故障信息请求
void CAN_SendErrorRequest(uint32 param);

void CAN_UP_DATA(uint32 param);

void CAN_UP_RESET(uint32 param);

extern CAN_PROTOCOL_RX canProtocolRX;
#endif
