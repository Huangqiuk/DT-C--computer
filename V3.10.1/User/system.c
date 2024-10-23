#include "common.h"
#include "system.h"
#include "delay.h"

// �ڲ���������
static void RCC_HSE_Configuration(void);
static void NVIC_Configuration(void);

// ϵͳӲ���ײ�����
void SYSTEM_Init(void)
{
	// ϵͳ������ʱ������
	RCC_HSE_Configuration();
	// �����ں�����ʱ�ӱ���
	SystemCoreClockUpdate();

	// �����жϷ������
	NVIC_Configuration();
}

// ϵͳ�ܸ�λ
void SYSTEM_Rst(uint32 param)
{
	__disable_fault_irq();
	NVIC_SystemReset();
}

/*************************************
* RCC_HSE_Configuration ��������HSE ��ΪPLL ��ʱ��Դ
* ִ��SystemInit ������оƬ�ڲ�ʱ��Ϊ��
* SYSCLK 72MHz
* AHB 72MHz
* PCLK1 36MHz
* PCLK2 72MHz
* PLL 72MHz
**************************************/
void RCC_HSE_Configuration(void)
{
	rcu_deinit();

	// �����ⲿ���پ���(HSE) HSE �����(ON)
	rcu_osci_on(RCU_HXTAL);

	if (SUCCESS == rcu_osci_stab_wait(RCU_HXTAL))
	{
		// configure the AHB prescaler selection
		rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
		// configure the APB1 prescaler selection
		rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2);
		// configure the APB2 prescaler selection
		rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);

		// �����Ƿ��Ƶ
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

// NVIC�жϷ�������
void NVIC_Configuration(void)
{
	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
}


