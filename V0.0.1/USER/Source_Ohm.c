#include "Source_Ohm.h"
#include "Source_Adc.h"
#include "Source_Relay.h"
//返回值，欧姆。
//channel范围：0-4
uint32_t PucsSource_OhmRead(uint8_t channel)
{
	  uint32_t OHM=0;
	  uint16_t adc=0;	
	  Source_RelayCtrl((RELAY_PURPOSE )channel,Bit_SET);
	  Delayms(100) ;
	  adc=Get_AdcValue(ADC_OHM);
	  	
	  //计算欧姆值,参考电阻改为10K
	  if(adc<4095)
	   OHM=adc*10000/(4505-adc);	
      else
       OHM=2000000;	
	   Source_RelayCtrl((RELAY_PURPOSE )channel,Bit_RESET);
	   Delayms(100) ;
      return OHM;	  
	  
}	



