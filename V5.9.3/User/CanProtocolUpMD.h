#ifndef 	__CAN_MD_PROTOCOL_H__
#define 	__CAN_MD_PROTOCOL_H__

#include "common.h"
#include "DutInfo.h"

//==========与协议强相关，根据不同协议进行修改======================================
// UART总线通讯故障时间
#define CAN_MD_PROTOCOL_COMMUNICATION_TIME_OUT		10000UL	// 单位:ms

//=====================================================================================================
#define CAN_MD_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// 通讯单向接收超时检测功能开关:0禁止，1使能

#define CAN_MD_PROTOCOL_RX_QUEUE_SIZE					3		// 接收命令队列尺寸
#define CAN_MD_PROTOCOL_TX_QUEUE_SIZE					3		// 发送命令队列尺寸

#define CAN_MD_PROTOCOL_CMD_HEAD						0x55	// 命令头
#define CAN_MD_PROTOCOL_CMD_HEAD2						0xAA	// 命令头

#define CAN_MD_PROTOCOL_CMD_TAIL			       	    0xF0	// 帧尾
#define CAN_MD_PROTOCOL_CMD_NONHEAD 					0xFF	// 非命令头
#define CAN_MD_PROTOCOL_CMD_CHECK_BYTE_SIZE 			0x04	// 校验码所占字节数
#define CAN_MD_PROTOCOL_CMD_TAIL_BYTE 					0x01	// 帧尾字节数

#define CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		160		// 最大发送命令帧长度
#define CAN_MD_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		160		// 最大接收命令帧长度
#define CAN_MD_PROTOCOL_RX_FIFO_SIZE					180		// 接收一级缓冲区大小
#define CAN_MD_PROTOCOL_CMD_FRAME_LENGTH_MIN			9		// 命令帧最小长度，包含:起始符、命令字、数据长度、校验和


// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define CAN_MD_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// 单位:ms
#define CAN_MD_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// 单位:ms

// CAN发送协议定义	
typedef enum
{
	CAN_MD_PROTOCOL_CMD_HEAD_INDEX = 0,					    // 帧头索引
	CAN_MD_PROTOCOL_CMD_HEAD_INDEX2,				        // 帧头索引 
	CAN_MD_PROTOCOL_CMD_MODE_INDEX, 						// 帧模式
	CAN_MD_PROTOCOL_CMD_LENGTH_INDEX,						// 数据长度
	CAN_MD_PROTOCOL_CMD_CMD_H_INDEX,						// 命令字索引
	CAN_MD_PROTOCOL_CMD_CMD_L_INDEX,						// 命令字索引
    
	CAN_MD_PROTOCOL_CMD_DATA1_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA2_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA3_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA4_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA5_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA6_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA7_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA8_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA9_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA10_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA11_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA12_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA13_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA14_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA15_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA16_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA17_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA18_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA19_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA20_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA21_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA22_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA23_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA24_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA25_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA26_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA27_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA28_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA29_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA30_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA31_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA32_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA33_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA34_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA35_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA36_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA37_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA38_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA39_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA40_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA41_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA42_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA43_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA44_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA45_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA46_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA47_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA48_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA49_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA50_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA51_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA52_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA53_INDEX,						// 数据索引
	CAN_MD_PROTOCOL_CMD_DATA54_INDEX,						// 数据索引

	CAN_MD_PROTOCOL_CMD_INDEX_MAX
}CAN_MD_PROTOCOL_DATE_FRAME;

// CAN总线节点编号
typedef enum{
	CAN_MD_PROTOCOL_MODE_READ = 0x11,					// 读模式 
	CAN_MD_PROTOCOL_MODE_WRITE = 0x16,					// 写模式 
	CAN_MD_PROTOCOL_MODE_ACK = 0x0C,					// 上报模式 

	CAN_MD_PROTOCOL_MODE_MAX							// 最大节点数
}CAN_MD_PROTOCOL_MODE;

// 被控命令
typedef enum{
	CAN_MD_PROTOCOL_CMD_NULL = 0,								// 空命令

	CAN_MD_PROTOCOL_CMD_UP_PROJECT_APPLY = 0xC109,						// 工程模式启用申请
	CAN_MD_PROTOCOL_CMD_DOWN_PROJECT_APPLY_ACK = 0xA10B,				// 工程模式批准应答 
	CAN_MD_PROTOCOL_CMD_UP_PROJECT_READY = 0xC402,						// 工程模式准备就绪
	
	CAN_MD_PROTOCOL_CMD_DOWN_IAP_WRITE_FLASH = 0xA385, 					// IAP数据写入
	CAN_MD_PROTOCOL_CMD_UP_IAP_WRITE_RIGHT_RESULT = 0xC202, 			// IAP数据写入结果上报,正确应答
	CAN_MD_PROTOCOL_CMD_UP_IAP_WRITE_ERROR_RESULT = 0xC302, 			// IAP数据写入结果上报，错误应答
	CAN_MD_PROTOCOL_CMD_DOWN_UPDATA_FINISH = 0xA401, 					// 系统升级结束
	CAN_MD_PROTOCOL_CMD_UP_UPDATA_FINISH_RESULT = 0xC502, 				// 系统升级结束确认	

	CAN_MD_PROTOCOL_CMD_MAX									// 总命令数
}CAN_MD_PROTOCOL_CMD;

// UART_RX命令帧定义
typedef struct
{
	uint16  deviceID;
	uint8	buff[CAN_MD_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];	// 命令帧缓冲区
	uint16	length; 										    // 命令帧有效数据个数
}CAN_MD_PROTOCOL_RX_CMD_FRAME;

// UART_TX命令帧定义
typedef struct
{
	uint16  deviceID;
	uint8	buff[CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];	// 命令帧缓冲区
	uint16	length; 										// 命令帧有效数据个数
}CAN_MD_PROTOCOL_TX_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
	// 一级接收缓冲区
	struct
	{
		volatile uint8	buff[CAN_MD_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
        volatile uint16	deviceID;
		uint16 currentProcessIndex;					// 当前待处理的字节的位置下标
	}rxFIFO;
	
	// 接收帧缓冲区数据结构
	struct{
		CAN_MD_PROTOCOL_RX_CMD_FRAME	cmdQueue[CAN_MD_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;						// 队列头索引
		uint16			end;						// 队列尾索引
	}rx;

	// 发送帧缓冲区数据结构
	struct{
		CAN_MD_PROTOCOL_TX_CMD_FRAME	cmdQueue[CAN_MD_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;						// 队列头索引
		uint16	end;						// 队列尾索引
		uint16	index;						// 当前待发送数据在命令帧中的索引号
		BOOL	txBusy;						// 发送请求，为TRUE时，从待发送队列中取出一个BYTE放入发送寄存器
	}tx;

	// 发送数据接口
	BOOL (*sendDataThrowService)(uint32 id, uint8 *pData, uint8 length);
	
}CAN_MD_PROTOCOL_CB;

extern CAN_MD_PROTOCOL_CB canMDProtocolCB;	

//=====外部调用函数申明=======================================================================
// 协议初始化
void CAN_MD_PROTOCOL_Init(void);

// CAN处理过程函数，在 main 中大循环内调用
void CAN_PROTOCOL_Process_DT(void);
	
// 向发送缓冲区中添加数据
void CAN_MD_PROTOCOL_TxAddData(uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void CAN_MD_PROTOCOL_TxAddFrame(void);

//============================================================================================

// 向发送缓冲区中添加一条待发送序列
BOOL CAN_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length);

// 发送命令带结果
void CAN_MD_PROTOCOL_SendCmdWithResult(uint16 cmdWord, uint8 result);

// 发送命令无结果
void CAN_MD_PROTOCOL_SendCmdNoResult(uint16 cmdWord);

// 发送升级数据
void CAN_MD_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_MD_PROTOCOL_CMD cmd, uint32 falshAddr, uint32 addr);

// UART报文接收处理函数(注意根据具体模块修改)
void CAN_MD_PROTOCOL_MacProcess(uint32 standarID, uint8* pData, uint16 length);

// UART协议层过程处理
void CAN_MD_PROTOCOL_Process(void);
#endif


