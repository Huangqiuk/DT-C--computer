#include "watchDog.h"

//*********************************************************** 
//�������ƣ� WDT_Init
//���Ź���ʼ��
//***********************************************************/
void WatchDog_Init(void)
{
	IWDG_WriteConfig(IWDG_WRITE_ENABLE);
	
	IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV32);
	
	IWDG_CntReload(0xFFF);
	
	IWDG_ReloadKey();
	
	IWDG_Enable();
}
	
