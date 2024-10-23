#ifndef __GSM_UART_H__
#define __GSM_UART_H__

#define GSM_UART_TX_QUERY_ICCID_TIME		200		// 发送查询ICCID时间间隔

#define GSM_UART_TYPE_DEF					UART3					 // 选择串口(串口中断和时钟都需要修改)
#define GSM_UART_IRQn_DEF					UART3_IRQn               // 选择串口(串口中断和时钟都需要修改)

#define GSM_UART_RX_TIME_OUT_CHECK_ENABLE	1		// 通讯超时检测功能开关:0禁止，1使能
#define GSM_UART_BAUD_RATE_CONTROL			115200	// 主控器与电机驱动器之间通讯波特率

#define GSM_DTR_HIGH()						gpio_bit_write(GPIOB, GPIO_PIN_8, SET)
#define GSM_DTR_LOW()						gpio_bit_write(GPIOB, GPIO_PIN_8, RESET)
#define GSM_ONOFF_HIGH()					gpio_bit_write(GPIOB, GPIO_PIN_9, RESET)	// 接了NPN三极管，因此需要反驱动
#define GSM_ONOFF_LOW()						gpio_bit_write(GPIOB, GPIO_PIN_9, SET)


// UART控制结构体定义
typedef struct
{
	// 接收帧缓冲区数据结构
	struct{	
		uint8 fifoBuff[50];
		uint8 iccidBuff[21];
		uint8 index;
	}rx;

	struct{	
		BOOL state;
	}power;

	BOOL rcvIccidOK;
	BOOL rcvIccidTimeOut;

	uint8 rcvIccidTimeCnt;

}GSM_UART_CB;

extern GSM_UART_CB gsmUartCB;		


//==============================================================================================================
/******************************************************************************
* 【外部接口声明】
******************************************************************************/
// UART初始化，在main的大循环之前调用，完成模块初始化
void GSM_UART_Init(void);

// UART模块处理入口，在main的大循环中调用
void GSM_UART_Process(void);

// 发送获取sim的ICCID
void GSM_UART_SendCmdGetICCID(uint32 param);

// 定时器回调发送获取sim卡ICCID命令
void GSM_UART_CALLBALL_GetSimIccidRequest(uint32 param);

// gsm模块开关机控制
void GSM_UART_GsmPowerOnOff(uint32 param);


#endif



