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
	
	// MCU底层配置
	SYSTEM_Init();

	// 定时器模块初始化
	TIMER_Init();

	// 初始化SPI Flash驱动端口
	SPIx_FLASH_Init();

	// ARM串口初始化
	ARM_UART_Init();

	// ARM协议串口初始化
	ARM_PROTOCOL_Init();

	// 状态机初始化
	STATE_Init();

	// 参数初始化
	PARAM_Init();

	// 继电器控制初始化,继电器上电断电
	DUTCTRL_Init();

	// dut断电
	DUTCTRL_PowerOnOff(0);

	// 电压选择12V
	//DutVol_12();

	// 通信协议选择
	DUTCTRL_BusInit();
        
	EI();

	while (1)
	{
		// 定时器任务队列处理
		TIMER_TaskQueueProcess();

		// ARM通讯驱动层过程处理
		ARM_UART_Process();

		// ARM通讯协议层过程处理
		ARM_PROTOCOL_Process();

		// 状态机处理过程
		STATE_Process();
	}
}

