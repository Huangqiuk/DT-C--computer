#ifndef 	__SYSTEM_H__
#define 	__SYSTEM_H__

#include "common.h"

#define	SYS_OSC 		48UL	// 系统时钟，单位:MHz
#define MMU_VTOR               ((__IO unsigned*)(0x40022024))
#define _VTOREN()              (*MMU_VTOR = (*MMU_VTOR) | 0x80000000);
#define _VTORVALUE()           (*MMU_VTOR = (*MMU_VTOR) | 0x08004000);//中断向量表重映射地址

// 函数声明
void SYSTEM_Init(void);

// 系统总复位
void SYSTEM_Rst(uint32 param);

#endif

