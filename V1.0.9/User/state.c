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

// vlk������ʾ
char vlkMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "VLK Testing",
    "AVO Measurement",
    "VLK Abnormal",
    "VLK Test Pass",
};

// vlk������ʾ
char gndMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "GND Testing",
    "AVO Measurement",
    "THRO Ab",
    "THRO Test Pass",
};

// �쳣������ʾ
char excepMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "Test Timeout",
};

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

// GND��� �Ȳ�����GND���ٲ�ɲ��GND
void  All_GND_Test(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_GND_TEST_WAIT_RESPONSE: // �ȴ����ñ��Ӧ
            break;

        case STEP_GND_TEST_ENTRY: // ��ڲ���
            gnd_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "GND");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();

            // ��ӳ�ʱ
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

        case STEP_GND_TEST_THROTTLE_GND: // �������迹
            STATE_SwitchStep(STEP_GND_TEST_WAIT_RESPONSE);

            // �򿪲�������
            THROTTLE_GND_TEST_EN_ON();

            // ֪ͨ���ñ���迹
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            Clear_All_Lines();
            Display_Centered(0, "GND");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE:     // �ȶ�GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);      // ι��

            // ����GND����
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

        case STEP_GND_TEST_BRAKE_GND: // ��ɲ���迹
            STATE_SwitchStep(STEP_GND_TEST_WAIT_RESPONSE);

            // �򿪲�������
            BRAKE_GND_TEST_EN_ON();

            // ֪ͨ���ñ���迹
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            Clear_All_Lines();
            Display_Centered(0, "GND");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_GND_TEST_CHECK_BRAKE_GND_VALUE:     // �ȶ�GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);  // ι��

            // ɲ��GND����
            if ((50 > ohm.INT) && (dut_info.gnd != GND_TYPE_BRAKE_DERAILLEUR))
            {
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "BRAKE");
                Display_Centered(2, "GND Normal");
                STATE_SwitchStep(STEP_GND_TEST_COMPLETE);
            }
            
            // ɲ��GND����
            if ((50 > ohm.INT) && (dut_info.gnd == GND_TYPE_BRAKE_DERAILLEUR))
            {
                Clear_All_Lines();
                Display_Centered(0, "GND");
                Display_Centered(1, "BRAKE");
                Display_Centered(2, "GND Normal");
                STATE_SwitchStep(STEP_GND_TEST_DERAILLEUR_GND);
            } 

            // ������            
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

        case STEP_GND_TEST_DERAILLEUR_GND:      // ����ӱ����迹
            STATE_SwitchStep(STEP_GND_TEST_WAIT_RESPONSE);

            // �򿪲�������
            DERAILLEUR_GND_TEST_EN_ON();

            // ֪ͨ���ñ���迹
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            Clear_All_Lines();
            Display_Centered(0, "GND");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_GND_TEST_CHECK_DERAILLEUR_GND_VALUE:     // �ȶ�GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);  // ι��

            // ɲ��GND����
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
            
        case STEP_GND_TEST_COMPLETE: // GND������ɲ��裬�رղ������ţ�Ȼ���ϱ����
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
    #if DEBUG_ENABLE
            Clear_All_Lines();
            Display_Centered(0, "GND Test Pass");
    #endif
            AVO_PIN_Reset();

            // �ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_GND_TEST, TRUE);
            STATE_EnterState(STATE_STANDBY);
            break;

        case STEP_GND_TEST_COMMUNICATION_TIME_OUT: // ͨѶ��ʱ
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
    #if DEBUG_ENABLE
            // LCD��ʾ
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
        case STEP_NULL: // �ղ���
            break;

        case STEP_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_UART_TEST_WAIT_RESPONSE);

            Clear_All_Lines();
            Display_Centered(0, "UART");
            Display_Centered(1, "Testing");

            // �򿪲�������
            DUT_TO_DTA_OFF_ON();
            UART_TEST_EN_ON();

            // ֪ͨ���ñ���ѹ
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          3000,
                          STATE_SwitchStep,
                          STEP_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_UART_TEST_WAIT_RESPONSE: // �ȴ����ñ��Ӧ
            break;

        case STEP_UART_TEST_CHECK_VOLTAGE_VALUE:     // �ȶ�VCC
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);  // ι��

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

        case STEP_UART_TEST_COMPLETE: // UART������ɲ��裬�رղ������ţ�Ȼ���ϱ����
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

        case STEP_UART_TEST_COMMUNICATION_TIME_OUT: // ͨѶ��ʱ
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


// ��Ʋ���
void STEP_Process_HeadLightUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_HEADLIGHT_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "HEADLIGHT");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
            break;

        case STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT: // ֪ͨDUT�򿪴�Ʋ���
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_HEADLIGHT_CONTROL, 1);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_HEADLIGHT_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_HEADLIGHT_UART_TEST_OPEN:      // �Ǳ�򿪴�Ƴɹ�
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��VCC��������
            LED_VCC_TEST_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "HEADLIGHT");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_HEADLIGHT_UART_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // �ȶ����ñ��ص���ֵ
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "HEADLIGHT");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // ��GND��������
                LED_VCC_TEST_EN_ON();
                LED_VCC_EN_ON();

                // ֪ͨ���ñ��VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "HEADLIGHT");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // �ȶ�ʧ��
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

        case STEP_HEADLIGHT_UART_TEST_CHECK_GND_VALUE: // �ȶ�GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // С��3V��GND��ͨ����ͨ��
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // �رմ��
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

        case STEP_HEADLIGHT_UART_TEST_COMPLETE: // �������
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "HEADLIGHT");
            Display_Centered(1, "Test Pass");

            // �ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // ��ʱ����
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

// ����Ʋ���
void STEP_Process_LbeamUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_LBEAM_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_LBEAM_UART_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
            break;

        case STEP_LBEAM_UART_TEST_NOTIFY_DUT: // ֪ͨDUT�򿪽���Ʋ���
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 2, 1);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_LBEAM_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_LBEAM_UART_TEST_OPEN:      // �Ǳ�򿪽���Ƴɹ�
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��VCC��������
            LED_LBEAM_TEST_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_LBEAM_UART_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // �ȶ����ñ��ص���ֵ
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "LBEAM");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // ��GND��������
                LED_LBEAM_TEST_EN_ON();
                LED_LBEAM_EN_ON();

                // ֪ͨ���ñ��VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "LBEAM");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // �ȶ�ʧ��
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

        case STEP_LBEAM_UART_TEST_CHECK_GND_VALUE: // �ȶ�GND
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // С��3V��GND��ͨ����ͨ��
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // �رս����
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

        case STEP_LBEAM_UART_TEST_COMPLETE: // �������
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "Test Pass");

            // �ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // ��ʱ����
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

// Զ��Ʋ���
void STEP_Process_HbeamUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_HBEAM_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_HBEAM_UART_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
            break;

        case STEP_HBEAM_UART_TEST_NOTIFY_DUT: // ֪ͨDUT��Զ��Ʋ���
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 3, 1);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_HBEAM_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_HBEAM_UART_TEST_OPEN:      // �Ǳ��Զ��Ƴɹ�
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��VCC��������
            LED_HBEAM_TEST_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_HBEAM_UART_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // �ȶ����ñ��ص���ֵ
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "HBEAM");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // ��GND��������
                LED_HBEAM_TEST_EN_ON();
                LED_HBEAM_EN_ON();

                // ֪ͨ���ñ��VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "HBEAM");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // �ȶ�ʧ��
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

        case STEP_HBEAM_UART_TEST_CHECK_GND_VALUE: // �ȶ�GND
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // С��3V��GND��ͨ����ͨ��
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // �ر�Զ���
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

        case STEP_HBEAM_UART_TEST_COMPLETE: // �������
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "Test Pass");

            // �ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // ��ʱ����
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

// ��ת��Ʋ���
void STEP_Process_Left_TurnSignalUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "LEFT TURN SIGNAL");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT: // ֪ͨDUT��Զ��Ʋ���
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 0, 1);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_LEFT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN:      // �Ǳ��Զ��Ƴɹ�
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��VCC��������
            LEFT_VCC_TEST_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "LEFT TURN SIGNAL");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // �ȶ����ñ��ص���ֵ
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "LEFT TURN SIGNAL");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // ��GND��������
                LEFT_VCC_TEST_EN_ON();
                LEFT_VCC_EN_ON();

                // ֪ͨ���ñ��VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "LEFT TURN SIGNAL");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // �ȶ�ʧ��
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

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE: // �ȶ�GND
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // С��3V��GND��ͨ����ͨ��
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // �ر�Զ���
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

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE: // �������
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "LEFT TURN SIGNAL");
            Display_Centered(1, "Test Pass");

            // �ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // ��ʱ����
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

// ��ת��Ʋ���
void STEP_Process_Right_TurnSignalUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "RIGHT TURN SIGNAL");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT: // ֪ͨDUT��Զ��Ʋ���
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 1, 1);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN:      // �Ǳ��Զ��Ƴɹ�
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��VCC��������
            LEFT_VCC_TEST_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "RIGHT TURN SIGNAL");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // �ȶ����ñ��ص���ֵ
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "RIGHT TURN SIGNAL");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // ��GND��������
                RIGHT_VCC_TEST_EN_ON();
                RIGHT_VCC_EN_ON();

                // ֪ͨ���ñ��VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "LEFT TURN SIGNAL");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // �ȶ�ʧ��
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

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE: // �ȶ�GND
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // С��3V��GND��ͨ����ͨ��
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // �ر�Զ���
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

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE: // �������
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "RIGHT TURN SIGNAL");
            Display_Centered(1, "Test Pass");

            // �ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // ��ʱ����
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

// ���Ų���
void STEP_Process_ThrottleUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_THROTTLE_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();

            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_THROTTLE_UART_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�����ñ��Ӧ
            break;

        case STEP_THROTTLE_UART_TEST_VCC: // ֪ͨ���ñ��VCC
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);

            // ��VCC��������
            THROTTLE_VCC_TEST_EN(TRUE);

            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_THROTTLE_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VCC
        case STEP_THROTTLE_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // ����DAC���,��λmv
                DAC0_output(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // ��ȡ����ADֵ
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 1);
            }
            // VCC�����������ϱ�
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

        // ��������ж���ֵ�Ƿ���������һ��
        case STEP_THROTTLE_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC1��ֵ����߼�
            if ((dut_info.throttleAd > FIRST_DAC - ERROR_DAC) && (dut_info.throttleAd < FIRST_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                DAC0_output(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ��������ڶ���
        case STEP_THROTTLE_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.throttleAd > SECOND_DAC - ERROR_DAC) && (dut_info.throttleAd < SECOND_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                DAC0_output(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ�������������
        case STEP_THROTTLE_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.throttleAd > THIRD_DAC - ERROR_DAC) && (dut_info.throttleAd < THIRD_DAC + ERROR_DAC))
            {
                // 3�ζ����������ͨ��
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

        // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
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

        // ͨ�ų�ʱ
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

// ɲ�Ѳ���
void STEP_Process_BrakeUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_BRAKE_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_BRAKE_UART_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�����ñ��Ӧ
            break;

        case STEP_BRAKE_UART_TEST_VCC: // ֪ͨ���ñ��VCC
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);

            // ��VCC��������
            BRAKE_VCC_TEST_EN_ON();

            // ֪ͨ���ñ���ѹ
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_BRAKE_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VCC
        case STEP_BRAKE_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // ����DAC���,��λmv
                DAC1_output(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // ��ȡɲ��ADֵ
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 2);
            }
            // VCC�����������ϱ�
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

        // ��������ж���ֵ�Ƿ���������һ��
        case STEP_BRAKE_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC1��ֵ����߼�
            if ((dut_info.brakeAd > FIRST_DAC - ERROR_DAC) && (dut_info.brakeAd < FIRST_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                DAC1_output(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // ��ȡɲ��ADֵ
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

        // ��������ж���ֵ�Ƿ��������ڶ���
        case STEP_BRAKE_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.brakeAd > SECOND_DAC - ERROR_DAC) && (dut_info.brakeAd < SECOND_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                DAC1_output(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // ��ȡɲ��ADֵ
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

        // ��������ж���ֵ�Ƿ�������������
        case STEP_BRAKE_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.brakeAd > THIRD_DAC - ERROR_DAC) && (dut_info.brakeAd < THIRD_DAC + ERROR_DAC))
            {
                // 3�ζ����������ͨ��
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

        // ɲ�Ѳ�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
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

        // ͨ�ų�ʱ
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

// ���ӱ��ٲ���
void STEP_Process_DerailleurUartTest(void)   
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_DERAILLEUR_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�����ñ��Ӧ
            break;

        case STEP_DERAILLEUR_UART_TEST_VCC: // ֪ͨ���ñ��VCC
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE);

            // ��VCC��������
            DERAILLEUR_VCC_TEST_EN_ON();

            // ֪ͨ���ñ���ѹ
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_DERAILLEUR_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VCC
        case STEP_DERAILLEUR_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // ����DAC���,��λmv
                PWM_OUT1_PWM(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // ��ȡɲ��ADֵ
                DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD, 3);
            }
            // VCC�����������ϱ�
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

        // ��������ж���ֵ�Ƿ���������һ��
        case STEP_DERAILLEUR_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC1��ֵ����߼�
            if ((dut_info.derailleurAd > FIRST_DAC - ERROR_DAC) && (dut_info.derailleurAd < FIRST_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                PWM_OUT1_PWM(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // ��ȡɲ��ADֵ
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

        // ��������ж���ֵ�Ƿ��������ڶ���
        case STEP_DERAILLEUR_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.derailleurAd > SECOND_DAC - ERROR_DAC) && (dut_info.derailleurAd < SECOND_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                PWM_OUT1_PWM(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // ��ȡɲ��ADֵ
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

        // ��������ж���ֵ�Ƿ�������������
        case STEP_DERAILLEUR_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.derailleurAd > THIRD_DAC - ERROR_DAC) && (dut_info.derailleurAd < THIRD_DAC + ERROR_DAC))
            {
                // 3�ζ����������ͨ��
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

        // ɲ�Ѳ�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
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

        // ͨ�ų�ʱ
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

// VLK����
void STEP_Process_VlkUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_VLK_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_VLK_UART_TEST_VLK);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();

            // LCD��ʾ
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 0);

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        // �ȴ����ñ��Ӧ
        case STEP_VLK_UART_TEST_WAIT_RESPONSE:
            break;

        // ֪ͨ���ñ��VLK
        case STEP_VLK_UART_TEST_VLK:
            STATE_SwitchStep(STEP_VLK_UART_TEST_WAIT_RESPONSE);

            // ʹ��VLK��������
            DUT_VLK_TEST_EN_ON();

            // ֪ͨ���ñ��VLK
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

            // LCD��ʾ
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 1);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          3000,
                          STATE_SwitchStep,
                          STEP_VLK_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VLKֵ
        case STEP_VLK_UART_TEST_CHECK_VOLTAGE_VALUE:

            STATE_SwitchStep(STEP_VLK_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                STATE_SwitchStep(STEP_VLK_UART_TEST_COMPLETE);
            }
            // VCC�����������ϱ�
            else
            {
                // LCD��ʾ
                Vertical_Scrolling_Display(vlkMsgBuff, 4, 2);
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
                TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            }
            break;

        // ������ɣ�Ȼ���ϱ����
        case STEP_VLK_UART_TEST_COMPLETE:
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);

            // LCD��ʾ
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 3);
            break;

        case STEP_VLK_UART_TEST_COMMUNICATION_TIME_OUT: // ͨ�ų�ʱ
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();

            // LCD��ʾ
            Vertical_Scrolling_Display(excepMsgBuff, 4, 2);
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// ��ѹУ׼
void STEP_Process_CalibrationUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_CALIBRATION_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_NOTIFY_POWER_BOARD);
            cali_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Testing");
            break;

        case STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�/��Դ���Ӧ
            break;

        case STEP_CALIBRATION_UART_TEST_NOTIFY_POWER_BOARD: // ֪ͨ��Դ������Ǳ����ѹ
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[0]);            // ֪ͨ��Դ�����DUT�����ѹ
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(0, "Voltage");
            Display_Centered(1, "Adjusting");

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          5000,
                          STATE_SwitchStep,
                          STEP_CALIBRATION_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // һ��У׼
        case STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION: // �������֪ͨ�Ǳ���е�ѹУ׼
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Once");
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[0]);
            break;

        case STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_POWER_BOARD: // ֪ͨ��Դ������Ǳ����ѹ
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[1]);            // ֪ͨ��Դ�����DUT�����ѹ
            break;

        // ����У׼
        case STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION: // �������֪ͨ�Ǳ���е�ѹУ׼
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Seconnd");
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[1]);
            break;

        case STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_POWER_BOARD: // ֪ͨ��Դ������Ǳ����ѹ
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[2]);            // ֪ͨ��Դ�����DUT�����ѹ
            break;

        // ����У׼
        case STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION: // �������֪ͨ�Ǳ���е�ѹУ׼
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Third");
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[2]);
            break;

        // ��ԭ�Ǳ����ѹΪ24V
        case STEP_CALIBRATION_UART_TEST_RESTORE_VOLTAGE_CALIBRATION:
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Restore Voltage");
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, 0x18);
            break;

        // У׼���
        case STEP_CALIBRATION_UART_TEST_COMPLETE: // ������ɣ�Ȼ���ϱ����
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, TRUE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            cali_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "CALIBRATION");
            Display_Centered(1, "Test Pass");
            STATE_EnterState(STATE_STANDBY);
            break;

        // ͨ�ų�ʱ
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

// CAN���Բ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��Ʋ���
void STEP_Process_HeadLightCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        // ��ڲ���
        case STEP_HEADLIGHT_CAN_TEST_ENTRY:
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_NOTIFY_DUT);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 
            //        CAN_PROTOCOL1_Init();
            //        can_light_cnt = 0;

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
            break;

        case STEP_HEADLIGHT_CAN_TEST_NOTIFY_DUT:   // ֪ͨDUT�򿪴�Ʋ���
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE);
            //        CAN_PROTOCOL1_SendCmdWithResult(CAN_PROTOCOL1_CMD_HEADLIGHT_CONTROL, 1);
            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_HEADLIGHT_CAN_TEST_OPEN:      // �յ��Ǳ�򿪴���ź�
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��GND��������
            //        LED_GND_TEST_EN_ON();

            // ֪ͨ���ñ��GND�迹ֵ
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
            break;

        case STEP_HEADLIGHT_CAN_TEST_CHECK_GND_VALUE: // ���ӵ�ֵ����
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // GND��ͨ����ͨ�������VCC
            if (measure.result == 0)
            {
                // ��VCC��������
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

        case STEP_HEADLIGHT_CAN_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
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

        case STEP_HEADLIGHT_CAN_TEST_CHECK_CURRENT_VALUE: // ������ֵ����
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��Ʋ���,�������
            if ((measure.result > dut_info.currentMin) && (measure.result < dut_info.currentMax))
            {
                // ����ͨ���رմ��
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

        case STEP_HEADLIGHT_CAN_TEST_COMPLETE: // ������ɲ���
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);

            // �رմ�Ʋ����ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            break;

        // ��ʱ����
        case STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT:
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// ����Ʋ���
void STEP_Process_LbeamCanTest(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    case STEP_LBEAM_CAN_TEST_ENTRY: // ��ڲ���
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_NOTIFY_DUT);
        CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 
        //        CAN_PROTOCOL1_Init();
        light_cnt = 0;
        headlight_cnt = 0;
        Clear_All_Lines();
        Display_Centered(0, "LBEAM");
        Display_Centered(1, "Testing");

        // �������Ÿ�λ
        AVO_PIN_Reset();
        break;

    case STEP_LBEAM_CAN_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
        break;

    case STEP_LBEAM_CAN_TEST_NOTIFY_DUT: // ֪ͨDUT�򿪽���Ʋ���
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_WAIT_RESPONSE);
        DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 2, 1);

        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_LBEAM_CAN_TEST_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_LBEAM_CAN_TEST_OPEN:      // �Ǳ�򿪽���Ƴɹ�
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // ��VCC��������
        LED_LBEAM_TEST_EN_ON();

        // ֪ͨ���ñ��VCC
        AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
        Clear_All_Lines();
        Display_Centered(0, "LBEAM");
        Display_Centered(1, "AVO METER");
        Display_Centered(2, "Measurement");
        break;

    case STEP_LBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // �ȶ����ñ��ص���ֵ
        if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
        {
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "VCC");
            Display_Centered(2, "Test Pass");

            // ��GND��������
            LED_LBEAM_TEST_EN_ON();
            LED_LBEAM_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "LBEAM");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
        }
        // �ȶ�ʧ��
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

    case STEP_LBEAM_CAN_TEST_CHECK_GND_VALUE: // �ȶ�GND
        STATE_SwitchStep(STEP_LBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // С��3V��GND��ͨ����ͨ��
        if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
        {
            // �رս����
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

    case STEP_LBEAM_CAN_TEST_COMPLETE: // �������
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        Clear_All_Lines();
        Display_Centered(0, "LBEAM");
        Display_Centered(1, "Test Pass");

        // �ϱ����Գɹ�
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
        AVO_PIN_Reset();
        STATE_EnterState(STATE_STANDBY);
        break;

    // ��ʱ����
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

// Զ��Ʋ���
void STEP_Process_HbeamCanTest(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    case STEP_HBEAM_CAN_TEST_ENTRY: // ��ڲ���
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_NOTIFY_DUT);
        IAP_CTRL_CAN_Init(dut_info.can_baudRate);
        CAN_PROTOCOL1_Init();
        light_cnt = 0;
        headlight_cnt = 0;
        Clear_All_Lines();
        Display_Centered(0, "HBEAM");
        Display_Centered(1, "Testing");

        // �������Ÿ�λ
        AVO_PIN_Reset();
        break;

    case STEP_HBEAM_CAN_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
        break;

    case STEP_HBEAM_CAN_TEST_NOTIFY_DUT: // ֪ͨDUT��Զ��Ʋ���
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_WAIT_RESPONSE);
        DUT_PROTOCOL_SendCmdTwoParamAck(DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL, 3, 1);

        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_HBEAM_CAN_TEST_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_HBEAM_CAN_TEST_OPEN:      // �Ǳ��Զ��Ƴɹ�
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // ��VCC��������
        LED_HBEAM_TEST_EN_ON();

        // ֪ͨ���ñ��VCC
        AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
        Clear_All_Lines();
        Display_Centered(0, "HBEAM");
        Display_Centered(1, "AVO METER");
        Display_Centered(2, "Measurement");
        break;

    case STEP_HBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // �ȶ����ñ��ص���ֵ
        if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
        {
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "VCC");
            Display_Centered(2, "Test Pass");

            // ��GND��������
            LED_HBEAM_TEST_EN_ON();
            LED_HBEAM_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "HBEAM");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
        }
        // �ȶ�ʧ��
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

    case STEP_HBEAM_CAN_TEST_CHECK_GND_VALUE: // �ȶ�GND
        STATE_SwitchStep(STEP_HBEAM_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // С��3V��GND��ͨ����ͨ��
        if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
        {
            // �ر�Զ���
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

    case STEP_HBEAM_CAN_TEST_COMPLETE: // �������
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
        Clear_All_Lines();
        Display_Centered(0, "HBEAM");
        Display_Centered(1, "Test Pass");

        // �ϱ����Գɹ�
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
        AVO_PIN_Reset();
        STATE_EnterState(STATE_STANDBY);
        break;

    // ��ʱ����
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

// ��ת��Ʋ���
void STEP_Process_Left_TurnSignalCanTest(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_ENTRY: // ��ڲ���
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT);
        IAP_CTRL_CAN_Init(dut_info.can_baudRate);
        CAN_PROTOCOL1_Init();
        //        can_left_light_cnt = 0;

        // �������Ÿ�λ
        AVO_PIN_Reset();
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT:    // ֪ͨDUT����ת���
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        //        CAN_PROTOCOL1_SendCmdWith_TwoResult(CAN_PROTOCOL1_CMD_TURN_SIGNAL_CONTROL, 0, 1);

        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_OPEN:           // ����ת��Ƴɹ�
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // ��GND��������
        //        TURN_GND_TEST_EN_ON();

        //        // ֪ͨ���ñ��GND�迹ֵ
        //        AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
        break;

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_GND_VALUE: // ���GND
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // GND��ͨ����ͨ�������VCC
        if (measure.result == 0)
        {
            // ��VCC��������
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

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
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

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_CURRENT_VALUE: // ������ֵ����
        STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // ��ת��Ʋ���,�������
        if ((measure.result > dut_info.currentMin) && (measure.result < dut_info.currentMax))
        {
            // ����ͨ���ر���ת���
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

    case STEP_LEFT_TURN_SIGNAL_CAN_TEST_COMPLETE: // ������ɲ���
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);

        // �ر���ת��Ʋ����ϱ����Գɹ�
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
        AVO_PIN_Reset();
        break;

    // ��ʱ����
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

// ��ת��Ʋ���
void STEP_Process_Right_TurnSignalCanTest(void)
{
    switch (stateCB.step)
    {
    case STEP_NULL: // �ղ���
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_ENTRY: // ��ڲ���
        STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_NOTIFY_DUT);
        IAP_CTRL_CAN_Init(dut_info.can_baudRate);
        CAN_PROTOCOL1_Init();
        //        can_right_light_cnt = 0;
        Display_Centered(0, "Testing");

        // �������Ÿ�λ
        AVO_PIN_Reset();
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT:    // ֪ͨDUT����ת���
        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        //        CAN_PROTOCOL1_SendCmdWith_TwoResult(CAN_PROTOCOL1_CMD_TURN_SIGNAL_CONTROL, 1, 1);

        // ��ӳ�ʱ
        TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                      4000,
                      STATE_SwitchStep,
                      STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT,
                      1,
                      ACTION_MODE_ADD_TO_QUEUE);
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_OPEN:         // ��ת��ƴ򿪳ɹ�
        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        //        // ��GND��������
        //        TURN_GND_TEST_EN_ON();

        //        // ֪ͨ���ñ��GND�迹ֵ
        //        AVO_PROTOCOL_Send_Type_Chl(MEASURE_OHM, 0);
        break;

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_GND_VALUE: // ���ӵ�ֵ����
        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // GND��ͨ����ͨ�������VCC
        if (measure.result == 0)
        {
            // ��VCC��������
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

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
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

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_CURRENT_VALUE: // ������ֵ����
        STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE);
        TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

        // ��ת��Ʋ���,�������
        if ((measure.result > dut_info.currentMin) && (measure.result < dut_info.currentMax))
        {
            // ����ͨ���ر���ת���
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

    case STEP_RIGHT_TURN_SIGNAL_CAN_TEST_COMPLETE: // ������ɲ���
        TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);

        // �ر���ת��Ƴɹ������ϱ����Գɹ�
        STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
        AVO_PIN_Reset();
        STATE_EnterState(STATE_STANDBY);
        break;

    // ��ʱ����
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

// ���Ų���
void STEP_Process_ThrottleCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_THROTTLE_CAN_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_VCC);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 

            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�����ñ��Ӧ
            break;

        case STEP_THROTTLE_CAN_TEST_VCC: // ֪ͨ���ñ��VCC
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE);

            // ��VCC��������
            THROTTLE_VCC_TEST_EN(TRUE);

            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_THROTTLE_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VCC
        case STEP_THROTTLE_CAN_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // ����DAC���,��λmv
                DAC0_output(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // ��ȡ����ADֵ
                CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 1);
            }
            // VCC�����������ϱ�
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

        // ��������ж���ֵ�Ƿ���������һ��
        case STEP_THROTTLE_CAN_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC1��ֵ����߼�
            if ((dut_info.throttleAd > FIRST_DAC - ERROR_DAC) && (dut_info.throttleAd < FIRST_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                DAC0_output(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ��������ڶ���
        case STEP_THROTTLE_CAN_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.throttleAd > SECOND_DAC - ERROR_DAC) && (dut_info.throttleAd < SECOND_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                DAC0_output(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ�������������
        case STEP_THROTTLE_CAN_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.throttleAd > THIRD_DAC - ERROR_DAC) && (dut_info.throttleAd < THIRD_DAC + ERROR_DAC))
            {
                // 3�ζ����������ͨ��
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

        // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
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

        // ͨ�ų�ʱ���������Ų��Ժ���������case����Ĳ���
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

// ɲ�Ѳ���
void STEP_Process_BrakeCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_BRAKE_CAN_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_VCC);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_BRAKE_CAN_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�����ñ��Ӧ
            break;

        case STEP_BRAKE_CAN_TEST_VCC: // ֪ͨ���ñ��VCC
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_WAIT_RESPONSE);

            // ��VCC��������
            BRAKE_VCC_TEST_EN_ON();

            // ֪ͨ���ñ���ѹ
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_BRAKE_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VCC
        case STEP_BRAKE_CAN_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // ����DAC���,��λmv
                DAC1_output(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // ��ȡ����ADֵ
                CAN_PROTOCOL_TEST_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 2);
            }
            // VCC�����������ϱ�
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

        // ��������ж���ֵ�Ƿ���������һ��
        case STEP_BRAKE_CAN_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC1��ֵ����߼�
            if ((dut_info.brakeAd > FIRST_DAC - ERROR_DAC) && (dut_info.brakeAd < FIRST_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                DAC1_output(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ��������ڶ���
        case STEP_BRAKE_CAN_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.brakeAd > SECOND_DAC - ERROR_DAC) && (dut_info.brakeAd < SECOND_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                DAC1_output(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ�������������
        case STEP_BRAKE_CAN_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.brakeAd > THIRD_DAC - ERROR_DAC) && (dut_info.brakeAd < THIRD_DAC + ERROR_DAC))
            {
                // 3�ζ����������ͨ��
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

        // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
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

        // ͨ�ų�ʱ���������Ų��Ժ���������case����Ĳ���
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

// ���ӱ��ٲ���
void STEP_Process_DerailleurCanTest(void)   
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_DERAILLEUR_CAN_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_VCC);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate); 
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�����ñ��Ӧ
            break;

        case STEP_DERAILLEUR_CAN_TEST_VCC: // ֪ͨ���ñ��VCC
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE);

            // ��VCC��������
            DERAILLEUR_VCC_TEST_EN_ON();

            // ֪ͨ���ñ���ѹ
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "DERAILLEUR");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_DERAILLEUR_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VCC
        case STEP_DERAILLEUR_CAN_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // ����DAC���,��λmv
                PWM_OUT1_PWM(FIRST_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // ��ȡ���ӱ���ADֵ
                DUT_PROTOCOL_SendCmdParamAck(CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD, 3);
            }
            // VCC�����������ϱ�
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

        // ��������ж���ֵ�Ƿ���������һ��
        case STEP_DERAILLEUR_CAN_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC1��ֵ����߼�
            if ((dut_info.derailleurAd > FIRST_DAC - ERROR_DAC) && (dut_info.derailleurAd < FIRST_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                PWM_OUT1_PWM(SECOND_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // ��ȡ���ӱ���ADֵ
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

        // ��������ж���ֵ�Ƿ��������ڶ���
        case STEP_DERAILLEUR_CAN_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.derailleurAd > SECOND_DAC - ERROR_DAC) && (dut_info.derailleurAd < SECOND_DAC + ERROR_DAC))
            {
                // ����DAC���,��λmv
                PWM_OUT1_PWM(THIRD_DAC);
                Delayms(300);
                Clear_All_Lines();
                Display_Centered(0, "DERAILLEUR");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // ��ȡ���ӱ���ADֵ
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

        // ��������ж���ֵ�Ƿ�������������
        case STEP_DERAILLEUR_CAN_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((dut_info.derailleurAd > THIRD_DAC - ERROR_DAC) && (dut_info.derailleurAd < THIRD_DAC + ERROR_DAC))
            {
                // 3�ζ����������ͨ��
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

        // ���ӱ��ٲ�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
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

        // ͨ�ų�ʱ
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

// VLK����
void STEP_Process_VlkCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        // ��ڲ���
        case STEP_VLK_CAN_TEST_ENTRY:
            STATE_SwitchStep(STEP_VLK_CAN_TEST_VLK);
            CAN_PROTOCOL_TEST_Init(dut_info.can_baudRate);

            // LCD��ʾ
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 0);

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        // �ȴ��Ǳ�����ñ��Ӧ
        case STEP_VLK_CAN_TEST_WAIT_RESPONSE:
            break;

        // ֪ͨ���ñ��VLK
        case STEP_VLK_CAN_TEST_VLK:
            STATE_SwitchStep(STEP_VLK_CAN_TEST_WAIT_RESPONSE);

            // ʹ��VLK��������
            DUT_VLK_TEST_EN_ON();

            // ֪ͨ���ñ��VLK
//            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

            // LCD��ʾ
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 1);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_VLK_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VLKֵ
        case STEP_VLK_CAN_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_VLK_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                STATE_SwitchStep(STEP_VLK_CAN_TEST_COMPLETE);
            }
            // VCC�����������ϱ�
            else
            {
                STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);

                // LCD��ʾ
                Vertical_Scrolling_Display(vlkMsgBuff, 4, 2);
            }
            break;

        case STEP_VLK_CAN_TEST_COMPLETE: // ������ɣ�Ȼ���ϱ����
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);

            // LCD��ʾ
            Vertical_Scrolling_Display(vlkMsgBuff, 4, 3);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        case STEP_VLK_CAN_TEST_COMMUNICATION_TIME_OUT: // ͨ�ų�ʱ
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

// ��ѹУ׼
void STEP_Process_CalibrationCanTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_CALIBRATION_CAN_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_ENTRY);

            break;

        case STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�/��Դ���Ӧ
            break;

        case STEP_CALIBRATION_CAN_TEST_NOTIFY_POWER_BOARD: // ֪ͨ��Դ������Ǳ����ѹ
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[0]);            // ֪ͨ��Դ�����DUT�����ѹ

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          3000,
                          STATE_SwitchStep,
                          STEP_CALIBRATION_CAN_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);

            break;

        // һ��У׼
        case STEP_CALIBRATION_CAN_TEST_NOTIFY_VOLTAGE_CALIBRATION: // �������֪ͨ�Ǳ���е�ѹУ׼
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[0]);
            break;

        case STEP_CALIBRATION_CAN_TEST_TWO_NOTIFY_POWER_BOARD: // ֪ͨ��Դ������Ǳ����ѹ
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[1]);            // ֪ͨ��Դ�����DUT�����ѹ

            break;

        // ����У׼
        case STEP_CALIBRATION_CAN_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION: // �������֪ͨ�Ǳ���е�ѹУ׼
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[1]);
            break;

        case STEP_CALIBRATION_CAN_TEST_THREE_NOTIFY_POWER_BOARD: // ֪ͨ��Դ������Ǳ����ѹ
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE, calibration.data[2]);            // ֪ͨ��Դ�����DUT�����ѹ

            break;

        // ����У׼
        case STEP_CALIBRATION_CAN_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION: // �������֪ͨ�Ǳ���е�ѹУ׼
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            DUT_PROTOCOL_SendCmdParamAck(DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION, calibration.data[2]);
            break;

        // У׼���
        case STEP_CALIBRATION_CAN_TEST_COMPLETE: // ������ɣ�Ȼ���ϱ����
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, TRUE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STATE_EnterState(STATE_STANDBY);
            break;

        // ͨ�ų�ʱ
        case STEP_CALIBRATION_CAN_TEST_COMMUNICATION_TIME_OUT:
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION, FALSE);
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}
/****************************************����********************************************/
/******����ȫ�ֱ�������*************/

// ��ӳ�ʱ��ⶨʱ��
void addTimeOutTimer(uint32 time)
{
    TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                  time,
                  STATE_SwitchStep,
                  STEP_CM_CAN_UPGRADE_COMMUNICATION_TIME_OUT,
                  1,
                  ACTION_MODE_ADD_TO_QUEUE);
}

// ͨ�ò�����������
void STEP_Process_CmUartUpgrade()
{
    uint8 i = 0;

    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        // �������
        case STEP_CM_UART_UPGRADE_ENTRY:
            TIMER_KillTask(TIMER_ID_ONLINE_DETECT);
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);                            // ֹͣ����config��ʱ��
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // ���ò�����
            UART_PROTOCOL_Init();                        // 55����Э��
            UART_PROTOCOL4_Init();                       // KM5S
            resetFlag = TRUE;
            switch_cnt = 0;

            // vlk����
            VLK_PW_EN_OFF();

            // �ر��Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // �ȴ���Դ��/dutӦ��
        case STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK:
            // ֻ���ȴ�����������
            break;

        // �Ǳ�����
        case STEP_CM_UART_BC_UPGRADE_RESTART:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);                            // ֹͣ����config��ʱ��

            // vlk����
            VLK_PW_EN_ON();

            // ���Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
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

            // �ر�vlk����
            VLK_PW_EN_OFF();

            // �ر��Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // ����config���
        case STEP_UART_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // ������
            UART_DRIVE_InitSelect(9600);     // UARTЭ����ʼ��
            UART_PROTOCOL_Init();            // 55����Э��
            UART_PROTOCOL4_Init();           // KM5S
            configResetFlag = TRUE;
            switch_cnt = 0;

            // �ر�vlk����
            VLK_PW_EN_OFF();

            // �ر��Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // �Ǳ���
        case STEP_UART_CONFIG_RESET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            // vlk����
            VLK_PW_EN_ON();

            // �����Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
            break;

        // Э���л�
        case STEP_UART_PROTOCOL_SWITCCH:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            // ���Ǳ�﮵�2���л�ΪKM5SЭ��
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);               // �������ͷ
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // ����豸��ַ
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_PROTOCOL_SWITCCH);   // 0xc0 ���������
            UART_PROTOCOL4_TxAddData(3);                                     // ������ݳ���
            UART_PROTOCOL4_TxAddData(1);
            UART_PROTOCOL4_TxAddData(1);
            UART_PROTOCOL4_TxAddData(1);

            // ��Ӽ������������������������
            UART_PROTOCOL4_TxAddFrame();
            break;

        // ����config����
        case STEP_UART_SEND_SET_CONFIG:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            /*����dut������Ϣ*/
            // ���ھ�Э��̶�������
            if ((configs[0] - 84 > 54) || (configs[0] - 84 < 54))
            {
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);                    // �������ͷ
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);              // ����豸��ַ
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_NEW_WRITE_CONTROL_PARAM); // 0x50 ���������
                UART_PROTOCOL4_TxAddData(configs[0]);                                 // ������ݳ���
                for (i = 1; i < configs[0] + 1; i++)                                  // configֻ��54���ֽ�
                {
                    UART_PROTOCOL4_TxAddData(configs[i]);
                }
                // ��Ӽ������������������������
                UART_PROTOCOL4_TxAddFrame();
            }

            // ��������Ϊ54���ֽ�
            if ((configs[0] - 84) == 54)
            {
                // ��Э��
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);                       // �������ͷ
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);                 // ����豸��ַ
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_NEW_WRITE_CONTROL_PARAM);    // 0x50 ���������
                UART_PROTOCOL4_TxAddData(configs[0]);                                    // ������ݳ���
                for (i = 1; i < configs[0] + 1; i++)                                     // configֻ��54���ֽ�
                {
                    UART_PROTOCOL4_TxAddData(configs[i]);
                }
                // ��Ӽ������������������������
                UART_PROTOCOL4_TxAddFrame();

                // ��Э��
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);                // �������ͷ
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);          // ����豸��ַ
                UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_WRITE_CONTROL_PARAM); // 0xc0 ���������
                UART_PROTOCOL4_TxAddData(54);                                     // ������ݳ���
                for (i = 0; i < 54; i++)                                          // configֻ��54���ֽ�
                {
                    UART_PROTOCOL4_TxAddData(resultArray[i]);
                }
                // ��Ӽ������������������������
                UART_PROTOCOL4_TxAddFrame();
            }
            break;

        // ��ȡ���ò���
        case STEP_UART_READ_CONFIG:
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);                              // ֹͣ����config��ʱ��
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);                    // �������ͷ
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);              // ����豸��ַ
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_READ_CONTROL_PARAM);      // 0xc2���������
            UART_PROTOCOL4_TxAddData(0);                                          // ������ݳ���

            // ��Ӽ������������������������
            UART_PROTOCOL4_TxAddFrame();
            break;

        // config���óɹ�
        case STEP_UART_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            // �ر�vlk����
            VLK_PW_EN_OFF();

            dut_info.configUpFlag = FALSE; // ��־λ��false
            dut_info.configUpSuccesss = TRUE;
            dut_info.dutPowerOnAllow = FALSE; // ������
            break;

        // ���������
        case STEP_CM_UART_BC_UPGRADE_ITEM_FINISH:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            if (dut_info.configUpFlag)
            {
                STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
            }
            else
            {
                // û��������
                STATE_EnterState(STATE_STANDBY);

                // �ر�vlk����
                VLK_PW_EN_OFF();
            }
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
            TIMER_KillTask(TIMER_ID_ONLINE_DETECT);
            STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
            UART_DRIVE_InitSelect(UART_DRIVE_BAUD_RATE); // UARTЭ����ʼ��
            UART_PROTOCOL_Init();                        // 55����Э��
            UART_PROTOCOL4_Init();                       // KM5S
            resetFlag = TRUE;
            switch_cnt = 0;

            // vlk����
            VLK_PW_EN_OFF();

            // �ر��Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // �ȴ�
        case STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK:
            break;

        // �Ǳ�����
        case STEP_CM_UART_SEG_UPGRADE_RESTART:

            // vlk����
            VLK_PW_EN_ON();

            // ���Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
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

        // app�����ɹ�����
        case STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS_RESET:

            // �ر��Ǳ����Դ
            switch_cnt = 0;
            appResetFlag = TRUE;

            // vlk����
            VLK_PW_EN_OFF();
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // ����config���
        case STEP_UART_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
            UART_DRIVE_InitSelect(9600); // UARTЭ����ʼ��
            UART_PROTOCOL_Init();        // 55����Э��
            UART_PROTOCOL4_Init();       // KM5S
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
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);       // ����Э���л�����
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR); // ����豸��ַ
            UART_PROTOCOL4_TxAddData(0xAB);                          // 0xAB��Э���л�
            UART_PROTOCOL4_TxAddData(0);                             // ������ݳ���
            UART_PROTOCOL4_TxAddData(01);
            UART_PROTOCOL4_TxAddFrame(); // ��Ӽ������������������������

            /*����dut������Ϣ*/
            SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60);  // ʵ��ֻ��54���ֽ�
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL4_CMD_HEAD);               // �������ͷ
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);         // ����豸��ַ
            UART_PROTOCOL4_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0 ���������
            UART_PROTOCOL4_TxAddData(54);                                    // ������ݳ���
            // UART_PROTOCOL4_TxAddData(1);//��������1��������
            for (i = 0; i < 54; i++)
            {
                UART_PROTOCOL4_TxAddData(configs[i]); // д����
            }
            UART_PROTOCOL4_TxAddFrame(); // ��Ӽ������������������������
            break;

        // ����config���
        case STEP_UART_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK);
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ��ʱ��
            UART_DRIVE_InitSelect(115200);           // ���ò�����
            dut_info.configUpFlag = FALSE;           // ��־λ��false
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

// ͨ��CAN����,APP����Ϊhex
void STEP_Process_CmCanUpgrade(void)
{
    uint8 configs[60] = {0};
    uint8 i;

    switch (stateCB.step)
    {
        // �ղ���
        case STEP_NULL:
            break;

        // ͨ��can������ڣ�dut�ϵ��can��ʼ��
        case STEP_CM_CAN_UPGRADE_ENTRY:
            TIMER_KillTask(TIMER_ID_ONLINE_DETECT);
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.can = CAN_FF_STANDARD;
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);
            resetFlag = TRUE;
            switch_cnt = 0;

            // vlk����
            VLK_PW_EN_OFF();

            // �ر��Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            dut_info.reconnectionRepeatOne = FALSE;
            break;

        // �ȴ���Դ��/dutӦ��
        case STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK:
            break;

        // �Ǳ�����
        case STEP_CM_CAN_UPGRADE_RESTART:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            // vlk����
            VLK_PW_EN_ON();

            // ���Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
            break;

        // �յ�dut��eco����,��ʱ100ms����eco����
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
            dut_info.dutPowerOnAllow = FALSE;
            TIMER_KillTask(TIMER_ID_RECONNECTION);
            break;

        // app�����ɹ�����
        case STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_RESET:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            // �ر��Ǳ����Դ
            switch_cnt = 0;
            appResetFlag = TRUE;

            // vlk����
            VLK_PW_EN_OFF();
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // �����ɹ�������ʾ��ɫ
        case STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_YELLOW:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.can = CAN_EXTENDED;
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            CAN_PROTOCOL1_Init(); // ͨ�����ó�ʼ��

            TIMER_AddTask(TIMER_ID_CHANGE_LCD_COLOR,
                          500,
                          CAN_PROTOCOL1_SendCmdChangeColor,
                          0,
                          -1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // ����config���
        case STEP_CAN_SET_CONFIG_ENTRY:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            dut_info.dutPowerOnAllow = TRUE; // ������
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_500K);
            CAN_PROTOCOL1_Init();  // ͨ�����ó�ʼ��
            addTimeOutTimer(5000); // ����configʱ�䣺5s
            configResetFlag = TRUE;
            switch_cnt = 0;

            // �ر�vlk����
            VLK_PW_EN_OFF();

            // �ر��Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, FALSE);
            break;

        // �Ǳ���
        case STEP_CAN_CONFIG_RESET:
            STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK);

            // vlk����
            VLK_PW_EN_ON();

            // �����Ǳ����Դ
            POWER_PROTOCOL_SendCmdParamAck(POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH, TRUE);
            break;

        // ����config����
        case STEP_CAN_SEND_SET_CONFIG:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            /*����dut������Ϣ*/
            // ���ھ�Э��̶�������
            if ((configs[0] - 84 > 54) || (configs[0] - 84 < 54))
            {
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                     // �������ͷ
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);              // ����豸��ַ
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_NEW_WRITE_CONTROL_PARAM);  // 0x50 ���������
                CAN_PROTOCOL1_TxAddData(configs[0]);                                 // ������ݳ���
                for (i = 1; i < configs[0] + 1; i++)                                 // configֻ��54���ֽ�
                {
                    CAN_PROTOCOL1_TxAddData(configs[i]);
                }
                // ��Ӽ������������������������
                CAN_PROTOCOL1_TxAddFrame();
            }

            // ��������Ϊ54���ֽ�
            if ((configs[0] - 84) == 54)
            {
                // ��Э��
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                        // �������ͷ
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);                 // ����豸��ַ
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_NEW_WRITE_CONTROL_PARAM);     // 0x50 ���������
                CAN_PROTOCOL1_TxAddData(configs[0]);                                    // ������ݳ���
                for (i = 1; i < configs[0] + 1; i++)                                    // configֻ��54���ֽ�
                {
                    CAN_PROTOCOL1_TxAddData(configs[i]);
                }
                // ��Ӽ������������������������
                CAN_PROTOCOL1_TxAddFrame();

                // ��Э��
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                 // �������ͷ
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);          // ����豸��ַ
                CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM);  // 0xc0 ���������
                CAN_PROTOCOL1_TxAddData(54);                                     // ������ݳ���
                for (i = 0; i < 54; i++)                                         // configֻ��54���ֽ�
                {
                    CAN_PROTOCOL1_TxAddData(resultArray[i]);
                }
                // ��Ӽ������������������������
                CAN_PROTOCOL1_TxAddFrame();
            }
            break;

        // ��ȡ���ò���
        case STEP_CAN_READ_CONFIG:
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG);                             // ֹͣ����config��ʱ��
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_HEAD);                     // �������ͷ
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_DEVICE_ADDR);              // ����豸��ַ
            CAN_PROTOCOL1_TxAddData(CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM);       // 0xc2���������
            CAN_PROTOCOL1_TxAddData(0);                                          // ������ݳ���

            // ��Ӽ������������������������
            CAN_PROTOCOL1_TxAddFrame();
            break;

        // config���óɹ�
        case STEP_CAN_SET_CONFIG_SUCCESS:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);
            TIMER_KillTask(TIMER_ID_SET_DUT_CONFIG); // ֹͣ����config��ʱ��
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);    // ֹͣ��ʱ��ʱ��
            IAP_CTRL_CAN_Init(CAN_BAUD_RATE_125K);   // ���ò�����
            dut_info.configUpFlag = FALSE; // ��־λ��false
            dut_info.configUpSuccesss = TRUE;
            dut_info.dutPowerOnAllow = FALSE;
            break;

        // �����������ɹ�
        case STEP_CM_CAN_UPGRADE_ITEM_FINISH:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK);

            // ѡ������
            if (dut_info.configUpFlag)
            {
                STATE_SwitchStep(STEP_CAN_SET_CONFIG_ENTRY);
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
            else if (dut_info.configUpFlag)
            {
                dut_info.configUpFaile = TRUE; // config����ʧ��
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

            STATE_EnterState(STATE_STANDBY);
            break;

        default:
            break;
    }
}

/****************************************С�ײ���********************************************/

// ��Ʋ���
void STEP_Process_XM_HeadLightUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_HEADLIGHT_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "XM HEADLIGHT");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
            break;

        case STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT: // ֪ͨDUT�򿪴�Ʋ���
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_LIGHT, 1);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_HEADLIGHT_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_HEADLIGHT_UART_TEST_OPEN:      // �Ǳ�򿪴�Ƴɹ�
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��VCC��������
            LED_VCC_TEST_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "XM HEADLIGHT");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_HEADLIGHT_UART_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // �ȶ����ñ��ص���ֵ
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "XM HEADLIGHT");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // ��GND��������
                LED_VCC_TEST_EN_ON();
                LED_VCC_EN_ON();

                // ֪ͨ���ñ��VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "XM HEADLIGHT");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // �ȶ�ʧ��
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

        case STEP_HEADLIGHT_UART_TEST_CHECK_GND_VALUE: // �ȶ�GND
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // С��3V��GND��ͨ����ͨ��
            if (LIGHT_GND_VOLTAGE_VALUE > measure.result)
            {
                // �رմ��
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

        case STEP_HEADLIGHT_UART_TEST_COMPLETE: // �������
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "XM HEADLIGHT");
            Display_Centered(1, "Test Pass");

            // �ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // ��ʱ����
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


// ���Ų���
void STEP_Process_XM_ThrottleUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_THROTTLE_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_THROTTLE_UART_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�����ñ��Ӧ
            break;

        case STEP_THROTTLE_UART_TEST_VCC: // ֪ͨ���ñ��VCC
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);

            // ��VCC��������
            THROTTLE_VCC_TEST_EN(TRUE);

            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "THROTTLE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_THROTTLE_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VCC
        case STEP_THROTTLE_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // ����DAC���,��λmv
                DAC0_output(1200);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // ��ȡ����ADֵ
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_TEST_Throttle_Break);
            }
            // VCC�����������ϱ�
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

        // ��������ж���ֵ�Ƿ���������һ��
        case STEP_THROTTLE_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC1��ֵ����߼�
            if (0 == dut_info.throttleAd )
            {
                // ����DAC���,��λmv
                DAC0_output(2100);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ��������ڶ���
        case STEP_THROTTLE_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((2 == dut_info.throttleAd )||(3 == dut_info.throttleAd))
            {
                // ����DAC���,��λmv
                DAC0_output(4200);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "THROTTLE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ�������������
        case STEP_THROTTLE_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if (6 == dut_info.throttleAd )
            {
                // 3�ζ����������ͨ��
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

        // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
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

        // ͨ�ų�ʱ���������Ų��Ժ���������case����Ĳ���
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

// ɲ�Ѳ���
void STEP_Process_XM_BrakeUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_BRAKE_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_VCC);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_BRAKE_UART_TEST_WAIT_RESPONSE: // �ȴ��Ǳ�����ñ��Ӧ
            break;

        case STEP_BRAKE_UART_TEST_VCC: // ֪ͨ���ñ��VCC
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);

            // ��VCC��������
            BRAKE_VCC_TEST_EN_ON();

            // ֪ͨ���ñ���ѹ
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "BRAKE");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_BRAKE_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        // �ȶ�VCC
        case STEP_BRAKE_UART_TEST_CHECK_VOLTAGE_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                // ����DAC���,��λmv
                DAC1_output(1200);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Normal");

                // ��ȡ����ADֵ
                UART_PROTOCOL_XM_SendCmdAck(UART_PROTOCOL_XM_CMD_TEST_Throttle_Break);
            }
            // VCC�����������ϱ�
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

        // ��������ж���ֵ�Ƿ���������һ��
        case STEP_BRAKE_UART_TEST_CHECK_DAC1_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC1��ֵ����߼�
            if (0 == dut_info.brakeAd)
            {
                // ����DAC���,��λmv
                DAC1_output(2100);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "First");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ��������ڶ���
        case STEP_BRAKE_UART_TEST_CHECK_DAC3_VALUE:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if ((2 == dut_info.brakeAd)||(3 == dut_info.brakeAd))
            {
                // ����DAC���,��λmv
                DAC1_output(4200);
                Delayms(50);
                Clear_All_Lines();
                Display_Centered(0, "BRAKE");
                Display_Centered(1, "Adjusting DAC");
                Display_Centered(2, "Second");

                // ��ȡ����ADֵ
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

        // ��������ж���ֵ�Ƿ�������������
        case STEP_BRAKE_UART_TEST_CHECK_DAC5_VALUE:
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ����DAC5��ֵ����߼�
            if (6 == dut_info.brakeAd)
            {
                // 3�ζ����������ͨ��
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

        // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
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

        // ͨ�ų�ʱ���������Ų��Ժ���������case����Ĳ���
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

// ��ת��Ʋ���
void STEP_Process_XM_Left_TurnSignalUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();
            light_cnt = 0;
            headlight_cnt = 0;           
            Clear_All_Lines();
            Display_Centered(0, "XM LEFT SIGNAL");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT:   // ֪ͨDUT����ת���
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT,1);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          4000,
                          STATE_SwitchStep,
                          STEP_LEFT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN:      // �Ǳ����ת��Ƴɹ�
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��VCC��������
            LEFT_VCC_TEST_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            Clear_All_Lines();
            Display_Centered(0, "XM LEFT SIGNAL");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            
            // �ȶ����ñ��ص���ֵ
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "XM LEFT SIGNAL");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");

                // ��GND��������
                LEFT_VCC_TEST_EN_ON();
                LEFT_VCC_EN_ON();
                vccVlaue = measure.result;
                
                // ֪ͨ���ñ��VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
//                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

                Clear_All_Lines();
                Display_Centered(0, "XM LEFT SIGNAL");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // �ȶ�ʧ��
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

        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE: // �ȶ�GND
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

//            // ��¼��һ��GND��ѹֵ
//            gndVlaue1 = measure.result;
//            
//            // ��GND��������
//            LEFT_VCC_TEST_EN_ON();
//            LEFT_VCC_EN_ON();
//            
//            // ֪ͨ���ñ��VCC
//            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
//            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);   
            
            // С��3V��GND��ͨ����ͨ��
            if (3000 > measure.result)
            {
                // �ر���ת���
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

//        case STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2: // �ȶ�GND
//            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
//            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
//            
//            // ��¼�ڶ���GND��ѹֵ
//            gndVlaue2 = measure.result; 
//            
//            result = abs(gndVlaue2 - gndVlaue1);
//            result = abs(gndVlaue2 - vccVlaue);
//            result = abs(gndVlaue1 - gndVlaue1);
//                     
//            if ((abs(gndVlaue2 - gndVlaue1) > 2000) ||  (abs(gndVlaue2 - vccVlaue) > 2000) ||  (abs(gndVlaue1 - vccVlaue) > 2000))
//            {
//                // �ر���ת���
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
            
        case STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE: // �������
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "XM LEFT SIGNAL");
            Display_Centered(1, "Test Pass");

            // �ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // ��ʱ����
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

// ��ת��Ʋ���
void STEP_Process_XM_Right_TurnSignalUartTest(void)
{
    switch (stateCB.step)
    {
        case STEP_NULL: // �ղ���
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY: // ��ڲ���
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT);
            UART_DRIVE_InitSelect(dut_info.uart_baudRate);
            DUT_PROTOCOL_Init();
            UART_PROTOCOL_XM_Init();            
            light_cnt = 0;
            headlight_cnt = 0;
            Clear_All_Lines();
            Display_Centered(0, "XM RIGHT SIGNAL");
            Display_Centered(1, "Testing");

            // �������Ÿ�λ
            AVO_PIN_Reset();
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE: // �ȴ���Ӧ����
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT: // ֪ͨDUT����ת���
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
             UART_PROTOCOL_XM_SendCmdParamAck(UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT, 2);

            // ��ӳ�ʱ
            TIMER_AddTask(TIMER_ID_DUT_TIMEOUT,
                          6000,
                          STATE_SwitchStep,
                          STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN:      // �Ǳ��Զ��Ƴɹ�
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // ��VCC��������
            RIGHT_VCC_TEST_EN_ON();

            // ֪ͨ���ñ��VCC
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);

            Clear_All_Lines();
            Display_Centered(0, "XM RIGHT SIGNAL");
            Display_Centered(1, "AVO METER");
            Display_Centered(2, "Measurement");
            break;

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE: // ����ѹֵ����
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);

            // �ȶ����ñ��ص���ֵ
            if ((measure.result > dut_info.voltageMin) && (measure.result < dut_info.voltageMax))
            {
                Clear_All_Lines();
                Display_Centered(0, "XM RIGHT SIGNAL");
                Display_Centered(1, "VCC");
                Display_Centered(2, "Test Pass");
                vccVlaue = measure.result;
                
                // ��GND��������
                RIGHT_VCC_TEST_EN_ON();
                RIGHT_VCC_EN_ON();
                
                // ֪ͨ���ñ��VCC
                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
                Clear_All_Lines();
                Display_Centered(0, "XM RIGHT SIGNAL");
                Display_Centered(1, "AVO METER");
                Display_Centered(2, "Measurement");
            }
            // �ȶ�ʧ��
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

        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE: // �ȶ�GND
//            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
//            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
//                gndVlaue1 = measure.result;

//                // ��GND��������
//                RIGHT_VCC_TEST_EN_ON();
//                RIGHT_VCC_EN_ON();
//                
//                // ֪ͨ���ñ��VCC
//                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
            // С��3V��GND��ͨ����ͨ��
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_WAIT_RESPONSE);
            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
            
            if (3000 > measure.result)
            {
                // �ر���ת���
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

//        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2: // �ȶ�GND
//            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
//            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
//                gndVlaue2 = measure.result;

//                // ��GND��������
//                RIGHT_VCC_TEST_EN_ON();
//                RIGHT_VCC_EN_ON();
//                
//                // ֪ͨ���ñ��VCC
//                AVO_PROTOCOL_Send_Type_Chl(MEASURE_DCV, 0);
//            break;
//            
//        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE3: // �ȶ�GND
//            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE);
//            TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);
//            gndVlaue3 = measure.result;

//            // С��3V��GND��ͨ����ͨ��                     
//            if ((abs(gndVlaue2 - gndVlaue1) > 2000) || (abs(gndVlaue2 - vccVlaue) > 2000) || (abs(gndVlaue1 - vccVlaue) > 2000)\
//            || (abs(gndVlaue3 - vccVlaue) > 2000) || (abs(gndVlaue3 - gndVlaue1) > 2000) || (abs(gndVlaue3 - gndVlaue2) > 2000))
//            {
//                // �ر���ת���
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
            
        case STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE: // �������
            TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
            Clear_All_Lines();
            Display_Centered(0, "XM RIGHT SIGNAL");
            Display_Centered(1, "Test Pass");

            // �ϱ����Գɹ�
            STS_PROTOCOL_SendCmdParamAck(STS_PROTOCOL_CMD_MULTIPLE_TEST, TRUE);
            AVO_PIN_Reset();
            STATE_EnterState(STATE_STANDBY);
            break;

        // ��ʱ����
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

/****************************************����-END********************************************/
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
            break;

        // GND����
        case STATE_UART_GND_TEST:
            STATE_SwitchStep(STEP_GND_TEST_ENTRY);
            break;

        /******************************************UART***************************************/

        // ��Ʋ���
        case STATE_UART_HEADLIGHT_TEST:
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_ENTRY);
            break;

        // ����Ʋ���
        case STATE_UART_LBEAM_TEST:
            STATE_SwitchStep(STEP_LBEAM_UART_TEST_ENTRY);
            break;

        // Զ��Ʋ���
        case STATE_UART_HBEAM_TEST:
            STATE_SwitchStep(STEP_HBEAM_UART_TEST_ENTRY);
            break;

        // ��ת��Ʋ���
        case STATE_UART_LEFT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY);
            break;

        // ��ת��Ʋ���
        case STATE_UART_RIGHT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY);
            break;

        // ���Ų���
        case STATE_UART_THROTTLE_TEST:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_ENTRY);
            break;

        // ɲ�Ѳ���
        case STATE_UART_BRAKE_TEST:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_ENTRY);
            break;

        // ���ӱ��ٲ���
        case STATE_UART_DERAILLEUR_TEST:
            STATE_SwitchStep(STEP_DERAILLEUR_UART_TEST_ENTRY);
            break;
            
        // vlk����
        case STATE_UART_VLK_TEST:
            STATE_SwitchStep(STEP_VLK_UART_TEST_ENTRY);
            break;

        // ��ѹУ׼����
        case STATE_UART_VOLTAGE_TEST:
            STATE_SwitchStep(STEP_CALIBRATION_UART_TEST_ENTRY);
            break;

        // UART����
        case STATE_UART_TEST:
            STATE_SwitchStep(STEP_UART_TEST_ENTRY);
            break;

        /*****************************CAN***************************************/

        // ��Ʋ���
        case STATE_CAN_HEADLIGHT_TEST:
            STATE_SwitchStep(STEP_HEADLIGHT_CAN_TEST_ENTRY);
            break;

        // ����Ʋ���
        case STATE_CAN_LBEAM_TEST:
            STATE_SwitchStep(STEP_LBEAM_CAN_TEST_ENTRY);
            break;

        // Զ��Ʋ���
        case STATE_CAN_HBEAM_TEST:
            STATE_SwitchStep(STEP_HBEAM_CAN_TEST_ENTRY);
            break;

        // ��ת��Ʋ���
        case STATE_CAN_LEFT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_CAN_TEST_ENTRY);
            break;

        // ��ת��Ʋ���
        case STATE_CAN_RIGHT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_CAN_TEST_ENTRY);
            break;

        // ���Ų���
        case STATE_CAN_THROTTLE_TEST:
            STATE_SwitchStep(STEP_THROTTLE_CAN_TEST_ENTRY);
            break;

        // ɲ������
        case STATE_CAN_BRAKE_TEST:
            STATE_SwitchStep(STEP_BRAKE_CAN_TEST_ENTRY);
            break;

        // ���ӱ��ٲ���
        case STATE_CAN_DERAILLEUR_TEST:
            STATE_SwitchStep(STEP_DERAILLEUR_CAN_TEST_COMMUNICATION_TIME_OUT);
            break;
            
        // vlk����
        case STATE_CAN_VLK_TEST:
            STATE_SwitchStep(STEP_VLK_CAN_TEST_ENTRY);
            break;

        // ��ѹУ׼����
        case STATE_CAN_VOLTAGE_TEST:
            STATE_SwitchStep(STEP_CALIBRATION_CAN_TEST_ENTRY);
            break;

        /*****************************UP***************************************/

        // ͨ�ò�����������
        case STATE_CM_UART_BC_UPGRADE:
            STATE_SwitchStep(STEP_CM_UART_UPGRADE_ENTRY);
            break;

        // ͨ�������/��������������
        case STATE_CM_UART_SEG_UPGRADE:
            STATE_SwitchStep(STEP_SEG_UART_UPGRADE_ENTRY);
            break;

        // ͨ�ò���CAN����
        case STATE_CM_CAN_UPGRADE:
            STATE_SwitchStep(STEP_CM_CAN_UPGRADE_ENTRY);
            break;

        /************************************����ΪС�״��ڲ���***************************************************/

        // ��Ʋ���
        case STATE_UART_XM_HEADLIGHT_TEST:
            STATE_SwitchStep(STEP_HEADLIGHT_UART_TEST_ENTRY);
            break;

        // ���Ų���
        case STATE_UART_XM_THROTTLE_TEST:
            STATE_SwitchStep(STEP_THROTTLE_UART_TEST_ENTRY);
            break;

        // ɲ������
        case STATE_UART_XM_BRAKE_TEST:
            STATE_SwitchStep(STEP_BRAKE_UART_TEST_ENTRY);
            break;

        // ��ת��Ʋ���
        case STATE_UART_XM_LEFT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY);
            break;

        // ��ת��Ʋ���
        case STATE_UART_XM_RIGHT_TURN_SIGNAL_TEST:
            STATE_SwitchStep(STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY);
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

        // UART����
        case STATE_UART_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            UART_Test();
            break;

        // GND����
        case STATE_UART_GND_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            All_GND_Test();
            break;

        /* ����Ϊ���ڲ���  */
        // ��Ʋ���
        case STATE_UART_HEADLIGHT_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_HeadLightUartTest();
            break;

        // ����Ʋ���
        case STATE_UART_LBEAM_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_LbeamUartTest();
            break;

        // Զ��Ʋ���
        case STATE_UART_HBEAM_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_HbeamUartTest();
            break;

        // ��ת��Ʋ���
        case STATE_UART_LEFT_TURN_SIGNAL_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_Left_TurnSignalUartTest();
            break;

        // ��ת��Ʋ���
        case STATE_UART_RIGHT_TURN_SIGNAL_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_Right_TurnSignalUartTest();
            break;

        // ���Ų���
        case STATE_UART_THROTTLE_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_ThrottleUartTest();
            break;

        // ɲ�Ѳ���
        case STATE_UART_BRAKE_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_BrakeUartTest();
            break;

        // ���ӱ��ٲ���
        case STATE_UART_DERAILLEUR_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_DerailleurUartTest();
            break;
            
        // VLK����
        case STATE_UART_VLK_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_VlkUartTest();
            break;

        // ��ѹУ׼����
        case STATE_UART_VOLTAGE_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            DUT_UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_CalibrationUartTest();
            break;

        /* ����ΪCAN����  */
        // ��Ʋ���
        case STATE_CAN_HEADLIGHT_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_HeadLightCanTest();
            break;

        // ����Ʋ���
        case STATE_CAN_LBEAM_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_LbeamCanTest();
            break;

        // Զ��Ʋ���
        case STATE_CAN_HBEAM_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_HbeamCanTest();
            break;

        // ��ת��Ʋ���
        case STATE_CAN_LEFT_TURN_SIGNAL_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_Left_TurnSignalCanTest();
            break;

        // ��ת��Ʋ���
        case STATE_CAN_RIGHT_TURN_SIGNAL_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_Right_TurnSignalCanTest();
            break;

        // ���Ų���
        case STATE_CAN_THROTTLE_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_ThrottleCanTest();
            break;

        // ɲ�Ѳ���
        case STATE_CAN_BRAKE_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_BrakeCanTest();
            break;

        // ���ӱ��ٲ���
        case STATE_CAN_DERAILLEUR_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_DerailleurCanTest();
            break;
            
        // VLK����
        case STATE_CAN_VLK_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_VlkCanTest();
            break;

        // ��ѹУ׼����
        case STATE_CAN_VOLTAGE_TEST:
            CAN_PROTOCOL_Process_Test();
            STEP_Process_CalibrationCanTest();
            break;

        /************************************����ΪCAN����***************************************************/

        // ͨ�ò���can����
        case STATE_CM_CAN_UPGRADE:
            CAN_PROTOCOL_Process_DT();      // ��̫����Э��
            CAN_PROTOCOL_Process_3A();      // ���app�е�3a����Э��
            STEP_Process_CmCanUpgrade();
            break;

        /************************************����Ϊ��������***************************************************/

        // ͨ�ò�����������
        case STATE_CM_UART_BC_UPGRADE:
            UART_DRIVE_Process();               // UART��������̴���
            UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_CmUartUpgrade();   // ����
            break;

        // ����ܡ���������������
        case STATE_CM_UART_SEG_UPGRADE:
            UART_DRIVE_Process();           // UART��������̴���
            UART_PROTOCOL_Process();        // UARTЭ�����̴���
            STEP_Process_SegUartUpgrade();
            break;
        
        /************************************����ΪС�״��ڲ���***************************************************/
        
        // (С��)��Ʋ���
        case STATE_UART_XM_HEADLIGHT_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            UART_PROTOCOL_XM_Process();         // UARTЭ�����̴���
            STEP_Process_XM_HeadLightUartTest();
            break;

        // (С��)��ת��Ʋ���
        case STATE_UART_XM_LEFT_TURN_SIGNAL_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            UART_PROTOCOL_XM_Process();         // UARTЭ�����̴���
            STEP_Process_XM_Left_TurnSignalUartTest();
            break;

        // (С��)��ת��Ʋ���
        case STATE_UART_XM_RIGHT_TURN_SIGNAL_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            UART_PROTOCOL_XM_Process();         // UARTЭ�����̴���
            STEP_Process_XM_Right_TurnSignalUartTest();
            break;

        // (С��)���Ų���
        case STATE_UART_XM_THROTTLE_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            UART_PROTOCOL_XM_Process();         // UARTЭ�����̴���
            STEP_Process_XM_ThrottleUartTest();
            break;

        // (С��)ɲ�Ѳ���
        case STATE_UART_XM_BRAKE_TEST:
            UART_DRIVE_Process();               // UART��������̴���
            UART_PROTOCOL_XM_Process();         // UARTЭ�����̴���
            STEP_Process_XM_BrakeUartTest();
            break;
            
        default:
            break;
    }
}
