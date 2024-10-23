#include "watchDog.h"

//*********************************************************** 
//函数名称： WDT_Init
//看门狗初始化
//***********************************************************/
void WatchDog_Init(void)
{
    // 使能对IWDG_PR和IWDG_RLR寄存器的写访问
    IWDG_WriteConfig(IWDG_WRITE_ENABLE);

    // IWDG counter clock: LSI/32, 最短时间:0.8s 最长时间:3276.8ms
    IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV32);

	// 填入装载值，超时时间为3276.8ms
    IWDG_CntReload(0xFFF);

	// 喂狗
    IWDG_ReloadKey();
	
		IWDG_Enable();
}
	
