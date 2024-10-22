#ifndef SI522_INTERFACE_BUS_H__
#define SI522_INTERFACE_BUS_H__

#define SLA_ADDR 0x50

void  I_SI522_IO_Init(void);
void  I_SI522_IO_Write(unsigned char RegAddr, unsigned char value);
unsigned char  I_SI522_IO_Read(unsigned char RegAddr);

#endif
