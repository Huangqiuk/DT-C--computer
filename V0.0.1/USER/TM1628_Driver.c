
#include "TM1628_Driver.h"
#include "Delay.h" 
                           
/***********************************************************
函数名称： TM1628_Init
函数功能： 初始化TM1628端口  
入口参数： 无
出口参数： 无
备    注： 
***********************************************************/
void TM1628_Init(void)
{
    GPIO_InitType GPIO_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA|RCC_APB2_PERIPH_GPIOB, ENABLE);
	
	GPIO_InitStruct(&GPIO_InitStructure);
    
    GPIO_InitStructure.Pin            = GPIO_PIN_0 |GPIO_PIN_2| GPIO_PIN_10;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    

    TM1628_DIO_LOW();
    TM1628_CLK_LOW();
    TM1628_STB_HIGH();  									 //端口值初始
    
    TM1628_WriteData(BITS_MODE_8, DIS_MODE_SET);  			 // 7位10段
    TM1628_STB_HIGH();
} 

/***********************************************************
函数名称： TM1628_ReadByte
函数功能： 从TM1628读取一个字节  
入口参数： 无
出口参数： 返回读取值
备    注： 在时钟的上升沿向TM16xx读数据
***********************************************************/
#ifdef KEY
UINT8 TM1628_ReadByte(void)
{
    
}
#endif

/***********************************************************
函数名称： TM1628_WriteByte
函数功能： 向TM1628写入一个字节  
入口参数： Data: 要写入数据
出口参数： 无
备    注： 在时钟的上升沿通过MCU向LED驱动IC——TM16xx写数据
***********************************************************/
#if SEGMENT_TYPE == SEGMENT_FOR_8
void TM1628_WriteByte(UINT8 Data)
{
    UINT8 i;
    
    TM1628_STB_LOW();           //保证"STB"为低电平
    
    for (i = 0; i < 8; i++)
    {
        TM1628_CLK_LOW();       //先将"CLK"清零    
        TM1628_DELAY();
        
        if ((Data & 0x01) == 0x01)
        {
            TM1628_DIO_HIGH();         
        }
        else
        {
            TM1628_DIO_LOW();          
        }
        TM1628_DELAY(); 
        TM1628_CLK_HIGH();     //时钟上升沿将数据写入
        TM1628_DELAY();        //延时至少0.1us保证数据稳定写入
        Data >>= 1;            //准备送下一个BIT
    }                          //送完一个字节后退出循环
}

/***********************************************************
函数名称： TM1628_DisplayFixAddr
函数功能： 内部地址固定模式显示数据  
入口参数： Addr:  显示地址  
           Data： 要显示的数据
出口参数： 无
备    注： 
***********************************************************/
void TM1628_DisplayFixAddr(UINT8 Addr, UINT8 Data)
{    
    TM1628_DIO_HIGH();                             //通讯开始前通讯端口全部初始化为"1"
    TM1628_CLK_HIGH();
    TM1628_STB_HIGH();
    
    TM1628_WriteByte(WRITE_MODE_ADDR_FIX);      //写模式命令,设置采用固定地址方式写显示数据
    TM1628_STB_HIGH();                             //数据设置命令传完后需要将"STB"置"1"
    TM1628_WriteByte(START_ADDR + Addr);           //写显示地址  
    //地址命令传完后，"STB"保持为"0"继续传需要显示的数据
    TM1628_WriteByte(Data);                     //写显示数据
    
    TM1628_STB_HIGH();                             //显示数据后，将"STB"置"1"
    TM1628_WriteByte(DIS_CONTROL_ALL_ON);       //传显示控制命令设置_最高亮度
    TM1628_STB_HIGH();                             //显示控制命令传完后将"STB"置"1"
}

/***********************************************************
函数名称： TM1628_DisplayAutoAddr
函数功能： 内部地址自动增加模式显示数据
入口参数： Addr:             显示的起始地址
           p_DataDisplayBuf：指向显示数据的地址
           Size:             显示数据长度
出口参数： 
备    注： 
***********************************************************/
void TM1628_DisplayAutoAddr(UINT8 Addr, UINT8 *p_DataDisplayBuf, UINT8 Size)
{
    UINT8 i; 
    
    TM1628_DIO_HIGH();                             //通讯开始前通讯端口全部初始化为"1"
    TM1628_CLK_HIGH();
    TM1628_STB_HIGH();
    
    TM1628_WriteByte(WRITE_MODE_ADDR_INC);      //写模式命令,设置采用地址自动加1方式写显示数据
    TM1628_STB_HIGH();                             //数据设置命令传完后需要将"STB"置"1"
    TM1628_WriteByte(START_ADDR + Addr);           //传要显示的起始地址
    //地址命令传完后，"STB"保持为"0"继续传需要显示的数据
    for (i = 0; i < Size; i++)
    {
        TM1628_WriteByte(*p_DataDisplayBuf++);  //在连续传显示的数据过程中,"STB"一直保持为"0"
    }
    
    TM1628_STB_HIGH();                             //传完所有的显示数据后，将"STB"置"1"
    //TM1628_WriteByte(DIS_CONTROL_ALL_ON);       //传显示控制命令设置_最高亮度
    TM1628_WriteByte(DIS_CONTROL_ALL_ON); 
    
    TM1628_STB_HIGH();                             //显示控制命令传完后将"STB"置"1"
}

#else
/***********************************************************
函数名称： TM1628_WriteData
函数功能： 向TM1628写入一个字  
入口参数： BitsMode: 一次性写入位数选择     
           Data：    要写入数据 
出口参数： 无
备    注： 在时钟的上升沿通过MCU向LED驱动IC——TM16xx写数据
           BitsMode-8位或16位
***********************************************************/
void TM1628_WriteData(UINT8 BitsMode, UINT16 Data)
{
    UINT8 i;
    
    TM1628_STB_LOW();           //保证"STB"为低电平
    
    for (i = 0; i < BitsMode; i++)
    {
        TM1628_CLK_LOW();       //先将"CLK"清零    
        TM1628_DELAY();
        
        if ((Data & 0x0001) == 0x0001)
        {
            TM1628_DIO_HIGH();         
        }
        else
        {
            TM1628_DIO_LOW();          
        }
        TM1628_DELAY(); 
        TM1628_CLK_HIGH();     //时钟上升沿将数据写入
        TM1628_DELAY();        //延时至少0.1us保证数据稳定写入
        Data >>= 1;            //准备送下一个BIT
    }                          //送完一个字节后退出循环
}

/***********************************************************
函数名称： TM1628_DisplayFixAddr
函数功能： 内部地址固定模式显示数据  
入口参数： Addr: 显示地址  
           Data：要显示的数据
出口参数： 无
备    注： 
***********************************************************/
void TM1628_DisplayFixAddr(UINT8 Addr, UINT16 Data)
{
    TM1628_DIO_HIGH();                                      //通讯开始前通讯端口全部初始化为"1"
    TM1628_CLK_HIGH();
    TM1628_STB_HIGH();
    
    TM1628_WriteData(BITS_MODE_8, WRITE_MODE_ADDR_FIX);  //写模式命令,设置采用固定地址方式写显示数据
    TM1628_STB_HIGH();                                      //数据设置命令传完后需要将"STB"置"1"
    TM1628_WriteData(BITS_MODE_8, START_ADDR + Addr);       //写显示地址  
    //地址命令传完后，"STB"保持为"0"继续传需要显示的数据
    TM1628_WriteData(BITS_MODE_16, Data);                //写显示数据
    
    TM1628_STB_HIGH();                                      //显示数据后，将"STB"置"1"
    TM1628_WriteData(BITS_MODE_8, DIS_CONTROL_ALL_ON);   //传显示控制命令设置_最高亮度
    TM1628_STB_HIGH();                                      //显示控制命令传完后将"STB"置"1"
}

/***********************************************************
函数名称： TM1628_DisplayAutoAddr
函数功能： 内部地址自动增加模式显示数据
入口参数： Addr:             显示的起始地址
           p_DataDisplayBuf：指向显示数据的地址
           Size:             显示数据长度
出口参数： 
备    注： 写满8个位数据时地址自动增1
***********************************************************/
void TM1628_DisplayAutoAddr(UINT8 Addr, UINT16 *p_DataDisplayBuf, UINT8 Size)
{
    UINT8 i; 
    
    TM1628_DIO_HIGH();                                           //通讯开始前通讯端口全部初始化为"1"
    TM1628_CLK_HIGH();
    TM1628_STB_HIGH();
    
    TM1628_WriteData(BITS_MODE_8, WRITE_MODE_ADDR_INC);       //写模式命令,设置采用地址自动加1方式写显示数据
    TM1628_STB_HIGH();                                           //数据设置命令传完后需要将"STB"置"1"
    TM1628_WriteData(BITS_MODE_8, START_ADDR + Addr);            //传要显示的起始地址
    //地址命令传完后，"STB"保持为"0"继续传需要显示的数据
    for (i = 0; i < Size; i++)
    {
        TM1628_WriteData(BITS_MODE_16, *p_DataDisplayBuf++);  //在连续传显示的数据过程中,"STB"一直保持为"0"
    }
    
    TM1628_STB_HIGH();                                           //传完所有的显示数据后，将"STB"置"1"
    //TM1628_WriteData(BITS_MODE_8, DIS_CONTROL_ALL_ON);        //传显示控制命令设置_最高亮度 
    TM1628_WriteData(BITS_MODE_8, DIS_CONTROL_ALL_ON);
    TM1628_STB_HIGH();                                           //显示控制命令传完后将"STB"置"1"
}


#endif 



