#ifndef __SOURCE_RELAY_H__
#define __SOURCE_RELAY_H__

#include "common.h"
#include "timer.h"



//继电器结构体
typedef struct{
	uint16_t      Pin;
	GPIO_Module*  GPIOx;
    uint32_t      Clock;
}RELAY_CHL;


// 欧姆通道继电器用途
typedef enum
{
	RELAY_OHM0 =0,
	RELAY_OHM1 =1,
	RELAY_OHM2 =2,
	RELAY_OHM3 =3,
	RELAY_OHM4 =4,
    RELAY_OHM_MAX
}OHM_RELAY_CHL;

// 电压通道继电器用途
typedef enum
{
	RELAY_VIN0  =0,
	RELAY_VIN1  =1,
	RELAY_VIN2  =2,
	RELAY_VIN3  =3,
	RELAY_VIN4  =4,
	RELAY_VIN5  =5,
  	RELAY_VIN_MAX
}VIN_RELAY_CHL;

// 电压通道继电器用途
typedef enum
{
	OFF =0,
	ON  =1,
}RELAY_STA;

#define  COM_PORT  GPIOA
#define  COM_PIN   GPIO_PIN_1

#define  COM_CHL_ON()   GPIO_WriteBit(COM_PORT, COM_PIN, Bit_SET);
#define  COM_CHL_OFF()   GPIO_WriteBit(COM_PORT, COM_PIN, Bit_RESET);


void Relay_HwInit(void);
void OHM_RelayCtrl(OHM_RELAY_CHL relay,RELAY_STA OnOff);


//欧姆通道选择
void OHM_RelayCtrl(OHM_RELAY_CHL relayCh,RELAY_STA OnOff);

//电压通道选择
void VIN_RelayCtrl(VIN_RELAY_CHL relayCh,RELAY_STA OnOff);

void RelayReset(void );


#endif
