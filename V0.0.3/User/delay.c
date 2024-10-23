#include "common.h"
#include "delay.h"

// 简单延时
void Delayus(uint8 i)
{	
	// 内核72MHz，1us
	while (i--) 
	{
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();	// 10
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();	// 10
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();	// 10
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();	// 6
	}
}

// 简单延时
void Delayms(uint8 i) 
{
	uint16 x;
	uint8 y;

	// 内核72MHz，1ms
	while (i--)
	{
		x = 700; 
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
