
#ifndef _USAR_ADC_
#define _USAR_ADC_


//电压值(ma)
extern unsigned short VOL;

//满电的电压值 3300ma
#define FULL_VOL 3300
//欠压电量值 2000ms
#define LOW_VOL 2000
//电量百分比 0-100 (uint8_t)
#define Bat_status Get_Bat_Status()
//低电量百分比
#define BAT_LOW_PER 10

#if (LOW_VOL >= FULL_VOL)
#error "LOW_VOL NOT SUPPORT GREATER THAN FULL_VOL"
#endif



//初始化ADC
void saadc_init(void);
//ADC过程处理
void ADC_Process();
//获取电量值
unsigned int Get_ADC();

//获取电量百分比
//0-100
unsigned char Get_Bat_Status();

#endif