#ifndef __STS_PROTOCOL_H__
#define __STS_PROTOCOL_H__

#include "common.h"

//=====================================================================================================
#define ERROR_VOL                 1000  // ��λ��mv

#define STARTUP                   0x01

#define POWER_ON_SUCCESS          1
#define THROTTLE_GND_TEST_FAILURE 1
#define BRAKING_GND_TEST_FAILURE  2
#define VLK_TEST_FAILURE          3
#define GND_TEST_TIMEOUT          4
#define SHUTDOWN                  0

#define STS_PROTOCOL_RX_QUEUE_SIZE 3 // ����������гߴ�
#define STS_PROTOCOL_TX_QUEUE_SIZE 3 // ����������гߴ�

#define STS_PROTOCOL_CMD_HEAD 0x55// ����ͷ

#define STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 150 // ���������֡����
#define STS_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 150 // ����������֡����
#define STS_PROTOCOL_RX_FIFO_SIZE 200            // ����һ����������С
#define STS_PROTOCOL_CMD_FRAME_LENGTH_MIN 4      // ����֡��С���ȣ�����:����ͷ�������֡����ݳ���

// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define STS_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / STS_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)      // ��λ:ms
#define STS_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / STS_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // ��λ:ms

// UART����Э�鶨��
typedef enum
{
    STS_PROTOCOL_CMD_HEAD_INDEX = 0, // ֡ͷ����
    STS_PROTOCOL_CMD_CMD_INDEX,      // ����������
    STS_PROTOCOL_CMD_LENGTH_INDEX,   // ���ݳ�������

    STS_PROTOCOL_CMD_DATA1_INDEX,  // ��������
    STS_PROTOCOL_CMD_DATA2_INDEX,  // ��������
    STS_PROTOCOL_CMD_DATA3_INDEX,  // ��������
    STS_PROTOCOL_CMD_DATA4_INDEX,  // ��������
    STS_PROTOCOL_CMD_DATA5_INDEX,  // ��������
    STS_PROTOCOL_CMD_DATA6_INDEX,  // ��������
    STS_PROTOCOL_CMD_DATA7_INDEX,  // ��������
    STS_PROTOCOL_CMD_DATA8_INDEX,  // ��������
    STS_PROTOCOL_CMD_DATA9_INDEX,  // ��������
    STS_PROTOCOL_CMD_DATA10_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA11_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA12_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA13_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA14_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA15_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA16_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA17_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA18_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA19_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA20_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA21_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA22_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA23_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA24_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA25_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA26_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA27_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA28_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA29_INDEX, // ��������
    STS_PROTOCOL_CMD_DATA30_INDEX, // ��������

    STS_PROTOCOL_CMD_INDEX_MAX
} STS_PROTOCOL_DATE_FRAME;

// ��������
typedef enum
{

    STS_PROTOCOL_CMD_NULL = 0,

    STS_PROTOCOL_CMD_REPORT_OF_IDENTITY = 0x34,            // DTA����ϱ���������=0x34��
    STS_PROTOCOL_CMD_CHECK_STATION_FLAG = 0x01,            // ��վ��־У��
    STS_PROTOCOL_CMD_CHECK_VERSION_INFO = 0x02,            // �汾��ϢУ������
    STS_PROTOCOL_CMD_GET_SOFTWARE_INFO = 0x30,             // ��ȡDTA�����Ϣ
    STS_PROTOCOL_CMD_CLEAR_APP = 0x31,                     // ����APP����
    STS_PROTOCOL_CMD_TOOL_UP_APP = 0x32,                   // APP����д��
    STS_PROTOCOL_CMD_TOOL_UP_END = 0x33,                   // APP��������
    STS_PROTOCOL_CMD_TOOL_SET_DUT_COMM_PARAM = 0x45,       // ����DUTͨѶ����
    STS_PROTOCOL_CMD_MULTIPLE_TEST = 0x0B,                 // �������

    STS_PROTOCOL_CMD_GND_TEST = 0x04,                      // �ϵ����
    STS_PROTOCOL_CMD_KEY_TEST = 0x05,                      // ��������
    STS_PROTOCOL_CMD_USB_CHARGE_TEST = 0x14,               // USB�����ԣ��������ã�
    STS_PROTOCOL_CMD_LIGHT_SENSING_TEST = 0x0E,            // ��������
    STS_PROTOCOL_CMD_DISPLAY_TEST = 0x06,                  // �ʶ���/�������ʾ����
    STS_PROTOCOL_CMD_BUZZER_TEST = 0x07,                   // ���������Ʋ��ԣ�������=0x07��
    
    STS_PROTOCOL_CMD_VOLTAGE_CALIBRATION = 0x15,           // ��ѹУ׼���༶У׼��
    STS_PROTOCOL_CMD_ONLINE_DETECTION = 0x18,              // ���߼�⣨�������ϵ�״̬��
    STS_PROTOCOL_CMD_UART_TEST = 0x19,                     // UART����
    STS_PROTOCOL_CMD_BLE_TEST = 0x0C,                      // ��������
    STS_PROTOCOL_CMD_ADJUST_DUT_VOLTAGE = 0x16,            // ����DUT�����ѹ
    STS_PROTOCOL_CMD_WRITE_FLAG_DATA = 0xA7,               // д��־������
    STS_PROTOCOL_CMD_EXECUTIVE_DIRECTOR = 0x1C,            // �����ű�
    STS_PROTOCOL_CMD_FLASH_CHECK_TEST = 0x09,              // Flash У�����
    STS_PROTOCOL_CMD_CONTROL_DUT_POWER = 0x1A,             // DUT��Դ����
    STS_PROTOCOL_CMD_OBTAIN_WAKE_UP_VOL = 0x1D,            // ��ȡWAKE��ѹ  
    STS_PROTOCOL_CMD_SIMULATION_KEY_TEST = 0x1E,            // ģ�ⰴ������    
    STS_PROTOCOL_CMD_OBTAIN_DUT_CURRENT = 0x1F,            // ��ȡDUT�������
    
    STS_PROTOCOL_CMD_TOOL_DUT_UP = 0x40,                   // ����DUT��������
    STS_PROTOCOL_CMD_TOOL_DUT_PROCESS = 0x41,              // DUT���������ϱ�
    STS_PROTOCOL_CMD_TOOL_CLEAR_BUFF = 0x42,               // �������������
    STS_PROTOCOL_CMD_TOOL_SET_DUT_INFO = 0x43,             // DTAд��DUT������Ϣ
    STS_PROTOCOL_CMD_TOOL_GET_DUT_INFO = 0x44,             // ��ȡDTA��¼��DUT������Ϣ
    STS_PROTOCOL_CMD_UP_CONFIG = 0x50,                     // д��DUT���ò���
    STS_PROTOCOL_CMD_UP_BOOT = 0x51,                       // BOOT����д��
    STS_PROTOCOL_CMD_UP_APP = 0x52,                        // APP����д��
    STS_PROTOCOL_CMD_UP_END = 0x53,                        // ���������
    STS_PROTOCOL_CMD_SET_NOT_WRITTEN = 0x46,               // ���ò�д����
    STS_PROTOCOL_CMD_POWER_GET_SOFTWARE_INFO = 0x35,       // ��ȡPOWER�����Ϣ
    STS_PROTOCOL_CMD_POWER_UP_APP_ERASE= 0x36,             // POWER-APP���ݲ���
    STS_PROTOCOL_CMD_POWER_UP_APP_WRITE = 0x37,            // POWER-APP����д��
    STS_PROTOCOL_CMD_POWER_UP_APP_WRITE_FINISH = 0x38,     // POWER-APP����д�����
    STS_PROTOCOL_CMD_POWER_UP_FINISH = 0x39,               // ����POWER-APP�ɹ�Ӧ��

    STS_PROTOCOL_CMD_TOOL_SET_TRAN_DUT_COMM_PARAM = 0x47,       // ����͸��DUTͨѶ����
    STS_PROTOCOL_CMD_TRAN_DUT_INSTRUCTION_CONTROL = 0x48,       // ͸��DUTָ�����   
    
    STS_CMD_MAX
} STS_PROTOCOL_CMD;

// UART_RX����֡����
typedef struct
{
    uint16 deviceID;
    uint8 buff[STS_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX]; // ����֡������
    uint16 length;                                    // ����֡��Ч���ݸ���
} STS_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
    uint16 deviceID;
    uint8 buff[STS_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // ����֡������
    uint16 length;                                    // ����֡��Ч���ݸ���
} STS_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
    // һ�����ջ�����
    struct
    {
        volatile uint8 buff[STS_PROTOCOL_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex; // ��ǰ��������ֽڵ�λ���±�
    } rxFIFO;

    // ����֡���������ݽṹ
    struct
    {
        STS_PROTOCOL_RX_CMD_FRAME cmdQueue[STS_PROTOCOL_RX_QUEUE_SIZE];
        uint16 head; // ����ͷ����
        uint16 end;  // ����β����
    } rx;

    // ����֡���������ݽṹ
    struct
    {
        STS_PROTOCOL_TX_CMD_FRAME cmdQueue[STS_PROTOCOL_TX_QUEUE_SIZE];
        uint16 head;  // ����ͷ����
        uint16 end;   // ����β����
        uint16 index; // ��ǰ����������������֡�е�������
        BOOL txBusy;  // ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
    } tx;

    // �������ݽӿ�
    BOOL(*sendDataThrowService)
    (uint16 id, uint8 *pData, uint16 length);

    uint32 ageFlag;
    BOOL isTimeCheck;
} STS_PROTOCOL_CB;

// ��ѹУ׼ֵ
typedef struct
{
    uint8 number;
    uint8 data[3];
} CALIBRATION;

extern STS_PROTOCOL_CB STSProtocolCB;

/******************************************************************************
 * ���ⲿ�ӿ�������
 ******************************************************************************/

// Э���ʼ��
void STS_PROTOCOL_Init(void);

// Э�����̴���
void STS_PROTOCOL_Process(void);

// ��������֡�������������
void STS_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void STS_PROTOCOL_TxAddFrame(void);

// ������STS������֡ͨ��UARTת����DUT EG��StsRxFrame_ToDut_Uart_Transmit��pCmdFrame��
void StsRxFrame_ToDut_Uart_Transmit(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

void StsRxFrame_ToDut_Can_Transmit(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// ��������ظ�����һ������
void STS_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// ���������DUT����һ������
void DUT_Serial_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

void StsRxFrame_ToDut_Transmit(STS_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// DUTͨѶ��ʱ����
void DutTimeOut(uint32 param);

// д���־��ʱ����
void WriteFlagTimeOut(uint32 param);

void OperateVerTimeOut(uint32 param);

void enterState(void);

// ����ϱ�
void Report_Identity(uint32 param);

// ��������ظ�
void STS_PROTOCOL_SendCmdAck(uint8 ackCmd);

// ��������ظ�������������
void STS_PROTOCOL_SendCmdParamTwoAck(uint8 ackCmd, uint8 ackParam, uint8 twoParam);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
extern CALIBRATION calibration;
extern BOOL STS_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length);
extern uint8 stationNumber;
extern char dutverBuff[100];
extern uint8_t dutverType;
extern BOOL verwriteFlag;
extern BOOL verreadFlag;
extern uint8 resultArray[150];  // ���ڴ洢�����һά����
extern uint8 verifiedBuff[150];
extern uint8 verifiedIndex;
extern uint8 configs[300];
extern uint8 noLenght ;
extern uint8 noNumber ;
extern BOOL testFlag ;
extern uint8 verBle[20];
extern uint8 keyValue;

#endif
