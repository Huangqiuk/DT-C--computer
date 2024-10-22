#include <stdio.h>
#include "SI522_interface.h"
#include "myiic.h"


void I_SI522_IO_Init(void)
{
    IIC_Init();
}
void I_SI522_IO_Write(unsigned char RegAddr,unsigned char value)
{
    SI522_I2C_LL_WriteRawRC(RegAddr,value);
}
unsigned char I_SI522_IO_Read(unsigned char RegAddr)
{
    return SI522_I2C_LL_ReadRawRC(RegAddr);
}


