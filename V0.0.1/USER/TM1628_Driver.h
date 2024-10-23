#ifndef __TM1628_DRIVER_H_
#define __TM1628_DRIVER_H_

/* Includes File*/
#include "common.h"
#include "delay.h"

#define TM1628_DELAY()				Delayus(2)

//��дģʽ����,�̶���ַ���ַ�Զ�����
#define START_ADDR                  0xC0    //��ʾ����ʼ��ַ
#define WRITE_MODE_ADDR_INC         0x40    //дģʽ,�Զ���ַ����
#define READ_KEY_MODE               0x42    //��ģʽ,������ֵ
#define WRITE_MODE_ADDR_FIX         0x44    //дģʽ,�̶���ַ

//��ʾģʽ����
#define DIS_MODE_SET                0x03    // 7λ10��

//��ʾ��������
#define DIS_CONTROL_ALL_ON          0x8F    //��ʾ����,�������ȫ��
#define DIS_CONTROL_ALL_OFF         0x80    //��ʾ����,ȫ��
#define DIS_CONTROL_LIGHT_LEVEL_1   0x88    //��ʾ����,���ȼ���1/16
#define DIS_CONTROL_LIGHT_LEVEL_2   0x89    //��ʾ����,���ȼ���2/16
#define DIS_CONTROL_LIGHT_LEVEL_4   0x8A    //��ʾ����,���ȼ���4/16
#define DIS_CONTROL_LIGHT_LEVEL_10  0x8B    //��ʾ����,���ȼ���10/16
#define DIS_CONTROL_LIGHT_LEVEL_11  0x8C    //��ʾ����,���ȼ���11/16
#define DIS_CONTROL_LIGHT_LEVEL_12  0x8D    //��ʾ����,���ȼ���12/16
#define DIS_CONTROL_LIGHT_LEVEL_13  0x8E    //��ʾ����,���ȼ���13/16
#define DIS_CONTROL_LIGHT_LEVEL_14  0x8F    //��ʾ����,���ȼ���14/16

//��ʾ��ַ��������
#define DISPLAY_ADDR_0              0xC0    //��ʾ����0
#define DISPLAY_ADDR_1              0xC1    //��ʾ����1
#define DISPLAY_ADDR_2              0xC2    //��ʾ����2
#define DISPLAY_ADDR_3              0xC3    //��ʾ����3
#define DISPLAY_ADDR_4              0xC4    //��ʾ����4
#define DISPLAY_ADDR_5              0xC5    //��ʾ����5
#define DISPLAY_ADDR_6              0xC6    //��ʾ����6
#define DISPLAY_ADDR_7              0xC7    //��ʾ����7
#define DISPLAY_ADDR_8              0xC8    //��ʾ����8
#define DISPLAY_ADDR_9              0xC9    //��ʾ����9
#define DISPLAY_ADDR_A              0xCA    //��ʾ����10
#define DISPLAY_ADDR_B              0xCB    //��ʾ����11
#define DISPLAY_ADDR_C              0xCC    //��ʾ����12
#define DISPLAY_ADDR_D              0xCD    //��ʾ����13
#define DISPLAY_ADDR_E              0xCE    //��ʾ����14
#define DISPLAY_ADDR_F              0xCF    //��ʾ����15

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

//TM1628��������
extern void TM1628_Init(void);
extern UINT8 TM1628_ReadByte(void);
extern void TransformSegToDisBuf(UINT8 Buf1, UINT8 Buf2, UINT8 Buf3, UINT8 Buf4, UINT8 Buf5, UINT8 Buf6);

#if SEGMENT_TYPE == SEGMENT_FOR_8
    //8������ܺ�������
    extern void TM1628_WriteByte(UINT8 Data);
    extern void TM1628_DisplayFixAddr(UINT8 Addr, UINT8 Data);
    extern void TM1628_DisplayAutoAddr(UINT8 Addr, UINT8 *p_DataDisplayBuf, UINT8 Size);
#else
    //16������ܺ�������
    extern void TM1628_WriteData(UINT8 BitsMode, UINT16 Data);
    extern void TM1628_DisplayFixAddr(UINT8 Addr, UINT16 Data);
    extern void TM1628_DisplayAutoAddr(UINT8 Addr, UINT16 *p_DataDisplayBuf, UINT8 Size);
#endif

#endif
