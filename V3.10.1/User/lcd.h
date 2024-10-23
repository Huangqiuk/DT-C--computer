//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HADDR[27:26]         选择的存储块        对应基址
//       00         存储块1 NOR/PSRAM 1     0x6000 0000
//       01         存储块1 NOR/PSRAM 2     0x6400 0000
//       10         存储块1 NOR/PSRAM 3     0x6800 0000
//       11         存储块1 NOR/PSRAM 4     0x6C00 0000

// FSMC提供了所有的LCD控制器的信号：
// FSMC_D[16:0]16bit的数据总线
// FSMC NEx：分配给NOR的256M，再分为4个区，每个区用来分配一个外设，这四个外设的片选分为是NE1-NE4，
//           对应的引脚为：PD7—NE1，PG9—NE2，PG10-NE3，PG12—NE4，
//           连接LCD的■CS■脚
// FSMC NOE：输出使能，连接LCD的■RD■脚。
// FSMC NWE：写使能，连接LCD的■RW■脚。
// FSMC  Ax：用在LCD显示RAM和寄存器之间进行选择的地址线，即该线用于选择LCD的■RS■脚，
//          该线可用地址线的任意一根线，范围：8位数据-FSMC_A[25:0]，16位数据-FSMC_A[24:0]
//          ■■注：RS = 0时，表示读写寄存器；RS = 1表示读写数据RAM■■
// 举例1：选择NOR的第一个存储区，并且使用FSMC_A16来控制LCD的RS引脚，
// 则我们访问LCD显示RAM的基址为0x6002 0000，访问LCD寄存器的地址为：0x6000 0000。
// 因为数据长度为16bit ,所以FSMC_A[24:0]对应HADDR[25:1]  
// 所以显示RAM的基址=0x60000000+2^16*2=0x6000 0000+0x2 0000=0x6002 0000
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __LCD_H__
#define __LCD_H__

#include "common.h"

// LCD单元测试开关
#define SPI_LCD_UNIT_TEST				0

#define LCD_WIDTH						128
#define LCD_HEIGHT						160

#define LCD_RST_H()						gpio_bit_set(GPIOB, GPIO_PIN_0)
#define LCD_RST_L()						gpio_bit_reset(GPIOB, GPIO_PIN_0)

#define LCD_BL_ON()						gpio_bit_set(GPIOA, GPIO_PIN_8)
#define LCD_BL_OFF()					gpio_bit_reset(GPIOA, GPIO_PIN_8)

#define LCD_COLOR_RED					0xF800	// 红色
#define LCD_COLOR_GREEN					0x07E0	// 绿色
#define LCD_COLOR_BLUE					0x001F	// 蓝色
#define LCD_COLOR_WHITE					0xFFFF	// 白色

#define LCD_COLOR_BLACK					(0x0000)
#define LCD_COLOR_BACKGROUND			(0xE73C)

#define LCD_COLOR_MAPPING_G1			(0x8410)
#define LCD_COLOR_MAPPING_G2			(0x059E)

// 显示数据和指令的起始地址
#define LCD_CMD							*((volatile uint16*)0x60000000)
#define LCD_DAT							*((volatile uint16*)0x60020000)
//#define LCD_DAT							*((volatile uint16*)0x6001FFFE)

// 操作模式
typedef enum
{
	LCD_OPERATOR_MODE_DRAW = 0,				// 如实绘制
	LCD_OPERATOR_MODE_BACKGROUND,			// 背景色
	LCD_OPERATOR_MODE_ALL_WHITE,			// 全白
	LCD_OPERATOR_MODE_ALL_BLACK,			// 全黑
}LCD_OPERATOR_MODE_E;

typedef struct
{
	uint16 x;
}LCD_CB;

extern LCD_CB lcdCB;

void LCD_FSMCInit(void);


//LCD IO状态初始化
void LCD_CtrlPortInit(void);

void LCD_DrawPixel(uint16 x,uint16 y,uint16 color);

void LCD_Init(void);

//LCD 处理过程函数
void LCD_Process(void);

// 在指定位置显示指定的图片
void LCD_DrawPic(uint16 x, uint16 y, const uint16* pData);

void LCD_FillColor(uint16 color);

// 显示指定位置的图片数据
void LCD_DrawPicFromAddress(uint16 x, uint16 y, uint16 w, uint16 h, uint32 addr);

// 在指定区域显示指定的颜色
void LCD_DrawAreaColor(uint16 x, uint16 y, uint16 w, uint16 h, uint16 data);

// 在指定位置显示指定ID的图片
void LCD_DrawBmpByID(uint16 x, uint16 y, uint8 id, uint8 mode);

// 画直线
void LCD_DrawLine(uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint16 usColor, uint16 usLineWidth);

// 从指定位置开始，打印一个字符串
void LCD_PrintString(uint16 x, uint16 y, char* pStr, uint16 color, LCD_OPERATOR_MODE_E mode);

// 在指定位置绘制正方形
void LCD_DrawRect(uint16 x, uint16 y, uint8 width, uint16 color);

// 将字符串转换成二维码并显示到指定的位置
void LCD_PrintQR(uint16 x, uint16 y);

// .动态显示
void LCD_DotDynamicShow(uint32 param);

#endif

