#include "common.h"
#include "delay.h"

// ����ʱ
void Delayus(uint8 i)
{	
	// �ں�48MHz��1us
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
		NOP();  		
	}
}

// ����ʱ
void Delayms(uint8 i) 
{
	uint16 x;
	uint8 y;

	// �ں�48MHz��1ms
	while (i--)
	{
		x = 47; 
		while (x--)
		{
			y = 54;
			while (y--)
			{
				NOP(); 
			} 
		}
	}		
}
