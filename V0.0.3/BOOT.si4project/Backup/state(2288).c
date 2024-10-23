#include "common.h"
#include "state.h"
#include "timer.h"
#include "iap.h"
#include "powerCtl.h"
#include "spiflash.h"

// 状态机结构体定义
STATE_CB stateCB;

// 状态机初始化
void STATE_Init(void)
{
	// 默认状态为空
	stateCB.state = STATE_NULL;
	stateCB.preState = STATE_NULL;

	// 延时进入入口状态
	TIMER_AddTask(TIMER_ID_STATE_CONTROL,
					10,
					STATE_EnterState,
					STATE_NULL,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
}

uint32 upAppDataSize = 0;

// 每个状态的入口处理
void STATE_EnterState(uint32 state)
{
	uint8 updateFlag = 0;

	// 让当前的状态成为历史
	stateCB.preState = stateCB.state;

	// 设置新的状态
	stateCB.state = (STATE_E)state;

	// 各状态的入口设定
	switch (state)
	{
		// ■■ 空状态 ■■
		case STATE_NULL:

			// 查询升级标志
			// 清除蓝牙升级标志和屏蔽蓝牙模块初始化标志	
			updateFlag = SPI_FLASH_ReadByte(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS);
		
			if (0xAA == updateFlag)
			{
				STATE_EnterState(STATE_ENTRY);
				//STATE_EnterState(STATE_JUMP);
								
			}
			else
			{
				STATE_EnterState(STATE_JUMP);
			//	STATE_EnterState(STATE_ENTRY);
			}
			break;

		// ■■ 入口状态的入口处理 ■■
		case STATE_ENTRY:
			POWER_ON();
		
			// 升级数据包大小
			upAppDataSize = SPI_FLASH_ReadWord(SPI_FLASH_TOOL_APP_DATA_SIZE)*128;
			
			// 擦除APP所有页
			IAP_EraseAPPArea();
			/*临时清除升级标志位*/
			//SPI_FLASH_EraseSector(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS);
			//STATE_EnterState(STATE_JUMP);
			break;

		// 跳转APP
		case STATE_JUMP:

			// 开启定时跳入APP
			TIMER_AddTask(TIMER_ID_IAP_RUN_WINDOW_TIME,
							200,
							IAP_JumpToAppFun,
							IAP_FLASH_APP1_ADDR,
							1,
							ACTION_MODE_ADD_TO_QUEUE);
			break;

		default:
			break;
	}
}

// 每个状态下的过程处理
void STATE_Process(void)
{
	uint8 buff[SPI_FLASH_APP_READ_UPDATA_SIZE] = {0};
	static uint16 i = 0;
	
	switch (stateCB.state)
	{
		// ■■ 空状态 ■■
		case STATE_NULL:
			break;

		// ■■ 入口状态过程处理 ■■
		case STATE_ENTRY:
			if (i > ((upAppDataSize / SPI_FLASH_APP_READ_UPDATA_SIZE) + 1))
			{
				STATE_EnterState(STATE_JUMP);
				//extern void IAP_CTRL_UART_SendCmdWithResult(uint8 cmdWord, uint8 result);
				//IAP_CTRL_UART_SendCmdWithResult(0x03, TRUE);//测试
				// 清除蓝牙升级标志和屏蔽蓝牙模块初始化标志
				//SPI_FLASH_EraseSector(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS);
				SPI_FLASH_WriteByte(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS, 0x00);
			}
			
			SPI_FLASH_ReadArray(buff, SPI_FLASH_TOOL_APP_ADDEESS + i*SPI_FLASH_APP_READ_UPDATA_SIZE, SPI_FLASH_APP_READ_UPDATA_SIZE);
			
			IAP_WriteAppBin(IAP_FLASH_APP1_ADDR + i*SPI_FLASH_APP_READ_UPDATA_SIZE, buff, SPI_FLASH_APP_READ_UPDATA_SIZE);
			
			i++;
			break;

		// 跳转APP
		case STATE_JUMP:
			break;
		
		default:
			break;		
	}
}

