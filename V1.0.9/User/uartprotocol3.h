/********************************************************************************************************
        BigStone_V9.2Э��
********************************************************************************************************/
#ifndef     __UART_PROTOCOL4_H__
#define     __UART_PROTOCOL4_H__

#include "common.h"

#define SMOOTH_BASE_TIME                            100     // ƽ����׼ʱ�䣬��λ:ms

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define UART_PROTOCOL4_COMMUNICATION_TIME_OUT       10000UL // ��λ:ms

// P��λ
#define PROTOCOL_ASSIST_P                           ASSIST_P

#define UART_PROTOCOL4_CMD_SEND_TIME                500     // �����ʱ�䣬��λ:ms

//=====================================================================================================
#define UART_PROTOCOL4_RX_TIME_OUT_CHECK_ENABLE     1       // ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define UART_PROTOCOL4_RX_QUEUE_SIZE                3       // ����������гߴ�
#define UART_PROTOCOL4_TX_QUEUE_SIZE                3      // ����������гߴ�

#define UART_PROTOCOL4_CMD_HEAD                     0x3A    // ����ͷ

#define UART_PROTOCOL4_RX_CMD_HMI_CTRL_RUN_LENGTH_MAX           150      // ������֡�����ճ���
#define UART_PROTOCOL4_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX    150     // ������֡�����ճ���
#define UART_PROTOCOL4_RX_CMD_READ_CONTROL_PARAM_LENGTH_MAX     150       // ������֡�����ճ���

#define UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX      150     // ���������֡����
#define UART_PROTOCOL4_RX_CMD_FRAME_LENGTH_MAX      150     // ����������֡����

#define UART_PROTOCOL4_RX_FIFO_SIZE                 200     // ����һ����������С
#define UART_PROTOCOL4_CMD_FRAME_LENGTH_MIN         8       // ����֡��С���ȣ�����:��ʼ����������ֵ��������Ϣ�����У��


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define UART_PROTOCOL4_BUS_UNIDIRECTIONAL_TIME_OUT  (uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL4_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)        // ��λ:ms
#define UART_PROTOCOL4_BUS_BIDIRECTIONAL_TIME_OUT   (uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL4_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)    // ��λ:ms

// UART����Э�鶨��
typedef enum
{

    UART_PROTOCOL4_CMD_HEAD_INDEX = 0,                  // ֡ͷ����
    UART_PROTOCOL4_CMD_DEVICE_ADDR_INDEX,               // �豸��ַ
    UART_PROTOCOL4_CMD_CMD_INDEX,                       // ����������
    UART_PROTOCOL4_CMD_LENGTH_INDEX,                    // ���ݳ���

    UART_PROTOCOL4_CMD_DATA1_INDEX,                     // ��������
    UART_PROTOCOL4_CMD_DATA2_INDEX,                     // ��������
    UART_PROTOCOL4_CMD_DATA3_INDEX,                     // ��������
    UART_PROTOCOL4_CMD_DATA4_INDEX,                     // ��������
    UART_PROTOCOL4_CMD_DATA5_INDEX,                     // ��������
    UART_PROTOCOL4_CMD_DATA6_INDEX,                     // ��������
    UART_PROTOCOL4_CMD_DATA7_INDEX,                     // ��������
    UART_PROTOCOL4_CMD_DATA8_INDEX,                     // ��������
    UART_PROTOCOL4_CMD_DATA9_INDEX,                     // ��������
    UART_PROTOCOL4_CMD_DATA10_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA11_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA12_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA13_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA14_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA15_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA16_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA17_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA18_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA19_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA20_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA21_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA22_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA23_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA24_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA25_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA26_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA27_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA28_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA29_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA30_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA31_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA32_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA33_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA34_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA35_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA36_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA37_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA38_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA39_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA40_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA41_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA42_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA43_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA44_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA45_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA46_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA47_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA48_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA49_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA50_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA51_INDEX,                    // ��������
    UART_PROTOCOL4_CMD_DATA52_INDEX,                    // ��������

    UART_PROTOCOL4_CMD_INDEX_MAX


} UART_PROTOCOL4_DATE_FRAME;

// ��������
typedef enum
{
    UART_PROTOCOL4_CMD_NULL = 0,                                // ������

    UART_PROTOCOL4_CMD_CONFIG_SET = 0x53,                       // ��������
    UART_PROTOCOL4_CMD_WRITE_CONTROL_PARAM = 0xC0,              // д����������
    UART_PROTOCOL4_CMD_WRITE_CONFIG_RESULT = 0xC1,              // д�������������Ӧ��
    UART_PROTOCOL4_CMD_READ_CONTROL_PARAM = 0xC2,               // ��ȡ����������
    UART_PROTOCOL4_CMD_READ_CONTROL_PARAM_REPORT = 0xC3,        // �����������ϱ�
    UART_PROTOCOL4_CMD_PROTOCOL_SWITCCH = 0xAB,                 // Э���л�����
    UART_PROTOCOL4_CMD_NEW_WRITE_CONTROL_PARAM = 0x50,          // д�����ò���

    UART_PROTOCOL4_CMD_MAX                                      // ��������
} UART_PROTOCOL4_CMD;

// UART_RX����֡����
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL4_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX]; // ����֡������
    uint16  length;                                         // ����֡��Ч���ݸ���
} UART_PROTOCOL4_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL4_TX_CMD_FRAME_LENGTH_MAX];   // ����֡������
    uint16  length;                                         // ����֡��Ч���ݸ���
} UART_PROTOCOL4_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
    // һ�����ջ�����
    struct
    {
        volatile uint8  buff[UART_PROTOCOL4_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex;                 // ��ǰ��������ֽڵ�λ���±�

        uint16 curCmdFrameLength;                   // ��ǰ����֡����
    } rxFIFO;

    // ����֡���������ݽṹ
    struct
    {
        UART_PROTOCOL4_RX_CMD_FRAME cmdQueue[UART_PROTOCOL4_RX_QUEUE_SIZE];
        uint16          head;                       // ����ͷ����
        uint16          end;                        // ����β����
    } rx;

    // ����֡���������ݽṹ
    struct
    {
        UART_PROTOCOL4_TX_CMD_FRAME cmdQueue[UART_PROTOCOL4_TX_QUEUE_SIZE];
        uint16  head;                       // ����ͷ����
        uint16  end;                        // ����β����
        uint16  index;                      // ��ǰ����������������֡�е�������
        BOOL    txBusy;                     // ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
    } tx;

    // �������ݽӿ�
    BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

    BOOL txPeriodRequest;       // ��������Է�����������
    BOOL txAtOnceRequest;       // ������Ϸ�����������

    BOOL paramSetOK;

    // ƽ���㷨���ݽṹ
    struct
    {
        uint16 realSpeed;
        uint16 proSpeed;
        uint16 difSpeed;
    } speedFilter;
} UART_PROTOCOL4_CB;

extern UART_PROTOCOL4_CB uartProtocolCB4;

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void UART_PROTOCOL4_Init(void);

// ��������֡�������������
void UART_PROTOCOL4_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void UART_PROTOCOL4_TxAddFrame(void);

// Э�����̴���
void UART_PROTOCOL_Process(void);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL4_RxFIFOProcess(UART_PROTOCOL4_CB *pCB);

// UART����֡����������
void UART_PROTOCOL4_CmdFrameProcess(UART_PROTOCOL4_CB *pCB);

// Э��㷢�ʹ������
void UART_PROTOCOL4_TxStateProcess(void);

//==================================================================================
// �����·������־���������ò���Ҳ�ǵ��ô˽ӿ�
void UART_PROTOCOL4_SetTxAtOnceRequest(uint32 param);
/*
// ����������Ͷ���
typedef enum
{
    ERROR_TYPE_NO_ERROR = 0,                    // �޴���
    ERROR_TYPE_CURRENT_ERROR = 0x21,            // �����쳣
    ERROR_TYPE_THROTTLE_ERROR = 0x22,           // ת�ѹ���
    ERROR_TYPE_MOTOR_PHASE_ERROR = 0x23,        // ������߹���
    ERROR_TYPE_HALL_SENSOR_ERROR = 0x24,        // ��������źŹ���
    ERROR_TYPE_BRAKE_ERROR = 0x25,              // ɲ������
    ERROR_TYPE_LOW_VOLTAGE_ERROR = 0x26,        // Ƿѹ����
    ERROR_TYPE_COMMUNICATION_TIME_OUT = 0x30,   // ͨѶ��ʱ

}ERROR_TYPE_E;
*/
// ���� ����·�����������ϵͳ���� ����
typedef struct
{
    uint8 assitLevel : 4;                   // ��λֵ��0-9
    uint8 isPushModeOn : 1;                 // 6km/h���й��ܣ�0:��ʾ�رգ�1:��ʾ��
    uint8 reserveByte1Bit65 : 2;
    uint8 isLightOn : 1;                    // �ƹ����

    uint8 wheelSizeCode : 3;                // �־�����
    uint8 speedLimitCode : 5;               // ����ֵ����

} UART_PROTOCOL4_SET_PARAM_CB;
#define UART_PROTOCOL4_SET_PARAM_SIZE   (sizeof(UART_PROTOCOL4_SET_PARAM_CB))

// ���� ����·�����������ʵʱ���� ����
typedef struct
{
    uint8 assitLevel : 4;                   // ��λֵ��0-9
    uint8 isPushModeOn : 1;                 // 6km/h���й��ܣ�0:��ʾ�رգ�1:��ʾ��
    uint8 reserveByte1Bit65 : 2;
    uint8 isLightOn : 1;                    // �ƹ����

    uint8 wheelSizeCode : 3;                // �־�����
    uint8 speedLimitCode : 5;               // ����ֵ����
} UART_PROTOCOL4_RUNNING_PARAM_CB;
#define UART_PROTOCOL4_RUNNING_PARAM_SIZE   (sizeof(UART_PROTOCOL4_RUNNING_PARAM_CB))

// ���� �������ϱ������Ĳ��� ����
typedef struct
{
    uint8 batteryVoltageLevel;              // ��ѹ����

    uint8 batteryCurrent;                   // ��ǰ��ص�������λ:1/3A

    uint8 oneCycleTimeH;                    // ��ǰһȦʱ�䣬���ֽ�
    uint8 oneCycleTimeL;                    // ��ǰһȦʱ�䣬���ֽ�

    uint8 xorAdjVal;                        // ������ֵ

    uint8 ucErrorCode;                      // �������

} UART_PROTOCOL4_RX_PARAM_CB;
#define UART_PROTOCOL4_RX_PARAM_SIZE            (sizeof(UART_PROTOCOL4_RX_PARAM_CB))


// �������������ݽ����ṹ�嶨��
typedef struct
{
    // ��������������
    union
    {
        uint8 buff[UART_PROTOCOL4_SET_PARAM_SIZE];
        UART_PROTOCOL4_SET_PARAM_CB param;
    } set;

    // ���������в���
    union
    {
        uint8 buff[UART_PROTOCOL4_RUNNING_PARAM_SIZE];
        UART_PROTOCOL4_RUNNING_PARAM_CB param;
    } running;

    // �������ϱ������Ĳ���
    union
    {
        uint8 buff[UART_PROTOCOL4_RX_PARAM_SIZE];
        UART_PROTOCOL4_RX_PARAM_CB param;
    } rx;
} UART_PROTOCOL4_PARAM_CB;

extern UART_PROTOCOL4_PARAM_CB hmiDriveCB3;


#endif


