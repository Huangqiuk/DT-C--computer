/*********************************************************
*Copyright (C), 2017, Shanghai Eastsoft Microelectronics Co., Ltd.
*文件名:  iic.c
*作  者:  Liut
*版  本:  V1.00
*日  期:  2017/06/26
*描  述:  IIC主机模块程序
*备  注:  适用于HRSDK-GDB-ES8P508x V1.0
          本软件仅供学习和演示使用，对用户直接引用代码所带来的风险或后果不承担任何法律责任。
**********************************************************/
#include "iic.h"
#include "common.h"
#include "delay.h"

#define IIC_DELAY() Delayus(4)

//产生ACK应答
void IIC_Ack(void)
{
	SCL_LOW();
	SDA_LOW();
  IIC_DELAY();
	SCL_HIGH();
  IIC_DELAY();
	SCL_LOW();
}

//不产生ACK应答		    
void IIC_NAck(void)
{
	SCL_LOW();
	SDA_HIGH();
	IIC_DELAY();
	SCL_HIGH();
  IIC_DELAY();
	SCL_LOW();
}
/*********************************************************
函数名: void IIC1MasterInit(void)
描  述: IIC主机初始化子程序
输入值: 无
输出值: 无
返回值: 无 
**********************************************************/
void IICMasterInit(void)
{
    GPIO_InitType GPIO_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    
    GPIO_InitStructure.Pin            = GPIO_PIN_8 | GPIO_PIN_7;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
}

/*****************************************************************************
 * @name       :void IIC_Start(void)
 * @date       :2018-09-13 
 * @function   :start iic bus
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void IIC_Start(void)
{	
	IIC_DELAY();
	SDA_HIGH();	  	  
	SCL_HIGH();
  IIC_DELAY();
 	SDA_LOW();
	IIC_DELAY();
	SCL_LOW();//钳住I2C总线，准备发送或接收数据 
}

/*****************************************************************************
 * @name       :void IIC_Stop(void)
 * @date       :2018-09-13 
 * @function   :stop iic bus
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void IIC_Stop(void)
{
	SCL_LOW();
	SDA_LOW();
	IIC_DELAY();
	SCL_HIGH(); 
	SDA_HIGH();
  IIC_DELAY();
}

/*****************************************************************************
 * @name       :void IIC_Wait_Ack(void)
 * @date       :2018-09-13 
 * @function   :wait iic ack
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
uint8 IIC_Wait_Ack(void)
{
	uint16 ucErrTime=0;
	
	SDA_DIR_IN();
	
	SDA_HIGH();
	IIC_DELAY();
	SCL_HIGH();
	IIC_DELAY(); 
	
	while(SDA_GET())
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	SCL_LOW();   
	return 0;  
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(uint8 txd)
{                        
	uint8 t;
	SDA_DIR_OUT();	
	SCL_LOW();//拉低时钟开始数据传输
	for (t = 0; t < 8; t++)
	{        
		if(0x80 == (txd & 0x80))
		{
			SDA_HIGH();
		}
		else
		{
			SDA_LOW();
		}	  
		IIC_DELAY();
		SCL_HIGH(); 
		IIC_DELAY();
		SCL_LOW();	
		IIC_DELAY();
		
		txd <<= 1;
	}	 
}
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;

	SDA_DIR_IN();
	
	for ( i = 0; i < 8; i++)
	{
		SCL_LOW(); 
		IIC_DELAY();
		SCL_HIGH();
		receive<<=1;
		if(SDA_GET())receive++;   
		IIC_DELAY(); 
	}		
	
	if (!ack)
		IIC_NAck();//发送nACK
	else
		IIC_Ack(); //发送ACK   
	return receive;
}

