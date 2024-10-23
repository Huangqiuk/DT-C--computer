#ifndef 	__IAP_CTRL_CAN_H__
#define 	__IAP_CTRL_CAN_H__

#include "common.h"
#include "dutInfo.h"
/** @  *****************************************************************************************************
  * 【程序开发可修改接口】 
  * 用户根据使用情况修改
************************************************************************************************************/
#define CAN_BAUD_RATE						CAN_BAUD_RATE_500K

// 定义模块本身编号
#define CAN_DEVICE_SELF_ID					CAN_DEVICE_ID_HMI	
 
// 源节点使能(顺序不能打乱,使能为TRUE，失能为FALSE)
#define CAN_RX_DEVICE_PUC					TRUE
#define CAN_RX_DEVICE_HMI					FALSE

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
#define CAN_DEVICE_SOURCE_ID_MAX			(CAN_RX_DEVICE_PUC+CAN_RX_DEVICE_HMI)

#define CAN_RX_FIFO_SIZE					150 		// 接收缓冲区尺寸
#define CAN_RX_QUEUE_SIZE					11		// 接收命令帧尺寸
#define CAN_TX_QUEUE_SIZE					10		// 发送命令帧尺寸	  

#define CAN_CMD_FRAME_LENGTH_MIN			4		// 命令帧最小长度，包括4个字节: 命令头(1Byte)+命令字(1Byte)+数据长度(1Byte)+校验码(1Byte)
#define CAN_CMD_FRAME_LENGTH_MAX			150		// 定义CAN最大的命令长度为140个字节

#define CAN_CMD_HEAD						0x55	// 命令头
#define CAN_CMD_NONHEAD 					0xFF	// 非命令头
#define CAN_CHECK_BYTE_SIZE 				0x01	// 校验码所占字节数
#define CAN_ONCEMESSAGE_MAX_SIZE 			0x08	// CAN总线链路层决定，最大发送8个字节

// CAN总线节点编号
typedef enum{
	CAN_DEVICE_ID_PUC = 0x000,					// PUC节点	0x000
	CAN_DEVICE_ID_HMI = 0x001,					// HMI节点	0x001

	CAN_DEVICE_ID_MAX							// 最大节点数
}CAN_DEVICE_ID;

// CAN发送协议定义	
typedef enum
{
	CAN_CMD_HEAD_INDEX = 0, 					// 帧头索引
	CAN_CMD_CMD_INDEX,							// 命令字索引
	CAN_CMD_LENGTH_INDEX,						// 数据长度

	CAN_CMD_DATA1_INDEX,						// 数据索引
	CAN_CMD_DATA2_INDEX,						// 数据索引
	CAN_CMD_DATA3_INDEX,						// 数据索引
	CAN_CMD_DATA4_INDEX,						// 数据索引
	CAN_CMD_DATA5_INDEX,						// 数据索引
	CAN_CMD_DATA6_INDEX,						// 数据索引
	CAN_CMD_DATA7_INDEX,						// 数据索引
	CAN_CMD_DATA8_INDEX,						// 数据索引
	CAN_CMD_DATA9_INDEX,						// 数据索引
	CAN_CMD_DATA10_INDEX,						// 数据索引
	CAN_CMD_DATA11_INDEX,						// 数据索引
	CAN_CMD_DATA12_INDEX,						// 数据索引
	CAN_CMD_DATA13_INDEX,						// 数据索引
	CAN_CMD_DATA14_INDEX,						// 数据索引
	CAN_CMD_DATA15_INDEX,						// 数据索引
	CAN_CMD_DATA16_INDEX,						// 数据索引
	CAN_CMD_DATA17_INDEX,						// 数据索引
	CAN_CMD_DATA18_INDEX,						// 数据索引
	CAN_CMD_DATA19_INDEX,						// 数据索引
	CAN_CMD_DATA20_INDEX,						// 数据索引
	CAN_CMD_DATA21_INDEX,						// 数据索引
	CAN_CMD_DATA22_INDEX,						// 数据索引
	CAN_CMD_DATA23_INDEX,						// 数据索引
	CAN_CMD_DATA24_INDEX,						// 数据索引
	CAN_CMD_DATA25_INDEX,						// 数据索引
	CAN_CMD_DATA26_INDEX,						// 数据索引
	CAN_CMD_DATA27_INDEX,						// 数据索引
	CAN_CMD_DATA28_INDEX,						// 数据索引
	CAN_CMD_DATA29_INDEX,						// 数据索引
	CAN_CMD_DATA30_INDEX,						// 数据索引
	CAN_CMD_DATA31_INDEX,						// 数据索引
	CAN_CMD_DATA32_INDEX,						// 数据索引
	CAN_CMD_DATA33_INDEX,						// 数据索引
	CAN_CMD_DATA34_INDEX,						// 数据索引
	CAN_CMD_DATA35_INDEX,						// 数据索引
	CAN_CMD_DATA36_INDEX,						// 数据索引
	CAN_CMD_DATA37_INDEX,						// 数据索引
	CAN_CMD_DATA38_INDEX,						// 数据索引	

	CAN_CMD_INDEX_MAX,
}CAN_DATE_FRAME;


// CAN总线命令
typedef enum{
	// 系统命令
	IAP_CTRL_CAN_CMD_EMPTY = 0,									// 空命令

	IAP_CTRL_CAN_CMD_UP_PROJECT_APPLY = 0x01,					// 工程模式启用申请
	IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK = 0x02,				// 工程模式批准应答 
	IAP_CTRL_CAN_CMD_UP_PROJECT_READY = 0x03,					// 工程模式准备就绪

	IAP_CTRL_CAN_CMD_SEG_RUN_APP_ACK = 0x04,					// 段码屏数码管运行app结果应答
	IAP_CTRL_CAN_HUAXIN_CMD_APP_EAR = 0x05,						// 华芯微特can升级擦除app命令

	IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP = 0x06,					// 华芯微特can升级写app命令
	IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP_COMPLETE = 0x07,			// 华芯微特can升级写app完成命令

	IAP_CTRL_CAN_CMD_DOWN_UI_DATA_ERASE = 0x12,					// UI数据擦除
	IAP_CTRL_CAN_CMD_UP_UI_DATA_ERASE_ACK = 0x13,				// UI数据擦除结果

	IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE = 0x14,					// UI数据写入
	IAP_CTRL_CAN_CMD_UP_UI_DATA_WRITE_RESULT = 0x15,			// UI数据写入结果

	IAP_CTRL_CAN_CMD_DOWN_IAP_ERASE_FLASH = 0x24, 				// 擦除APP_FLASH命令
	IAP_CTRL_CAN_CMD_UP_IAP_ERASE_FLASH_RESULT = 0x25, 			// 擦除APP_FLASH结果上报命令
	IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH = 0x26, 				// IAP数据写入
	IAP_CTRL_CAN_CMD_UP_IAP_WRITE_FLASH_RESULT = 0x27, 			// IAP数据写入结果上报
	IAP_CTRL_CAN_CMD_DOWN_CHECK_FALSH_BLANK = 0x28, 			// 查空
	IAP_CTRL_CAN_CMD_UP_CHECK_FLASH_BLANK_RESULT = 0x29, 		// 查空结果上报
	IAP_CTRL_CAN_CMD_DOWN_UPDATA_FINISH = 0x2A, 				// 升级结束
	IAP_CTRL_CAN_CMD_UP_UPDATA_FINISH_RESULT = 0x2B, 			// 升级结束确认	
	
}CAN_CMD;

// CAN命令帧定义
typedef struct
{
	uint32	deviceID;
	uint8	buff[CAN_CMD_FRAME_LENGTH_MAX]; 	 // 命令帧缓冲区 
	uint16	length; 							 // 命令帧有效数据个数
}CAN_CMD_FRAME;

// 一级接收缓冲区(存放上层协议数据)
typedef struct
{
	volatile uint8	buff[CAN_RX_FIFO_SIZE];
	volatile uint16	head;
	volatile uint16	end;
	uint16 currentProcessIndex;					// 当前待处理的字节的位置下标
}CAN_RX_FIFO;

// 接收帧缓冲区数据结构(解析成标准CAN消息，并进行命令解析)
typedef struct{
	CAN_CMD_FRAME	cmdQueue[CAN_RX_QUEUE_SIZE];
	uint16		head;							// 队列头索引
	uint16		end;							// 队列尾索引
}CAN_RX;


// CAN控制结构体定义
typedef struct
{
	// 一级接收缓冲区(存放上层协议数据)
	struct{
		CAN_RX_FIFO rxFIFOEachNode[CAN_DEVICE_SOURCE_ID_MAX];
	}rxFIFO;
	
	
	// 接收帧缓冲区数据结构(解析成标准CAN消息，并进行命令解析)
	struct{
		CAN_RX rxEachNode[CAN_DEVICE_SOURCE_ID_MAX];
	}rx;


	// 发送帧缓冲区数据结构(解析成标准CAN消息)
	struct{
		CAN_CMD_FRAME	cmdQueue[CAN_TX_QUEUE_SIZE];
		volatile uint16 head;						// 队列头索引
		volatile uint16 end; 						// 队列尾索引
		volatile uint16 index;						// 当前待发送数据在命令帧中的索引号
		volatile BOOL txBusy;						// 发送请求
	}tx;

}CAN_CB;

extern CAN_CB sysCanCB;

/**********************************************************************************************************/


/** @  *****************************************************************************************************
  * 【CAN发送状态机】 
  *  Can 高级应用层协议定义，驱动程序员根据协议修改
  * @{	
  **********************************************************************************************************/

// CAN发送系统状态定义
typedef enum
{
	CAN_TX_STATE_NULL = 0,									// 空状态
	
	CAN_TX_STATE_ENTRY,										// 入口状态 
	CAN_TX_STATE_STANDBY,									// 待机状态
	CAN_TX_STATE_FILL_SEND, 								// Tx填充与发送
	CAN_TX_STATE_SENDING,									// Tx发送中
	CAN_TX_STATE_ERR_PROCESS,								// Tx错误处理
	CAN_TX_STATE_ERR_RESCOVRY,								// Tx错误修复
	CAN_TX_STATE_ERR_ALARM, 								// Tx错误报警
	
	CAN_TX_STATE_MAX,										// Tx状态数  
	
}CAN_TX_STATE_E;

typedef struct
{
	CAN_TX_STATE_E state;									// 当前系统状态
	CAN_TX_STATE_E preState;								// 上一个状态	
	uint8 canTxErrNum;										// CAN消息发送错误计数器
	uint8 retry;											// CAN消息重发
	uint8 retrySnd; 										// 修复错误后，CAN消息第二次重发

	uint8 canIntFlag;										// CAN消息中断标志位
	uint8 canErrFlag;										// CAN消息错误标志位

}CAN_TX_STATE_CB;

extern CAN_TX_STATE_CB canTxStateCB;

//=====外部调用函数申明=======================================================================
// 模块初始化接口
void IAP_CTRL_CAN_Init(uint16 bund);

void CAN_HW_Init(uint16 bund);

// CAN处理过程函数，在 main 中大循环内调用
void CAN_PROTOCOL_Process_DT(void);
	
// 向发送缓冲区中添加数据
void IAP_CTRL_CAN_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void IAP_CTRL_CAN_TxAddFrame(void);

//=============================================================================================
// 工程模式启用
void IAP_CTRL_CAN_SendCmdProjectApply(uint8 *param);

// 向发送缓冲区中添加一条待发送序列
BOOL CAN_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length);

void IAP_CTRL_CAN_SendCmdNoAck(uint8 param);

// 发送升级数据
void IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_CMD cmd, uint32 falshAddr, uint32 addr);

// CAN 报文接收处理函数(注意根据具体模块修改)
extern void CAN_PROTOCOL_MacProcess(uint32 standarID, uint8 *pData, uint8 length);

#endif


