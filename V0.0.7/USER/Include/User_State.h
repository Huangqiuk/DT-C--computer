#ifndef _USER_STATE_H_
#define _USER_STATE_H_





// 系统状态定义
typedef enum
{
    STATE_NULL = 0,     // 空状态
    STATE_BROADCAST,    // 广播状态
    STATE_PAIR,             // 配对状态
    STATE_CONNECT,      // 连接状态

    STATE_SLEEP,            //低功耗状态

    STATE_MAX                       // 状态数
} STATE_E;


typedef struct
{
    STATE_E state;                  // 当前系统状态
    STATE_E preState;               // 上一个状态
    STATE_E persLoveState;          // 个人喜爱的状态


} STATE_CB;

extern STATE_CB stateCB;


// 状态机初始化
void STATE_Init(void);

// 状态迁移
void STATE_EnterState(STATE_E state);

// 状态机处理
void STATE_Process(void);


#endif  //_USER_TIMER_H_
