#include "common.h"
#include "state.h"
#include "timer.h"
#include "iap.h"
#include "powerCtl.h"
#include "spiflash.h"

// ״̬���ṹ�嶨��
STATE_CB stateCB;

// ״̬����ʼ��
void STATE_Init(void)
{
	// Ĭ��״̬Ϊ��
	stateCB.state = STATE_NULL;
	stateCB.preState = STATE_NULL;

	// ��ʱ�������״̬
	TIMER_AddTask(TIMER_ID_STATE_CONTROL,
					10,
					STATE_EnterState,
					STATE_NULL,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
}

uint32 upAppDataSize = 0;

// ÿ��״̬����ڴ���
void STATE_EnterState(uint32 state)
{
	uint8 updateFlag = 0;

	// �õ�ǰ��״̬��Ϊ��ʷ
	stateCB.preState = stateCB.state;

	// �����µ�״̬
	stateCB.state = (STATE_E)state;

	// ��״̬������趨
	switch (state)
	{
		// ���� ��״̬ ����
		case STATE_NULL:

			// ��ѯ������־
			// �������������־����������ģ���ʼ����־	
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

		// ���� ���״̬����ڴ��� ����
		case STATE_ENTRY:
			POWER_ON();
		
			// �������ݰ���С
			upAppDataSize = SPI_FLASH_ReadWord(SPI_FLASH_TOOL_APP_DATA_SIZE)*128;
			
			// ����APP����ҳ
			IAP_EraseAPPArea();
			/*��ʱ���������־λ*/
			//SPI_FLASH_EraseSector(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS);
			//STATE_EnterState(STATE_JUMP);
			break;

		// ��תAPP
		case STATE_JUMP:

			// ������ʱ����APP
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

// ÿ��״̬�µĹ��̴���
void STATE_Process(void)
{
	uint8 buff[SPI_FLASH_APP_READ_UPDATA_SIZE] = {0};
	static uint16 i = 0;
	
	switch (stateCB.state)
	{
		// ���� ��״̬ ����
		case STATE_NULL:
			break;

		// ���� ���״̬���̴��� ����
		case STATE_ENTRY:
			if (i > ((upAppDataSize / SPI_FLASH_APP_READ_UPDATA_SIZE) + 1))
			{
				STATE_EnterState(STATE_JUMP);
				//extern void IAP_CTRL_UART_SendCmdWithResult(uint8 cmdWord, uint8 result);
				//IAP_CTRL_UART_SendCmdWithResult(0x03, TRUE);//����
				// �������������־����������ģ���ʼ����־
				//SPI_FLASH_EraseSector(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS);
				SPI_FLASH_WriteByte(SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS, 0x00);
			}
			
			SPI_FLASH_ReadArray(buff, SPI_FLASH_TOOL_APP_ADDEESS + i*SPI_FLASH_APP_READ_UPDATA_SIZE, SPI_FLASH_APP_READ_UPDATA_SIZE);
			
			IAP_WriteAppBin(IAP_FLASH_APP1_ADDR + i*SPI_FLASH_APP_READ_UPDATA_SIZE, buff, SPI_FLASH_APP_READ_UPDATA_SIZE);
			
			i++;
			break;

		// ��תAPP
		case STATE_JUMP:
			break;
		
		default:
			break;		
	}
}

