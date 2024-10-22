#ifndef     __USER_KEY_H__
#define     __USER_KEY_H__


#include "USER_Common.h"





#define KEY_FILTER_LEVEL_N				3	// 按键滤波等级，取值范围1-7

/*-----------------------------------
M键  -  12
+键	 -	14
-键	 -	15
--------------------------------------*/
#define KEY_ADD 14 //＋键
#define KEY_DEC 15 //-键
#define KEY_M 12 //设置键

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

