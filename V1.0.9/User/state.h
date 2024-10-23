#ifndef __STATE_H__
#define __STATE_H__

#include "common.h"

#define FIRST_DAC                                1000
#define SECOND_DAC                               3000
#define THIRD_DAC                                5000
#define ERROR_DAC                                500
#define LIGHT_GND_VOLTAGE_VALUE                  3000       // 大灯GND电压值，用于检测大灯是否正常工作,单位mv

// 系统状态定义
typedef enum
{
    STATE_NULL = 0,                     // 空状态
    STATE_STANDBY,

    STATE_UART_TEST,
    STATE_UART_GND_TEST,                    // GND测试

    // 串口通讯
    STATE_UART_HEADLIGHT_TEST,              // UART通信大灯测试
    STATE_UART_LBEAM_TEST,                  // UART通信近光灯测试
    STATE_UART_HBEAM_TEST,                  // UART通信远光灯测试
    STATE_UART_LEFT_TURN_SIGNAL_TEST,       // UART通信左转向灯测试
    STATE_UART_RIGHT_TURN_SIGNAL_TEST,      // UART通信右转向灯测试
    STATE_UART_THROTTLE_TEST,               // UART通信油门测试
    STATE_UART_BRAKE_TEST,                  // UART通信刹把测试
    STATE_UART_BUTTON_TEST,                 // UART通信按键测试
    STATE_UART_LIGHT_SENSING_TEST,          // UART通信光敏测试
    STATE_UART_VLK_TEST,                    // UART通信VLK测试
    STATE_UART_VOLTAGE_TEST,                // UART通信电压校准测试
    STATE_UART_DISPLAY_TEST,                // UART通信显示测试
    STATE_UART_DERAILLEUR_TEST,             // UART通信电子变速测试

    // CAN通信
    STATE_CAN_HEADLIGHT_TEST,               // CAN通信大灯测试
    STATE_CAN_LBEAM_TEST,                   // 近光灯测试
    STATE_CAN_HBEAM_TEST,                   // 远光灯测试
    STATE_CAN_LEFT_TURN_SIGNAL_TEST,        // CAN通信左转向灯测试
    STATE_CAN_RIGHT_TURN_SIGNAL_TEST,       // CAN通信右转向灯测试
    STATE_CAN_THROTTLE_TEST,                // CAN通信油门测试
    STATE_CAN_BRAKE_TEST,                   // CAN通信刹把测试
    STATE_CAN_BUTTON_TEST,                  // CAN通信按键测试
    STATE_CAN_LIGHT_SENSING_TEST,           // CAN通信光敏测试
    STATE_CAN_VLK_TEST,                     // CAN通信VLK测试
    STATE_CAN_VOLTAGE_TEST,                 // CAN通信电压校准测试
    STATE_CAN_DISPLAY_TEST,                 // CAN通信显示测试
    STATE_CAN_DERAILLEUR_TEST,              // CAN通信电子变速测试
/////////////////////////////////////////////////////////////////////////////////

    STATE_GAOBIAO_CAN_UPGRADE,                  // 高标CAN升级
    STATE_CM_CAN_UPGRADE,                       // 通用can升级
    STATE_HUAXIN_CAN_UPGRADE,                   // 华芯微特can升级
    STATE_SEG_CAN_UPGRADE,                      // 数码管、段码屏can升级
    STATE_KAIYANG_CAN_UPGRADE,                  // 开阳can升级
    STATE_SPARROW_CAN_UPGRADE,                  // SPARROW CAN升级
    STATE_CM_UART_SEG_UPGRADE,                  // 段码屏、数码管的通用串口升级
    STATE_CM_UART_BC_UPGRADE,                   // 通用彩屏的通用串口升级
    STATE_HUAXIN_UART_BC_UPGRADE,               // 华芯维特彩屏的串口升级
    STATE_KAIYANG_UART_BC_UPGRADE,              // 开阳彩屏的串口升级
    STATE_LIME_UART_BC_UPGRADE,                 // LIME的串口升级
    
/////////////////////////////////////////////////////////////////////////////////

    STATE_UART_XM_HEADLIGHT_TEST,              // UART(小米)通信大灯测试
    STATE_UART_XM_LEFT_TURN_SIGNAL_TEST,       // UART(小米)通信左转向灯测试
    STATE_UART_XM_RIGHT_TURN_SIGNAL_TEST,      // UART(小米)通信右转向灯测试
    STATE_UART_XM_THROTTLE_TEST,               // UART(小米)通信油门测试
    STATE_UART_XM_BRAKE_TEST,                  // UART(小米)通信刹把测试
    STATE_UART_XM_VLK_TEST,                    // UART(小米)通信VLK测试

    STATE_MAX                                   // 状态数
} STATE_E;


// 系统状态定义
typedef enum
{
    STEP_NULL = 0,                          // 空

    // GND测试
    STEP_GND_TEST_ENTRY,                    // 入口步骤
    STEP_GND_TEST_WAIT_RESPONSE,            // 等待万用表回应
    STEP_GND_TEST_THROTTLE_GND,
    STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE, // 比对油门GND
    STEP_GND_TEST_BRAKE_GND,
    STEP_GND_TEST_CHECK_BRAKE_GND_VALUE,    // 比对刹把GND
    STEP_GND_TEST_DERAILLEUR_GND,
    STEP_GND_TEST_CHECK_DERAILLEUR_GND_VALUE,    // 比对电子变速GND    
    STEP_GND_TEST_COMPLETE,                 //
    STEP_GND_TEST_COMMUNICATION_TIME_OUT,   // 通讯超时

    // UART测试
    STEP_UART_TEST_ENTRY,                       // 入口步骤
    STEP_UART_TEST_WAIT_RESPONSE,               // 等待万用表回应
    STEP_UART_TEST_CHECK_VOLTAGE_VALUE,         // 比对VCC
    STEP_UART_TEST_COMPLETE,                    // UART测试完成步骤，关闭测试引脚，然后上报结果
    STEP_UART_TEST_COMMUNICATION_TIME_OUT,      // 通讯超时

    /* UART */
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 大灯测试
    STEP_HEADLIGHT_UART_TEST_ENTRY,               // 入口步骤
    STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT,          // 通知DUT打开大灯，等待DUT回应
    STEP_HEADLIGHT_UART_TEST_OPEN,                // 打开大灯,测    VCC
    STEP_HEADLIGHT_UART_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_HEADLIGHT_UART_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_HEADLIGHT_UART_TEST_COMPLETE,            // 大灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_HEADLIGHT_UART_TEST_COMMUNICATION_TIME_OUT,

    // 近光灯测试
    STEP_LBEAM_UART_TEST_ENTRY,               // 入口步骤
    STEP_LBEAM_UART_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_LBEAM_UART_TEST_NOTIFY_DUT,          // 通知DUT打开近光灯，等待DUT回应
    STEP_LBEAM_UART_TEST_OPEN,                // 打开近光灯,测  VCC
    STEP_LBEAM_UART_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_LBEAM_UART_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_LBEAM_UART_TEST_COMPLETE,            // 近光灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_LBEAM_UART_TEST_COMMUNICATION_TIME_OUT,

    // 远光灯测试
    STEP_HBEAM_UART_TEST_ENTRY,               // 入口步骤
    STEP_HBEAM_UART_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_HBEAM_UART_TEST_NOTIFY_DUT,          // 通知DUT打开远光灯，等待DUT回应
    STEP_HBEAM_UART_TEST_OPEN,                // 打开远光灯,测  VCC
    STEP_HBEAM_UART_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_HBEAM_UART_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_HBEAM_UART_TEST_COMPLETE,            // 远光灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_HBEAM_UART_TEST_COMMUNICATION_TIME_OUT,

    // 左转向灯测试
    STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY,               // 入口步骤
    STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT,          // 通知DUT打开远光灯，等待DUT回应
    STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN,                // 打开远光灯,测   VCC
    STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2,
    STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE,            // 远光灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_LEFT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,

    // 右转向灯测试
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY,               // 入口步骤
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT,          // 通知DUT打开远光灯，等待DUT回应
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN,                // 打开远光灯,测  VCC
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2,    // 比对GND
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE3,    // 比对GND        
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE,            // 远光灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,

    // 油门测试
    STEP_THROTTLE_UART_TEST_ENTRY,               // 入口步骤
    STEP_THROTTLE_UART_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_THROTTLE_UART_TEST_VCC,                 // 通知万用表测VCC
    STEP_THROTTLE_UART_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_THROTTLE_UART_TEST_CHECK_DAC1_VALUE,    // 比对DAC
    STEP_THROTTLE_UART_TEST_CHECK_DAC3_VALUE,    // 比对DAC
    STEP_THROTTLE_UART_TEST_CHECK_DAC5_VALUE,    // 比对DAC
    STEP_THROTTLE_UART_TEST_COMPLETE,            // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
    STEP_THROTTLE_UART_TEST_COMMUNICATION_TIME_OUT,  // 通信超时

    // 刹把测试
    STEP_BRAKE_UART_TEST_ENTRY,               // 入口步骤
    STEP_BRAKE_UART_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_BRAKE_UART_TEST_VCC,                 // 通知万用表测VCC
    STEP_BRAKE_UART_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_BRAKE_UART_TEST_CHECK_DAC1_VALUE,    // 比对DAC
    STEP_BRAKE_UART_TEST_CHECK_DAC3_VALUE,    // 比对DAC
    STEP_BRAKE_UART_TEST_CHECK_DAC5_VALUE,    // 比对DAC
    STEP_BRAKE_UART_TEST_COMPLETE,            // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
    STEP_BRAKE_UART_TEST_COMMUNICATION_TIME_OUT,  // 通信超时

    // 电子变速测试
    STEP_DERAILLEUR_UART_TEST_ENTRY,               // 入口步骤
    STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_DERAILLEUR_UART_TEST_VCC,                 // 通知万用表测VCC
    STEP_DERAILLEUR_UART_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_DERAILLEUR_UART_TEST_CHECK_DAC1_VALUE,    // 比对DAC
    STEP_DERAILLEUR_UART_TEST_CHECK_DAC3_VALUE,    // 比对DAC
    STEP_DERAILLEUR_UART_TEST_CHECK_DAC5_VALUE,    // 比对DAC
    STEP_DERAILLEUR_UART_TEST_COMPLETE,            // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
    STEP_DERAILLEUR_UART_TEST_COMMUNICATION_TIME_OUT,  // 通信超时
    
    // VLK测试->通知万用表测VLK电压，与下发的电压比对
    STEP_VLK_UART_TEST_ENTRY,                   // 入口步骤
    STEP_VLK_UART_TEST_WAIT_RESPONSE,           // 等待仪表或万用表回应
    STEP_VLK_UART_TEST_VLK,                     // 通知万用表测VLK
    STEP_VLK_UART_TEST_CHECK_VOLTAGE_VALUE,     // 比对VLK值
    STEP_VLK_UART_TEST_COMPLETE,                // 测试完成，然后上报结果
    STEP_VLK_UART_TEST_COMMUNICATION_TIME_OUT,  // 通信超时

    // 电压校准 -> 得到校准电压 -> 通知电源板调整仪表供电电压 ->  调整完成 ->  通知仪表进行电压校准 -> 告知DTA校准结果 ->判断有多少个电压值需要调整 -> 如果就一个 -> 上报STS校准结果 -> 有多个 ->重复校准流程
    STEP_CALIBRATION_UART_TEST_ENTRY,                      // 入口步骤
    STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE,              // 等待仪表回应
    STEP_CALIBRATION_UART_TEST_GET_CALIBRATION_VOLTAGE,    // 得到校准电压
    STEP_CALIBRATION_UART_TEST_NOTIFY_POWER_BOARD,         // 通知电源板调整仪表供电电压
    STEP_CALIBRATION_UART_TEST_WAIT_ADJUSTMENT,            // 等待电源板调整完成
    STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION, // 通知仪表进行电压校准

    STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_POWER_BOARD,         // 通知电源板调整仪表供电电压
    STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION, // 通知仪表进行电压校准

    STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_POWER_BOARD,         // 通知电源板调整仪表供电电压
    STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION, // 通知仪表进行电压校准
    STEP_CALIBRATION_UART_TEST_RESTORE_VOLTAGE_CALIBRATION,
    STEP_CALIBRATION_UART_TEST_COMPLETE,                   // 测试完成，然后上报结果
    STEP_CALIBRATION_UART_TEST_COMMUNICATION_TIME_OUT,     // 通信超时

    STEP_UART_CONFIG_PARAMETER_WRITE,         // 写入配置参数
    STEP_UART_CONFIG_PARAMETER_VALIDATE,      // 配置参数校验

    /* CAN */
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 大灯测试
    STEP_HEADLIGHT_CAN_TEST_ENTRY,               // 入口步骤
    STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_HEADLIGHT_CAN_TEST_NOTIFY_DUT,          // 通知DUT打开大灯，等待DUT回应
    STEP_HEADLIGHT_CAN_TEST_OPEN,                // 打开大灯，等待万用表回应
    STEP_HEADLIGHT_CAN_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_HEADLIGHT_CAN_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_HEADLIGHT_CAN_TEST_CHECK_CURRENT_VALUE, // 比对电流
    STEP_HEADLIGHT_CAN_TEST_COMPLETE,            // 大灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT,

    // 近光灯测试
    STEP_LBEAM_CAN_TEST_ENTRY,               // 入口步骤
    STEP_LBEAM_CAN_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_LBEAM_CAN_TEST_NOTIFY_DUT,          // 通知DUT打开近光灯，等待DUT回应
    STEP_LBEAM_CAN_TEST_OPEN,                // 打开近光灯,测   VCC
    STEP_LBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_LBEAM_CAN_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_LBEAM_CAN_TEST_COMPLETE,            // 近光灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_LBEAM_CAN_TEST_COMMUNICATION_TIME_OUT,

    // 远光灯测试
    STEP_HBEAM_CAN_TEST_ENTRY,               // 入口步骤
    STEP_HBEAM_CAN_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_HBEAM_CAN_TEST_NOTIFY_DUT,          // 通知DUT打开远光灯，等待DUT回应
    STEP_HBEAM_CAN_TEST_OPEN,                // 打开远光灯,测   VCC
    STEP_HBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_HBEAM_CAN_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_HBEAM_CAN_TEST_COMPLETE,            // 远光灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_HBEAM_CAN_TEST_COMMUNICATION_TIME_OUT,

    // 左转向灯测试
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_ENTRY,               // 入口步骤
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT,          // 通知DUT打开左转向灯，等待DUT回应
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_OPEN,                // 打开左转向灯，等待万用表回应
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_CURRENT_VALUE, // 比对电流
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_COMPLETE,            // 左转向灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_COMMUNICATION_TIME_OUT,  // 左转向灯测试通信超时

    // 右转向灯测试
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_ENTRY,               // 入口步骤
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT,          // 通知DUT打开转向灯，等待DUT回应
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_OPEN,                // 打开右转向灯，等待万用表回应
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_GND_VALUE,     // 比对GND
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_CURRENT_VALUE, // 比对电流
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_COMPLETE,            // 转向灯测试完成步骤，关闭测试引脚，然后上报结果
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_COMMUNICATION_TIME_OUT,  // 转向灯测试通信超时

    // 油门测试
    STEP_THROTTLE_CAN_TEST_ENTRY,               // 入口步骤
    STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_THROTTLE_CAN_TEST_VCC,                 // 通知万用表测VCC
    STEP_THROTTLE_CAN_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_THROTTLE_CAN_TEST_CHECK_DAC1_VALUE,    // 比对DAC
    STEP_THROTTLE_CAN_TEST_CHECK_DAC3_VALUE,    // 比对DAC
    STEP_THROTTLE_CAN_TEST_CHECK_DAC5_VALUE,    // 比对DAC
    STEP_THROTTLE_CAN_TEST_COMPLETE,            // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
    STEP_THROTTLE_CAN_TEST_COMMUNICATION_TIME_OUT,  // 通信超时

    // 刹把测试
    STEP_BRAKE_CAN_TEST_ENTRY,               // 入口步骤
    STEP_BRAKE_CAN_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_BRAKE_CAN_TEST_VCC,                 // 通知万用表测VCC
    STEP_BRAKE_CAN_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_BRAKE_CAN_TEST_CHECK_DAC1_VALUE,    // 比对DAC
    STEP_BRAKE_CAN_TEST_CHECK_DAC3_VALUE,    // 比对DAC
    STEP_BRAKE_CAN_TEST_CHECK_DAC5_VALUE,    // 比对DAC
    STEP_BRAKE_CAN_TEST_COMPLETE,            // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
    STEP_BRAKE_CAN_TEST_COMMUNICATION_TIME_OUT,  // 通信超时

    // 电子变速测试
    STEP_DERAILLEUR_CAN_TEST_ENTRY,               // 入口步骤
    STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE,       // 等待仪表或万用表回应
    STEP_DERAILLEUR_CAN_TEST_VCC,                 // 通知万用表测VCC
    STEP_DERAILLEUR_CAN_TEST_CHECK_VOLTAGE_VALUE, // 比对VCC
    STEP_DERAILLEUR_CAN_TEST_CHECK_DAC1_VALUE,    // 比对DAC
    STEP_DERAILLEUR_CAN_TEST_CHECK_DAC3_VALUE,    // 比对DAC
    STEP_DERAILLEUR_CAN_TEST_CHECK_DAC5_VALUE,    // 比对DAC
    STEP_DERAILLEUR_CAN_TEST_COMPLETE,            // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
    STEP_DERAILLEUR_CAN_TEST_COMMUNICATION_TIME_OUT,  // 通信超时
    
    // VLK测试->通知万用表测VLK电压，与下发的电压比对
    STEP_VLK_CAN_TEST_ENTRY,                   // 入口步骤
    STEP_VLK_CAN_TEST_WAIT_RESPONSE,           // 等待仪表或万用表回应
    STEP_VLK_CAN_TEST_VLK,                     // 通知万用表测VLK
    STEP_VLK_CAN_TEST_CHECK_VOLTAGE_VALUE,     // 比对VLK值
    STEP_VLK_CAN_TEST_COMPLETE,                // 测试完成，然后上报结果
    STEP_VLK_CAN_TEST_COMMUNICATION_TIME_OUT,  // 通信超时

    // 电压校准 -> 得到校准电压 -> 通知电源板调整仪表供电电压 ->  调整完成 ->  通知仪表进行电压校准 -> 告知DTA校准结果 ->判断有多少个电压值需要调整 -> 如果就一个 -> 上报STS校准结果 -> 有多个 ->重复校准流程
    STEP_CALIBRATION_CAN_TEST_ENTRY,                      // 入口步骤
    STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE,              // 等待仪表回应
    STEP_CALIBRATION_CAN_TEST_GET_CALIBRATION_VOLTAGE,    // 得到校准电压
    STEP_CALIBRATION_CAN_TEST_NOTIFY_POWER_BOARD,         // 通知电源板调整仪表供电电压
    STEP_CALIBRATION_CAN_TEST_WAIT_ADJUSTMENT,            // 等待电源板调整完成
    STEP_CALIBRATION_CAN_TEST_NOTIFY_VOLTAGE_CALIBRATION, // 通知仪表进行电压校准

    STEP_CALIBRATION_CAN_TEST_TWO_NOTIFY_POWER_BOARD,         // 通知电源板调整仪表供电电压
    STEP_CALIBRATION_CAN_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION, // 通知仪表进行电压校准

    STEP_CALIBRATION_CAN_TEST_THREE_NOTIFY_POWER_BOARD,         // 通知电源板调整仪表供电电压
    STEP_CALIBRATION_CAN_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION, // 通知仪表进行电压校准

    STEP_CALIBRATION_CAN_TEST_CHECK_CALIBRATION_RESULT,   // 告知校准结果
    STEP_CALIBRATION_CAN_TEST_CHECK_VOLTAGE_VALUES,       // 判断有多少个电压值需要调整
    STEP_CALIBRATION_CAN_TEST_REPORT_STS_RESULT,          // 上报STS校准结果
    STEP_CALIBRATION_CAN_TEST_REPEAT_CALIBRATION,         // 重复校准流程
    STEP_CALIBRATION_CAN_TEST_COMPLETE,                   // 测试完成，然后上报结果
    STEP_CALIBRATION_CAN_TEST_COMMUNICATION_TIME_OUT,     // 通信超时

    STEP_CAN_CONFIG_PARAMETER_WRITE,         // 写入配置参数
    STEP_CAN_CONFIG_PARAMETER_VALIDATE,      // 配置参数校验


    /****************************************升级********************************************/

	// 高标can升级
	STEP_GAOBIAO_CAN_UPGRADE_ENTRY,			// 入口步骤
	STEP_GAOBIAO_CAN_UPGRADE_CAN_INIT,		// CAN初始化
	STEP_GAOBIAO_CAN_UPGRADE_POWER_ON,
	STEP_GAOBIAO_CAN_UPGRADE_WAIT_3S,
	STEP_GAOBIAO_CAN_UPGRADE_SEND_KEEP_ELECTRICITY,
	STEP_GAOBIAO_CAN_UPGRADE_SEND_BOOT_CMD,
	STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY,			// 询问是否可以接收数据
	STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY,
	STEP_GAOBIAO_CAN_UPGRADE_SEND_DATA,
	STEP_GAOBIAO_CAN_UPGRADE_WRITE_ERROR,
	STEP_GAOBIAO_CAN_UPGRADE_FINISH,
	STEP_GAOBIAO_CAN_UPGRADE_POWER_OFF,
	STEP_GAOBIAO_CAN_UPGRADE_TIME_OUT,
	STEP_GAOBIAO_CAN_UPGRADE_FAILE,
	
	// 通用can升级步骤
	STEP_CM_CAN_UPGRADE_ENTRY,								// 入口步骤
	STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK,    					// 等待dut回应
    STEP_CM_CAN_UPGRADE_RESTART,                            // 仪表重启使能VLK供电重启
	STEP_CM_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// 收到dut的eco请求
	STEP_CM_CAN_UPGRADE_UP_ALLOW,							// 工程模式允许

	STEP_CM_CAN_UPGRADE_SEND_UI_EAR,						// 发送ui擦除指令
	STEP_CM_CAN_UPGRADE_UI_EAR_RESUALT,						// 接收到dut返回的ui擦除结果
	STEP_CM_CAN_UPGRADE_SEND_UI_WRITE,						// 发送ui升级数据
	STEP_CM_CAN_UPGRADE_UI_UP_SUCCESS,				 		// ui升级完成
	STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_RESET,               // app升级完成重启
	STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_YELLOW,              // app升级完成屏幕显示黄色    
    
	STEP_CM_CAN_UPGRADE_SEND_APP_EAR,						// 发送app擦除指令
	STEP_CM_CAN_UPGRADE_APP_EAR_RESUALT,					// 接收到dut返回的app擦除结果
	STEP_CM_CAN_UPGRADE_SEND_APP_WRITE,						// 发送app升级数据
	STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS,						// app升级完成

	STEP_CM_CAN_UPGRADE_ITEM_FINISH,						// 升级项升级完成
	
	STEP_CM_CAN_UPGRADE_COMMUNICATION_TIME_OUT,				// 握手超时
	STEP_CM_CAN_UPGRADE_RECONNECTION,						// 重新连接
	STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT,				// 重新连接超时

	// 华芯微特can升级步骤
	STEP_HUAXIN_CAN_UPGRADE_ENTRY,							// 入口步骤
	STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK,					// 等待dut回应
	STEP_HUAXIN_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// 收到dut的eco请求
	STEP_HUAXIN_CAN_UPGRADE_UP_ALLOW,						// 工程模式允许

	STEP_HUAXIN_CAN_UPGRADE_SEND_APP_EAR,					// 发送app擦除指令
	STEP_HUAXIN_CAN_UPGRADE_APP_EAR_RESUALT,				// 接收到dut返回的app擦除结果
	STEP_HUAXIN_CAN_UPGRADE_SEND_APP_WRITE,					// 发送app升级数据
	STEP_HUAXIN_CAN_UPGRADE_APP_UP_SUCCESS,					// app升级完成

	STEP_HUAXIN_CAN_UPGRADE_SEND_UI_EAR,					// 发送ui擦除指令
	STEP_HUAXIN_CAN_UPGRADE_UI_EAR_RESUALT,					// 接收到dut返回的ui擦除结果
	STEP_HUAXIN_CAN_UPGRADE_SEND_UI_WRITE,					// 发送ui升级数据
	STEP_HUAXIN_CAN_UPGRADE_UI_UP_SUCCESS,					// ui升级完成

	STEP_HUAXIN_CAN_UPGRADE_ITEM_FINISH,					// 升级项升级完成

	STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,				// 握手超时

	// can升级段码屏和数码管
	STEP_SEG_CAN_UPGRADE_ENTRY,								// 入口步骤
	STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK,    					// 等待dut回应
	STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// 收到dut的eco请求
	STEP_SEG_CAN_UPGRADE_UP_ALLOW,							// 工程模式允许

	STEP_SEG_CAN_UPGRADE_SEND_APP_EAR,						// 发送app擦除指令
	STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT,					// 接收到dut返回的app擦除结果
	STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE,					// 发送app升级数据
	STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS,					// app升级完成

	STEP_SEG_CAN_UPGRADE_ITEM_FINISH,						// 升级项升级完成

	STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT, 			// 握手超时

	// can升级开阳
	STEP_KAIYANG_CAN_UPGRADE_ENTRY,								// 入口步骤
	STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK,    					// 等待dut回应
	STEP_KAIYANG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// 收到dut的eco请求
	STEP_KAIYANG_CAN_UPGRADE_UP_ALLOW,							// 工程模式允许

	
	STEP_KAIYANG_CAN_UPGRADE_SEND_APP_WRITE,					// 发送app升级数据
	STEP_KAIYANG_CAN_UPGRADE_SEND_APP_AGAIN,					// 手动发送下一帧数据
	STEP_KAIYANG_CAN_UPGRADE_APP_UP_SUCCESS,					// app升级完成

	STEP_KAIYANG_CAN_UPGRADE_SEND_UI_WRITE,					// 发送ui升级数据
	STEP_KAIYANG_CAN_UPGRADE_SEND_UI_AGAIN,					// 重新发送ui数据包
	STEP_KAIYANG_CAN_UPGRADE_UI_UP_SUCCESS,					// ui升级完成

	STEP_KAIYANG_CAN_UPGRADE_ITEM_FINISH,						// 升级项升级完成

 	// can升级配置信息
 	STEP_CAN_SET_CONFIG_ENTRY,								// 设置config入口
    STEP_CAN_CONFIG_RESET,                                  // 重启
	STEP_CAN_SEND_SET_CONFIG,								// 发送config内容		
	STEP_CAN_SET_CONFIG_SUCCESS,							// 设置config成功
    STEP_CAN_READ_CONFIG,                                   // 读取配置参数
    STEP_CAN_VERIFY_CONFIG,                                 // 校验配置参数
    
    //通用彩屏升级
    STEP_CM_UART_UPGRADE_ENTRY,                         // 入口步骤		
    STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK,               // 等待dut回应
    STEP_CM_UART_BC_UPGRADE_UP_ALLOW,                   // 工程模式允许
    STEP_CM_UART_BC_UPGRADE_RESTART,                    // 仪表重启使能VLK供电重启
    STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR,               // 发送app擦除指令
    STEP_CM_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET,      // 发送第一包数据
    STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET,            // 发送app升级数据
    STEP_CM_UART_BC_UPGRADE_APP_UP_SUCCESS,             // app升级完成

    STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR,                // 发送ui擦除指令
    STEP_CM_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET,       // 发送第一包数据
    STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET,             // 发送ui升级数据
    STEP_CM_UART_BC_UPGRADE_UI_UP_SUCCESS,              // ui升级完成

    STEP_CM_UART_BC_UPGRADE_ITEM_FINISH,                // 升级项升级完成

	// 华芯维特彩屏串口升级
	STEP_HUAXIN_UART_UPGRADE_ENTRY,							// 入口步骤
	STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK,    				// 等待dut回应
	STEP_HUAXIN_UART_BC_UPGRADE_UP_ALLOW,						// 工程模式允许

	STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_EAR,					// 发送app擦除指令
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET,			// 发送第一包数据
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_PACKET,				// 发送app升级数据
	STEP_HUAXIN_UART_BC_UPGRADE_APP_UP_SUCCESS,					// app升级完成

	STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR,					// 发送ui擦除指令
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET,			// 发送第一包数据
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_PACKET,					// 发送ui升级数据
	STEP_HUAXIN_UART_BC_UPGRADE_UI_UP_SUCCESS,					// ui升级完成

	STEP_HUAXIN_UART_BC_UPGRADE_ITEM_FINISH,					// 升级项升级完成
	
	// 数码管/段码屏串口升级
	STEP_SEG_UART_UPGRADE_ENTRY,							// 入口步骤
	STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK,    				// 等待dut回应
    STEP_CM_UART_SEG_UPGRADE_RESTART,                       // 仪表重启使能VLK供电重启
	STEP_CM_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// 收到dut的eco请求
	STEP_CM_UART_SEG_UPGRADE_UP_ALLOW,						// 工程模式允许

	STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR,					// 发送app擦除指令
	STEP_CM_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET,			// 发送第一包数据
	STEP_CM_UART_SEG_UPGRADE_SEND_APP_PACKET,				// 发送app升级数据
	STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS,				// app升级完成
	STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS_RESET,          // app升级完成重启
    
	STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH,					// 升级项升级完成

	// 串口升级开阳
	STEP_KAIYANG_UART_UPGRADE_ENTRY,						// 入口步骤
	STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK,    				// 等待dut回应
	STEP_KAIYANG_UART_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// 收到dut的eco请求
	STEP_KAIYANG_UART_UPGRADE_UP_ALLOW,						// 工程模式允许

	
	STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE,				// 发送app升级数据
	STEP_KAIYANG_UART_UPGRADE_SEND_APP_AGAIN,				// 手动重发
	STEP_KAIYANG_UART_UPGRADE_APP_UP_SUCCESS,				// app升级完成

	STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE,				// 发送ui升级数据
	STEP_KAIYANG_UART_UPGRADE_SEND_UI_AGAIN,				// 手动重发
	STEP_KAIYANG_UART_UPGRADE_UI_UP_SUCCESS,				// ui升级完成

	STEP_KAIYANG_UART_UPGRADE_ITEM_FINISH,					// 升级项升级完成
	
	STEP_KAIYANG_UART_UPGRADE_COMMUNICATION_TIME_OUT,		// 握手超时

	// LIME升级
	STEP_LIME_UART_UPGRADE_ENTRY,							// 入口步骤
	STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK,    				  // 等待dut回应
	STEP_LIME_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// 收到dut的eco请求
	STEP_LIME_UART_SEG_UPGRADE_UP_ALLOW,						      // 工程模式允许

	STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR,				     	// 发送app擦除指令
	STEP_LIME_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET,			// 发送第一包数据
	STEP_LIME_UART_SEG_UPGRADE_SEND_APP_PACKET,				    // 发送app升级数据
	STEP_LIME_UART_SEG_UPGRADE_APP_UP_SUCCESS,				    // app升级完成
	
	STEP_LIME_UART_SEG_UPGRADE_ITEM_FINISH,				       	// 升级项升级完成
	
	STEP_LIME_UART_UPGRADE_COMMUNICATION_TIME_OUT,		// 握手超时
    
    // 串口配置信息升级
    STEP_UART_SET_CONFIG_ENTRY,                              // 设置config入口
    STEP_UART_CONFIG_RESET,                                  // 重启
    STEP_UART_PROTOCOL_SWITCCH,                              // 协议切换
    STEP_UART_SEND_SET_CONFIG,                               // 发送config内容
    STEP_UART_SET_CONFIG_SUCCESS,                            // 设置config成功
    STEP_UART_READ_CONFIG,                                   // 读取配置参数
    STEP_UART_VERIFY_CONFIG,                                 // 校验配置参数
    
    STEP_MAX
} STEP_E;

typedef struct
{
    STATE_E state;                  // 当前系统状态
    STATE_E preState;               // 上一个状态

    STEP_E step;
} STATE_CB;

extern STATE_CB stateCB;


// 状态机初始化
void STATE_Init(void);

// 重启自动关机计时
void STATE_ResetAutoPowerOffControl(void);

// 状态迁移
void STATE_EnterState(uint32 state);

// 状态机处理
void STATE_Process(void);

// 步骤切换
void STATE_SwitchStep(uint32 param);

// 电源程序升级步骤
void STEP_Process_PowerUartUpgrade(void);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern BOOL resetFlag;
extern BOOL configResetFlag;
extern BOOL appResetFlag;
extern uint8 verifiedBuff[150];
#endif


