#include "Source_Current.h"
#include "Source_Adc.h"
#include "Source_Relay.h"

//оƬCC6902SO-5A�Ľ����ĵ������㷽�����£�
//��û�е�������ʱ��OUT�������ѹΪ2.5V����ʱ����Ϊ0���Դ�Ϊ��׼��ÿ����400mv�͵���������1A
//���ݵ�·ԭ��OUT����ĵ�ѹ�о������˷�ǰ�˵���ķ�ѹ�����Ա�����б������㡣
//����ֵ��λ��mA,  ����channel��Χ:  0����1
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
  //����оƬOUT�˵�ѹֵ
  Vout=adc/4096.0*3.0*2.0;
  //�������ֵ��2.5Ϊÿ1mv��Ӧ2.5ma����
  current=(Vout*1000.0-2500.0)*2.5;

  return current;
}

