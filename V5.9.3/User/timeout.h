#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__

#include "common.h"

#define TIME_OUT_ARM_CTRL           1500
#define TIME_OUT_COMMUNICATION      500
#define TIME_OUT_APP_EAR            4000
#define TIME_OUT_UI_EAR             10000
#define TIME_OUT_CONFIG_EAR         15000
#define TIME_OUT_NO_ACK_REPEAT      1000

// 通信超时处理
void communicationTimeOut(uint32 temp);

// 添加超时定时器
void addTimeOutTimer(uint32 time);

// 重发处理
void addDutNoAckRepeat(uint32 reset);

// 添加超时检测定时器
void addDutNoAckRepeatTimer(uint32 time);

#endif
