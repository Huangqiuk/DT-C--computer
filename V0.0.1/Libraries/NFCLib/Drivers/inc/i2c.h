/*********************************************************
*Copyright (C), 2017, Shanghai Eastsoft Microelectronics Co., Ltd.
*文件名:  iic.h
*作  者:  Liut
*版  本:  V1.00
*日  期:  2017/06/26
*描  述:  IIC从机模块头文件
*备  注:  适用于HRSDK-GDB-ES8P508x V1.0
          本软件仅供学习和演示使用，对用户直接引用代码所带来的风险或后果不承担任何法律责任。
**********************************************************/
#ifndef __IICUSER_H__
#define __IICUSER_H__

#include "common.h"

//#define SLAVE_ADDR  0x5A    //从机地址

extern void i2c_Init(void);
extern uint8_t i2c_Read(uint8_t slave_addr, uint8_t *buf, uint16_t size );
extern uint8_t i2c_Write(uint8_t slave_addr, uint8_t *buf, uint16_t size);

#endif
