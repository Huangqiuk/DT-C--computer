#ifndef _KEY_H_
#define _KEY_H_
#include "main.h"
#include "Keymsgprocess.h"

#define KEY_FILTER_LEVEL_N				5	// 按键滤波等级，取值范围1-7

/*-----------------------------------
开机键 - KEY_M - GPIO1

--------------------------------------*/
#define KEY_M GPIO_NUM_1 //开机键

 
// 按键缓存
typedef struct
{
	uint8_t buff[KEY_NAME_MAX];

	uint8_t lastValue[KEY_NAME_MAX];
}KEY_CB;

extern KEY_CB keyCB;

void KEY_Init(void);

void KEY_CALLBACK_Scan(uint8_t* p8bitKeyValueArray);




#endif
