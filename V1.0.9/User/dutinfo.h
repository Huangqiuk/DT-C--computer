#ifndef __DUT_INFO_H__
#define __DUT_INFO_H__

#include "common.h"

typedef enum
{
    DUT_TYPE_NULL,
    DUT_TYPE_GB,
    DUT_TYPE_CM,
    DUT_TYPE_HUAXIN,
    DUT_TYPE_SEG,
    DUT_TYPE_KAIYANG,
    DUT_TYPE_LIME,
    DUT_TYPE_SPARROW,
    DUT_TYPE_MAX
} DUT_TYPE;

typedef enum
{
    CAN_TYPE_NULL,
    CAN_EXTENDED,
    CAN_STANDARD,
    CAN_TYPE_MAX
} CAN_TYPE;

typedef enum
{
    DUT_FILE_TYPE_TXT,
    DUT_FILE_TYPE_BIN,
    DUT_FILE_TYPE_JSON,
    DUT_FILE_TYPE_INI,
    DUT_FILE_TYPE_HEX,

    DUT_FILE_TYPE_MAX
} DUT_FILE_TYPE;


typedef enum
{
    GND_TYPE_NULL,
    GND_TYPE_THROTTLE,
    GND_TYPE_BRAKE,
    GND_TYPE_THROTTLE_BRAKE,
    GND_TYPE_DERAILLEUR,
    GND_TYPE_BRAKE_DERAILLEUR,    
    
    GND_TYPE_MAX
} GND_TYPE;

typedef enum
{
    TEST_TYPE_NULL,

    TEST_TYPE_UART,                   // UART����
    TEST_TYPE_GND,                    // GND����
    TEST_TYPE_HEADLIGHT,              // ��Ʋ���
    TEST_TYPE_LBEAM,                  // ����Ʋ���
    TEST_TYPE_HBEAM,                  // Զ��Ʋ���
    TEST_TYPE_LEFT_TURN_SIGNAL,       // ��ת��Ʋ���
    TEST_TYPE_RIGHT_TURN_SIGNAL,      // ��ת��Ʋ���
    TEST_TYPE_THROTTLE,               // ���Ų���
    TEST_TYPE_BRAKE,                  // ɲ�Ѳ���
    TEST_TYPE_DERAILLEUR,             // ���ӱ��ٲ���
    TEST_TYPE_BUTTON,                 // ��������
    TEST_TYPE_LIGHT_SENSING,          // ��������
    TEST_TYPE_VLK,                    // VLK����
    TEST_TYPE_VOLTAGE,                // ��ѹ����
    TEST_TYPE_DISPLAY,                // ��ʾ����
    
    TEST_TYPE_XM_HEADLIGHT,           // ��ƣ�С�ף�����
    TEST_TYPE_XM_LEFT_TURN_SIGNAL,    // ��ת��ƣ�С�ף�����
    TEST_TYPE_XM_RIGHT_TURN_SIGNAL,   // ��ת��ƣ�С�ף�����
    TEST_TYPE_XM_THROTTLE,            // ���ţ�С�ף�����
    TEST_TYPE_XM_BRAKE,               // ɲ�ѣ�С�ף�����
    
    TEST_TYPE_MAX
} TEST_TYPE;

typedef struct
{
    int8 name[20];
    uint8 voltage;
    uint8 nameLen;
    DUT_TYPE ID;
    TEST_TYPE test;
    GND_TYPE gnd;
    CAN_TYPE can;
    
    // dutͨѶ����
    BOOL dutBusType; // 0:���ڣ�1��can

    uint16 can_baudRate;
    uint32 uart_baudRate;

    // ������־λ
    BOOL bootUpFlag;
    BOOL appUpFlag;
    BOOL configUpFlag;
    BOOL uiUpFlag;

    // ���Ա�־λ
    BOOL powerFlag;
    BOOL calibrationFlag;
    BOOL calibration_one_Flag;
    BOOL calibration_two_Flag;
    BOOL calibration_three_Flag;

    // �ػ���־λ
    BOOL adjustState;

    // �����Ǳ��Դ
    BOOL powerOnFlag;

    // ��ʱʱ��
    uint8 timeout_Period;

    uint8 cali_cnt;
    uint8 volPower ;
    uint32 voltageMax ;
    uint32 voltageMin ;
    uint16 currentMax ;
    uint16 currentMin ;

    uint16 throttleAd;
    uint16 brakeAd;
    uint16 derailleurAd;

    uint8 cal_number;
    uint8 cal_value;

    // ����С
    uint32 bootSize;
    uint32 appSize;
    uint32 uiSize;

    // ��ǰ�����İ���
    uint32 currentBootSize;
    uint32 currentAppSize;
    uint32 currentUiSize;

    // ����ʧ�ܱ�־λ
    BOOL bootUpFaile;
    BOOL appUpFaile;
    BOOL uiUpFaile;
    BOOL configUpFaile;

    // ��������λ
    BOOL dutPowerOnAllow;

    // �����ɹ���־λ
    BOOL configUpSuccesss;
    BOOL uiUpSuccesss;
    BOOL appUpSuccesss;

    // dut�����ϱ�
    uint8 dutProgress[8];

    // ��������һ��tool��Ӧһ��dut
    uint8 toolRank;

    // dut��Ӧ��ʱ�ط�
    BOOL reconnectionFlag;
    BOOL reconnectionRepeatOne;
    uint8 reconnectionRepeatContent[150];

    // ͨѶЭ��
    BOOL commProt;
    
    // ����ģ���־
    BOOL buttonSimulationFlag;
    
    BOOL VLKFlag;
    uint8 bleVerBuff[20];
    BOOL passThroughControl;
//    BOOL passThroughStsToDut;
    uint32 online_dete_cnt;
    BOOL passThroughStsToDut;
    
} DUT_INFO;


extern DUT_INFO dut_info;
extern void DutInfoUpdata(void);

#endif
