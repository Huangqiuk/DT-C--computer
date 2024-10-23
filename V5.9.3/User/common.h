#ifndef __COMMON_H__
#define __COMMON_H__

#include "gd32f10x.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// APP版本号
#define APP_VERSION_1         			5
#define APP_VERSION_2         			9
#define APP_VERSION_3         			2
  
//boot版本号
#define BOOT_VERSION_1					0
#define BOOT_VERSION_2					0
#define BOOT_VERSION_3					2 	//0.0.2
 
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
#define	NULL				 0
#endif

#define UINT32_NULL	(0xFFFFFFFF)
#define UINT16_NULL	(0xFFFF)
#define UINT8_NULL	(0xFF)

#define SYS_OK		(0)


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

#endif

