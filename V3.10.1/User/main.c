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

	// �ϵ����
	POWER_Init();

	// MCU�ײ�����
	SYSTEM_Init();

	// ��ʱ��ģ���ʼ��
	TIMER_Init();
	
	// IAP_UART��ʼ��
	IAP_CTRL_UART_Init();
	
	// SPI_Flash��ʼ��
	SPIx_FLASH_Init();

	// IAP��ʼ��
	IAP_Init();

	// �����س�ʼ��
	PARAM_Init();

	// ������ʼ��
	KEY_Init();

	// ״̬����ʼ��
	STATE_Init();
	
	// ���Ź���ʼ��
	//WDT_Init();
	
	EI();

	while(1)
	{
		// �忴�Ź�
		//WDT_Clear();
		
		// ״̬��ģ����̴���
		STATE_Process();
		
		// ��ʱ��������д���
		TIMER_TaskQueueProcess();

		// IAP_UARTͨѶģ�鴦��
		IAP_CTRL_UART_Process();
		
		// ע�ᴦ��
		Param_Process();
		
		// �������̴���
		KEYMSG_Process();
	}
}

