#include "common.h"
#include "system.h"
#include "timer.h"
#include "watchdog.h"
#include "delay.h"
#include "gsmUart.h"

// UART初始化
void GSM_UART_HwInit(uint32 baud);

// 发送处理
void GSM_UART_TxProcess(GSM_UART_CB* pCB);

// UART1接收与发送中断处理，用于内部通讯
void UART3_IRQHandler(void);

// 接收ICCID处理
void GSM_UART_RcvSimICCID(uint8 data);

void GSM_UART_GpioOnOffControl(uint32 param);

// 全局变量定义
GSM_UART_CB gsmUartCB;
//===========================================================================================


// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// UART初始化
void GSM_UART_Init(void)
{
	GSM_UART_HwInit(GSM_UART_BAUD_RATE_CONTROL);
}

// UART初始化
void GSM_UART_HwInit(uint32 baud)
{
	// 打开时钟
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
	
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_11);

	// UART时钟配置
	rcu_periph_clock_enable(RCU_UART3);
	usart_deinit(GSM_UART_TYPE_DEF);									// 复位串口
		
	// 串口配置
	usart_baudrate_set(GSM_UART_TYPE_DEF, baud);						// 波特率
	usart_word_length_set(GSM_UART_TYPE_DEF, USART_WL_8BIT);			// 8位数据位
	usart_stop_bit_set(GSM_UART_TYPE_DEF, USART_STB_1BIT); 				// 一个停止位
	usart_parity_config(GSM_UART_TYPE_DEF, USART_PM_NONE); 				// 无奇偶校验
	usart_hardware_flow_rts_config(GSM_UART_TYPE_DEF, USART_RTS_DISABLE); // 无硬件数据流控制
	usart_hardware_flow_cts_config(GSM_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(GSM_UART_TYPE_DEF, USART_TRANSMIT_ENABLE);	// 使能发射
	usart_receive_config(GSM_UART_TYPE_DEF, USART_RECEIVE_ENABLE); 		// 使能接收

	// 中断配置
	nvic_irq_enable(GSM_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(GSM_UART_TYPE_DEF, USART_INT_RBNE); 			// 开启中断

	usart_enable(GSM_UART_TYPE_DEF);									// 使能串口 

//====================================================================================================================
	// 打开时钟
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);
	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_9);

	// DTR唤醒
	GSM_DTR_LOW();

	// gsm模块上电默认关机
	GSM_ONOFF_HIGH();
}

// 阻塞发送一个字节数据
void GSM_UART_BC_SendData(uint8 data)
{	
	// 防止丢失第一个字节(清除空标志位,务必加) 
	usart_flag_get(GSM_UART_TYPE_DEF, USART_FLAG_TC);		

	// 填充数据
	usart_data_transmit(GSM_UART_TYPE_DEF, data);

	// 未发送完，持续等待
	while(usart_flag_get(GSM_UART_TYPE_DEF, USART_FLAG_TC) != SET); 		
}

/**************************************************************
 * @brief  void USART1_IRQHandler(void);
 * @input  串口1接收函数，串口1的中断发送函数
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
#if 0
void UART3_IRQHandler(void)
{
	// 判断DR是否有数据，中断接收
	if(usart_interrupt_flag_get(GSM_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET) 
	{
		uint8 rxData;
		
		// 接收数据
		rxData = (uint8)usart_data_receive(GSM_UART_TYPE_DEF);

		GSM_UART_RcvSimICCID(rxData);
	}

	// Other USART1 interrupts handler can go here ...				 
	if (usart_interrupt_flag_get(GSM_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- 接收溢出中断 
	{
		usart_flag_get(GSM_UART_TYPE_DEF, USART_FLAG_ORERR); 		 //----------------------- 清除接收溢出中断标志位 
		usart_data_receive(GSM_UART_TYPE_DEF);										 //----------------------- 清空寄存器
	}
	
}
#endif
// 发送获取sim的ICCID
void GSM_UART_SendCmdGetICCID(uint32 param)
{
	uint8 i;
	uint8 iccidStr[] = {'A','T','+','I','C','C','I','D','\r','\n','\0'};

	// 启动阻塞式发送命令
	for (i = 0; iccidStr[i] != '\0';)
	{
		// 填充数据，启动发送
		GSM_UART_BC_SendData(iccidStr[i++]);
	}
}

// 接收ICCID处理
void GSM_UART_RcvSimICCID(uint8 data)
{
	uint8 i;
	
	// 连续接收数据
	gsmUartCB.rx.fifoBuff[gsmUartCB.rx.index++] = data;

	// 越界判断
	if (gsmUartCB.rx.index >= sizeof(gsmUartCB.rx.fifoBuff))
	{
		gsmUartCB.rx.index = 0;

		return;
	}
	
	if (data == '\n')			// 收到结束标志
	{
		gsmUartCB.rx.index = 0;
		
		// 将读到的ICCID提取出来
		if (strstr((const char*)gsmUartCB.rx.fifoBuff, (const char*)"+ICCID: ") != NULL)
		{
			for (i = 0; i < 20; i++)
			{
				gsmUartCB.rx.iccidBuff[i] = gsmUartCB.rx.fifoBuff[i+8];
			}
			gsmUartCB.rx.iccidBuff[i] = '\0';

			// 关闭定时器
			TIMER_KillTask(TIMER_ID_GSM_CONTROL);
			
			// gsm模块开关机控制
			GSM_UART_GsmPowerOnOff(FALSE);

			// ICCID接收完成
			gsmUartCB.rcvIccidOK = TRUE;

			// 非超时所致
			gsmUartCB.rcvIccidTimeOut = FALSE;
		}		
	}
}

// 定时器回调发送获取sim卡ICCID命令
void GSM_UART_CALLBALL_GetSimIccidRequest(uint32 param)
{
	uint8 i;
	
	// 是否已经开机
	if (!gsmUartCB.power.state)
	{
		return;
	}
	
	// 发送获取sim的ICCID
	GSM_UART_SendCmdGetICCID(TRUE);

	// 由实验得知5秒内可以获取GSM模块的ICCID，超出此时间判断失败
	if (++gsmUartCB.rcvIccidTimeCnt > (5500/GSM_UART_TX_QUERY_ICCID_TIME))
	{
		gsmUartCB.rcvIccidTimeCnt = 0;
		gsmUartCB.rcvIccidTimeOut = TRUE;

		// ICCID接收失败
		gsmUartCB.rcvIccidOK = FALSE;

		// 关闭定时器
		TIMER_KillTask(TIMER_ID_GSM_CONTROL);

		for (i = 0; i < 20; i++)
		{
			gsmUartCB.rx.iccidBuff[i] = '0';
		}
		gsmUartCB.rx.iccidBuff[i] = '\0';
	}
}

// gsm模块开关机控制
void GSM_UART_GsmPowerOnOff(uint32 param)
{
	// 模块ON/OFF管脚一个持续时间1S以上的低电平模块即可开关机
	GSM_ONOFF_LOW();

	// 开启定时器，定时1秒即可
	TIMER_AddTask(TIMER_ID_GSM_POWER_CONTROL,
					1000,
					GSM_UART_GpioOnOffControl,
					param,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
}

void GSM_UART_GpioOnOffControl(uint32 param)
{
	// 拉高释放引脚
	GSM_ONOFF_HIGH();

	// 上电完成
	gsmUartCB.power.state = (BOOL)param;
}

