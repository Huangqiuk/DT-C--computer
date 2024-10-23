#ifndef __COMMON_H__
#define __COMMON_H__

#include "gd32f10x.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

// 以下为研发调试开关，发布时全部置为0

// 仪表型号
#define DEV_VERSION												"201"

// 固件版本编号(注意 : 这个编号固定为0)
#define FW_BINNUM												0

// 固件版本号
#define DEV_VERSION_REMAIN										0
#define DEV_RELEASE_VERSION										0//4
#define FW_UPDATE_VERSION											0
#define	FW_DEBUG_RELEASE_VERSION							0//1

// IAP版本号
#define IAP_VERSION												"B_CM_FW_BL_DT_BC201_V0.0.0"//"B_CM_FW_BL_DT_BC201_V4.0.1"
#define IAP_VERSION_LENGTH										(sizeof(IAP_VERSION)-1)


#define GPS_TYPE_NO						"0"
#define GPS_TYPE_YES					"1"

#define HMI_TYPE_BC18EU					"BC18EU"	// 迪太码表BC18，TFT屏，UART通讯
#define HMI_TYPE_BC18SU					"BC18SU"	// 迪太码表BC18，TFT屏，BLE，UART通讯
#define HMI_TYPE_BC28EU					"BC28EU"	// 迪太码表BC28，TFT屏，UART通讯
#define HMI_TYPE_BC28SU					"BC28SU"	// 迪太码表BC28，TFT屏，BLE，UART通讯
#define HMI_TYPE_BC28NU					"BC28NU"	// 迪太码表BC28，TFT屏，BLE，GSM，GPS，UART通讯
#define HMI_TYPE_BC18EC					"BC18EC"	// 天腾码表BC18，TFT屏，CAN通讯
#define HMI_TYPE_BC28EC					"BC28EC"	// 天腾码表BC28，TFT屏，CAN通讯
#define HMI_TYPE_BCxxxx					"BCxxxx"	// 通用

// Typedef definitions
typedef signed char         int8;
typedef unsigned char       uint8;
typedef signed short        int16;
typedef unsigned short      uint16;
typedef signed long         int32;
typedef unsigned long       uint32;
typedef signed long long    int64;    // 有符号64位整型变量
typedef unsigned long long  uint64;   // 无符号64位整型变量
typedef float               fp32;     // 单精度浮点数(32位长度)  
typedef double              fp64;     // 双精度浮点数(64位长度) 

#ifndef NULL
#define	NULL				0
#endif

#define UINT32_NULL	(0xFFFFFFFF)
#define UINT16_NULL	(0xFFFF)
#define UINT8_NULL	(0xFF)


// 布尔变量定义
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


//宏定义与常量定义
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

