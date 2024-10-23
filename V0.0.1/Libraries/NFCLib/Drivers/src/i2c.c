#include "common.h"
#include "i2c.h"
#include <stdint.h>
#include <board.h>
#include <tool.h>

#define I2C_MASTER_ADDR   0xFF
#define I2C_SLAVE_ADDR    0x50
#define I2CT_FLAG_TIMEOUT ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))

void i2c_Init(void)
{
//	I2C_InitType i2c1_master;
//    GPIO_InitType i2c1_gpio;
//    
//    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_I2C2, ENABLE);
//    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

//    // PB8 -- SCL; PB9 -- SDA
//    i2c1_gpio.Pin               = GPIO_PIN_9 | GPIO_PIN_10;
//    i2c1_gpio.GPIO_Slew_Rate    = GPIO_Slew_Rate_High;
//    i2c1_gpio.GPIO_Mode         = GPIO_Mode_AF_OD;
//    i2c1_gpio.GPIO_Alternate    = GPIO_AF6_I2C2;
//    i2c1_gpio.GPIO_Pull         = GPIO_Pull_Up;	  
//    GPIO_InitPeripheral(GPIOA, &i2c1_gpio);

//    I2C_DeInit(I2C2);
//    i2c1_master.BusMode     = I2C_BUSMODE_I2C;
//    i2c1_master.FmDutyCycle = I2C_FMDUTYCYCLE_2;
//    i2c1_master.OwnAddr1    = I2C_MASTER_ADDR;
//    i2c1_master.AckEnable   = I2C_ACKEN;
//    i2c1_master.AddrMode    = I2C_ADDR_MODE_7BIT;
//    i2c1_master.ClkSpeed    = 100000; // 400K
//    I2C_Init(I2C2, &i2c1_master);
//    
//    I2C_Enable(I2C2, ENABLE);


    I2C_InitType i2c1_master;
    GPIO_InitType i2c1_gpio;
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_I2C2, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

    i2c1_gpio.Pin               = GPIO_PIN_10 | GPIO_PIN_9;
    i2c1_gpio.GPIO_Slew_Rate    = GPIO_Slew_Rate_High;
    i2c1_gpio.GPIO_Mode         = GPIO_Mode_AF_OD;
    i2c1_gpio.GPIO_Alternate    = GPIO_AF6_I2C2;
    i2c1_gpio.GPIO_Pull         = GPIO_Pull_Up;	  
    GPIO_InitPeripheral(GPIOA, &i2c1_gpio);

    I2C_DeInit(I2C2);
    i2c1_master.BusMode     = I2C_BUSMODE_I2C;
    i2c1_master.FmDutyCycle = I2C_FMDUTYCYCLE_2;
    i2c1_master.OwnAddr1    = I2C_MASTER_ADDR;
    i2c1_master.AckEnable   = I2C_ACKEN;
    i2c1_master.AddrMode    = I2C_ADDR_MODE_7BIT;
    i2c1_master.ClkSpeed    = 100000; // 100000 100K

    I2C_Init(I2C2, &i2c1_master);
    I2C_Enable(I2C2, ENABLE);
		
}

uint8_t i2c_Write(uint8_t slave_addr, uint8_t *buf, uint16_t u16Len)
{
    uint32_t I2CTimeout = I2CT_LONG_TIMEOUT;

    Sleep(1);
    
    while (I2C_GetFlag(I2C2, I2C_FLAG_BUSY))
    {
       if ((I2CTimeout--) == 0)
        {
					   I2C_GenerateStop(I2C2, ENABLE);
					   i2c_Init();
            return ERROR;
        }
    }

    I2C_ConfigAck(I2C2, ENABLE);

    I2C_GenerateStart(I2C2, ENABLE);
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C2, I2C_EVT_MASTER_MODE_FLAG)) // EV5
    {
        if ((I2CTimeout--) == 0)
        {
					  I2C_GenerateStop(I2C2, ENABLE);
					  i2c_Init();
            return ERROR;
        }
    }

		I2C_SendAddr7bit(I2C2, slave_addr, I2C_DIRECTION_SEND);

    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C2, I2C_EVT_MASTER_TXMODE_FLAG)) // EV6
    {
        if ((I2CTimeout--) == 0)
        {
					   I2C_GenerateStop(I2C2, ENABLE);
					   i2c_Init();
            return ERROR;
        }
    }

    // send data
    while (u16Len-- > 0)
    {
        I2C_SendData(I2C2, *buf++);
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_CheckEvent(I2C2, I2C_EVT_MASTER_DATA_SENDING)) // EV8
        {
            if ((I2CTimeout--) == 0)
            {
							   I2C_GenerateStop(I2C2, ENABLE);
							   i2c_Init();
                return ERROR;
            }
        }
    }

    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C2, I2C_EVT_MASTER_DATA_SENDED)) // EV8-2
    {
        if ((I2CTimeout--) == 0)
        {
					  I2C_GenerateStop(I2C2, ENABLE);
					  i2c_Init();
            return ERROR;
        }
    }
    I2C_GenerateStop(I2C2, ENABLE);
    
    return SUCCESS;
}

uint8_t i2c_Read(uint8_t slave_addr, uint8_t *buf, uint16_t u16Len)
{
	uint32_t I2CTimeout = I2CT_LONG_TIMEOUT;

	Sleep(1);
    
    while (I2C_GetFlag(I2C2, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
        {
					  I2C_GenerateStop(I2C2, ENABLE);
					  i2c_Init();
            return ERROR;
        }
    }

    I2C_ConfigAck(I2C2, ENABLE);

    // send start
    I2C_GenerateStart(I2C2, ENABLE);
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C2, I2C_EVT_MASTER_MODE_FLAG)) // EV5
    {
        if ((I2CTimeout--) == 0)
        {
					I2C_GenerateStop(I2C2, ENABLE);
					i2c_Init();
            return ERROR;
        }
    }

    // send addr
    I2C_SendAddr7bit(I2C2, (slave_addr), I2C_DIRECTION_RECV);
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C2, I2C_EVT_MASTER_RXMODE_FLAG)) // EV6
    {
        if ((I2CTimeout--) == 0)
        {
					I2C_GenerateStop(I2C2, ENABLE);
					i2c_Init();
            return ERROR;
        }
    }

    // recv data
    while (u16Len-- > 0)
    {
        if (u16Len == 0)
        {
            I2C_ConfigAck(I2C2, DISABLE);
            I2C_GenerateStop(I2C2, ENABLE);
        }
        
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_CheckEvent(I2C2, I2C_EVT_MASTER_DATA_RECVD_FLAG)) // EV7
        {
            if ((I2CTimeout--) == 0)
            {
							  I2C_GenerateStop(I2C2, ENABLE);
							  i2c_Init();
                return ERROR;
            }
        }
        *buf++ = I2C_RecvData(I2C2);
    }
    
    return SUCCESS;
}




