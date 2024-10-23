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

void DUTCTRL_Init() // 继电器输出初始化
{
	rcu_periph_clock_enable(RCU_GPIOE);
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11); // 继电器使能
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12); // 电压选择
	
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
        case 0: // 串口升级
            // UART驱动层初始化
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UART协议层初始化
            UART_PROTOCOL_Init();						 // 55升级协议
            UART_PROTOCOL3_Init();						 // KM5S
            LIME_UART_PROTOCOL_Init();
            break;
            
        case 1: // can升级
            switch (dut_info.ID)
            {
                case DUT_TYPE_GB:
                    // 高标升级
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
                    CAN_PROTOCOL_Init(); // 高标升级初始化
                    CAN_TEST_PROTOCOL_Init();                    
                    break;

                case DUT_TYPE_MEIDI:
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
                    CAN_MD_PROTOCOL_Init(); // MeiDi升级初始化                  
                    break;
                    
                default:
                    // 通用can升级
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);
                    CAN_PROTOCOL1_Init(); // 通用配置初始化
                    CAN_TEST_PROTOCOL_Init();
                    break;
            }
            break;
	}
}
