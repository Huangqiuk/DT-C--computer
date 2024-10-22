#include "main.h"

uint32 updateFlag = 0;
uint32 buff[2] = {0};
#define app_update_flag_addr 		0x08007400	//地址要超出APP的内容范围。demo的bin内容在0x080054F8截止
int main(void)
{
//	// 关闭中断
//	DI();

	// 初始化系统时钟
	SYSTEM_Init();

    // 串口初始化
    UART_DRIVE_Init(UART_DRIVE_BAUD_RATE);

    // 协议串口初始化
    UART_PROTOCOL_Init();
    
	// 定时器初始化
	TIMER_Init();
	
//	// IAP初始化
//	IAP_Init();

//	// 状态机初始化
//	STATE_Init();
        
     // 测试
    TIMER_AddTask(TIMER_ID_TEST,
                  500,
                  UP_Finish_Report,
                  0,
                  -1,
                  ACTION_MODE_ADD_TO_QUEUE);          
        
    while (1)
    {
//		// 状态机模块过程处理
//		STATE_Process();
		
		// 定时器任务队列处理
		TIMER_TaskQueueProcess();
        
       // 驱动层过程处理
        UART_DRIVE_Process();

        // 协议层过程处理
        UART_PROTOCOL_Process();
    }
}
