#include "dutCtl.h"
#include "dutInfo.h"
#include "common.h"
#include "timer.h"
#include "DutUartDrive.h"
#include "uartProtocol.h"
#include "uartProtocol3.h"
#include "CanProtocol_3A.h"
#include "CanProtocolUpGB.h"
#include "CanProtocolUpDT.h"


void DUTCTRL_BusInit(void)
{
	switch (dut_info.dutBusType)
	{
        case 0: // 串口升级
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UART协议层初始化
            UART_PROTOCOL_Init();						 // 55升级协议
            UART_PROTOCOL4_Init();						 // KM5S
    //		LIME_UART_PROTOCOL_Init();
            break;
            
        case 1: // can升级
            switch (dut_info.ID)
            {
                case DUT_TYPE_GB:
                    // 高标升级
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
                    CAN_PROTOCOL_Init(); // 高标升级初始化
                    break;
//                    
                default:
//                    // 通用can升级
//                    
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
                    CAN_PROTOCOL1_Init(); // 通用配置初始化
                    break;
            }
        break;
	}
}
