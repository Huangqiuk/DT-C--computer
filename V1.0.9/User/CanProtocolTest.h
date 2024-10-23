#ifndef 	__CAN_PROTOCOL_TEST_H__
#define 	__CAN_PROTOCOL_TEST_H__

#include "common.h"
#include "dutInfo.h"
/** @  *****************************************************************************************************
  * 【程序开发可修改接口】 
  * 用户根据使用情况修改
************************************************************************************************************/
#define CAN_BAUD_RATE						CAN_BAUD_RATE_500K

// 定义模块本身编号
#define CAN_TEST_DEVICE_SELF_ID					CAN_TEST_DEVICE_ID_HMI	
 
// 源节点使能(顺序不能打乱,使能为TRUE，失能为FALSE)
#define CAN_TEST_RX_DEVICE_PUC					TRUE
#define CAN_TEST_RX_DEVICE_HMI					FALSE

// CAN单元测试开关
#define CAN_UNIT_TEST						0

//============================================================
#define CAN_BAUD_RATE_2K					2
#define CAN_BAUD_RATE_3K					3
#define CAN_BAUD_RATE_5K					5
#define CAN_BAUD_RATE_10K					10
#define CAN_BAUD_RATE_20K					20
#define CAN_BAUD_RATE_30K					30
#define CAN_BAUD_RATE_40K					40
#define CAN_BAUD_RATE_50K					50
#define CAN_BAUD_RATE_60K					60
#define CAN_BAUD_RATE_80K					80
#define CAN_BAUD_RATE_90K					90
#define CAN_BAUD_RATE_100K					100
#define CAN_BAUD_RATE_125K					125
#define CAN_BAUD_RATE_150K					150
#define CAN_BAUD_RATE_200K					200
#define CAN_BAUD_RATE_250K					250
#define CAN_BAUD_RATE_300K					300
#define CAN_BAUD_RATE_400K					400
#define CAN_BAUD_RATE_500K					500
#define CAN_BAUD_RATE_600K					600
#define CAN_BAUD_RATE_666K					666
#define CAN_BAUD_RATE_800K					800
#define CAN_BAUD_RATE_900K					900
#define CAN_BAUD_RATE_1000K					1000

/**********************************************************************************************************/


/** @  *****************************************************************************************************
  * 【 Can 高级应用层协议定义】 
  * 驱动程序员根据协议修改(以下修改需要咨询驱动开发人员) 
  **********************************************************************************************************/  
	// 定义最大源节点数(用户无须修改)
#define CAN_TEST_DEVICE_SOURCE_ID_MAX			(CAN_TEST_RX_DEVICE_PUC + CAN_TEST_RX_DEVICE_HMI)

#define CAN_TEST_RX_FIFO_SIZE					80	    // 接收缓冲区尺寸
#define CAN_TEST_RX_QUEUE_SIZE					80	    // 接收命令帧尺寸
#define CAN_TEST_TX_QUEUE_SIZE					80		// 发送命令帧尺寸	  

#define CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN			4		// 命令帧最小长度，包括4个字节: 命令头(1Byte)+命令字(1Byte)+数据长度(1Byte)+校验码(1Byte)
#define CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX			140		// 定义CAN最大的命令长度为140个字节

#define CAN_PROTOCOL_TEST_HEAD						0x55	// 命令头
#define CAN_PROTOCOL_TEST_NONHEAD 					0xFF	// 非命令头
#define CAN_CHECK_BYTE_SIZE 				0x01	// 校验码所占字节数
#define CAN_ONCEMESSAGE_MAX_SIZE 			0x08	// CAN总线链路层决定，最大发送8个字节

// CAN总线节点编号
typedef enum{
	CAN_TEST_DEVICE_ID_PUC = 0x000,					// PUC节点	0x000
	CAN_TEST_DEVICE_ID_HMI = 0x001,					// HMI节点	0x001

	CAN_TEST_DEVICE_ID_MAX							// 最大节点数
}CAN_TEST_DEVICE_ID;

// CAN发送协议定义	
typedef enum
{
	CAN_PROTOCOL_TEST_HEAD_INDEX = 0, 					// 帧头索引
	CAN_PROTOCOL_TEST_CMD_INDEX,						// 命令字索引
	CAN_PROTOCOL_TEST_LENGTH_INDEX,						// 数据长度

	CAN_PROTOCOL_TEST_DATA1_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA2_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA3_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA4_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA5_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA6_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA7_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA8_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA9_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA10_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA11_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA12_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA13_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA14_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA15_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA16_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA17_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA18_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA19_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA20_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA21_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA22_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA23_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA24_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA25_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA26_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA27_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA28_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA29_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA30_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA31_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA32_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA33_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA34_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA35_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA36_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA37_INDEX,						// 数据索引
	CAN_PROTOCOL_TEST_DATA38_INDEX,						// 数据索引	

	CAN_PROTOCOL_TEST_INDEX_MAX,
}CAN_TEST_DATE_FRAME;


// CAN总线命令
typedef enum{
	// 系统命令
	CAN_PROTOCOL_TEST_CMD_NULL = 0,                              // 空命令

    CAN_PROTOCOL_TEST_CMD_HEADLIGHT_CONTROL = 0xEA,              // 大灯控制
    CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD = 0xEB,          // 获取油门/刹车的模拟数字值
    CAN_PROTOCOL_TEST_CMD_TURN_SIGNAL_CONTROL = 0xEC,            // 转向灯控制
	CAN_PROTOCOL_TEST_CMD_ONLINE_DETECTION = 0xED,               // 在线检测
    CAN_PROTOCOL_TEST_CMD_GET_PHOTORESISTOR_VALUE = 0x90,        // 获取光敏传感器的数值
    CAN_PROTOCOL_TEST_CMD_VOLTAGE_CALIBRATION = 0x91,            // 进行电压校准
    CAN_PROTOCOL_TEST_CMD_KEY_TEST = 0x05,                       // 按键测试

    CAN_PROTOCOL_TEST_CMD_READ_FLAG_DATA = 0xA6,               // 读取标志区数据
    CAN_PROTOCOL_TEST_CMD_WRITE_FLAG_DATA = 0xA7,              // 写入标志区数据
    CAN_PROTOCOL_TEST_CMD_WRITE_VERSION_TYPE_DATA = 0xA8,      // 写入版本类型数据
    CAN_PROTOCOL_TEST_CMD_READ_VERSION_TYPE_INFO = 0xA9,       // 读取版本类型信息
		
	CAN_PROTOCOL_TEST_CMD_BLUETOOTH_MAC_ADDRESS_READ = 0x0C,   // 蓝牙测试  
//	CAN_PROTOCOL_TEST_CMD_BLUETOOTH_TEST = 0x0A,               // 蓝牙测试     
    CAN_PROTOCOL_TEST_CMD_TEST_LCD = 0x03,                     // LCD颜色测试（命令字=0x03）
    CAN_PROTOCOL_TEST_CMD_FLASH_CHECK_TEST = 0x09,             // Flash 校验测试

    CAN_PROTOCOL_TEST_CMD_MAX                                  // 总命令数


}CAN_PROTOCOL_TEST;

// CAN命令帧定义
typedef struct
{
	uint32	deviceID;
	uint8	buff[CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX]; 	 // 命令帧缓冲区 
	uint16	length; 							 // 命令帧有效数据个数
}CAN_PROTOCOL_TEST_FRAME;

// 一级接收缓冲区(存放上层协议数据)
typedef struct
{
	volatile uint8	buff[CAN_TEST_RX_FIFO_SIZE];
	volatile uint16	head;
	volatile uint16	end;
	uint16 currentProcessIndex;					// 当前待处理的字节的位置下标
}CAN_TEST_RX_FIFO;

// 接收帧缓冲区数据结构(解析成标准CAN消息，并进行命令解析)
typedef struct{
	CAN_PROTOCOL_TEST_FRAME	cmdQueue[CAN_TEST_RX_QUEUE_SIZE];
	uint16		head;							// 队列头索引
	uint16		end;							// 队列尾索引
}CAN_TEST_RX;


// CAN控制结构体定义
typedef struct
{
	// 一级接收缓冲区(存放上层协议数据)
	struct{
		CAN_TEST_RX_FIFO rxFIFOEachNode[CAN_TEST_DEVICE_SOURCE_ID_MAX];
	}rxFIFO;
	
	
	// 接收帧缓冲区数据结构(解析成标准CAN消息，并进行命令解析)
	struct{
		CAN_TEST_RX rxEachNode[CAN_TEST_DEVICE_SOURCE_ID_MAX];
	}rx;


	// 发送帧缓冲区数据结构(解析成标准CAN消息)
	struct{
		CAN_PROTOCOL_TEST_FRAME	cmdQueue[CAN_TEST_TX_QUEUE_SIZE];
		volatile uint16 head;						// 队列头索引
		volatile uint16 end; 						// 队列尾索引
		volatile uint16 index;						// 当前待发送数据在命令帧中的索引号
		volatile BOOL txBusy;						// 发送请求
	}tx;

}CAN_TEST_CB;

extern CAN_TEST_CB testCanCB;

/**********************************************************************************************************/


/** @  *****************************************************************************************************
  * 【CAN发送状态机】 
  *  Can 高级应用层协议定义，驱动程序员根据协议修改
  * @{	
  **********************************************************************************************************/

// CAN发送系统状态定义
typedef enum
{
	CAN_TEST_TX_STATE_NULL = 0,									// 空状态
	
	CAN_TEST_TX_STATE_ENTRY,										// 入口状态 
	CAN_TEST_TX_STATE_STANDBY,									// 待机状态
	CAN_TEST_TX_STATE_FILL_SEND, 								// Tx填充与发送
	CAN_TEST_TX_STATE_SENDING,									// Tx发送中
	CAN_TEST_TX_STATE_ERR_PROCESS,								// Tx错误处理
	CAN_TEST_TX_STATE_ERR_RESCOVRY,								// Tx错误修复
	CAN_TEST_TX_STATE_ERR_ALARM, 								// Tx错误报警
	
	CAN_TEST_TX_STATE_MAX,										// Tx状态数  
	
}CAN_TEST_TX_STATE_E;

typedef struct
{
	CAN_TEST_TX_STATE_E state;									// 当前系统状态
	CAN_TEST_TX_STATE_E preState;								// 上一个状态	
	uint8 canTxErrNum;										// CAN消息发送错误计数器
	uint8 retry;											// CAN消息重发
	uint8 retrySnd; 										// 修复错误后，CAN消息第二次重发

	uint8 canIntFlag;										// CAN消息中断标志位
	uint8 canErrFlag;										// CAN消息错误标志位

}CAN_TEST_TX_STATE_CB;

extern CAN_TEST_TX_STATE_CB canTestTxStateCB;

//=====外部调用函数申明=======================================================================
// 模块初始化接口
void CAN_PROTOCOL_TEST_Init(uint16 bund);

void CAN_TEST_HW_Init(uint16 bund);

// CAN处理过程函数，在 main 中大循环内调用
void CAN_PROTOCOL_Process_Test(void);
	
// 向发送缓冲区中添加数据
void CAN_PROTOCOL_TEST_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void CAN_PROTOCOL_TEST_TxAddFrame(void);

// 写入标志区数据
void CAN_PROTOCOL_TEST_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam);

// 发送命令不带应答
void CAN_PROTOCOL_TEST_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);
//=============================================================================================
// 工程模式启用
void CAN_PROTOCOL_TEST_SendCmdProjectApply(uint8 *param);

// 向发送缓冲区中添加一条待发送序列
BOOL CAN_TEST_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length);

void CAN_PROTOCOL_TEST_SendCmdAck(uint8 param);

// 发送升级数据
void CAN_PROTOCOL_TEST_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_PROTOCOL_TEST cmd, uint32 falshAddr, uint32 addr);

// CAN 报文接收处理函数(注意根据具体模块修改)
extern void CAN_TEST_PROTOCOL_MacProcess(uint32 standarID, uint8 *pData, uint8 length);

// CAN 报文接收处理函数(注意根据具体模块修改)
void CAN_TEST_MacProcess(uint16 standarID, uint8 *data, uint8 length);

// 发送命令测试
void CAN_PROTOCOL_TEST_SendCmdTest(uint32 param);

//// 发送成功启动下一帧
//BOOL CAN_TEST_Tx_NextFram(CAN_TEST_CB *pCB);

//// 填充与发送处理:TRUE,填充与发送；FALSE，环形队列向前滑移.
//BOOL CAN_TEST_Tx_FillAndSend(CAN_TEST_CB *pCB);
#endif


