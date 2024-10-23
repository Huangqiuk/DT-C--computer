#ifndef __COMMON_H__
#define __COMMON_H__

#include "gd32f10x.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

// ����Ϊ�з����Կ��أ�����ʱȫ����Ϊ0

// �Ǳ��ͺ�
#define DEV_VERSION												"201"

// �̼��汾���(ע�� : �����Ź̶�Ϊ0)
#define FW_BINNUM												0

// �̼��汾��
#define DEV_VERSION_REMAIN										0
#define DEV_RELEASE_VERSION										0//4
#define FW_UPDATE_VERSION											0
#define	FW_DEBUG_RELEASE_VERSION							0//1

// IAP�汾��
#define IAP_VERSION												"B_CM_FW_BL_DT_BC201_V0.0.0"//"B_CM_FW_BL_DT_BC201_V4.0.1"
#define IAP_VERSION_LENGTH										(sizeof(IAP_VERSION)-1)


#define GPS_TYPE_NO						"0"
#define GPS_TYPE_YES					"1"

#define HMI_TYPE_BC18EU					"BC18EU"	// ��̫���BC18��TFT����UARTͨѶ
#define HMI_TYPE_BC18SU					"BC18SU"	// ��̫���BC18��TFT����BLE��UARTͨѶ
#define HMI_TYPE_BC28EU					"BC28EU"	// ��̫���BC28��TFT����UARTͨѶ
#define HMI_TYPE_BC28SU					"BC28SU"	// ��̫���BC28��TFT����BLE��UARTͨѶ
#define HMI_TYPE_BC28NU					"BC28NU"	// ��̫���BC28��TFT����BLE��GSM��GPS��UARTͨѶ
#define HMI_TYPE_BC18EC					"BC18EC"	// �������BC18��TFT����CANͨѶ
#define HMI_TYPE_BC28EC					"BC28EC"	// �������BC28��TFT����CANͨѶ
#define HMI_TYPE_BCxxxx					"BCxxxx"	// ͨ��

// Typedef definitions
typedef signed char         int8;
typedef unsigned char       uint8;
typedef signed short        int16;
typedef unsigned short      uint16;
typedef signed long         int32;
typedef unsigned long       uint32;
typedef signed long long    int64;    // �з���64λ���ͱ���
typedef unsigned long long  uint64;   // �޷���64λ���ͱ���
typedef float               fp32;     // �����ȸ�����(32λ����)  
typedef double              fp64;     // ˫���ȸ�����(64λ����) 

#ifndef NULL
#define	NULL				0
#endif

#define UINT32_NULL	(0xFFFFFFFF)
#define UINT16_NULL	(0xFFFF)
#define UINT8_NULL	(0xFF)


// ������������
typedef enum{
	FALSE = 0,
	TRUE  = 1,
}BOOL;

typedef enum
{
	UART_0 = 0,
	UART_1,
	UART_2,
	UART_3,
	UART_4,
	UART_5
}UART_TYPE;

typedef union {
	uint8	byte;
	struct {
			uint8	bit0:1;
			uint8	bit1:1;
			uint8	bit2:1;
			uint8	bit3:1;
			uint8	bit4:1;
			uint8	bit5:1;
			uint8	bit6:1;
			uint8	bit7:1;
	} Bits;
} UNION_BITS;  

typedef struct {
			uint8	bit0:1;
			uint8	bit1:1;
			uint8	bit2:1;
			uint8	bit3:1;
			uint8	bit4:1;
			uint8	bit5:1;
			uint8	bit6:1;
			uint8	bit7:1;
} STRUCT_BITS; 


//�궨���볣������
#define EI()    {__asm volatile ("cpsie i");}
#define DI()    {__asm volatile ("cpsid i");}
#define NOP()   {__asm volatile ("nop");}

#define	PI      3.141593f

#define BIT_MASK(bit)			(1 << bit)
#define BIT_SET(date, bit)		(date |= BIT_MASK(bit))
#define BIT_CLEAR(date, bit)	(date &= ( ~ (BIT_MASK(bit))))
#define BIT_GET(date, bit)		((date & (BIT_MASK(bit))) >> bit)
#define BIT_REVERSE(date, bit)	(date ^=  BIT_MASK(bit))


#define CHECK_PARAM_DIFF_RETURN(a, b)	{\
	if(a != b)							\
	{									\
		return;							\
	}									\
}
#define CHECK_PARAM_SAME_RETURN(a, b)	{\
	if(a == b)							\
	{									\
		return;							\
	}									\
}										
#define CHECK_PARAM_DIFF_CONTINUE(a, b)	{\
	if(a != b)							\
	{									\
		continue;							\
	}									\
}										

#define CHECK_PARAM_SAME_CONTINUE(a, b)	{\
	if(a == b)							\
	{									\
		continue;							\
	}									\
}

#define CHECK_PARAM_OVER_RETURN(a, b)	{\
	if(a > b)							\
	{									\
		return;							\
	}									\
}

#define CHECK_PARAM_OVER_SAME_RETURN(a, b)	{\
	if(a >= b)							\
	{									\
		return;							\
	}									\
}


#define CHECK_PARAM_LESS_RETURN(a, b)	{\
	if(a < b)							\
	{									\
		return;							\
	}									\
}

#define CHECK_PARAM_LESS_SAME_RETURN(a, b)	{\
	if(a <= b)							\
	{									\
		return;							\
	}									\
}

#define LIMIT_TO_MAX(a, b)	{\
	if(a > b)							\
	{									\
		a = b;							\
	}									\
}

#define LIMIT_TO_MIN(a, b)	{\
	if(a < b)							\
	{									\
		a = b;							\
	}									\
}


#endif

