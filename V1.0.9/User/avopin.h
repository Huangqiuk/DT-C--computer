#ifndef     __AVO_PIN_H__
#define     __AVO_PIN_H__

#include "common.h"

// 灯
#define LED_VCC_TEST_EN_ON()            gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11); gpio_bit_set(GPIOD, GPIO_PIN_11)
#define LED_HBEAM_TEST_EN_ON()          gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12); gpio_bit_set(GPIOD, GPIO_PIN_12)
#define LED_LBEAM_TEST_EN_ON()          gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13); gpio_bit_set(GPIOD, GPIO_PIN_13)
#define LEFT_VCC_TEST_EN_ON()           gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3); gpio_bit_set(GPIOD, GPIO_PIN_3)
#define RIGHT_VCC_TEST_EN_ON()          gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4); gpio_bit_set(GPIOD, GPIO_PIN_4)
 
#define LED_VCC_EN_ON()               gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12); gpio_bit_set(GPIOB, GPIO_PIN_12)
#define LED_HBEAM_EN_ON()             gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13); gpio_bit_set(GPIOB, GPIO_PIN_13)
#define LED_LBEAM_EN_ON()             gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14); gpio_bit_set(GPIOB, GPIO_PIN_14)
#define LEFT_VCC_EN_ON()              gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15); gpio_bit_set(GPIOB, GPIO_PIN_15)
#define RIGHT_VCC_EN_ON()                gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8); gpio_bit_set(GPIOD, GPIO_PIN_8)

// 油门/刹车
#define THROTTLE_GND_TEST_EN_ON()               gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7); gpio_bit_set(GPIOC, GPIO_PIN_7)   // 油门GND导通测试
#define THROTTLE_VCC_TEST_EN_ON()               gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15); gpio_bit_set(GPIOA, GPIO_PIN_15)  // 油门电压测试
#define BRAKE_GND_TEST_EN_ON()                  gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7); gpio_bit_set(GPIOD, GPIO_PIN_7)   // 刹把GND导通测试
#define BRAKE_VCC_TEST_EN_ON()                  gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6); gpio_bit_set(GPIOC, GPIO_PIN_6)   // 刹把电压测试

// POWER/VLK
#define DUT_POWER_TEST_EN_ON()                  gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1); gpio_bit_set(GPIOD, GPIO_PIN_1)   // DUT_POWER电压测试
#define DUT_VLK_TEST_EN_ON()                    gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5); gpio_bit_set(GPIOD, GPIO_PIN_5)   // DUT_VLK电压测试

// USB负载测试
#define DUT_USB_EN1_ON()                  gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4); gpio_bit_set(GPIOE, GPIO_PIN_4)   // USB 1A负载
#define DUT_USB_EN2_ON()                  gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5); gpio_bit_set(GPIOE, GPIO_PIN_5)   // USB 0.5A负载
#define DUT_USB_EN3_ON()                  gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6); gpio_bit_set(GPIOE, GPIO_PIN_6)   // USB 0.5A负载

#define DUT_USB_EN1_OFF()                 gpio_bit_reset(GPIOE, GPIO_PIN_4)   // USB 1A负载
#define DUT_USB_EN2_OFF()                 gpio_bit_reset(GPIOE, GPIO_PIN_5)   // USB 0.5A负载
#define DUT_USB_EN3_OFF()                 gpio_bit_reset(GPIOE, GPIO_PIN_6)   // USB 0.5A负载

// UART测试
#define DUT_TO_DTA_OFF_ON()               gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8); gpio_bit_set(GPIOA, GPIO_PIN_8)
#define UART_TEST_EN_ON()                 gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0); gpio_bit_set(GPIOD, GPIO_PIN_0)

// VLK_PW_EN
#define VLK_PW_EN_ON()                 gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7); gpio_bit_set(GPIOB, GPIO_PIN_7)
#define VLK_PW_EN_OFF()                gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7); gpio_bit_reset(GPIOB, GPIO_PIN_7)

// 按键测试
#define KEY_TEST_EN0_ON()                 gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15); gpio_bit_set(GPIOD, GPIO_PIN_15)
#define KEY_TEST_EN0_OFF()                gpio_bit_reset(GPIOD, GPIO_PIN_15)
#define KEY_TEST_EN1_ON()                 gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14); gpio_bit_set(GPIOD, GPIO_PIN_14)
#define KEY_TEST_EN1_OFF()                gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14); gpio_bit_reset(GPIOD, GPIO_PIN_14)
#define KEY_TEST_EN2_ON()                 gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10); gpio_bit_set(GPIOD, GPIO_PIN_10)
#define KEY_TEST_EN2_OFF()                gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10); gpio_bit_reset(GPIOD, GPIO_PIN_10)
#define KEY_TEST_EN3_ON()                 gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9); gpio_bit_set(GPIOD, GPIO_PIN_9)
#define KEY_TEST_EN3_OFF()                gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9); gpio_bit_reset(GPIOD, GPIO_PIN_9)
#define KEY_TEST_EN4_ON()                 gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11); gpio_bit_set(GPIOA, GPIO_PIN_11)
#define KEY_TEST_EN4_OFF()                gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11); gpio_bit_reset(GPIOA, GPIO_PIN_11)
#define KEY_TEST_EN5_ON()                 gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12); gpio_bit_set(GPIOA, GPIO_PIN_12)
#define KEY_TEST_EN5_OFF()                gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12); gpio_bit_reset(GPIOA, GPIO_PIN_12)
#define KEY_TEST_EN6_ON()                 gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6); gpio_bit_set(GPIOD, GPIO_PIN_6)
#define KEY_TEST_EN6_OFF()                gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6); gpio_bit_reset(GPIOD, GPIO_PIN_6)
#define KEY_TEST_EN7_ON()                 gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6); gpio_bit_set(GPIOB, GPIO_PIN_6)
#define KEY_TEST_EN7_OFF()                gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6); gpio_bit_reset(GPIOB, GPIO_PIN_6)

// 电子变速
#define DERAILLEUR_GND_TEST_EN_ON()                  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1); gpio_bit_set(GPIOA, GPIO_PIN_1)   // 刹把GND导通测试
#define DERAILLEUR_VCC_TEST_EN_ON()                  gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3); gpio_bit_set(GPIOC, GPIO_PIN_3)   // 刹把GND导通测试

// 按键位号
typedef enum
{
    KEY_NUMBER_ZERO = 0,
    KEY_NUMBER_ONE,
    KEY_NUMBER_TWO,
    KEY_NUMBER_THREE,
    KEY_NUMBER_FOUR,
    KEY_NUMBER_FIVE,
    KEY_NUMBER_SIX,
    KEY_NUMBER_SEVEN,

    KEY_NUMBER_MAX
} KEY_NUMBER;

// 测试引脚初始化
void AVO_PIN_Init(void);

// 测试引脚复位
void AVO_PIN_Reset(void);

// DAC初始化
void DAC_Init(void);

// 油门DAC输出
void DAC0_output(uint16_t value);

// 刹车DAC输出
void DAC1_output(uint16_t value);

// 简单延时
void Delayus(uint16 i);

// 简单延时
void Delayms(uint16 i);

// DAC失能
void DAC_Uinit(void);

// USB硬件ADC初始化
void ADC_HwInit(void);

// 读取USB输入电压
uint16_t USB_ADC_Read(void);

// 读取USB负载电流（毫安）
uint16_t USB_CURRENT_Read(void);

// 读取wake up输入电压
uint16_t KEY_ADC_Read(void);

// 电源使能、八方高标VLK供电
void _5VOUT_EN(uint8_t level);
#endif
