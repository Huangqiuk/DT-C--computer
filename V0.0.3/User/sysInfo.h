#ifndef 	__SYSINFO_H__
#define 	__SYSINFO_H__

#include "common.h"

// 结构体定义
typedef struct
{
	struct
	{
		uint32 sn0;
		uint32 sn1;
		uint32 sn2;
		uint8 snStr[25];
	}watchUid;

	BOOL qrRegEnable;
}SYS_INFO_CB;

extern SYS_INFO_CB sysInfoCB;	

void SYS_INFO_GetSysInfo(void);

void SYS_INFO_ParamProcess(void);


#endif

