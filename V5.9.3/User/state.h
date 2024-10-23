#ifndef __STATE_H__
#define __STATE_H__				

#include "common.h"

#define TIME_WAIT_AFTER_OK						1000ul		// 设置成功后、返回上级菜单前的等待时间
#define TIME_WAIT_POWER_ON_OFF					2000ul		// 开关机的等待时间
#define TIME_WAIT_MEUN_AUTO_BACK_RIDING			60000ul		// 普通菜单界面和高级菜单界面自动返回骑行界面的等待时间

#define TIME_WAIT_NO_POSITON_RECORD				(600000ul)	// 开始骑行包记录后，无位置记录的超时时间

// 系统状态定义
typedef enum
{
	STATE_NULL = 0,								// 空状态

	STATE_STANDBY,
	
	STATE_GAOBIAO_CAN_UPGRADE,					// 高标CAN升级
	
	STATE_CM_CAN_UPGRADE,      					// 通用can升级

	STATE_HUAXIN_CAN_UPGRADE,      				// 华芯微特can升级
	
	STATE_SEG_CAN_UPGRADE,						// 数码管、段码屏can升级

	STATE_KAIYANG_CAN_UPGRADE,					// 开阳can升级

	STATE_SPARROW_CAN_UPGRADE,					// SPARROW CAN升级
	
    STATE_MEIDI_CAN_UPGRADE,                    // MEIDI CAN升级
    
	STATE_CM_UART_SEG_UPGRADE,  				// 段码屏、数码管的通用串口升级

	STATE_CM_UART_BC_UPGRADE,  					// 通用彩屏的通用串口升级

	STATE_HUAXIN_UART_BC_UPGRADE,  				// 华芯维特彩屏的串口升级

	STATE_KAIYANG_UART_BC_UPGRADE,  			// 开阳彩屏的串口升级
	
	STATE_LIME_UART_BC_UPGRADE,                 // LIME的串口升级

    STATE_IOT_UART_UPGRADE,                     // IOT的串口升级

    STATE_IOT_CAN_UPGRADE,                      // IOT的串口升级
	
	STATE_MAX									// 状态数
}STATE_E;


// 系统状态定义
typedef enum
{
	STEP_NULL = 0,								// 空

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
	STEP_CM_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// 收到dut的eco请求
	STEP_CM_CAN_UPGRADE_UP_ALLOW,							// 工程模式允许

	STEP_CM_CAN_UPGRADE_SEND_UI_EAR,						// 发送ui擦除指令
	STEP_CM_CAN_UPGRADE_UI_EAR_RESUALT,						// 接收到dut返回的ui擦除结果
	STEP_CM_CAN_UPGRADE_SEND_UI_WRITE,						// 发送ui升级数据
	STEP_CM_CAN_UPGRADE_UI_UP_SUCCESS,						// ui升级完成
	STEP_CM_CAN_WRITE_UI_VER,                               // 写UI版本号
    STEP_CM_CAN_READ_UI_VER,                                // 读UI版本号
    STEP_CM_CAN_WRITE_UI_VER_SUCCESS,                       // 写UI版本号成功
    
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

	STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,			// 握手超时


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

	// 串口IOT升级
	STEP_IOT_CAN_UPGRADE_ENTRY,							// 入口步骤
	STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK,    			// 等待dut回应
	STEP_IOT_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// 收到dut的eco请求
	STEP_IOT_CAN_UPGRADE_UP_ALLOW,					// 工程模式允许
	STEP_IOT_CAN_UPGRADE_SEND_APP_EAR,				    // 发送app擦除指令
	STEP_IOT_CAN_UPGRADE_SEND_FIRST_APP_PACKET,			// 发送第一包数据
	STEP_IOT_CAN_UPGRADE_SEND_APP_PACKET,				    // 发送app升级数据
	STEP_IOT_CAN_UPGRADE_APP_UP_SUCCESS,				    // app升级完成	
	STEP_IOT_CAN_UPGRADE_ITEM_FINISH,				       	// 升级项升级完成	
	STEP_IOT_CAN_UPGRADE_COMMUNICATION_TIME_OUT,		        // 握手超时
    
 	// can升级配置信息
 	STEP_CAN_SET_CONFIG_ENTRY,								// 设置config入口
	STEP_CAN_SEND_SET_CONFIG,								// 发送config内容		
	STEP_CAN_SET_CONFIG_SUCCESS,							// 设置config成功
    STEP_CAN_READ_CONFIG,	                                // 读取配置内容
	//STEP_CAN_VERIFY_CONFIG,								// 验证配置信息是否写成功
	STEP_CAN_DCD_FLAG_WRITE,							    // 写入配置标志位
	STEP_CAN_DCD_FLAG_READ,						        	// 读取配置标志位
    
	STEP_KAIYANG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,		// 握手超时


	//通用彩屏升级
	STEP_CM_UART_UPGRADE_ENTRY,							// 入口步骤
	STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK,    			// 等待dut回应
	STEP_CM_UART_BC_UPGRADE_UP_ALLOW,					// 工程模式允许

	STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR,				// 发送app擦除指令
	STEP_CM_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET,		// 发送第一包数据
	STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET,			// 发送app升级数据
	STEP_CM_UART_BC_UPGRADE_APP_UP_SUCCESS,				// app升级完成

	STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR,				// 发送ui擦除指令
	STEP_CM_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET,		// 发送第一包数据
	STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET,				// 发送ui升级数据
	STEP_CM_UART_BC_UPGRADE_UI_UP_SUCCESS,				// ui升级完成
	STEP_CM_UART_BC_WRITE_UI_VER,                       // 写UI版本号
	STEP_CM_UART_BC_READ_UI_VER,                        // 读UI版本号    
    STEP_CM_UART_BC_WRITE_UI_VER_SUCCESS,               // 写UI版本号成功

	STEP_CM_UART_BC_UPGRADE_ITEM_FINISH,				// 升级项升级完成

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
	STEP_CM_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// 收到dut的eco请求
	STEP_CM_UART_SEG_UPGRADE_UP_ALLOW,						// 工程模式允许

	STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR,					// 发送app擦除指令
	STEP_CM_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET,			// 发送第一包数据
	STEP_CM_UART_SEG_UPGRADE_SEND_APP_PACKET,				// 发送app升级数据
	STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS,				// app升级完成
	
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

	// 串口LIME升级
	STEP_LIME_UART_UPGRADE_ENTRY,							// 入口步骤
	STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK,    			// 等待dut回应
	STEP_LIME_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// 收到dut的eco请求
	STEP_LIME_UART_SEG_UPGRADE_UP_ALLOW,					// 工程模式允许
	STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR,				    // 发送app擦除指令
	STEP_LIME_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET,			// 发送第一包数据
	STEP_LIME_UART_SEG_UPGRADE_SEND_APP_PACKET,				    // 发送app升级数据
	STEP_LIME_UART_SEG_UPGRADE_APP_UP_SUCCESS,				    // app升级完成	
	STEP_LIME_UART_SEG_UPGRADE_ITEM_FINISH,				       	// 升级项升级完成	
	STEP_LIME_UART_UPGRADE_COMMUNICATION_TIME_OUT,		        // 握手超时

	// 串口IOT升级
	STEP_IOT_UART_UPGRADE_ENTRY,							// 入口步骤
	STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK,    			// 等待dut回应
	STEP_IOT_UART_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// 收到dut的eco请求
	STEP_IOT_UART_UPGRADE_UP_ALLOW,					// 工程模式允许
	STEP_IOT_UART_UPGRADE_SEND_APP_EAR,				    // 发送app擦除指令
	STEP_IOT_UART_UPGRADE_SEND_FIRST_APP_PACKET,			// 发送第一包数据
	STEP_IOT_UART_UPGRADE_SEND_APP_PACKET,				    // 发送app升级数据
	STEP_IOT_UART_UPGRADE_APP_UP_SUCCESS,				    // app升级完成	
	STEP_IOT_UART_UPGRADE_ITEM_FINISH,				       	// 升级项升级完成	
	STEP_IOT_UART_UPGRADE_COMMUNICATION_TIME_OUT,		        // 握手超时
	
	// 串口配置信息升级
	STEP_UART_SET_CONFIG_ENTRY,								// 设置config入口
	STEP_UART_PROTOCOL_SWITCCH,                             // 协议切换
	STEP_UART_SEND_SET_CONFIG,								// 发送config内容
    STEP_UART_READ_CONFIG,	                                // 读取配置内容
	STEP_UART_SET_CONFIG_SUCCESS,							// 设置config成功
	STEP_UART_DCD_FLAG_WRITE,							// 写入配置标志位
	STEP_UART_DCD_FLAG_READ,							// 读取配置标志位

    // 通用彩屏BOOT更新    
	STEP_CM_UART_BC_UPGRADE_BOOT_ENTRY,						// 更新BOOT入口
	STEP_CM_UART_BC_UPGRADE_SEND_BOOT_EAR,					// 发送BOOT擦除指令
	STEP_CM_UART_BC_UPGRADE_SEND_FIRST_BOOT_PACKET,			// 发送第一包数据
	STEP_CM_UART_BC_UPGRADE_SEND_BOOT_PACKET,				// 发送BOOT升级数据
	STEP_CM_UART_BC_UPGRADE_BOOT_UP_SUCCESS,				// BOOT升级完成
	
 	STEP_CM_CAN_BC_UPGRADE_BOOT_ENTRY,						// 更新BOOT入口
	STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_EAR,					// 发送BOOT擦除指令
	STEP_CM_CAN_BC_UPGRADE_SEND_FIRST_BOOT_PACKET,			// 发送第一包数据
	STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_PACKET,				// 发送BOOT升级数据
	STEP_CM_CAN_BC_UPGRADE_BOOT_UP_SUCCESS,				    // BOOT升级完成
	     
	STEP_MAX
}STEP_E;

typedef struct
{	
	STATE_E state;					// 当前系统状态
	STATE_E preState;				// 上一个状态

	STEP_E step;
    
 
}STATE_CB;

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

extern uint8 verifiedBuff[150];
extern uint8 uiVerifiedBuff[50];
extern BOOL writeUiFlag;
extern BOOL writeUiVerFlag;  
#endif


