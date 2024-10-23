/***********************************************************************************************
 *   driver_config.h:  Type definition Header file for NXP Family
 *   Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History:		Created on: Aug 31, 2010
 *
 ***********************************************************************************************/
#ifndef __DRIVER_CONFIG_H__
#define __DRIVER_CONFIG_H__

//#include <chip.h>
#include <board.h>

/***********************************************************************************************
 **	Global macros and definitions
 ***********************************************************************************************/
#define CONFIG_ENABLE_DRIVER_CRP                1
#define CONFIG_CRP_SETTING_NO_CRP               1

/* definitions for i2c link */
#define CONFIG_ENABLE_DRIVER_I2C                1
#define CONFIG_I2C_DEFAULT_I2C_IRQHANDLER       1

/* definitions for SPI link */
#define CONFIG_ENABLE_DRIVER_SSP                1

/* definitions for GPIO */
#define CONFIG_ENABLE_DRIVER_GPIO               1

/* definitions for UART */
#define CONFIG_ENABLE_DRIVER_UART               1
#define CONFIG_UART_DEFAULT_UART_IRQHANDLER     1
#define CONFIG_UART_ENABLE_INTERRUPT            1
#define CONFIG_UART_ENABLE_TX_INTERRUPT         1

#define CONFIG_ENABLE_DRIVER_PRINTF             1

//#define BOARD_NXPNCI_I2C_ADDR		  0x28U
#define BOARD_NXPNCI_I2C_ADDR		  0x50U


#define PORT0 0

#define PORT_IRQ              PORT0
#define PORT_VEN              PORT0
#define PIN_IRQ            	  13 // P0.13
#define PIN_VEN            	  17 // P0.17

#define SET_OUT               1
#define SET_IN                0

#define HIGH              	  1
#define LOW               	  0

/***********************************************************************************************
 **	Global variables
 ***********************************************************************************************/

/***********************************************************************************************
 **	Global function prototypes
 ***********************************************************************************************/

#endif // __DRIVER_CONFIG_H__

/***********************************************************************************************
 **                            End Of File
 ***********************************************************************************************/
