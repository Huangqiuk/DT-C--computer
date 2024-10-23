#include "common.h"
#include "systick.h"

volatile uint32 SystickTimingCnt;

// ��ʼ��ϵͳ�δ�ʱ��
void SysTick_Init(uint32 ntime)
{
	if (SysTick_Config(SystemCoreClock / 1000000ul) * ntime)
	{
		while (1)
			;
	}
}

// ��ʱus����
void Systick_Delayus(uint32 nTime)
{
	SystickTimingCnt = nTime;
	while (SystickTimingCnt != 0)
		; 
}

// ��ʱms����
void Systick_Delayms(uint32 nTime)
{
	while (nTime != 0)
	{
		Systick_Delayus(1000);
		nTime--;
	}
}

// ����ʱ
void Delayms(uint16 i)
{
    uint16 x;
    uint16 y;

    // �ں�72MHz��1ms
    while (i--)
    {
        x = 1000;
        while (x--)
        {
            y = 10;
            while (y--)
            {
                NOP();
            }
        }
    }
}