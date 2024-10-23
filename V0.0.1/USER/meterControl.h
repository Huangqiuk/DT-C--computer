/********************************************************************************
  * 文 件 名: tempCapture.h
  * 版 本 号: 初版            
  *******************************************************************************/
#ifndef __METERCONTROL_H__
#define __METERCONTROL_H__

/********************************* 头文件************************************/
#include "common.h"
#include "timer.h"

// 接收消息长度
#define AVOMETER_REV_LENGTH_MAX     16
// 接收错误次数
#define AVOMETER_REV_ERR_CNT_MAX     3
// 滤波器尺寸
#define AVOMETER_FILTER_BUF_MAX     3
// 滤波器误差系数
#define AVOMETER_FILTER_ERR_COE    0.1
// 非法的数字代表
#define AVOMETER_REV_NUM_ERR_VAL    10000.0

/* 引脚配置结构体 */
typedef struct
{
    uint32_t Clock;             /* 时钟 */
    GPIO_Module* Port;             /* 端口 */
    uint16_t Pin;              /* 引脚 */
}AVOMETER_PT_PIN;

// 万用表功能键枚举
typedef enum{
			
	FUNTION_PIN_SELECT = 0x00,								
	FUNTION_PIN_RANGE,								
    FUNTION_PIN_RS232,	
    FUNTION_PIN_TX_EN,    
    FUNTION_PIN_MAX,
}AVOMETER_FUNTION_PIN_E;

// 万用表测试档位枚举
typedef enum{
			
	AVOMETER_GEARS_DCV = 0x01,								
	AVOMETER_GEARS_ACV,								
	AVOMETER_GEARS_DCMA,								
	AVOMETER_GEARS_ACMA,								
	AVOMETER_GEARS_OHM,						        
	AVOMETER_GEARS_DCA ,							    
	AVOMETER_GEARS_ACA ,							    
	AVOMETER_GEARS_MAX,
}AVOMETER_GEARS_CMD;


typedef enum
{
    AVOMETER_SYMBOL_CHAR = 0,
    AVOMETER_SYMBOL_P1,
    AVOMETER_SYMBOL_P2,
    AVOMETER_SYMBOL_P3,
    AVOMETER_SYMBOL_DC,
    AVOMETER_SYMBOL_AC,
    AVOMETER_SYMBOL_OHM,
    AVOMETER_SYMBOL_K_OHM,
    AVOMETER_SYMBOL_M_OHM,
	AVOMETER_SYMBOL_MILLI,
    AVOMETER_SYMBOL_MAX,
}AVOMETER_SYMBOL_E;


// 万用表测试档位枚举
typedef enum{
			
AVOMETER_STATE_NULL = 0x00,								
AVOMETER_STATE_PT,				// 设置PT				
AVOMETER_STATE_CHECK_CHAR,		// 检查测量图标是否需要按secect				
AVOMETER_STATE_GET_RES,			// 获取结果										        						    
AVOMETER_STATE_MAX,
}AVOMETER_STATE_E;


/********************************* 结构体***************************/

typedef struct
{
    uint8 length;
    float measureRes[AVOMETER_FILTER_BUF_MAX];
}METER_FILTER_CB;	


typedef struct
{
	uint32 canUse;
	uint8  chl;
	uint8  mesureType; 				
}SEND_RES_PARA;	




typedef struct
{
		uint8 dataDir;
    BOOL revUpdata;
    AVOMETER_STATE_E state;
    AVOMETER_GEARS_CMD gearsMode;
		uint16 revLength;
    uint8 RevBuf[AVOMETER_REV_LENGTH_MAX];
		float  revNewNumRes;		
		float  reportRes;	
    METER_FILTER_CB filter;
	
    SEND_RES_PARA resPara;	
}METER_CONTROL_CB;			





extern METER_CONTROL_CB meterControlCB;						// 表头采集

/********************************* 函数外部申明***************************/
// 万用表初始化
void Avoment_Init(void);
// 切换档位
void Avoment_StitchGear(AVOMETER_GEARS_CMD gear);
// 万用表数据处理
void Avoment_Process(void);
// 测量结果上报
void Avoment_ReportMeasureResultACK(uint32 canUse);
#endif
