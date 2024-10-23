//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HADDR[27:26]         ѡ��Ĵ洢��        ��Ӧ��ַ
//       00         �洢��1 NOR/PSRAM 1     0x6000 0000
//       01         �洢��1 NOR/PSRAM 2     0x6400 0000
//       10         �洢��1 NOR/PSRAM 3     0x6800 0000
//       11         �洢��1 NOR/PSRAM 4     0x6C00 0000

// FSMC�ṩ�����е�LCD���������źţ�
// FSMC_D[16:0]16bit����������
// FSMC NEx�������NOR��256M���ٷ�Ϊ4������ÿ������������һ�����裬���ĸ������Ƭѡ��Ϊ��NE1-NE4��
//           ��Ӧ������Ϊ��PD7��NE1��PG9��NE2��PG10-NE3��PG12��NE4��
//           ����LCD�ġ�CS����
// FSMC NOE�����ʹ�ܣ�����LCD�ġ�RD���š�
// FSMC NWE��дʹ�ܣ�����LCD�ġ�RW���š�
// FSMC  Ax������LCD��ʾRAM�ͼĴ���֮�����ѡ��ĵ�ַ�ߣ�����������ѡ��LCD�ġ�RS���ţ�
//          ���߿��õ�ַ�ߵ�����һ���ߣ���Χ��8λ����-FSMC_A[25:0]��16λ����-FSMC_A[24:0]
//          ����ע��RS = 0ʱ����ʾ��д�Ĵ�����RS = 1��ʾ��д����RAM����
// ����1��ѡ��NOR�ĵ�һ���洢��������ʹ��FSMC_A16������LCD��RS���ţ�
// �����Ƿ���LCD��ʾRAM�Ļ�ַΪ0x6002 0000������LCD�Ĵ����ĵ�ַΪ��0x6000 0000��
// ��Ϊ���ݳ���Ϊ16bit ,����FSMC_A[24:0]��ӦHADDR[25:1]  
// ������ʾRAM�Ļ�ַ=0x60000000+2^16*2=0x6000 0000+0x2 0000=0x6002 0000
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __LCD_H__
#define __LCD_H__

#include "common.h"

// LCD��Ԫ���Կ���
#define SPI_LCD_UNIT_TEST				0

#define LCD_WIDTH						128
#define LCD_HEIGHT						160

#define LCD_RST_H()						gpio_bit_set(GPIOB, GPIO_PIN_0)
#define LCD_RST_L()						gpio_bit_reset(GPIOB, GPIO_PIN_0)

#define LCD_BL_ON()						gpio_bit_set(GPIOA, GPIO_PIN_8)
#define LCD_BL_OFF()					gpio_bit_reset(GPIOA, GPIO_PIN_8)

#define LCD_COLOR_RED					0xF800	// ��ɫ
#define LCD_COLOR_GREEN					0x07E0	// ��ɫ
#define LCD_COLOR_BLUE					0x001F	// ��ɫ
#define LCD_COLOR_WHITE					0xFFFF	// ��ɫ

#define LCD_COLOR_BLACK					(0x0000)
#define LCD_COLOR_BACKGROUND			(0xE73C)

#define LCD_COLOR_MAPPING_G1			(0x8410)
#define LCD_COLOR_MAPPING_G2			(0x059E)

// ��ʾ���ݺ�ָ�����ʼ��ַ
#define LCD_CMD							*((volatile uint16*)0x60000000)
#define LCD_DAT							*((volatile uint16*)0x60020000)
//#define LCD_DAT							*((volatile uint16*)0x6001FFFE)

// ����ģʽ
typedef enum
{
	LCD_OPERATOR_MODE_DRAW = 0,				// ��ʵ����
	LCD_OPERATOR_MODE_BACKGROUND,			// ����ɫ
	LCD_OPERATOR_MODE_ALL_WHITE,			// ȫ��
	LCD_OPERATOR_MODE_ALL_BLACK,			// ȫ��
}LCD_OPERATOR_MODE_E;

typedef struct
{
	uint16 x;
}LCD_CB;

extern LCD_CB lcdCB;

void LCD_FSMCInit(void);


//LCD IO״̬��ʼ��
void LCD_CtrlPortInit(void);

void LCD_DrawPixel(uint16 x,uint16 y,uint16 color);

void LCD_Init(void);

//LCD ������̺���
void LCD_Process(void);

// ��ָ��λ����ʾָ����ͼƬ
void LCD_DrawPic(uint16 x, uint16 y, const uint16* pData);

void LCD_FillColor(uint16 color);

// ��ʾָ��λ�õ�ͼƬ����
void LCD_DrawPicFromAddress(uint16 x, uint16 y, uint16 w, uint16 h, uint32 addr);

// ��ָ��������ʾָ������ɫ
void LCD_DrawAreaColor(uint16 x, uint16 y, uint16 w, uint16 h, uint16 data);

// ��ָ��λ����ʾָ��ID��ͼƬ
void LCD_DrawBmpByID(uint16 x, uint16 y, uint8 id, uint8 mode);

// ��ֱ��
void LCD_DrawLine(uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint16 usColor, uint16 usLineWidth);

// ��ָ��λ�ÿ�ʼ����ӡһ���ַ���
void LCD_PrintString(uint16 x, uint16 y, char* pStr, uint16 color, LCD_OPERATOR_MODE_E mode);

// ��ָ��λ�û���������
void LCD_DrawRect(uint16 x, uint16 y, uint8 width, uint16 color);

// ���ַ���ת���ɶ�ά�벢��ʾ��ָ����λ��
void LCD_PrintQR(uint16 x, uint16 y);

// .��̬��ʾ
void LCD_DotDynamicShow(uint32 param);

#endif

