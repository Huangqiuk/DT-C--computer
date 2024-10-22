
#ifndef _USAR_LED_
#define _USAR_LED_

#include "USER_Common.h"


#define USER_LED1 16
#define USER_LED2 18
#define USER_LED3 20

//初始化红灯；绿黄灯
#define INIT_LED_R	nrf_gpio_cfg_output(USER_LED1)
#define INIT_LED_GY	nrf_gpio_cfg_output(USER_LED2)

////亮灭红灯
//#define CLOSE_LED_R	   nrf_gpio_pin_clear(USER_LED1)
//#define  OPEN_LED_R		 nrf_gpio_pin_set(USER_LED1)
////亮灭绿黄灯
//#define CLOSE_LED_GY	  nrf_gpio_pin_clear(USER_LED2)
//#define  OPEN_LED_GY		nrf_gpio_pin_set(USER_LED2)

//亮灭红灯
#define CLOSE_LED_R	   nrf_gpio_pin_set(USER_LED1)
#define  OPEN_LED_R		 nrf_gpio_pin_clear(USER_LED1)
//亮灭绿黄灯
#define CLOSE_LED_GY	  nrf_gpio_pin_set(USER_LED2)
#define  OPEN_LED_GY		nrf_gpio_pin_clear(USER_LED2)


typedef enum{
	
	LED_RED,
	LED_YG,

	LED_MAX,
}LED_NAME;

// LED控制码
typedef enum
{
	LED_MODE_PAIR,//配对模式
	LED_MODE_CONNECT,//连接模式
	LED_MODE_AWAKE,//唤醒模式
	LED_MODE_LOW_POWER,//低电量模式

	LED_MOD_MAX
}LED_MODE_NAME;

//周期 - 0 工作一次

//配对
#define LED_MODE_PAIR_on 30//亮时间
#define LED_MODE_PAIR_off 1000//灭时间
#define LED_MODE_PAIR_Count 1000//周期
//连接
#define LED_MODE_CONNECT_on 2000
#define LED_MODE_CONNECT_off 0
#define LED_MODE_CONNECT_Count 0
//唤醒
#define LED_MODE_AWAKE_on 200
#define LED_MODE_AWAKE_off 1000
//#define LED_MODE_AWAKE_Count 2
#define LED_MODE_AWAKE_Count 0

#define LED_MODE_LOW_POWER_on 30
#define LED_MODE_LOW_POWER_off  10000
#define LED_MODE_LOW_POWER_Count 10000


//LED初始化
void Led_Init();
// 设置LED 模式
void LED_SetMode(LED_NAME led_name, LED_MODE_NAME ledMode);
//停止当前LED
void Led_Uninit();

#endif
