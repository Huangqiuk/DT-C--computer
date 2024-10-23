//-----------------------------------------------------------------------------
// drv_uart.c
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
#include "drv_uart.h"

void UART1Enable(void)
{
    USART_Enable(USART1, ENABLE);
}

void UART1Disable(void)
{
    USART_Enable(USART1, DISABLE);
}

/** 
  * @function UART1Open()
  * @param    baudrate
  */
void UART1Open(teUartBaudRate baudrate)
{
    USART_InitType USART_InitStructure;
    NVIC_InitType NVIC_InitStructure;
    GPIO_InitType GPIO_InitStructure;

	  /** Configure USART1 Tx (PA.9) Rx (PA.10) */
	  GPIO_InitStructure.Pin 			  = GPIO_PIN_9;
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
	
  	GPIO_InitStructure.Pin 			 = GPIO_PIN_10;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
  	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
	
		/** USART1 INIT */
		USART_InitStructure.BaudRate   					= baudrate;
		USART_InitStructure.WordLength 					= USART_WL_8B;
		USART_InitStructure.StopBits   					= USART_STPB_1;
		USART_InitStructure.Parity     					= USART_PE_NO;
		USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
		USART_InitStructure.Mode                = USART_MODE_TX | USART_MODE_RX;
		USART_Init(USART1, &USART_InitStructure);
		
		/** Interrupt	Init */
		USART_ConfigInt(USART1, USART_INT_RXDNE, ENABLE);
		USART_ConfigInt(USART1, USART_INT_IDLEF, ENABLE);
		USART_ClrFlag(USART1, (USART_INT_TXDE | USART_INT_TXC | USART_INT_RXDNE));
    NVIC_ClearPendingIRQ(USART1_IRQn);
		NVIC_InitStructure.NVIC_IRQChannel 									 = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			 = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd								 = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		USART_Enable(USART1, ENABLE);
}

/** 
  * @function UART1Close()
  */
void UART1Close(void)
{
		GPIO_InitType GPIO_InitStructure;
		NVIC_InitType NVIC_InitStructure;

		USART_Enable(USART1, DISABLE);
			
		RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_USART1, DISABLE);
		GPIO_InitStructure.Pin 			 = GPIO_PIN_9 | GPIO_PIN_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

		NVIC_InitStructure.NVIC_IRQChannel 					  = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd         = DISABLE;
		NVIC_Init(&NVIC_InitStructure);
		USART_ConfigInt(USART1, USART_INT_RXDNE, DISABLE);
}

/** UART1PutChar()
  * @brief  uart output 1byte
  * @param  ch
  * @retval null
  */
void UART1PutChar(u8 ch)
{
    //USART1->DAT = ch;
   // while(!(USART1->STS & USART_FLAG_TXC));
	UART_DRIVE_BC_SendData(ch);
}

void UART1PutString(char *str)
{
    while(*str!=NULL)
    {
        UART1PutChar(*str++);
    }
}


u8 UART1GetChar(void)
{
    u32 tm_out=0x17FFF;
    u8 ch;

    while((!(USART1->STS & USART_FLAG_RXDNE))&&(tm_out--));
    ch = USART1->DAT;

    return ch;
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------


