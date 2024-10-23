#include "common.h"
#include "system.h"
#include "timer.h"
#include "delay.h"
#include "iap.h"
#include "state.h"
#include "uartdrive.h"
#include "uartprotocol.h"
#include "watchDog.h"
#include "meterControl.h"
#include "Source_Relay.h"
#include "state.h"
#include "debug.h"
#include "led.h"
#include "key.h"



//uint8_t sta=0;
//void test_callback(uint32_t para)
//{
//  if(sta)
//  {
//    OHM_RelayCtrl(RELAY_OHM0,OFF);
//	OHM_RelayCtrl(RELAY_OHM1,OFF);  
//	OHM_RelayCtrl(RELAY_OHM2,OFF);  
//	OHM_RelayCtrl(RELAY_OHM3,OFF);  
//	OHM_RelayCtrl(RELAY_OHM4,OFF);   
//    VIN_RelayCtrl(RELAY_VIN0,OFF);
//	VIN_RelayCtrl(RELAY_VIN1,OFF);  
//	VIN_RelayCtrl(RELAY_VIN2,OFF);  
//	VIN_RelayCtrl(RELAY_VIN3,OFF);  
//	VIN_RelayCtrl(RELAY_VIN4,OFF);  
//	VIN_RelayCtrl(RELAY_VIN5,OFF); 
//    COM_CHL_OFF();
//	sta=0;
//  }
//  else
//  {
//    OHM_RelayCtrl(RELAY_OHM0,ON);
//	OHM_RelayCtrl(RELAY_OHM1,ON);  
//	OHM_RelayCtrl(RELAY_OHM2,ON);  
//	OHM_RelayCtrl(RELAY_OHM3,ON);  
//	OHM_RelayCtrl(RELAY_OHM4,ON);   
//    VIN_RelayCtrl(RELAY_VIN0,ON);
//	VIN_RelayCtrl(RELAY_VIN1,ON);  
//	VIN_RelayCtrl(RELAY_VIN2,ON);  
//	VIN_RelayCtrl(RELAY_VIN3,ON);  
//	VIN_RelayCtrl(RELAY_VIN4,ON);  
//	VIN_RelayCtrl(RELAY_VIN5,ON); 
//    COM_CHL_ON();
//	sta=1; 
//  }	  
//}

//void test_start(void)
//{
//    TIMER_AddTask(TIMER_ID_RELAY_TEST,
//        2000,
//        test_callback,
//        0,
//        TIMER_LOOP_FOREVER,
//        ACTION_MODE_ADD_TO_QUEUE);

//}


int main(void)
{	
	// 关闭中断
	DI();

	// 初始化系 统时钟
	SYSTEM_Init();
	
    // 延时是为了比SD7890芯片更慢启动
	//Delayms(250);	
	//Delayms(250);
	
	// 定时器初始化
	TIMER_Init();

	// IAP初始化
	IAP_Init();
		
	// 串口驱动初始化
	UART_DRIVE_Init();
	
	// 协议层初始化
	UART_PROTOCOL_Init();
	LoadPrameToLocal();	
	LED_Init();
	
	Avoment_Init();
	
	Debug_Init();

	KEY_Init();
	
	Relay_HwInit();
		
	printf("\r\n");
	
	printf("初始化成功!!\r\n");
     		
	//打开中断
	EI();
	//test_start();
	while (1)
	{		
		// 定时器任务队列处理
		TIMER_TaskQueueProcess();
		
		// meterUuart协议层过程处理
		Avoment_Process(); 
		
		// 驱动层过程处理
		UART_DRIVE_Process();
		
		// 协议层过程处理
		UART_PROTOCOL_Process();
				
		// 万用表数据处理
		Avoment_Process();
		
	 }
	
}


