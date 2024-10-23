#include "dutCtl.h"
#include "dutInfo.h"
#include "common.h"
#include "timer.h"
#include "uartDrive.h"
#include "uartProtocol.h"
#include "uartProtocol3.h"
#include "CanProtocolUpDT.h"
#include "CanProtocolUpGB.h"
#include "canProtocol_3A.h"
#include "LimeUartProtocol.h"
#include "CanProtocolUpTest.h"
#include "CanProtocolUpMD.h"

void DUTCTRL_Init() // �̵��������ʼ��
{
	rcu_periph_clock_enable(RCU_GPIOE);
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11); // �̵���ʹ��
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12); // ��ѹѡ��
	
	if (dut_info.voltage)// 24V
	{
		DutVol_24();
	}
	else
	{
		DutVol_12();
	}
}

void DUTCTRL_PowerOnOff(uint32 param)
{
	if (param)
	{
		Relay_ON();
	}
	else
	{
		Relay_OFF();
	}
}

void DUTCTRL_BusInit()
{
	switch (dut_info.dutBusType)
	{
        case 0: // ��������
            // UART�������ʼ��
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UARTЭ����ʼ��
            UART_PROTOCOL_Init();						 // 55����Э��
            UART_PROTOCOL3_Init();						 // KM5S
            LIME_UART_PROTOCOL_Init();
            break;
            
        case 1: // can����
            switch (dut_info.ID)
            {
                case DUT_TYPE_GB:
                    // �߱�����
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
                    CAN_PROTOCOL_Init(); // �߱�������ʼ��
                    CAN_TEST_PROTOCOL_Init();                    
                    break;

                case DUT_TYPE_MEIDI:
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
                    CAN_MD_PROTOCOL_Init(); // MeiDi������ʼ��                  
                    break;
                    
                default:
                    // ͨ��can����
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);
                    CAN_PROTOCOL1_Init(); // ͨ�����ó�ʼ��
                    CAN_TEST_PROTOCOL_Init();
                    break;
            }
            break;
	}
}
