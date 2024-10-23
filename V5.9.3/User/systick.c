#include "common.h"
#include "systick.h"

volatile uint32 SystickTimingCnt;

// 初始化系统滴答时钟
void SysTick_Init(uint32 ntime)
{
	if (SysTick_Config(SystemCoreClock / 1000000ul) * ntime)
	{
		while (1)
			;
	}
}

// 延时us函数
void Systick_Delayus(uint32 nTime)
{
	SystickTimingCnt = nTime;
	while (SystickTimingCnt != 0)
		; 
}

// 延时ms函数
void Systick_Delayms(uint32 nTime)
{
	while (nTime != 0)
	{
		Systick_Delayus(1000);
		nTime--;
	}
}

// 简单延时
void Delayms(uint16 i)
{
    uint16 x;
    uint16 y;

    // 内核72MHz，1ms
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