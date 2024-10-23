#include "Common.h"
#include "Watchdog.h" 

//*********************************************************** 
//函数名称： WDT_Init
//独立看门狗初始化
//***********************************************************/
void WDT_Init(void)
{
#if WDT_ENABLE
	 /* enable IRC40K */
    rcu_osci_on(RCU_IRC40K);
    
    /* wait till IRC40K is ready */
    while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K));
	
	fwdgt_config((uint16)(WDT_RESET_TIME/6.4), FWDGT_PSC_DIV256);  
	
	fwdgt_enable();

#endif
}

//***********************************************************
//函数名称： WDT_Clear
//函数功能： 清看门狗
//入口参数： 无
//出口参数： 无
//备 注： 
//************************************************************/
void WDT_Clear(void)
{
#if WDT_ENABLE
	fwdgt_counter_reload();                           // 重启计数器（喂狗）
#endif
}
