#ifndef     __KEY_H__
#define     __KEY_H__

#include "keyMsgProcess.h"

#define KEY_FILTER_LEVEL_N				3	// �����˲��ȼ���ȡֵ��Χ1-7

// ��������
typedef struct
{
	uint8 buff[KEY_NAME_MAX];

	uint8 lastValue[KEY_NAME_MAX];
}KEY_CB;


void KEY_Init(void);

void KEY_CALLBACK_Scan(uint8* p8bitKeyValueArray);

#endif

