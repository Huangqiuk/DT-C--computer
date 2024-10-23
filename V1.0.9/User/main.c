#include "common.h"
#include "system.h"
#include "state.h"
#include "timer.h"
#include "spiflash.h"
#include "AvoDrive.h"
#include "AvoProtocol.h"
#include "StsDrive.h"
#include "StsProtocol.h"
#include "PowerDrive.h"
#include "POWERProtocol.h"
#include "DutUartDrive.h"
#include "AvoPin.h"
#include "DutUartProtocol.h"
#include "param.h"
#include "CanProtocol_3A.h"
#include "key.h"
#include "keyMsgProcess.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "CanProtocolUpDT.h"
#include "CanProtocolTest.h"
#include "UartprotocolXM.h"
#include "adc.h"
#include "pwm.h"
#include "uartProtocol.h"

// ��������ָ��
void Key_Info(uint32 param)
{
    STS_PROTOCOL_SendCmdAck(STS_PROTOCOL_CMD_EXECUTIVE_DIRECTOR);
    
    // ������͸��ģʽ�£�����������ť�������߼�ⶨʱ��
    if(dut_info.passThroughControl)
    {
        dut_info.online_dete_cnt = 0;
        
          TIMER_AddTask(TIMER_ID_ONLINE_DETECT,
          3000,
          DutTimeOut,
          0,
          1,
          ACTION_MODE_ADD_TO_QUEUE);
    }
}

int main(void)
{
    DI();

    // MCU�ײ�����
    SYSTEM_Init();

    // LCD��ʼ��
    Init_Lcd();

    // ��ӡ������Ϣ
    POWER_ON_Information();

    // ��ʱ��ģ���ʼ��
    TIMER_Init();

    // ��ʼ��SPI Flash�����˿�
    SPIx_FLASH_Init();

    // ���ñ����ų�ʼ��
    AVO_PIN_Init();

    // ADC ��ʼ��
    ADC_HwInit();

    // DAC��ʼ��
    DAC_Init();
    
	// ADģ���ʼ��
    ADC_BaseInit();

    // PWM��ʼ��
    PWM_OUT1_Init();
   
    // STS���ڳ�ʼ��
    STS_UART_Init();

    // STSЭ�鴮�ڳ�ʼ��
    STS_PROTOCOL_Init();

    // ��ԴͨѶ���ڳ�ʼ��
    POWER_UART_Init();

    // ��ԴͨѶЭ���ʼ��
    POWER_PROTOCOL_Init();

    // ���ñ�ͨѶ���ڳ�ʼ��
    AVO_UART_Init();

    // ���ñ�ͨѶЭ���ʼ��
    AVO_PROTOCOL_Init() ;

    // ״̬����ʼ��
    STATE_Init();

    // ����������ʼ��
    KEY_Init();

    // ע�����а���
    KEYMSG_StopAllService();

    // ע�ᰴ��
    KEYMSG_RegisterMsgService(KEY_NAME_START, KEY_MSG_UP, Key_Info, 0);

    // ������ʼ��
    PARAM_Init();

    // ����Э��ѡ��
    DUTCTRL_BusInit();

    // ����ϱ�
    TIMER_AddTask(TIMER_ID_REPORT_OF_IDENTITY,
                  500,
                  Report_Identity,
                  0,
                  -1,
                  ACTION_MODE_ADD_TO_QUEUE);              
    
    EI();

    while (1)
    {
        // ��ʱ��������д���
        TIMER_TaskQueueProcess();
 
        // STSͨѶ��������̴���
        STS_UART_Process();

        // STSͨѶЭ�����̴���
        STS_PROTOCOL_Process();

        // �������̴���
        KEYMSG_Process();

        // ��Դ��ͨѶ��������̴���
        POWER_UART_Process();

        // ��Դ��ͨѶЭ�����̴���
        POWER_PROTOCOL_Process();

        // ���ñ�ͨѶ��������̴���
        AVO_UART_Process();

        // ���ñ�ͨѶ��������̴���
        AVO_PROTOCOL_Process();

        // UART��������̴���
        UART_DRIVE_Process();

        // UARTЭ�����̴���
        DUT_UART_PROTOCOL_Process();

        // С��Э�����̴���
        UART_PROTOCOL_XM_Process();

        // CANЭ�����̴���
        CAN_PROTOCOL_Process_Test();

        // 3A����Э��
        CAN_PROTOCOL_Process_3A();
        
        // 3A����Э��
        UART_PROTOCOL_Process();
        
        // ״̬�����̴���
        STATE_Process();    

		// ADC���̴���
		ADC_Process();        
    }
}

