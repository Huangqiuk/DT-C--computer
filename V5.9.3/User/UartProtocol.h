/********************************************************************************************************
		KM5S与控制器通讯协议
********************************************************************************************************/
#ifndef 	__UART_PROTOCOL_H__
#define 	__UART_PROTOCOL_H__

#include "common.h"

//=====================================================================================================
#define UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// 通讯单向接收超时检测功能开关:0禁止，1使能

#define UART_PROTOCOL_RX_QUEUE_SIZE					5		// 接收命令队列尺寸
#define UART_PROTOCOL_TX_QUEUE_SIZE					5		// 发送命令队列尺寸

#define UART_PROTOCOL_CMD_HEAD						0x55	// 命令头
#define UART_PROTOCOL_CMD_HEAD_3A					0x3A	// 命令头

#define UART_PROTOCOL_CMD_DEVICE_ADDR				0x1A	// 设备地址

#define UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		150		// 最大发送命令帧长度
#define UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		150		// 最大接收命令帧长度
#define UART_PROTOCOL_RX_FIFO_SIZE					200		// 接收一级缓冲区大小
#define UART_PROTOCOL_CMD_FRAME_LENGTH_MIN			4		// 命令帧最小长度，包含:命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA


// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// 单位:ms
#define UART_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// 单位:ms

// UART发送协议定义	
typedef enum
{
	UART_PROTOCOL_CMD_HEAD_INDEX = 0,					// 帧头索引
	UART_PROTOCOL_CMD_CMD_INDEX, 						// 命令字索引
	UART_PROTOCOL_CMD_LENGTH_INDEX,						// 数据长度

	UART_PROTOCOL_CMD_DATA1_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA2_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA3_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA4_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA5_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA6_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA7_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA8_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA9_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA10_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA11_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA12_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA13_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA14_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA15_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA16_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA17_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA18_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA19_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA20_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA21_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA22_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA23_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA24_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA25_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA26_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA27_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA28_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA29_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA30_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA31_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA32_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA33_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA34_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA35_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA36_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA37_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA38_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA39_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA40_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA41_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA42_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA43_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA44_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA45_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA46_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA47_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA48_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA49_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA50_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA51_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA52_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA53_INDEX,						// 数据索引
	UART_PROTOCOL_CMD_DATA54_INDEX,						// 数据索引

	UART_PROTOCOL_CMD_INDEX_MAX
}UART_PROTOCOL_DATE_FRAME;

// 被控命令
typedef enum{
	UART_PROTOCOL_CMD_NULL = 0,								// 空命令

	UART_CMD_UP_PROJECT_APPLY = 0x01,						// DUT工程模式申请,bc280ul

	UART_CMD_UP_PROJECT_ALLOW = 0x02,						// DUT工程模式批准,bc352

	UART_CMD_UP_PROJECT_READY = 0x03,						// DUT工程模式准备就绪

	UART_ECO_CMD_ECO_JUMP_APP = 0x04,						// 升级完成后跳入app

	UART_CMD_UP_APP_EAR = 0x05,								// 352app升级开始

	UART_CMD_UP_APP_UP = 0x06,								// 352app升级

	UART_CMD_UP_APP_UP_OVER = 0x07,							// 352app升级结束

	UART_CMD_WRITE_SYS_PARAM = 0x10,						// DUT_config写入（可能要在状态机中发送送）

	UART_CMD_READ_SYS_PARAM = 0x11,							// DUT_config读取

	UART_CMD_DUT_UI_DATA_ERASE = 0x12,						// DUT_UI擦写

	UART_CMD_DUT_UI_DATA_ERASE_ACK = 0x13,					// DUT_UI擦写应答

	UART_CMD_DUT_UI_DATA_WRITE = 0x14,						// DUT_UI写入

	UART_CMD_DUT_UI_DATA_WRITE_RES = 0x15,					// DUT_UI写入结果

	UART_CMD_DUT_APP_ERASE_FLASH = 0x24,					// DUT_APP擦写

	UART_CMD_DUT_APP_ERASE_RESULT = 0x25,					// DUT_APP擦写结果查询

	UART_CMD_DUT_APP_WRITE_FLASH = 0x26,					// DUT_APP写入

	UART_CMD_DUT_APP_WRITE_FLASH_RES = 0x27,				// DUT_APP写入结果

	UART_CMD_DUT_UPDATA_FINISH = 0x2A,                      // DUT_APP写入完成

	UART_CMD_UP_UPDATA_FINISH_RESULT = 0x2B,		     	// DUT_APP写入结果确认
	
	
	// 控制命令
	UART_PROTOCOL_CMD_HMI_CTRL_RUN = 0x52,					// 正常运行状态

	UART_PROTOCOL_CMD_HMI_CTRL_PARAM_SET = 0x53,			// 参数设置

	UART_PROTOCOL_CMD_RST = 0x80,							// 复位命令

	UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM = 0xC0,			// 写控制器参数
	UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM_RESULT = 0xC1,	// 写控制器参数结果应答
	UART_PROTOCOL_CMD_READ_CONTROL_PARAM = 0xC2,			// 读取控制器参数
	UART_PROTOCOL_CMD_READ_CONTROL_PARAM_REPORT = 0xC3,		// 控制器参数上报

	UART_PROTOCOL_CMD_FLAG_ARRAY_READ = 0xA6,				// 读标志区数据
	UART_PROTOCOL_CMD_FLAG_ARRAY_WRITE = 0xA7,				// 写标志区数据

	UART_PROTOCOL_CMD_VERSION_TYPE_WRITE = 0xA8,			// 写入版本信息
	UART_PROTOCOL_CMD_VERSION_TYPE_READ = 0xA9,				// 读取版本信息

	UART_PROTOCOL_CMD_TEST_LCD = 0xAA,						// LCD颜色显示控制
	UART_PROTOCOL_CMD_PROTOCOL_SWITCH = 0xAB,				// 协议切换命令

	UART_PROTOCOL_CMD_AD_RATIO_REF = 0xAC,					// ldo与分压电压的校准值
	UART_PROTOCOL_CMD_KEY_CHECK_TEST = 0xAD,				// 按键检测测试
	UART_PROTOCOL_CMD_CLOCK_CHECK_TEST = 0xAE,				// 时钟检测测试
	UART_PROTOCOL_CMD_SENSOR_CHECK_TEST = 0xAF,				// 光敏检测测试
	UART_PROTOCOL_CMD_BLUE_MACADDR_TEST = 0xB0,				// 蓝牙MAC地址读取
	UART_PROTOCOL_CMD_CLEAR_MILEAGE = 0xB5,					// 清除里程数据
	

	UART_PROTOCOL_CMD_MAX									// 总命令数
}UART_PROTOCOL_CMD;

// UART_RX命令帧定义
typedef struct
{
	uint16  deviceID;
	uint8	buff[UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];	// 命令帧缓冲区
	uint16	length; 										// 命令帧有效数据个数
}UART_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
	uint16  deviceID;
	uint8	buff[UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];	// 命令帧缓冲区
	uint16	length; 										// 命令帧有效数据个数
}UART_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
	// 一级接收缓冲区
	struct
	{
		volatile uint8	buff[UART_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;					// 当前待处理的字节的位置下标
	}rxFIFO;
	
	// 接收帧缓冲区数据结构
	struct{
		UART_PROTOCOL_RX_CMD_FRAME	cmdQueue[UART_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;						// 队列头索引
		uint16			end;						// 队列尾索引
	}rx;

	// 发送帧缓冲区数据结构
	struct{
		UART_PROTOCOL_TX_CMD_FRAME	cmdQueue[UART_PROTOCOL_TX_QUEUE_SIZE];
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
}UART_PROTOCOL_CB;

extern UART_PROTOCOL_CB uartProtocolCB;		

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

void  UART_PROTOCOL_TxAddFrame_3A(void);

// 发送命令回复，带一个参数
void UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// UART报文接收处理函数(注意根据具体模块修改)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// 发送数据包
void uartProtocol_SendOnePacket(uint32 flashAddr,uint32 addr);
// 发送数据包（HEX）
void uartProtocol_SendOnePacket_Hex(uint32 flashAddr);
#endif

