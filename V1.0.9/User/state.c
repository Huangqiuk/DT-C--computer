#include "common.h"
#include "system.h"
#include "timer.h"
#include "state.h"
#include "DutUartDrive.h"
#include "CanProtocolUpDT.h"
#include "DutUartProtocol.h"
#include "CanProtocol_3A.h"
#include "StsProtocol.h"
#include "spiflash.h"
#include "timeOut.h"
#include "AvoPin.h"
#include "AvoProtocol.h"
#include "param.h"
#include "PowerProtocol.h"
#include "DutInfo.h"
#include "uartProtocol3.h"
#include "uartProtocol.h"
#include "CanProtocolTest.h"
#include "UartprotocolXM.h"
#include "pwm.h"

STATE_CB stateCB;
BOOL resetFlag;
BOOL configResetFlag;
BOOL appResetFlag;

// vlk测试显示
char vlkMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "VLK Testing",
    "AVO Measurement",
    "VLK Abnormal",
    "VLK Test Pass",
};

// vlk测试显示
char gndMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "GND Testing",
    "AVO Measurement",
    "THRO Ab",
    "THRO Test Pass",
};

// 异常测试显示
char excepMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "Test Timeout",
};

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

// GND检测 先测油门GND，再测刹把GND
void  All_GND_Test(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_GND_TEST_WAIT_RESPONSE: // 等待万用表回应
            break;

        case STEP_GND_TEST_ENTRY: // 入口步骤
            gnd_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "GND");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          dut_info.timeout_Period * 1000,
                          STATE_SwitchStep,
                          STEP_GND_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);

            if (dut_info.gnd == GND_TYPE_THROTTLE_BRAKE)
            {
                STATE_SwitchStep(STEP_GND_TEST_THROTTLE_GND);
            }
            if (dut_info.gnd == GND_TYPE_BRAKE)
            {
                STATE_SwitchStep(STEP_GND_TEST_BRAKE_GND);
            }
            if (dut_info.gnd == GND_TYPE_THROTTLE)
            {
                STATE_SwitchStep(STEP_GND_TEST_THROTTLE_GND);
            }
            if (dut_info.gnd == GND_TYPE_DERAILLEUR)
            {
                STATE_SwitchStep(STEP_GND_TEST_DERAILLEUR_GND);
            }
            if (dut_info.gnd == GND_TYPE_BRAKE_DERAILLEUR)
            {
                STATE_SwitchStep(STEP_GND_TEST_BRAKE_GND);
            }            
            break;

        case STEP_GND_TEST_THROTTLE_GND: // 测油门阻抗
            STATE_SwitchStep(STEP_GND_TEST_WAIT_RESPONSE);

            // 打开测试引脚
            THROTTLE_GND_TEST_EN_ON();

            // 通知万用表测阻抗
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            Clear_All_Lines();
            Display_Centered(0, "GND");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE:     // 比对GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);      // 喂狗

            // 油门GND正常
            if ((50 > ohm.INT) && (dut_info.gnd == GND_TYPE_THROTTLE_BRAKE))
            {
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "THROTTLE");
                Display_Centered(2, "Normal");

                STATE_SwitchStep(STEP_GND_TEST_BRAKE_GND);
            }
            else if ((50 > ohm.INT) && (dut_info.gnd == GND_TYPE_THROTTLE))
            {
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "THROTTLE");
                Display_Centered(2, "Normal");
                STATE_SwitchStep(STEP_GND_TEST_COMPLETE);
            }
            else if (ohm.INT)
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "THROTTLE");
                Display_Centered(2, "Abnormal");
                Display_YX_Format(3, 0, "%d", ohm.INT);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_GND_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        case STEP_GND_TEST_BRAKE_GND: // 测刹把阻抗
            STATE_SwitchStep(STEP_GND_TEST_WAIT_RESPONSE);

            // 打开测试引脚
            BRAKE_GND_TEST_EN_ON();

            // 通知万用表测阻抗
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            Clear_All_Lines();
            Display_Centered(0, "GND");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_GND_TEST_CHECK_BRAKE_GND_VALUE:     // 比对GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);  // 喂狗

            // 刹把GND正常
            if ((50 > ohm.INT) && (dut_info.gnd != GND_TYPE_BRAKE_DERAILLEUR))
            {
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "BRAKE");
                Display_Centered(2, "GND Normal");
                STATE_SwitchStep(STEP_GND_TEST_COMPLETE);
            }
            
            // 刹把GND正常
            if ((50 > ohm.INT) && (dut_info.gnd == GND_TYPE_BRAKE_DERAILLEUR))
            {
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "BRAKE");
                Display_Centered(2, "GND Normal");
                STATE_SwitchStep(STEP_GND_TEST_DERAILLEUR_GND);
            } 

            // 不正常            
            if(50 < ohm.INT)
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "BRAKE");
                Display_Centered(2, "Abnormal");
                Display_YX_Format(3, 0, "%d", ohm.INT);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_GND_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        case STEP_GND_TEST_DERAILLEUR_GND:      // 测电子变速阻抗
            STATE_SwitchStep(STEP_GND_TEST_WAIT_RESPONSE);

            // 打开测试引脚
            DERAILLEUR_GND_TEST_EN_ON();

            // 通知万用表测阻抗
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            Clear_All_Lines();
            Display_Centered(0, "GND");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_GND_TEST_CHECK_DERAILLEUR_GND_VALUE:     // 比对GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);  // 喂狗

            // 刹把GND正常
            if (50 > ohm.INT)
            {
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "DERAILLEUR");
                Display_Centered(2, "GND Normal");
                STATE_SwitchStep(STEP_GND_TEST_COMPLETE);
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "DERAILLEUR");
                Display_Centered(2, "Abnormal");
                Display_YX_Format(3, 0, "%d", ohm.INT);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_GND_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;
            
        case STEP_GND_TEST_COMPLETE: // GND测试完成步骤，关闭测试引脚，然后上报结果
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
    #if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "GND Test Pass");
    #endif
            AVO_PIN_Reset();

            // 上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_GND_TEST, TRUE);
            STATE_EnterState(STATE_STANDBY);
            break;

        case STEP_GND_TEST_COMMUNICATION_TIME_OUT: // 通讯超时
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
    #if DEBUG_ENABLE
            // LCD显示
            Vertical_Scrolling_Display(excepMsgBuff, 4, 0);
    #endif
            AVO_PIN_Reset();
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_GND_TEST, FALSE);
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// UART
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void  UART_Test(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_UART_TEST_WAIT_RESPONSE);

            Clear_All_Lines();
            Display_Centered(0, "UART");
            Display_Centered(1, "Testing");

            // 打开测试引脚
            DUT_TO_DTA_OFF_ON();
            UART_TEST_EN_ON();

            // 通知万用表测电压
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          3000,
                          STATE_SwitchStep,
                          STEP_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_UART_TEST_WAIT_RESPONSE: // 等待万用表回应
            break;

        case STEP_UART_TEST_CHECK_VOLTAGE_VALUE:     // 比对VCC
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);  // 喂狗

            if (measure.result > 2000)
            {
                STATE_SwitchStep(STEP_UART_TEST_COMPLETE);
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "UART");
                Display_Centered(1, "Abnormal");
                Display_YX_Format(2, 0, "%f", measure.result);
    #endif
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UART_TEST, FALSE);
            }
            break;

        case STEP_UART_TEST_COMPLETE: // UART测试完成步骤，关闭测试引脚，然后上报结果
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
    #if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "UART");
            Display_Centered(1, "Test Pass");
    #endif
            AVO_PIN_Reset();
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UART_TEST, TRUE);
            STATE_EnterState(STATE_STANDBY);
            break;

        case STEP_UART_TEST_COMMUNICATION_TIME_OUT: // 通讯超时
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
    #if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "UART Test Timeout");
    #endif
            AVO_PIN_Reset();
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_UART_TEST, FALSE);

            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}


// 大灯测试
void STEP_Process_HeadLightUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_HEADLIGHT_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "HEADLIGHT");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE: // 等待回应步骤
            break;

        case STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT: // 通知DUT打开大灯步骤
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_HEADLIGHT_CONTROL, 1);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_HEADLIGHT_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_HEADLIGHT_UART_TEST_OPEN:      // 仪表打开大灯成功
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 打开VCC测试引脚
            LED_VCC_TEST_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "HEADLIGHT");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_HEADLIGHT_UART_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 比对万用表返回的数值
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "HEADLIGHT");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // 打开GND测试引脚
                LED_VCC_TEST_EN_ON();
                LED_VCC_EN_ON();

                // 通知万用表测VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "HEADLIGHT");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // 比对失败
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "VCC Abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_HEADLIGHT_UART_TEST_CHECK_GND_VALUE: // 比对GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 小于3V则GND导通测试通过
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // 关闭大灯
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_HEADLIGHT_CONTROL, 0);
                Clear_All_Lines();
                Display_Centered(0, "HEADLIGHT");
                Display_Centered(1, "GND");
                Display_Centered(2, "Test Pass");
                STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);       
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "HEADLIGHT");
                Display_Centered(1, "GND Abnormal");
                Display_YX_Format(2, 0, "%f", measure.result);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_SwitchStep(STATE_STANDBY);

            }
            break;

        case STEP_HEADLIGHT_UART_TEST_COMPLETE: // 测试完成
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "HEADLIGHT");
            Display_Centered(1, "Test Pass");

            // 上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // 超时处理
        case STEP_HEADLIGHT_UART_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "HEADLIGHT");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 近光灯测试
void STEP_Process_LbeamUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_LBEAM_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_LBEAM_UART_TEST_WAIT_RESPONSE: // 等待回应步骤
            break;

        case STEP_LBEAM_UART_TEST_NOTIFY_DUT: // 通知DUT打开近光灯步骤
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 2, 1);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_LBEAM_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_LBEAM_UART_TEST_OPEN:      // 仪表打开近光灯成功
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 打开VCC测试引脚
            LED_LBEAM_TEST_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_LBEAM_UART_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 比对万用表返回的数值
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "LBEAM");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // 打开GND测试引脚
                LED_LBEAM_TEST_EN_ON();
                LED_LBEAM_EN_ON();

                // 通知万用表测VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "LBEAM");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // 比对失败
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "VCC Abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_LBEAM_UART_TEST_CHECK_GND_VALUE: // 比对GND
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 小于3V则GND导通测试通过
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // 关闭近光灯
                DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 2, 0);
                Clear_All_Lines();
                Display_Centered(0, "LBEAM");
                Display_Centered(1, "GND");
                Display_Centered(2, "Test Pass");
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "LBEAM");
                Display_Centered(1, "GND Abnormal");
                Display_YX_Format(2, 0, "%f", measure.result);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_LBEAM_UART_TEST_COMPLETE: // 测试完成
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "Test Pass");

            // 上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // 超时处理
        case STEP_LBEAM_UART_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 远光灯测试
void STEP_Process_HbeamUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_HBEAM_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_HBEAM_UART_TEST_WAIT_RESPONSE: // 等待回应步骤
            break;

        case STEP_HBEAM_UART_TEST_NOTIFY_DUT: // 通知DUT打开远光灯步骤
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 3, 1);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_HBEAM_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_HBEAM_UART_TEST_OPEN:      // 仪表打开远光灯成功
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 打开VCC测试引脚
            LED_HBEAM_TEST_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_HBEAM_UART_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 比对万用表返回的数值
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "HBEAM");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // 打开GND测试引脚
                LED_HBEAM_TEST_EN_ON();
                LED_HBEAM_EN_ON();

                // 通知万用表测VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "HBEAM");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // 比对失败
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "VCC Abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_HBEAM_UART_TEST_CHECK_GND_VALUE: // 比对GND
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 小于3V则GND导通测试通过
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // 关闭远光灯
                DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 3, 0);
                Clear_All_Lines();
                Display_Centered(0, "HBEAM");
                Display_Centered(1, "GND");
                Display_Centered(2, "Test Pass");
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "HBEAM");
                Display_Centered(1, "GND Abnormal");
                Display_YX_Format(2, 0, "%f", measure.result);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_HBEAM_UART_TEST_COMPLETE: // 测试完成
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "Test Pass");

            // 上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // 超时处理
        case STEP_HBEAM_UART_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 左转向灯测试
void STEP_Process_Left_TurnSignalUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "LEFT TURN SIGNAL");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE: // 等待回应步骤
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT: // 通知DUT打开远光灯步骤
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 0, 1);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_LEFT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN:      // 仪表打开远光灯成功
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 打开VCC测试引脚
            LEFT_VCC_TEST_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "LEFT TURN SIGNAL");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 比对万用表返回的数值
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "LEFT TURN SIGNAL");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // 打开GND测试引脚
                LEFT_VCC_TEST_EN_ON();
                LEFT_VCC_EN_ON();

                // 通知万用表测VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "LEFT TURN SIGNAL");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // 比对失败
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "VCC Abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE: // 比对GND
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 小于3V则GND导通测试通过
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // 关闭远光灯
                DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 0, 0);
                Clear_All_Lines();
                Display_Centered(0, "LEFT TURN SIGNAL");
                Display_Centered(1, "GND");
                Display_Centered(2, "Test Pass");
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "LEFT TURN SIGNAL");
                Display_Centered(1, "GND Abnormal");
                Display_YX_Format(2, 0, "%f", measure.result);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE: // 测试完成
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "LEFT TURN SIGNAL");
            Display_Centered(1, "Test Pass");

            // 上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // 超时处理
        case STEP_LEFT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "LEFT TURN SIGNAL");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 右转向灯测试
void STEP_Process_Right_TurnSignalUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "RIGHT TURN SIGNAL");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE: // 等待回应步骤
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT: // 通知DUT打开远光灯步骤
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 1, 1);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN:      // 仪表打开远光灯成功
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 打开VCC测试引脚
            LEFT_VCC_TEST_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "RIGHT TURN SIGNAL");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 比对万用表返回的数值
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "RIGHT TURN SIGNAL");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // 打开GND测试引脚
                RIGHT_VCC_TEST_EN_ON();
                RIGHT_VCC_EN_ON();

                // 通知万用表测VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "LEFT TURN SIGNAL");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // 比对失败
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "VCC Abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE: // 比对GND
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 小于3V则GND导通测试通过
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // 关闭远光灯
                DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 1, 0);
                Clear_All_Lines();
                Display_Centered(0, "RIGHT TURN SIGNAL");
                Display_Centered(1, "GND");
                Display_Centered(2, "Test Pass");
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "RIGHT TURN SIGNAL");
                Display_Centered(1, "GND Abnormal");
                Display_YX_Format(2, 0, "%f", measure.result);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE: // 测试完成
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "RIGHT TURN SIGNAL");
            Display_Centered(1, "Test Pass");

            // 上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // 超时处理
        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "RIGHT TURN SIGNAL");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 油门测试
void STEP_Process_ThrottleUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_THROTTLE_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();

            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_THROTTLE_UART_TEST_WAIT_RESPONSE: // 等待仪表或万用表回应
            break;

        case STEP_THROTTLE_UART_TEST_VCC: // 通知万用表测VCC
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);

            // 打开VCC测试引脚
            THROTTLE_VCC_TEST_EN(TRUE);

            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_THROTTLE_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VCC
        case STEP_THROTTLE_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // 调整DAC输出,单位mv
                DAC0_output(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // 获取油门AD值
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 1);
            }
            // VCC不正常，则上报
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第一次
        case STEP_THROTTLE_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC1数值检查逻辑
            if ((dut_info.throttleAd > FIRST_DAC - ERROR_DAC) && (dut_info.throttleAd < FIRST_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                DAC0_output(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // 获取油门AD值
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 1);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "SIG");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第二次
        case STEP_THROTTLE_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.throttleAd > SECOND_DAC - ERROR_DAC) && (dut_info.throttleAd < SECOND_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                DAC0_output(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // 获取油门AD值
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 1);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "SIG");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第三次
        case STEP_THROTTLE_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.throttleAd > THIRD_DAC - ERROR_DAC) && (dut_info.throttleAd < THIRD_DAC + ERROR_DAC))
            {
                // 3次都正常则测试通过
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Third");
                STATE_SwitchStep(STEP_THROTTLE_UART_TEST_COMPLETE);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
        case STEP_THROTTLE_UART_TEST_COMPLETE:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            break;

        // 通信超时
        case STEP_THROTTLE_UART_TEST_COMMUNICATION_TIME_OUT:
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 刹把测试
void STEP_Process_BrakeUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_BRAKE_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_BRAKE_UART_TEST_WAIT_RESPONSE: // 等待仪表或万用表回应
            break;

        case STEP_BRAKE_UART_TEST_VCC: // 通知万用表测VCC
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);

            // 打开VCC测试引脚
            BRAKE_VCC_TEST_EN_ON();

            // 通知万用表测电压
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_BRAKE_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VCC
        case STEP_BRAKE_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // 调整DAC输出,单位mv
                DAC1_output(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // 获取刹把AD值
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 2);
            }
            // VCC不正常，则上报
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第一次
        case STEP_BRAKE_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC1数值检查逻辑
            if ((dut_info.brakeAd > FIRST_DAC - ERROR_DAC) && (dut_info.brakeAd < FIRST_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                DAC1_output(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // 获取刹把AD值
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 2);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "SIG1");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第二次
        case STEP_BRAKE_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.brakeAd > SECOND_DAC - ERROR_DAC) && (dut_info.brakeAd < SECOND_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                DAC1_output(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // 获取刹把AD值
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 2);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "SIG2");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第三次
        case STEP_BRAKE_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.brakeAd > THIRD_DAC - ERROR_DAC) && (dut_info.brakeAd < THIRD_DAC + ERROR_DAC))
            {
                // 3次都正常则测试通过
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Third");
                STATE_SwitchStep(STEP_BRAKE_UART_TEST_COMPLETE);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "SIG3");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 刹把测试完成步骤，关闭DAC输出调整为0，然后上报结果
        case STEP_BRAKE_UART_TEST_COMPLETE:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            break;

        // 通信超时
        case STEP_BRAKE_UART_TEST_COMMUNICATION_TIME_OUT:
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 电子变速测试
void STEP_Process_DerailleurUartTest(void)   
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_DERAILLEUR_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE: // 等待仪表或万用表回应
            break;

        case STEP_DERAILLEUR_UART_TEST_VCC: // 通知万用表测VCC
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE);

            // 打开VCC测试引脚
            DERAILLEUR_VCC_TEST_EN_ON();

            // 通知万用表测电压
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_DERAILLEUR_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VCC
        case STEP_DERAILLEUR_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // 调整DAC输出,单位mv
                PWM_OUT1_PWM(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // 获取刹把AD值
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 3);
            }
            // VCC不正常，则上报
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第一次
        case STEP_DERAILLEUR_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC1数值检查逻辑
            if ((dut_info.derailleurAd > FIRST_DAC - ERROR_DAC) && (dut_info.derailleurAd < FIRST_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                PWM_OUT1_PWM(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // 获取刹把AD值
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 3);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "SIG1");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第二次
        case STEP_DERAILLEUR_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.derailleurAd > SECOND_DAC - ERROR_DAC) && (dut_info.derailleurAd < SECOND_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                PWM_OUT1_PWM(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // 获取刹把AD值
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 3);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "SIG2");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第三次
        case STEP_DERAILLEUR_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.derailleurAd > THIRD_DAC - ERROR_DAC) && (dut_info.derailleurAd < THIRD_DAC + ERROR_DAC))
            {
                // 3次都正常则测试通过
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Third");
                STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_COMPLETE);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "SIG3");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 刹把测试完成步骤，关闭DAC输出调整为0，然后上报结果
        case STEP_DERAILLEUR_UART_TEST_COMPLETE:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            break;

        // 通信超时
        case STEP_DERAILLEUR_UART_TEST_COMMUNICATION_TIME_OUT:
            PWM_OUT1_PWM(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// VLK测试
void STEP_Process_VlkUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_VLK_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_VLK_UART_TEST_VLK);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();

            // LCD显示
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 0);

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        // 等待万用表回应
        case STEP_VLK_UART_TEST_WAIT_RESPONSE:
            break;

        // 通知万用表测VLK
        case STEP_VLK_UART_TEST_VLK:
            STATE_SwitchStep(STEP_VLK_UART_TEST_WAIT_RESPONSE);

            // 使能VLK测试引脚
            DUT_VLK_TEST_EN_ON();

            // 通知万用表测VLK
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

            // LCD显示
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 1);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          3000,
                          STATE_SwitchStep,
                          STEP_VLK_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VLK值
        case STEP_VLK_UART_TEST_CHECK_VOLTAGE_VALUE:

            STATE_SwitchStep(STEP_VLK_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                STATE_SwitchStep(STEP_VLK_UART_TEST_COMPLETE);
            }
            // VCC不正常，则上报
            else
            {
                // LCD显示
                Vertical_Scrolling_Display(vlkMsgBuff, 4, 2);
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        // 测试完成，然后上报结果
        case STEP_VLK_UART_TEST_COMPLETE:
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);

            // LCD显示
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 3);
            break;

        case STEP_VLK_UART_TEST_COMMUNICATION_TIME_OUT: // 通信超时
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();

            // LCD显示
            Vertical_Scrolling_Display(excepMsgBuff, 4, 2);
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 电压校准
void STEP_Process_CalibrationUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_CALIBRATION_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_NOTIFY_POWER_BOARD);
            cali_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Testing");
            break;

        case STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE: // 等待仪表/电源板回应
            break;

        case STEP_CALIBRATION_UART_TEST_NOTIFY_POWER_BOARD: // 通知电源板调整仪表供电电压
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[0]);            // 通知电源板调整DUT供电电压
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(0, "Voltage");
            Display_Centered(1, "Adjusting");

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          5000,
                          STATE_SwitchStep,
                          STEP_CALIBRATION_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 一级校准
        case STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION: // 调整完成通知仪表进行电压校准
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Once");
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[0]);
            break;

        case STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_POWER_BOARD: // 通知电源板调整仪表供电电压
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[1]);            // 通知电源板调整DUT供电电压
            break;

        // 二级校准
        case STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION: // 调整完成通知仪表进行电压校准
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Seconnd");
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[1]);
            break;

        case STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_POWER_BOARD: // 通知电源板调整仪表供电电压
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[2]);            // 通知电源板调整DUT供电电压
            break;

        // 三级校准
        case STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION: // 调整完成通知仪表进行电压校准
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Third");
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[2]);
            break;

        // 还原仪表供电电压为24V
        case STEP_CALIBRATION_UART_TEST_RESTORE_VOLTAGE_CALIBRATION:
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Restore Voltage");
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, 0x18);
            break;

        // 校准完成
        case STEP_CALIBRATION_UART_TEST_COMPLETE: // 测试完成，然后上报结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, TRUE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            cali_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
            break;

        // 通信超时
        case STEP_CALIBRATION_UART_TEST_COMMUNICATION_TIME_OUT:
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            cali_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// CAN测试步骤
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 大灯测试
void STEP_Process_HeadLightCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        // 入口步骤
        case STEP_HEADLIGHT_CAN_TEST_ENTRY:
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_NOTIFY_DUT);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 
            //        CAN_PROTOCOL1_Init();
            //        can_light_cnt = 0;

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE: // 等待回应步骤
            break;

        case STEP_HEADLIGHT_CAN_TEST_NOTIFY_DUT:   // 通知DUT打开大灯步骤
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE);
            //        CAN_PROTOCOL1_SendCmdWithResult(CAN_PROTOCOL1_CMD_HEADLIGHT_CONTROL, 1);
            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_HEADLIGHT_CAN_TEST_OPEN:      // 收到仪表打开大灯信号
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 打开GND测试引脚
            //        LED_GND_TEST_EN_ON();

            // 通知万用表测GND阻抗值
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            break;

        case STEP_HEADLIGHT_CAN_TEST_CHECK_GND_VALUE: // 检查接地值步骤
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // GND导通测试通过，则测VCC
            if (measure.result == 0)
            {
                // 打开VCC测试引脚
                LED_VCC_TEST_EN_ON();
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "Headlights GND abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Delayms(1000);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            }
            break;

        case STEP_HEADLIGHT_CAN_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                //            CURRENT1_TEST_EN_ON();
                //            LED_VCC_CURRENT_TEST_EN_ON();
                //            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCMA, 0);
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "Headlights VCC abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
                Delayms(1000);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            }
            break;

        case STEP_HEADLIGHT_CAN_TEST_CHECK_CURRENT_VALUE: // 检查电流值步骤
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 大灯测试,最后测电流
            if ((measure.result > dut_info.currentMin) && (measure.result < dut_info.currentMax))
            {
                // 测试通过关闭大灯
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_HEADLIGHT_CONTROL, 0);
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "Headlights VCC abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
                Delayms(1000);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            }
            break;

        case STEP_HEADLIGHT_CAN_TEST_COMPLETE: // 测试完成步骤
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);

            // 关闭大灯并且上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            break;

        // 超时处理
        case STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 近光灯测试
void STEP_Process_LbeamCanTest(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    case STEP_LBEAM_CAN_TEST_ENTRY: // 入口步骤
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_NOTIFY_DUT);
        CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 
        //        CAN_PROTOCOL1_Init();
        light_cnt = 0;
        headlight_cnt = 0;
        Clear_All_Lines();
        Display_Centered(0, "LBEAM");
        Display_Centered(1, "Testing");

        // 测试引脚复位
        AVO_PIN_Reset();
        break;

    case STEP_LBEAM_CAN_TEST_WAIT_RESPONSE: // 等待回应步骤
        break;

    case STEP_LBEAM_CAN_TEST_NOTIFY_DUT: // 通知DUT打开近光灯步骤
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_WAIT_RESPONSE);
        DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 2, 1);

        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_LBEAM_CAN_TEST_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_LBEAM_CAN_TEST_OPEN:      // 仪表打开近光灯成功
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // 打开VCC测试引脚
        LED_LBEAM_TEST_EN_ON();

        // 通知万用表测VCC
        AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
        Clear_All_Lines();
        Display_Centered(0, "LBEAM");
        Display_Centered(1, "AVO METER");
        Display_Centered(2, "Measurement");
        break;

    case STEP_LBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // 比对万用表返回的数值
        if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
        {
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "VCC");
            Display_Centered(2, "Test Pass");

            // 打开GND测试引脚
            LED_LBEAM_TEST_EN_ON();
            LED_LBEAM_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
        }
        // 比对失败
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "VCC Abnormal");
            Display_YX_Format(1, 0, "%f", measure.result);
            Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
            Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        }
        break;

    case STEP_LBEAM_CAN_TEST_CHECK_GND_VALUE: // 比对GND
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // 小于3V则GND导通测试通过
        if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
        {
            // 关闭近光灯
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 2, 0);
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "GND");
            Display_Centered(2, "Test Pass");
        }
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "GND Abnormal");
            Display_YX_Format(2, 0, "%f", measure.result);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        }
        break;

    case STEP_LBEAM_CAN_TEST_COMPLETE: // 测试完成
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        Clear_All_Lines();
        Display_Centered(0, "LBEAM");
        Display_Centered(1, "Test Pass");

        // 上报测试成功
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
        AVO_PIN_Reset();
        STATE_EnterState(STATE_STANDBY);
        break;

    // 超时处理
    case STEP_LBEAM_CAN_TEST_COMMUNICATION_TIME_OUT:
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
        AVO_PIN_Reset();
        Clear_All_Lines();
        Display_Centered(0, "LBEAM");
        Display_Centered(1, "Test Timeout");
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}

// 远光灯测试
void STEP_Process_HbeamCanTest(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    case STEP_HBEAM_CAN_TEST_ENTRY: // 入口步骤
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_NOTIFY_DUT);
        IAP_CTRL_CAN_Init(dut_info.can_baudRate);
        CAN_PROTOCOL1_Init();
        light_cnt = 0;
        headlight_cnt = 0;
        Clear_All_Lines();
        Display_Centered(0, "HBEAM");
        Display_Centered(1, "Testing");

        // 测试引脚复位
        AVO_PIN_Reset();
        break;

    case STEP_HBEAM_CAN_TEST_WAIT_RESPONSE: // 等待回应步骤
        break;

    case STEP_HBEAM_CAN_TEST_NOTIFY_DUT: // 通知DUT打开远光灯步骤
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_WAIT_RESPONSE);
        DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 3, 1);

        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_HBEAM_CAN_TEST_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_HBEAM_CAN_TEST_OPEN:      // 仪表打开远光灯成功
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // 打开VCC测试引脚
        LED_HBEAM_TEST_EN_ON();

        // 通知万用表测VCC
        AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
        Clear_All_Lines();
        Display_Centered(0, "HBEAM");
        Display_Centered(1, "AVO METER");
        Display_Centered(2, "Measurement");
        break;

    case STEP_HBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // 比对万用表返回的数值
        if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
        {
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "VCC");
            Display_Centered(2, "Test Pass");

            // 打开GND测试引脚
            LED_HBEAM_TEST_EN_ON();
            LED_HBEAM_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
        }
        // 比对失败
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "VCC Abnormal");
            Display_YX_Format(1, 0, "%f", measure.result);
            Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
            Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        }
        break;

    case STEP_HBEAM_CAN_TEST_CHECK_GND_VALUE: // 比对GND
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // 小于3V则GND导通测试通过
        if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
        {
            // 关闭远光灯
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 3, 0);
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "GND");
            Display_Centered(2, "Test Pass");
        }
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "GND Abnormal");
            Display_YX_Format(2, 0, "%f", measure.result);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        }
        break;

    case STEP_HBEAM_CAN_TEST_COMPLETE: // 测试完成
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        Clear_All_Lines();
        Display_Centered(0, "HBEAM");
        Display_Centered(1, "Test Pass");

        // 上报测试成功
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
        AVO_PIN_Reset();
        STATE_EnterState(STATE_STANDBY);
        break;

    // 超时处理
    case STEP_HBEAM_CAN_TEST_COMMUNICATION_TIME_OUT:
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
        AVO_PIN_Reset();
        Clear_All_Lines();
        Display_Centered(0, "HBEAM");
        Display_Centered(1, "Test Timeout");
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}

// 左转向灯测试
void STEP_Process_Left_TurnSignalCanTest(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_ENTRY: // 入口步骤
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT);
        IAP_CTRL_CAN_Init(dut_info.can_baudRate);
        CAN_PROTOCOL1_Init();
        //        can_left_light_cnt = 0;

        // 测试引脚复位
        AVO_PIN_Reset();
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE: // 等待回应步骤
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT:    // 通知DUT打开左转向灯
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        //        CAN_PROTOCOL1_SendCmdWith_TwoResult(CAN_PROTOCOL1_CMD_TURN_SIGNAL_CONTROL, 0, 1);

        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_OPEN:           // 打开左转向灯成功
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // 打开GND测试引脚
        //        TURN_GND_TEST_EN_ON();

        //        // 通知万用表测GND阻抗值
        //        AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_GND_VALUE: // 检查GND
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // GND导通测试通过，则测VCC
        if (measure.result == 0)
        {
            // 打开VCC测试引脚
            LEFT_VCC_TEST_EN_ON();
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
        }
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "Left TurnSignal GND abnormal");
            Display_YX_Format(1, 0, "%f", measure.result);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
        }
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
        {
//            CURRENT2_TEST_EN_ON();
//            LEFT_VCC_CURRENT_TEST_EN_ON();
//            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCMA, 0);
        }
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "Left TurnSignal VCC abnormal");
            Display_YX_Format(1, 0, "%f", measure.result);
            Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
            Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
        }
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_CURRENT_VALUE: // 检查电流值步骤
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // 左转向灯测试,最后测电流
        if ((measure.result > dut_info.currentMin) && (measure.result < dut_info.currentMax))
        {
            // 测试通过关闭左转向灯
            //            CAN_PROTOCOL1_SendCmdWith_TwoResult(CAN_PROTOCOL1_CMD_TURN_SIGNAL_CONTROL, 0, 0);
        }
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "Left TurnSignal CURRENT abnormal");
            Display_YX_Format(1, 0, "%f", measure.result);
            Display_YX_Format(2, 0, "%d", dut_info.currentMin);
            Display_YX_Format(3, 0, "%d", dut_info.currentMax);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
        }
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_COMPLETE: // 测试完成步骤
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);

        // 关闭左转向灯并且上报测试成功
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
        AVO_PIN_Reset();
        break;

    // 超时处理
    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_COMMUNICATION_TIME_OUT:
#if DEBUG_ENABLE
        Clear_All_Lines();
        Display_Centered(0, "Left TurnSignal Timeout");
#endif
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        AVO_PIN_Reset();
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}

// 右转向灯测试
void STEP_Process_Right_TurnSignalCanTest(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // 空步骤
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_ENTRY: // 入口步骤
        STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_NOTIFY_DUT);
        IAP_CTRL_CAN_Init(dut_info.can_baudRate);
        CAN_PROTOCOL1_Init();
        //        can_right_light_cnt = 0;
        Display_Centered(0, "Testing");

        // 测试引脚复位
        AVO_PIN_Reset();
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE: // 等待回应步骤
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT:    // 通知DUT打开右转向灯
        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        //        CAN_PROTOCOL1_SendCmdWith_TwoResult(CAN_PROTOCOL1_CMD_TURN_SIGNAL_CONTROL, 1, 1);

        // 添加超时
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_OPEN:         // 右转向灯打开成功
        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        //        // 打开GND测试引脚
        //        TURN_GND_TEST_EN_ON();

        //        // 通知万用表测GND阻抗值
        //        AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_GND_VALUE: // 检查接地值步骤
        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // GND导通测试通过，则测VCC
        if (measure.result == 0)
        {
            // 打开VCC测试引脚
            RIGHT_VCC_TEST_EN_ON();
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
        }
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "Right TurnSignal GND abnormal");
            Display_YX_Format(1, 0, "%f", measure.result);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
        }
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
        if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
        {
            //            CURRENT2_TEST_EN_ON();
            //            RIGHT_VCC_CURRENT_TEST_EN_ON();
            //            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCMA, 0);
        }
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "Right TurnSignal VCC abnormal");
            Display_YX_Format(1, 0, "%f", measure.result);
            Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
            Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
        }
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_CURRENT_VALUE: // 检查电流值步骤
        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // 右转向灯测试,最后测电流
        if ((measure.result > dut_info.currentMin) && (measure.result < dut_info.currentMax))
        {
            // 测试通过关闭右转向灯
            //            CAN_PROTOCOL1_SendCmdWith_TwoResult(CAN_PROTOCOL1_CMD_TURN_SIGNAL_CONTROL, 1, 0);
        }
        else
        {
#if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "Right TurnSignal CURRENT abnormal");
            Display_YX_Format(1, 0, "%f", measure.result);
            Display_YX_Format(2, 0, "%d", dut_info.currentMin);
            Display_YX_Format(3, 0, "%d", dut_info.currentMax);
            Delayms(1000);
#endif
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
        }
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_COMPLETE: // 测试完成步骤
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);

        // 关闭右转向灯成功并且上报测试成功
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
        AVO_PIN_Reset();
        STATE_EnterState(STATE_STANDBY);
        break;

    // 超时处理
    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_COMMUNICATION_TIME_OUT:
#if DEBUG_ENABLE
        Clear_All_Lines();
        Display_Centered(0, "Right TurnSignal Timeout");
        Delayms(1000);
#endif
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        AVO_PIN_Reset();
        STATE_EnterState(STATE_STANDBY);
        break;

    default:
        break;
    }
}

// 油门测试
void STEP_Process_ThrottleCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_THROTTLE_CAN_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_VCC);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 

            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE: // 等待仪表或万用表回应
            break;

        case STEP_THROTTLE_CAN_TEST_VCC: // 通知万用表测VCC
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE);

            // 打开VCC测试引脚
            THROTTLE_VCC_TEST_EN(TRUE);

            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_THROTTLE_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VCC
        case STEP_THROTTLE_CAN_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // 调整DAC输出,单位mv
                DAC0_output(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // 获取油门AD值
                CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 1);
            }
            // VCC不正常，则上报
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第一次
        case STEP_THROTTLE_CAN_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC1数值检查逻辑
            if ((dut_info.throttleAd > FIRST_DAC - ERROR_DAC) && (dut_info.throttleAd < FIRST_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                DAC0_output(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // 获取油门AD值
                CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 1);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "SIG");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第二次
        case STEP_THROTTLE_CAN_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.throttleAd > SECOND_DAC - ERROR_DAC) && (dut_info.throttleAd < SECOND_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                DAC0_output(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // 获取油门AD值
                CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 1);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "SIG");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第三次
        case STEP_THROTTLE_CAN_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.throttleAd > THIRD_DAC - ERROR_DAC) && (dut_info.throttleAd < THIRD_DAC + ERROR_DAC))
            {
                // 3次都正常则测试通过
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Third");
                STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_COMPLETE);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
        case STEP_THROTTLE_CAN_TEST_COMPLETE:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
//            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            break;

        // 通信超时，给出油门测试函数，并且case上面的步骤
        case STEP_THROTTLE_CAN_TEST_COMMUNICATION_TIME_OUT:
//            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 刹把测试
void STEP_Process_BrakeCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_BRAKE_CAN_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_VCC);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_BRAKE_CAN_TEST_WAIT_RESPONSE: // 等待仪表或万用表回应
            break;

        case STEP_BRAKE_CAN_TEST_VCC: // 通知万用表测VCC
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_WAIT_RESPONSE);

            // 打开VCC测试引脚
            BRAKE_VCC_TEST_EN_ON();

            // 通知万用表测电压
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_BRAKE_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VCC
        case STEP_BRAKE_CAN_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // 调整DAC输出,单位mv
                DAC1_output(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // 获取油门AD值
                CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 2);
            }
            // VCC不正常，则上报
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第一次
        case STEP_BRAKE_CAN_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC1数值检查逻辑
            if ((dut_info.brakeAd > FIRST_DAC - ERROR_DAC) && (dut_info.brakeAd < FIRST_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                DAC1_output(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // 获取油门AD值
                CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 2);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "SIG1");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第二次
        case STEP_BRAKE_CAN_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.brakeAd > SECOND_DAC - ERROR_DAC) && (dut_info.brakeAd < SECOND_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                DAC1_output(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // 获取油门AD值
                CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 2);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "SIG2");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第三次
        case STEP_BRAKE_CAN_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.brakeAd > THIRD_DAC - ERROR_DAC) && (dut_info.brakeAd < THIRD_DAC + ERROR_DAC))
            {
                // 3次都正常则测试通过
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Third");
                STATE_SwitchStep(STEP_BRAKE_CAN_TEST_COMPLETE);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "SIG3");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
        case STEP_BRAKE_CAN_TEST_COMPLETE:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            break;

        // 通信超时，给出油门测试函数，并且case上面的步骤
        case STEP_BRAKE_CAN_TEST_COMMUNICATION_TIME_OUT:
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 电子变速测试
void STEP_Process_DerailleurCanTest(void)   
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_DERAILLEUR_CAN_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_VCC);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE: // 等待仪表或万用表回应
            break;

        case STEP_DERAILLEUR_CAN_TEST_VCC: // 通知万用表测VCC
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE);

            // 打开VCC测试引脚
            DERAILLEUR_VCC_TEST_EN_ON();

            // 通知万用表测电压
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_DERAILLEUR_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VCC
        case STEP_DERAILLEUR_CAN_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // 调整DAC输出,单位mv
                PWM_OUT1_PWM(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // 获取电子变速AD值
                DUT_PROTOCOL_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 3);
            }
            // VCC不正常，则上报
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第一次
        case STEP_DERAILLEUR_CAN_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC1数值检查逻辑
            if ((dut_info.derailleurAd > FIRST_DAC - ERROR_DAC) && (dut_info.derailleurAd < FIRST_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                PWM_OUT1_PWM(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // 获取电子变速AD值
                DUT_PROTOCOL_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 3);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "SIG1");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第二次
        case STEP_DERAILLEUR_CAN_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.derailleurAd > SECOND_DAC - ERROR_DAC) && (dut_info.derailleurAd < SECOND_DAC + ERROR_DAC))
            {
                // 调整DAC输出,单位mv
                PWM_OUT1_PWM(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // 获取电子变速AD值
                DUT_PROTOCOL_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 3);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "SIG2");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第三次
        case STEP_DERAILLEUR_CAN_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((dut_info.derailleurAd > THIRD_DAC - ERROR_DAC) && (dut_info.derailleurAd < THIRD_DAC + ERROR_DAC))
            {
                // 3次都正常则测试通过
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Third");
                STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_COMPLETE);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "SIG3");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 电子变速测试完成步骤，关闭DAC输出调整为0，然后上报结果
        case STEP_DERAILLEUR_CAN_TEST_COMPLETE:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            break;

        // 通信超时
        case STEP_DERAILLEUR_CAN_TEST_COMMUNICATION_TIME_OUT:
            PWM_OUT1_PWM(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// VLK测试
void STEP_Process_VlkCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        // 入口步骤
        case STEP_VLK_CAN_TEST_ENTRY:
            STATE_SwitchStep(STEP_VLK_CAN_TEST_VLK);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate);

            // LCD显示
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 0);

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        // 等待仪表或万用表回应
        case STEP_VLK_CAN_TEST_WAIT_RESPONSE:
            break;

        // 通知万用表测VLK
        case STEP_VLK_CAN_TEST_VLK:
            STATE_SwitchStep(STEP_VLK_CAN_TEST_WAIT_RESPONSE);

            // 使能VLK测试引脚
            DUT_VLK_TEST_EN_ON();

            // 通知万用表测VLK
//            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

            // LCD显示
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 1);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_VLK_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VLK值
        case STEP_VLK_CAN_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_VLK_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                STATE_SwitchStep(STEP_VLK_CAN_TEST_COMPLETE);
            }
            // VCC不正常，则上报
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);

                // LCD显示
                Vertical_Scrolling_Display(vlkMsgBuff, 4, 2);
            }
            break;

        case STEP_VLK_CAN_TEST_COMPLETE: // 测试完成，然后上报结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);

            // LCD显示
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 3);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        case STEP_VLK_CAN_TEST_COMMUNICATION_TIME_OUT: // 通信超时
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 电压校准
void STEP_Process_CalibrationCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_CALIBRATION_CAN_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_ENTRY);

            break;

        case STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE: // 等待仪表/电源板回应
            break;

        case STEP_CALIBRATION_CAN_TEST_NOTIFY_POWER_BOARD: // 通知电源板调整仪表供电电压
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[0]);            // 通知电源板调整DUT供电电压

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          3000,
                          STATE_SwitchStep,
                          STEP_CALIBRATION_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);

            break;

        // 一级校准
        case STEP_CALIBRATION_CAN_TEST_NOTIFY_VOLTAGE_CALIBRATION: // 调整完成通知仪表进行电压校准
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[0]);
            break;

        case STEP_CALIBRATION_CAN_TEST_TWO_NOTIFY_POWER_BOARD: // 通知电源板调整仪表供电电压
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[1]);            // 通知电源板调整DUT供电电压

            break;

        // 二级校准
        case STEP_CALIBRATION_CAN_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION: // 调整完成通知仪表进行电压校准
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[1]);
            break;

        case STEP_CALIBRATION_CAN_TEST_THREE_NOTIFY_POWER_BOARD: // 通知电源板调整仪表供电电压
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[2]);            // 通知电源板调整DUT供电电压

            break;

        // 三级校准
        case STEP_CALIBRATION_CAN_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION: // 调整完成通知仪表进行电压校准
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[2]);
            break;

        // 校准完成
        case STEP_CALIBRATION_CAN_TEST_COMPLETE: // 测试完成，然后上报结果
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, TRUE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STATE_EnterState(STATE_STANDBY);
            break;

        // 通信超时
        case STEP_CALIBRATION_CAN_TEST_COMMUNICATION_TIME_OUT:
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}
/****************************************升级********************************************/
/******升级全局变量定义*************/

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

// 通用彩屏升级步骤
void STEP_Process_CmUartUpgrade()
{
    uint8 i = 0;

    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        // 升级入口
        case STEP_CM_UART_UPGRADE_ENTRY:
            TIMER_KillTask(TIMER_ID_ONLINE_DETECT);
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);                            // 停止设置config定时器
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // 设置波特率
            UART_PROTOCOL_Init();                        // 55升级协议
            UART_PROTOCOL4_Init();                       // KM5S
            resetFlag = TRUE;
            switch_cnt = 0;

            // vlk供电
            VLK_PW_EN_OFF();

            // 关闭仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // 等待电源板/dut应答
        case STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK:
            // 只做等待，不做操作
            break;

        // 仪表重启
        case STEP_CM_UART_BC_UPGRADE_RESTART:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);                            // 停止设置config定时器

            // vlk供电
            VLK_PW_EN_ON();

            // 打开仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
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

            // 关闭vlk供电
            VLK_PW_EN_OFF();

            // 关闭仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // 设置config入口
        case STEP_UART_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // 允许开机
            UART_DRIVE_InitSelect(9600);     // UART协议层初始化
            UART_PROTOCOL_Init();            // 55升级协议
            UART_PROTOCOL4_Init();           // KM5S
            configResetFlag = TRUE;
            switch_cnt = 0;

            // 关闭vlk供电
            VLK_PW_EN_OFF();

            // 关闭仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // 仪表开机
        case STEP_UART_CONFIG_RESET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            // vlk供电
            VLK_PW_EN_ON();

            // 开启仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
            break;

        // 协议切换
        case STEP_UART_PROTOCOL_SWITCCH:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            // 将仪表锂电2号切换为KM5S协议
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);               // 添加命令头
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // 添加设备地址
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_PROTOCOL_SWITCCH);   // 0xc0 添加命令字
            UART_PROTOCOL4_TxAddData(3);                                     // 添加数据长度
            UART_PROTOCOL4_TxAddData(1);
            UART_PROTOCOL4_TxAddData(1);
            UART_PROTOCOL4_TxAddData(1);

            // 添加检验和与结束符，并添加至发送
            UART_PROTOCOL4_TxAddFrame();
            break;

        // 发送config内容
        case STEP_UART_SEND_SET_CONFIG:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            /*发送dut配置信息*/
            // 大于旧协议固定数据数
            if ((configs[0] - 84 > 54) || (configs[0] - 84 < 54))
            {
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);                    // 添加命令头
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);              // 添加设备地址
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_NEW_WRITE_CONTROL_PARAM); // 0x50 添加命令字
                UART_PROTOCOL4_TxAddData(configs[0]);                                 // 添加数据长度
                for (i = 1; i < configs[0] + 1; i++)                                  // config只有54个字节
                {
                    UART_PROTOCOL4_TxAddData(configs[i]);
                }
                // 添加检验和与结束符，并添加至发送
                UART_PROTOCOL4_TxAddFrame();
            }

            // 参数内容为54个字节
            if ((configs[0] - 84) == 54)
            {
                // 新协议
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);                       // 添加命令头
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);                 // 添加设备地址
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_NEW_WRITE_CONTROL_PARAM);    // 0x50 添加命令字
                UART_PROTOCOL4_TxAddData(configs[0]);                                    // 添加数据长度
                for (i = 1; i < configs[0] + 1; i++)                                     // config只有54个字节
                {
                    UART_PROTOCOL4_TxAddData(configs[i]);
                }
                // 添加检验和与结束符，并添加至发送
                UART_PROTOCOL4_TxAddFrame();

                // 旧协议
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);                // 添加命令头
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);          // 添加设备地址
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_WRITE_CONTROL_PARAM); // 0xc0 添加命令字
                UART_PROTOCOL4_TxAddData(54);                                     // 添加数据长度
                for (i = 0; i < 54; i++)                                          // config只有54个字节
                {
                    UART_PROTOCOL4_TxAddData(resultArray[i]);
                }
                // 添加检验和与结束符，并添加至发送
                UART_PROTOCOL4_TxAddFrame();
            }
            break;

        // 读取配置参数
        case STEP_UART_READ_CONFIG:
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);                              // 停止设置config定时器
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);                    // 添加命令头
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);              // 添加设备地址
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_READ_CONTROL_PARAM);      // 0xc2添加命令字
            UART_PROTOCOL4_TxAddData(0);                                          // 添加数据长度

            // 添加检验和与结束符，并添加至发送
            UART_PROTOCOL4_TxAddFrame();
            break;

        // config设置成功
        case STEP_UART_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            // 关闭vlk供电
            VLK_PW_EN_OFF();

            dut_info.configUpFlag = FALSE; // 标志位置false
            dut_info.configUpSuccesss = TRUE;
            dut_info.dutPowerOnAllow = FALSE; // 允许开机
            break;

        // 升级项完成
        case STEP_CM_UART_BC_UPGRADE_ITEM_FINISH:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            if (dut_info.configUpFlag)
            {
                STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
            }
            else
            {
                // 没有升级项
                STATE_EnterState(STATE_STANDBY);

                // 关闭vlk供电
                VLK_PW_EN_OFF();
            }
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
            TIMER_KillTask(TIMER_ID_ONLINE_DETECT);
            STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UART协议层初始化
            UART_PROTOCOL_Init();                        // 55升级协议
            UART_PROTOCOL4_Init();                       // KM5S
            resetFlag = TRUE;
            switch_cnt = 0;

            // vlk供电
            VLK_PW_EN_OFF();

            // 关闭仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // 等待
        case STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK:
            break;

        // 仪表重启
        case STEP_CM_UART_SEG_UPGRADE_RESTART:

            // vlk供电
            VLK_PW_EN_ON();

            // 打开仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
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

        // app升级成功重启
        case STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS_RESET:

            // 关闭仪表供电电源
            switch_cnt = 0;
            appResetFlag = TRUE;

            // vlk供电
            VLK_PW_EN_OFF();
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // 升级config入口
        case STEP_UART_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
            UART_DRIVE_InitSelect(9600); // UART协议层初始化
            UART_PROTOCOL_Init();        // 55升级协议
            UART_PROTOCOL4_Init();       // KM5S
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
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);       // 发送协议切换命令
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR); // 添加设备地址
            UART_PROTOCOL4_TxAddData(0xAB);                          // 0xAB：协议切换
            UART_PROTOCOL4_TxAddData(0);                             // 添加数据长度
            UART_PROTOCOL4_TxAddData(01);
            UART_PROTOCOL4_TxAddFrame(); // 添加检验和与结束符，并添加至发送

            /*发送dut配置信息*/
            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60);  // 实际只有54个字节
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);               // 添加命令头
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // 添加设备地址
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0 添加命令字
            UART_PROTOCOL4_TxAddData(54);                                    // 添加数据长度
            // UART_PROTOCOL4_TxAddData(1);//背光亮度1，看现象
            for (i = 0; i < 54; i++)
            {
                UART_PROTOCOL4_TxAddData(configs[i]); // 写入结果
            }
            UART_PROTOCOL4_TxAddFrame(); // 添加检验和与结束符，并添加至发送
            break;

        // 升级config完成
        case STEP_UART_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止定时器
            UART_DRIVE_InitSelect(115200);           // 设置波特率
            dut_info.configUpFlag = FALSE;           // 标志位置false
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

// 通用CAN升级,APP类型为hex
void STEP_Process_CmCanUpgrade(void)
{
    uint8 configs[60] = {0};
    uint8 i;

    switch (stateCB.step)
    {
        // 空步骤
        case STEP_NULL:
            break;

        // 通用can升级入口，dut上电和can初始化
        case STEP_CM_CAN_UPGRADE_ENTRY:
            TIMER_KillTask(TIMER_ID_ONLINE_DETECT);
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.can = CAN_FF_STANDARD;
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);
            resetFlag = TRUE;
            switch_cnt = 0;

            // vlk供电
            VLK_PW_EN_OFF();

            // 关闭仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            dut_info.reconnectionRepeatOne = FALSE;
            break;

        // 等待电源板/dut应答
        case STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK:
            break;

        // 仪表重启
        case STEP_CM_CAN_UPGRADE_RESTART:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            // vlk供电
            VLK_PW_EN_ON();

            // 打开仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
            break;

        // 收到dut的eco请求,延时100ms后发送eco允许
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
            dut_info.dutPowerOnAllow = FALSE;
            TIMER_KillTask(TIMER_ID_RECONNECTION);
            break;

        // app升级成功重启
        case STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_RESET:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            // 关闭仪表供电电源
            switch_cnt = 0;
            appResetFlag = TRUE;

            // vlk供电
            VLK_PW_EN_OFF();
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // 升级成功彩屏显示黄色
        case STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_YELLOW:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.can = CAN_EXTENDED;
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            CAN_PROTOCOL1_Init(); // 通用配置初始化

            TIMER_AddTask(TIMER_ID_CHANGE_LCD_COLOR,
                          500,
                          CAN_PROTOCOL1_SendCmdChangeColor,
                          0,
                          -1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 设置config入口
        case STEP_CAN_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // 允许开机
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            CAN_PROTOCOL1_Init();  // 通用配置初始化
            addTimeOutTimer(5000); // 设置config时间：5s
            configResetFlag = TRUE;
            switch_cnt = 0;

            // 关闭vlk供电
            VLK_PW_EN_OFF();

            // 关闭仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // 仪表开机
        case STEP_CAN_CONFIG_RESET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            // vlk供电
            VLK_PW_EN_ON();

            // 开启仪表供电电源
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
            break;

        // 发送config内容
        case STEP_CAN_SEND_SET_CONFIG:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            /*发送dut配置信息*/
            // 大于旧协议固定数据数
            if ((configs[0] - 84 > 54) || (configs[0] - 84 < 54))
            {
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                     // 添加命令头
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);              // 添加设备地址
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_NEW_WRITE_CONTROL_PARAM);  // 0x50 添加命令字
                CAN_PROTOCOL1_TxAddData(configs[0]);                                 // 添加数据长度
                for (i = 1; i < configs[0] + 1; i++)                                 // config只有54个字节
                {
                    CAN_PROTOCOL1_TxAddData(configs[i]);
                }
                // 添加检验和与结束符，并添加至发送
                CAN_PROTOCOL1_TxAddFrame();
            }

            // 参数内容为54个字节
            if ((configs[0] - 84) == 54)
            {
                // 新协议
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                        // 添加命令头
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);                 // 添加设备地址
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_NEW_WRITE_CONTROL_PARAM);     // 0x50 添加命令字
                CAN_PROTOCOL1_TxAddData(configs[0]);                                    // 添加数据长度
                for (i = 1; i < configs[0] + 1; i++)                                    // config只有54个字节
                {
                    CAN_PROTOCOL1_TxAddData(configs[i]);
                }
                // 添加检验和与结束符，并添加至发送
                CAN_PROTOCOL1_TxAddFrame();

                // 旧协议
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                 // 添加命令头
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);          // 添加设备地址
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM);  // 0xc0 添加命令字
                CAN_PROTOCOL1_TxAddData(54);                                     // 添加数据长度
                for (i = 0; i < 54; i++)                                         // config只有54个字节
                {
                    CAN_PROTOCOL1_TxAddData(resultArray[i]);
                }
                // 添加检验和与结束符，并添加至发送
                CAN_PROTOCOL1_TxAddFrame();
            }
            break;

        // 读取配置参数
        case STEP_CAN_READ_CONFIG:
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);                             // 停止设置config定时器
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                     // 添加命令头
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);              // 添加设备地址
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM);       // 0xc2添加命令字
            CAN_PROTOCOL1_TxAddData(0);                                          // 添加数据长度

            // 添加检验和与结束符，并添加至发送
            CAN_PROTOCOL1_TxAddFrame();
            break;

        // config设置成功
        case STEP_CAN_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // 停止设置config定时器
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // 停止超时定时器
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);   // 设置波特率
            dut_info.configUpFlag = FALSE; // 标志位置false
            dut_info.configUpSuccesss = TRUE;
            dut_info.dutPowerOnAllow = FALSE;
            break;

        // 升级项升级成功
        case STEP_CM_CAN_UPGRADE_ITEM_FINISH:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            // 选择升级
            if (dut_info.configUpFlag)
            {
                STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
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
            else if (dut_info.configUpFlag)
            {
                dut_info.configUpFaile = TRUE; // config升级失败
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

            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

/****************************************小米测试********************************************/

// 大灯测试
void STEP_Process_XM_HeadLightUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_HEADLIGHT_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "XM HEADLIGHT");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE: // 等待回应步骤
            break;

        case STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT: // 通知DUT打开大灯步骤
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_LIGHT, 1);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_HEADLIGHT_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_HEADLIGHT_UART_TEST_OPEN:      // 仪表打开大灯成功
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 打开VCC测试引脚
            LED_VCC_TEST_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "XM HEADLIGHT");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_HEADLIGHT_UART_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 比对万用表返回的数值
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "XM HEADLIGHT");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // 打开GND测试引脚
                LED_VCC_TEST_EN_ON();
                LED_VCC_EN_ON();

                // 通知万用表测VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "XM HEADLIGHT");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // 比对失败
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "XM VCC Abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_HEADLIGHT_UART_TEST_CHECK_GND_VALUE: // 比对GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 小于3V则GND导通测试通过
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // 关闭大灯
                UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_LIGHT, 0);
                Clear_All_Lines();
                Display_Centered(0, "XM HEADLIGHT");
                Display_Centered(1, "GND");
                Display_Centered(2, "Test Pass");
                STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);       
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "XM HEADLIGHT");
                Display_Centered(1, "GND Abnormal");
                Display_YX_Format(2, 0, "%f", measure.result);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_SwitchStep(STATE_STANDBY);

            }
            break;

        case STEP_HEADLIGHT_UART_TEST_COMPLETE: // 测试完成
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "XM HEADLIGHT");
            Display_Centered(1, "Test Pass");

            // 上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // 超时处理
        case STEP_HEADLIGHT_UART_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "XM HEADLIGHT");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}


// 油门测试
void STEP_Process_XM_ThrottleUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_THROTTLE_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_THROTTLE_UART_TEST_WAIT_RESPONSE: // 等待仪表或万用表回应
            break;

        case STEP_THROTTLE_UART_TEST_VCC: // 通知万用表测VCC
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);

            // 打开VCC测试引脚
            THROTTLE_VCC_TEST_EN(TRUE);

            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_THROTTLE_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VCC
        case STEP_THROTTLE_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // 调整DAC输出,单位mv
                DAC0_output(1200);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // 获取油门AD值
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_TEST_Throttle_Break);
            }
            // VCC不正常，则上报
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第一次
        case STEP_THROTTLE_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC1数值检查逻辑
            if (0 == dut_info.throttleAd )
            {
                // 调整DAC输出,单位mv
                DAC0_output(2100);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // 获取油门AD值
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_TEST_Throttle_Break);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "SIG");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第二次
        case STEP_THROTTLE_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((2 == dut_info.throttleAd )||(3 == dut_info.throttleAd))
            {
                // 调整DAC输出,单位mv
                DAC0_output(4200);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // 获取油门AD值
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_TEST_Throttle_Break);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "SIG");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第三次
        case STEP_THROTTLE_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if (6 == dut_info.throttleAd )
            {
                // 3次都正常则测试通过
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Third");
                STATE_SwitchStep(STEP_THROTTLE_UART_TEST_COMPLETE);
            }
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
        case STEP_THROTTLE_UART_TEST_COMPLETE:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            break;

        // 通信超时，给出油门测试函数，并且case上面的步骤
        case STEP_THROTTLE_UART_TEST_COMMUNICATION_TIME_OUT:
            DAC0_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 刹把测试
void STEP_Process_XM_BrakeUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_BRAKE_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_BRAKE_UART_TEST_WAIT_RESPONSE: // 等待仪表或万用表回应
            break;

        case STEP_BRAKE_UART_TEST_VCC: // 通知万用表测VCC
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);

            // 打开VCC测试引脚
            BRAKE_VCC_TEST_EN_ON();

            // 通知万用表测电压
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_BRAKE_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // 比对VCC
        case STEP_BRAKE_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // 调整DAC输出,单位mv
                DAC1_output(1200);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // 获取油门AD值
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_TEST_Throttle_Break);
            }
            // VCC不正常，则上报
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第一次
        case STEP_BRAKE_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC1数值检查逻辑
            if (0 == dut_info.brakeAd)
            {
                // 调整DAC输出,单位mv
                DAC1_output(2100);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // 获取油门AD值
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_TEST_Throttle_Break);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "SIG1");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第二次
        case STEP_BRAKE_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if ((2 == dut_info.brakeAd)||(3 == dut_info.brakeAd))
            {
                // 调整DAC输出,单位mv
                DAC1_output(4200);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // 获取油门AD值
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_TEST_Throttle_Break);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "SIG2");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 在这进行判断数值是否正常，第三次
        case STEP_BRAKE_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 处理DAC5数值检查逻辑
            if (6 == dut_info.brakeAd)
            {
                // 3次都正常则测试通过
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Third");
                STATE_SwitchStep(STEP_BRAKE_UART_TEST_COMPLETE);
            }
            else
            {
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "SIG3");
                Display_Centered(2, "Abnormal");
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
                STATE_EnterState(STATE_STANDBY);
            }
            break;

        // 油门测试完成步骤，关闭DAC输出调整为0，然后上报结果
        case STEP_BRAKE_UART_TEST_COMPLETE:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
            DAC1_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            break;

        // 通信超时，给出油门测试函数，并且case上面的步骤
        case STEP_BRAKE_UART_TEST_COMMUNICATION_TIME_OUT:
            DAC1_output(0);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

uint16 result = 0;
uint16 vccVlaue = 0;
uint16 gndVlaue1 = 0;
uint16 gndVlaue2 = 0;
uint16 gndVlaue3 = 0;

// 左转向灯测试
void STEP_Process_XM_Left_TurnSignalUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();
            light_cnt = 0;
            headlight_cnt = 0;           
            Clear_All_Lines();
            Display_Centered(0, "XM LEFT SIGNAL");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE: // 等待回应步骤
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT:   // 通知DUT打开左转向灯
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT,1);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_LEFT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN:      // 仪表打开左转向灯成功
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 打开VCC测试引脚
            LEFT_VCC_TEST_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "XM LEFT SIGNAL");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            
            // 比对万用表返回的数值
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "XM LEFT SIGNAL");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // 打开GND测试引脚
                LEFT_VCC_TEST_EN_ON();
                LEFT_VCC_EN_ON();
                vccVlaue = measure.result;
                
                // 通知万用表测VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
//                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

                Clear_All_Lines();
                Display_Centered(0, "XM LEFT SIGNAL");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // 比对失败
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "VCC Abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE: // 比对GND
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

//            // 记录第一次GND电压值
//            gndVlaue1 = measure.result;
//            
//            // 打开GND测试引脚
//            LEFT_VCC_TEST_EN_ON();
//            LEFT_VCC_EN_ON();
//            
//            // 通知万用表测VCC
//            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
//            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);   
            
            // 小于3V则GND导通测试通过
            if (3000 > measure.result)
            {
                // 关闭左转向灯
                UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT,0);
                Clear_All_Lines();
                Display_Centered(0, "XM LEFT SIGNAL");
                Display_Centered(1, "GND");
                Display_Centered(2, "Test Pass");
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "XM LEFT SIGNAL");
                Display_Centered(1, "GND Abnormal");
                Display_YX_Format(2, 0, "%f", measure.result);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

//        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2: // 比对GND
//            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
//            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
//            
//            // 记录第二次GND电压值
//            gndVlaue2 = measure.result; 
//            
//            result = abs(gndVlaue2 - gndVlaue1);
//            result = abs(gndVlaue2 - vccVlaue);
//            result = abs(gndVlaue1 - gndVlaue1);
//                     
//            if ((abs(gndVlaue2 - gndVlaue1) > 2000) ||  (abs(gndVlaue2 - vccVlaue) > 2000) ||  (abs(gndVlaue1 - vccVlaue) > 2000))
//            {
//                // 关闭左转向灯
//                UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT,0);
//                Clear_All_Lines();
//                Display_Centered(0, "XM LEFT SIGNAL");
//                Display_Centered(1, "GND");
//                Display_Centered(2, "Test Pass");
//            }
//            else
//            {
//    #if DEBUG_ENABLE
//                Clear_All_Lines();
//                Display_Centered(0, "XM LEFT SIGNAL");
//                Display_Centered(1, "GND Abnormal");
//                Display_YX_Format(2, 0, "%f", measure.result);
//    #endif
//                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
//                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
//            }
//            break;
            
        case STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE: // 测试完成
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "XM LEFT SIGNAL");
            Display_Centered(1, "Test Pass");

            // 上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // 超时处理
        case STEP_LEFT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "XM LEFT SIGNAL");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// 右转向灯测试
void STEP_Process_XM_Right_TurnSignalUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // 空步骤
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY: // 入口步骤
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();            
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "XM RIGHT SIGNAL");
            Display_Centered(1, "Testing");

            // 测试引脚复位
            AVO_PIN_Reset();
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE: // 等待回应步骤
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT: // 通知DUT打开右转向灯
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
             UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT, 2);

            // 添加超时
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          6000,
                          STATE_SwitchStep,
                          STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN:      // 仪表打开远光灯成功
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 打开VCC测试引脚
            RIGHT_VCC_TEST_EN_ON();

            // 通知万用表测VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

            Clear_All_Lines();
            Display_Centered(0, "XM RIGHT SIGNAL");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE: // 检查电压值步骤
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // 比对万用表返回的数值
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "XM RIGHT SIGNAL");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");
                vccVlaue = measure.result;
                
                // 打开GND测试引脚
                RIGHT_VCC_TEST_EN_ON();
                RIGHT_VCC_EN_ON();
                
                // 通知万用表测VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "XM RIGHT SIGNAL");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // 比对失败
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "XM VCC Abnormal");
                Display_YX_Format(1, 0, "%f", measure.result);
                Display_YX_Format(2, 0, "%d", dut_info.voltageMin);
                Display_YX_Format(3, 0, "%d", dut_info.voltageMax);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE: // 比对GND
//            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
//            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
//                gndVlaue1 = measure.result;

//                // 打开GND测试引脚
//                RIGHT_VCC_TEST_EN_ON();
//                RIGHT_VCC_EN_ON();
//                
//                // 通知万用表测VCC
//                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            // 小于3V则GND导通测试通过
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            
            if (3000 > measure.result)
            {
                // 关闭左转向灯
                UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT,0);
                Clear_All_Lines();
                Display_Centered(0, "XM LEFT SIGNAL");
                Display_Centered(1, "GND");
                Display_Centered(2, "Test Pass");
            }
            else
            {
    #if DEBUG_ENABLE
                Clear_All_Lines();
                Display_Centered(0, "XM LEFT SIGNAL");
                Display_Centered(1, "GND Abnormal");
                Display_YX_Format(2, 0, "%f", measure.result);
    #endif
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

//        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2: // 比对GND
//            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
//            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
//                gndVlaue2 = measure.result;

//                // 打开GND测试引脚
//                RIGHT_VCC_TEST_EN_ON();
//                RIGHT_VCC_EN_ON();
//                
//                // 通知万用表测VCC
//                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
//            break;
//            
//        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE3: // 比对GND
//            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
//            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
//            gndVlaue3 = measure.result;

//            // 小于3V则GND导通测试通过                     
//            if ((abs(gndVlaue2 - gndVlaue1) > 2000) || (abs(gndVlaue2 - vccVlaue) > 2000) || (abs(gndVlaue1 - vccVlaue) > 2000)\
//            || (abs(gndVlaue3 - vccVlaue) > 2000) || (abs(gndVlaue3 - gndVlaue1) > 2000) || (abs(gndVlaue3 - gndVlaue2) > 2000))
//            {
//                // 关闭右转向灯
//                UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT, 0);
//                Clear_All_Lines();
//                Display_Centered(0, "XM RIGHT SIGNAL");
//                Display_Centered(1, "GND");
//                Display_Centered(2, "Test Pass");
//            }
//            else
//            {
//    #if DEBUG_ENABLE
//                Clear_All_Lines();
//                Display_Centered(0, "XM RIGHT SIGNAL");
//                Display_Centered(1, "GND Abnormal");
//                Display_YX_Format(2, 0, "%f", measure.result);
//    #endif
//                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
//                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
//            }
//            break;           
            
        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE: // 测试完成
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "XM RIGHT SIGNAL");
            Display_Centered(1, "Test Pass");

            // 上报测试成功
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // 超时处理
        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            AVO_PIN_Reset();
            Clear_All_Lines();
            Display_Centered(0, "XM RIGHT SIGNAL");
            Display_Centered(1, "Test Timeout");
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

/****************************************升级-END********************************************/
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
            break;

        // GND测试
        case STATE_UART_GND_TEST:
            STATE_SwitchStep(STEP_GND_TEST_ENTRY);
            break;

        /******************************************UART***************************************/

        // 大灯测试
        case STATE_UART_HEADLIGHT_TEST:
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_ENTRY);
            break;

        // 近光灯测试
        case STATE_UART_LBEAM_TEST:
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_ENTRY);
            break;

        // 远光灯测试
        case STATE_UART_HBEAM_TEST:
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_ENTRY);
            break;

        // 左转向灯测试
        case STATE_UART_LEFT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY);
            break;

        // 右转向灯测试
        case STATE_UART_RIGHT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY);
            break;

        // 油门测试
        case STATE_UART_THROTTLE_TEST:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_ENTRY);
            break;

        // 刹把测试
        case STATE_UART_BRAKE_TEST:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_ENTRY);
            break;

        // 电子变速测试
        case STATE_UART_DERAILLEUR_TEST:
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_ENTRY);
            break;
            
        // vlk测试
        case STATE_UART_VLK_TEST:
            STATE_SwitchStep(STEP_VLK_UART_TEST_ENTRY);
            break;

        // 电压校准测试
        case STATE_UART_VOLTAGE_TEST:
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_ENTRY);
            break;

        // UART测试
        case STATE_UART_TEST:
            STATE_SwitchStep(STEP_UART_TEST_ENTRY);
            break;

        /*****************************CAN***************************************/

        // 大灯测试
        case STATE_CAN_HEADLIGHT_TEST:
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_ENTRY);
            break;

        // 近光灯测试
        case STATE_CAN_LBEAM_TEST:
            STATE_SwitchStep(STEP_LBEAM_CAN_TEST_ENTRY);
            break;

        // 远光灯测试
        case STATE_CAN_HBEAM_TEST:
            STATE_SwitchStep(STEP_HBEAM_CAN_TEST_ENTRY);
            break;

        // 左转向灯测试
        case STATE_CAN_LEFT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_ENTRY);
            break;

        // 右转向灯测试
        case STATE_CAN_RIGHT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_ENTRY);
            break;

        // 油门测试
        case STATE_CAN_THROTTLE_TEST:
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_ENTRY);
            break;

        // 刹车测试
        case STATE_CAN_BRAKE_TEST:
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_ENTRY);
            break;

        // 电子变速测试
        case STATE_CAN_DERAILLEUR_TEST:
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_COMMUNICATION_TIME_OUT);
            break;
            
        // vlk测试
        case STATE_CAN_VLK_TEST:
            STATE_SwitchStep(STEP_VLK_CAN_TEST_ENTRY);
            break;

        // 电压校准测试
        case STATE_CAN_VOLTAGE_TEST:
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_ENTRY);
            break;

        /*****************************UP***************************************/

        // 通用彩屏串口升级
        case STATE_CM_UART_BC_UPGRADE:
            STATE_SwitchStep(STEP_CM_UART_UPGRADE_ENTRY);
            break;

        // 通用数码管/段码屏串口升级
        case STATE_CM_UART_SEG_UPGRADE:
            STATE_SwitchStep(STEP_SEG_UART_UPGRADE_ENTRY);
            break;

        // 通用彩屏CAN升级
        case STATE_CM_CAN_UPGRADE:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ENTRY);
            break;

        /************************************以下为小米串口测试***************************************************/

        // 大灯测试
        case STATE_UART_XM_HEADLIGHT_TEST:
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_ENTRY);
            break;

        // 油门测试
        case STATE_UART_XM_THROTTLE_TEST:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_ENTRY);
            break;

        // 刹车测试
        case STATE_UART_XM_BRAKE_TEST:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_ENTRY);
            break;

        // 左转向灯测试
        case STATE_UART_XM_LEFT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY);
            break;

        // 右转向灯测试
        case STATE_UART_XM_RIGHT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY);
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

        // UART测试
        case STATE_UART_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            UART_Test();
            break;

        // GND测试
        case STATE_UART_GND_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            All_GND_Test();
            break;

        /* 以下为串口测试  */
        // 大灯测试
        case STATE_UART_HEADLIGHT_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_HeadLightUartTest();
            break;

        // 近光灯测试
        case STATE_UART_LBEAM_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_LbeamUartTest();
            break;

        // 远光灯测试
        case STATE_UART_HBEAM_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_HbeamUartTest();
            break;

        // 左转向灯测试
        case STATE_UART_LEFT_TURN_SIGNAL_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_Left_TurnSignalUartTest();
            break;

        // 右转向灯测试
        case STATE_UART_RIGHT_TURN_SIGNAL_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_Right_TurnSignalUartTest();
            break;

        // 油门测试
        case STATE_UART_THROTTLE_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_ThrottleUartTest();
            break;

        // 刹把测试
        case STATE_UART_BRAKE_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_BrakeUartTest();
            break;

        // 电子变速测试
        case STATE_UART_DERAILLEUR_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_DerailleurUartTest();
            break;
            
        // VLK测试
        case STATE_UART_VLK_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_VlkUartTest();
            break;

        // 电压校准测试
        case STATE_UART_VOLTAGE_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            DUT_UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_CalibrationUartTest();
            break;

        /* 以下为CAN测试  */
        // 大灯测试
        case STATE_CAN_HEADLIGHT_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_HeadLightCanTest();
            break;

        // 近光灯测试
        case STATE_CAN_LBEAM_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_LbeamCanTest();
            break;

        // 远光灯测试
        case STATE_CAN_HBEAM_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_HbeamCanTest();
            break;

        // 左转向灯测试
        case STATE_CAN_LEFT_TURN_SIGNAL_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_Left_TurnSignalCanTest();
            break;

        // 右转向灯测试
        case STATE_CAN_RIGHT_TURN_SIGNAL_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_Right_TurnSignalCanTest();
            break;

        // 油门测试
        case STATE_CAN_THROTTLE_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_ThrottleCanTest();
            break;

        // 刹把测试
        case STATE_CAN_BRAKE_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_BrakeCanTest();
            break;

        // 电子变速测试
        case STATE_CAN_DERAILLEUR_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_DerailleurCanTest();
            break;
            
        // VLK测试
        case STATE_CAN_VLK_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_VlkCanTest();
            break;

        // 电压校准测试
        case STATE_CAN_VOLTAGE_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_CalibrationCanTest();
            break;

        /************************************以下为CAN升级***************************************************/

        // 通用彩屏can升级
        case STATE_CM_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // 迪太升级协议
            CAN_PROTOCOL_Process_3A();      // 添加app中的3a解析协议
            STEP_Process_CmCanUpgrade();
            break;

        /************************************以下为串口升级***************************************************/

        // 通用彩屏串口升级
        case STATE_CM_UART_BC_UPGRADE:
            UART_DRIVE_Process();               // UART驱动层过程处理
            UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_CmUartUpgrade();   // 步骤
            break;

        // 数码管、段码屏串口升级
        case STATE_CM_UART_SEG_UPGRADE:
            UART_DRIVE_Process();           // UART驱动层过程处理
            UART_PROTOCOL_Process();        // UART协议层过程处理
            STEP_Process_SegUartUpgrade();
            break;
        
        /************************************以下为小米串口测试***************************************************/
        
        // (小米)大灯测试
        case STATE_UART_XM_HEADLIGHT_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            UART_PROTOCOL_XM_Process();         // UART协议层过程处理
            STEP_Process_XM_HeadLightUartTest();
            break;

        // (小米)左转向灯测试
        case STATE_UART_XM_LEFT_TURN_SIGNAL_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            UART_PROTOCOL_XM_Process();         // UART协议层过程处理
            STEP_Process_XM_Left_TurnSignalUartTest();
            break;

        // (小米)右转向灯测试
        case STATE_UART_XM_RIGHT_TURN_SIGNAL_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            UART_PROTOCOL_XM_Process();         // UART协议层过程处理
            STEP_Process_XM_Right_TurnSignalUartTest();
            break;

        // (小米)油门测试
        case STATE_UART_XM_THROTTLE_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            UART_PROTOCOL_XM_Process();         // UART协议层过程处理
            STEP_Process_XM_ThrottleUartTest();
            break;

        // (小米)刹把测试
        case STATE_UART_XM_BRAKE_TEST:
            UART_DRIVE_Process();               // UART驱动层过程处理
            UART_PROTOCOL_XM_Process();         // UART协议层过程处理
            STEP_Process_XM_BrakeUartTest();
            break;
            
        default:
            break;
    }
}
