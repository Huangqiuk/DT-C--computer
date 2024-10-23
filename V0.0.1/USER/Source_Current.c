#include "Source_Current.h"
#include "Source_Adc.h"
#include "Source_Relay.h"

//芯片CC6902SO-5A的巾帼的电流计算方法如下：
//在没有电流测量时，OUT脚输出电压为2.5V，此时电流为0。以此为基准，每增加400mv就电流就增加1A
//根据电路原理OUT输出的电压有经过与运放前端电阻的分压，所以必须进行比例换算。
//返回值单位：mA,  输入channel范围:  0或者1
float PucsSource_CurRead(uint8_t channel)
{
  float current=0;
  float Vout=0;
  uint16_t adc=0;
  if(channel>1)
	  return 0;	
  if(channel==0)  
    adc=Get_AdcValue(ADC_CURRENT0);	
  else
	adc=Get_AdcValue(ADC_CURRENT1);	  
  //计算芯片OUT端电压值
  Vout=adc/4096.0*3.0*2.0;
  //计算电流值，2.5为每1mv对应2.5ma电流
  current=(Vout*1000.0-2500.0)*2.5;

  return current;
}

