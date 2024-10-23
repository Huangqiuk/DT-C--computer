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
    DUT_TYPE_MAX
} DUT_TYPE;

typedef enum
{
    CAN_TYPE_NULL,
    CAN_EXTENDED,
    CAN_STANDARD,
    CAN_TYPE_MAX
} CAN_TYPE;

typedef enum
{
    DUT_FILE_TYPE_TXT,
    DUT_FILE_TYPE_BIN,
    DUT_FILE_TYPE_JSON,
    DUT_FILE_TYPE_INI,
    DUT_FILE_TYPE_HEX,

    DUT_FILE_TYPE_MAX
} DUT_FILE_TYPE;


typedef enum
{
    GND_TYPE_NULL,
    GND_TYPE_THROTTLE,
    GND_TYPE_BRAKE,
    GND_TYPE_THROTTLE_BRAKE,
    GND_TYPE_DERAILLEUR,
    GND_TYPE_BRAKE_DERAILLEUR,    
    
    GND_TYPE_MAX
} GND_TYPE;

typedef enum
{
    TEST_TYPE_NULL,

    TEST_TYPE_UART,                   // UART测试
    TEST_TYPE_GND,                    // GND测试
    TEST_TYPE_HEADLIGHT,              // 大灯测试
    TEST_TYPE_LBEAM,                  // 近光灯测试
    TEST_TYPE_HBEAM,                  // 远光灯测试
    TEST_TYPE_LEFT_TURN_SIGNAL,       // 左转向灯测试
    TEST_TYPE_RIGHT_TURN_SIGNAL,      // 右转向灯测试
    TEST_TYPE_THROTTLE,               // 油门测试
    TEST_TYPE_BRAKE,                  // 刹把测试
    TEST_TYPE_DERAILLEUR,             // 电子变速测试
    TEST_TYPE_BUTTON,                 // 按键测试
    TEST_TYPE_LIGHT_SENSING,          // 光敏测试
    TEST_TYPE_VLK,                    // VLK测试
    TEST_TYPE_VOLTAGE,                // 电压测试
    TEST_TYPE_DISPLAY,                // 显示测试
    
    TEST_TYPE_XM_HEADLIGHT,           // 大灯（小米）测试
    TEST_TYPE_XM_LEFT_TURN_SIGNAL,    // 左转向灯（小米）测试
    TEST_TYPE_XM_RIGHT_TURN_SIGNAL,   // 右转向灯（小米）测试
    TEST_TYPE_XM_THROTTLE,            // 油门（小米）测试
    TEST_TYPE_XM_BRAKE,               // 刹把（小米）测试
    
    TEST_TYPE_MAX
} TEST_TYPE;

typedef struct
{
    int8 name[20];
    uint8 voltage;
    uint8 nameLen;
    DUT_TYPE ID;
    TEST_TYPE test;
    GND_TYPE gnd;
    CAN_TYPE can;
    
    // dut通讯类型
    BOOL dutBusType; // 0:串口，1：can

    uint16 can_baudRate;
    uint32 uart_baudRate;

    // 升级标志位
    BOOL bootUpFlag;
    BOOL appUpFlag;
    BOOL configUpFlag;
    BOOL uiUpFlag;

    // 测试标志位
    BOOL powerFlag;
    BOOL calibrationFlag;
    BOOL calibration_one_Flag;
    BOOL calibration_two_Flag;
    BOOL calibration_three_Flag;

    // 关机标志位
    BOOL adjustState;

    // 开启仪表电源
    BOOL powerOnFlag;

    // 超时时间
    uint8 timeout_Period;

    uint8 cali_cnt;
    uint8 volPower ;
    uint32 voltageMax ;
    uint32 voltageMin ;
    uint16 currentMax ;
    uint16 currentMin ;

    uint16 throttleAd;
    uint16 brakeAd;
    uint16 derailleurAd;

    uint8 cal_number;
    uint8 cal_value;

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

    // dut进度上报
    uint8 dutProgress[8];

    // 工具排序一个tool对应一个dut
    uint8 toolRank;

    // dut无应答超时重发
    BOOL reconnectionFlag;
    BOOL reconnectionRepeatOne;
    uint8 reconnectionRepeatContent[150];

    // 通讯协议
    BOOL commProt;
    
    // 按键模拟标志
    BOOL buttonSimulationFlag;
    
    BOOL VLKFlag;
    uint8 bleVerBuff[20];
    BOOL passThroughControl;
//    BOOL passThroughStsToDut;
    uint32 online_dete_cnt;
    BOOL passThroughStsToDut;
    
} DUT_INFO;


extern DUT_INFO dut_info;
extern void DutInfoUpdata(void);

#endif
