/*********************************************************
*Copyright (C), 2017, Shanghai Eastsoft Microelectronics Co., Ltd.
*�ļ���:  iic.h
*��  ��:  Liut
*��  ��:  V1.00
*��  ��:  2017/06/26
*��  ��:  IIC�ӻ�ģ��ͷ�ļ�
*��  ע:  ������HRSDK-GDB-ES8P508x V1.0
          ���������ѧϰ����ʾʹ�ã����û�ֱ�����ô����������ķ��ջ������е��κη������Ρ�
**********************************************************/
#ifndef __IICUSER_H__
#define __IICUSER_H__

#include "common.h"

//#define SLAVE_ADDR  0x5A    //�ӻ���ַ

extern void i2c_Init(void);
extern uint8_t i2c_Read(uint8_t slave_addr, uint8_t *buf, uint16_t size );
extern uint8_t i2c_Write(uint8_t slave_addr, uint8_t *buf, uint16_t size);

#endif
