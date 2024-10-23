#include "common.h"
#include "system.h"
#include "delay.h"

// 内部函数声明
static void RCC_HSE_Configuration(void);
static void NVIC_Configuration(void);

// 系统硬件底层配置
void SYSTEM_Init(void)
{
	// 系统各运行时钟配置
	RCC_HSE_Configuration();
	// 更新内核运行时钟变量
	SystemCoreClockUpdate();

	// 配置中断分组情况
	NVIC_Configuration();
}

// 系统总复位
void SYSTEM_Rst(uint32 param)
{
	__disable_fault_irq();
	NVIC_SystemReset();
}

/*************************************
* RCC_HSE_Configuration 函数设置HSE 作为PLL 的时钟源
* 执行SystemInit 函数后芯片内部时钟为：
* SYSCLK 72MHz
* AHB 72MHz
* PCLK1 36MHz
* PCLK2 72MHz
* PLL 72MHz
**************************************/
void RCC_HSE_Configuration(void)
{
	rcu_deinit();

	// 设置外部高速晶振(HSE) HSE 晶振打开(ON)
	rcu_osci_on(RCU_HXTAL);

	if (SUCCESS == rcu_osci_stab_wait(RCU_HXTAL))
	{
		// configure the AHB prescaler selection
		rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
		// configure the APB1 prescaler selection
		rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2);
		// configure the APB2 prescaler selection
		rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);

		// 晶振是否分频
		rcu_predv0_config(RCU_PREDV0_DIV1);

		// configure the PLL clock source selection and PLL multiply factor
		rcu_pll_config(RCU_PLLSRC_HXTAL, RCU_PLL_MUL9);

		rcu_osci_on(RCU_PLL_CK);

		rcu_osci_stab_wait(RCU_PLL_CK);

		rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);

		while (RCU_SCSS_PLL != rcu_system_clock_source_get());
	}
	else
	{
		// configure the AHB prescaler selection
		rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
		// configure the APB1 prescaler selection
		rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2);
		// configure the APB2 prescaler selection
		rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);

		// configure the PLL clock source selection and PLL multiply factor
		rcu_pll_config(RCU_PLLSRC_IRC8M_DIV2, RCU_PLL_MUL18);

		rcu_osci_on(RCU_PLL_CK);

		rcu_osci_stab_wait(RCU_PLL_CK);

		rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);

		while (RCU_SCSS_PLL != rcu_system_clock_source_get());
	}
}

// NVIC中断分组配置
void NVIC_Configuration(void)
{
	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
}


