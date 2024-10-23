//-----------------------------------------------------------------------------
// drv_uart.h
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

#ifndef __DRV_UART_H_
#define __DRV_UART_H_

#include "type.h"

//-----------------------------------------------------------------------------
// UART for printf
//-----------------------------------------------------------------------------

typedef enum
{
	UART_9600   = 9600,
	UART_19200  = 19200,
	UART_38400  = 38400,
	UART_115200 = 115200
} teUartBaudRate;

//NZ3802
void UART1Open(teUartBaudRate baudrate);
void UART1Close(void);
void UART1Enable(void);
void UART1Disable(void);
void UART1PutChar(u8 ch);
void UART1PutString(char *str);
u8   UART1GetChar(void);

//PRINTF
void UART2Open(u32 baudrate);
void UART2Close(void);
void UART2Enable(void);
void UART2Disable(void);
void UART2PutChar(u8 ch);
void UART2PutString(char *str);
u8   UART2GetChar(void);

#endif



