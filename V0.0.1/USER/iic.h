/*********************************************************
*Copyright (C), 2017, Shanghai Eastsoft Microelectronics Co., Ltd.
*文件名:  iic.h
*作  者:  Liut
*版  本:  V1.00
*日  期:  2017/06/26
*描  述:  IIC从机模块头文件
*备  注:  适用于HRSDK-GDB-ES8P508x V1.0
          本软件仅供学习和演示使用，对用户直接引用代码所带来的风险或后果不承担任何法律责任。
**********************************************************/
#ifndef __IICUSER_H__
#define __IICUSER_H__

#include "common.h"

#define SLAVE_ADDR  0x78    //从机地址

//--------------IIC总线引脚定义-----------------------
#define OLED_SDA_GPIO      	 			GPIOB  //OLED屏IIC数据信号
#define OLED_SCL_GPIO       			GPIOB  //OLED屏IIC时钟信号
#define OLED_SDA      	 				GPIO_PIN_7  //OLED屏IIC数据信号
#define OLED_SCL       					GPIO_PIN_8  //OLED屏IIC时钟信号

#define SDA_DIR_OUT()
#define SDA_DIR_IN()					SDA_HIGH()	// 开漏模式，输出1后即可读取外部电平

#define SCL_HIGH()						GPIO_SetBits(OLED_SCL_GPIO, OLED_SCL)
#define SCL_LOW()						GPIO_ResetBits(OLED_SCL_GPIO, OLED_SCL)
#define SDA_HIGH()						GPIO_SetBits(OLED_SDA_GPIO, OLED_SDA)
#define SDA_LOW()						GPIO_ResetBits(OLED_SDA_GPIO, OLED_SDA)

#define SDA_GET()						GPIO_ReadInputDataBit(GPIOB,OLED_SDA)

// SDA引脚模式
typedef enum
{
	SDA_IN = 0,
	SDA_OUT
}SDA_PIN_MODE;

//IIC操作函数
void IICMasterInit(void);
//产生IIC起始信号 //START:when CLK is high,DATA change form high to low 
void IIC_Start(void);
//产生IIC停止信号 //STOP:when CLK is high DATA change form low to high
void IIC_Stop(void);
//等待应答信号到来
uint8 IIC_Wait_Ack(void);
//产生ACK应答
void IIC_Ack(void);
//不产生ACK应答		    
void IIC_NAck(void);
//IIC发送一个字节	  
void IIC_Send_Byte(uint8 txd);
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8 IIC_Read_Byte(unsigned char ack);
////OLED写命令
//void Write_IIC_Command(uint8 OLED_Command);
////OLED写数据,这个写数据是指要传输数据
//void Write_IIC_Data(uint8 OLED_Data);

#endif

