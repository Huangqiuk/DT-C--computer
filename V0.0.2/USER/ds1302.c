include "common.h"
#include "ds1302.h"
#include "timer.h"
#include "param.h"
#include "record.h"

DS1302_CB ds1302CB;

// ����ʱ
void DS1302_Delayus(uint8 i);

// ����ʱ
void DS1302_Delayms(uint8 i);

// ��Ԫ����
void DS1302_UnitTest(void);

// ���ֽ�дDS1302��8��RTC�Ĵ�����RAM��Ԫ
void DS1302_BurstWrite(uint8 *pBuf, uint8 Size, uint8 RegFlag);

// ���ֽڶ�DS1302��RTC�Ĵ�����RAM��Ԫ
void DS1302_BurstRead(uint8 *pBuf, uint8 Size, uint8 RegFlag);

// �ص����������������Ե����ø�������
void DS1302_CALLBACK_SetUpdateRequest(uint32 param);

// 16����תѹ��BCD��
uint8 DS1302_HexToBCD(uint8 data);

// ѹ��BCD��ת16����
uint8 DS1302_BCDToHex(uint8 data);

// �����жϺ���
int isleap(int year);

// ʱ������ʱ���
time_t DS1302_mktime(TM_T dt);

// ʱ���ת����ʱ��
void DS1302_localtime(time_t time, TM_T*t);

/*********************************************************
* ����������
*********************************************************/
// ����ʱ
void DS1302_Delayus(volatile uint8 i)
{	 
	// �ں�72MHz��1us
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

// ����ʱ
void DS1302_Delayms(volatile uint8 i)
{	 
	volatile uint16 x;
	volatile uint8 y;

	// �ں�72MHz��1ms
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

// ��Ԫ����
void DS1302_UnitTest(void)
{
#if DS1302_UNIT_TEST
	
	//DS1302_SetDate(2019,12,04,15,03,00);
	//DS1302_SetDate(2099,12,31,23,59,59);
	//DS1302_SetDate(2000,1,1,0,0,0);
	//DS1302_SetDate(2019,2,29,0,0,0);
	//DS1302_SetDate(2100,2,28,0,0,0);
	//while (1)
	{
		TIME_E date1 = {2000,1,1,0,0,0};
		TIME_E date2 = {2019,12,06,12,12,12};
		TIME_E date3 = {2099,12,30,1,1,1};
		uint32 time1,time2,time3;
		
		time1 = DS1302_GetTimeStampFromDateTime(date1);
		time2 = DS1302_GetTimeStampFromDateTime(date2);
		time3 = DS1302_GetTimeStampFromDateTime(date3);

		DS1302_TimeStampToDateTime(time1, &date3);
		DS1302_TimeStampToDateTime(time2, &date2);
		DS1302_TimeStampToDateTime(time3, &date1);
	}	
	
#endif
}

// DS1302��ʼ��
void DS1302_Init(void)
{
	// �ϵ��ʼ��ʱ��
	ds1302CB.rtc.year = DS1302_START_YEAR;	
	ds1302CB.rtc.month = 0x01;
	ds1302CB.rtc.day = 0x01;
	ds1302CB.rtc.hour = 0x00;	
	ds1302CB.rtc.minute = 0x00;
	ds1302CB.rtc.second = 0x00;
	
	// ������ʱ�����������Եؼ���ʱ��	
	TIMER_AddTask(TIMER_ID_RTC_READ,
				  DS1302_REFRESH_TIME,
				  DS1302_CALLBACK_SetUpdateRequest,
				  0,
				  TIMER_LOOP_FOREVER,
				  ACTION_MODE_DO_AT_ONCE);

#if DS1302_UNIT_TEST
	DS1302_UnitTest();
#endif
}

// RTC������main�е���
void DS1302_Process(void)
{
	// ����ǰ����ʱ����ӵ�������
	PARAM_SetRTC(ds1302CB.rtc.year,
				ds1302CB.rtc.month,
				ds1302CB.rtc.day,
				ds1302CB.rtc.hour,
				ds1302CB.rtc.minute,
				ds1302CB.rtc.second);
}

// ����ʱ��
void DS1302_SetTime(uint8 hour, uint8 minute,  uint8 second)
{
    // ʱ���ʽ����
    if ((hour > 23) || (minute > 59) || (second > 59))
    {
        return;
    }
    
	ds1302CB.rtc.hour = hour;
	ds1302CB.rtc.minute = minute;
	ds1302CB.rtc.second = second;

	/*
	// д���µ�ʱ��
	DS1302_WriteRegister(DS1302_REG_WP, DS1302_PROTECT_OFF);	// Open Write Protect bit
	
	DS1302_WriteRegister(DS1302_REG_SECOND, DS1302_START(DS1302_HexToBCD(second) & DS1302_MASK_SECOND));	    // д��
	
	DS1302_WriteRegister(DS1302_REG_MINUTE, DS1302_HexToBCD(minute) & DS1302_MASK_MINUTE);	            		// д����
	
	DS1302_WriteRegister(DS1302_REG_HOUR, DS1302_HOUR_MODE_24(DS1302_HexToBCD(hour)) & DS1302_MASK_HOUR);   	// дСʱ(24Сʱ��)
	
	DS1302_WriteRegister(DS1302_REG_WP, DS1302_PROTECT_ON);		// Close Write Protect bit
	*/
}

// ��������
uint32 DS1302_SetDate(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute, uint8 second)
{
	uint8 dayCompare = 31;
	
    // ʱ���ʽ����
    if ((year > 2099 ) || (month > 12) || (day > 31) || (hour > 23) || (minute > 59) || (second > 59))
    {
        return UINT32_NULL;
    }

	
	 // ���ݲ�ͬ���·��ж����õ������Ƿ����Ҫ��
	if (2 == month)
	{
		// ����2��29��
		if ((0 == (year % 400)) 
			|| ((0 == (year % 4)) && (0 != (year % 100))))
		{
			dayCompare = 29;
		}
		else
		{
			dayCompare = 28;
		}
	}
	else if ((4 == month) 
			|| (6 == month)
			|| (9 == month)
			|| (11 == month))
	{
		dayCompare = 30;
	}
	else
	{
		dayCompare = 31;
	}

	// ������ʽ����
	if (day > dayCompare)
	{
		return UINT32_NULL;
	}

    ds1302CB.rtc.year = year;
	ds1302CB.rtc.month = month;
	ds1302CB.rtc.day = day;
	ds1302CB.rtc.hour = hour;
	ds1302CB.rtc.minute = minute;
	ds1302CB.rtc.second = second;

	return 0;
}

// ��ȡ����
void DS1302_GetDateCB(TIME_E* time)
{
	// ��μ���
	if(NULL == time)
	{
		return;
	}

	time->year = ds1302CB.rtc.year;
	time->month = ds1302CB.rtc.month;
	time->day = ds1302CB.rtc.day;
	time->hour = ds1302CB.rtc.hour;
	time->minute = ds1302CB.rtc.minute;
	time->second = ds1302CB.rtc.second;
}

// �ص����������������Ե����ø������󣬶�ʱ������
void DS1302_CALLBACK_SetUpdateRequest(uint32 param)
{
	uint8 monthDays = 0;
	
	// �������
	if (60 == (++ds1302CB.rtc.second))
	{
	   ds1302CB.rtc.second = 0;
	   ds1302CB.rtc.minute ++;
	}
	else
	{
		return;
	}

	// ����Сʱ
	if (60 == ds1302CB.rtc.minute)
	{
		ds1302CB.rtc.minute = 0;
		ds1302CB.rtc.hour ++;
	}

	// ������
	if (24 == ds1302CB.rtc.hour)
	{
		ds1302CB.rtc.hour = 0;
		ds1302CB.rtc.day ++;
	}

	 // ������
	if (2 == ds1302CB.rtc.month)
	{
		// ����2��29��
		if ((0 == (ds1302CB.rtc.year % 400)) 
			|| ((0 == (ds1302CB.rtc.year % 4)) && (0 != (ds1302CB.rtc.year % 100))))
		{
			monthDays = 29;
		}
		else
		{
			monthDays = 28;
		}
	}
	else if ((4 == ds1302CB.rtc.month) 
			|| (6 == ds1302CB.rtc.month)
			|| (9 == ds1302CB.rtc.month)
			|| (11 == ds1302CB.rtc.month))
	{
		monthDays = 30;
	}
	else
	{
		monthDays = 31;
	}
	
	if (ds1302CB.rtc.day == (monthDays + 1))
	{
		ds1302CB.rtc.day = 1;
		ds1302CB.rtc.month ++;
	}

	// ������ 
	if (13 == ds1302CB.rtc.month)
	{
		ds1302CB.rtc.month = 1;
		ds1302CB.rtc.year ++;
	}

}

// 16����תѹ��BCD��
uint8 DS1302_HexToBCD(uint8 data)
{
	uint8 bcd;

	bcd = (data/10)<<4;
	bcd += data%10;

	return bcd;
}

// ѹ��BCD��ת16����
uint8 DS1302_BCDToHex(uint8 data)
{
	uint8 hex;

	hex = (data>>4)*10;
	hex += data&0x0F;

	return hex;
}

// ��һ���ֽ�д��RAM
void DS1302_WriteRAM(uint8 addr, uint8 data)
{
	// ��ַ�Ϸ����жϣ��Ϸ��ĵ�ַ��Χ��0-30����31���ֽڣ���˳���30���ж��Ƿ�ֵ
	if(addr > RECORD_PARAM_ADDR_MAX)
	{
		return;
	}

	recordCB.ds1302Addr[addr] = data;
}

// ��RAM��ȡһ���ֽ�
uint8 DS1302_ReadRAM(uint8 addr)
{
	// ��ַ�Ϸ����жϣ��Ϸ��ĵ�ַ��Χ��0-30����31���ֽڣ���˳���30���ж��Ƿ�ֵ
	if(addr > RECORD_PARAM_ADDR_MAX)
	{
		return 0;
	}

	//return DS1302_ReadRegister(DS1302_RAM(addr));

	return recordCB.ds1302Addr[addr];
}

// ����ʣ��������ȡ������·�������
void DS1302_GetMonthAndDay(uint32 nDays, BOOL IsLeapYear, uint8* nMonth, uint8* nDay)
{
	uint8 DayOfMon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};	// ƽ��ÿ���¶�Ӧ������
	int i = 0, nTempDay = 0;

	// �����ж�
	if ((NULL == nMonth) || (NULL == nDay))
	{
		return;
	}

	//ѭ����ȥ12������ÿ���µ�������ֱ��ʣ������С�ڵ���0�����ҵ��˶�Ӧ���·�
	for (i = 0; i < 12; i++)
	{
		// ��������꣬2�·����⴦��		
		if ((IsLeapYear) &&(1 == i))
		{
			DayOfMon[i] = 29; 
		}
		
		// nTemp <= 0��֤���Ѿ��ǵ����ˣ����㵱���·�������
		nTempDay = nDays - DayOfMon[i];
		if (nTempDay <= 0)
		{
			// ���㵱���·�
			nMonth[0] = i + 1;

			// ���㵱������
			if (0 == nTempDay)			//��ʾ�պ�������µ����һ�죬��ô������������µ���������
			{
				nDay[0] = DayOfMon[i];
			}
			else
			{
				nDay[0] = nDays;
			}
			
			break;
		}
		
		nDays = nTempDay;
	}
}

// ʱ���ת��Ϊ����
// second��DS1302_START_YEAR��1��1�տ�ʼ�������
void DS1302_TimeStampToDateTime(uint32 second, TIME_E* dateTime)
{

	TM_T timeTemp;
	uint32 secTemp = second;
	DS1302_localtime(secTemp,&timeTemp);

	dateTime->year = (uint16)timeTemp.tm_year;
	dateTime->month = (uint8)timeTemp.tm_mon;
	dateTime->day = (uint8)timeTemp.tm_mday;
	dateTime->hour = (uint8)timeTemp.tm_hour;
	dateTime->minute = (uint8)timeTemp.tm_min;
	dateTime->second = (uint8)timeTemp.tm_sec;
	
}
 

// ����ת��Ϊʱ���
// ��DS1302_START_YEAR/1/1/0/0/0�����ڵ�ʱ���
// ����UINT32_NULLΪ��Чֵ
uint32 DS1302_GetTimeStampFromDateTime(TIME_E dataTime)
{
	TM_T timeTemp;
	timeTemp.tm_year = (int)dataTime.year;
	timeTemp.tm_mon = (int)dataTime.month;
	timeTemp.tm_mday = (int)dataTime.day;
	timeTemp.tm_hour = (int)dataTime.hour;
	timeTemp.tm_min = (int)dataTime.minute;
	timeTemp.tm_sec = (int)dataTime.second;

	return DS1302_mktime(timeTemp);
}

int isleap(int year)
{
	return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

time_t DS1302_mktime(TM_T dt)
{
	time_t mon_yday[2][12] =
	{
		{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
		{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
	};

	time_t result;
	int i = 0;
	
	// ��ƽ��ʱ����������
	result = (dt.tm_year - 1970) * 365 * 24 * 3600 +
			 (mon_yday[isleap(dt.tm_year)][dt.tm_mon - 1] + dt.tm_mday - 1) * 24 * 3600 +
			 dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
	// �������������
	for (i = 1970; i < dt.tm_year; i++)
	{
		if (isleap(i))
		{
			result += 24 * 3600;
		}
	}
	return (result);
}


void DS1302_localtime(time_t time, TM_T*t)
{
	char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	unsigned int Pass4year;
	int hours_per_year;
	
	//ȡ��ʱ��
	t->tm_sec = (int)(time % 60);
	time /= 60;
	//ȡ����ʱ��
	t->tm_min = (int)(time % 60);
	time /= 60;
	//ȡ��ȥ���ٸ����꣬ÿ������ 1461*24 Сʱ
	Pass4year = ((unsigned int)time / (1461L * 24L));
	//�������
	t->tm_year = (Pass4year << 2) + 1970;
	//������ʣ�µ�Сʱ��
	time %= 1461L * 24L;
	//У������Ӱ�����ݣ�����һ����ʣ�µ�Сʱ��
	
	for (;;)
	{
		//һ���Сʱ��
		hours_per_year = 365 * 24;
		//�ж�����
		if ((t->tm_year & 3) == 0)
		{
			//�����꣬һ�����24Сʱ����һ��
			hours_per_year += 24;
		}
		if (time < hours_per_year)
		{
			break;
		}
		t->tm_year++;
		time -= hours_per_year;
	}
	
	//Сʱ��
	t->tm_hour = (int)(time % 24);
	//һ����ʣ�µ�����
	time /= 24;
	//�ٶ�Ϊ����
	time++;
	
	//У��������������·ݣ�����
	if ((t->tm_year & 3) == 0)
	{
		if (time > 60)
		{
			time--;
		}
		else
		{
			if (time == 60)
			{
				t->tm_mon = 2;
				t->tm_mday = 29;
				return;
			}
		}
	}
	
	//��������
	for (t->tm_mon = 0; Days[t->tm_mon] < time; t->tm_mon++)
	{
		time -= Days[t->tm_mon];
	}
	t->tm_mon += 1;
	t->tm_mday = (int)(time);
	return;
}




