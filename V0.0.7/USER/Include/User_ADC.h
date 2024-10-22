
#ifndef _USAR_ADC_
#define _USAR_ADC_


//��ѹֵ(ma)
extern unsigned short VOL;

//����ĵ�ѹֵ 3300ma
#define FULL_VOL 3300
//Ƿѹ����ֵ 2000ms
#define LOW_VOL 2000
//�����ٷֱ� 0-100 (uint8_t)
#define Bat_status Get_Bat_Status()
//�͵����ٷֱ�
#define BAT_LOW_PER 10

#if (LOW_VOL >= FULL_VOL)
#error "LOW_VOL NOT SUPPORT GREATER THAN FULL_VOL"
#endif



//��ʼ��ADC
void saadc_init(void);
//ADC���̴���
void ADC_Process();
//��ȡ����ֵ
unsigned int Get_ADC();

//��ȡ�����ٷֱ�
//0-100
unsigned char Get_Bat_Status();

#endif