#include "common.h"
#include "system.h"
#include "timer.h"
#include "ArmUart.h"
#include "ArmProtocol.h"
#include "spiflash.h"
#include "uartDrive.h"
#include "uartProtocol.h"
#include "uartProtocol3.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "CanProtocolUpDT.h"
#include "CanProtocolUpGB.h"
#include "canProtocol_3A.h"
#include "state.h"
#include "param.h"
#include "CanProtocolUpMD.h"

int main(void)
{
	DI();
	
	// MCU�ײ�����
	SYSTEM_Init();

	// ��ʱ��ģ���ʼ��
	TIMER_Init();

	// ��ʼ��SPI Flash�����˿�
	SPIx_FLASH_Init();

	// ARM���ڳ�ʼ��
	ARM_UART_Init();

	// ARMЭ�鴮�ڳ�ʼ��
	ARM_PROTOCOL_Init();

	// ״̬����ʼ��
	STATE_Init();

	// ������ʼ��
	PARAM_Init();

	// �̵������Ƴ�ʼ��,�̵����ϵ�ϵ�
	DUTCTRL_Init();

	// dut�ϵ�
	DUTCTRL_PowerOnOff(0);

	// ��ѹѡ��12V
	//DutVol_12();

	// ͨ��Э��ѡ��
	DUTCTRL_BusInit();
        
	EI();

	while (1)
	{
		// ��ʱ��������д���
		TIMER_TaskQueueProcess();

		// ARMͨѶ��������̴���
		ARM_UART_Process();

		// ARMͨѶЭ�����̴���
		ARM_PROTOCOL_Process();

		// ״̬���������
		STATE_Process();
	}
}

