#ifndef 	__DS1302_H__
#define 	__DS1302_H__

#include "common.h"

#define	DS1302_REFRESH_TIME				1000		// ʱ��������ڣ���λ:ms
#define	MINUTE_COUNT_OF_24_HOURS		(24*60)		// 24Сʱ�ķ�����

// DS1302��Ԫ���Կ���
#define DS1302_UNIT_TEST				0

#define DS1302_START_YEAR				1970

// DS1302����I2C�˿ڶ���(�û������Լ��˿ڽ����޸�)
#define DS1302_CE_DIR_OUT()				
#define DS1302_CE_DIR_IN()				
#define DS1302_SCLK_DIR_OUT()			
#define DS1302_SCLK_DIR_IN()			
#define DS1302_SDA_DIR_OUT()			
#define DS1302_SDA_DIR_IN() 			DS1302_SDA_HIGH()	// ��©ģʽ�����1���ɶ�ȡ�ⲿ��ƽ

#define DS1302_CE_HIGH()				
#define DS1302_CE_LOW() 				
#define DS1302_SCLK_HIGH()				
#define DS1302_SCLK_LOW()				
#define DS1302_SDA_HIGH()				
#define DS1302_SDA_LOW()				
#define DS1302_SDA_GET()				
#define DS1302_SDA_SET(i)				

// ��ʱ����(�û������Լ���Ҫ���е���)
#define	DS1302_DELAY_SWITCH 			1		// ��ʱ��ʽѡ��0:��̷�ʽ��ʱNOP()	 1:�Ӻ�����ʱDS1302_Delayus(i)
#if DS1302_DELAY_SWITCH
#define	DS1302_DELAY()					do{\
											DS1302_Delayus(5);\
										}while(0)											
#else
#define	DS1302_DELAY()					do{\
											NOP();\
											NOP();\
										}while(0)											
#endif

// �������������� ���²��ֲ��������޸� ������������������������������
#define	DS1302_REG_SECOND				0x80	// ���д��ַ
#define	DS1302_REG_MINUTE				0x82	// ���ӵ�д��ַ
#define	DS1302_REG_HOUR					0x84	// Сʱ��д��ַ
#define	DS1302_REG_DATE					0x86	// �յ�д��ַ
#define	DS1302_REG_MONTH				0x88	// �µ�д��ַ
#define	DS1302_REG_WEEK 				0x8A	// ���ڵ�д��ַ
#define	DS1302_REG_YEAR					0x8C	// ���д��ַ
#define	DS1302_REG_WP					0x8E	// д����
#define	DS1302_REG_CHARGE				0x90	// ���
#define DS1302_RAM(X)					(0xC0+(X)*2)// ���ڼ��� DS1302_RAM ��ַ�ĺ�

#define	DS1302_REG_MULTIPLE				0xBE	// ���ֽ�RTCд����
#define	DS1302_RAM_MULTIPLE				0xFE	// ���ֽ�RAMд����
#define DS1302_CHARGE_ENABLE			0xA0	// ʹ�ܳ��
#define DS1302_CHARGE_DISABLE			0x00	// ��ֹ���
#define DS1302_DIODE_NONE				0x00	// �޶����ܽ���
#define DS1302_DIODE_ONE				0x04	// һ�������ܽ���
#define DS1302_DIODE_TWO				0x08	// ���������ܽ���
#define DS1302_RES_NONE					0x00	// �޵������
#define DS1302_RES_2K					0x01	// 2K�������
#define DS1302_RES_4K					0x02	// 4K�������
#define DS1302_RES_8K					0x03	// 8K�������

#define DS1302_PROTECT_ON				0x80		// ����ʹ��
#define DS1302_PROTECT_OFF				0x00		// ������ֹ
#define DS1302_START(X)					(X&0x7F)	// ����ʱ��
#define DS1302_STOP(X)					(X|0x80)	// ֹͣʱ��
#define DS1302_HOUR_MODE_12(X)			(X|0x80)	// 12Сʱ��
#define DS1302_HOUR_MODE_24(X)			(X&0x7F)	// 24Сʱ��
#define DS1302_RD(X)					(X|0x01)	// ��ȡ����
#define DS1302_WR(X)					(X&0xFE)	// д�����

#define	DS1302_MASK_YEAR				0xFF	// ��
#define	DS1302_MASK_WEEK				0x07	// ����
#define	DS1302_MASK_MONTH				0x1F	// ��
#define	DS1302_MASK_DATE				0x3F	// ��
#define	DS1302_MASK_HOUR				0x3F	// ʱ
#define	DS1302_MASK_MINUTE				0x7F	// ��
#define	DS1302_MASK_SECOND				0x7F	// ��

typedef unsigned int time_t;
typedef struct 
{
	int tm_year; // ��ݣ���ֵ����ʵ����ݼ�ȥ1900
	int tm_mon;	 // �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11]
	int tm_mday; // һ�����е����� - ȡֵ����Ϊ[1,31]
	int tm_hour; // ʱ - ȡֵ����Ϊ[0,23]
	int tm_min;	 // �� - ȡֵ����Ϊ[0,59]
	int tm_sec;	 // �� �C ȡֵ����Ϊ[0,59]
}TM_T;

typedef struct
{
	uint16 year;
	uint8 month;
	uint8 day;
	uint8 hour;
	uint8 minute;
	uint8 second;
}TIME_E;

typedef struct
{
	TIME_E rtc;						// ʵʱʱ��

}DS1302_CB;

extern DS1302_CB ds1302CB;

// DS1302��ʼ��
void DS1302_Init(void);

// RTC������main�е���
void DS1302_Process(void);

// ����ʱ��
void DS1302_SetTime(uint8 hour, uint8 minute, uint8 second);

// ��������
uint32 DS1302_SetDate(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute, uint8 second);

#if 0
// ��������
void DS1302_SetDateCB(TIME_E* time);
#endif

// ��ȡ����
void DS1302_GetDateCB(TIME_E* time);

// ��һ���ֽ�д��RAM
void DS1302_WriteRAM(uint8 addr, uint8 data);

// ��RAM��ȡһ���ֽ�
uint8 DS1302_ReadRAM(uint8 addr);

// ����תʱ���
uint32 DS1302_GetTimeStampFromDateTime(TIME_E dataTime);

// ʱ���ת��Ϊ����
// second�Ǵ�1900��1��1�տ�ʼ�������
void DS1302_TimeStampToDateTime(uint32 second, TIME_E* dateTime);

#endif

