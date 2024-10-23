#ifndef __STATE_H__
#define __STATE_H__				

#define TIME_WAIT_AFTER_OK						1000ul		// 设置成功后、返回上级菜单前的等待时间
#define TIME_WAIT_POWER_ON_OFF					2000ul		// 开关机的等待时间

#define TIME_WAIT_NO_POSITON_RECORD				(600000ul)	// 开始骑行包记录后，无位置记录的超时时间

// 系统状态定义
typedef enum
{
	STATE_NULL = 0,								// 空状态

	STATE_ENTRY,								// 入口状态

	STATE_JUMP,									// 跳转APP

	STATE_MAX									// 状态数
}STATE_E;

typedef struct
{	
	STATE_E state;					// 当前系统状态
	STATE_E preState;				// 上一个状态
	STATE_E persLoveState;			// 个人喜爱的状态

	
}STATE_CB;

extern STATE_CB stateCB;


// 状态机初始化
void STATE_Init(void);

// 状态迁移
void STATE_EnterState(uint32 state);

// 状态机处理
void STATE_Process(void);

#endif


