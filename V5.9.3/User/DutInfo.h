#ifndef __DUT_INFO_H__
#define __DUT_INFO_H__

#include "common.h"

typedef enum
{
	DUT_TYPE_NULL,
	DUT_TYPE_GB,
	DUT_TYPE_CM,
	DUT_TYPE_HUAXIN,
	DUT_TYPE_SEG,
	DUT_TYPE_KAIYANG,
	DUT_TYPE_LIME,
	DUT_TYPE_SPARROW,	
    DUT_TYPE_MEIDI,	
    DUT_TYPE_IOT,	    
	DUT_TYPE_MAX
} DUT_TYPE;

typedef enum
{
	DUT_FILE_TYPE_TXT,
	DUT_FILE_TYPE_BIN,
	DUT_FILE_TYPE_UI_BIN,
	DUT_FILE_TYPE_JSON,
	DUT_FILE_TYPE_INI,
	DUT_FILE_TYPE_HEX,

	DUT_FILE_TYPE_MAX
} DUT_FILE_TYPE;

typedef enum
{
	DUT_NAME_NULL,
	DUT_NAME_BC147,
    DUT_NAME_BC280,
    DUT_NAME_BC281,
    DUT_NAME_BC281EVL,
    DUT_NAME_IOT550,    
	DUT_NAME_MAX
} DUT_NAME;

typedef enum
{
	DUT_FRAME_TYPE_STD,
	DUT_FRAME_TYPE_EXT,

	DUT_FRAME_TYPE_MAX
} DUT_FRAME_TYPE;


typedef struct
{
	int8 name[20];
	uint8 voltage;
	uint8 nameLen;
	DUT_TYPE ID;
    DUT_NAME nam;
    DUT_FRAME_TYPE frameType;
    
	// dut通讯类型
	BOOL dutBusType; // 0:串口，1：can
	
	// 文件类型
	//	DUT_FILE_TYPE bootType;
	//	DUT_FILE_TYPE appType;
	//	DUT_FILE_TYPE configType;
	//	DUT_FILE_TYPE uiType;

	// 升级标志位
	BOOL bootUpFlag;
	BOOL appUpFlag;
	BOOL configUpFlag;
	BOOL uiUpFlag;

	// 包大小
	uint32 bootSize;
	uint32 appSize;
	uint32 uiSize;

	// 当前升级的包数
	uint32 currentBootSize;
	uint32 currentAppSize;
	uint32 currentUiSize;

	// 升级失败标志位
	BOOL bootUpFaile;
	BOOL appUpFaile;
	BOOL uiUpFaile;
	BOOL configUpFaile;

	// 开机允许位
	BOOL dutPowerOnAllow;

	// 升级成功标志位
	BOOL configUpSuccesss;
	BOOL uiUpSuccesss;
	BOOL appUpSuccesss;
	BOOL bootUpSuccesss;

	// dut进度上报
	uint8 dutProgress[8];

	// 工具排序一个tool对应一个dut
	uint8 toolRank;

	// dut无应答超时重发
	BOOL reconnectionFlag;
	BOOL reconnectionRepeatOne;
	uint8 reconnectionRepeatContent[150];
    
	// BOOT擦除标志位
	BOOL bootEaring;  

    // iot升级
    uint8_t iotLastPackDateLen;  
    uint8_t iotCrc8;
    BOOL iotUpAppFlag;  
    uint32_t iotAppUpDataLen;

} DUT_INFO;

extern DUT_INFO dut_info;
extern void DutInfoUpdata(void);

#endif
