#ifndef __STATE_H__
#define __STATE_H__				

#define TIME_WAIT_AFTER_OK						1000ul		// 设置成功后、返回上级菜单前的等待时间
#define TIME_WAIT_POWER_ON_OFF					2000ul		// 开关机的等待时间

#define TIME_WAIT_NO_POSITON_RECORD				(600000ul)	// 开始骑行包记录后，无位置记录的超时时间


#define LED_PWMBUFF    							100
#define LED_PWMBUFF0                           	0  

// 系统状态定义
typedef enum
{
	STATE_NULL = 0,								// 空状态

	STATE_ENTRY,								// 入口状态

	STATE_JUMP,									// 跳转APP

	STATE_MAX									// 状态数
}STATE_E;

typedef  enum
{

	UPDATE_PROGRESS_NULL=0,
	UPDATE_PROGRESS_0,
	UPDATE_PROGRESS_20,
	UPDATE_PROGRESS_40,
	UPDATE_PROGRESS_60,
	UPDATE_PROGRESS_80,
	UPDATE_PROGRESS_100,

	UPDATE_PROGRESS_MAX

}BLUE_UPDATE;


typedef struct
{	
	STATE_E state;					// 当前系统状态
	STATE_E preState;				// 上一个状态
	STATE_E persLoveState;			// 个人喜爱的状态

	BLUE_UPDATE  ledPrePro;
	BLUE_UPDATE  ledNewPro;

	
}STATE_CB;

extern STATE_CB stateCB;


// 状态机初始化
void STATE_Init(void);

// 状态迁移
void STATE_EnterState(uint32 state);

// 状态机处理
void STATE_Process(void);

void STATE_UpradeStateShow(uint32 param);

#endif


