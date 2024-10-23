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
	// �ر��ж�
	DI();

	// ��ʼ��ϵ ͳʱ��
	SYSTEM_Init();
	
    // ��ʱ��Ϊ�˱�SD7890оƬ��������
	//Delayms(250);	
	//Delayms(250);
	
	// ��ʱ����ʼ��
	TIMER_Init();

	// IAP��ʼ��
	IAP_Init();
		
	// ����������ʼ��
	UART_DRIVE_Init();
	
	// Э����ʼ��
	UART_PROTOCOL_Init();
	LoadPrameToLocal();	
	LED_Init();
	
	Avoment_Init();
	
	Debug_Init();

	KEY_Init();
	
	Relay_HwInit();
		
	printf("\r\n");
	
	printf("��ʼ���ɹ�!!\r\n");
     		
	//���ж�
	EI();
	//test_start();
	while (1)
	{		
		// ��ʱ��������д���
		TIMER_TaskQueueProcess();
		
		// meterUuartЭ�����̴���
		Avoment_Process(); 
		
		// ��������̴���
		UART_DRIVE_Process();
		
		// Э�����̴���
		UART_PROTOCOL_Process();
				
		// ���ñ����ݴ���
		Avoment_Process();
		
	 }
	
}


