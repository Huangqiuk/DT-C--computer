#ifndef __STATE_H__
#define __STATE_H__

#include "common.h"

#define FIRST_DAC                                1000
#define SECOND_DAC                               3000
#define THIRD_DAC                                5000
#define ERROR_DAC                                500
#define LIGHT_GND_VOLTAGE_VALUE                  3000       // ���GND��ѹֵ�����ڼ�����Ƿ���������,��λmv

// ϵͳ״̬����
typedef enum
{
    STATE_NULL = 0,                     // ��״̬
    STATE_STANDBY,

    STATE_UART_TEST,
    STATE_UART_GND_TEST,                    // GND����

    // ����ͨѶ
    STATE_UART_HEADLIGHT_TEST,              // UARTͨ�Ŵ�Ʋ���
    STATE_UART_LBEAM_TEST,                  // UARTͨ�Ž���Ʋ���
    STATE_UART_HBEAM_TEST,                  // UARTͨ��Զ��Ʋ���
    STATE_UART_LEFT_TURN_SIGNAL_TEST,       // UARTͨ����ת��Ʋ���
    STATE_UART_RIGHT_TURN_SIGNAL_TEST,      // UARTͨ����ת��Ʋ���
    STATE_UART_THROTTLE_TEST,               // UARTͨ�����Ų���
    STATE_UART_BRAKE_TEST,                  // UARTͨ��ɲ�Ѳ���
    STATE_UART_BUTTON_TEST,                 // UARTͨ�Ű�������
    STATE_UART_LIGHT_SENSING_TEST,          // UARTͨ�Ź�������
    STATE_UART_VLK_TEST,                    // UARTͨ��VLK����
    STATE_UART_VOLTAGE_TEST,                // UARTͨ�ŵ�ѹУ׼����
    STATE_UART_DISPLAY_TEST,                // UARTͨ����ʾ����
    STATE_UART_DERAILLEUR_TEST,             // UARTͨ�ŵ��ӱ��ٲ���

    // CANͨ��
    STATE_CAN_HEADLIGHT_TEST,               // CANͨ�Ŵ�Ʋ���
    STATE_CAN_LBEAM_TEST,                   // ����Ʋ���
    STATE_CAN_HBEAM_TEST,                   // Զ��Ʋ���
    STATE_CAN_LEFT_TURN_SIGNAL_TEST,        // CANͨ����ת��Ʋ���
    STATE_CAN_RIGHT_TURN_SIGNAL_TEST,       // CANͨ����ת��Ʋ���
    STATE_CAN_THROTTLE_TEST,                // CANͨ�����Ų���
    STATE_CAN_BRAKE_TEST,                   // CANͨ��ɲ�Ѳ���
    STATE_CAN_BUTTON_TEST,                  // CANͨ�Ű�������
    STATE_CAN_LIGHT_SENSING_TEST,           // CANͨ�Ź�������
    STATE_CAN_VLK_TEST,                     // CANͨ��VLK����
    STATE_CAN_VOLTAGE_TEST,                 // CANͨ�ŵ�ѹУ׼����
    STATE_CAN_DISPLAY_TEST,                 // CANͨ����ʾ����
    STATE_CAN_DERAILLEUR_TEST,              // CANͨ�ŵ��ӱ��ٲ���
/////////////////////////////////////////////////////////////////////////////////

    STATE_GAOBIAO_CAN_UPGRADE,                  // �߱�CAN����
    STATE_CM_CAN_UPGRADE,                       // ͨ��can����
    STATE_HUAXIN_CAN_UPGRADE,                   // ��о΢��can����
    STATE_SEG_CAN_UPGRADE,                      // ����ܡ�������can����
    STATE_KAIYANG_CAN_UPGRADE,                  // ����can����
    STATE_SPARROW_CAN_UPGRADE,                  // SPARROW CAN����
    STATE_CM_UART_SEG_UPGRADE,                  // ������������ܵ�ͨ�ô�������
    STATE_CM_UART_BC_UPGRADE,                   // ͨ�ò�����ͨ�ô�������
    STATE_HUAXIN_UART_BC_UPGRADE,               // ��оά�ز����Ĵ�������
    STATE_KAIYANG_UART_BC_UPGRADE,              // ���������Ĵ�������
    STATE_LIME_UART_BC_UPGRADE,                 // LIME�Ĵ�������
    
/////////////////////////////////////////////////////////////////////////////////

    STATE_UART_XM_HEADLIGHT_TEST,              // UART(С��)ͨ�Ŵ�Ʋ���
    STATE_UART_XM_LEFT_TURN_SIGNAL_TEST,       // UART(С��)ͨ����ת��Ʋ���
    STATE_UART_XM_RIGHT_TURN_SIGNAL_TEST,      // UART(С��)ͨ����ת��Ʋ���
    STATE_UART_XM_THROTTLE_TEST,               // UART(С��)ͨ�����Ų���
    STATE_UART_XM_BRAKE_TEST,                  // UART(С��)ͨ��ɲ�Ѳ���
    STATE_UART_XM_VLK_TEST,                    // UART(С��)ͨ��VLK����

    STATE_MAX                                   // ״̬��
} STATE_E;


// ϵͳ״̬����
typedef enum
{
    STEP_NULL = 0,                          // ��

    // GND����
    STEP_GND_TEST_ENTRY,                    // ��ڲ���
    STEP_GND_TEST_WAIT_RESPONSE,            // �ȴ����ñ��Ӧ
    STEP_GND_TEST_THROTTLE_GND,
    STEP_GND_TEST_CHECK_THROTTLE_GND_VALUE, // �ȶ�����GND
    STEP_GND_TEST_BRAKE_GND,
    STEP_GND_TEST_CHECK_BRAKE_GND_VALUE,    // �ȶ�ɲ��GND
    STEP_GND_TEST_DERAILLEUR_GND,
    STEP_GND_TEST_CHECK_DERAILLEUR_GND_VALUE,    // �ȶԵ��ӱ���GND    
    STEP_GND_TEST_COMPLETE,                 //
    STEP_GND_TEST_COMMUNICATION_TIME_OUT,   // ͨѶ��ʱ

    // UART����
    STEP_UART_TEST_ENTRY,                       // ��ڲ���
    STEP_UART_TEST_WAIT_RESPONSE,               // �ȴ����ñ��Ӧ
    STEP_UART_TEST_CHECK_VOLTAGE_VALUE,         // �ȶ�VCC
    STEP_UART_TEST_COMPLETE,                    // UART������ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_UART_TEST_COMMUNICATION_TIME_OUT,      // ͨѶ��ʱ

    /* UART */
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ��Ʋ���
    STEP_HEADLIGHT_UART_TEST_ENTRY,               // ��ڲ���
    STEP_HEADLIGHT_UART_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_HEADLIGHT_UART_TEST_NOTIFY_DUT,          // ֪ͨDUT�򿪴�ƣ��ȴ�DUT��Ӧ
    STEP_HEADLIGHT_UART_TEST_OPEN,                // �򿪴��,��    VCC
    STEP_HEADLIGHT_UART_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_HEADLIGHT_UART_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_HEADLIGHT_UART_TEST_COMPLETE,            // ��Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_HEADLIGHT_UART_TEST_COMMUNICATION_TIME_OUT,

    // ����Ʋ���
    STEP_LBEAM_UART_TEST_ENTRY,               // ��ڲ���
    STEP_LBEAM_UART_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_LBEAM_UART_TEST_NOTIFY_DUT,          // ֪ͨDUT�򿪽���ƣ��ȴ�DUT��Ӧ
    STEP_LBEAM_UART_TEST_OPEN,                // �򿪽����,��  VCC
    STEP_LBEAM_UART_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_LBEAM_UART_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_LBEAM_UART_TEST_COMPLETE,            // ����Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_LBEAM_UART_TEST_COMMUNICATION_TIME_OUT,

    // Զ��Ʋ���
    STEP_HBEAM_UART_TEST_ENTRY,               // ��ڲ���
    STEP_HBEAM_UART_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_HBEAM_UART_TEST_NOTIFY_DUT,          // ֪ͨDUT��Զ��ƣ��ȴ�DUT��Ӧ
    STEP_HBEAM_UART_TEST_OPEN,                // ��Զ���,��  VCC
    STEP_HBEAM_UART_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_HBEAM_UART_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_HBEAM_UART_TEST_COMPLETE,            // Զ��Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_HBEAM_UART_TEST_COMMUNICATION_TIME_OUT,

    // ��ת��Ʋ���
    STEP_LEFT_TURN_SIGNAL_UART_TEST_ENTRY,               // ��ڲ���
    STEP_LEFT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_LEFT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT,          // ֪ͨDUT��Զ��ƣ��ȴ�DUT��Ӧ
    STEP_LEFT_TURN_SIGNAL_UART_TEST_OPEN,                // ��Զ���,��   VCC
    STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_LEFT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2,
    STEP_LEFT_TURN_SIGNAL_UART_TEST_COMPLETE,            // Զ��Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_LEFT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,

    // ��ת��Ʋ���
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_ENTRY,               // ��ڲ���
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_NOTIFY_DUT,          // ֪ͨDUT��Զ��ƣ��ȴ�DUT��Ӧ
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_OPEN,                // ��Զ���,��  VCC
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE2,    // �ȶ�GND
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_CHECK_GND_VALUE3,    // �ȶ�GND        
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMPLETE,            // Զ��Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_RIGHT_TURN_SIGNAL_UART_TEST_COMMUNICATION_TIME_OUT,

    // ���Ų���
    STEP_THROTTLE_UART_TEST_ENTRY,               // ��ڲ���
    STEP_THROTTLE_UART_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_THROTTLE_UART_TEST_VCC,                 // ֪ͨ���ñ��VCC
    STEP_THROTTLE_UART_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_THROTTLE_UART_TEST_CHECK_DAC1_VALUE,    // �ȶ�DAC
    STEP_THROTTLE_UART_TEST_CHECK_DAC3_VALUE,    // �ȶ�DAC
    STEP_THROTTLE_UART_TEST_CHECK_DAC5_VALUE,    // �ȶ�DAC
    STEP_THROTTLE_UART_TEST_COMPLETE,            // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
    STEP_THROTTLE_UART_TEST_COMMUNICATION_TIME_OUT,  // ͨ�ų�ʱ

    // ɲ�Ѳ���
    STEP_BRAKE_UART_TEST_ENTRY,               // ��ڲ���
    STEP_BRAKE_UART_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_BRAKE_UART_TEST_VCC,                 // ֪ͨ���ñ��VCC
    STEP_BRAKE_UART_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_BRAKE_UART_TEST_CHECK_DAC1_VALUE,    // �ȶ�DAC
    STEP_BRAKE_UART_TEST_CHECK_DAC3_VALUE,    // �ȶ�DAC
    STEP_BRAKE_UART_TEST_CHECK_DAC5_VALUE,    // �ȶ�DAC
    STEP_BRAKE_UART_TEST_COMPLETE,            // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
    STEP_BRAKE_UART_TEST_COMMUNICATION_TIME_OUT,  // ͨ�ų�ʱ

    // ���ӱ��ٲ���
    STEP_DERAILLEUR_UART_TEST_ENTRY,               // ��ڲ���
    STEP_DERAILLEUR_UART_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_DERAILLEUR_UART_TEST_VCC,                 // ֪ͨ���ñ��VCC
    STEP_DERAILLEUR_UART_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_DERAILLEUR_UART_TEST_CHECK_DAC1_VALUE,    // �ȶ�DAC
    STEP_DERAILLEUR_UART_TEST_CHECK_DAC3_VALUE,    // �ȶ�DAC
    STEP_DERAILLEUR_UART_TEST_CHECK_DAC5_VALUE,    // �ȶ�DAC
    STEP_DERAILLEUR_UART_TEST_COMPLETE,            // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
    STEP_DERAILLEUR_UART_TEST_COMMUNICATION_TIME_OUT,  // ͨ�ų�ʱ
    
    // VLK����->֪ͨ���ñ��VLK��ѹ�����·��ĵ�ѹ�ȶ�
    STEP_VLK_UART_TEST_ENTRY,                   // ��ڲ���
    STEP_VLK_UART_TEST_WAIT_RESPONSE,           // �ȴ��Ǳ�����ñ��Ӧ
    STEP_VLK_UART_TEST_VLK,                     // ֪ͨ���ñ��VLK
    STEP_VLK_UART_TEST_CHECK_VOLTAGE_VALUE,     // �ȶ�VLKֵ
    STEP_VLK_UART_TEST_COMPLETE,                // ������ɣ�Ȼ���ϱ����
    STEP_VLK_UART_TEST_COMMUNICATION_TIME_OUT,  // ͨ�ų�ʱ

    // ��ѹУ׼ -> �õ�У׼��ѹ -> ֪ͨ��Դ������Ǳ����ѹ ->  ������� ->  ֪ͨ�Ǳ���е�ѹУ׼ -> ��֪DTAУ׼��� ->�ж��ж��ٸ���ѹֵ��Ҫ���� -> �����һ�� -> �ϱ�STSУ׼��� -> �ж�� ->�ظ�У׼����
    STEP_CALIBRATION_UART_TEST_ENTRY,                      // ��ڲ���
    STEP_CALIBRATION_UART_TEST_WAIT_RESPONSE,              // �ȴ��Ǳ��Ӧ
    STEP_CALIBRATION_UART_TEST_GET_CALIBRATION_VOLTAGE,    // �õ�У׼��ѹ
    STEP_CALIBRATION_UART_TEST_NOTIFY_POWER_BOARD,         // ֪ͨ��Դ������Ǳ����ѹ
    STEP_CALIBRATION_UART_TEST_WAIT_ADJUSTMENT,            // �ȴ���Դ��������
    STEP_CALIBRATION_UART_TEST_NOTIFY_VOLTAGE_CALIBRATION, // ֪ͨ�Ǳ���е�ѹУ׼

    STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_POWER_BOARD,         // ֪ͨ��Դ������Ǳ����ѹ
    STEP_CALIBRATION_UART_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION, // ֪ͨ�Ǳ���е�ѹУ׼

    STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_POWER_BOARD,         // ֪ͨ��Դ������Ǳ����ѹ
    STEP_CALIBRATION_UART_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION, // ֪ͨ�Ǳ���е�ѹУ׼
    STEP_CALIBRATION_UART_TEST_RESTORE_VOLTAGE_CALIBRATION,
    STEP_CALIBRATION_UART_TEST_COMPLETE,                   // ������ɣ�Ȼ���ϱ����
    STEP_CALIBRATION_UART_TEST_COMMUNICATION_TIME_OUT,     // ͨ�ų�ʱ

    STEP_UART_CONFIG_PARAMETER_WRITE,         // д�����ò���
    STEP_UART_CONFIG_PARAMETER_VALIDATE,      // ���ò���У��

    /* CAN */
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ��Ʋ���
    STEP_HEADLIGHT_CAN_TEST_ENTRY,               // ��ڲ���
    STEP_HEADLIGHT_CAN_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_HEADLIGHT_CAN_TEST_NOTIFY_DUT,          // ֪ͨDUT�򿪴�ƣ��ȴ�DUT��Ӧ
    STEP_HEADLIGHT_CAN_TEST_OPEN,                // �򿪴�ƣ��ȴ����ñ��Ӧ
    STEP_HEADLIGHT_CAN_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_HEADLIGHT_CAN_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_HEADLIGHT_CAN_TEST_CHECK_CURRENT_VALUE, // �ȶԵ���
    STEP_HEADLIGHT_CAN_TEST_COMPLETE,            // ��Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_HEADLIGHT_CAN_TEST_COMMUNICATION_TIME_OUT,

    // ����Ʋ���
    STEP_LBEAM_CAN_TEST_ENTRY,               // ��ڲ���
    STEP_LBEAM_CAN_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_LBEAM_CAN_TEST_NOTIFY_DUT,          // ֪ͨDUT�򿪽���ƣ��ȴ�DUT��Ӧ
    STEP_LBEAM_CAN_TEST_OPEN,                // �򿪽����,��   VCC
    STEP_LBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_LBEAM_CAN_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_LBEAM_CAN_TEST_COMPLETE,            // ����Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_LBEAM_CAN_TEST_COMMUNICATION_TIME_OUT,

    // Զ��Ʋ���
    STEP_HBEAM_CAN_TEST_ENTRY,               // ��ڲ���
    STEP_HBEAM_CAN_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_HBEAM_CAN_TEST_NOTIFY_DUT,          // ֪ͨDUT��Զ��ƣ��ȴ�DUT��Ӧ
    STEP_HBEAM_CAN_TEST_OPEN,                // ��Զ���,��   VCC
    STEP_HBEAM_CAN_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_HBEAM_CAN_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_HBEAM_CAN_TEST_COMPLETE,            // Զ��Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_HBEAM_CAN_TEST_COMMUNICATION_TIME_OUT,

    // ��ת��Ʋ���
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_ENTRY,               // ��ڲ���
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT,          // ֪ͨDUT����ת��ƣ��ȴ�DUT��Ӧ
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_OPEN,                // ����ת��ƣ��ȴ����ñ��Ӧ
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_CHECK_CURRENT_VALUE, // �ȶԵ���
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_COMPLETE,            // ��ת��Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_LEFT_TURN_SIGNAL_CAN_TEST_COMMUNICATION_TIME_OUT,  // ��ת��Ʋ���ͨ�ų�ʱ

    // ��ת��Ʋ���
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_ENTRY,               // ��ڲ���
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_NOTIFY_DUT,          // ֪ͨDUT��ת��ƣ��ȴ�DUT��Ӧ
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_OPEN,                // ����ת��ƣ��ȴ����ñ��Ӧ
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_GND_VALUE,     // �ȶ�GND
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_CHECK_CURRENT_VALUE, // �ȶԵ���
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_COMPLETE,            // ת��Ʋ�����ɲ��裬�رղ������ţ�Ȼ���ϱ����
    STEP_RIGHT_TURN_SIGNAL_CAN_TEST_COMMUNICATION_TIME_OUT,  // ת��Ʋ���ͨ�ų�ʱ

    // ���Ų���
    STEP_THROTTLE_CAN_TEST_ENTRY,               // ��ڲ���
    STEP_THROTTLE_CAN_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_THROTTLE_CAN_TEST_VCC,                 // ֪ͨ���ñ��VCC
    STEP_THROTTLE_CAN_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_THROTTLE_CAN_TEST_CHECK_DAC1_VALUE,    // �ȶ�DAC
    STEP_THROTTLE_CAN_TEST_CHECK_DAC3_VALUE,    // �ȶ�DAC
    STEP_THROTTLE_CAN_TEST_CHECK_DAC5_VALUE,    // �ȶ�DAC
    STEP_THROTTLE_CAN_TEST_COMPLETE,            // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
    STEP_THROTTLE_CAN_TEST_COMMUNICATION_TIME_OUT,  // ͨ�ų�ʱ

    // ɲ�Ѳ���
    STEP_BRAKE_CAN_TEST_ENTRY,               // ��ڲ���
    STEP_BRAKE_CAN_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_BRAKE_CAN_TEST_VCC,                 // ֪ͨ���ñ��VCC
    STEP_BRAKE_CAN_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_BRAKE_CAN_TEST_CHECK_DAC1_VALUE,    // �ȶ�DAC
    STEP_BRAKE_CAN_TEST_CHECK_DAC3_VALUE,    // �ȶ�DAC
    STEP_BRAKE_CAN_TEST_CHECK_DAC5_VALUE,    // �ȶ�DAC
    STEP_BRAKE_CAN_TEST_COMPLETE,            // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
    STEP_BRAKE_CAN_TEST_COMMUNICATION_TIME_OUT,  // ͨ�ų�ʱ

    // ���ӱ��ٲ���
    STEP_DERAILLEUR_CAN_TEST_ENTRY,               // ��ڲ���
    STEP_DERAILLEUR_CAN_TEST_WAIT_RESPONSE,       // �ȴ��Ǳ�����ñ��Ӧ
    STEP_DERAILLEUR_CAN_TEST_VCC,                 // ֪ͨ���ñ��VCC
    STEP_DERAILLEUR_CAN_TEST_CHECK_VOLTAGE_VALUE, // �ȶ�VCC
    STEP_DERAILLEUR_CAN_TEST_CHECK_DAC1_VALUE,    // �ȶ�DAC
    STEP_DERAILLEUR_CAN_TEST_CHECK_DAC3_VALUE,    // �ȶ�DAC
    STEP_DERAILLEUR_CAN_TEST_CHECK_DAC5_VALUE,    // �ȶ�DAC
    STEP_DERAILLEUR_CAN_TEST_COMPLETE,            // ���Ų�����ɲ��裬�ر�DAC�������Ϊ0��Ȼ���ϱ����
    STEP_DERAILLEUR_CAN_TEST_COMMUNICATION_TIME_OUT,  // ͨ�ų�ʱ
    
    // VLK����->֪ͨ���ñ��VLK��ѹ�����·��ĵ�ѹ�ȶ�
    STEP_VLK_CAN_TEST_ENTRY,                   // ��ڲ���
    STEP_VLK_CAN_TEST_WAIT_RESPONSE,           // �ȴ��Ǳ�����ñ��Ӧ
    STEP_VLK_CAN_TEST_VLK,                     // ֪ͨ���ñ��VLK
    STEP_VLK_CAN_TEST_CHECK_VOLTAGE_VALUE,     // �ȶ�VLKֵ
    STEP_VLK_CAN_TEST_COMPLETE,                // ������ɣ�Ȼ���ϱ����
    STEP_VLK_CAN_TEST_COMMUNICATION_TIME_OUT,  // ͨ�ų�ʱ

    // ��ѹУ׼ -> �õ�У׼��ѹ -> ֪ͨ��Դ������Ǳ����ѹ ->  ������� ->  ֪ͨ�Ǳ���е�ѹУ׼ -> ��֪DTAУ׼��� ->�ж��ж��ٸ���ѹֵ��Ҫ���� -> �����һ�� -> �ϱ�STSУ׼��� -> �ж�� ->�ظ�У׼����
    STEP_CALIBRATION_CAN_TEST_ENTRY,                      // ��ڲ���
    STEP_CALIBRATION_CAN_TEST_WAIT_RESPONSE,              // �ȴ��Ǳ��Ӧ
    STEP_CALIBRATION_CAN_TEST_GET_CALIBRATION_VOLTAGE,    // �õ�У׼��ѹ
    STEP_CALIBRATION_CAN_TEST_NOTIFY_POWER_BOARD,         // ֪ͨ��Դ������Ǳ����ѹ
    STEP_CALIBRATION_CAN_TEST_WAIT_ADJUSTMENT,            // �ȴ���Դ��������
    STEP_CALIBRATION_CAN_TEST_NOTIFY_VOLTAGE_CALIBRATION, // ֪ͨ�Ǳ���е�ѹУ׼

    STEP_CALIBRATION_CAN_TEST_TWO_NOTIFY_POWER_BOARD,         // ֪ͨ��Դ������Ǳ����ѹ
    STEP_CALIBRATION_CAN_TEST_TWO_NOTIFY_VOLTAGE_CALIBRATION, // ֪ͨ�Ǳ���е�ѹУ׼

    STEP_CALIBRATION_CAN_TEST_THREE_NOTIFY_POWER_BOARD,         // ֪ͨ��Դ������Ǳ����ѹ
    STEP_CALIBRATION_CAN_TEST_THREE_NOTIFY_VOLTAGE_CALIBRATION, // ֪ͨ�Ǳ���е�ѹУ׼

    STEP_CALIBRATION_CAN_TEST_CHECK_CALIBRATION_RESULT,   // ��֪У׼���
    STEP_CALIBRATION_CAN_TEST_CHECK_VOLTAGE_VALUES,       // �ж��ж��ٸ���ѹֵ��Ҫ����
    STEP_CALIBRATION_CAN_TEST_REPORT_STS_RESULT,          // �ϱ�STSУ׼���
    STEP_CALIBRATION_CAN_TEST_REPEAT_CALIBRATION,         // �ظ�У׼����
    STEP_CALIBRATION_CAN_TEST_COMPLETE,                   // ������ɣ�Ȼ���ϱ����
    STEP_CALIBRATION_CAN_TEST_COMMUNICATION_TIME_OUT,     // ͨ�ų�ʱ

    STEP_CAN_CONFIG_PARAMETER_WRITE,         // д�����ò���
    STEP_CAN_CONFIG_PARAMETER_VALIDATE,      // ���ò���У��


    /****************************************����********************************************/

	// �߱�can����
	STEP_GAOBIAO_CAN_UPGRADE_ENTRY,			// ��ڲ���
	STEP_GAOBIAO_CAN_UPGRADE_CAN_INIT,		// CAN��ʼ��
	STEP_GAOBIAO_CAN_UPGRADE_POWER_ON,
	STEP_GAOBIAO_CAN_UPGRADE_WAIT_3S,
	STEP_GAOBIAO_CAN_UPGRADE_SEND_KEEP_ELECTRICITY,
	STEP_GAOBIAO_CAN_UPGRADE_SEND_BOOT_CMD,
	STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY,			// ѯ���Ƿ���Խ�������
	STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY,
	STEP_GAOBIAO_CAN_UPGRADE_SEND_DATA,
	STEP_GAOBIAO_CAN_UPGRADE_WRITE_ERROR,
	STEP_GAOBIAO_CAN_UPGRADE_FINISH,
	STEP_GAOBIAO_CAN_UPGRADE_POWER_OFF,
	STEP_GAOBIAO_CAN_UPGRADE_TIME_OUT,
	STEP_GAOBIAO_CAN_UPGRADE_FAILE,
	
	// ͨ��can��������
	STEP_CM_CAN_UPGRADE_ENTRY,								// ��ڲ���
	STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK,    					// �ȴ�dut��Ӧ
    STEP_CM_CAN_UPGRADE_RESTART,                            // �Ǳ�����ʹ��VLK��������
	STEP_CM_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// �յ�dut��eco����
	STEP_CM_CAN_UPGRADE_UP_ALLOW,							// ����ģʽ����

	STEP_CM_CAN_UPGRADE_SEND_UI_EAR,						// ����ui����ָ��
	STEP_CM_CAN_UPGRADE_UI_EAR_RESUALT,						// ���յ�dut���ص�ui�������
	STEP_CM_CAN_UPGRADE_SEND_UI_WRITE,						// ����ui��������
	STEP_CM_CAN_UPGRADE_UI_UP_SUCCESS,				 		// ui�������
	STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_RESET,               // app�����������
	STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS_YELLOW,              // app���������Ļ��ʾ��ɫ    
    
	STEP_CM_CAN_UPGRADE_SEND_APP_EAR,						// ����app����ָ��
	STEP_CM_CAN_UPGRADE_APP_EAR_RESUALT,					// ���յ�dut���ص�app�������
	STEP_CM_CAN_UPGRADE_SEND_APP_WRITE,						// ����app��������
	STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS,						// app�������

	STEP_CM_CAN_UPGRADE_ITEM_FINISH,						// �������������
	
	STEP_CM_CAN_UPGRADE_COMMUNICATION_TIME_OUT,				// ���ֳ�ʱ
	STEP_CM_CAN_UPGRADE_RECONNECTION,						// ��������
	STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT,				// �������ӳ�ʱ

	// ��о΢��can��������
	STEP_HUAXIN_CAN_UPGRADE_ENTRY,							// ��ڲ���
	STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK,					// �ȴ�dut��Ӧ
	STEP_HUAXIN_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_HUAXIN_CAN_UPGRADE_UP_ALLOW,						// ����ģʽ����

	STEP_HUAXIN_CAN_UPGRADE_SEND_APP_EAR,					// ����app����ָ��
	STEP_HUAXIN_CAN_UPGRADE_APP_EAR_RESUALT,				// ���յ�dut���ص�app�������
	STEP_HUAXIN_CAN_UPGRADE_SEND_APP_WRITE,					// ����app��������
	STEP_HUAXIN_CAN_UPGRADE_APP_UP_SUCCESS,					// app�������

	STEP_HUAXIN_CAN_UPGRADE_SEND_UI_EAR,					// ����ui����ָ��
	STEP_HUAXIN_CAN_UPGRADE_UI_EAR_RESUALT,					// ���յ�dut���ص�ui�������
	STEP_HUAXIN_CAN_UPGRADE_SEND_UI_WRITE,					// ����ui��������
	STEP_HUAXIN_CAN_UPGRADE_UI_UP_SUCCESS,					// ui�������

	STEP_HUAXIN_CAN_UPGRADE_ITEM_FINISH,					// �������������

	STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,				// ���ֳ�ʱ

	// can�����������������
	STEP_SEG_CAN_UPGRADE_ENTRY,								// ��ڲ���
	STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK,    					// �ȴ�dut��Ӧ
	STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// �յ�dut��eco����
	STEP_SEG_CAN_UPGRADE_UP_ALLOW,							// ����ģʽ����

	STEP_SEG_CAN_UPGRADE_SEND_APP_EAR,						// ����app����ָ��
	STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT,					// ���յ�dut���ص�app�������
	STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE,					// ����app��������
	STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS,					// app�������

	STEP_SEG_CAN_UPGRADE_ITEM_FINISH,						// �������������

	STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT, 			// ���ֳ�ʱ

	// can��������
	STEP_KAIYANG_CAN_UPGRADE_ENTRY,								// ��ڲ���
	STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK,    					// �ȴ�dut��Ӧ
	STEP_KAIYANG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// �յ�dut��eco����
	STEP_KAIYANG_CAN_UPGRADE_UP_ALLOW,							// ����ģʽ����

	
	STEP_KAIYANG_CAN_UPGRADE_SEND_APP_WRITE,					// ����app��������
	STEP_KAIYANG_CAN_UPGRADE_SEND_APP_AGAIN,					// �ֶ�������һ֡����
	STEP_KAIYANG_CAN_UPGRADE_APP_UP_SUCCESS,					// app�������

	STEP_KAIYANG_CAN_UPGRADE_SEND_UI_WRITE,					// ����ui��������
	STEP_KAIYANG_CAN_UPGRADE_SEND_UI_AGAIN,					// ���·���ui���ݰ�
	STEP_KAIYANG_CAN_UPGRADE_UI_UP_SUCCESS,					// ui�������

	STEP_KAIYANG_CAN_UPGRADE_ITEM_FINISH,						// �������������

 	// can����������Ϣ
 	STEP_CAN_SET_CONFIG_ENTRY,								// ����config���
    STEP_CAN_CONFIG_RESET,                                  // ����
	STEP_CAN_SEND_SET_CONFIG,								// ����config����		
	STEP_CAN_SET_CONFIG_SUCCESS,							// ����config�ɹ�
    STEP_CAN_READ_CONFIG,                                   // ��ȡ���ò���
    STEP_CAN_VERIFY_CONFIG,                                 // У�����ò���
    
    //ͨ�ò�������
    STEP_CM_UART_UPGRADE_ENTRY,                         // ��ڲ���		
    STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK,               // �ȴ�dut��Ӧ
    STEP_CM_UART_BC_UPGRADE_UP_ALLOW,                   // ����ģʽ����
    STEP_CM_UART_BC_UPGRADE_RESTART,                    // �Ǳ�����ʹ��VLK��������
    STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR,               // ����app����ָ��
    STEP_CM_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET,      // ���͵�һ������
    STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET,            // ����app��������
    STEP_CM_UART_BC_UPGRADE_APP_UP_SUCCESS,             // app�������

    STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR,                // ����ui����ָ��
    STEP_CM_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET,       // ���͵�һ������
    STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET,             // ����ui��������
    STEP_CM_UART_BC_UPGRADE_UI_UP_SUCCESS,              // ui�������

    STEP_CM_UART_BC_UPGRADE_ITEM_FINISH,                // �������������

	// ��оά�ز�����������
	STEP_HUAXIN_UART_UPGRADE_ENTRY,							// ��ڲ���
	STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK,    				// �ȴ�dut��Ӧ
	STEP_HUAXIN_UART_BC_UPGRADE_UP_ALLOW,						// ����ģʽ����

	STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_EAR,					// ����app����ָ��
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET,			// ���͵�һ������
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_PACKET,				// ����app��������
	STEP_HUAXIN_UART_BC_UPGRADE_APP_UP_SUCCESS,					// app�������

	STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR,					// ����ui����ָ��
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET,			// ���͵�һ������
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_PACKET,					// ����ui��������
	STEP_HUAXIN_UART_BC_UPGRADE_UI_UP_SUCCESS,					// ui�������

	STEP_HUAXIN_UART_BC_UPGRADE_ITEM_FINISH,					// �������������
	
	// �����/��������������
	STEP_SEG_UART_UPGRADE_ENTRY,							// ��ڲ���
	STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK,    				// �ȴ�dut��Ӧ
    STEP_CM_UART_SEG_UPGRADE_RESTART,                       // �Ǳ�����ʹ��VLK��������
	STEP_CM_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_CM_UART_SEG_UPGRADE_UP_ALLOW,						// ����ģʽ����

	STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR,					// ����app����ָ��
	STEP_CM_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET,			// ���͵�һ������
	STEP_CM_UART_SEG_UPGRADE_SEND_APP_PACKET,				// ����app��������
	STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS,				// app�������
	STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS_RESET,          // app�����������
    
	STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH,					// �������������

	// ������������
	STEP_KAIYANG_UART_UPGRADE_ENTRY,						// ��ڲ���
	STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK,    				// �ȴ�dut��Ӧ
	STEP_KAIYANG_UART_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_KAIYANG_UART_UPGRADE_UP_ALLOW,						// ����ģʽ����

	
	STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE,				// ����app��������
	STEP_KAIYANG_UART_UPGRADE_SEND_APP_AGAIN,				// �ֶ��ط�
	STEP_KAIYANG_UART_UPGRADE_APP_UP_SUCCESS,				// app�������

	STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE,				// ����ui��������
	STEP_KAIYANG_UART_UPGRADE_SEND_UI_AGAIN,				// �ֶ��ط�
	STEP_KAIYANG_UART_UPGRADE_UI_UP_SUCCESS,				// ui�������

	STEP_KAIYANG_UART_UPGRADE_ITEM_FINISH,					// �������������
	
	STEP_KAIYANG_UART_UPGRADE_COMMUNICATION_TIME_OUT,		// ���ֳ�ʱ

	// LIME����
	STEP_LIME_UART_UPGRADE_ENTRY,							// ��ڲ���
	STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK,    				  // �ȴ�dut��Ӧ
	STEP_LIME_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_LIME_UART_SEG_UPGRADE_UP_ALLOW,						      // ����ģʽ����

	STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR,				     	// ����app����ָ��
	STEP_LIME_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET,			// ���͵�һ������
	STEP_LIME_UART_SEG_UPGRADE_SEND_APP_PACKET,				    // ����app��������
	STEP_LIME_UART_SEG_UPGRADE_APP_UP_SUCCESS,				    // app�������
	
	STEP_LIME_UART_SEG_UPGRADE_ITEM_FINISH,				       	// �������������
	
	STEP_LIME_UART_UPGRADE_COMMUNICATION_TIME_OUT,		// ���ֳ�ʱ
    
    // ����������Ϣ����
    STEP_UART_SET_CONFIG_ENTRY,                              // ����config���
    STEP_UART_CONFIG_RESET,                                  // ����
    STEP_UART_PROTOCOL_SWITCCH,                              // Э���л�
    STEP_UART_SEND_SET_CONFIG,                               // ����config����
    STEP_UART_SET_CONFIG_SUCCESS,                            // ����config�ɹ�
    STEP_UART_READ_CONFIG,                                   // ��ȡ���ò���
    STEP_UART_VERIFY_CONFIG,                                 // У�����ò���
    
    STEP_MAX
} STEP_E;

typedef struct
{
    STATE_E state;                  // ��ǰϵͳ״̬
    STATE_E preState;               // ��һ��״̬

    STEP_E step;
} STATE_CB;

extern STATE_CB stateCB;


// ״̬����ʼ��
void STATE_Init(void);

// �����Զ��ػ���ʱ
void STATE_ResetAutoPowerOffControl(void);

// ״̬Ǩ��
void STATE_EnterState(uint32 state);

// ״̬������
void STATE_Process(void);

// �����л�
void STATE_SwitchStep(uint32 param);

// ��Դ������������
void STEP_Process_PowerUartUpgrade(void);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern BOOL resetFlag;
extern BOOL configResetFlag;
extern BOOL appResetFlag;
extern uint8 verifiedBuff[150];
#endif


