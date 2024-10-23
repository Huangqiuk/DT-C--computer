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

//void Delayms(uint32_t ms)
//{
//    for (uint32_t i = 0; i < ms; i++)
//    {
//        // ִ�� nop ����
//        __asm__("nop");
//    }
//}
