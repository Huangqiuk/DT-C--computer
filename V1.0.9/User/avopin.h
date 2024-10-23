#ifndef     __AVO_PIN_H__
#define     __AVO_PIN_H__

#include "common.h"

// ��
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

// ����/ɲ��
#define THROTTLE_GND_TEST_EN_ON()               gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7); gpio_bit_set(GPIOC, GPIO_PIN_7)   // ����GND��ͨ����
#define THROTTLE_VCC_TEST_EN_ON()               gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15); gpio_bit_set(GPIOA, GPIO_PIN_15)  // ���ŵ�ѹ����
#define BRAKE_GND_TEST_EN_ON()                  gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7); gpio_bit_set(GPIOD, GPIO_PIN_7)   // ɲ��GND��ͨ����
#define BRAKE_VCC_TEST_EN_ON()                  gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6); gpio_bit_set(GPIOC, GPIO_PIN_6)   // ɲ�ѵ�ѹ����

// POWER/VLK
#define DUT_POWER_TEST_EN_ON()                  gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1); gpio_bit_set(GPIOD, GPIO_PIN_1)   // DUT_POWER��ѹ����
#define DUT_VLK_TEST_EN_ON()                    gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5); gpio_bit_set(GPIOD, GPIO_PIN_5)   // DUT_VLK��ѹ����

// USB���ز���
#define DUT_USB_EN1_ON()                  gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4); gpio_bit_set(GPIOE, GPIO_PIN_4)   // USB 1A����
#define DUT_USB_EN2_ON()                  gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5); gpio_bit_set(GPIOE, GPIO_PIN_5)   // USB 0.5A����
#define DUT_USB_EN3_ON()                  gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6); gpio_bit_set(GPIOE, GPIO_PIN_6)   // USB 0.5A����

#define DUT_USB_EN1_OFF()                 gpio_bit_reset(GPIOE, GPIO_PIN_4)   // USB 1A����
#define DUT_USB_EN2_OFF()                 gpio_bit_reset(GPIOE, GPIO_PIN_5)   // USB 0.5A����
#define DUT_USB_EN3_OFF()                 gpio_bit_reset(GPIOE, GPIO_PIN_6)   // USB 0.5A����

// UART����
#define DUT_TO_DTA_OFF_ON()               gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8); gpio_bit_set(GPIOA, GPIO_PIN_8)
#define UART_TEST_EN_ON()                 gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0); gpio_bit_set(GPIOD, GPIO_PIN_0)

// VLK_PW_EN
#define VLK_PW_EN_ON()                 gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7); gpio_bit_set(GPIOB, GPIO_PIN_7)
#define VLK_PW_EN_OFF()                gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7); gpio_bit_reset(GPIOB, GPIO_PIN_7)

// ��������
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

// ���ӱ���
#define DERAILLEUR_GND_TEST_EN_ON()                  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1); gpio_bit_set(GPIOA, GPIO_PIN_1)   // ɲ��GND��ͨ����
#define DERAILLEUR_VCC_TEST_EN_ON()                  gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3); gpio_bit_set(GPIOC, GPIO_PIN_3)   // ɲ��GND��ͨ����

// ����λ��
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

// �������ų�ʼ��
void AVO_PIN_Init(void);

// �������Ÿ�λ
void AVO_PIN_Reset(void);

// DAC��ʼ��
void DAC_Init(void);

// ����DAC���
void DAC0_output(uint16_t value);

// ɲ��DAC���
void DAC1_output(uint16_t value);

// ����ʱ
void Delayus(uint16 i);

// ����ʱ
void Delayms(uint16 i);

// DACʧ��
void DAC_Uinit(void);

// USBӲ��ADC��ʼ��
void ADC_HwInit(void);

// ��ȡUSB�����ѹ
uint16_t USB_ADC_Read(void);

// ��ȡUSB���ص�����������
uint16_t USB_CURRENT_Read(void);

// ��ȡwake up�����ѹ
uint16_t KEY_ADC_Read(void);

// ��Դʹ�ܡ��˷��߱�VLK����
void _5VOUT_EN(uint8_t level);
#endif
