#include "common.h"
#include "timer.h"
#include "timeout.h"
#include "spiflash.h"

//// 添加超时检测定时器
//void addTimeOutTimer(uint32 time)
//{
//	TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
//				  time,
//				  STATE_SwitchStep,
//				  STEP_POWER_UPGRADE_COMMUNICATION_TIME_OUT,
//				  1,
//				  ACTION_MODE_ADD_TO_QUEUE);
//}
