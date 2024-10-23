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

// 步骤切换函数
void STATE_SwitchStep(uint32 param)
{
    stateCB.step = (STEP_E)param;
}

void STEP_Process_GaobiaoCanUpgrade(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    // 入口步骤，做一些初始化的工作
    case STEP_GAOBIAO_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_CAN_INIT);
        dut_info.currentUiSize = 0;
        dut_info.uiUpFaile = FALSE;
        dut_info.currentAppSize = 0;
        dut_info.appUpFaile = FALSE;       
        break;

    // CAN初始化
    case STEP_GAOBIAO_CAN_UPGRADE_CAN_INIT:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_POWER_ON);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
        CAN_PROTOCOL_Init(); // 高标升级初始化
        break;

    // dut上电
    case STEP_GAOBIAO_CAN_UPGRADE_POWER_ON:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAIT_3S);

        // 给DUT上电
        DUTCTRL_PowerOnOff(1);

        // 延时3s
        TIMER_AddTask(TIMER_ID_TIME_GB_POWER_ON,
                      3000,
                      STATE_SwitchStep,
                      STEP_GAOBIAO_CAN_UPGRADE_SEND_KEEP_ELECTRICITY,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);

        break;

    // 这里不需要做任何事情，只需要等待3s即可
    case STEP_GAOBIAO_CAN_UPGRADE_WAIT_3S:
        break;

    // 上电等待3s后执行到此步骤,保电
    case STEP_GAOBIAO_CAN_UPGRADE_SEND_KEEP_ELECTRICITY:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY);
        // 升级开始，UPDATA和UPDATA1确保先跳入boot后发送请求dut接收
        canProtocolCB.UPDATA = FALSE;
        canProtocolCB.UPDATA1 = FALSE;
        CAN_UP_RESET(1); // 电池保电，先保电后升级
        break;

    // 向dut发送跳入boot指令
    case STEP_GAOBIAO_CAN_UPGRADE_SEND_BOOT_CMD:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY);
        CAN_UP_RESET(2); // 1:保电，2：跳入boot

        // 添加超时
        TIMER_AddTask(TIMER_ID_TIME_OUT_DETECTION,
                      10000,
                      STATE_SwitchStep,
                      STEP_GAOBIAO_CAN_UPGRADE_TIME_OUT,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 询问dut是否接收数据
    case STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY);
        // 喂狗
        TIMER_ResetTimer(TIMER_ID_TIME_OUT_DETECTION);
        CAN_UP_DATA(1);

        break;

    case STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY:
        // 只做等待，不做动作
        break;

    // dut允许接收数据后开始发送数据
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

    // dut返回数据写入失败后重新发送
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
        // 清空发送队列
        sysCanCB.tx.head = 0;
        sysCanCB.tx.end = 0;
        break;

    // 升级结束
    case STEP_GAOBIAO_CAN_UPGRADE_FINISH:
        STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_WAIT_3S);
        dut_info.uiUpFlag = FALSE; // 升级项ui复位
        dut_info.currentUiSize = 0;
        dut_info.uiUpSuccesss = TRUE; // 上报升级成功
        dut_info.appUpFlag = FALSE; // 升级项APP复位
        dut_info.currentAppSize = 0;
        dut_info.appUpSuccesss = TRUE; // 上报升级成功 
        
        // dut延时断电
        TIMER_AddTask(TIMER_ID_TIME_GB_POWER_OFF,
                      5000,
                      STATE_SwitchStep,
                      STEP_GAOBIAO_CAN_UPGRADE_POWER_OFF,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
                      
        // 结束超时定时器
        TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION);
        break;

    // dut断电
    case STEP_GAOBIAO_CAN_UPGRADE_POWER_OFF:
        STATE_SwitchStep(STEP_NULL);
        DUTCTRL_PowerOnOff(0); // 0： dut断电
        break;

    // 发送超时
    case STEP_GAOBIAO_CAN_UPGRADE_TIME_OUT:
        STATE_SwitchStep(STEP_NULL);
        DUTCTRL_PowerOnOff(0);     // 0： dut断电
        dut_info.uiUpFaile = TRUE; // ui升级失败
        dut_info.appUpFaile = TRUE; // ui升级失败
        TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION);
        break;

    default:
        break;
    }
}

// 通用CAN升级,APP类型为hex
void STEP_Process_CmCanUpgrade(void)
{
    uint8 configs[60] = {0};
    uint8 i, j;

    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        // 通用can升级入口，dut上电和can初始化
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
            DUTCTRL_PowerOnOff(1); // dut上电
            dut_info.dutPowerOnAllow = FALSE;
            dut_info.reconnectionRepeatOne = FALSE;
            addTimeOutTimer(5000); // 握手超时时间：2S，维乐高的s1需要2s的握手时间
            break;

        // 等待dut应答
        case STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK:
            break;

        // 收到dut的eco请求,延时50ms后发送eco允许
        case STEP_CM_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                          50,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_UP_ALLOW,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 发送eco允许
        case STEP_CM_CAN_UPGRADE_UP_ALLOW:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,工程模式允许
            break;

        // 发送ui擦除指令
        case STEP_CM_CAN_UPGRADE_SEND_UI_EAR:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            addTimeOutTimer(15000);                                         // 擦除ui超时时间：15s
            IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_UI_DATA_ERASE); // 0x12:擦除ui命令
            break;

        // 接收到dut返回的ui擦除结果
        case STEP_CM_CAN_UPGRADE_UI_EAR_RESUALT:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            dut_info.currentUiSize = 0;

            // 命令0x14，ui数据写入
            // 适配BC281维乐高 UI数据分区
            if(DUT_NAME_BC281EVL == dut_info.nam)
            {
                  IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_UI_BIN, IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE, SPI_FLASH_DUT_UI_ADDEESS, (dut_info.currentUiSize) * 132);            
            }
            else
            {
                  IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE, SPI_FLASH_DUT_UI_ADDEESS, (dut_info.currentUiSize) * 128);           
            }
            dut_info.currentUiSize++;
            
            // 添加断电重发定时器
            TIMER_AddTask(TIMER_ID_RECONNECTION,
                          500,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_RECONNECTION,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 发送ui升级数据
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
            
            // 喂狗
            TIMER_ResetTimer(TIMER_ID_RECONNECTION);
            
            // 适配BC281维乐高
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

        // ui升级完成，写入UI版本号
        case STEP_CM_CAN_UPGRADE_UI_UP_SUCCESS:        
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.frameType = DUT_FRAME_TYPE_EXT;
            dut_info.dutPowerOnAllow = TRUE; // 允许开机
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            DUTCTRL_PowerOnOff(0);
            writeUiVerFlag = TRUE;
            addTimeOutTimer(10000); // 设置config时间：10s
            
            TIMER_AddTask(TIMER_ID_SET_DUT_UI_VER,
                          1000,
                          STATE_SwitchStep,
                          STEP_CM_CAN_WRITE_UI_VER,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);    
            break;

        // 写UI版本号
        case STEP_CM_CAN_WRITE_UI_VER:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            SPI_FLASH_ReadArray(uiVer, SPI_FLASH_UI_VERSION_ADDEESS, 50); // 实际只有54个字节
            DUTCTRL_PowerOnOff(1);                                        // dut上电
            Delayms(250);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_VERSION_TYPE_WRITE); // 0xA8
            CAN_PROTOCOL1_TxAddData(uiVer[0] + 2);
            CAN_PROTOCOL1_TxAddData(8);         // UI版本类型
            CAN_PROTOCOL1_TxAddData(uiVer[0]);  // 信息长度
            j = 0;
            
            for (i = 1; i < uiVer[0] + 1; i++)
            {
                CAN_PROTOCOL1_TxAddData(uiVer[i]);
                uiVerifiedBuff[j++] = uiVer[i];
            }
            CAN_PROTOCOL1_TxAddFrame();
            break;
            
        // 读UI版本号
        case STEP_CM_CAN_READ_UI_VER:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);    
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_VERSION_TYPE_READ); // 0xA9
            CAN_PROTOCOL1_TxAddData(1);    // 数据长度
            CAN_PROTOCOL1_TxAddData(8);    // 类型
            CAN_PROTOCOL1_TxAddFrame();
            break;
            
        // 写UI版本号成功
        case STEP_CM_CAN_WRITE_UI_VER_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ITEM_FINISH);
            dut_info.uiUpSuccesss = TRUE;
            dut_info.uiUpFlag = FALSE;
            dut_info.reconnectionFlag = FALSE;
            TIMER_KillTask(TIMER_ID_RECONNECTION);    
            break;        
        
        // 发送app擦除指令
        case STEP_CM_CAN_UPGRADE_SEND_APP_EAR:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            addTimeOutTimer(5000);                                            // 擦除app超时时间：5s
            IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_IAP_ERASE_FLASH); // 0x24,擦除app
            break;

        // app擦除结果处理
        case STEP_CM_CAN_UPGRADE_APP_EAR_RESUALT:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            
            // 发送第一帧数据
            dut_info.currentAppSize = 0;
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_HEX, IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 116);
            dut_info.currentAppSize++;
            
            // 添加断电重发定时器
            TIMER_AddTask(TIMER_ID_RECONNECTION,
                          500,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_RECONNECTION,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 发送app升级包
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
            
            // 喂狗
            TIMER_ResetTimer(TIMER_ID_RECONNECTION);
            IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_HEX, IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 116);
            dut_info.currentAppSize++;
            break;

        // app升级成功
        case STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_UPDATA_FINISH); // 0x2A:升级成功
            dut_info.appUpFlag = FALSE;
            dut_info.appUpSuccesss = TRUE;
            dut_info.reconnectionFlag = FALSE;
            TIMER_KillTask(TIMER_ID_RECONNECTION);
            break;

        // 设置config入口
        case STEP_CAN_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.frameType = DUT_FRAME_TYPE_EXT;
            dut_info.dutPowerOnAllow = TRUE; // 允许开机
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            DUTCTRL_PowerOnOff(0);
            //        addTimeOutTimer(5000); // 设置config时间：5s
            TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                          500,
                          STATE_SwitchStep,
                          STEP_CAN_SEND_SET_CONFIG,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 发送config内容
        case STEP_CAN_SEND_SET_CONFIG:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节
            DUTCTRL_PowerOnOff(1);                                          // dut上电
            
            // 添加命令头
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            
            // 添加设备地址
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            
            // 添加命令字
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM); // 0xc0
            
            // 添加数据长度
            CAN_PROTOCOL1_TxAddData(54);

            for (i = 0; i < 54; i++)
            {
                CAN_PROTOCOL1_TxAddData(configs[i]);
            }
            // 添加检验和与结束符，并添加至发送
            CAN_PROTOCOL1_TxAddFrame();

            j = 0;
            
            // 处理配置数据，去掉时间，获取待校验buffer
            for (i = 0; i < 54 ; i++)
            {
                if (i == 2)
                {
                    i = i + 5;
                }
                verifiedBuff[j++] = configs[i];
            }
            break;

        // 读取配置参数
        case STEP_CAN_READ_CONFIG:
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);                              // 停止设置config定时器
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                      // 添加命令头
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM);        // 0xC2添加命令字
            CAN_PROTOCOL1_TxAddData(0);                                           // 添加数据长度

            // 添加检验和与结束符，并添加至发送
            CAN_PROTOCOL1_TxAddFrame();
            break;

        // 写入DCD标志
        case STEP_CAN_DCD_FLAG_WRITE:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DCD_FLAG_WRITE);
            CAN_PROTOCOL1_TxAddData(0x06);   // 数据长度
            CAN_PROTOCOL1_TxAddData(0x05);   // DCD标志
            CAN_PROTOCOL1_TxAddData(0x00);
            CAN_PROTOCOL1_TxAddData(0x00);
            CAN_PROTOCOL1_TxAddData(0x00);
            CAN_PROTOCOL1_TxAddData(0x02);
            CAN_PROTOCOL1_TxAddData(0);
            CAN_PROTOCOL1_TxAddFrame();               
            break;
        
        // 读取DCD标志
        case STEP_CAN_DCD_FLAG_READ:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DCD_FLAG_READ);
            CAN_PROTOCOL1_TxAddData(0);
            CAN_PROTOCOL1_TxAddFrame();                           
            break;

        // config设置成功
        case STEP_CAN_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ITEM_FINISH);
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止设置config定时器
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // 停止超时定时器
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);   // 设置波特率
            DUTCTRL_PowerOnOff(0);         // dut断电
            dut_info.configUpFlag = FALSE; // 标志位置false
            dut_info.configUpSuccesss = TRUE;
            dut_info.dutPowerOnAllow = FALSE;
            dut_info.frameType = DUT_FRAME_TYPE_STD;
            break;

        // 更新BOOT入口
        case STEP_CM_CAN_BC_UPGRADE_BOOT_ENTRY:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // 允许开机
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            CAN_TEST_PROTOCOL_Init();
            DUTCTRL_PowerOnOff(0);           // 仪表断电
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            TIMER_AddTask(TIMER_ID_UPGRADE_DUT_BOOT,
                          500,
                          STATE_SwitchStep,
                          STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_EAR,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
                          dut_info.frameType = DUT_FRAME_TYPE_EXT;
            break;

        // 发送BOOT擦除指令
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

        // 发送BOOT升级数据
        case STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_PACKET:
           TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            dut_info.bootEaring = FALSE;
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_TEST_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE_BIN, CAN_TEST_PROTOCOL_CMD_BOOT_WRITE_FLASH, SPI_FLASH_DUT_BOOT_ADDEESS, (dut_info.currentBootSize) * 128);
            dut_info.currentBootSize++;
            break;

        // BOOT升级完成
        case STEP_CM_CAN_BC_UPGRADE_BOOT_UP_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_TEST_PROTOCOL_SendCmdWithResult(CAN_TEST_PROTOCOL_CMD_BOOT_UPDATA_FINISH, TRUE);
            dut_info.bootUpFlag = FALSE;
            dut_info.bootUpSuccesss = TRUE;
            break;

        // 升级项升级成功
        case STEP_CM_CAN_UPGRADE_ITEM_FINISH:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            // 选择升级
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
                STATE_SwitchStep(STEP_CM_CAN_UPGRADE_SEND_UI_EAR); // 0x12:擦除ui命令
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
                // 没有升级项断电
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 握手超时
        case STEP_CM_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
            if (dut_info.appUpFlag)
            {
                dut_info.appUpFaile = TRUE;
            }
            else if (dut_info.uiUpFlag)
            {
                dut_info.uiUpFaile = TRUE; // ui升级失败
            }
            else if (dut_info.configUpFlag)
            {
                dut_info.configUpFaile = TRUE; // config升级失败
            }
            else if (dut_info.bootUpFlag)
            {
                dut_info.bootUpFaile = TRUE;   // boot更新失败
            }

            STATE_EnterState(STATE_STANDBY);
            break;

        // dut断电重新连接
        case STEP_CM_CAN_UPGRADE_RECONNECTION:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.reconnectionRepeatOne = TRUE;
            dut_info.reconnectionFlag = TRUE;
            // 每100ms发送一次eco允许
            TIMER_AddTask(TIMER_ID_RECONNECTION,
                          100,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_UP_ALLOW,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);

            // 3s后停止发送eco允许，上报升级失败
            TIMER_AddTask(TIMER_ID_RECONNECTION_TIME_OUT,
                          3000,
                          STATE_SwitchStep,
                          STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 重新连接超时
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
                dut_info.bootUpFaile = TRUE;   // boot更新失败
            }

            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 华芯微特can升级步骤,app是bin类型
void STEP_Process_HuaXinCanUpgrade()
{
    uint8 configs[60] = {0};
    uint8 i;
    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    // 通用can升级入口，dut上电和can初始化
    case STEP_HUAXIN_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);
        DUTCTRL_PowerOnOff(1); // dut上电
        dut_info.dutPowerOnAllow = FALSE;
        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      10000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 等待dut应答
    case STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK:
        break;

    // 收到dut的eco请求,延时50ms后发送eco允许
    case STEP_HUAXIN_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        
        // 50ms后发送eco允许
        TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                      50,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_UP_ALLOW,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);

        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      2000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送eco允许
    case STEP_HUAXIN_CAN_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,工程模式允
        break;

    // 发送app擦除指令
    case STEP_HUAXIN_CAN_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_APP_EAR); // 0x05,擦除app
        // 添加超时,8秒的app擦除时间
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      8000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送app升级包
    case STEP_HUAXIN_CAN_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
        dut_info.currentAppSize++;
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,// 喂狗+重设超时时间（由原来的8s改为1s）
                      1000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // app升级成功
    case STEP_HUAXIN_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP_COMPLETE); // 0x07:升级成功
        dut_info.appUpFlag = FALSE;
        dut_info.appUpSuccesss = TRUE;
        break;

    // 发送ui擦除指令
    case STEP_HUAXIN_CAN_UPGRADE_SEND_UI_EAR:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_UI_DATA_ERASE); // 0x12:擦除ui命令
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      30000,                // ui擦除时间10s
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送ui升级数据
    case STEP_HUAXIN_CAN_UPGRADE_SEND_UI_WRITE:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE, SPI_FLASH_DUT_UI_ADDEESS, (dut_info.currentUiSize) * 128);
        dut_info.currentUiSize++;
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT, // 喂狗+重设超时时间（由原来的10s改为1s）
                      2000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // ui升级完成
    case STEP_HUAXIN_CAN_UPGRADE_UI_UP_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_UPDATA_FINISH); // 0x2A:升级成功
        dut_info.uiUpSuccesss = TRUE;
        dut_info.uiUpFlag = FALSE;
        dut_info.reconnectionFlag = FALSE;
        break;

    // 设置config入口
    case STEP_CAN_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        dut_info.dutPowerOnAllow = TRUE; // 允许开机
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
        DUTCTRL_PowerOnOff(0);
        // 500ms发送一次配置信息
        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_CAN_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        // 添加5s的超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      5000,
                      STATE_SwitchStep,
                      STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送config内容
    case STEP_CAN_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节
        DUTCTRL_PowerOnOff(1);                                          // dut上电
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                // 添加命令头
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);         // 添加设备地址
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM); // 0xc0 添加命令字
        CAN_PROTOCOL1_TxAddData(54);                                    // 添加数据长度
        for (i = 0; i < 54; i++)
        {
            CAN_PROTOCOL1_TxAddData(configs[i]);
        }
        CAN_PROTOCOL1_TxAddFrame(); // 添加检验和与结束符，并添加至发送
        break;

    // config设置成功
    case STEP_CAN_SET_CONFIG_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止设置config定时器
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);   // 设置波特率
        dut_info.configUpFlag = FALSE;           // 标志位置false
        DUTCTRL_PowerOnOff(0);                   // dut断电
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE;
        break;

    // 升级项升级成功
    case STEP_HUAXIN_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        // 选择升级
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
            // 没有升级项断电
            STATE_EnterState(STATE_STANDBY);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        }
        break;

    // 握手超时
    case STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
        dut_info.currentUiSize--;
			if (dut_info.currentUiSize < (dut_info.uiSize + 1))
        {
            STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_SEND_UI_WRITE);
        }
        break;

    // dut断电重新连接
    case STEP_CM_CAN_UPGRADE_RECONNECTION:
        STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
        dut_info.reconnectionRepeatOne = TRUE;
        // 每100ms发送一次eco允许
        TIMER_AddTask(TIMER_ID_RECONNECTION,
                      100,
                      STATE_SwitchStep,
                      STEP_CM_CAN_UPGRADE_UP_ALLOW,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        // 3s后停止发送eco允许，上报升级失败
        TIMER_AddTask(TIMER_ID_RECONNECTION_TIME_OUT,
                      3000,
                      STATE_SwitchStep,
                      STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        dut_info.reconnectionFlag = TRUE;
        break;

    // 重新连接超时
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

// 段码屏、数码管can升级，app是bin格式
void STEP_Process_SegCanUpgrade()
{
    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    // 通用can升级入口，dut上电和can初始化
    case STEP_SEG_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);
        DUTCTRL_PowerOnOff(1); // dut上电
        dut_info.dutPowerOnAllow = FALSE;

        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      2000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 等待dut应答
    case STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK:
        break;

    // 收到dut的eco请求,延时50ms后发送eco允许
    case STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        // 50ms后发送eco允许
        TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                      50,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_UP_ALLOW,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        // 添加1s的握手超时检测
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      1000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // 握手超时
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送eco允许
    case STEP_SEG_CAN_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,工程模式允
        break;

    // 发送app擦除指令
    case STEP_SEG_CAN_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_APP_EAR); // 05:擦除app命令
        // 添加8s的握手超时检测
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      8000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // 握手超时
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // app擦除结果处理
    case STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        // 发送第一帧数据
        dut_info.currentAppSize = 0;
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_HEX, IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 116);
        dut_info.currentAppSize++;
        // 添加1s的握手超时检测
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      1000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // 握手超时
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送app升级包
    case STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
        dut_info.currentAppSize++;
        break;

    // app升级成功
    case STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP_COMPLETE); // 0x07:升级成功
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // 设置config入口
    //  case STEP_CAN_SET_CONFIG_ENTRY:
    //      STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
    //      dut_info.dutPowerOnAllow = TRUE; // 允许开机
    //      IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
    //      DUTCTRL_PowerOnOff(0);
    //      addTimeOutTimer(5000); // 设置config时间：5s
    //      TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
    //                    500,
    //                    STATE_SwitchStep,
    //                    STEP_CAN_SEND_SET_CONFIG,
    //                    TIMER_LOOP_FOREVER,
    //                    ACTION_MODE_ADD_TO_QUEUE);
    //      break;

    //  // 发送config内容
    //  case STEP_CAN_SEND_SET_CONFIG:
    //      STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
    //      SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节
    //      DUTCTRL_PowerOnOff(1);                                          // dut上电
    //      /* 发送协议切换命令 */
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);        // 添加命令头
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR); // 添加设备地址
    //      CAN_PROTOCOL1_TxAddData(0xab);                          // 0xab：协议切换
    //      CAN_PROTOCOL1_TxAddData(0);                             // 添加数据长度
    //      CAN_PROTOCOL1_TxAddData(0x01);
    //      CAN_PROTOCOL1_TxAddFrame(); // 添加检验和与结束符，并添加至发送

    //      /* 发送配置信息 */
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                // 添加命令头
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);         // 添加设备地址
    //      CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM); // 0xc0 添加命令字
    //      CAN_PROTOCOL1_TxAddData(54);                                    // 添加数据长度
    //      for (i = 0; i < 54; i++)
    //      {
    //          CAN_PROTOCOL1_TxAddData(configs[i]);
    //      }
    //      CAN_PROTOCOL1_TxAddFrame(); // 添加检验和与结束符，并添加至发送
    //      break;

    //  // config设置成功
    //  case STEP_CAN_SET_CONFIG_SUCCESS:
    //      STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_ITEM_FINISH);
    //      TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止设置config定时器

    //      IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K); // 设置波特率
    //      dut_info.configUpFlag = FALSE;         // 标志位置false
    //      DUTCTRL_PowerOnOff(0);                 // dut断电
    //      dut_info.configUpSuccesss = TRUE;
    //      dut_info.dutPowerOnAllow = FALSE;
    //      break;

    // 升级项升级成功
    case STEP_SEG_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        // 选择升级
        if (dut_info.configUpFlag)
        {
            STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
        }
        else
        {
            STATE_EnterState(STATE_STANDBY); // 没有升级项断电
        }
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT); // 停止超时计时器
        break;

    // 握手超时
    case STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        else if (dut_info.uiUpFlag)
        {
            dut_info.uiUpFaile = TRUE; // ui升级失败
        }
        else if (dut_info.configUpFlag)
        {
            dut_info.configUpFaile = TRUE; // config升级失败
        }
        STATE_EnterState(STATE_STANDBY);
        break;

    // dut断电重新连接
    case STEP_CM_CAN_UPGRADE_RECONNECTION:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        break;

    // 重新连接超时
    case STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT:
        STATE_SwitchStep(STEP_NULL);

        break;

    default:
        break;
    }
}

// Meidi can升级，app是bin格式
void STEP_Process_MeiDiCanUpgrade()
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        // 通用can升级入口，dut上电和can初始化
        case STEP_SEG_CAN_UPGRADE_ENTRY:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
            CAN_MD_PROTOCOL_Init(); // MeiDi升级初始化
            DUTCTRL_PowerOnOff(1);  // dut上电
            dut_info.dutPowerOnAllow = FALSE;

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          6000,
                          STATE_SwitchStep,
                          STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 等待dut应答
        case STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK:
            break;

        // 收到dut的eco请求,延时50ms后发送eco允许
        case STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            // 50ms后发送eco允许
            TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                          50,
                          STATE_SwitchStep,
                          STEP_SEG_CAN_UPGRADE_UP_ALLOW,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
//            // 添加1s的握手超时检测
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          500,
                          STATE_SwitchStep,
                          STEP_SEG_CAN_UPGRADE_UP_ALLOW,            // 握手超时
                          -1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 发送eco允许
        case STEP_SEG_CAN_UPGRADE_UP_ALLOW:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_MD_PROTOCOL_SendCmdNoResult(CAN_MD_PROTOCOL_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,工程模式允
            break;

        // APP数据写入
        case STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

            // 发送第一帧数据
            dut_info.currentAppSize = 0;
            CAN_MD_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE_BIN, CAN_MD_PROTOCOL_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
            dut_info.currentAppSize++;
            
            // 添加1s的握手超时检测
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          1000,
                          STATE_SwitchStep,
                          STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // 握手超时
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 发送app升级包
        case STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
            CAN_MD_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE_BIN, CAN_MD_PROTOCOL_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
            dut_info.currentAppSize++;
            break;

        // app升级成功
        case STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_MD_PROTOCOL_SendCmdNoResult(CAN_MD_PROTOCOL_CMD_DOWN_UPDATA_FINISH); // 0x07:升级成功
            dut_info.appUpSuccesss = TRUE;
            dut_info.appUpFlag = FALSE;
            break;

        // 升级项升级成功
        case STEP_SEG_CAN_UPGRADE_ITEM_FINISH:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

            // 选择升级
            if (dut_info.configUpFlag)
            {
                STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
            }
            else
            {
                STATE_EnterState(STATE_STANDBY); // 没有升级项断电
            }
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT); // 停止超时计时器
            break;

        // 握手超时
        case STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
            if (dut_info.appUpFlag)
            {
                dut_info.appUpFaile = TRUE;
            }
            else if (dut_info.uiUpFlag)
            {
                dut_info.uiUpFaile = TRUE; // ui升级失败
            }
            else if (dut_info.configUpFlag)
            {
                dut_info.configUpFaile = TRUE; // config升级失败
            }
            STATE_EnterState(STATE_STANDBY);
            break;

        // dut断电重新连接
        case STEP_CM_CAN_UPGRADE_RECONNECTION:
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

            break;

        // 重新连接超时
        case STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT:
            STATE_SwitchStep(STEP_NULL);

            break;

        default:
            break;
    }
}

// Sparrow can升级，app是bin格式
void STEP_Process_Sparrow_SegCanUpgrade()
{
    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    // 通用can升级入口，dut上电和can初始化
    case STEP_SEG_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
        DUTCTRL_PowerOnOff(1); // dut上电
        dut_info.dutPowerOnAllow = FALSE;

        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      6000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 等待dut应答
    case STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK:
        break;

    // 收到dut的eco请求,延时50ms后发送eco允许
    case STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        // 50ms后发送eco允许
        TIMER_AddTask(TIMER_ID_DELAY_ENTER_UP_MODE,
                      50,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_UP_ALLOW,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        // 添加1s的握手超时检测
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      1000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // 握手超时
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送eco允许
    case STEP_SEG_CAN_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK); // 0x02,工程模式允
        break;

    // 发送app擦除指令
    case STEP_SEG_CAN_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_APP_EAR); // 05:擦除app命令
        // 添加8s的握手超时检测
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      8000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // 握手超时
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // app擦除结果处理
    case STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        // 发送第一帧数据
        dut_info.currentAppSize = 0;
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
        dut_info.currentAppSize++;
        // 添加1s的握手超时检测
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      1000,
                      STATE_SwitchStep,
                      STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,            // 握手超时
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送app升级包
    case STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE_BIN, IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP, SPI_FLASH_DUT_APP_ADDEESS, (dut_info.currentAppSize) * 128);
        dut_info.currentAppSize++;
        break;

    // app升级成功
    case STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP_COMPLETE); // 0x07:升级成功
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // 升级项升级成功
    case STEP_SEG_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        // 选择升级
        if (dut_info.configUpFlag)
        {
            STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
        }
        else
        {
            STATE_EnterState(STATE_STANDBY); // 没有升级项断电
        }
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT); // 停止超时计时器
        break;

    // 握手超时
    case STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        else if (dut_info.uiUpFlag)
        {
            dut_info.uiUpFaile = TRUE; // ui升级失败
        }
        else if (dut_info.configUpFlag)
        {
            dut_info.configUpFaile = TRUE; // config升级失败
        }
        STATE_EnterState(STATE_STANDBY);
        break;

    // dut断电重新连接
    case STEP_CM_CAN_UPGRADE_RECONNECTION:
        STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK);

        break;

    // 重新连接超时
    case STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT:
        STATE_SwitchStep(STEP_NULL);

        break;

    default:
        break;
    }
}

// 开阳can升级步骤
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
        DUTCTRL_PowerOnOff(1); // dut上电
        dut_info.dutPowerOnAllow = FALSE;
        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_KAIYANG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK: // 等待
        break;

    case STEP_KAIYANG_CAN_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_SendCmdNoAck(IAP_CTRL_CAN_CMD_UP_PROJECT_APPLY); // 0x01,工程模式允
        // 下发文件类型
        IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD); // 添加命令头;//头
        IAP_CTRL_CAN_TxAddData(0x02);         // 命令，0x02
        IAP_CTRL_CAN_TxAddData(0x00);         // 添加临时数据长度
        if (dut_info.appUpFlag)
        {
            IAP_CTRL_CAN_TxAddData(0x03);                   // 0x03:表示是app文件
            IAP_CTRL_CAN_TxAddData(dut_info.appSize >> 16); // 包数高位
            IAP_CTRL_CAN_TxAddData(dut_info.appSize >> 8);  // 包数中位
            IAP_CTRL_CAN_TxAddData(dut_info.appSize);       // 包数低位
        }
        else if (dut_info.uiUpFlag)
        {
            IAP_CTRL_CAN_TxAddData(0x01);                  // 0x01:表示是ui文件
            IAP_CTRL_CAN_TxAddData(dut_info.uiSize >> 16); // 包数高位
            IAP_CTRL_CAN_TxAddData(dut_info.uiSize >> 8);  // 包数中位
            IAP_CTRL_CAN_TxAddData(dut_info.uiSize);       // 包数低位
        }

        IAP_CTRL_CAN_TxAddFrame(); // 调整帧格式,修改长度和添加校验
        break;

    case STEP_KAIYANG_CAN_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);

        TIMER_ResetTimer(TIMER_ID_RECONNECTION);// 喂狗
        SPI_FLASH_ReadArray(dataPacket, SPI_FLASH_DUT_APP_ADDEESS + (dut_info.currentAppSize) * 128, 128); // 工具读取128字节

        k = 128;

        // 去掉最后一包数据的0xff
        if (dut_info.currentAppSize == dut_info.appSize - 1)
        {
            for (i = 0; i < 128; i++)
            {
                if (0xff == dataPacket[i])
                {
                    for (j = i; j < 128; j++) // 如果后面全是0xff
                    {
                        if (0xff != dataPacket[j - 1])
                        {
                            break;
                        }

                        if (127 == j) // 后面全是0xff
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
        IAP_CTRL_CAN_TxAddData(0x00);                            // 临时添加数据长度
        IAP_CTRL_CAN_TxAddData(dut_info.currentAppSize % 0x100); // 临时添加数据长度

        for (i = 0; i < k; i++)
        {
            IAP_CTRL_CAN_TxAddData(dataPacket[i]);
        }
        IAP_CTRL_CAN_TxAddFrame(); // 调整帧格式,修改长度和添加校验
        break;

    // 手动发送下一帧ui数据
    case STEP_KAIYANG_CAN_UPGRADE_SEND_APP_AGAIN:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_SEND_APP_WRITE);
        dut_info.currentAppSize++;
        break;

    case STEP_KAIYANG_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);
        IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
        IAP_CTRL_CAN_TxAddData(0x04);
        IAP_CTRL_CAN_TxAddData(1); // 数据长度
        IAP_CTRL_CAN_TxAddData(0x01);
        IAP_CTRL_CAN_TxAddFrame();
        TIMER_KillTask(TIMER_ID_RECONNECTION);
        break;

    // 发送ui数据
    case STEP_KAIYANG_CAN_UPGRADE_SEND_UI_WRITE:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_RECONNECTION);// 喂狗
        SPI_FLASH_ReadArray(dataPacket, SPI_FLASH_DUT_UI_ADDEESS + (dut_info.currentUiSize) * 128, 128); // 工具读取128字节
        k = 128;

        if (dut_info.currentUiSize == dut_info.uiSize - 1) // 最后一包去0xff
        {
            for (i = 0; i < 128; i++)
            {
                if (0xff == dataPacket[i])
                {
                    for (j = i; j < 128; j++) // 如果后面全是0xff
                    {
                        if (0xff != dataPacket[j - 1])
                        {
                            break;
                        }

                        if (127 == j) // 后面全是0xff;
                        {
                            k = i + 1;// 保留两个0xff
                            i = 128;
                        }
                    }
                }
            }
        }

        IAP_CTRL_CAN_TxAddData(CAN_CMD_HEAD);
        IAP_CTRL_CAN_TxAddData(0x03); // 03: 升级命令
        IAP_CTRL_CAN_TxAddData(0x00); // 临时添加数据长度
        IAP_CTRL_CAN_TxAddData(dut_info.currentUiSize % 0x100);
        for (i = 0; i < k; i++)
        {
            IAP_CTRL_CAN_TxAddData(dataPacket[i]);
        }
        IAP_CTRL_CAN_TxAddFrame(); // 调整帧格式,修改长度和添加校验
        break;

    // 手动发送下一帧ui数据
    case STEP_KAIYANG_CAN_UPGRADE_SEND_UI_AGAIN:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_SEND_UI_WRITE);
        dut_info.currentUiSize++;
        break;

    // 设置config入口
    case STEP_CAN_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        dut_info.dutPowerOnAllow = TRUE; // 允许开机
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
        DUTCTRL_PowerOnOff(0);
        // 500ms发送一次配置信息
        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_CAN_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        // 添加5s的超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_KAIYANG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送config内容
    case STEP_CAN_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节
        DUTCTRL_PowerOnOff(1);                                          // dut上电
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                // 添加命令头
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);         // 添加设备地址
        CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM); // 0xc0 添加命令字
        CAN_PROTOCOL1_TxAddData(54);                                    // 添加数据长度
        for (i = 0; i < 54; i++)
        {
            CAN_PROTOCOL1_TxAddData(configs[i]);
        }
        CAN_PROTOCOL1_TxAddFrame(); // 添加检验和与结束符，并添加至发送
        break;

    // config设置成功
    case STEP_CAN_SET_CONFIG_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止设置config定时器
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);   // 设置波特率
        dut_info.configUpFlag = FALSE;           // 标志位置false
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE;
        STATE_EnterState(STATE_STANDBY);
        break;

    // 升级项结束
    case STEP_KAIYANG_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK);
        if (dut_info.appUpFlag || dut_info.uiUpFlag)
        {
            DUTCTRL_PowerOnOff(0);                 // 仪表断电
            TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG, // 500ms后进入入口状态
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
            // 没有升级项断电
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
            dut_info.uiUpFaile = TRUE; // ui升级失败
        }
        else if (dut_info.configUpFlag)
        {
            dut_info.configUpFaile = TRUE; // config升级失败
        }
        STATE_EnterState(STATE_STANDBY);
        TIMER_KillTask(TIMER_ID_RECONNECTION);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        break;

    default:
        break;
    }
}

// IOT的CAN升级步骤
void STEP_Process_IotCanUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;

    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    // 入口步骤
    case STEP_IOT_CAN_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST);
        dut_info.frameType = DUT_FRAME_TYPE_STD;
        IAP_CTRL_CAN_Init(CAN_BAUD_RATE_250K);
        DUTCTRL_PowerOnOff(1); // dut上电
        dut_info.dutPowerOnAllow = FALSE;
        dut_info.reconnectionRepeatOne = FALSE;
        addTimeOutTimer(5000); // 握手超时时间：2S，维乐高的s1需要2s的握手时间
        
        dut_info.iotAppUpDataLen = SPI_FLASH_ReadWord(SPI_FLASH_IOT_APP_UP_DATA_LEN);
        dut_info.iotCrc8 = SPI_FLASH_ReadWord(SPI_FLASH_IOT_CRC8);        
        break;

    // 等待
    case STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK:
        break;

    // 发送升级固件请求
    case STEP_IOT_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        
        // 发送升级固件请求
        TIMER_AddTask(TIMER_ID_SEND_UP_APP_REQUEST,
                      3000,
                      CAN_PROTOCOL1_SendUpAppReuqest,
                      1,
                      -1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // dut准备就绪，收到同意升级，发送固件信息
    case STEP_IOT_CAN_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        CAN_PROTOCOL1_SendCmdUpApp(); 
        break;

    // 固件长度校验通过，发送第一包APP数据
    case STEP_IOT_CAN_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        dut_info.currentAppSize = 0; // 第一包数据
        CAN_PROTOCOL1_SendOnePacket_Bin(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize);
        dut_info.currentAppSize++; // 准备下一包数据
        break;

    // 发送app数据包
    case STEP_IOT_CAN_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        CAN_PROTOCOL1_SendOnePacket_Bin(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128);
        dut_info.currentAppSize++; // 准备下一包数据
        break;

    // app升级成功
    case STEP_IOT_CAN_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // 停止超时定时器
        CAN_PROTOCOL1_SendCmdNoResult(CAN_PROTOCOL1_CMD_WRITE_COMPLETE); // 发送数据包写入完成
        break;

    // 升级项升级完成，检查是否还有需要升级的项
    case STEP_IOT_CAN_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK);
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;        
        STATE_EnterState(STATE_STANDBY);
        break;

    // 超时处理
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

// IOT的串口升级步骤
void STEP_Process_IotUartUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;

    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    // 入口步骤
    case STEP_IOT_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_RECEIVE_DUT_ECO_REQUEST);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UART协议层初始化
        UART_PROTOCOL_Init();                                    // 
        UART_PROTOCOL3_Init();                                   // 3A升级协议
        DUTCTRL_PowerOnOff(1);                       // dut上电
        
        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      3000,
                      STATE_SwitchStep,
                      STEP_IOT_UART_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 等待
    case STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK:
        break;

    // 发送升级固件请求
    case STEP_IOT_UART_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        
        // 发送升级固件请求
        TIMER_AddTask(TIMER_ID_SEND_UP_APP_REQUEST,
                      3000,
                      UART_PROTOCOL3_SendUpAppReuqest,
                      1,
                      -1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // dut准备就绪，收到同意升级，发送固件信息
    case STEP_IOT_UART_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        UART_PROTOCOL3_SendCmdUpApp(); 
        break;

    // 固件长度校验通过，发送第一包APP数据
    case STEP_IOT_UART_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        dut_info.currentAppSize = 0; // 第一包数据
        UART_PROTOCOL3_SendOnePacket_Bin(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize);
        dut_info.currentAppSize++; // 准备下一包数据
        break;

    // 发送app数据包
    case STEP_IOT_UART_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        UART_PROTOCOL3_SendOnePacket_Bin(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128);
        dut_info.currentAppSize++; // 准备下一包数据
        break;

    // app升级成功
    case STEP_IOT_UART_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // 停止超时定时器
        UART_PROTOCOL3_SendCmdNoResult(UART_PROTOCOL3_CMD_WRITE_COMPLETE); // 发送数据包写入完成
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // 升级项升级完成，检查是否还有需要升级的项
    case STEP_IOT_UART_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK);
        dut_info.appUpFlag = FALSE;
        STATE_EnterState(STATE_STANDBY);
        break;

    // 超时处理
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


// LIME的串口升级步骤
void STEP_Process_LiMeUartUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;

    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    // 入口步骤
    case STEP_LIME_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UART协议层初始化
        UART_PROTOCOL_Init();                                    // 55升级协议
        UART_PROTOCOL3_Init();                                 // KM5S
        LIME_UART_PROTOCOL_Init();
        DUTCTRL_PowerOnOff(1);                       // dut上电
        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      3000,
                      STATE_SwitchStep,
                      STEP_LIME_UART_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 等待
    case STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK:
        break;

    // 收到eco请求，发送允许进入工程模式
    case STEP_LIME_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        LIME_UART_PROTOCOL_SendCmdParamAck(LIME_UART_PROTOCOL_ECO_CMD_REQUEST, 1); // 进入工程模式
        break;

    // dut准备就绪，发送app擦除指令
    case STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        LIME_UART_PROTOCOL_SendCmd(LIME_UART_PROTOCOL_ECO_CMD_APP_ERASE); // 04:擦除app命令
        break;

    // 发送第一包数据
    case STEP_LIME_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        dut_info.currentAppSize = 0; // 第一包数据
        LIME_Protocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize);
        dut_info.currentAppSize++; // 准备下一包数据
        break;

    // 发送app数据包
    case STEP_LIME_UART_SEG_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT); // 喂狗
        LIME_Protocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128);
        dut_info.currentAppSize++; // 准备下一包数据
        break;

    // app升级成功
    case STEP_LIME_UART_SEG_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // 停止超时定时器
        LIME_UART_PROTOCOL_SendCmdParamAck(LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE_FINISH, TRUE); // 发送成功
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // 升级项升级完成，检查是否还有需要升级的项
    case STEP_LIME_UART_SEG_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        LIME_UART_PROTOCOL_SendCmdParamAck(LIME_UART_PROTOCOL_ECO_CMD_FACTORY_RST, 3);      // 是仪表跳入app
        dut_info.appUpFlag = FALSE;
        STATE_EnterState(STATE_STANDBY);
        break;

    // 超时处理
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

// 数码管、段码屏的串口升级步骤
void STEP_Process_SegUartUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;

    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    // 入口步骤
    case STEP_SEG_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UART协议层初始化
        UART_PROTOCOL_Init();                        // 55升级协议
        UART_PROTOCOL3_Init();                       // KM5S
        DUTCTRL_PowerOnOff(1);                       // dut上电
        break;

    // 等待
    case STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK:
        break;

    // 收到eco请求，发送允许进入工程模式
    case STEP_CM_UART_SEG_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_PROJECT_ALLOW, TRUE); // 进入工程模式
        break;

    // dut准备就绪，发送app擦除指令
    case STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_EAR, TRUE); // 05:擦除app命令
        break;

    // 发送第一包数据
    case STEP_CM_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        dut_info.currentAppSize = 0; // 第一包数据
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize);
        dut_info.currentAppSize++; // 准备下一包数据

        break;

    // 发送app数据包
    case STEP_CM_UART_SEG_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128);
        dut_info.currentAppSize++; // 准备下一包数据
        break;

    // app升级成功
    case STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_UP_OVER, TRUE); // 发送成功
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // 升级config入口
    case STEP_UART_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(9600); // UART协议层初始化
        UART_PROTOCOL_Init();        // 55升级协议
        UART_PROTOCOL3_Init();       // KM5S
        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_UART_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        break;

    // 发送config升级包
    case STEP_UART_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        /*发送协议切换命令*/
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);       // 发送协议切换命令
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR); // 添加设备地址
        UART_PROTOCOL3_TxAddData(0xAB);                          // 0xAB：协议切换
        UART_PROTOCOL3_TxAddData(0);                             // 添加数据长度
        UART_PROTOCOL3_TxAddData(01);
        UART_PROTOCOL3_TxAddFrame(); // 添加检验和与结束符，并添加至发送

        /*发送dut配置信息*/
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60);  // 实际只有54个字节
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);               // 添加命令头
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // 添加设备地址
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0 添加命令字
        UART_PROTOCOL3_TxAddData(54);                                    // 添加数据长度
        // UART_PROTOCOL3_TxAddData(1);//背光亮度1，看现象
        for (i = 0; i < 54; i++)
        {
            UART_PROTOCOL3_TxAddData(configs[i]); // 写入结果
        }
        UART_PROTOCOL3_TxAddFrame(); // 添加检验和与结束符，并添加至发送
        break;

    // 升级config完成
    case STEP_UART_SET_CONFIG_SUCCESS:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止定时器
        UART_DRIVE_InitSelect(115200);           // 设置波特率
        dut_info.configUpFlag = FALSE;           // 标志位置false
        DUTCTRL_PowerOnOff(0);                   // 仪表断电
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE;
        break;

    // 升级项升级完成，检查是否还有需要升级的项
    case STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
        if (dut_info.configUpFlag)
        {
            UART_PROTOCOL_SendCmdParamAck(UART_ECO_CMD_ECO_JUMP_APP, TRUE); // 是仪表跳入app
            // 500ms后（等待仪表开机）开始升级config
            TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                          500,
                          STATE_SwitchStep,
                          STEP_UART_SET_CONFIG_ENTRY,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
        }
        else
        {
            // 没有升级项断电
            STATE_EnterState(STATE_STANDBY);
        }
        break;

    default:
        break;
    }
}

// 开阳uart升级
void STEP_Process_KaiYangUartUpgrade()
{
    uint8 dataPacket[130];
    uint8 i, j, k;
    uint8 configs[60];

    switch (stateCB.step)
    {
    case STEP_KAIYANG_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UART协议层初始化
        UART_PROTOCOL_Init();                        // 55升级协议
        UART_PROTOCOL3_Init();                       // KM5S
        DUTCTRL_PowerOnOff(1);                       // dut上电
        dut_info.dutPowerOnAllow = FALSE;
        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_KAIYANG_UART_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK: // 等待
        break;

    case STEP_KAIYANG_UART_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_PROJECT_APPLY, TRUE); // 0x01,工程模式允
        // 下发文件类型
        UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD); // 添加命令头;//头
        UART_PROTOCOL_TxAddData(0x02);        // 命令，0x02
        UART_PROTOCOL_TxAddData(0x00);        // 添加临时数据长度
        if (dut_info.appUpFlag)
        {
            UART_PROTOCOL_TxAddData(0x03);                  // 0x03:表示是app文件
            UART_PROTOCOL_TxAddData(dut_info.appSize >> 16); // 包数高位
            UART_PROTOCOL_TxAddData(dut_info.appSize >> 8); // 包数中位
            UART_PROTOCOL_TxAddData(dut_info.appSize);      // 包数低位
        }
        else if (dut_info.uiUpFlag)
        {
            UART_PROTOCOL_TxAddData(0x01);                 // 0x01:表示是ui文件
            UART_PROTOCOL_TxAddData(dut_info.uiSize >> 16); // 包数高位
            UART_PROTOCOL_TxAddData(dut_info.uiSize >> 8);  // 包数中位
            UART_PROTOCOL_TxAddData(dut_info.uiSize);      // 包数低位
        }

        UART_PROTOCOL_TxAddFrame(); // 调整帧格式,修改长度和添加校验
        break;

    case STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);

        TIMER_ResetTimer(TIMER_ID_RECONNECTION);// 喂狗

        SPI_FLASH_ReadArray(dataPacket, SPI_FLASH_DUT_APP_ADDEESS + (dut_info.currentAppSize) * 128, 128); // 工具读取128字节

        k = 128;
        // 去掉最后一包数据的0xff
        if (dut_info.currentAppSize == dut_info.appSize - 1)
        {
            for (i = 0; i < 128; i++)
            {
                if (0xff == dataPacket[i])
                {
                    for (j = i; j < 128; j++) // 如果后面全是0xff
                    {
                        if (0xff != dataPacket[j - 1])
                        {
                            break;
                        }
                        if (127 == j) // 后面全是0xff;
                        {
                            k = i - 1;
                            i = 128;
                        }
                    }
                }
            }
        }
        UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);
        UART_PROTOCOL_TxAddData(0x03);                              // 0x03: 数据写入命令
        UART_PROTOCOL_TxAddData(0x00);                              // 临时添加数据长度
        UART_PROTOCOL_TxAddData(dut_info.currentAppSize % 0x100);   // 发送包数

        for (i = 0; i < k; i++)
        {
            UART_PROTOCOL_TxAddData(dataPacket[i]);
        }
        UART_PROTOCOL_TxAddFrame(); // 调整帧格式,修改长度和添加校验
        break;

    // 手动发送下一帧ui数据
    case STEP_KAIYANG_UART_UPGRADE_SEND_APP_AGAIN:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE);
        dut_info.currentAppSize++;
        break;

    case STEP_KAIYANG_UART_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);
        UART_PROTOCOL_TxAddData(0x04);
        UART_PROTOCOL_TxAddData(1); // 数据长度
        UART_PROTOCOL_TxAddData(0x01);
        UART_PROTOCOL_TxAddFrame();
        break;

    // 发送ui数据
    case STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        TIMER_ResetTimer(TIMER_ID_RECONNECTION);// 喂狗
        SPI_FLASH_ReadArray(dataPacket, SPI_FLASH_DUT_UI_ADDEESS + (dut_info.currentUiSize) * 128, 128); // 工具读取128字节
        k = 128;
        if (dut_info.currentUiSize == dut_info.uiSize - 1) // 最后一包去0xff
        {
            for (i = 0; i < 128; i++)
            {
                if (0xff == dataPacket[i])
                {
                    for (j = i; j < 128; j++) // 如果后面全是0xff
                    {
                        if (0xff != dataPacket[j - 1])
                        {
                            break;
                        }
                        if (127 == j) // 后面全是0xff;
                        {
                            k = i + 1;
                            i = 128;
                        }
                    }
                }
            }
        }
        UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);
        UART_PROTOCOL_TxAddData(0x03); // 03: 升级命令
        UART_PROTOCOL_TxAddData(0x00); // 临时添加数据长度
        UART_PROTOCOL_TxAddData(dut_info.currentUiSize % 0x100);
        for (i = 0; i < k; i++)
        {
            UART_PROTOCOL_TxAddData(dataPacket[i]);
        }
        UART_PROTOCOL_TxAddFrame(); // 调整帧格式,修改长度和添加校验
        break;

    // 手动发送下一帧ui数据
    case STEP_KAIYANG_UART_UPGRADE_SEND_UI_AGAIN:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE);
        dut_info.currentUiSize++;
        break;

    // 设置config入口
    case STEP_UART_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_Init();                        // 55升级协议
        UART_PROTOCOL3_Init();                       // KM5S
        UART_DRIVE_InitSelect(9600); // UART协议层初始化
        DUTCTRL_PowerOnOff(0);                       // dut断电
        // 500ms发送一次配置信息
        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_UART_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);

        // 添加5s的超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_KAIYANG_UART_UPGRADE_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送config内容
    case STEP_UART_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK);
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节
        DUTCTRL_PowerOnOff(1);                                          // dut上电
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);              // 添加命令头
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);        // 添加设备地址
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0 添加命令字
        UART_PROTOCOL3_TxAddData(54);                                   // 添加数据长度
        for (i = 0; i < 54; i++)
        {
            UART_PROTOCOL3_TxAddData(configs[i]);
        }
        UART_PROTOCOL3_TxAddFrame(); // 添加检验和与结束符，并添加至发送
        break;

    // config设置成功
    case STEP_UART_SET_CONFIG_SUCCESS:
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止设置config定时器
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // 停止超时定时器
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UART协议层初始化
        UART_PROTOCOL_Init();                        // 55升级协议
        UART_PROTOCOL3_Init();                       // KM5S

        dut_info.configUpFlag = FALSE;           // 标志位置false
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE;
        STATE_EnterState(STATE_STANDBY);
        break;

    // 升级项结束
    case STEP_KAIYANG_UART_UPGRADE_ITEM_FINISH:
        STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK);
        if (dut_info.appUpFlag || dut_info.uiUpFlag)
        {
            DUTCTRL_PowerOnOff(0);                 // 仪表断电
            TIMER_AddTask(TIMER_ID_STATE_CONTROL, // 500ms后进入入口状态
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
            // 没有升级项断电
            STATE_EnterState(STATE_STANDBY);
            TIMER_KillTask(TIMER_ID_RECONNECTION);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        }
        break;


    // 超时处理
    case STEP_KAIYANG_UART_UPGRADE_COMMUNICATION_TIME_OUT:
        if (dut_info.appUpFlag)
        {
            dut_info.appUpFaile = TRUE;
        }
        else if (dut_info.uiUpFlag)
        {
            dut_info.uiUpFaile = TRUE; // ui升级失败
        }
        else if (dut_info.configUpFlag)
        {
            dut_info.configUpFaile = TRUE; // config升级失败
        }

        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        TIMER_KillTask(TIMER_ID_RECONNECTION);
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}


// 华芯微特升级步骤
void STEP_Process_HuaXinUartCbUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i;
    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    // 升级入口
    case STEP_HUAXIN_UART_UPGRADE_ENTRY:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // 设置波特率
        UART_PROTOCOL_Init();                        // 55升级协议
        UART_PROTOCOL3_Init();                       // KM5S
        DUTCTRL_PowerOnOff(1);                       // dut上电
        addTimeOutTimer(5000);                       // 握手超时时间：5S
        break;

    // 等待dut应答
    case STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK:
        // 只做等待，不做操作
        break;

    // 允许dut进入工程模式
    case STEP_HUAXIN_UART_BC_UPGRADE_UP_ALLOW:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_PROJECT_ALLOW, TRUE);
        break;

    // 发送app擦除指令
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_EAR:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_EAR, TRUE);
        addTimeOutTimer(5000); // 擦除app超时时间：5s
        break;

    // 发送第一帧app数据
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        dut_info.currentAppSize = 0;                                                          // 第一包数据
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128); // 发送128字节，数据中不包含地址
        dut_info.currentAppSize++;                                                            // 准备下一包数据
        break;

    // 发送app数据
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_PACKET:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_APP_ADDEESS, dut_info.currentAppSize * 128); // 发送128字节，数据中不包含地址
        dut_info.currentAppSize++;
        break;

    // app升级完成
    case STEP_HUAXIN_UART_BC_UPGRADE_APP_UP_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_UP_OVER, TRUE); // 发送成功
        dut_info.appUpSuccesss = TRUE;
        dut_info.appUpFlag = FALSE;
        break;

    // ui升级流程
    // 发送擦除ui命令
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_UI_DATA_ERASE, TRUE);
        break;

    // 发送第一帧ui数据
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        dut_info.currentUiSize = 0;                                                         // 第一包数据
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_UI_ADDEESS, dut_info.currentUiSize * 128); // 发送128字节，数据中不包含地址
        dut_info.currentUiSize++;                                                           // 准备下一包数据
        break;

    // 发送ui数据
    case STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_PACKET:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        uartProtocol_SendOnePacket(SPI_FLASH_DUT_UI_ADDEESS, dut_info.currentUiSize * 128); // 发送128字节，数据中不包含地址
        dut_info.currentUiSize++;
        break;

    // ui升级完成
    case STEP_HUAXIN_UART_BC_UPGRADE_UI_UP_SUCCESS:
        STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK);
        UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_APP_UP_OVER, TRUE); // 发送成功
        dut_info.uiUpSuccesss = TRUE;
        dut_info.uiUpFlag = FALSE;
        break;

    // 设置config入口
    case STEP_UART_SET_CONFIG_ENTRY:
        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        dut_info.dutPowerOnAllow = TRUE; // 允许开机
        UART_DRIVE_InitSelect(9600);     // UART协议层初始化
        UART_PROTOCOL_Init();            // 55升级协议
        UART_PROTOCOL3_Init();           // KM5S
        DUTCTRL_PowerOnOff(0);           // 仪表断电

        TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                      500,
                      STATE_SwitchStep,
                      STEP_UART_SEND_SET_CONFIG,
                      TIMER_LOOP_FOREVER,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    // 发送config内容
    case STEP_UART_SEND_SET_CONFIG:
        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节
        DUTCTRL_PowerOnOff(1);                                          // dut上电
        /*发送dut配置信息*/
        SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节
        // 添加命令头
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
        // 添加设备地址
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
        // 添加命令字
        UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0
        // 添加数据长度
        UART_PROTOCOL3_TxAddData(54);
        for (i = 0; i < 54; i++)
        {
            UART_PROTOCOL3_TxAddData(configs[i]);
        }
        // 添加检验和与结束符，并添加至发送
        UART_PROTOCOL3_TxAddFrame();
        break;

    // config设置成功
    case STEP_UART_SET_CONFIG_SUCCESS:
        STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止设置config定时器
        UART_DRIVE_InitSelect(115200); // 设置波特率
        dut_info.configUpFlag = FALSE; // 标志位置false
        DUTCTRL_PowerOnOff(0);         // 仪表断电
        dut_info.configUpSuccesss = TRUE;
        dut_info.dutPowerOnAllow = FALSE; // 允许开机
        break;

    // 升级项完成
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
            // 没有升级项断电
            STATE_EnterState(STATE_STANDBY);
        }
        break;

    default:
        break;
    }
}

// 通用彩屏升级步骤
void STEP_Process_CmUartUpgrade()
{
    uint8 configs[100] = {0};
    uint8 i, j;
    
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        // 升级入口
        case STEP_CM_UART_UPGRADE_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // 设置波特率
            UART_PROTOCOL_Init();                        // 55升级协议
            UART_PROTOCOL3_Init();                       // KM5S
            DUTCTRL_PowerOnOff(1);                       // dut上电
            addTimeOutTimer(5000);                    // 握手超时时间：5S
            break;

        // 等待dut应答
        case STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK:
            // 只做等待，不做操作
            break;

        // 允许dut进入工程模式
        case STEP_CM_UART_BC_UPGRADE_UP_ALLOW:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL_SendCmdParamAck(UART_CMD_UP_PROJECT_ALLOW, TRUE);
            break;

        // 发送app擦除指令
        case STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_APP_ERASE_FLASH, TRUE); // 0x24:擦除app命令
            break;

        // 发送app数据
        case STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            uartProtocol_SendOnePacket_Hex(SPI_FLASH_DUT_APP_ADDEESS);
            dut_info.currentAppSize++;
            break;

        // app升级完成
        case STEP_CM_UART_BC_UPGRADE_APP_UP_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_UPDATA_FINISH, TRUE); // 0x2A:升级成功
            dut_info.appUpFlag = FALSE;
            dut_info.appUpSuccesss = TRUE;
            break;

        // ui升级流程
        // 发送擦除ui命令
        case STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_UI_DATA_ERASE, TRUE); // 0x12:擦除ui命令
            break;

        // 发送ui数据
        case STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            uartProtocol_SendOnePacket(SPI_FLASH_DUT_UI_ADDEESS, dut_info.currentUiSize * 128); // 发送128字节，数据中不包含地址
            dut_info.currentUiSize++;
            break;

        // ui升级完成
        case STEP_CM_UART_BC_UPGRADE_UI_UP_SUCCESS:
//            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
//            UART_PROTOCOL_SendCmdParamAck(UART_CMD_DUT_UPDATA_FINISH, TRUE); // 0x2A:升级成功
//            dut_info.uiUpSuccesss = TRUE;
//            dut_info.uiUpFlag = FALSE;
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // 允许开机
            UART_DRIVE_InitSelect(9600);     // UART协议层初始化
            UART_PROTOCOL_Init();            // 55升级协议
            UART_PROTOCOL3_Init();           // KM5S
            DUTCTRL_PowerOnOff(0);           // 仪表断电

            TIMER_AddTask(TIMER_ID_SET_DUT_UI_VER,
                          1000,
                          STATE_SwitchStep,
                          STEP_CM_UART_BC_WRITE_UI_VER,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);         
            break;

        // 写UI版本号
        case STEP_CM_UART_BC_WRITE_UI_VER:
            writeUiFlag = TRUE;
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            DUTCTRL_PowerOnOff(1);                                           // dut上电

            // 将仪表锂电2号切换为KM5S协议
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);               // 添加命令头
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // 添加设备地址
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_PROTOCOL_SWITCCH);   // 0xAB 添加命令字
            UART_PROTOCOL3_TxAddData(3);                                     // 添加数据长度
            UART_PROTOCOL3_TxAddData(1);
            UART_PROTOCOL3_TxAddData(1);
            UART_PROTOCOL3_TxAddData(1);

            // 添加检验和与结束符，并添加至发送
            UART_PROTOCOL3_TxAddFrame();

            // 发送版本信息
            SPI_FLASH_ReadArray(uiVer, SPI_FLASH_UI_VERSION_ADDEESS, 50); // 实际只有54个字节
            
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_VERSION_TYPE_WRITE); // 0xA8
            UART_PROTOCOL3_TxAddData(uiVer[0] + 2);
            UART_PROTOCOL3_TxAddData(8);         // UI版本类型
            UART_PROTOCOL3_TxAddData(uiVer[0]);  //  信息长度

            j = 0;
            for (i = 1; i < uiVer[0] + 1; i++)
            {
                UART_PROTOCOL3_TxAddData(uiVer[i]);
                uiVerifiedBuff[j++] = uiVer[i];
            }
            UART_PROTOCOL3_TxAddFrame();          
            break;
            
         // 读UI版本号
        case STEP_CM_UART_BC_READ_UI_VER:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);    
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_VERSION_TYPE_READ); // 0xA9
            UART_PROTOCOL3_TxAddData(1);    // 数据长度
            UART_PROTOCOL3_TxAddData(8);    // 类型
            UART_PROTOCOL3_TxAddFrame();
            break;
            
        // 写UI版本号成功
        case STEP_CM_UART_BC_WRITE_UI_VER_SUCCESS:
            writeUiFlag = FALSE;
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_ITEM_FINISH);
            dut_info.uiUpSuccesss = TRUE;
            dut_info.uiUpFlag = FALSE;   
            break;  
            
        // 设置config入口
        case STEP_UART_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // 允许开机
            UART_DRIVE_InitSelect(9600);     // UART协议层初始化
            UART_PROTOCOL_Init();            // 55升级协议
            UART_PROTOCOL3_Init();           // KM5S
            DUTCTRL_PowerOnOff(0);           // 仪表断电
            writeUiFlag = FALSE;
            
            TIMER_AddTask(TIMER_ID_SET_DUT_CONFIG,
                          500,
                          STATE_SwitchStep,
                          STEP_UART_PROTOCOL_SWITCCH,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 协议切换
        case STEP_UART_PROTOCOL_SWITCCH:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            DUTCTRL_PowerOnOff(1);                                           // dut上电

            // 将仪表锂电2号切换为KM5S协议
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);               // 添加命令头
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // 添加设备地址
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_PROTOCOL_SWITCCH);   // 0xAB 添加命令字
            UART_PROTOCOL3_TxAddData(3);                                     // 添加数据长度
            UART_PROTOCOL3_TxAddData(1);
            UART_PROTOCOL3_TxAddData(1);
            UART_PROTOCOL3_TxAddData(1);

            // 添加检验和与结束符，并添加至发送
            UART_PROTOCOL3_TxAddFrame();
            break;

        // 发送config内容
        case STEP_UART_SEND_SET_CONFIG:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节
            //          DUTCTRL_PowerOnOff(1);                                          // dut上电
            /*发送dut配置信息*/
            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节

            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);               // 添加命令头
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // 添加设备地址
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0 添加命令字
            UART_PROTOCOL3_TxAddData(54);                                    // 添加数据长度
            for (i = 0; i < 54; i++) // config只有54个字节
            {
                UART_PROTOCOL3_TxAddData(configs[i]);
            }
            // 添加检验和与结束符，并添加至发送
            UART_PROTOCOL3_TxAddFrame();

            // 获取待校验buffer
            j = 0;
            // 处理配置数据，去掉时间，获取待校验buffer
            for (i = 0; i < 54 ; i++)
            {
                if (i == 2)
                {
                    i = i + 5;
                }
                verifiedBuff[j++] = configs[i];
            }
            break;

        // 读取配置参数
        case STEP_UART_READ_CONFIG:
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);                              // 停止设置config定时器
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);                    // 添加命令头
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);              // 添加设备地址
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_READ_CONTROL_PARAM);      // 0xC2添加命令字
            UART_PROTOCOL3_TxAddData(0);                                          // 添加数据长度

            // 添加检验和与结束符，并添加至发送
            UART_PROTOCOL3_TxAddFrame();
            break;
            
        // 写入DCD标志
        case STEP_UART_DCD_FLAG_WRITE:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_DCD_FLAG_WRITE);
            UART_PROTOCOL3_TxAddData(0x06);   // 数据长度
            UART_PROTOCOL3_TxAddData(0x05);   // DCD标志
            UART_PROTOCOL3_TxAddData(0x00);
            UART_PROTOCOL3_TxAddData(0x00);
            UART_PROTOCOL3_TxAddData(0x00);
            UART_PROTOCOL3_TxAddData(0x02);
            UART_PROTOCOL3_TxAddData(0);
            UART_PROTOCOL3_TxAddFrame();               
            break;
        
        // 读取DCD标志
        case STEP_UART_DCD_FLAG_READ:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
        
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);
            UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_DCD_FLAG_READ);
            UART_PROTOCOL3_TxAddData(0);
            UART_PROTOCOL3_TxAddFrame();                           
            break;
            
        // config设置成功
        case STEP_UART_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_ITEM_FINISH);
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止设置config定时器

            UART_DRIVE_InitSelect(115200); // 设置波特率
            dut_info.configUpFlag = FALSE; // 标志位置false
            DUTCTRL_PowerOnOff(0);         // 仪表断电
            dut_info.configUpSuccesss = TRUE;
            dut_info.dutPowerOnAllow = FALSE; // 允许开机
            break;

        // 更新BOOT入口
        case STEP_CM_UART_BC_UPGRADE_BOOT_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // 允许开机
            UART_DRIVE_InitSelect(9600);     // UART协议层初始化
            UART_PROTOCOL3_Init();           // 在该协议接收数据
            DTA_UART_PROTOCOL_Init();
            DUTCTRL_PowerOnOff(0);           // 仪表断电

            TIMER_AddTask(TIMER_ID_UPGRADE_DUT_BOOT,
                          500,
                          STATE_SwitchStep,
                          STEP_CM_UART_BC_UPGRADE_SEND_BOOT_EAR,
                          TIMER_LOOP_FOREVER,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 发送BOOT擦除指令
        case STEP_CM_UART_BC_UPGRADE_SEND_BOOT_EAR:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            DUTCTRL_PowerOnOff(1);
            DTA_UART_PROTOCOL_SendCmdNoResult(DTA_UART_CMD_DUT_BOOT_ERASE_FLASH);
            break;

        // 发送BOOT升级数据
        case STEP_CM_UART_BC_UPGRADE_SEND_BOOT_PACKET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dtaUartProtocol_SendOnePacket(SPI_FLASH_DUT_BOOT_ADDEESS, (dut_info.currentBootSize) * 128);
            dut_info.currentBootSize++;
            break;

        // BOOT升级完成
        case STEP_CM_UART_BC_UPGRADE_BOOT_UP_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            DTA_UART_PROTOCOL_SendCmdParamAck(DTA_UART_CMD_DUT_UPDATA_FINISH, TRUE);
            dut_info.bootUpFlag = FALSE;
            dut_info.bootUpSuccesss = TRUE;
            break;

        // 升级项完成
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
                STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR); // 0x12:擦除ui命令
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
                // 没有升级项断电
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        default:
            break;
    }
}

// 每个状态的入口处理
void STATE_EnterState(uint32 state)
{
    // 让当前的状态成为历史
    stateCB.preState = stateCB.state;

    // 设置新的状态
    stateCB.state = (STATE_E)state;

    // 各状态的入口设定
    switch (state)
    {
        // ■■ 空状态 ■■
        case STATE_NULL: // 不使用
            break;

        case STATE_STANDBY:
            STATE_SwitchStep(STEP_NULL);
            TIMER_KillTask(TIMER_ID_RECONNECTION);          // 结束重连定时器
            TIMER_KillTask(TIMER_ID_RECONNECTION_TIME_OUT); // 结束超时重连定时器
            TIMER_KillTask(TIMER_ID_TIME_OUT_DETECTION);    // 结束握手超时定时器
            TIMER_KillTask(TIMER_ID_SET_DUT_UI_VER); 
            dut_info.dutPowerOnAllow = FALSE;               // 不允许开机，开始接收eco请求
            dut_info.reconnectionFlag = FALSE;              // 清除断电重连标志位
            DUTCTRL_PowerOnOff(0);                          // 仪表断电
            break;

        case STATE_GAOBIAO_CAN_UPGRADE:
            // 高标can升级
            STATE_SwitchStep(STEP_GAOBIAO_CAN_UPGRADE_ENTRY);
            break;

        case STATE_CM_CAN_UPGRADE:
            // 切换通用can升级入口
           STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ENTRY);
            break;

        case STATE_HUAXIN_CAN_UPGRADE:
            // 切换华芯微特can升级入口
            STATE_SwitchStep(STEP_HUAXIN_CAN_UPGRADE_ENTRY);
            break;

        case STATE_SEG_CAN_UPGRADE:
            // 段码屏、数码管can升级
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_ENTRY);
            break;

        case STATE_KAIYANG_CAN_UPGRADE:
            // 开阳can升级入口
            STATE_SwitchStep(STEP_KAIYANG_CAN_UPGRADE_ENTRY);
            break;

        // 通用彩屏串口升级
        case STATE_CM_UART_BC_UPGRADE:
            STATE_SwitchStep(STEP_CM_UART_UPGRADE_ENTRY);
            break;

        // 华芯微特串口升级入口
        case STATE_HUAXIN_UART_BC_UPGRADE:
            STATE_SwitchStep(STEP_HUAXIN_UART_UPGRADE_ENTRY);
            break;

        // 数码管段码屏串口升级入口
        case STATE_CM_UART_SEG_UPGRADE:
            // 切换通用uart升级入口
            STATE_SwitchStep(STEP_SEG_UART_UPGRADE_ENTRY);
            break;

        case STATE_KAIYANG_UART_BC_UPGRADE:
            // 开阳can升级入口
            STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_ENTRY);
            break;

        case STATE_LIME_UART_BC_UPGRADE:
            // LIME uart升级入口
            STATE_SwitchStep(STEP_LIME_UART_UPGRADE_ENTRY);
            break;

        case STATE_SPARROW_CAN_UPGRADE:
            // SPARROW_CAN升级入口
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_ENTRY);
            break;

        case STATE_MEIDI_CAN_UPGRADE:
            // SPARROW_CAN升级入口
            STATE_SwitchStep(STEP_SEG_CAN_UPGRADE_ENTRY);
            break;

        case STATE_IOT_UART_UPGRADE:
            // IOT升级入口
            STATE_SwitchStep(STEP_IOT_UART_UPGRADE_ENTRY);
            break;
            
        case STATE_IOT_CAN_UPGRADE:
            // IOT升级入口
            STATE_SwitchStep(STEP_IOT_CAN_UPGRADE_ENTRY);
            break;
            
        default:
            break;
    }
}

// 每个状态下的过程处理
void STATE_Process(void)
{
    switch (stateCB.state)
    {
        // ■■ 空状态 ■■
        case STATE_NULL:
            break;

        // 待机状态
        case STATE_STANDBY:
            break;

        // 高标can升级
        case STATE_GAOBIAO_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // 迪太升级协议
            CAN_PROTOCOL_Process_GB();      // 高标升级协议
            CAN_TEST_PROTOCOL_Process();    // 测试协议
            STEP_Process_GaobiaoCanUpgrade();
            break;

        // 美的can升级
        case STATE_MEIDI_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // 迪太升级协议
            CAN_MD_PROTOCOL_Process();      // 添加app中的3a解析协议
            CAN_TEST_PROTOCOL_Process();    // 测试协议
            STEP_Process_MeiDiCanUpgrade();
            break;
            
        // sparrow can升级
        case STATE_SPARROW_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // 迪太升级协议
            CAN_PROTOCOL_Process_3A();      // 添加app中的3a解析协议，升级配置
            CAN_TEST_PROTOCOL_Process();    // 测试协议
            STEP_Process_Sparrow_SegCanUpgrade();
            break;

        // 通用彩屏can升级
        case STATE_CM_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // 迪太升级协议
            CAN_PROTOCOL_Process_3A();      // 添加app中的3a解析协议
            CAN_TEST_PROTOCOL_Process();    // 测试协议
            STEP_Process_CmCanUpgrade();
            break;

        // 华芯微特can升级
        case STATE_HUAXIN_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // 迪太升级协议，升级app，ui
            CAN_PROTOCOL_Process_3A();      // 添加app中的3a解析协议，升级配置
            CAN_TEST_PROTOCOL_Process();    // 测试协议
            STEP_Process_HuaXinCanUpgrade();
            break;

        // 数码管、段码屏can升级
        case STATE_SEG_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // 迪太升级协议
            CAN_PROTOCOL_Process_3A();      // 添加app中的3a解析协议
            CAN_TEST_PROTOCOL_Process();    // 测试协议
            STEP_Process_SegCanUpgrade();
            break;

        // 开阳can升级
        case STATE_KAIYANG_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // 迪太升级协议
            CAN_PROTOCOL_Process_3A();      // 添加app中的3a解析协议
            CAN_TEST_PROTOCOL_Process();    // 测试协议
            STEP_Process_KaiYangCanUpgrade();
            break;

        // IOT CAN
        case STATE_IOT_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // 迪太升级协议
            CAN_PROTOCOL_Process_3A();      // 添加app中的3a解析协议
            STEP_Process_IotCanUpgrade();
            break;        
        
        /* 以下为串口升级  */
        // 通用彩屏串口升级
        case STATE_CM_UART_BC_UPGRADE:
            UART_DRIVE_Process();           // UART驱动层过程处理
            UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_CmUartUpgrade();   // 步骤
            break;

        // 华芯维特串口升级
        case STATE_HUAXIN_UART_BC_UPGRADE:
            UART_DRIVE_Process();           // UART驱动层过程处理
            UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_HuaXinUartCbUpgrade();
            break;

        // 数码管、段码屏串口升级
        case STATE_CM_UART_SEG_UPGRADE:
            UART_DRIVE_Process();           // UART驱动层过程处理
            UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_SegUartUpgrade();
            break;

        // 开阳串口升级
        case STATE_KAIYANG_UART_BC_UPGRADE:
            UART_DRIVE_Process();           // UART驱动层过程处理
            UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_KaiYangUartUpgrade();
            break;

        // LIME串口升级
        case STATE_LIME_UART_BC_UPGRADE:
            UART_DRIVE_Process();           // UART驱动层过程处理
            LIME_UART_PROTOCOL_Process();   // UART协议层过程处理
            STEP_Process_LiMeUartUpgrade();
            break;

        // IOT串口升级
        case STATE_IOT_UART_UPGRADE:
            UART_DRIVE_Process();           // UART驱动层过程处理
            UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_IotUartUpgrade();
            break;
            
        default:
            break;
    }
}
