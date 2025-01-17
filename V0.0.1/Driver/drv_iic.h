//-----------------------------------------------------------------------------
// drv_global.h
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
// xu.kai,QQ89362582

#ifndef __DRV_IIC_H__
#define __DRV_IIC_H__

#include "type.h"

//-----------------------------------------------------------------------------
// SPI for NZ3802
//-----------------------------------------------------------------------------
#define MIMETIC_IO_IIC_EN 1  //  =1,IOģ��IIC
                             //  =0,MCUӲ��IIC


typedef enum
{
	IIC_50KHZ =50,
    IIC_100KHZ=100,
	IIC_200KHZ=200,
	IIC_400KHZ=400,
	IIC_TOTAL
}type_iic_baudrate;

typedef enum
{
	IIC_WRITE,
	IIC_READ
}type_iic_op;
typedef enum
{
	IIC_ACK,
	IIC_NACK
}type_iic_ack;

void iic1Init(type_iic_baudrate baudrate);
bool iic1SendByte(u8 SlvAddr, u8 WriteAddr, u8 pBuffer);
bool iic1RecvByte(u8 SlvAddr, u8 WriteAddr, u8* pBuffer);

#endif



