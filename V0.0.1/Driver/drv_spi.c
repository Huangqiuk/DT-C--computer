//-----------------------------------------------------------------------------
// drv_spi.c
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
#include "drv_spi.h"

void SPI2DeInit(void)
{
    SPI_Enable(SPI2, DISABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_SPI2, DISABLE);
}

//-----------------------------------------------------------------------------
// spi_init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Configures SPI0 to use 3-wire Single Master mode. The SPI timing is
// configured for Mode 0,0 (data centered on first edge of clock phase and
// SCK line low in idle state).
//
//-----------------------------------------------------------------------------
void SPI2Init(teSpiBaudRate BaudRate)
{
    GPIO_InitType GPIO_InitStructure;
    SPI_InitType  SPI_InitStructure;
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_SPI2|RCC_APB2_PERIPH_AFIO|RCC_APB2_PERIPH_GPIOB|RCC_APB2_PERIPH_GPIOA, ENABLE);
	/** SPI GPIO INIT */
	//PIN-NSS
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.Pin        = GPIO_PIN_12;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	//PIN-RST
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.Pin        = GPIO_PIN_5;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
	
	//PIN-IRQ
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Input;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.Pin        = GPIO_PIN_6;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
	SetPortA6High();
	
	
	SOCKET_RF_SPI_CS_DIS();
	
	//PIN-SCK、PIN-MOSI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_SPI2;
	GPIO_InitStructure.Pin       = GPIO_PIN_13 | GPIO_PIN_15;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	
	//PIN-MISO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
	GPIO_InitStructure.Pin 			 = GPIO_PIN_14;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    /** SPI2 configuration */
    SPI_InitStructure.DataDirection = SPI_DIR_DOUBLELINE_FULLDUPLEX;
    SPI_InitStructure.SpiMode       = SPI_MODE_MASTER;
    SPI_InitStructure.DataLen       = SPI_DATA_SIZE_8BITS;
    SPI_InitStructure.CLKPOL        = SPI_CLKPOL_LOW;
    SPI_InitStructure.CLKPHA        = SPI_CLKPHA_FIRST_EDGE;
    SPI_InitStructure.NSS 		      = SPI_NSS_SOFT;
    SPI_InitStructure.FirstBit      = SPI_FB_MSB;
		
    switch(BaudRate)
    {
        case SPI_1MHZ:
            SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_64;
            break;
        case SPI_2MHZ:
            SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_32;
            break;
        case SPI_4MHZ:
            SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_16;
            break;
        case SPI_8MHZ:
            SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_8;
            break;
		case SPI_16MHZ:
            SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_4;
            break;
		case SPI_32MHZ:
            SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_2;
            break;
        default:
            SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_16;
            break;
    }

    SPI_Init(SPI2, &SPI_InitStructure);
    SPI_Enable(SPI2, ENABLE);
}

//-----------------------------------------------------------------------------
// SPI1_Byte_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Note: SPI_Data must contain the data to be sent before calling this
// function.
//
// Writes a single byte to the SPI Slave.  The slave does not respond to this
// command, so the command consists of:
//
// Command = SPI_WRITE
// Length = 1 byte of command, 1 byte of data
//
//-----------------------------------------------------------------------------
u8 SPI2TxRxByte(u8 tx_dat)
{
    SPI2->DAT = tx_dat;
    while (!(SPI2->STS & SPI_I2S_RNE_FLAG));
    return SPI2->DAT;
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------

