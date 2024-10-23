#ifndef __STATE_H__
#define __STATE_H__				

#define TIME_WAIT_AFTER_OK						1000ul		// 设置成功后、返回上级菜单前的等待时间
#define TIME_WAIT_POWER_ON_OFF					2000ul		// 开关机的等待时间

// 系统状态定义
typedef enum
{
	STATE_NULL = 0,								// 空状态

	STATE_ENTRY,								// 入口状态
	STATE_LOAD_NVM,								// 参数加载状态
	STATE_SHOW_POWER_ON,						// 开机界面状态

	STATE_STANDBY,								// 待机状态

	STATE_RESERVED,								// 预约状态

	STATE_ERROR,								// 不可使用状态

	STATE_NORIDING,								// 禁行状态

	STATE_NOPARKING,							// 禁停状态
	
	STATE_LOWSPEED,								// 低速状态

	STATE_RIDING,								// 骑行状态

	STATE_CHARGING,								// 充电状态

	STATE_UPDATA,								// 升级状态

	STATE_RIDING_NORMAL_INTERFACE,				// 骑行正常界面状态

	STATE_LOGO,									// LOGO界面状态

	STATE_AGING_TEST,							// 老化测试状态

	STATE_FOUCTION_SEG_DATA,					// 

	STATE_MAX									// 状态数
}STATE_E;

typedef struct
{	
	STATE_E state;					// 当前系统状态
	STATE_E preState;				// 上一个状态
	STATE_E persLoveState;			// 个人喜爱的状态
	uint8 charingValue;
	uint8 upDataValue;
	
	uint32 lowSpeedAlarmFlag;
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

// 电子铃铛
void UART_PROTOCOL_SendCmdElectronicBell(uint32 param);

// 一键锁车
void UART_PROTOCOL_SendCmdKeyLock(uint32 param);

// 充电UI电量变化显示控制
void STATE_ShowChargeBatLattice(uint32 param);

void SEGMENT_ShowUpdateGui(BOOL refreshAll);


// 询问板卡类型应答
void UartCmdAskBoardTypeACK(uint8 dataDir);
// 重启应答
void UartCmdResetACK(uint8 state,uint8 dataDir);

void MultimeterSendMsgToMaistr(uint8 cmd,uint8 *msg,uint16 length);

// 系统复位
void SYSTEM_Rst(uint32 param);

#endif


