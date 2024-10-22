#include "myiic.h"

 //PA.6 --- SCK ---(SPI.MISO)
 //PA.4 --- SDA ---(SPI.CSN)
 #define SLA_ADDR 0x56
//初始化IIC
void IIC_Init(void)
{					     

		GPIO_InitType GPIO_InitStructure;
    RCC_APB_Peripheral_Clock_Enable(RCC_APB_PERIPH_IOPA);  
    GPIO_Structure_Initialize(&GPIO_InitStructure);
		
		GPIO_InitStructure.Pin            = SCL_PIN | SDA_PIN;
    GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_OUT_OD;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_SLEW_RATE_FAST;
	
    GPIO_Peripheral_Initialize(I2C_PORT, &GPIO_InitStructure); 

/*
		// 中断引脚初始化
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);          ///<Enable AFIO clock
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);  ///<Connect EXTI9 Line to PA7 pin
		
		EXTI_StructInit(&EXTI_InitStructure);
		
		EXTI_InitStructure.EXTI_Line = EXTI_Line7;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);                               ///<Configure EXTI9 line
		
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);                               ///<Enable and set EXTI9_5 Interrupt to the lowest priority
*/
}

extern uint8_t Si522_IRQ_flag ;
extern uint8_t Si522_IRQ_flagA ;

// 外部中断处理
/*
void EXTI9_5_IRQHandler(void)
{
	if (RESET != EXTI_GetITStatus(EXTI_Line7))
	{     
	
			Si522_IRQ_flag = 1;		// Set flag		
		Si522_IRQ_flagA = 1;		// Set flag			
			EXTI_ClearITPendingBit(EXTI_Line7);
	}
}
*/
//SDA线输入输出模式设置函数
void IIC_SDA_MODE(uint8_t IO_MODE)
{
}


//产生IIC起始信号
void IIC_Start(void)
{
	SDA_DIR_OUT();     //sda线输出
	delay_us(2);
	SDA_HIGH();	  	  
	SCL_HIGH();
	delay_us(4);
 	SDA_LOW();//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	SCL_LOW();//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_DIR_OUT();  //sda线输出
	SCL_LOW();
	SDA_HIGH();//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	SCL_HIGH(); 
	SDA_HIGH();//发送I2C总线结束信号
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_DIR_IN();        //SDA设置为输入  
	SDA_HIGH();delay_us(1);	   
	SCL_HIGH();delay_us(1);	 
	while(SDA_GET())
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	SCL_LOW();//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	SCL_LOW();
	SDA_DIR_OUT();  
	SDA_LOW();
	delay_us(2);
	SCL_HIGH();
	delay_us(2);
	SCL_LOW();
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	SCL_LOW();
	SDA_DIR_OUT();  
	SDA_HIGH();
	delay_us(2);
	SCL_HIGH();
	delay_us(2);
	SCL_LOW();
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_DIR_OUT();   	    
    SCL_LOW();//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
//        IIC_SDA=(txd&0x80)>>7;
		if((txd&0x80)>>7)
			SDA_HIGH();
		else
			SDA_LOW();
		txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		SCL_HIGH();
		delay_us(2); 
		SCL_LOW();	
		delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_DIR_IN();  //SDA设置为输入
    for(i=0;i<8;i++ )
	{
        SCL_LOW(); 
        delay_us(2);
		SCL_HIGH();
        receive<<=1;
        if(SDA_GET())receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}

void SI522_I2C_LL_WriteRawRC(unsigned char RegAddr,unsigned char value)
{
	IIC_Start();  				 
	IIC_Send_Byte(SLA_ADDR);     	//发送写器件指令	 
	IIC_Wait_Ack();	   
  IIC_Send_Byte(RegAddr);   			//发送寄存器地址
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(value);     		//发送值					   
	IIC_Wait_Ack();  		    	   
  IIC_Stop();						//产生一个停止条件 	
}


unsigned char SI522_I2C_LL_ReadRawRC(unsigned char RegAddr)
{
	uint8_t RegVal=0;
	IIC_Start(); 
 	
	IIC_Send_Byte(SLA_ADDR);	//发送写器件指令	 
	RegVal = IIC_Wait_Ack();
	
    IIC_Send_Byte(RegAddr);   		//发送寄存器地址
	RegVal = IIC_Wait_Ack(); 
	
	IIC_Start();  	 	   		//重新启动
	IIC_Send_Byte(SLA_ADDR + 1);	//发送读器件指令	
	
	RegVal = IIC_Wait_Ack();	   
    RegVal = IIC_Read_Byte(0);		//读取一个字节,不继续再读,发送NAK 	  	
    IIC_Stop();					//产生一个停止条件 
	return RegVal;
}






















