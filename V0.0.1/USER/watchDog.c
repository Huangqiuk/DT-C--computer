#include "watchDog.h"

//*********************************************************** 
//�������ƣ� WDT_Init
//���Ź���ʼ��
//***********************************************************/
void WatchDog_Init(void)
{
    // ʹ�ܶ�IWDG_PR��IWDG_RLR�Ĵ�����д����
    IWDG_WriteConfig(IWDG_WRITE_ENABLE);

    // IWDG counter clock: LSI/32, ���ʱ��:0.8s �ʱ��:3276.8ms
    IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV32);

	// ����װ��ֵ����ʱʱ��Ϊ3276.8ms
    IWDG_CntReload(0xFFF);

	// ι��
    IWDG_ReloadKey();
	
		IWDG_Enable();
}
	
