#ifndef __COMMON_H__
#define __COMMON_H__

#include "n32g003.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

// ����Ϊ�з����Կ��أ�������ʱȫ����Ϊ0��
#define __SYSTEM_DEBUG__								1		// �з����Կ���
#define __SYSTEM_NO_TIME_OUT_ERROR__					0
// ͨѶ��ʱ���ο��أ�Ϊ1ʱ����ʾͨѶ��ʱ

// APP�汾��
#define APP_VERSION_1         			0
#define APP_VERSION_2         			0
#define APP_VERSION_3         			0
  
//boot�汾��
#define BOOT_VERSION_1					0
#define BOOT_VERSION_2					0
#define BOOT_VERSION_3					2

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

typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned long       UINT32;

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

// λ����
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

#define CHECK_PARAM_OVER_SAME_RETURN_VALUE(a, b, c)	{\
	if(a >= b)							\
	{									\
		return c;						\
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

