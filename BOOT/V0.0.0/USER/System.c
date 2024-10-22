#include "main.h"

// �ڲ���������
static void CLOCK_Configuration(void);
static void NVIC_Configuration(void);

// ϵͳӲ���ײ�����
void SYSTEM_Init(void)
{
	CLOCK_Configuration();

	// �����ں�����ʱ�ӱ���
	System_Core_Clock_Frequency_Update();

	// �����жϷ������
	NVIC_Configuration();

	// �ж�������ƫ��
#if __SYSTEM_DEBUG__
#else
    _VTORVALUE();         //��д�ж�ӳ���ַ�����ڶ����תʱ��ÿ��д֮ǰ�����Ƚ��мĴ�����0�����������ַ����
    _VTOREN();            //ʹ�ܼĴ���λ
#endif
}

// ϵͳ�ܸ�λ
void SYSTEM_Rst(uint32 param)
{
//	__disable_fault_irq();
    TIM_Interrupt_Disable(TIM3, TIM_INT_UPDATE);
    TIM_Off(TIM3);
    UART_Interrput_Disable(UART1, UART_INT_RXDNE);
    UART_Disable(UART1);	
	NVIC_SystemReset();
}

// ����ʱ������
void CLOCK_Configuration(void)
{
	RCC_Reset();
	
	// Enable HSE        
//	RCC_ConfigHse(RCC_HSE_ENABLE);

//	// Wait till HSE is ready        
//	if (RCC_WaitHseStable() == SUCCESS)// �ȴ�HSE ����SUCCESS;HSE �����ȶ��Ҿ���
//	{
//		RCC_ConfigHclk(RCC_SYSCLK_DIV1);

//		RCC_ConfigPclk1(RCC_HCLK_DIV2);
//		RCC_ConfigPclk2(RCC_HCLK_DIV1);
//		// �� HCLK <= 32MHz ʱ���ȴ���������СΪ 0���� 32MHz < HCLK <= 64MHzʱ���ȴ���������СΪ 1
//		FLASH_SetLatency(FLASH_LATENCY_1);
//		FLASH_PrefetchBufSet(FLASH_PrefetchBuf_EN);
//		RCC_ConfigPll(RCC_PLL_SRC_HSE_DIV1, RCC_PLL_MUL_6, RCC_PLLDIVCLK_DISABLE);
//		RCC_EnablePll(ENABLE);
//		while (RCC_GetFlagStatus(RCC_CTRL_FLAG_PLLRDF) == RESET); 	// ���ָ����RCC ��־λ(PLL ׼���ñ�־)�������
//		
//		RCC_ConfigSysclk(RCC_SYSCLK_SRC_PLLCLK);
//		while (RCC_GetSysclkSrc() != (uint32)0x0C);
//	}
//	else
//	{
//		RCC_ConfigHclk(RCC_SYSCLK_DIV1);

//		RCC_ConfigPclk1(RCC_HCLK_DIV2);
//		RCC_ConfigPclk2(RCC_HCLK_DIV1);
//		// �� HCLK <= 32MHz ʱ���ȴ���������СΪ 0���� 32MHz < HCLK <= 64MHzʱ���ȴ���������СΪ 1
//		FLASH_SetLatency(FLASH_LATENCY_1);
//		FLASH_PrefetchBufSet(FLASH_PrefetchBuf_EN);
//		RCC_ConfigPll(RCC_PLL_HSI_PRE_DIV1, RCC_PLL_MUL_3, RCC_PLLDIVCLK_DISABLE);
//		RCC_EnablePll(ENABLE);
//		while (RCC_GetFlagStatus(RCC_CTRL_FLAG_PLLRDF) == RESET); 	// ���ָ����RCC ��־λ(PLL ׼���ñ�־)�������
//		
//		RCC_ConfigSysclk(RCC_SYSCLK_SRC_PLLCLK);
//		while (RCC_GetSysclkSrc() != (uint32)0x0C);
//	}
}

// NVIC�жϷ�������
void NVIC_Configuration(void)
{
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
}

