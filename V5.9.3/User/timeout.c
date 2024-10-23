#include "common.h"
#include "timer.h"
#include "timeout.h"
#include "dutInfo.h"
#include "dutCtl.h"
#include "CanProtocolUpDT.h"
#include "CanProtocol_3A.h"
#include "spiflash.h"
#include "state.h"

// 添加超时检测定时器
void addTimeOutTimer(uint32 time)
{
	TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
				  time,
				  STATE_SwitchStep,
				  STEP_CM_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
				  1,
				  ACTION_MODE_ADD_TO_QUEUE);
}
