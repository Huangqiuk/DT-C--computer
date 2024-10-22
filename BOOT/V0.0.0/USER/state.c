#include "main.h"

// ״̬���ṹ�嶨��
STATE_CB stateCB;
uint32 upAppDataSize = 0;

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

// ÿ��״̬����ڴ���
void STATE_EnterState(uint32 state)
{	
    uint8 updateFlag = 0;
    uint32 updateBuff[1] = {0};
    uint32 buff[1] = {0};
    
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
            updateFlag = IAP_FlashReadWord(PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS + 3);

            if (0xAA == updateFlag)
            {
                STATE_EnterState(STATE_ENTRY);
            }
            else
            {
                STATE_EnterState(STATE_JUMP);
            }
            break;


        // ���� ���״̬����ڴ��� ����
        case STATE_ENTRY:
    //          POWER_ON();

            // �������ݰ���С
            upAppDataSize = IAP_FlashReadWord(PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS + 4);

            // ����APP����ҳ
            IAP_EraseAPPArea();
            break;

        // ��תAPP
        case STATE_JUMP:

            // ������ʱ����APP
            TIMER_AddTask(TIMER_ID_JUMP,
                          200,
                          IAP_JumpToAppFun,
                          0,
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
    uint32 buff[2] = {0};
    uint8  AppBuff[IAP_FLASH_APP_READ_UPDATA_SIZE] = {0};
    static uint16 i = 0;

    switch (stateCB.state)
    {
//        // ���� ��״̬ ����
//        case STATE_NULL:
//            break;

//        // ���� ���״̬���̴��� ����
//        case STATE_ENTRY:

//            if (i > ((upAppDataSize * 128 / IAP_FLASH_APP_READ_UPDATA_SIZE) + 1))
//            {

//                buff[0] = 0xFFFFFFFF;
//                
//                // ���APP������־
//                IAP_FlashWriteWordArrayWithErase(PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS, buff, 1);
//                
//                // д�������ɹ���־λ
//                buff[0] = 0xCC000000;

//                // APP�����ɹ���־
//                IAP_FlashWriteWordArrayWithErase(PARAM_UPDATA_SUCCESS_FLAG_ADDEESS, buff, 1);     
//                
//                STATE_EnterState(STATE_JUMP);                           
//            }

//            // ��ȡAPP2�е�����
//            IAP_FlashReadByteArray(IAP_FLASH_APP2_ADDR + i * IAP_FLASH_APP_READ_UPDATA_SIZE, AppBuff,  IAP_FLASH_APP_READ_UPDATA_SIZE);

//            // ��APP2�е�����д��APP1
//            IAP_WriteAppBin(IAP_FLASH_APP1_ADDR + i * IAP_FLASH_APP_READ_UPDATA_SIZE, AppBuff, IAP_FLASH_APP_READ_UPDATA_SIZE);

//            i++;
//            break;

//        // ��תAPP
//        case STATE_JUMP:
//            break;

//        default:
//            break;
    }
}


