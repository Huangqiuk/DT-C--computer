#ifndef _BUTTON_H_
#define _BUTTON_H_
#include "main.h"

//==========ADC按键枚举========
typedef enum{
	BUTTON_NULL 	 = 0x00,//无键值
	
	BUTTON_DEC 		 = 0x01,//减
	BUTTON_ADD 		 = 0x02,//加
	BUTTON_TRUMPET = 0x04,//喇叭
	BUTTON_LIGHT 	 = 0x08,//大灯
	BUTTON_ADDNFC  = 0x10,//加NFC - （+灯）
	BUTTON_DELNFC  = 0x20,//删NFC - （-灯）
	BUTTON_BLE 		 = 0x40,//蓝牙 - 	（+- ）
	
	
//	ADC_BUTTON_MAX = 0xFF,
}KEY_NUM;
//--------------ADC按键阈值-------------
	//加
#define	BUTTON_ADD_MAX	393
#define	BUTTON_ADD_MIN	352
	//减
#define	BUTTON_DEC_MAX	716
#define	BUTTON_DEC_MIN	648
	//大灯
#define	BUTTON_LIGHT_MAX	1716
#define	BUTTON_LIGHT_MIN	1597
	//喇叭
#define	BUTTON_TRUMPET_MAX	1362
#define	BUTTON_TRUMPET_MIN	1255
	//加NFC - （+灯）
#define	BUTTON_ADDNFC_MAX	346
#define	BUTTON_ADDNFC_MIN	310
	//删NFC - （-灯）
#define	BUTTON_DELNFC_MAX	577
#define	BUTTON_DELNFC_MIN	520
	//蓝牙 - 	（+- ）
#define	BUTTON_BLE_MAX		270
#define	BUTTON_BLE_MIN		242

//按键初始化
void Button_Init();

//读取键值
KEY_NUM ADC_Button_Read();



#endif
