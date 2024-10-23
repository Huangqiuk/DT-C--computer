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
        case 0: // ��������
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UARTЭ����ʼ��
            UART_PROTOCOL_Init();						 // 55����Э��
            UART_PROTOCOL4_Init();						 // KM5S
    //		LIME_UART_PROTOCOL_Init();
            break;
            
        case 1: // can����
            switch (dut_info.ID)
            {
                case DUT_TYPE_GB:
                    // �߱�����
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
                    CAN_PROTOCOL_Init(); // �߱�������ʼ��
                    break;
//                    
                default:
//                    // ͨ��can����
//                    
                    IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
                    CAN_PROTOCOL1_Init(); // ͨ�����ó�ʼ��
                    break;
            }
        break;
	}
}
