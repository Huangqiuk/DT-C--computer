#include "common.h"
#include "system.h"
#include "timer.h"
#include "delay.h"
#include "Watchdog.h"
#include "powerCtl.h"
#include "spiflash.h"
#include "iapctrluart.h"
#include "iap.h"
#include "param.h"
#include "state.h"
#include "keyMsgProcess.h"
#include "key.h"

int main(void)
{	
	DI();

	// 上电控制
	POWER_Init();

	// MCU底层配置
	SYSTEM_Init();

	// 定时器模块初始化
	TIMER_Init();
	
	// IAP_UART初始化
	IAP_CTRL_UART_Init();
	
	// SPI_Flash初始化
	SPIx_FLASH_Init();

	// IAP初始化
	IAP_Init();

	// 参数池初始化
	PARAM_Init();

	// 按键初始化
	KEY_Init();

	// 状态机初始化
	STATE_Init();
	
	// 看门狗初始化
	//WDT_Init();
	
	EI();

	while(1)
	{
		// 清看门狗
		//WDT_Clear();
		
		// 状态机模块过程处理
		STATE_Process();
		
		// 定时器任务队列处理
		TIMER_TaskQueueProcess();

		// IAP_UART通讯模块处理
		IAP_CTRL_UART_Process();
		
		// 注册处理
		Param_Process();
		
		// 按键过程处理
		KEYMSG_Process();
	}
}

