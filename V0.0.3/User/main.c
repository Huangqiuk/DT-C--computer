#include "common.h"
#include "system.h"
#include "timer.h"
#include "delay.h"
#include "e2prom.h" 
#include "Watchdog.h"
#include "powerCtl.h"
#include "spiflash.h"
#include "iapctrluart.h"
#include "iap.h"
#include "bleuart.h"
#include "gsmuart.h"
#include "sysinfo.h"
#include "param.h"
//#include "lcd.h"
//#include "key.h"
//#include "keyMsgProcess.h"
#include "state.h"
#include "BleProtocol.h"


uint8 arr[100] = {0};

int main(void)
{	

	uint8 upAppFlag = 0x00;

	DI();

	// 上电控制
	POWER_Init();

	// MCU底层配置
	SYSTEM_Init();

	// 定时器模块初始化
	TIMER_Init();
	
//	// LCD驱动模块初始化入口	
//	LCD_Init();	 

	// IAP_UART初始化
	IAP_CTRL_UART_Init();

	// SPI_Flash初始化
	SPIx_FLASH_Init();

	// IAP初始化
	IAP_Init();

//	// 参数池初始化
	PARAM_Init();

	// 状态机初始化
	STATE_Init();
	
	// 看门狗初始化
	//WDT_Init();
//	IAP_CTRL_UART_SendCmdWithResult(0x03, TRUE);
//	extern void SPI_FLASH_ReadArray(uint8* pBuffer, uint32 ReadAddr, uint16 NumByteToRead);
//	extern void IAP_RunAPP(uint32 appAddr);
//	SPI_FLASH_ReadArray(arr,0x00,10);
//	SPI_FLASH_ReadArray(&arr[11],SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS,1);
//	SPI_FLASH_ReadArray(&arr[12],SPI_FLASH_APP_DATA_SIZE,4);

	
	EI();
	
	while(1)
	{
		// 清看门狗
		//WDT_Clear();
		//
		// 状态机模块过程处理
		STATE_Process();
		 
		// 定时器任务队列处理
		TIMER_TaskQueueProcess();

		// IAP_UART通讯模块处理
		IAP_CTRL_UART_Process();

//		// 按键过程处理
//		KEYMSG_Process();
	}
}

