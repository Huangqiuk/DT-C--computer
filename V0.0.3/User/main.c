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

	// �ϵ����
	POWER_Init();

	// MCU�ײ�����
	SYSTEM_Init();

	// ��ʱ��ģ���ʼ��
	TIMER_Init();
	
//	// LCD����ģ���ʼ�����	
//	LCD_Init();	 

	// IAP_UART��ʼ��
	IAP_CTRL_UART_Init();

	// SPI_Flash��ʼ��
	SPIx_FLASH_Init();

	// IAP��ʼ��
	IAP_Init();

//	// �����س�ʼ��
	PARAM_Init();

	// ״̬����ʼ��
	STATE_Init();
	
	// ���Ź���ʼ��
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
		// �忴�Ź�
		//WDT_Clear();
		//
		// ״̬��ģ����̴���
		STATE_Process();
		 
		// ��ʱ��������д���
		TIMER_TaskQueueProcess();

		// IAP_UARTͨѶģ�鴦��
		IAP_CTRL_UART_Process();

//		// �������̴���
//		KEYMSG_Process();
	}
}

