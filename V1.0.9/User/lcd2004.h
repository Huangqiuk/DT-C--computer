#ifndef     __LCD2004_H__
#define     __LCD2004_H__

#include "common.h"

#define comm 0
#define dat 1
#define uchar uint8_t
#define MAX_ONE_LINES_LENGTH 20      // һ����ʾ�ַ�����󳤶�
#define MAX_TWO_LINES_LENGTH 40      // ������ʾ�ַ�����󳤶�
#define MAX_THREE_LENGTH_LENGTH 60   // ������ʾ�ַ�����󳤶�
#define MAX_ALL_LENGTH 80            // ������ʾ�ַ�����󳤶�
#define MAX_LINES 4                  // �������

// RS    RW     E
// PE7   PE2    PE3
#define E_1()  gpio_bit_set(GPIOE, GPIO_PIN_3)         
#define E_0()  gpio_bit_reset(GPIOE, GPIO_PIN_3)

#define WR_1()  gpio_bit_set(GPIOE, GPIO_PIN_2)                 
#define WR_0()  gpio_bit_reset(GPIOE, GPIO_PIN_2)

#define RS_1()  gpio_bit_set(GPIOE, GPIO_PIN_7)             
#define RS_0()  gpio_bit_reset(GPIOE, GPIO_PIN_7) 

#define LCD_ON()   gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);gpio_bit_set(GPIOB, GPIO_PIN_1)         
#define LCD_OFF()  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);gpio_bit_reset(GPIOB, GPIO_PIN_1) 

//#define WR_DATA(data)  GPIO_OCTL(GPIOE) = ((uint32_t)data<<8);// ��ֵ����8λ

/* �������� ---------------------------------------------------------------------------------------------------------*/
// LCD��ʼ��
void Init_Lcd(void);

// LCDд
void wr_lcd(uint8_t dat_cmd, uint8_t content);

// ָ��y������ʾ
void Display_Y(uint8_t y, const char *code);

// ָ��Y��X������ʾ
void Display_YX(uint8_t y, uint8_t x, const char *code);

// ָ��Y���겢������ʾ
void Display_Centered(uint8_t y, const char *code);

// ��ʾǰ80���ַ�
void Display_Max(const char *code);

// �ӵ�Y�е�X�п�ʼ��ʾ�ַ���
void Display_YX_Max(uint8_t y, uint8_t x, const char *code);

// ��LCD��ʾ����ָ��Y,X������ʾ�ַ����������򸡵���
void Display_YX_Format(uint8_t y, uint8_t x, const char* format, ...);

// ��LCD��ʾ����ָ��Y������ʾ���е��ַ����������򸡵���
void Display_Centered_Format(uint8_t y, const char* format, ...);

// ��յ�ǰ��
void Clear_Line(uint8_t y);

// ���������
void Clear_All_Lines(void);

// ��ӡ������Ϣ
void POWER_ON_Information(void);

// ��ֱ������ʾ��������LCD��Ļ��������ʾ��Ϣ���������
void Vertical_Scrolling_Display(char (* const Msg)[MAX_ONE_LINES_LENGTH], uchar numLines, uchar currentStartLine);

// ����Ϣ����ָ��λ������ַ���
void addString(uchar line, uchar column, const char *str, char Msg[MAX_LINES][MAX_ONE_LINES_LENGTH]);

void addFormattedString(uchar line, uchar column, const char *format, char Msg[MAX_LINES][MAX_ONE_LINES_LENGTH], ...);

void WR_DATA(uint32_t data);
#endif
