#ifndef __MYIIC_H
#define __MYIIC_H

#include "main.h"
//IO方向设置
//#define SDA_IN()  {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=(u32)8<<16;}
//#define SDA_OUT() {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=(u32)3<<16;}

//IO操作函数	 
//#define IIC_SCL    PAout(4) 		//SCL
//#define IIC_SDA    PAout(5) 		//SDA	 
//#define READ_SDA   PAin(5) 	    //输入SDA 

#define delay_us(a)   //Delayus(a)
#define delay_ms(a)   Delayms(a) 



#define I2C_PORT GPIOA
#define SCL_PIN  GPIO_PIN_4
#define SDA_PIN  GPIO_PIN_5


#define SDA_DIR_OUT()                                //IIC_SDA_MODE(0)  
#define SDA_DIR_IN()					 SDA_HIGH()	// 开漏模式，输出1后即可读取外部电平

#define SCL_HIGH()					GPIO_Pins_Set(I2C_PORT, SCL_PIN)
#define SCL_LOW()						GPIO_Pins_Reset(I2C_PORT, SCL_PIN)
#define SDA_HIGH()					GPIO_Pins_Set(I2C_PORT, SDA_PIN)
#define SDA_LOW()						GPIO_Pins_Reset(I2C_PORT, SDA_PIN)

#define SDA_GET()						GPIO_Input_Pin_Data_Get(I2C_PORT,SDA_PIN)


//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  

void SI522_I2C_LL_WriteRawRC(unsigned char RegAddr,unsigned char value);
unsigned char SI522_I2C_LL_ReadRawRC(unsigned char RegAddr);
#endif





