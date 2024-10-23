#include "Common.h"
#include "Watchdog.h" 

//*********************************************************** 
//�������ƣ� WDT_Init
//�������Ź���ʼ��
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
//�������ƣ� WDT_Clear
//�������ܣ� �忴�Ź�
//��ڲ����� ��
//���ڲ����� ��
//�� ע�� 
//************************************************************/
void WDT_Clear(void)
{
#if WDT_ENABLE
	fwdgt_counter_reload();                           // ������������ι����
#endif
}
