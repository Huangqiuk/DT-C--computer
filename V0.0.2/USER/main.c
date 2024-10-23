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
    // 关闭中断
    DI();

    // 初始化系统时钟
    SYSTEM_Init();

    // 定时器初始化
    TIMER_Init();

	// 初始化参数
	PARAM_Init();
    
	// IAP初始化
	IAP_Init();

    // 串口初始化
    UART_DRIVE_Init(115200);

    // 协议串口初始化
    UART_PROTOCOL_Init();

	// 状态机初始化
	STATE_Init();
    
    EI();

    while (1)
    {
        // 驱动层过程处理
        UART_DRIVE_Process();

        // 协议层过程处理
        UART_PROTOCOL_Process();

        // 定时器任务队列处理
        TIMER_TaskQueueProcess();
        
        // 状态机模块过程处理
		STATE_Process();
        
    }
}


