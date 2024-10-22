
#ifndef _USAR_LED_
#define _USAR_LED_

#include "USER_Common.h"


#define USER_LED1 16
#define USER_LED2 18
#define USER_LED3 20

//��ʼ����ƣ��̻Ƶ�
#define INIT_LED_R	nrf_gpio_cfg_output(USER_LED1)
#define INIT_LED_GY	nrf_gpio_cfg_output(USER_LED2)

////������
//#define CLOSE_LED_R	   nrf_gpio_pin_clear(USER_LED1)
//#define  OPEN_LED_R		 nrf_gpio_pin_set(USER_LED1)
////�����̻Ƶ�
//#define CLOSE_LED_GY	  nrf_gpio_pin_clear(USER_LED2)
//#define  OPEN_LED_GY		nrf_gpio_pin_set(USER_LED2)

//������
#define CLOSE_LED_R	   nrf_gpio_pin_set(USER_LED1)
#define  OPEN_LED_R		 nrf_gpio_pin_clear(USER_LED1)
//�����̻Ƶ�
#define CLOSE_LED_GY	  nrf_gpio_pin_set(USER_LED2)
#define  OPEN_LED_GY		nrf_gpio_pin_clear(USER_LED2)


typedef enum{
	
	LED_RED,
	LED_YG,

	LED_MAX,
}LED_NAME;

// LED������
typedef enum
{
	LED_MODE_PAIR,//���ģʽ
	LED_MODE_CONNECT,//����ģʽ
	LED_MODE_AWAKE,//����ģʽ
	LED_MODE_LOW_POWER,//�͵���ģʽ

	LED_MOD_MAX
}LED_MODE_NAME;

//���� - 0 ����һ��

//���
#define LED_MODE_PAIR_on 30//��ʱ��
#define LED_MODE_PAIR_off 1000//��ʱ��
#define LED_MODE_PAIR_Count 1000//����
//����
#define LED_MODE_CONNECT_on 2000
#define LED_MODE_CONNECT_off 0
#define LED_MODE_CONNECT_Count 0
//����
#define LED_MODE_AWAKE_on 200
#define LED_MODE_AWAKE_off 1000
//#define LED_MODE_AWAKE_Count 2
#define LED_MODE_AWAKE_Count 0

#define LED_MODE_LOW_POWER_on 30
#define LED_MODE_LOW_POWER_off  10000
#define LED_MODE_LOW_POWER_Count 10000


//LED��ʼ��
void Led_Init();
// ����LED ģʽ
void LED_SetMode(LED_NAME led_name, LED_MODE_NAME ledMode);
//ֹͣ��ǰLED
void Led_Uninit();

#endif
