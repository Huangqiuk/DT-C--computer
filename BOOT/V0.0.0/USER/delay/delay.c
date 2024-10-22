#include "delay.h"

// 简单延时
void Delayus(uint16_t i)
{	
	uint8_t j=0;
	// 内核72MHz，1us
	while (i--) 
	{
		j=3;
		while(j--);

	}
}

// 简单延时
void Delayms(uint16_t i) 
{
	uint16_t x;
	uint16_t y;

	// 内核72MHz，1ms
	while (i--)
	{
		x = 444; 
		while (x--)
		{
			y = 10;
			while (y--){} 
		}
	}		
}  
