#ifndef 	__SYSTEM_H__
#define 	__SYSTEM_H__

#include "common.h"

#define	SYS_OSC 		48UL	// ϵͳʱ�ӣ���λ:MHz
#define MMU_VTOR               ((__IO unsigned*)(0x40022024))
#define _VTOREN()              (*MMU_VTOR = (*MMU_VTOR) | 0x80000000);
#define _VTORVALUE()           (*MMU_VTOR = (*MMU_VTOR) | 0x08004000);//�ж���������ӳ���ַ

// ��������
void SYSTEM_Init(void);

// ϵͳ�ܸ�λ
void SYSTEM_Rst(uint32 param);

#endif

