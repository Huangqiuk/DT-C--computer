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

// 发送启动指令
void Key_Info(uint32 param)
{
    STS_PROTOCOL_SendCmdAck(STS_PROTOCOL_CMD_EXECUTIVE_DIRECTOR);
    
    // 若处于透传模式下，按下启动按钮则开启在线检测定时器
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

    // MCU底层配置
    SYSTEM_Init();

    // LCD初始化
    Init_Lcd();

    // 打印开机信息
    POWER_ON_Information();

    // 定时器模块初始化
    TIMER_Init();

    // 初始化SPI Flash驱动端口
    SPIx_FLASH_Init();

    // 万用表引脚初始化
    AVO_PIN_Init();

    // ADC 初始化
    ADC_HwInit();

    // DAC初始化
    DAC_Init();
    
	// AD模块初始化
    ADC_BaseInit();

    // PWM初始化
    PWM_OUT1_Init();
   
    // STS串口初始化
    STS_UART_Init();

    // STS协议串口初始化
    STS_PROTOCOL_Init();

    // 电源通讯串口初始化
    POWER_UART_Init();

    // 电源通讯协议初始化
    POWER_PROTOCOL_Init();

    // 万用表通讯串口初始化
    AVO_UART_Init();

    // 万用表通讯协议初始化
    AVO_PROTOCOL_Init() ;

    // 状态机初始化
    STATE_Init();

    // 启动按键初始化
    KEY_Init();

    // 注销所有按键
    KEYMSG_StopAllService();

    // 注册按键
    KEYMSG_RegisterMsgService(KEY_NAME_START, KEY_MSG_UP, Key_Info, 0);

    // 参数初始化
    PARAM_Init();

    // 升级协议选择
    DUTCTRL_BusInit();

    // 身份上报
    TIMER_AddTask(TIMER_ID_REPORT_OF_IDENTITY,
                  500,
                  Report_Identity,
                  0,
                  -1,
                  ACTION_MODE_ADD_TO_QUEUE);              
    
    EI();

    while (1)
    {
        // 定时器任务队列处理
        TIMER_TaskQueueProcess();
 
        // STS通讯驱动层过程处理
        STS_UART_Process();

        // STS通讯协议层过程处理
        STS_PROTOCOL_Process();

        // 按键过程处理
        KEYMSG_Process();

        // 电源板通讯驱动层过程处理
        POWER_UART_Process();

        // 电源板通讯协议层过程处理
        POWER_PROTOCOL_Process();

        // 万用表通讯驱动层过程处理
        AVO_UART_Process();

        // 万用表通讯驱动层过程处理
        AVO_PROTOCOL_Process();

        // UART驱动层过程处理
        UART_DRIVE_Process();

        // UART协议层过程处理
        DUT_UART_PROTOCOL_Process();

        // 小米协议层过程处理
        UART_PROTOCOL_XM_Process();

        // CAN协议层过程处理
        CAN_PROTOCOL_Process_Test();

        // 3A生产协议
        CAN_PROTOCOL_Process_3A();
        
        // 3A生产协议
        UART_PROTOCOL_Process();
        
        // 状态机过程处理
        STATE_Process();    

		// ADC过程处理
		ADC_Process();        
    }
}

