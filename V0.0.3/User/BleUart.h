#ifndef __BLE_UART_H__
#define __BLE_UART_H__

#include "common.h"

#define BLE_UART_TX_MODE					BLE_UART_INTERRUPT_TX_MODE			// 选择阻塞发送还是中断发送数据
#define BLE_UART_TYPE_DEF					USART0								// 选择串口(串口中断和时钟都需要修改)
#define BLE_UART_IRQn_DEF					USART0_IRQn 						// 选择串口(串口中断和时钟都需要修改)
#define BLE_UART_BAUD_RATE					9600								// 通讯波特率

#define BLE_RST_RESET() 					gpio_bit_reset(GPIOA, GPIO_PIN_12)
#define BLE_RST_RELEASE()					gpio_bit_set(GPIOA, GPIO_PIN_12)
#define BLE_EN_ENABLE() 					gpio_bit_reset(GPIOA, GPIO_PIN_11)		// 开始广播
#define BLE_EN_DISABLE()					gpio_bit_set(GPIOA, GPIO_PIN_11)		// 完全睡眠状态

#define BLE_BRTS_TX_REQUEST() 		
#define BLE_BRTS_TX_RELEASE()				gpio_bit_set(GPIOC, GPIO_PIN_12)

// ■■■■■■■ 以下部分不能随意修改 ■■■■■■■■■■■■■■■
// 中断方式
#define BLE_UART_BLOCKING_TX_MODE							0U
#define BLE_UART_INTERRUPT_TX_MODE							1U
// 尺寸
#define BLE_UART_DRIVE_TX_QUEUE_SIZE						3			// 驱动层发送命令帧尺寸
#define BLE_UART_DRIVE_CMD_FRAME_LENGTH_MAX					150			// 命令帧最大长度

// UART命令帧定义
typedef struct
{
	volatile uint16	deviceID;
	volatile uint8	buff[BLE_UART_DRIVE_CMD_FRAME_LENGTH_MAX];	// 命令帧缓冲区 
	volatile uint16	length;										// 命令帧有效数据个数
}BLE_UART_DRIVE_CMD_FRAME;


// UART控制结构体定义
typedef struct
{
	// 接收数据抛出接口
	void (*receiveDataThrowService)(uint16 id, uint8 *pData, uint16 length);
	
	// 发送帧缓冲区数据结构
	struct{
		BLE_UART_DRIVE_CMD_FRAME cmdQueue[BLE_UART_DRIVE_TX_QUEUE_SIZE];
		volatile uint16 head;						// 队列头索引
		volatile uint16 end; 						// 队列尾索引
		volatile uint16 index;						// 当前待发送数据在命令帧中的索引号
		volatile BOOL txBusy;						// 发送请求
	}tx;
	
	// 接收
	struct
	{
		BOOL startFlag;
		
		uint8 fifoBuff[50];
		uint8 macBuff[20];
		uint8 index;
	}rx;

	BOOL rcvBleMacOK;

}BLE_UART_CB;

extern BLE_UART_CB bleUartCB;		// UART数据结构

/******************************************************************************
* 【外部接口声明】
******************************************************************************/

// UART初始化
void BLE_UART_Init(void);

// UART模块处理入口
void BLE_UART_Process(void);

// 发送获取MAC地址
void BLE_UART_SendCmdGetBleMacAddr(uint32 param);

// 定时器回调发送获取蓝牙地址命令
void BLE_UART_CALLBALL_GetBleMacAddrRequest(uint32 param);

// 注册数据抛出接口服务
void BLE_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length));

// 向发送缓冲区中添加一条待发送序列
BOOL BLE_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length);

// 蓝牙模块复位
void BLE_MODULE_Reset(uint32 param);

#endif

