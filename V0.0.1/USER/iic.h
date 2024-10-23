/*********************************************************
*Copyright (C), 2017, Shanghai Eastsoft Microelectronics Co., Ltd.
*�ļ���:  iic.h
*��  ��:  Liut
*��  ��:  V1.00
*��  ��:  2017/06/26
*��  ��:  IIC�ӻ�ģ��ͷ�ļ�
*��  ע:  ������HRSDK-GDB-ES8P508x V1.0
          ���������ѧϰ����ʾʹ�ã����û�ֱ�����ô����������ķ��ջ������е��κη������Ρ�
**********************************************************/
#ifndef __IICUSER_H__
#define __IICUSER_H__

#include "common.h"

#define SLAVE_ADDR  0x78    //�ӻ���ַ

//--------------IIC�������Ŷ���-----------------------
#define OLED_SDA_GPIO      	 			GPIOB  //OLED��IIC�����ź�
#define OLED_SCL_GPIO       			GPIOB  //OLED��IICʱ���ź�
#define OLED_SDA      	 				GPIO_PIN_7  //OLED��IIC�����ź�
#define OLED_SCL       					GPIO_PIN_8  //OLED��IICʱ���ź�

#define SDA_DIR_OUT()
#define SDA_DIR_IN()					SDA_HIGH()	// ��©ģʽ�����1�󼴿ɶ�ȡ�ⲿ��ƽ

#define SCL_HIGH()						GPIO_SetBits(OLED_SCL_GPIO, OLED_SCL)
#define SCL_LOW()						GPIO_ResetBits(OLED_SCL_GPIO, OLED_SCL)
#define SDA_HIGH()						GPIO_SetBits(OLED_SDA_GPIO, OLED_SDA)
#define SDA_LOW()						GPIO_ResetBits(OLED_SDA_GPIO, OLED_SDA)

#define SDA_GET()						GPIO_ReadInputDataBit(GPIOB,OLED_SDA)

// SDA����ģʽ
typedef enum
{
	SDA_IN = 0,
	SDA_OUT
}SDA_PIN_MODE;

//IIC��������
void IICMasterInit(void);
//����IIC��ʼ�ź� //START:when CLK is high,DATA change form high to low 
void IIC_Start(void);
//����IICֹͣ�ź� //STOP:when CLK is high DATA change form low to high
void IIC_Stop(void);
//�ȴ�Ӧ���źŵ���
uint8 IIC_Wait_Ack(void);
//����ACKӦ��
void IIC_Ack(void);
//������ACKӦ��		    
void IIC_NAck(void);
//IIC����һ���ֽ�	  
void IIC_Send_Byte(uint8 txd);
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8 IIC_Read_Byte(unsigned char ack);
////OLEDд����
//void Write_IIC_Command(uint8 OLED_Command);
////OLEDд����,���д������ָҪ��������
//void Write_IIC_Data(uint8 OLED_Data);

#endif

