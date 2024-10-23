//-----------------------------------------------------------------------------
// drv_iic.c
//-----------------------------------------------------------------------------
// Copyright 2017 nationz Ltd, Inc.
// http://www.nationz.com
//
// Program Description:
//
// driver definitions for the nfc reader
//
// PRJ:            nfc reader
// Target:         N32G45x
// Tool chain:     KEIL
// Command Line:   None
//
// Release 1.0
//    -Initial Revision (NZ)
//    -27 Oct 2017
//    -Latest release before new firmware coding standard
//


#include "my_io_defs.h"
#include "drv_iic.h"
#include "NZ3802_com.h"

extern int my_printf(const char *fmt,...);
u16 iic1_speed = 5;
static void iic1delay(void)
{
    volatile u16 icnt = iic1_speed;
    while(icnt--);
}

static void iic1_start(void)
{
		IIC1_SET_SDA_HIGH();
		iic1delay();
		IIC1_SET_SCL_HIGH();
		iic1delay();
		IIC1_SET_SDA_LOW();
		iic1delay();
		IIC1_SET_SCL_LOW();
		iic1delay();
}
static void iic1_stop(void)
{
		IIC1_SET_SDA_LOW();
		iic1delay();
		IIC1_SET_SCL_HIGH();
		iic1delay();
		IIC1_SET_SDA_HIGH();
		iic1delay();
}
static bool iic1_check_ack(void)
{
		bool bret = FALSE;
		u16 timeout = 20000;

		IIC1_SET_SDA_HIGH();
		iic1delay();
		IIC1_SET_SCL_HIGH();
		iic1delay();

		do
		{
				if(!IIC1_CHK_SDA())
				{
					bret = TRUE;
					break;
				}
		}
		while (--timeout);
		
		IIC1_SET_SCL_LOW();
		iic1delay();
		
		return bret;
}
void iic1_tx_ack(type_iic_ack ack)
{
		if(ack==IIC_NACK)
		{
				IIC1_SET_SDA_HIGH();//NACK
    }
		else
		{
				IIC1_SET_SDA_LOW();//ACK
    }
		
		iic1delay();
		IIC1_SET_SCL_HIGH();
		iic1delay();
		IIC1_SET_SCL_LOW();
		iic1delay();
}

static bool iic1_tx_byte(u8 byte)
{
		u8 i = 8;
		bool bret;

		do
		{
				if (byte&0x80)
				{
					IIC1_SET_SDA_HIGH();
				}
				else
				{
					IIC1_SET_SDA_LOW();
				}

				byte<<=1;
				iic1delay();
				IIC1_SET_SCL_HIGH();
				iic1delay();
				IIC1_SET_SCL_LOW();
				iic1delay();
		}
		while(--i);
	
		bret = iic1_check_ack();
		return bret;
}

static u8 iic1_rx_byte(type_iic_ack ack)
{
		u8 i = 8;
		u8 byte = 0;

		IIC1_SET_SDA_HIGH();

		do
		{
				IIC1_SET_SCL_HIGH();
				iic1delay();
				byte <<= 1;
				if(IIC1_CHK_SDA())
				{
						byte |= 0x01;
				}
				iic1delay();
				IIC1_SET_SCL_LOW();
				iic1delay();
		}
		while(--i);
		
		iic1_tx_ack(ack);

		return byte;
}

void iic1Init(type_iic_baudrate baudrate)
{
		GPIO_InitType GPIO_InitStructure;
	
		/** SOFT IIC PIN Config --> SCL£ºPB6¡¢SDA£ºPB7*/
		GPIO_InitStructure.Pin        = GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
	  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    if(baudrate==IIC_50KHZ)
        iic1_speed = 30;
    else if(baudrate==IIC_100KHZ)
        iic1_speed = 14;
    else if(baudrate==IIC_200KHZ)
        iic1_speed = 6;
    else if(baudrate==IIC_400KHZ)
        iic1_speed = 1;

    IIC1_SET_SDA_HIGH();
    IIC1_SET_SCL_HIGH();
}

bool iic1SendByte(u8 SlvAddr, u8 WriteAddr, u8 pBuffer)
{
		bool bret = FALSE;

		iic1_start();
	
		if(iic1_tx_byte(SlvAddr))//address
		{
				if(iic1_tx_byte(WriteAddr))//command
				{
						bret = TRUE;
						if(!iic1_tx_byte(pBuffer))//tx data
						{
								bret = FALSE;
						}
				}
		}
		
		iic1_stop();

		return bret;
}

bool iic1RecvByte(u8 SlvAddr, u8 ReadAddr, u8* pBuffer)
{
	bool flag = FALSE;
	
	iic1_start();
	
	if (iic1_tx_byte(SlvAddr))//address
	{
			if (iic1_tx_byte(ReadAddr))//command
			{
					iic1_start();
					if(iic1_tx_byte(SlvAddr|0x01))//read cmd
					{
							*pBuffer = iic1_rx_byte(IIC_NACK);//rx data
							flag = TRUE;
					}
			}
	}
	
	iic1_stop();
    
 	return flag;
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------

