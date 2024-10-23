//-----------------------------------------------------------------------------
// drv_spi.h
//-----------------------------------------------------------------------------

#ifndef __DRV_SPI_H_
#define __DRV_SPI_H_

#include "type.h"

typedef enum
{
	SPI_1MHZ=1,
	SPI_2MHZ=2,
	SPI_4MHZ=4,
	SPI_8MHZ=8,
	SPI_16MHZ=16,
	SPI_32MHZ=32,
	SPI_TOTAL
} teSpiBaudRate;

void SPI2DeInit(void);
void SPI2Init(teSpiBaudRate BaudRate);
u8 SPI2TxRxByte(u8 tx_dat);

#endif



