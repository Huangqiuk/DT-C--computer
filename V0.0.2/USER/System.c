#include "common.h"
#include "System.h"
#include "delay.h"

// �ڲ���������
static void CLOCK_Configuration(void);
static void NVIC_Configuration(void);
//static void LVD_Configuration(void);

// ϵͳӲ���ײ�����
void SYSTEM_Init(void)
{
	CLOCK_Configuration();

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

// ����ʱ������
void CLOCK_Configuration(void)
{
	RCC_DeInit();
	
	// Enable HSE        
	RCC_ConfigHse(RCC_HSE_ENABLE);

	// Wait till HSE is ready        
	if (RCC_WaitHseStable() == SUCCESS)// �ȴ�HSE ����SUCCESS;HSE �����ȶ��Ҿ���
	{
		RCC_ConfigHclk(RCC_SYSCLK_DIV1);

		RCC_ConfigPclk1(RCC_HCLK_DIV2);
		RCC_ConfigPclk2(RCC_HCLK_DIV1);
		// �� HCLK <= 32MHz ʱ���ȴ���������СΪ 0���� 32MHz < HCLK <= 64MHzʱ���ȴ���������СΪ 1
		FLASH_SetLatency(FLASH_LATENCY_1);
		FLASH_PrefetchBufSet(FLASH_PrefetchBuf_EN);
		RCC_ConfigPll(RCC_PLL_SRC_HSE_DIV1, RCC_PLL_MUL_6, RCC_PLLDIVCLK_DISABLE);
		RCC_EnablePll(ENABLE);
		while (RCC_GetFlagStatus(RCC_CTRL_FLAG_PLLRDF) == RESET); 	// ���ָ����RCC ��־λ(PLL ׼���ñ�־)�������
		
		RCC_ConfigSysclk(RCC_SYSCLK_SRC_PLLCLK);
		while (RCC_GetSysclkSrc() != (uint32)0x0C);
	}
	else
	{
		RCC_EnableHsi(ENABLE);
		RCC_WaitHsiStable();
		
		RCC_ConfigHclk(RCC_SYSCLK_DIV1);

		RCC_ConfigPclk1(RCC_HCLK_DIV2);
		RCC_ConfigPclk2(RCC_HCLK_DIV1);
		// �� HCLK <= 32MHz ʱ���ȴ���������СΪ 0���� 32MHz < HCLK <= 64MHzʱ���ȴ���������СΪ 1
		FLASH_SetLatency(FLASH_LATENCY_1);
		FLASH_PrefetchBufSet(FLASH_PrefetchBuf_EN);
		RCC_ConfigPll(RCC_PLL_HSI_PRE_DIV1, RCC_PLL_MUL_3, RCC_PLLDIVCLK_DISABLE);
		RCC_EnablePll(ENABLE);
		while (RCC_GetFlagStatus(RCC_CTRL_FLAG_PLLRDF) == RESET); 	// ���ָ����RCC ��־λ(PLL ׼���ñ�־)�������
		
		RCC_ConfigSysclk(RCC_SYSCLK_SRC_PLLCLK);
		while (RCC_GetSysclkSrc() != (uint32)0x0C);
	}
}

// NVIC�жϷ�������
void NVIC_Configuration(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
}

// �͵�ѹ�������
//void LVD_Configuration(void)
//{
//	EXTI_InitType EXTI_InitStructure;
//	NVIC_InitType NVIC_InitStructure;

//	// Enable PWR and BKP clock  
//	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR, ENABLE);
//	
//	// Configure EXTI Line16(PVD Output) to generate an interrupt on rising and falling edges     
//	EXTI_ClrITPendBit(EXTI_LINE16);    
//	EXTI_InitStructure.EXTI_Line    = EXTI_LINE16;    
//	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;    
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;    
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;    
//	EXTI_InitPeripheral(&EXTI_InitStructure);

//    // Enable the PVD Interrupt
//    NVIC_InitStructure.NVIC_IRQChannel                   = PVD_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);

//    // Configure the PVD Level to 2.95V
//    PWR_PVDLevelConfig(PWR_PVDLEVEL_2V95);

//    // Enable the PVD Output
//    PWR_PvdEnable(ENABLE);
//}
