#ifndef 	__SYSTEM_H__
#define 	__SYSTEM_H__

#include "common.h"

#define	SYS_OSC 		48UL	// ϵͳʱ�ӣ���λ:MHz

// ��������
void SYSTEM_Init(void);

// ϵͳ�ܸ�λ
void SYSTEM_Rst(uint32 param);

#endif

