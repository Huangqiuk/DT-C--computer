#ifndef __TM1628_DRIVER_H_
#define __TM1628_DRIVER_H_

/* Includes File*/
#include "common.h"
#include "delay.h"

#define TM1628_DELAY()				Delayus(2)

//读写模式设置,固定地址或地址自动增加
#define START_ADDR                  0xC0    //显示的起始地址
#define WRITE_MODE_ADDR_INC         0x40    //写模式,自动地址增加
#define READ_KEY_MODE               0x42    //读模式,读按键值
#define WRITE_MODE_ADDR_FIX         0x44    //写模式,固定地址

//显示模式设置
#define DIS_MODE_SET                0x03    // 7位10段

//显示命令设置
#define DIS_CONTROL_ALL_ON          0x8F    //显示控制,最大亮度全开
#define DIS_CONTROL_ALL_OFF         0x80    //显示控制,全关
#define DIS_CONTROL_LIGHT_LEVEL_1   0x88    //显示控制,亮度级别1/16
#define DIS_CONTROL_LIGHT_LEVEL_2   0x89    //显示控制,亮度级别2/16
#define DIS_CONTROL_LIGHT_LEVEL_4   0x8A    //显示控制,亮度级别4/16
#define DIS_CONTROL_LIGHT_LEVEL_10  0x8B    //显示控制,亮度级别10/16
#define DIS_CONTROL_LIGHT_LEVEL_11  0x8C    //显示控制,亮度级别11/16
#define DIS_CONTROL_LIGHT_LEVEL_12  0x8D    //显示控制,亮度级别12/16
#define DIS_CONTROL_LIGHT_LEVEL_13  0x8E    //显示控制,亮度级别13/16
#define DIS_CONTROL_LIGHT_LEVEL_14  0x8F    //显示控制,亮度级别14/16

//显示地址命令设置
#define DISPLAY_ADDR_0              0xC0    //显示控制0
#define DISPLAY_ADDR_1              0xC1    //显示控制1
#define DISPLAY_ADDR_2              0xC2    //显示控制2
#define DISPLAY_ADDR_3              0xC3    //显示控制3
#define DISPLAY_ADDR_4              0xC4    //显示控制4
#define DISPLAY_ADDR_5              0xC5    //显示控制5
#define DISPLAY_ADDR_6              0xC6    //显示控制6
#define DISPLAY_ADDR_7              0xC7    //显示控制7
#define DISPLAY_ADDR_8              0xC8    //显示控制8
#define DISPLAY_ADDR_9              0xC9    //显示控制9
#define DISPLAY_ADDR_A              0xCA    //显示控制10
#define DISPLAY_ADDR_B              0xCB    //显示控制11
#define DISPLAY_ADDR_C              0xCC    //显示控制12
#define DISPLAY_ADDR_D              0xCD    //显示控制13
#define DISPLAY_ADDR_E              0xCE    //显示控制14
#define DISPLAY_ADDR_F              0xCF    //显示控制15

#define TM1628_DIO_HIGH()			GPIO_SetBits(GPIOB, GPIO_PIN_0)		
#define TM1628_DIO_LOW()			GPIO_ResetBits(GPIOB, GPIO_PIN_0)

#define TM1628_CLK_HIGH()			GPIO_SetBits(GPIOB, GPIO_PIN_10)	
#define TM1628_CLK_LOW()			GPIO_ResetBits(GPIOB, GPIO_PIN_10)

#define TM1628_STB_HIGH()			GPIO_SetBits(GPIOB, GPIO_PIN_2)	
#define TM1628_STB_LOW()			GPIO_ResetBits(GPIOB, GPIO_PIN_2)

#define TM1628_DIO_IN()				GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_0)                                                            

#define BITS_MODE_8                 8
#define BITS_MODE_16                16

#define SEGMENT_FOR_8               1
#define SEGMENT_FOR_16              2
#define SEGMENT_TYPE                SEGMENT_FOR_16

//TM1628函数声明
extern void TM1628_Init(void);
extern UINT8 TM1628_ReadByte(void);
extern void TransformSegToDisBuf(UINT8 Buf1, UINT8 Buf2, UINT8 Buf3, UINT8 Buf4, UINT8 Buf5, UINT8 Buf6);

#if SEGMENT_TYPE == SEGMENT_FOR_8
    //8段数码管函数声明
    extern void TM1628_WriteByte(UINT8 Data);
    extern void TM1628_DisplayFixAddr(UINT8 Addr, UINT8 Data);
    extern void TM1628_DisplayAutoAddr(UINT8 Addr, UINT8 *p_DataDisplayBuf, UINT8 Size);
#else
    //16段数码管函数声明
    extern void TM1628_WriteData(UINT8 BitsMode, UINT16 Data);
    extern void TM1628_DisplayFixAddr(UINT8 Addr, UINT16 Data);
    extern void TM1628_DisplayAutoAddr(UINT8 Addr, UINT16 *p_DataDisplayBuf, UINT8 Size);
#endif

#endif
