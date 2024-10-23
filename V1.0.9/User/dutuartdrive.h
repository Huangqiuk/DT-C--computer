#ifndef 	__UART_DRIVE_H__
#define 	__UART_DRIVE_H__

/*******************************************************************************
 *                                  头文件	                                   *
********************************************************************************/
#include "common.h"

//============================================================
#define UART_BAUD_RATE_1200         1200
#define UART_BAUD_RATE_2400         2400
#define UART_BAUD_RATE_4800         4800
#define UART_BAUD_RATE_9600         9600
#define UART_BAUD_RATE_14400        14400
#define UART_BAUD_RATE_19200        19200
#define UART_BAUD_RATE_38400        38400
#define UART_BAUD_RATE_43000        43000
#define UART_BAUD_RATE_57600        57600
#define UART_BAUD_RATE_76800        76800
#define UART_BAUD_RATE_115200       115200
#define UART_BAUD_RATE_128000       128000

/*******************************************************************************
 *                                  宏定义	                                   *
********************************************************************************/
#define UART_DRIVE_TX_MODE						UART_DRIVE_INTERRUPT_TX_MODE	// 选择阻塞发送还是中断发送数据
#define UART_DRIVE_TYPE_DEF						UART3							// 选择串口(串口中断和时钟都需要修改)
#define UART_DRIVE_IRQn_DEF						UART3_IRQn 					// 选择串口(串口中断和时钟都需要修改)

#define _5V_CHANGE_OFF() gpio_bit_reset(GPIOC, GPIO_PIN_8)
#define _5V_CHANGE_ON() gpio_bit_set(GPIOC, GPIO_PIN_8)
#define VCCB_EN_OFF() gpio_bit_reset(GPIOC, GPIO_PIN_9)
#define VCCB_EN_ON() gpio_bit_set(GPIOC, GPIO_PIN_9)

#define UART_DRIVE_BAUD_RATE					115200	// 通讯波特率

#define UART_DRIVE_TX_QUEUE_SIZE				150		// 驱动层发送命令帧尺寸
#define UART_DRIVE_CMD_FRAME_LENGTH_MAX			150		// 命令帧最大长度

// ■■■■■■■ 以下部分不能随意修改 ■■■■■■■■■■■■■■■
#define UART_DRIVE_BLOCKING_TX_MODE				0U
#define UART_DRIVE_INTERRUPT_TX_MODE			1U

// UART命令帧定义
typedef struct
{
	volatile uint16	deviceID;
	volatile uint8	buff[UART_DRIVE_CMD_FRAME_LENGTH_MAX];	// 命令帧缓冲区 
	volatile uint16	length;									// 命令帧有效数据个数
}UART_DRIVE_CMD_FRAME;

// UART控制结构体定义
typedef struct
{
	// 接收数据抛出接口
	void (*receiveDataThrowService)(uint16 id, uint8 *pData, uint16 length);
	
	// 发送帧缓冲区数据结构
	struct{
		UART_DRIVE_CMD_FRAME cmdQueue[UART_DRIVE_TX_QUEUE_SIZE];
		volatile uint16 head;						// 队列头索引
		volatile uint16 end; 						// 队列尾索引
		volatile uint16 index;						// 当前待发送数据在命令帧中的索引号
		volatile BOOL txBusy;						// 发送请求
	}tx;
}UART_DRIVE_CB;

extern UART_DRIVE_CB uartDriveCB;

/******************************************************************************
* 【外部接口声明】
******************************************************************************/
// UART初始化，在main的大循环之前调用，完成模块初始化
void UART_DRIVE_Init(void);

void UART_DRIVE_InitSelect(uint32 baud);

// UART模块处理入口，在main的大循环中调用
void UART_DRIVE_Process(void);

// 注册数据抛出接口服务
void UART_DRIVE_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length));

void CHANGE_Init(void);

// 向发送缓冲区中添加一条待发送序列
BOOL UART_DRIVE_AddTxArray(uint16 id, uint8 *pArray, uint16 length);

#endif


