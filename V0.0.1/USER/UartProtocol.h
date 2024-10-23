/********************************************************************************************************
		KM5S与控制器通讯协议
********************************************************************************************************/
#ifndef 	__UART_PROTOCOL_H__
#define 	__UART_PROTOCOL_H__

#include "common.h"

#define SMOOTH_BASE_TIME							100		// 平滑基准时间，单位:ms

//==========与协议强相关，根据不同协议进行修改======================================
// UART总线通讯故障时间
#define UART_PROTOCOL_COMMUNICATION_TIME_OUT		10000UL	// 单位:ms

// P档位
#define PROTOCOL_ASSIST_P							ASSIST_P

#define UART_PROTOCOL_CMD_SEND_TIME					500		// 命令发送周期

//=====================================================================================================
#define UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// 通讯单向接收超时检测功能开关:0禁止，1使能

#define UART_PROTOCOL_RX_QUEUE_SIZE					5		// 接收命令队列尺寸
#define UART_PROTOCOL_TX_QUEUE_SIZE					5		// 发送命令队列尺寸

#define UART_PROTOCOL_CMD_HEAD1						0x55		// 命令头
#define UART_PROTOCOL_CMD_HEAD2						0x00		// 命令头1
#define UART_PROTOCOL_CMD_HEAD3						0xAA		// 命令头2

#define UART_PROTOCOL_CMD_PROTOCOL_VERSION			0x01	// 协议版本
#define UART_PROTOCOL_CMD_DEVICE_ADDR				0x10	// 设备号
#define UART_PROTOCOL_CMD_NONHEAD 					0xFF	// 非命令头
#define UART_PROTOCOL_HEAD_BYTE 					3	// 命令头字节数
#define UART_PROTOCOL_CHECK_BYTE 					1	// 校验字节数

#define UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		160		// 最大发送命令帧长度
#define UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		160		// 最大接收命令帧长度

#define UART_PROTOCOL_RX_FIFO_SIZE					200		// 接收一级缓冲区大小
#define UART_PROTOCOL_CMD_FRAME_LENGTH_MIN			6		// 命令帧最小长度，包含:5个命令头、协议版本、设备号、命令字、2个数据长度、校验码


// 通讯超时时间，根据波特率和命令长度确定，同时留足余量，为2倍
#define UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// 单位:ms
#define UART_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// 单位:ms

// UART发送协议定义	
typedef enum
{
	UART_PROTOCOL_CMD_HEAD1_INDEX = 0,					// 帧头1索引
	UART_PROTOCOL_CMD_HEAD2_INDEX,						// 帧头2索引
	UART_PROTOCOL_CMD_HEAD3_INDEX,						// 帧头3索引	
	UART_PROTOCOL_CMD_INDEX,                            /* 命令字索引 */
	UART_PROTOCOL_CMD_LENGTH_INDEX,                     /* 数据长度 */
	UART_PROTOCOL_CMD_BOARD_INDEX,	                    /* 目标板卡号 */

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

	UART_PROTOCOL_CMD_INDEX_MAX
}UART_PROTOCOL_DATE_FRAME;


 /*单板编号*/
typedef enum
{		
	BOARD_ARM =0X00,   /*ARM板编号*/
	BOARD_MOM =0X01,   /*母版板编号*/
	BOARD_COM,         /*接口板板编号*/
	BOARD_PRO,         /*烧录卡板编号*/
	BOARD_AVO,         /*万用表板编号*/
	BOARD_MAX     
}UART_PROTOCOL_BDNUM;

// 被控命令
typedef enum{	
	UART_CMD_NULL        = 0xFF,
	UART_CMD_CALL        = 0x01,						 // 查询板卡信息，上下行一样
	UART_CMD_POWERROR    = 0x02,		                 // 检测到异常掉电
	UART_CMD_RES         = 0x03, 						 // 复位，上下行一样
	UART_CMD_GO_APP      = 0x04, 						 // 进入APP，上下行一样	
	UART_CMD_UPGRADE     = 0x05, 						 // 升级固件，上下行一样
	UART_CMD_WRITE_DATA  = 0x06, 					     // 写入数据，上下行一样
    //母版指令开始
    UART_CMD_JS_CTRLKEY  = 0x20,                         //JS脚本解析器控制按键命令
    UART_CMD_POWER_CTRL  = 0x21,                         //电源输出口控制命令
    UART_CMD_POWER_ADJ   = 0X22,                         //可调升压电源控制命令
    UART_CMD_ADJVOL_READ = 0X23,                         //母版可调电源当前电压读取命令
	UART_CMD_ADJCUR_READ = 0X24,                         //母版可调电源当前电流读取命令
	UART_CMD_MOM_DOWM_SWITCH = 0X25,                     //烧录开关控制
	//烧录卡指令
	UART_CMD_DOWN_SEND_DOWNINFO = 0x80,                  // 烧录卡传输配置指令
    UART_CMD_DOWN_SEND_DOWNINFO_ACK = 0x80,              // 烧录卡传输配置指令应答	
	UART_CMD_DOWN_SEND_DOWNFILE = 0x81,                  // 烧录卡传输文件指令
    UART_CMD_DOWN_SEND_DOWNFILE_ACK = 0x81,              // 烧录卡传输文件指令应答	
	UART_CMD_DOWN_SEND_DOWNFILE_FINISH = 0x82,           // 烧录卡传输文件完毕
    UART_CMD_DOWN_SEND_DOWNFILE_FINISH_ACK = 0x82,       // 烧录卡传输文件应答	
	UART_CMD_DOWN_START_DOWNLOARD = 0x83,                // 烧录卡烧录目标板指令
    UART_CMD_DOWN_START_DOWNLOARD_ACK = 0x83,            // 烧录卡烧录目标板应答
	
	//万用表板卡命令
	UART_CMD_AVOMETER = 0xb0,
	UART_CMD_AVOMETER_ACK = 0xb0,
	// 万用表复位命令
	UART_CMD_AVORESET = 0xb1,
	UART_CMD_AVORESET_ACK = 0xb1,
	
	//广播命令
	//广播复位命令
    UART_CMD_RESET_SOURCE  =0xc0,                               //板子所有资源都恢复到初始状态
	UART_CMD_ALL_GO_APP    =0xc1,                               //广播命令，所有板子进入APP	
	// 表头档位通信命令
	//UART_CMD_MEMET_CAPTURE_GET_GEARS = 0x50, 			// 查询表头档位命令
	//UART_CMD_MEMET_CAPTURE_GET_GEARS_ACK = 0x50,			// 查询表头档位命令应答
	
	UART_PROTOCOL_CMD_MAX									// 总命令数
}UART_PROTOCOL_CMD;


 /*单板编号*/
typedef enum
{		
	MEASURE_DCV   =1,  //直流电压（V）
    MEASURE_ACV,       //交流电压（V）
	MEASURE_OHM,       //电阻（Ω）
	MEASURE_DCMA,      //直流电流（mA）
	MEASURE_ACMA,      //交流电流（mA）
	MEASURE_DCA,       //直流电流（A）
	MEASURE_ACA,       //交流电流（A）		
}AVO_MEASURE_TYPE;



//浮点数结构体
typedef union
{
   uint8_t data[4];
   float fl;
}FLOAT_PARA;


//整形数结构体
typedef union
{
   uint8_t data[4];
   int INT;
}INT_PARA;





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

	// 平滑算法数据结构
	struct{
		uint16 realSpeed;
		uint16 proSpeed;
		uint16 difSpeed;
	}speedFilter;
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

//==================================================================================
// 设置下发请求标志，按键设置参数也是调用此接口
void UART_PROTOCOL_SetTxAtOnceRequest(uint32 param);


void UART_PROTOCOL_SendCmdLedAck(void);
void UART_LED_SetControlMode(const uint8* buff);

// 发送命令字
void UART_PROTOCOL_SendCmd(uint32 sndCmd);

// 发送ECO请求
void UART_PROTOCOL_SendCmdEco(uint32 param);

// 读取FLASH回复
void UART_PROTOCOL_SendCmdReadFlagCmd(uint32* pBuf);

// 发送卡ID
void UART_PROTOCOL_SendCardId(uint8 cardStyle, uint8 cardIdLen, uint8 *cardId);

// 设备上报函数
void APP_ReportMsg(uint8_t cmd, uint8_t *msg, uint8_t length);
static bool UART_CheckSUM_(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

void Uart2_CmdAskVerACK(void);
#endif


