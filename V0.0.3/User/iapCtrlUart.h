#ifndef 	__IAP_CTRL_UART_H__
#define 	__IAP_CTRL_UART_H__

#include "common.h"

#define IAP_CTRL_UART_TX_MODE					IAP_CTRL_INTERRUPT_TX_MODE		// 选择阻塞发送还是中断发送数据
#define IAP_CTRL_UART_TYPE_DEF					USART1							// 选择串口(串口中断和时钟都需要修改)
#define IAP_CTRL_UART_IRQn_DEF					USART1_IRQn 					// 选择串口(串口中断和时钟都需要修改)

#define IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE		0		// 通讯单向接收超时检测功能开关:0禁止，1使能
#define IAP_CTRL_UART_TXRX_TIME_OUT_CHECK_ENABLE	0		// 通讯双向接收超时检测功能开关:0禁止，1使能

#define IAP_CTRL_UART_BAUD_RATE					115200	// 通讯波特率

#define IAP_CTRL_UART_RX_QUEUE_SIZE				5		// 接收命令队列尺寸
#define IAP_CTRL_UART_TX_QUEUE_SIZE				5		// 发送命令队列尺寸

#define IAP_CTRL_UART_CMD_LENGTH_MAX 			160		// 最大命令长度
#define IAP_CTRL_UART_RX_FIFO_SIZE				250		// 接收一级缓冲区大小

#define IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN		4		// 命令帧最小长度，包含:命令头、命令字、数据长度、校验码

#define IAP_CTRL_UART_CMD_HEAD					0x55	// 命令头

#define UART_PROTOCOL_MCU_UID_BUFF_LENGTH			12		// SN号长度

// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define IAP_CTRL_UART_BUS_UNIDIRECTIONAL_TIME_OUT		(uint32)((1000.0/IAP_CTRL_UART_BAUD_RATE*10*IAP_CTRL_UART_CMD_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// 单位:ms
#define IAP_CTRL_UART_BUS_BIDIRECTIONAL_TIME_OUT		(uint32)(((1000.0/IAP_CTRL_UART_BAUD_RATE*10*IAP_CTRL_UART_CMD_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// 单位:ms

// ■■■■■■■ 以下部分不能随意修改 ■■■■■■■■■■■■■■■
#define IAP_CTRL_BLOCKING_TX_MODE				0U
#define IAP_CTRL_INTERRUPT_TX_MODE				1U

// UART发送协议定义	
typedef enum
{
	IAP_CTRL_UART_CMD_HEAD_INDEX = 0,					// 帧头索引
	IAP_CTRL_UART_CMD_CMD_INDEX, 						// 命令字索引
	IAP_CTRL_UART_CMD_LENGTH_INDEX,						// 数据长度

	IAP_CTRL_UART_CMD_DATA1_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA2_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA3_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA4_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA5_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA6_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA7_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA8_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA9_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA10_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA11_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA12_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA13_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA14_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA15_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA16_INDEX,						// 数据索引
	IAP_CTRL_UART_CMD_DATA17_INDEX,						// 数据索引

	IAP_CTRL_UART_CMD_INDEX_MAX
}IAP_CTRL_UART_DATE_FRAME;


// 串口命令
typedef enum{
	IAP_CTRL_UART_CMD_NULL = 0,									// 复位命令

	IAP_CTRL_UART_CMD_UP_PROJECT_APPLY = 0x01,					// 工程模式启用申请
	IAP_CTRL_UART_CMD_DOWN_PROJECT_APPLY_ACK = 0x02,			// 工程模式批准应答 
	IAP_CTRL_UART_CMD_UP_PROJECT_READY = 0x03,					// 工程模式准备就绪

	IAP_CTRL_UART_CMD_DOWN_FACTORY_RESET = 0x04,				// 恢复出厂设置
	IAP_CTRL_UART_CMD_UP_FACTORY_RESET_ACK = 0x05,				// 恢复出厂设置应答

	IAP_CTRL_UART_CMD_DOWN_RTC_SET = 0x06,						// 设置RTC
	IAP_CTRL_UART_CMD_UP_RTC_SET_RESULT = 0x07,					// 设置RTC结果
	UART_ECO_CMD_ECO_MCU_UID = 0x08,										// 比较MCU_UID

	IAP_CTRL_UART_CMD_WRITE_SYS_PARAM = 0x10,					// 系统参数写入
	IAP_CTRL_UART_CMD_READ_SYS_PARAM = 0x11,					// 系统参数读取

	IAP_CTRL_UART_CMD_DOWN_UI_DATA_ERASE = 0x12,				// UI数据擦除
	IAP_CTRL_UART_CMD_UP_UI_DATA_ERASE_ACK = 0x13,				// UI数据擦除结果

	IAP_CTRL_UART_CMD_DOWN_UI_DATA_WRITE = 0x14,				// UI数据写入
	IAP_CTRL_UART_CMD_UP_UI_DATA_WRITE_RESULT = 0x15,			// UI数据写入结果

	IAP_CTRL_UART_CMD_DOWN_IAP_ERASE_FLASH = 0x24, 				// 擦除APP_FLASH命令
	IAP_CTRL_UART_CMD_UP_IAP_ERASE_FLASH_RESULT = 0x25, 		// 擦除APP_FLASH结果上报命令
	IAP_CTRL_UART_CMD_DOWN_IAP_WRITE_FLASH = 0x26, 				// IAP数据写入
	IAP_CTRL_UART_CMD_UP_IAP_WRITE_FLASH_RESULT = 0x27, 		// IAP数据写入结果上报
	IAP_CTRL_UART_CMD_DOWN_CHECK_FALSH_BLANK = 0x28, 			// 查空
	IAP_CTRL_UART_CMD_UP_CHECK_FLASH_BLANK_RESULT = 0x29, 		// 查空结果上报
	IAP_CTRL_UART_CMD_DOWN_UPDATA_FINISH = 0x2A, 				// 系统升级结束
	IAP_CTRL_UART_CMD_UP_UPDATA_FINISH_RESULT = 0x2B, 			// 系统升级结束确认

	IAP_CTRL_UART_CMD_DOWN_START_QR_REGISTER = 0x40, 			// 启动二维码注册
	IAP_CTRL_UART_CMD_UP_QR_PARAM_REPORT = 0x41, 				// 二维码注册参数上报
	IAP_CTRL_UART_CMD_DOWN_QR_DATA_WRITE = 0x42,				// 写入二维码数据
	IAP_CTRL_UART_CMD_UP_QR_DATA_WRITE_RESULT = 0x43,			// 保存二维码数据结果上报

	IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ERESE = 0x60,				// SPI 绝对地址擦除
	IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_WRITE = 0x61,				// SPI 绝对地址写入
	IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_READ = 0x62,				// SPI 绝对地址读取
	IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ER_WR = 0x63,				// SPI 绝对地址擦写
	
	IAP_CTRL_UART_CMD_SPI_FLASH_UI_ERESE = 0x70, 					// SPI UI 擦除
	IAP_CTRL_UART_CMD_SPI_FLASH_UI_WRITE = 0x71,					// SPI UI 写入
	IAP_CTRL_UART_CMD_SPI_FLASH_UI_FINISH = 0x72,					// SPI UI 写入完成
	
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_ERESE = 0x73, 			// SPI 出厂APP 擦除
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_WRITE = 0x74,			// SPI 出厂APP 写入
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_FINISH = 0x75,			// SPI 出厂APP 写入完成
	
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_ERESE = 0x76, 			// SPI 升级APP 擦除
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_WRITE = 0x77,			// SPI 升级APP 写入
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_FINISH = 0x78,			// SPI 升级APP 写入完成
	
	IAP_CTRL_UART_CMD_SPI_FLASH_WRITE_SPI_TO_MCU = 0x79, 			// 镜像写入

	IAP_CTRL_UART_CMD_JUMP_TO_BOOT = 0x80, 							// 复位命令
	IAP_CTRL_UART_CMD_JUMP_TO_APP1 = 0x81, 							// 串口命令强制控制BOOT跳转到APP1
	IAP_CTRL_UART_CMD_JUMP_TO_APP2 = 0x82, 							// 串口命令强制控制BOOT跳转到APP2

	IAP_CTRL_UART_CMD_CHECK_VERSION = 0x90,							// 版本核对命令
	
	IAP_CTRL_UART_CMD_FLAG_ARRAY_READ = 0x91,						// 读标志区数据
	IAP_CTRL_UART_CMD_FLAG_ARRAY_WRITE = 0x92,						// 写标志区数据

	IAP_CTRL_UART_CMD_VERSION_TYPE_WRITE = 0x93,					// 写入版本信息
	IAP_CTRL_UART_CMD_VERSION_TYPE_READ = 0x94,						// 读取版本信息

	IAP_CTRL_UART_CMD_KEY_TESTING = 0x95,							// 按键测试命令
	IAP_CTRL_UART_CMD_TEST_LCD = 0x96,								// LCD颜色显示控制
	

	IAP_CTRL_UART_CMD_MAX											// 总命令数
}IAP_CTRL_UART_CMD;

// 版本核对枚举
typedef enum{
	IAP_CTRL_UART_CHECK_VERSION_ALL,

	IAP_CTRL_UART_CHECK_VERSION_APP,
	IAP_CTRL_UART_CHECK_VERSION_BOOT,
	IAP_CTRL_UART_CHECK_VERSION_UI,
	IAP_CTRL_UART_CHECK_VERSION_HW,
	IAP_CTRL_UART_CHECK_VERSION_SN_NUM,
	IAP_CTRL_UART_CHECK_VERSION_BLE_MAC,
	IAP_CTRL_UART_CHECK_VERSION_QR_CODE,
}IAP_CTRL_UART_CHECK_VERSION_E;

// UART命令帧定义
typedef struct
{
	uint8	buff[IAP_CTRL_UART_CMD_LENGTH_MAX];	// 命令帧缓冲区
	uint16	length; 						// 命令帧有效数据个数
}IAP_CTRL_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
	// 一级接收缓冲区
	struct
	{
		volatile uint8	buff[IAP_CTRL_UART_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;					// 当前待处理的字节的位置下标
	}rxFIFO;
	
	// 接收帧缓冲区数据结构
	struct{
		IAP_CTRL_CMD_FRAME	cmdQueue[IAP_CTRL_UART_RX_QUEUE_SIZE];
		uint16			head;						// 队列头索引
		uint16			end;						// 队列尾索引
	}rx;

	// 发送帧缓冲区数据结构
	struct{
		IAP_CTRL_CMD_FRAME	cmdQueue[IAP_CTRL_UART_TX_QUEUE_SIZE];
		volatile uint16	head;						// 队列头索引
		volatile uint16	end;						// 队列尾索引
		volatile uint16	index;						// 当前待发送数据在命令帧中的索引号
		volatile BOOL	txBusy;						// 发送请求，为TRUE时，从待发送队列中取出一个BYTE放入发送寄存器
	}tx;
	
	uint32 ageFlag;
}IAP_CTRL_UART_CB;

extern IAP_CTRL_UART_CB iapCtrlUartCB;		

/******************************************************************************
* 【外部接口声明】
******************************************************************************/
// UART初始化，在main的大循环之前调用，完成模块初始化
void IAP_CTRL_UART_Init(void);

// UART模块处理入口，在main的大循环中调用
void IAP_CTRL_UART_Process(void);

// 向发送缓冲区中添加数据
void IAP_CTRL_UART_TxAddData(IAP_CTRL_UART_CB* pCB, uint8 data);

// 启动发送，函数会自动校正发送命令帧的数据长度，并计算和添加校验码
void IAP_CTRL_UART_TxAddFrame(IAP_CTRL_UART_CB* pCB);

// 启动阻塞发送
void IAP_CTRL_UART_BC_StartTx(IAP_CTRL_UART_CB* pCB);

// 通讯超时处理-双向
void IAP_CTRL_UART_CALLBACK_TxRxTimeOut(uint32 param);

//==============================================================================
// 工程模式启用
void IAP_CTRL_UART_SendCmdProjectApply(uint32 para);

// 二维码注册参数上报
void IAP_CTRL_UART_SendCmdQrRegParam(uint8 *param);

// 发送命令无结果
void IAP_CTRL_UART_SendVersionCheck(uint32 checkNum);

// 串口显示控制
uint8 IAP_CTRL_UART_LcdControl(uint8 lcdCmd);

// 发送命令带结果
void IAP_CTRL_UART_SendCmdWithResult(uint8 cmdWord, uint8 result);

#endif


