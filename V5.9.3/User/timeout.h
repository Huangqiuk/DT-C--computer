#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__

#include "common.h"

#define TIME_OUT_ARM_CTRL           1500
#define TIME_OUT_COMMUNICATION      500
#define TIME_OUT_APP_EAR            4000
#define TIME_OUT_UI_EAR             10000
#define TIME_OUT_CONFIG_EAR         15000
#define TIME_OUT_NO_ACK_REPEAT      1000

// ͨ�ų�ʱ����
void communicationTimeOut(uint32 temp);

// ��ӳ�ʱ��ʱ��
void addTimeOutTimer(uint32 time);

// �ط�����
void addDutNoAckRepeat(uint32 reset);

// ��ӳ�ʱ��ⶨʱ��
void addDutNoAckRepeatTimer(uint32 time);

#endif
