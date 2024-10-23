#ifndef     __KEY_H__
#define     __KEY_H__

#include "keyMsgProcess.h"
#include "common.h"

#define KEY_FILTER_LEVEL_N              3   // 按键滤波等级，取值范围1-7

#define KEY_CHECK_RCU_PORT RCU_GPIOC
#define KEY_CHECK_PORT     GPIOC
#define KEY_CHECK_PIN      GPIO_PIN_13

// 按键缓存
typedef struct
{
    uint8 buff[KEY_NAME_MAX];

    uint8 lastValue[KEY_NAME_MAX];
} KEY_CB;


void KEY_Init(void);

void KEY_CALLBACK_Scan(uint8 *p8bitKeyValueArray);

#endif

