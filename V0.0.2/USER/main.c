#include "common.h"
#include "system.h"
#include "delay.h"
#include "iap.h"
#include "timer.h"
#include "powerctl.h"
#include "watchDog.h"
#include "state.h"
#include "uartdrive.h"
#include "uartprotocol.h"
#include "param.h"

int main(void)
{
    // �ر��ж�
    DI();

    // ��ʼ��ϵͳʱ��
    SYSTEM_Init();

    // ��ʱ����ʼ��
    TIMER_Init();

	// ��ʼ������
	PARAM_Init();
    
	// IAP��ʼ��
	IAP_Init();

    // ���ڳ�ʼ��
    UART_DRIVE_Init(115200);

    // Э�鴮�ڳ�ʼ��
    UART_PROTOCOL_Init();

	// ״̬����ʼ��
	STATE_Init();
    
    EI();

    while (1)
    {
        // ��������̴���
        UART_DRIVE_Process();

        // Э�����̴���
        UART_PROTOCOL_Process();

        // ��ʱ��������д���
        TIMER_TaskQueueProcess();
        
        // ״̬��ģ����̴���
		STATE_Process();
        
    }
}


