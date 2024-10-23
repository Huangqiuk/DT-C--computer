#include "common.h"
#include "system.h"
#include "timer.h"
#include "state.h"
#include "ArmProtocol.h"
#include "uartDrive.h"
#include "uartProtocol.h"
#include "uartProtocol3.h"
#include "dutCtl.h"
#include "CanProtocolUpDT.h"
#include "CanProtocolUpGB.h"
#include "canProtocol_3A.h"
#include "dutInfo.h"
#include "spiflash.h"
#include "timeOut.h"
#include "LimeUartProtocol.h"
#include "DtaUartProtocol.h"
#include "CanProtocolUpTest.h"
#include "systick.h"
#include "CanProtocolUpMD.h"

STATE_CB stateCB;
uint8 verifiedBuff[150];
uint8 uiVerifiedBuff[50];
uint8 uiVer[50] = {0};
BOOL writeUiFlag = FALSE;  
BOOL writeUiVerFlag = FALSE;  
  
void STATE_Init(void)
{
    stateCB.preState = STATE_STANDBY;
    stateCB.state = STATE_STANDBY;
}

// �����л�����
void STATE_SwitchStep(uint32 param)
{
    stateCB.step = (STEP_E)param;
}

void STEP_Process_GaobiaoCanUpgrade(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    // ��ڲ��裬��һЩ��ʼ���Ĺ���
    case STEP_GAOBIAO_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_CAN_INIT);
        dut_info.currentUiSize = 0;
        dut_info.uiUpFaile = FALSE;
        dut_info.currentAppSize = 0;
        dut_info.appUpFaile = FALSE;       
        break;

    // CAN��ʼ��
    case STEP_GAOBIAO_CAN_UPGRADE_CAN_INIT:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_POWER_ON);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
        CAN_PROTOCOL_Init(); // �߱�������ʼ��
        break;

    // dut�ϵ�
    case STEP_GAOBIAO_CAN_UPGRADE_POWER_ON:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAIT_3S);

        // ��DUT�ϵ�
        DUTCTRL_PowerOnOff(1);

        // ��ʱ3s
        TIMER_AddTask(TIMER_ID_TIME_GB_POWER_ON,
                      3000,
                      STATE_SwitchStep,
                      STEP_GAOBIAO_CAN_UPGRADE_SEND_KEEP_ELECTRICITY,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);

        break;

    // ���ﲻ��Ҫ���κ����飬ֻ��Ҫ�ȴ�3s����
    case STEP_GAOBIAO_CAN_UPGRADE_WAIT_3S:
        break;

    // �ϵ�ȴ�3s��ִ�е��˲���,����
    case STEP_GAOBIAO_CAN_UPGRADE_SEND_KEEP_ELECTRICITY:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY);
        // ������ʼ��UPDATA��UPDATA1ȷ��������boot��������dut����
        canProtocolCB.UPDATA = FALSE;
        canProtocolCB.UPDATA1 = FALSE;
        CAN_UP_RESET(1); // ��ر��磬�ȱ��������
        break;

    // ��dut��������bootָ��
    case STEP_GAOBIAO_CAN_UPGRADE_SEND_BOOT_CMD:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY);
        CAN_UP_RESET(2); // 1:���磬2������boot

        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_TIME_OUT_DETECTION,
                      10000,
                      STATE_SwitchStep,
                      STEP_GAOBIAO_CAN_UPGRADE_TIME_OUT,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ѯ��dut�Ƿ��������
    case STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY);
        // ι��
        TIMER_ResetTimer(TIMER_ID_TIME_OUT_DETECTION);
        CAN_UP_DATA(1);

        break;

    case STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY:
        // ֻ���ȴ�����������
        break;

    // dut����������ݺ�ʼ��������
    case STEP_GAOBIAO_CAN_UPGRADE_SEND_DATA:
        if(dut_info.uiUpFlag)
        {
            CAN_PGN_RequestSendData(STATE_PGN_SEND_2, 0xEF, 0x27, 0xF9);            
        }
        if(dut_info.appUpFlag)
        {
            CAN_PGN_RequestSendData(STATE_PGN_SEND_2, 0xEF, 0x28, 0xF9);                    
        }
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY);
        break;

    // dut��������д��ʧ�ܺ����·���
    case STEP_GAOBIAO_CAN_UPGRADE_WRITE_ERROR:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY);
        if (0 < dut_info.currentUiSize)
        {
            dut_info.currentUiSize--;
        }
        if (0 < dut_info.currentAppSize)
        {
            dut_info.currentAppSize--;
        }
        // ��շ��Ͷ���
        sysCanCB.tx.head = 0;
        sysCanCB.tx.end = 0;
        break;

    // ��������
    case STEP_GAOBIAO_CAN_UPGRADE_FINISH:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAIT_3S);
        dut_info.uiUpFlag = FALSE; // ������ui��λ
        dut_info.currentUiSize = 0;
        dut_info.uiUpSuccesss = TRUE; // �ϱ������ɹ�
        dut_info.appUpFlag = FALSE; // ������APP��λ
        dut_info.currentAppSize = 0;
        dut_info.appUpSuccesss = TRUE; // �ϱ������ɹ� 
        
        // dut��ʱ�ϵ�
        TIMER_AddTask(TIMER_ID_TIME_GB_POWER_OFF,
                      5000,
                      STATE_SwitchStep,
                      STEP_GAOBIAO_CAN_UPGRADE_POWER_OFF,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
                      
        // ������ʱ��ʱ��
        TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION);
        break;

    // dut�ϵ�
    case STEP_GAOBIAO_CAN_UPGRADE_POWER_OFF:
        STATE_SwitchStep(STEP_NULL);
        DUTCTRL_PowerOnOff(0); // 0�� dut�ϵ�
        break;

    // ���ͳ�ʱ
    case STEP_GAOBIAO_CAN_UPGRADE_TIME_OUT:
        STATE_SwitchStep(STEP_NULL);
        DUTCTRL_PowerOnOff(0);     // 0�� dut�ϵ�
        dut_info.uiUpFaile = TRUE; // ui����ʧ��
        dut_info.appUpFaile = TRUE; // ui����ʧ��
        TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION);
        break;

    default:
        break;
    }
}

// ͨ��CAN����,APP����Ϊhex
void STEP_Process_CmCanUpgrade(void)
{
    uint8 configs[60] = {0};
    uint8 i, j;

    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        // ͨ��can������ڣ�dut�ϵ��can��ʼ��
        case STEP_CM_CAN_UPGRADE_ENTRY:
            dut_info.frameType = DUT_FRAME_TYPE_STD;

            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            if (DUT_NAME_BC147 == dut_info.nam)
            {
                IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            }
            else
            {
                IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);
            }
            DUTCTRL_PowerOnOff(1); // dut�ϵ�
            dut_info.dutPowerOnAllow = FALSE;
            dut_info.reconnectionRepeatOne = FALSE;
            addTimeOutTimer(5000); // ���ֳ�ʱʱ�䣺2S��ά�ָߵ�s1��Ҫ2s������ʱ��
            break;

        // �ȴ�dutӦ��
        case STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK:
            break;

        // �յ�dut��eco����,��ʱ50ms����eco����
        case STEP_CM_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                          50,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_UP_ALLOW,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // ����eco����
        case STEP_CM_CAN_UPGRADE_UP_ALLOW:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,����ģʽ����
            break;

        // ����ui����ָ��
        case STEP_CM_CAN_UPGRADE_SEND_UI_EAR:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            addTimeOutTimer(15000);                                         // ����ui��ʱʱ�䣺15s
            IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_UI_DATA_ERASE); // 0x12:����ui����
            break;

        // ���յ�dut���ص�ui�������
        case STEP_CM_CAN_UPGRADE_UI_EAR_RESUALT:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            dut_info.currentUiSize = 0;

            // ����0x14��ui����д��
            // ����BC281ά�ָ� UI���ݷ���
            if(DUT_NAME_BC281EVL == dut_info.nam)
            {
                  IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_UI_BIN, IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE, SPI_FLASH_DUT_UI_ADDEESS, (dut_info.currentUiSize) * 132);            
            }
            else
            {
                  IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE, SPI_FLASH_DUT_UI_ADDEESS, (dut_info.currentUiSize) * 128);           
            }
            dut_info.currentUiSize++;
            
            // ��Ӷϵ��ط���ʱ��
            TIMER_AddTask(TIMER_ID_RECONNECTION,
                          500,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_RECONNECTION,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // ����ui��������
        case STEP_CM_CAN_UPGRADE_SEND_UI_WRITE:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            
            if (dut_info.reconnectionRepeatOne)
            {
                dut_info.reconnectionRepeatOne = FALSE;
                TIMER_AddTask(TIMER_ID_RECONNECTION,
                              500,
                              STATE_SwitchStep,
                              STEP_CM_CAN_UPGRADE_RECONNECTION,
                              TIMER_LOOP_FOREVER,
                              ACTION_MODE_ADD_TO_QUEUE);
                TIMER_KillTask(TIMER_ID_RECONNECTION_TIME_OUT);
                dut_info.currentUiSize--;
            }
            
            // ι��
            TIMER_ResetTimer(TIMER_ID_RECONNECTION);
            
            // ����BC281ά�ָ�
            if(DUT_NAME_BC281EVL == dut_info.nam)
            {
                  IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_UI_BIN, IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE, SPI_FLASH_DUT_UI_ADDEESS, (dut_info.currentUiSize) * 132);            
            }
            else
            {
                  IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE, SPI_FLASH_DUT_UI_ADDEESS, (dut_info.currentUiSize) * 128);           
            }
            dut_info.currentUiSize++;
            break;

        // ui������ɣ�д��UI�汾��
        case STEP_CM_CAN_UPGRADE_UI_UP_SUCCESS:        
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.frameType = DUT_FRAME_TYPE_EXT;
            dut_info.dutPowerOnAllow = TRUE; // ������
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            DUTCTRL_PowerOnOff(0);
            writeUiVerFlag = TRUE;
            addTimeOutTimer(10000); // ����configʱ�䣺10s
            
            TIMER_AddTask(TIMER_ID_SET_DUT_UI_VER,
                          1000,
                          STATE_SwitchStep,
                          STEP_CM_CAN_WRITE_UI_VER,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);    
            break;

        // дUI�汾��
        case STEP_CM_CAN_WRITE_UI_VER:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            SPI_FLASH_ReadArray(uiVer, SPI_FLASH_UI_VERSION_ADDEESS, 50); // ʵ��ֻ��54���ֽ�
            DUTCTRL_PowerOnOff(1);                                        // dut�ϵ�
            Delayms(250);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_VERSION_TYPE_WRITE); // 0xA8
            CAN_PROTOCOL1_TxAddData(uiVer[0] + 2);
            CAN_PROTOCOL1_TxAddData(8);         // UI�汾����
            CAN_PROTOCOL1_TxAddData(uiVer[0]);  // ��Ϣ����
            j = 0;
            
            for (i = 1; i < uiVer[0] + 1; i++)
            {
                CAN_PROTOCOL1_TxAddData(uiVer[i]);
                uiVerifiedBuff[j++] = uiVer[i];
            }
            CAN_PROTOCOL1_TxAddFrame();
            break;
            
        // ��UI�汾��
        case STEP_CM_CAN_READ_UI_VER:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);    
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_VERSION_TYPE_READ); // 0xA9
            CAN_PROTOCOL1_TxAddData(1);    // ���ݳ���
            CAN_PROTOCOL1_TxAddData(8);    // ����
            CAN_PROTOCOL1_TxAddFrame();
            break;
            
        // дUI�汾�ųɹ�
        case STEP_CM_CAN_WRITE_UI_VER_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ITEM_FINISH);
            dut_info.uiUpSuccesss = TRUE;
            dut_info.uiUpFlag = FALSE;
            dut_info.reconnectionFlag = FALSE;
            TIMER_KillTask(TIMER_ID_RECONNECTION);    
            break;        
        
        // ����app����ָ��
        case STEP_CM_CAN_UPGRADE_SEND_APP_EAR:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            addTimeOutTimer(5000);                                            // ����app��ʱʱ�䣺5s
            IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_IAP_ERASE_FLASH); // 0x24,����app
            break;

        // app�����������
        case STEP_CM_CAN_UPGRADE_APP_EAR_RESUALT:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            
            // ���͵�һ֡����
            dut_info.currentAppSize = 0;
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_HEX, IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 116);
            dut_info.currentAppSize++;
            
            // ��Ӷϵ��ط���ʱ��
            TIMER_AddTask(TIMER_ID_RECONNECTION,
                          500,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_RECONNECTION,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // ����app������
        case STEP_CM_CAN_UPGRADE_SEND_APP_WRITE:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            if (dut_info.reconnectionRepeatOne)
            {
                dut_info.reconnectionRepeatOne = FALSE;
                dut_info.currentAppSize--;
                TIMER_AddTask(TIMER_ID_RECONNECTION,
                              500,
                              STATE_SwitchStep,
                              STEP_CM_CAN_UPGRADE_RECONNECTION,
                              TIMER_LOOP_FOREVER,
                              ACTION_MODE_ADD_TO_QUEUE);
                TIMER_KillTask(TIMER_ID_RECONNECTION_TIME_OUT);
            }
            
            // ι��
            TIMER_ResetTimer(TIMER_ID_RECONNECTION);
            IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_HEX, IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 116);
            dut_info.currentAppSize++;
            break;

        // app�����ɹ�
        case STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_UPDATA_FINISH); // 0x2A:�����ɹ�
            dut_info.appUpFlag = FALSE;
            dut_info.appUpSuccesss = TRUE;
            dut_info.reconnectionFlag = FALSE;
            TIMER_KillTask(TIMER_ID_RECONNECTION);
            break;

        // ����config���
        case STEP_CAN_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.frameType = DUT_FRAME_TYPE_EXT;
            dut_info.dutPowerOnAllow = TRUE; // ������
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            DUTCTRL_PowerOnOff(0);
            //        addTimeOutTimer(5000); // ����configʱ�䣺5s
            TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                          500,
                          STATE_SwitchStep,
                          STEP_CAN_SEND_SET_CONFIG,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // ����config����
        case STEP_CAN_SEND_SET_CONFIG:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�
            DUTCTRL_PowerOnOff(1);                                          // dut�ϵ�
            
            // �������ͷ
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            
            // ����豸��ַ
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            
            // ���������
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM); // 0xc0
            
            // ������ݳ���
            CAN_PROTOCOL1_TxAddData(54);

            for (i = 0; i < 54; i++)
            {
                CAN_PROTOCOL1_TxAddData(configs[i]);
            }
            // ��Ӽ������������������������
            CAN_PROTOCOL1_TxAddFrame();

            j = 0;
            
            // �����������ݣ�ȥ��ʱ�䣬��ȡ��У��buffer
            for (i = 0; i < 54 ; i++)
            {
                if (i == 2)
                {
                    i = i + 5;
                }
                verifiedBuff[j++] = configs[i];
            }
            break;

        // ��ȡ���ò���
        case STEP_CAN_READ_CONFIG:
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);                              // ֹͣ����config��ʱ��
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                      // �������ͷ
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM);        // 0xC2���������
            CAN_PROTOCOL1_TxAddData(0);                                           // ������ݳ���

            // ��Ӽ������������������������
            CAN_PROTOCOL1_TxAddFrame();
            break;

        // д��DCD��־
        case STEP_CAN_DCD_FLAG_WRITE:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DCD_FLAG_WRITE);
            CAN_PROTOCOL1_TxAddData(0x06);   // ���ݳ���
            CAN_PROTOCOL1_TxAddData(0x05);   // DCD��־
            CAN_PROTOCOL1_TxAddData(0x00);
            CAN_PROTOCOL1_TxAddData(0x00);
            CAN_PROTOCOL1_TxAddData(0x00);
            CAN_PROTOCOL1_TxAddData(0x02);
            CAN_PROTOCOL1_TxAddData(0);
            CAN_PROTOCOL1_TxAddFrame();               
            break;
        
        // ��ȡDCD��־
        case STEP_CAN_DCD_FLAG_READ:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DCD_FLAG_READ);
            CAN_PROTOCOL1_TxAddData(0);
            CAN_PROTOCOL1_TxAddFrame();                           
            break;

        // config���óɹ�
        case STEP_CAN_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ITEM_FINISH);
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ����config��ʱ��
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // ֹͣ��ʱ��ʱ��
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);   // ���ò�����
            DUTCTRL_PowerOnOff(0);         // dut�ϵ�
            dut_info.configUpFlag = FALSE; // ��־λ��false
            dut_info.configUpSuccesss = TRUE;
            dut_info.dutPowerOnAllow = FALSE;
            dut_info.frameType = DUT_FRAME_TYPE_STD;
            break;

        // ����BOOT���
        case STEP_CM_CAN_BC_UPGRADE_BOOT_ENTRY:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // ������
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            CAN_TEST_PROTOCOL_Init();
            DUTCTRL_PowerOnOff(0);           // �Ǳ�ϵ�
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            TIMER_AddTask(TIMER_ID_UPGRADE_DUT_BOOT,
                          500,
                          STATE_SwitchStep,
                          STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_EAR,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
                          dut_info.frameType = DUT_FRAME_TYPE_EXT;
            break;

        // ����BOOT����ָ��
        case STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_EAR:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            DUTCTRL_PowerOnOff(1);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
//            Delayms(500);
//            dut_info.dutPowerOnAllow = FALSE;
            dut_info.bootEaring = TRUE; 
            dut_info.reconnectionRepeatOne = FALSE;
            CAN_TEST_PROTOCOL_SendCmdNoResult(CAN_TEST_PROTOCOL_CMD_BOOT_ERASE_FLASH);
            break;

        // ����BOOT��������
        case STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_PACKET:
           TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            dut_info.bootEaring = FALSE;
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_TEST_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE_BIN, CAN_TEST_PROTOCOL_CMD_BOOT_WRITE_FLASH, SPI_FLASH_DUT_BOOT_ADDEESS, (dut_info.currentBootSize) * 128);
            dut_info.currentBootSize++;
            break;

        // BOOT�������
        case STEP_CM_CAN_BC_UPGRADE_BOOT_UP_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_TEST_PROTOCOL_SendCmdWithResult(CAN_TEST_PROTOCOL_CMD_BOOT_UPDATA_FINISH, TRUE);
            dut_info.bootUpFlag = FALSE;
            dut_info.bootUpSuccesss = TRUE;
            break;

        // �����������ɹ�
        case STEP_CM_CAN_UPGRADE_ITEM_FINISH:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            // ѡ������
            if (dut_info.appUpFlag)
            {
                dut_info.currentAppSize = 0;
                dut_info.dutPowerOnAllow = FALSE;
                DUTCTRL_PowerOnOff(0);
                TIMER_AddTask(TIMER_ID_DUT_UPGRATE,
                              500,
                              STATE_SwitchStep,
                              STEP_CM_CAN_UPGRADE_ENTRY,
                              1,
                              ACTION_MODE_ADD_TO_QUEUE);
            }
            else if (dut_info.uiUpFlag)
            {
                dut_info.currentUiSize = 0;
                STATE_SwitchStep(STEP_CM_CAN_UPGRADE_SEND_UI_EAR); // 0x12:����ui����
            }
            else if (dut_info.configUpFlag)
            {
                STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
            }
            else if (dut_info.bootUpFlag)
            {
                STATE_SwitchStep(STEP_CM_CAN_BC_UPGRADE_BOOT_ENTRY);
            }
            else
            {
                // û��������ϵ�
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // ���ֳ�ʱ
        case STEP_CM_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
            if (dut_info.appUpFlag)
            {
                dut_info.appUpFaile = TRUE;
            }
            else if (dut_info.uiUpFlag)
            {
                dut_info.uiUpFaile = TRUE; // ui����ʧ��
            }
            else if (dut_info.configUpFlag)
            {
                dut_info.configUpFaile = TRUE; // config����ʧ��
            }
            else if (dut_info.bootUpFlag)
            {
                dut_info.bootUpFaile = TRUE;   // boot����ʧ��
            }

            STATE_EnterState(STATE_STANDBY);
            break;

        // dut�ϵ���������
        case STEP_CM_CAN_UPGRADE_RECONNECTION:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.reconnectionRepeatOne = TRUE;
            dut_info.reconnectionFlag = TRUE;
            // ÿ100ms����һ��eco����
            TIMER_AddTask(TIMER_ID_RECONNECTION,
                          100,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_UP_ALLOW,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);

            // 3s��ֹͣ����eco�����ϱ�����ʧ��
            TIMER_AddTask(TIMER_ID_RECONNECTION_TIME_OUT,
                          3000,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �������ӳ�ʱ
        case STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT:
            STATE_SwitchStep(STEP_NULL);
            TIMER_KillTask(TIMER_ID_RECONNECTION);
            TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION);
            dut_info.reconnectionFlag = FALSE;

            if (dut_info.appUpFlag)
            {
                dut_info.appUpFaile = TRUE;
            }
            else if (dut_info.uiUpFlag)
            {
                dut_info.uiUpFaile = TRUE;
            }
            else if (dut_info.bootUpFlag)
            {
                dut_info.bootUpFaile = TRUE;   // boot����ʧ��
            }

            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// ��о΢��can��������,app��bin����
void STEP_Process_HuaXinCanUpgrade()
{
    uint8 configs[60] = {0};
    uint8 i;
    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    // ͨ��can������ڣ�dut�ϵ��can��ʼ��
    case STEP_HUAXIN_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);
        DUTCTRL_PowerOnOff(1); // dut�ϵ�
        dut_info.dutPowerOnAllow = FALSE;
        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      10000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // �ȴ�dutӦ��
    case STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK:
        break;

    // �յ�dut��eco����,��ʱ50ms����eco����
    case STEP_HUAXIN_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        
        // 50ms����eco����
        TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                      50,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_UP_ALLOW,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);

        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      2000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����eco����
    case STEP_HUAXIN_CAN_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,����ģʽ��
        break;

    // ����app����ָ��
    case STEP_HUAXIN_CAN_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_APP_EAR); // 0x05,����app
        // ��ӳ�ʱ,8���app����ʱ��
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      8000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����app������
    case STEP_HUAXIN_CAN_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
        dut_info.currentAppSize++;
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,// ι��+���賬ʱʱ�䣨��ԭ����8s��Ϊ1s��
                      1000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // app�����ɹ�
    case STEP_HUAXIN_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP_COMPLETE); // 0x07:�����ɹ�
        dut_info.appUpFlag = FALSE;
        dut_info.appUpSuccesss = TRUE;
        break;

    // ����ui����ָ��
    case STEP_HUAXIN_CAN_UPGRADE_SEND_UI_EAR:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_UI_DATA_ERASE); // 0x12:����ui����
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      30000,                // ui����ʱ��10s
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����ui��������
    case STEP_HUAXIN_CAN_UPGRADE_SEND_UI_WRITE:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE, SPI_FLASH_DUT_UI_ADDEESS, (dut_info.currentUiSize) * 128);
        dut_info.currentUiSize++;
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT, // ι��+���賬ʱʱ�䣨��ԭ����10s��Ϊ1s��
                      2000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ui�������
    case STEP_HUAXIN_CAN_UPGRADE_UI_UP_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_UPDATA_FINISH); // 0x2A:�����ɹ�
        dut_info.uiUpSuccesss = TRUE;
        dut_info.uiUpFlag = FALSE;
        dut_info.reconnectionFlag = FALSE;
        break;

    // ����config���
    case STEP_CAN_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        dut_info.dutPowerOnAllow = TRUE; // ������
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
        DUTCTRL_PowerOnOff(0);
        // 500ms����һ��������Ϣ
        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_CAN_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        // ���5s�ĳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      5000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����config����
    case STEP_CAN_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�
        DUTCTRL_PowerOnOff(1);                                          // dut�ϵ�
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                // �������ͷ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);         // ����豸��ַ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM); // 0xc0 ���������
        CAN_PROTOCOL1_TxAddData(54);                                    // ������ݳ���
        for (i = 0; i < 54; i++)
        {
            CAN_PROTOCOL1_TxAddData(configs[i]);
        }
        CAN_PROTOCOL1_TxAddFrame(); // ��Ӽ������������������������
        break;

    // config���óɹ�
    case STEP_CAN_SET_CONFIG_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ����config��ʱ��
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);   // ���ò�����
        dut_info.configUpFlag = FALSE;           // ��־λ��false
        DUTCTRL_PowerOnOff(0);                   // dut�ϵ�
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE;
        break;

    // �����������ɹ�
    case STEP_HUAXIN_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        // ѡ������
        if (dut_info.uiUpFlag)
        {
            STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_SEND_UI_EAR);
        }
        else if (dut_info.configUpFlag)
        {
            STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
        }
        else
        {
            // û��������ϵ�
            STATE_EnterState(STATE_STANDBY);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        }
        break;

    // ���ֳ�ʱ
    case STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
        dut_info.currentUiSize--;
			if (dut_info.currentUiSize < (dut_info.uiSize + 1))
        {
            STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_SEND_UI_WRITE);
        }
        break;

    // dut�ϵ���������
    case STEP_CM_CAN_UPGRADE_RECONNECTION:
        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
        dut_info.reconnectionRepeatOne = TRUE;
        // ÿ100ms����һ��eco����
        TIMER_AddTask(TIMER_ID_RECONNECTION,
                      100,
                      STATE_SwitchStep,
                      STEP_CM_CAN_UPGRADE_UP_ALLOW,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        // 3s��ֹͣ����eco�����ϱ�����ʧ��
        TIMER_AddTask(TIMER_ID_RECONNECTION_TIME_OUT,
                      3000,
                      STATE_SwitchStep,
                      STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        dut_info.reconnectionFlag = TRUE;
        break;

    // �������ӳ�ʱ
    case STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT:
        STATE_SwitchStep(STEP_NULL);
        TIMER_KillTask(TIMER_ID_RECONNECTION);
        TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION);
        dut_info.reconnectionFlag = FALSE;
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        else if (dut_info.uiUpFlag)
        {
            dut_info.uiUpFaile = TRUE;
        }
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}

// �������������can������app��bin��ʽ
void STEP_Process_SegCanUpgrade()
{
    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    // ͨ��can������ڣ�dut�ϵ��can��ʼ��
    case STEP_SEG_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);
        DUTCTRL_PowerOnOff(1); // dut�ϵ�
        dut_info.dutPowerOnAllow = FALSE;

        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      2000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // �ȴ�dutӦ��
    case STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK:
        break;

    // �յ�dut��eco����,��ʱ50ms����eco����
    case STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        // 50ms����eco����
        TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                      50,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_UP_ALLOW,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        // ���1s�����ֳ�ʱ���
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      1000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // ���ֳ�ʱ
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����eco����
    case STEP_SEG_CAN_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,����ģʽ��
        break;

    // ����app����ָ��
    case STEP_SEG_CAN_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_APP_EAR); // 05:����app����
        // ���8s�����ֳ�ʱ���
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      8000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // ���ֳ�ʱ
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // app�����������
    case STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        // ���͵�һ֡����
        dut_info.currentAppSize = 0;
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_HEX, IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 116);
        dut_info.currentAppSize++;
        // ���1s�����ֳ�ʱ���
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      1000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // ���ֳ�ʱ
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����app������
    case STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
        dut_info.currentAppSize++;
        break;

    // app�����ɹ�
    case STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP_COMPLETE); // 0x07:�����ɹ�
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // ����config���
    //  case STEP_CAN_SET_CONFIG_ENTRY:
    //      STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
    //      dut_info.dutPowerOnAllow = TRUE; // ������
    //      IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
    //      DUTCTRL_PowerOnOff(0);
    //      addTimeOutTimer(5000); // ����configʱ�䣺5s
    //      TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
    //                    500,
    //                    STATE_SwitchStep,
    //                    STEP_CAN_SEND_SET_CONFIG,
    //                    TIMER_LOOP_FOREVER,
    //                    ACTION_MODE_ADD_TO_QUEUE);
    //      break;

    //  // ����config����
    //  case STEP_CAN_SEND_SET_CONFIG:
    //      STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
    //      SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�
    //      DUTCTRL_PowerOnOff(1);                                          // dut�ϵ�
    //      /* ����Э���л����� */
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);        // �������ͷ
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR); // ����豸��ַ
    //      CAN_PROTOCOL1_TxAddData(0xab);                          // 0xab��Э���л�
    //      CAN_PROTOCOL1_TxAddData(0);                             // ������ݳ���
    //      CAN_PROTOCOL1_TxAddData(0x01);
    //      CAN_PROTOCOL1_TxAddFrame(); // ��Ӽ������������������������

    //      /* ����������Ϣ */
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                // �������ͷ
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);         // ����豸��ַ
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM); // 0xc0 ���������
    //      CAN_PROTOCOL1_TxAddData(54);                                    // ������ݳ���
    //      for (i = 0; i < 54; i++)
    //      {
    //          CAN_PROTOCOL1_TxAddData(configs[i]);
    //      }
    //      CAN_PROTOCOL1_TxAddFrame(); // ��Ӽ������������������������
    //      break;

    //  // config���óɹ�
    //  case STEP_CAN_SET_CONFIG_SUCCESS:
    //      STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_ITEM_FINISH);
    //      TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ����config��ʱ��

    //      IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K); // ���ò�����
    //      dut_info.configUpFlag = FALSE;         // ��־λ��false
    //      DUTCTRL_PowerOnOff(0);                 // dut�ϵ�
    //      dut_info.configUpSuccesss = TRUE;
    //      dut_info.dutPowerOnAllow = FALSE;
    //      break;

    // �����������ɹ�
    case STEP_SEG_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        // ѡ������
        if (dut_info.configUpFlag)
        {
            STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
        }
        else
        {
            STATE_EnterState(STATE_STANDBY); // û��������ϵ�
        }
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT); // ֹͣ��ʱ��ʱ��
        break;

    // ���ֳ�ʱ
    case STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        else if (dut_info.uiUpFlag)
        {
            dut_info.uiUpFaile = TRUE; // ui����ʧ��
        }
        else if (dut_info.configUpFlag)
        {
            dut_info.configUpFaile = TRUE; // config����ʧ��
        }
        STATE_EnterState(STATE_STANDBY);
        break;

    // dut�ϵ���������
    case STEP_CM_CAN_UPGRADE_RECONNECTION:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        break;

    // �������ӳ�ʱ
    case STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT:
        STATE_SwitchStep(STEP_NULL);

        break;

    default:
        break;
    }
}

// Meidi can������app��bin��ʽ
void STEP_Process_MeiDiCanUpgrade()
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        // ͨ��can������ڣ�dut�ϵ��can��ʼ��
        case STEP_SEG_CAN_UPGRADE_ENTRY:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
            CAN_MD_PROTOCOL_Init(); // MeiDi������ʼ��
            DUTCTRL_PowerOnOff(1);  // dut�ϵ�
            dut_info.dutPowerOnAllow = FALSE;

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          6000,
                          STATE_SwitchStep,
                          STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȴ�dutӦ��
        case STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK:
            break;

        // �յ�dut��eco����,��ʱ50ms����eco����
        case STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            // 50ms����eco����
            TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                          50,
                          STATE_SwitchStep,
                          STEP_SEG_CAN_UPGRADE_UP_ALLOW,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
//            // ���1s�����ֳ�ʱ���
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          500,
                          STATE_SwitchStep,
                          STEP_SEG_CAN_UPGRADE_UP_ALLOW,            // ���ֳ�ʱ
                          -1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // ����eco����
        case STEP_SEG_CAN_UPGRADE_UP_ALLOW:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_MD_PROTOCOL_SendCmdNoResult(CAN_MD_PROTOCOL_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,����ģʽ��
            break;

        // APP����д��
        case STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

            // ���͵�һ֡����
            dut_info.currentAppSize = 0;
            CAN_MD_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE_BIN, CAN_MD_PROTOCOL_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
            dut_info.currentAppSize++;
            
            // ���1s�����ֳ�ʱ���
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          1000,
                          STATE_SwitchStep,
                          STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // ���ֳ�ʱ
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // ����app������
        case STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
            CAN_MD_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE_BIN, CAN_MD_PROTOCOL_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
            dut_info.currentAppSize++;
            break;

        // app�����ɹ�
        case STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_MD_PROTOCOL_SendCmdNoResult(CAN_MD_PROTOCOL_CMD_DOWN_UPDATA_FINISH); // 0x07:�����ɹ�
            dut_info.appUpSuccesss = TRUE;
            dut_info.appUpFlag = FALSE;
            break;

        // �����������ɹ�
        case STEP_SEG_CAN_UPGRADE_ITEM_FINISH:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

            // ѡ������
            if (dut_info.configUpFlag)
            {
                STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
            }
            else
            {
                STATE_EnterState(STATE_STANDBY); // û��������ϵ�
            }
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT); // ֹͣ��ʱ��ʱ��
            break;

        // ���ֳ�ʱ
        case STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
            if (dut_info.appUpFlag)
            {
                dut_info.appUpFaile = TRUE;
            }
            else if (dut_info.uiUpFlag)
            {
                dut_info.uiUpFaile = TRUE; // ui����ʧ��
            }
            else if (dut_info.configUpFlag)
            {
                dut_info.configUpFaile = TRUE; // config����ʧ��
            }
            STATE_EnterState(STATE_STANDBY);
            break;

        // dut�ϵ���������
        case STEP_CM_CAN_UPGRADE_RECONNECTION:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

            break;

        // �������ӳ�ʱ
        case STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT:
            STATE_SwitchStep(STEP_NULL);

            break;

        default:
            break;
    }
}

// Sparrow can������app��bin��ʽ
void STEP_Process_Sparrow_SegCanUpgrade()
{
    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    // ͨ��can������ڣ�dut�ϵ��can��ʼ��
    case STEP_SEG_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
        DUTCTRL_PowerOnOff(1); // dut�ϵ�
        dut_info.dutPowerOnAllow = FALSE;

        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      6000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // �ȴ�dutӦ��
    case STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK:
        break;

    // �յ�dut��eco����,��ʱ50ms����eco����
    case STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        // 50ms����eco����
        TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                      50,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_UP_ALLOW,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        // ���1s�����ֳ�ʱ���
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      1000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // ���ֳ�ʱ
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����eco����
    case STEP_SEG_CAN_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,����ģʽ��
        break;

    // ����app����ָ��
    case STEP_SEG_CAN_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_APP_EAR); // 05:����app����
        // ���8s�����ֳ�ʱ���
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      8000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // ���ֳ�ʱ
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // app�����������
    case STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        // ���͵�һ֡����
        dut_info.currentAppSize = 0;
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
        dut_info.currentAppSize++;
        // ���1s�����ֳ�ʱ���
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      1000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // ���ֳ�ʱ
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����app������
    case STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
        dut_info.currentAppSize++;
        break;

    // app�����ɹ�
    case STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP_COMPLETE); // 0x07:�����ɹ�
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // �����������ɹ�
    case STEP_SEG_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        // ѡ������
        if (dut_info.configUpFlag)
        {
            STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
        }
        else
        {
            STATE_EnterState(STATE_STANDBY); // û��������ϵ�
        }
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT); // ֹͣ��ʱ��ʱ��
        break;

    // ���ֳ�ʱ
    case STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        else if (dut_info.uiUpFlag)
        {
            dut_info.uiUpFaile = TRUE; // ui����ʧ��
        }
        else if (dut_info.configUpFlag)
        {
            dut_info.configUpFaile = TRUE; // config����ʧ��
        }
        STATE_EnterState(STATE_STANDBY);
        break;

    // dut�ϵ���������
    case STEP_CM_CAN_UPGRADE_RECONNECTION:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        break;

    // �������ӳ�ʱ
    case STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT:
        STATE_SwitchStep(STEP_NULL);

        break;

    default:
        break;
    }
}

// ����can��������
void STEP_Process_KaiYangCanUpgrade()
{
    uint8 dataPacket[130];
    uint8 i, j, k;
    uint8 configs[60] = {0};

    switch (stateCB.step)
    {
    case STEP_KAIYANG_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
        DUTCTRL_PowerOnOff(1); // dut�ϵ�
        dut_info.dutPowerOnAllow = FALSE;
        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_KAIYANG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK: // �ȴ�
        break;

    case STEP_KAIYANG_CAN_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_UP_PROJECT_APPLY); // 0x01,����ģʽ��
        // �·��ļ�����
        IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD); // �������ͷ;//ͷ
        IAP_CTRL_CAN_TxAddData(0x02);         // ���0x02
        IAP_CTRL_CAN_TxAddData(0x00);         // �����ʱ���ݳ���
        if (dut_info.appUpFlag)
        {
            IAP_CTRL_CAN_TxAddData(0x03);                   // 0x03:��ʾ��app�ļ�
            IAP_CTRL_CAN_TxAddData(dut_info.appSize >> 16); // ������λ
            IAP_CTRL_CAN_TxAddData(dut_info.appSize >> 8);  // ������λ
            IAP_CTRL_CAN_TxAddData(dut_info.appSize);       // ������λ
        }
        else if (dut_info.uiUpFlag)
        {
            IAP_CTRL_CAN_TxAddData(0x01);                  // 0x01:��ʾ��ui�ļ�
            IAP_CTRL_CAN_TxAddData(dut_info.uiSize >> 16); // ������λ
            IAP_CTRL_CAN_TxAddData(dut_info.uiSize >> 8);  // ������λ
            IAP_CTRL_CAN_TxAddData(dut_info.uiSize);       // ������λ
        }

        IAP_CTRL_CAN_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
        break;

    case STEP_KAIYANG_CAN_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);

        TIMER_ResetTimer(TIMER_ID_RECONNECTION);// ι��
        SPI_FLASH_ReadArray(dataPacket, SPI_FLASH_DUT_APP_ADDEESS + (dut_info.currentAppSize) * 128, 128); // ���߶�ȡ128�ֽ�

        k = 128;

        // ȥ�����һ�����ݵ�0xff
        if (dut_info.currentAppSize == dut_info.appSize - 1)
        {
            for (i = 0; i < 128; i++)
            {
                if (0xff == dataPacket[i])
                {
                    for (j = i; j < 128; j++) // �������ȫ��0xff
                    {
                        if (0xff != dataPacket[j - 1])
                        {
                            break;
                        }

                        if (127 == j) // ����ȫ��0xff
                        {
                            k = i - 1;
                            i = 128;
                        }
                    }
                }
            }
        }

        IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
        IAP_CTRL_CAN_TxAddData(0x03);
        IAP_CTRL_CAN_TxAddData(0x00);                            // ��ʱ������ݳ���
        IAP_CTRL_CAN_TxAddData(dut_info.currentAppSize % 0x100); // ��ʱ������ݳ���

        for (i = 0; i < k; i++)
        {
            IAP_CTRL_CAN_TxAddData(dataPacket[i]);
        }
        IAP_CTRL_CAN_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
        break;

    // �ֶ�������һ֡ui����
    case STEP_KAIYANG_CAN_UPGRADE_SEND_APP_AGAIN:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_SEND_APP_WRITE);
        dut_info.currentAppSize++;
        break;

    case STEP_KAIYANG_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
        IAP_CTRL_CAN_TxAddData(0x04);
        IAP_CTRL_CAN_TxAddData(1); // ���ݳ���
        IAP_CTRL_CAN_TxAddData(0x01);
        IAP_CTRL_CAN_TxAddFrame();
        TIMER_KillTask(TIMER_ID_RECONNECTION);
        break;

    // ����ui����
    case STEP_KAIYANG_CAN_UPGRADE_SEND_UI_WRITE:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_RECONNECTION);// ι��
        SPI_FLASH_ReadArray(dataPacket, SPI_FLASH_DUT_UI_ADDEESS + (dut_info.currentUiSize) * 128, 128); // ���߶�ȡ128�ֽ�
        k = 128;

        if (dut_info.currentUiSize == dut_info.uiSize - 1) // ���һ��ȥ0xff
        {
            for (i = 0; i < 128; i++)
            {
                if (0xff == dataPacket[i])
                {
                    for (j = i; j < 128; j++) // �������ȫ��0xff
                    {
                        if (0xff != dataPacket[j - 1])
                        {
                            break;
                        }

                        if (127 == j) // ����ȫ��0xff;
                        {
                            k = i + 1;// ��������0xff
                            i = 128;
                        }
                    }
                }
            }
        }

        IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
        IAP_CTRL_CAN_TxAddData(0x03); // 03: ��������
        IAP_CTRL_CAN_TxAddData(0x00); // ��ʱ������ݳ���
        IAP_CTRL_CAN_TxAddData(dut_info.currentUiSize % 0x100);
        for (i = 0; i < k; i++)
        {
            IAP_CTRL_CAN_TxAddData(dataPacket[i]);
        }
        IAP_CTRL_CAN_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
        break;

    // �ֶ�������һ֡ui����
    case STEP_KAIYANG_CAN_UPGRADE_SEND_UI_AGAIN:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_SEND_UI_WRITE);
        dut_info.currentUiSize++;
        break;

    // ����config���
    case STEP_CAN_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        dut_info.dutPowerOnAllow = TRUE; // ������
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
        DUTCTRL_PowerOnOff(0);
        // 500ms����һ��������Ϣ
        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_CAN_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        // ���5s�ĳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_KAIYANG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����config����
    case STEP_CAN_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�
        DUTCTRL_PowerOnOff(1);                                          // dut�ϵ�
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                // �������ͷ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);         // ����豸��ַ
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM); // 0xc0 ���������
        CAN_PROTOCOL1_TxAddData(54);                                    // ������ݳ���
        for (i = 0; i < 54; i++)
        {
            CAN_PROTOCOL1_TxAddData(configs[i]);
        }
        CAN_PROTOCOL1_TxAddFrame(); // ��Ӽ������������������������
        break;

    // config���óɹ�
    case STEP_CAN_SET_CONFIG_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ����config��ʱ��
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);   // ���ò�����
        dut_info.configUpFlag = FALSE;           // ��־λ��false
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE;
        STATE_EnterState(STATE_STANDBY);
        break;

    // ���������
    case STEP_KAIYANG_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);
        if (dut_info.appUpFlag || dut_info.uiUpFlag)
        {
            DUTCTRL_PowerOnOff(0);                 // �Ǳ�ϵ�
            TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG, // 500ms��������״̬
                          500,
                          STATE_SwitchStep,
                          STEP_KAIYANG_CAN_UPGRADE_ENTRY,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
        }
        else if (dut_info.configUpFlag)
        {
            STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
        }
        else
        {
            // û��������ϵ�
            STATE_EnterState(STATE_STANDBY);
        }
        break;

    case STEP_KAIYANG_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        else if (dut_info.uiUpFlag)
        {
            dut_info.uiUpFaile = TRUE; // ui����ʧ��
        }
        else if (dut_info.configUpFlag)
        {
            dut_info.configUpFaile = TRUE; // config����ʧ��
        }
        STATE_EnterState(STATE_STANDBY);
        TIMER_KillTask(TIMER_ID_RECONNECTION);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        break;

    default:
        break;
    }
}

// IOT��CAN��������
void STEP_Process_IotCanUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;

    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    // ��ڲ���
    case STEP_IOT_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST);
        dut_info.frameType = DUT_FRAME_TYPE_STD;
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
        DUTCTRL_PowerOnOff(1); // dut�ϵ�
        dut_info.dutPowerOnAllow = FALSE;
        dut_info.reconnectionRepeatOne = FALSE;
        addTimeOutTimer(5000); // ���ֳ�ʱʱ�䣺2S��ά�ָߵ�s1��Ҫ2s������ʱ��
        
        dut_info.iotAppUpDataLen = SPI_FLASH_ReadWord(SPI_FLASH_IOT_APP_UP_DATA_LEN);
        dut_info.iotCrc8 = SPI_FLASH_ReadWord(SPI_FLASH_IOT_CRC8);        
        break;

    // �ȴ�
    case STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK:
        break;

    // ���������̼�����
    case STEP_IOT_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        
        // ���������̼�����
        TIMER_AddTask(TIMER_ID_SEND_UP_APP_REQUEST,
                      3000,
                      CAN_PROTOCOL1_SendUpAppReuqest,
                      1,
                      -1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // dut׼���������յ�ͬ�����������͹̼���Ϣ
    case STEP_IOT_CAN_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        CAN_PROTOCOL1_SendCmdUpApp(); 
        break;

    // �̼�����У��ͨ�������͵�һ��APP����
    case STEP_IOT_CAN_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        dut_info.currentAppSize = 0; // ��һ������
        CAN_PROTOCOL1_SendOnePacket_Bin(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize);
        dut_info.currentAppSize++; // ׼����һ������
        break;

    // ����app���ݰ�
    case STEP_IOT_CAN_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        CAN_PROTOCOL1_SendOnePacket_Bin(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128);
        dut_info.currentAppSize++; // ׼����һ������
        break;

    // app�����ɹ�
    case STEP_IOT_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // ֹͣ��ʱ��ʱ��
        CAN_PROTOCOL1_SendCmdNoResult(CAN_PROTOCOL1_CMD_WRITE_COMPLETE); // �������ݰ�д�����
        break;

    // ������������ɣ�����Ƿ�����Ҫ��������
    case STEP_IOT_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;        
        STATE_EnterState(STATE_STANDBY);
        break;

    // ��ʱ����
    case STEP_IOT_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        TIMER_KillTask(TIMER_ID_SEND_UP_APP_REQUEST);
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}

// IOT�Ĵ�����������
void STEP_Process_IotUartUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;

    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    // ��ڲ���
    case STEP_IOT_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_RECEIVE_DUT_ECO_REQUEST);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UARTЭ����ʼ��
        UART_PROTOCOL_Init();                                    // 
        UART_PROTOCOL3_Init();                                   // 3A����Э��
        DUTCTRL_PowerOnOff(1);                       // dut�ϵ�
        
        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      3000,
                      STATE_SwitchStep,
                      STEP_IOT_UART_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // �ȴ�
    case STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK:
        break;

    // ���������̼�����
    case STEP_IOT_UART_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        
        // ���������̼�����
        TIMER_AddTask(TIMER_ID_SEND_UP_APP_REQUEST,
                      3000,
                      UART_PROTOCOL3_SendUpAppReuqest,
                      1,
                      -1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // dut׼���������յ�ͬ�����������͹̼���Ϣ
    case STEP_IOT_UART_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        UART_PROTOCOL3_SendCmdUpApp(); 
        break;

    // �̼�����У��ͨ�������͵�һ��APP����
    case STEP_IOT_UART_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        dut_info.currentAppSize = 0; // ��һ������
        UART_PROTOCOL3_SendOnePacket_Bin(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize);
        dut_info.currentAppSize++; // ׼����һ������
        break;

    // ����app���ݰ�
    case STEP_IOT_UART_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        UART_PROTOCOL3_SendOnePacket_Bin(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128);
        dut_info.currentAppSize++; // ׼����һ������
        break;

    // app�����ɹ�
    case STEP_IOT_UART_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // ֹͣ��ʱ��ʱ��
        UART_PROTOCOL3_SendCmdNoResult(UART_PROTOCOL3_CMD_WRITE_COMPLETE); // �������ݰ�д�����
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // ������������ɣ�����Ƿ�����Ҫ��������
    case STEP_IOT_UART_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        dut_info.appUpFlag = FALSE;
        STATE_EnterState(STATE_STANDBY);
        break;

    // ��ʱ����
    case STEP_IOT_UART_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        TIMER_KillTask(TIMER_ID_SEND_UP_APP_REQUEST);
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}


// LIME�Ĵ�����������
void STEP_Process_LiMeUartUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;

    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    // ��ڲ���
    case STEP_LIME_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UARTЭ����ʼ��
        UART_PROTOCOL_Init();                                    // 55����Э��
        UART_PROTOCOL3_Init();                                 // KM5S
        LIME_UART_PROTOCOL_Init();
        DUTCTRL_PowerOnOff(1);                       // dut�ϵ�
        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      3000,
                      STATE_SwitchStep,
                      STEP_LIME_UART_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // �ȴ�
    case STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK:
        break;

    // �յ�eco���󣬷���������빤��ģʽ
    case STEP_LIME_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        LIME_UART_PROTOCOL_SendCmdParamAck(LIME_UART_PROTOCOL_ECO_CMD_REQUEST, 1); // ���빤��ģʽ
        break;

    // dut׼������������app����ָ��
    case STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        LIME_UART_PROTOCOL_SendCmd(LIME_UART_PROTOCOL_ECO_CMD_APP_ERASE); // 04:����app����
        break;

    // ���͵�һ������
    case STEP_LIME_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        dut_info.currentAppSize = 0; // ��һ������
        LIME_Protocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize);
        dut_info.currentAppSize++; // ׼����һ������
        break;

    // ����app���ݰ�
    case STEP_LIME_UART_SEG_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // ι��
        LIME_Protocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128);
        dut_info.currentAppSize++; // ׼����һ������
        break;

    // app�����ɹ�
    case STEP_LIME_UART_SEG_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // ֹͣ��ʱ��ʱ��
        LIME_UART_PROTOCOL_SendCmdParamAck(LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE_FINISH, TRUE); // ���ͳɹ�
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // ������������ɣ�����Ƿ�����Ҫ��������
    case STEP_LIME_UART_SEG_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        LIME_UART_PROTOCOL_SendCmdParamAck(LIME_UART_PROTOCOL_ECO_CMD_FACTORY_RST, 3);      // ���Ǳ�����app
        dut_info.appUpFlag = FALSE;
        STATE_EnterState(STATE_STANDBY);
        break;

    // ��ʱ����
    case STEP_LIME_UART_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}

// ����ܡ��������Ĵ�����������
void STEP_Process_SegUartUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;

    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    // ��ڲ���
    case STEP_SEG_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UARTЭ����ʼ��
        UART_PROTOCOL_Init();                        // 55����Э��
        UART_PROTOCOL3_Init();                       // KM5S
        DUTCTRL_PowerOnOff(1);                       // dut�ϵ�
        break;

    // �ȴ�
    case STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK:
        break;

    // �յ�eco���󣬷���������빤��ģʽ
    case STEP_CM_UART_SEG_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_PROJECT_ALLOW, TRUE); // ���빤��ģʽ
        break;

    // dut׼������������app����ָ��
    case STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_EAR, TRUE); // 05:����app����
        break;

    // ���͵�һ������
    case STEP_CM_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        dut_info.currentAppSize = 0; // ��һ������
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize);
        dut_info.currentAppSize++; // ׼����һ������

        break;

    // ����app���ݰ�
    case STEP_CM_UART_SEG_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128);
        dut_info.currentAppSize++; // ׼����һ������
        break;

    // app�����ɹ�
    case STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_UP_OVER, TRUE); // ���ͳɹ�
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // ����config���
    case STEP_UART_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(9600); // UARTЭ����ʼ��
        UART_PROTOCOL_Init();        // 55����Э��
        UART_PROTOCOL3_Init();       // KM5S
        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_UART_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        break;

    // ����config������
    case STEP_UART_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        /*����Э���л�����*/
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);       // ����Э���л�����
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR); // ����豸��ַ
        UART_PROTOCOL3_TxAddData(0xAB);                          // 0xAB��Э���л�
        UART_PROTOCOL3_TxAddData(0);                             // ������ݳ���
        UART_PROTOCOL3_TxAddData(01);
        UART_PROTOCOL3_TxAddFrame(); // ��Ӽ������������������������

        /*����dut������Ϣ*/
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60);  // ʵ��ֻ��54���ֽ�
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);               // �������ͷ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // ����豸��ַ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0 ���������
        UART_PROTOCOL3_TxAddData(54);                                    // ������ݳ���
        // UART_PROTOCOL3_TxAddData(1);//��������1��������
        for (i = 0; i < 54; i++)
        {
            UART_PROTOCOL3_TxAddData(configs[i]); // д����
        }
        UART_PROTOCOL3_TxAddFrame(); // ��Ӽ������������������������
        break;

    // ����config���
    case STEP_UART_SET_CONFIG_SUCCESS:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ��ʱ��
        UART_DRIVE_InitSelect(115200);           // ���ò�����
        dut_info.configUpFlag = FALSE;           // ��־λ��false
        DUTCTRL_PowerOnOff(0);                   // �Ǳ�ϵ�
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE;
        break;

    // ������������ɣ�����Ƿ�����Ҫ��������
    case STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        if (dut_info.configUpFlag)
        {
            UART_PROTOCOL_SendCmdParamAck(UART_ECO_CMD_ECO_JUMP_APP, TRUE); // ���Ǳ�����app
            // 500ms�󣨵ȴ��Ǳ�������ʼ����config
            TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                          500,
                          STATE_SwitchStep,
                          STEP_UART_SET_CONFIG_ENTRY,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
        }
        else
        {
            // û��������ϵ�
            STATE_EnterState(STATE_STANDBY);
        }
        break;

    default:
        break;
    }
}

// ����uart����
void STEP_Process_KaiYangUartUpgrade()
{
    uint8 dataPacket[130];
    uint8 i, j, k;
    uint8 configs[60];

    switch (stateCB.step)
    {
    case STEP_KAIYANG_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UARTЭ����ʼ��
        UART_PROTOCOL_Init();                        // 55����Э��
        UART_PROTOCOL3_Init();                       // KM5S
        DUTCTRL_PowerOnOff(1);                       // dut�ϵ�
        dut_info.dutPowerOnAllow = FALSE;
        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_KAIYANG_UART_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK: // �ȴ�
        break;

    case STEP_KAIYANG_UART_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_PROJECT_APPLY, TRUE); // 0x01,����ģʽ��
        // �·��ļ�����
        UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD); // �������ͷ;//ͷ
        UART_PROTOCOL_TxAddData(0x02);        // ���0x02
        UART_PROTOCOL_TxAddData(0x00);        // �����ʱ���ݳ���
        if (dut_info.appUpFlag)
        {
            UART_PROTOCOL_TxAddData(0x03);                  // 0x03:��ʾ��app�ļ�
            UART_PROTOCOL_TxAddData(dut_info.appSize >> 16); // ������λ
            UART_PROTOCOL_TxAddData(dut_info.appSize >> 8); // ������λ
            UART_PROTOCOL_TxAddData(dut_info.appSize);      // ������λ
        }
        else if (dut_info.uiUpFlag)
        {
            UART_PROTOCOL_TxAddData(0x01);                 // 0x01:��ʾ��ui�ļ�
            UART_PROTOCOL_TxAddData(dut_info.uiSize >> 16); // ������λ
            UART_PROTOCOL_TxAddData(dut_info.uiSize >> 8);  // ������λ
            UART_PROTOCOL_TxAddData(dut_info.uiSize);      // ������λ
        }

        UART_PROTOCOL_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
        break;

    case STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);

        TIMER_ResetTimer(TIMER_ID_RECONNECTION);// ι��

        SPI_FLASH_ReadArray(dataPacket, SPI_FLASH_DUT_APP_ADDEESS + (dut_info.currentAppSize) * 128, 128); // ���߶�ȡ128�ֽ�

        k = 128;
        // ȥ�����һ�����ݵ�0xff
        if (dut_info.currentAppSize == dut_info.appSize - 1)
        {
            for (i = 0; i < 128; i++)
            {
                if (0xff == dataPacket[i])
                {
                    for (j = i; j < 128; j++) // �������ȫ��0xff
                    {
                        if (0xff != dataPacket[j - 1])
                        {
                            break;
                        }
                        if (127 == j) // ����ȫ��0xff;
                        {
                            k = i - 1;
                            i = 128;
                        }
                    }
                }
            }
        }
        UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);
        UART_PROTOCOL_TxAddData(0x03);                              // 0x03: ����д������
        UART_PROTOCOL_TxAddData(0x00);                              // ��ʱ������ݳ���
        UART_PROTOCOL_TxAddData(dut_info.currentAppSize % 0x100);   // ���Ͱ���

        for (i = 0; i < k; i++)
        {
            UART_PROTOCOL_TxAddData(dataPacket[i]);
        }
        UART_PROTOCOL_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
        break;

    // �ֶ�������һ֡ui����
    case STEP_KAIYANG_UART_UPGRADE_SEND_APP_AGAIN:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE);
        dut_info.currentAppSize++;
        break;

    case STEP_KAIYANG_UART_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);
        UART_PROTOCOL_TxAddData(0x04);
        UART_PROTOCOL_TxAddData(1); // ���ݳ���
        UART_PROTOCOL_TxAddData(0x01);
        UART_PROTOCOL_TxAddFrame();
        break;

    // ����ui����
    case STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_RECONNECTION);// ι��
        SPI_FLASH_ReadArray(dataPacket, SPI_FLASH_DUT_UI_ADDEESS + (dut_info.currentUiSize) * 128, 128); // ���߶�ȡ128�ֽ�
        k = 128;
        if (dut_info.currentUiSize == dut_info.uiSize - 1) // ���һ��ȥ0xff
        {
            for (i = 0; i < 128; i++)
            {
                if (0xff == dataPacket[i])
                {
                    for (j = i; j < 128; j++) // �������ȫ��0xff
                    {
                        if (0xff != dataPacket[j - 1])
                        {
                            break;
                        }
                        if (127 == j) // ����ȫ��0xff;
                        {
                            k = i + 1;
                            i = 128;
                        }
                    }
                }
            }
        }
        UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);
        UART_PROTOCOL_TxAddData(0x03); // 03: ��������
        UART_PROTOCOL_TxAddData(0x00); // ��ʱ������ݳ���
        UART_PROTOCOL_TxAddData(dut_info.currentUiSize % 0x100);
        for (i = 0; i < k; i++)
        {
            UART_PROTOCOL_TxAddData(dataPacket[i]);
        }
        UART_PROTOCOL_TxAddFrame(); // ����֡��ʽ,�޸ĳ��Ⱥ����У��
        break;

    // �ֶ�������һ֡ui����
    case STEP_KAIYANG_UART_UPGRADE_SEND_UI_AGAIN:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE);
        dut_info.currentUiSize++;
        break;

    // ����config���
    case STEP_UART_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_Init();                        // 55����Э��
        UART_PROTOCOL3_Init();                       // KM5S
        UART_DRIVE_InitSelect(9600); // UARTЭ����ʼ��
        DUTCTRL_PowerOnOff(0);                       // dut�ϵ�
        // 500ms����һ��������Ϣ
        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_UART_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        // ���5s�ĳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_KAIYANG_UART_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����config����
    case STEP_UART_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�
        DUTCTRL_PowerOnOff(1);                                          // dut�ϵ�
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);              // �������ͷ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);        // ����豸��ַ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0 ���������
        UART_PROTOCOL3_TxAddData(54);                                   // ������ݳ���
        for (i = 0; i < 54; i++)
        {
            UART_PROTOCOL3_TxAddData(configs[i]);
        }
        UART_PROTOCOL3_TxAddFrame(); // ��Ӽ������������������������
        break;

    // config���óɹ�
    case STEP_UART_SET_CONFIG_SUCCESS:
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ����config��ʱ��
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // ֹͣ��ʱ��ʱ��
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UARTЭ����ʼ��
        UART_PROTOCOL_Init();                        // 55����Э��
        UART_PROTOCOL3_Init();                       // KM5S

        dut_info.configUpFlag = FALSE;           // ��־λ��false
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE;
        STATE_EnterState(STATE_STANDBY);
        break;

    // ���������
    case STEP_KAIYANG_UART_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        if (dut_info.appUpFlag || dut_info.uiUpFlag)
        {
            DUTCTRL_PowerOnOff(0);                 // �Ǳ�ϵ�
            TIMER_AddTask(TIMER_ID_STATE_CONTROL, // 500ms��������״̬
                          500,
                          STATE_SwitchStep,
                          STEP_KAIYANG_UART_UPGRADE_ENTRY,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
        }
        else if (dut_info.configUpFlag)
        {
            STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
        }
        else
        {
            // û��������ϵ�
            STATE_EnterState(STATE_STANDBY);
            TIMER_KillTask(TIMER_ID_RECONNECTION);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        }
        break;


    // ��ʱ����
    case STEP_KAIYANG_UART_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        else if (dut_info.uiUpFlag)
        {
            dut_info.uiUpFaile = TRUE; // ui����ʧ��
        }
        else if (dut_info.configUpFlag)
        {
            dut_info.configUpFaile = TRUE; // config����ʧ��
        }

        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        TIMER_KillTask(TIMER_ID_RECONNECTION);
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}


// ��о΢����������
void STEP_Process_HuaXinUartCbUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;
    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    // �������
    case STEP_HUAXIN_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // ���ò�����
        UART_PROTOCOL_Init();                        // 55����Э��
        UART_PROTOCOL3_Init();                       // KM5S
        DUTCTRL_PowerOnOff(1);                       // dut�ϵ�
        addTimeOutTimer(5000);                       // ���ֳ�ʱʱ�䣺5S
        break;

    // �ȴ�dutӦ��
    case STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK:
        // ֻ���ȴ�����������
        break;

    // ����dut���빤��ģʽ
    case STEP_HUAXIN_UART_BC_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_PROJECT_ALLOW, TRUE);
        break;

    // ����app����ָ��
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_EAR, TRUE);
        addTimeOutTimer(5000); // ����app��ʱʱ�䣺5s
        break;

    // ���͵�һ֡app����
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        dut_info.currentAppSize = 0;                                                          // ��һ������
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128); // ����128�ֽڣ������в�������ַ
        dut_info.currentAppSize++;                                                            // ׼����һ������
        break;

    // ����app����
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128); // ����128�ֽڣ������в�������ַ
        dut_info.currentAppSize++;
        break;

    // app�������
    case STEP_HUAXIN_UART_BC_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_UP_OVER, TRUE); // ���ͳɹ�
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // ui��������
    // ���Ͳ���ui����
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_UI_DATA_ERASE, TRUE);
        break;

    // ���͵�һ֡ui����
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        dut_info.currentUiSize = 0;                                                         // ��һ������
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_UI_ADDEESS, dut_info.currentUiSize * 128); // ����128�ֽڣ������в�������ַ
        dut_info.currentUiSize++;                                                           // ׼����һ������
        break;

    // ����ui����
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_PACKET:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_UI_ADDEESS, dut_info.currentUiSize * 128); // ����128�ֽڣ������в�������ַ
        dut_info.currentUiSize++;
        break;

    // ui�������
    case STEP_HUAXIN_UART_BC_UPGRADE_UI_UP_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_UP_OVER, TRUE); // ���ͳɹ�
        dut_info.uiUpSuccesss = TRUE;
        dut_info.uiUpFlag = FALSE;
        break;

    // ����config���
    case STEP_UART_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        dut_info.dutPowerOnAllow = TRUE; // ������
        UART_DRIVE_InitSelect(9600);     // UARTЭ����ʼ��
        UART_PROTOCOL_Init();            // 55����Э��
        UART_PROTOCOL3_Init();           // KM5S
        DUTCTRL_PowerOnOff(0);           // �Ǳ�ϵ�

        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_UART_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ����config����
    case STEP_UART_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�
        DUTCTRL_PowerOnOff(1);                                          // dut�ϵ�
        /*����dut������Ϣ*/
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�
        // �������ͷ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
        // ����豸��ַ
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
        // ���������
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0
        // ������ݳ���
        UART_PROTOCOL3_TxAddData(54);
        for (i = 0; i < 54; i++)
        {
            UART_PROTOCOL3_TxAddData(configs[i]);
        }
        // ��Ӽ������������������������
        UART_PROTOCOL3_TxAddFrame();
        break;

    // config���óɹ�
    case STEP_UART_SET_CONFIG_SUCCESS:
        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ����config��ʱ��
        UART_DRIVE_InitSelect(115200); // ���ò�����
        dut_info.configUpFlag = FALSE; // ��־λ��false
        DUTCTRL_PowerOnOff(0);         // �Ǳ�ϵ�
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE; // ������
        break;

    // ���������
    case STEP_HUAXIN_UART_BC_UPGRADE_ITEM_FINISH:
        if (dut_info.uiUpFlag)
        {
            STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR);
        }
        else if (dut_info.configUpFlag)
        {
            STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
        }
        else
        {
            // û��������ϵ�
            STATE_EnterState(STATE_STANDBY);
        }
        break;

    default:
        break;
    }
}

// ͨ�ò�����������
void STEP_Process_CmUartUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i, j;
    
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        // �������
        case STEP_CM_UART_UPGRADE_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // ���ò�����
            UART_PROTOCOL_Init();                        // 55����Э��
            UART_PROTOCOL3_Init();                       // KM5S
            DUTCTRL_PowerOnOff(1);                       // dut�ϵ�
            addTimeOutTimer(5000);                    // ���ֳ�ʱʱ�䣺5S
            break;

        // �ȴ�dutӦ��
        case STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK:
            // ֻ���ȴ�����������
            break;

        // ����dut���빤��ģʽ
        case STEP_CM_UART_BC_UPGRADE_UP_ALLOW:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_PROJECT_ALLOW, TRUE);
            break;

        // ����app����ָ��
        case STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_APP_ERASE_FLASH, TRUE); // 0x24:����app����
            break;

        // ����app����
        case STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            uartProtocol_SendOnePacket_Hex(SPI_FLASH_DUT_APP_ADDEESS);
            dut_info.currentAppSize++;
            break;

        // app�������
        case STEP_CM_UART_BC_UPGRADE_APP_UP_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_UPDATA_FINISH, TRUE); // 0x2A:�����ɹ�
            dut_info.appUpFlag = FALSE;
            dut_info.appUpSuccesss = TRUE;
            break;

        // ui��������
        // ���Ͳ���ui����
        case STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_UI_DATA_ERASE, TRUE); // 0x12:����ui����
            break;

        // ����ui����
        case STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            uartProtocol_SendOnePacket(SPI_FLASH_DUT_UI_ADDEESS, dut_info.currentUiSize * 128); // ����128�ֽڣ������в�������ַ
            dut_info.currentUiSize++;
            break;

        // ui�������
        case STEP_CM_UART_BC_UPGRADE_UI_UP_SUCCESS:
//            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
//            UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_UPDATA_FINISH, TRUE); // 0x2A:�����ɹ�
//            dut_info.uiUpSuccesss = TRUE;
//            dut_info.uiUpFlag = FALSE;
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // ������
            UART_DRIVE_InitSelect(9600);     // UARTЭ����ʼ��
            UART_PROTOCOL_Init();            // 55����Э��
            UART_PROTOCOL3_Init();           // KM5S
            DUTCTRL_PowerOnOff(0);           // �Ǳ�ϵ�

            TIMER_AddTask(TIMER_ID_SET_DUT_UI_VER,
                          1000,
                          STATE_SwitchStep,
                          STEP_CM_UART_BC_WRITE_UI_VER,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);         
            break;

        // дUI�汾��
        case STEP_CM_UART_BC_WRITE_UI_VER:
            writeUiFlag = TRUE;
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            DUTCTRL_PowerOnOff(1);                                           // dut�ϵ�

            // ���Ǳ�﮵�2���л�ΪKM5SЭ��
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);               // �������ͷ
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // ����豸��ַ
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_PROTOCOL_SWITCCH);   // 0xAB ���������
            UART_PROTOCOL3_TxAddData(3);                                     // ������ݳ���
            UART_PROTOCOL3_TxAddData(1);
            UART_PROTOCOL3_TxAddData(1);
            UART_PROTOCOL3_TxAddData(1);

            // ��Ӽ������������������������
            UART_PROTOCOL3_TxAddFrame();

            // ���Ͱ汾��Ϣ
            SPI_FLASH_ReadArray(uiVer, SPI_FLASH_UI_VERSION_ADDEESS, 50); // ʵ��ֻ��54���ֽ�
            
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_VERSION_TYPE_WRITE); // 0xA8
            UART_PROTOCOL3_TxAddData(uiVer[0] + 2);
            UART_PROTOCOL3_TxAddData(8);         // UI�汾����
            UART_PROTOCOL3_TxAddData(uiVer[0]);  //  ��Ϣ����

            j = 0;
            for (i = 1; i < uiVer[0] + 1; i++)
            {
                UART_PROTOCOL3_TxAddData(uiVer[i]);
                uiVerifiedBuff[j++] = uiVer[i];
            }
            UART_PROTOCOL3_TxAddFrame();          
            break;
            
         // ��UI�汾��
        case STEP_CM_UART_BC_READ_UI_VER:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);    
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_VERSION_TYPE_READ); // 0xA9
            UART_PROTOCOL3_TxAddData(1);    // ���ݳ���
            UART_PROTOCOL3_TxAddData(8);    // ����
            UART_PROTOCOL3_TxAddFrame();
            break;
            
        // дUI�汾�ųɹ�
        case STEP_CM_UART_BC_WRITE_UI_VER_SUCCESS:
            writeUiFlag = FALSE;
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_ITEM_FINISH);
            dut_info.uiUpSuccesss = TRUE;
            dut_info.uiUpFlag = FALSE;   
            break;  
            
        // ����config���
        case STEP_UART_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // ������
            UART_DRIVE_InitSelect(9600);     // UARTЭ����ʼ��
            UART_PROTOCOL_Init();            // 55����Э��
            UART_PROTOCOL3_Init();           // KM5S
            DUTCTRL_PowerOnOff(0);           // �Ǳ�ϵ�
            writeUiFlag = FALSE;
            
            TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                          500,
                          STATE_SwitchStep,
                          STEP_UART_PROTOCOL_SWITCCH,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // Э���л�
        case STEP_UART_PROTOCOL_SWITCCH:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            DUTCTRL_PowerOnOff(1);                                           // dut�ϵ�

            // ���Ǳ�﮵�2���л�ΪKM5SЭ��
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);               // �������ͷ
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // ����豸��ַ
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_PROTOCOL_SWITCCH);   // 0xAB ���������
            UART_PROTOCOL3_TxAddData(3);                                     // ������ݳ���
            UART_PROTOCOL3_TxAddData(1);
            UART_PROTOCOL3_TxAddData(1);
            UART_PROTOCOL3_TxAddData(1);

            // ��Ӽ������������������������
            UART_PROTOCOL3_TxAddFrame();
            break;

        // ����config����
        case STEP_UART_SEND_SET_CONFIG:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�
            //          DUTCTRL_PowerOnOff(1);                                          // dut�ϵ�
            /*����dut������Ϣ*/
            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // ʵ��ֻ��54���ֽ�

            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);               // �������ͷ
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // ����豸��ַ
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0 ���������
            UART_PROTOCOL3_TxAddData(54);                                    // ������ݳ���
            for (i = 0; i < 54; i++) // configֻ��54���ֽ�
            {
                UART_PROTOCOL3_TxAddData(configs[i]);
            }
            // ��Ӽ������������������������
            UART_PROTOCOL3_TxAddFrame();

            // ��ȡ��У��buffer
            j = 0;
            // �����������ݣ�ȥ��ʱ�䣬��ȡ��У��buffer
            for (i = 0; i < 54 ; i++)
            {
                if (i == 2)
                {
                    i = i + 5;
                }
                verifiedBuff[j++] = configs[i];
            }
            break;

        // ��ȡ���ò���
        case STEP_UART_READ_CONFIG:
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);                              // ֹͣ����config��ʱ��
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);                    // �������ͷ
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);              // ����豸��ַ
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_READ_CONTROL_PARAM);      // 0xC2���������
            UART_PROTOCOL3_TxAddData(0);                                          // ������ݳ���

            // ��Ӽ������������������������
            UART_PROTOCOL3_TxAddFrame();
            break;
            
        // д��DCD��־
        case STEP_UART_DCD_FLAG_WRITE:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_DCD_FLAG_WRITE);
            UART_PROTOCOL3_TxAddData(0x06);   // ���ݳ���
            UART_PROTOCOL3_TxAddData(0x05);   // DCD��־
            UART_PROTOCOL3_TxAddData(0x00);
            UART_PROTOCOL3_TxAddData(0x00);
            UART_PROTOCOL3_TxAddData(0x00);
            UART_PROTOCOL3_TxAddData(0x02);
            UART_PROTOCOL3_TxAddData(0);
            UART_PROTOCOL3_TxAddFrame();               
            break;
        
        // ��ȡDCD��־
        case STEP_UART_DCD_FLAG_READ:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_DCD_FLAG_READ);
            UART_PROTOCOL3_TxAddData(0);
            UART_PROTOCOL3_TxAddFrame();                           
            break;
            
        // config���óɹ�
        case STEP_UART_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_ITEM_FINISH);
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ����config��ʱ��

            UART_DRIVE_InitSelect(115200); // ���ò�����
            dut_info.configUpFlag = FALSE; // ��־λ��false
            DUTCTRL_PowerOnOff(0);         // �Ǳ�ϵ�
            dut_info.configUpSuccesss = TRUE;
            dut_info.dutPowerOnAllow = FALSE; // ������
            break;

        // ����BOOT���
        case STEP_CM_UART_BC_UPGRADE_BOOT_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // ������
            UART_DRIVE_InitSelect(9600);     // UARTЭ����ʼ��
            UART_PROTOCOL3_Init();           // �ڸ�Э���������
            DTA_UART_PROTOCOL_Init();
            DUTCTRL_PowerOnOff(0);           // �Ǳ�ϵ�

            TIMER_AddTask(TIMER_ID_UPGRADE_DUT_BOOT,
                          500,
                          STATE_SwitchStep,
                          STEP_CM_UART_BC_UPGRADE_SEND_BOOT_EAR,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // ����BOOT����ָ��
        case STEP_CM_UART_BC_UPGRADE_SEND_BOOT_EAR:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            DUTCTRL_PowerOnOff(1);
            DTA_UART_PROTOCOL_SendCmdNoResult(DTA_UART_CMD_DUT_BOOT_ERASE_FLASH);
            break;

        // ����BOOT��������
        case STEP_CM_UART_BC_UPGRADE_SEND_BOOT_PACKET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dtaUartProtocol_SendOnePacket(SPI_FLASH_DUT_BOOT_ADDEESS, (dut_info.currentBootSize) * 128);
            dut_info.currentBootSize++;
            break;

        // BOOT�������
        case STEP_CM_UART_BC_UPGRADE_BOOT_UP_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            DTA_UART_PROTOCOL_SendCmdParamAck(DTA_UART_CMD_DUT_UPDATA_FINISH, TRUE);
            dut_info.bootUpFlag = FALSE;
            dut_info.bootUpSuccesss = TRUE;
            break;

        // ���������
        case STEP_CM_UART_BC_UPGRADE_ITEM_FINISH:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            if (dut_info.appUpFlag)
            {
                dut_info.currentAppSize = 0;
                dut_info.dutPowerOnAllow = FALSE;
                DUTCTRL_PowerOnOff(0);

                TIMER_AddTask(TIMER_ID_DUT_UPGRATE,
                              500,
                              STATE_SwitchStep,
                              STEP_CM_UART_UPGRADE_ENTRY,
                              1,
                              ACTION_MODE_ADD_TO_QUEUE);
            }
            else if (dut_info.uiUpFlag)
            {
                dut_info.currentUiSize = 0;
                STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR); // 0x12:����ui����
            }
            else if (dut_info.configUpFlag)
            {
                STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
            }
            else if (dut_info.bootUpFlag)
            {
                STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_BOOT_ENTRY);
            }
            else
            {
                // û��������ϵ�
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        default:
            break;
    }
}

// ÿ��״̬����ڴ���
void STATE_EnterState(uint32 state)
{
    // �õ�ǰ��״̬��Ϊ��ʷ
    stateCB.preState = stateCB.state;

    // �����µ�״̬
    stateCB.state = (STATE_E)state;

    // ��״̬������趨
    switch (state)
    {
        // ���� ��״̬ ����
        case STATE_NULL: // ��ʹ��
            break;

        case STATE_STANDBY:
            STATE_SwitchStep(STEP_NULL);
            TIMER_KillTask(TIMER_ID_RECONNECTION);          // ����������ʱ��
            TIMER_KillTask(TIMER_ID_RECONNECTION_TIME_OUT); // ������ʱ������ʱ��
            TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION);    // �������ֳ�ʱ��ʱ��
            TIMER_KillTask(TIMER_ID_SET_DUT_UI_VER); 
            dut_info.dutPowerOnAllow = FALSE;               // ������������ʼ����eco����
            dut_info.reconnectionFlag = FALSE;              // ����ϵ�������־λ
            DUTCTRL_PowerOnOff(0);                          // �Ǳ�ϵ�
            break;

        case STATE_GAOBIAO_CAN_UPGRADE:
            // �߱�can����
            STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_ENTRY);
            break;

        case STATE_CM_CAN_UPGRADE:
            // �л�ͨ��can�������
           STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ENTRY);
            break;

        case STATE_HUAXIN_CAN_UPGRADE:
            // �л���о΢��can�������
            STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_ENTRY);
            break;

        case STATE_SEG_CAN_UPGRADE:
            // �������������can����
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_ENTRY);
            break;

        case STATE_KAIYANG_CAN_UPGRADE:
            // ����can�������
            STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_ENTRY);
            break;

        // ͨ�ò�����������
        case STATE_CM_UART_BC_UPGRADE:
            STATE_SwitchStep(STEP_CM_UART_UPGRADE_ENTRY);
            break;

        // ��о΢�ش����������
        case STATE_HUAXIN_UART_BC_UPGRADE:
            STATE_SwitchStep(STEP_HUAXIN_UART_UPGRADE_ENTRY);
            break;

        // ����ܶ����������������
        case STATE_CM_UART_SEG_UPGRADE:
            // �л�ͨ��uart�������
            STATE_SwitchStep(STEP_SEG_UART_UPGRADE_ENTRY);
            break;

        case STATE_KAIYANG_UART_BC_UPGRADE:
            // ����can�������
            STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_ENTRY);
            break;

        case STATE_LIME_UART_BC_UPGRADE:
            // LIME uart�������
            STATE_SwitchStep(STEP_LIME_UART_UPGRADE_ENTRY);
            break;

        case STATE_SPARROW_CAN_UPGRADE:
            // SPARROW_CAN�������
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_ENTRY);
            break;

        case STATE_MEIDI_CAN_UPGRADE:
            // SPARROW_CAN�������
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_ENTRY);
            break;

        case STATE_IOT_UART_UPGRADE:
            // IOT�������
            STATE_SwitchStep(STEP_IOT_UART_UPGRADE_ENTRY);
            break;
            
        case STATE_IOT_CAN_UPGRADE:
            // IOT�������
            STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_ENTRY);
            break;
            
        default:
            break;
    }
}

// ÿ��״̬�µĹ��̴���
void STATE_Process(void)
{
    switch (stateCB.state)
    {
        // ���� ��״̬ ����
        case STATE_NULL:
            break;

        // ����״̬
        case STATE_STANDBY:
            break;

        // �߱�can����
        case STATE_GAOBIAO_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // ��̫����Э��
            CAN_PROTOCOL_Process_GB();      // �߱�����Э��
            CAN_TEST_PROTOCOL_Process();    // ����Э��
            STEP_Process_GaobiaoCanUpgrade();
            break;

        // ����can����
        case STATE_MEIDI_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // ��̫����Э��
            CAN_MD_PROTOCOL_Process();      // ���app�е�3a����Э��
            CAN_TEST_PROTOCOL_Process();    // ����Э��
            STEP_Process_MeiDiCanUpgrade();
            break;
            
        // sparrow can����
        case STATE_SPARROW_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // ��̫����Э��
            CAN_PROTOCOL_Process_3A();      // ���app�е�3a����Э�飬��������
            CAN_TEST_PROTOCOL_Process();    // ����Э��
            STEP_Process_Sparrow_SegCanUpgrade();
            break;

        // ͨ�ò���can����
        case STATE_CM_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // ��̫����Э��
            CAN_PROTOCOL_Process_3A();      // ���app�е�3a����Э��
            CAN_TEST_PROTOCOL_Process();    // ����Э��
            STEP_Process_CmCanUpgrade();
            break;

        // ��о΢��can����
        case STATE_HUAXIN_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // ��̫����Э�飬����app��ui
            CAN_PROTOCOL_Process_3A();      // ���app�е�3a����Э�飬��������
            CAN_TEST_PROTOCOL_Process();    // ����Э��
            STEP_Process_HuaXinCanUpgrade();
            break;

        // ����ܡ�������can����
        case STATE_SEG_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // ��̫����Э��
            CAN_PROTOCOL_Process_3A();      // ���app�е�3a����Э��
            CAN_TEST_PROTOCOL_Process();    // ����Э��
            STEP_Process_SegCanUpgrade();
            break;

        // ����can����
        case STATE_KAIYANG_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // ��̫����Э��
            CAN_PROTOCOL_Process_3A();      // ���app�е�3a����Э��
            CAN_TEST_PROTOCOL_Process();    // ����Э��
            STEP_Process_KaiYangCanUpgrade();
            break;

        // IOT CAN
        case STATE_IOT_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // ��̫����Э��
            CAN_PROTOCOL_Process_3A();      // ���app�е�3a����Э��
            STEP_Process_IotCanUpgrade();
            break;        
        
        /* ����Ϊ��������  */
        // ͨ�ò�����������
        case STATE_CM_UART_BC_UPGRADE:
            UART_DRIVE_Process();           // UART��������̴���
            UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_CmUartUpgrade();   // ����
            break;

        // ��оά�ش�������
        case STATE_HUAXIN_UART_BC_UPGRADE:
            UART_DRIVE_Process();           // UART��������̴���
            UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_HuaXinUartCbUpgrade();
            break;

        // ����ܡ���������������
        case STATE_CM_UART_SEG_UPGRADE:
            UART_DRIVE_Process();           // UART��������̴���
            UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_SegUartUpgrade();
            break;

        // ������������
        case STATE_KAIYANG_UART_BC_UPGRADE:
            UART_DRIVE_Process();           // UART��������̴���
            UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_KaiYangUartUpgrade();
            break;

        // LIME��������
        case STATE_LIME_UART_BC_UPGRADE:
            UART_DRIVE_Process();           // UART��������̴���
            LIME_UART_PROTOCOL_Process();   // UARTЭ�����̴���
            STEP_Process_LiMeUartUpgrade();
            break;

        // IOT��������
        case STATE_IOT_UART_UPGRADE:
            UART_DRIVE_Process();           // UART��������̴���
            UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_IotUartUpgrade();
            break;
            
        default:
            break;
    }
}
