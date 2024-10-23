#ifndef     __LCD2004_H__
#define     __LCD2004_H__

#include "common.h"

#define comm 0
#define dat 1
#define uchar uint8_t
#define MAX_ONE_LINES_LENGTH 20      // 一行显示字符的最大长度
#define MAX_TWO_LINES_LENGTH 40      // 两行显示字符的最大长度
#define MAX_THREE_LENGTH_LENGTH 60   // 三行显示字符的最大长度
#define MAX_ALL_LENGTH 80            // 四行显示字符的最大长度
#define MAX_LINES 4                  // 最大行数

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

//#define WR_DATA(data)  GPIO_OCTL(GPIOE) = ((uint32_t)data<<8);// 左赋值给高8位

/* 函数声明 ---------------------------------------------------------------------------------------------------------*/
// LCD初始化
void Init_Lcd(void);

// LCD写
void wr_lcd(uint8_t dat_cmd, uint8_t content);

// 指定y坐标显示
void Display_Y(uint8_t y, const char *code);

// 指定Y和X坐标显示
void Display_YX(uint8_t y, uint8_t x, const char *code);

// 指定Y坐标并居中显示
void Display_Centered(uint8_t y, const char *code);

// 显示前80个字符
void Display_Max(const char *code);

// 从第Y行第X列开始显示字符串
void Display_YX_Max(uint8_t y, uint8_t x, const char *code);

// 在LCD显示屏上指定Y,X坐标显示字符串、整数或浮点数
void Display_YX_Format(uint8_t y, uint8_t x, const char* format, ...);

// 在LCD显示屏上指定Y坐标显示居中的字符串、整数或浮点数
void Display_Centered_Format(uint8_t y, const char* format, ...);

// 清空当前行
void Clear_Line(uint8_t y);

// 清空所有行
void Clear_All_Lines(void);

// 打印开机信息
void POWER_ON_Information(void);

// 垂直滚动显示函数，在LCD屏幕上逐行显示消息数组的内容
void Vertical_Scrolling_Display(char (* const Msg)[MAX_ONE_LINES_LENGTH], uchar numLines, uchar currentStartLine);

// 向消息数组指定位置添加字符串
void addString(uchar line, uchar column, const char *str, char Msg[MAX_LINES][MAX_ONE_LINES_LENGTH]);

void addFormattedString(uchar line, uchar column, const char *format, char Msg[MAX_LINES][MAX_ONE_LINES_LENGTH], ...);

void WR_DATA(uint32_t data);
#endif
