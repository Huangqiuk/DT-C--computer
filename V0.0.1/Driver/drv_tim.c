//-----------------------------------------------------------------------------
// drv_tim.c
//-----------------------------------------------------------------------------
#include "drv_tim.h"
#include "n32g45x_io_action.h"
#include "my_io_defs.h"
#include "bsp_init.h"

static u32  fac_us=0;	 //us��ʱ������			   
static u32  fac_ms=0;	 //ms��ʱ������
static u8   max_ms=116;

void Systick_init(void)
{
		SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);	//ѡ���ⲿʱ��HCLK
		fac_us=SystemCoreClock/1000000;										//Ϊϵͳʱ��
		fac_ms=fac_us*1000;																//����ÿ��ms��Ҫ��systickʱ����   
}

void HwSleepUs(u32 nus)
{
		u32 temp;	    	 
		SysTick->LOAD=nus*fac_us; 								//ʱ�����	  		 
		SysTick->VAL=0x00;        								//��ռ�����
		SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����	  
		do
		{
			temp = SysTick->CTRL;
		}
		while((temp&0x01) && !(temp&(1<<16)));		//�ȴ�ʱ�䵽��

		SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
		SysTick->VAL =0X00;      									//��ռ�����	 
}

void delay_ms(u32 nms)
{
		u32 temp;
		SysTick->LOAD=(u32)nms*fac_ms;						//ʱ�����(SysTick->LOADΪ24bit)
		SysTick->VAL =0x00;												//��ռ�����
		SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����  
		do
		{
			temp=SysTick->CTRL;
		}while((temp&0x01)&&!(temp&(1<<16)));			//�ȴ�ʱ�䵽��   
		SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
		SysTick->VAL =0X00;       								//��ռ�����	  	    
}

void HwSleepMs(u32 nms)
{
		u16 i;
		u16 count_1 = (nms / max_ms);  //max_ms=116
		u16 count_2 = (nms % max_ms);
		
		if(0 == count_1)
		{
				delay_ms(nms);
		}
		else
		{
				for(i=0;i<count_1;i++)
				{
					delay_ms(max_ms);
				}
				
				if(count_2 != 0)
				{
					delay_ms(count_2);
				}
		}	
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------


