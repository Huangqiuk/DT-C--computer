
/**
*\*\file main.h
*\*\author Nations
*\*\version v1.0.0
*\*\copyright Copyright (c) 2022, Nations Technologies Inc. All rights reserved.
**/
#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"
#include "n32g003.h"
#include "delay.h"
#include "UartDrive.h"
#include "UartProtocol.h"
#include "Timer.h"
#include "Key.h"
#include "Keymsgprocess.h"
#include "common.h"
#include "System.h"
#include "iap.h"
#include "param.h"
#include "state.h"

extern uint8_t UID2HMI[4];
extern uint8_t KEYVALUE2HMI;

enum{//数据交互指令
	
	KEY2HMI,
	KEY_HMI_CMD_KEY,//按键命令
	KEY_HMI_CMD_NFC,//NFC命令
	KEY_HMI_CMD_CONN,//连接活性命令
	KEY_HMI_CMD_ERR,//错误命令

};

enum{//按键键值
	KEY_VALUE_ADDNFC,//增加NFC
	KEY_VALUE_DECNFC,//减少NFC
	KEY_VALUE_BLE,//配对蓝牙
	
	KEY_VALUE_TRUMPET_YES,//喇叭响
	KEY_VALUE_TRUMPET_NO,//喇叭灭
	
	KEY_VALUE_HELP_YES,//助力开
	KEY_VALUE_HELP_NO,//助力关
	
	KEY_VALUE_ADD,//加挡
	KEY_VALUE_DEC,//减挡
	
	KEY_VALUE_LIGHT,//灯
	
};


void Show_Bin(uint8_t Hex);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */
