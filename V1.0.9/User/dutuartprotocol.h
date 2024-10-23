/********************************************************************************************************
        BigStone_V9.2Э��
********************************************************************************************************/
#ifndef     __DUTUART_PROTOCOL_H__
#define     __DUTUART_PROTOCOL_H__

#include "common.h"

#define SMOOTH_BASE_TIME                            100     // ƽ����׼ʱ�䣬��λ:ms

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define DUT_PROTOCOL_COMMUNICATION_TIME_OUT     10000UL // ��λ:ms

#define LEFT_TURN_SIGNAL       0
#define RIGHT_TURN_SIGNAL      1
#define LBEAM                  2
#define HBEAM                  3

#define DUT_PROTOCOL_CMD_SEND_TIME              500     // �����ʱ�䣬��λ:ms

//=====================================================================================================
#define DUT_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE       1       // ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define DUT_PROTOCOL_RX_QUEUE_SIZE              5       // ����������гߴ�
#define DUT_PROTOCOL_TX_QUEUE_SIZE              5       // ����������гߴ�

#define DUT_PROTOCOL_CMD_HEAD                       0x55    // ����ͷ

#define DUT_PROTOCOL_RX_CMD_HMI_CTRL_RUN_LENGTH_MAX         8       // ������֡�����ճ���
#define DUT_PROTOCOL_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX  80      // ������֡�����ճ���
#define DUT_PROTOCOL_RX_CMD_READ_CONTROL_PARAM_LENGTH_MAX       4       // ������֡�����ճ���

#define DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX        150     // ���������֡����
#define DUT_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX        150     // ����������֡����

#define DUT_PROTOCOL_RX_FIFO_SIZE                   200     // ����һ����������С
#define DUT_PROTOCOL_CMD_FRAME_LENGTH_MIN           4       // ����֡��С���ȣ�����:��ʼ����������ֵ��������Ϣ�����У��


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define DUT_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT    (uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*DUT_PROTOCOL_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)      // ��λ:ms
#define DUT_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*DUT_PROTOCOL_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)  // ��λ:ms

// UART����Э�鶨��
typedef enum
{

    DUT_PROTOCOL_CMD_HEAD_INDEX = 0,                // ֡ͷ����
    DUT_PROTOCOL_CMD_CMD_INDEX,                     // ����������
    DUT_PROTOCOL_CMD_LENGTH_INDEX,                  // ���ݳ���

    DUT_PROTOCOL_CMD_DATA1_INDEX,                       // ��������
    DUT_PROTOCOL_CMD_DATA2_INDEX,                       // ��������
    DUT_PROTOCOL_CMD_DATA3_INDEX,                       // ��������
    DUT_PROTOCOL_CMD_DATA4_INDEX,                       // ��������
    DUT_PROTOCOL_CMD_DATA5_INDEX,                       // ��������
    DUT_PROTOCOL_CMD_DATA6_INDEX,                       // ��������
    DUT_PROTOCOL_CMD_DATA7_INDEX,                       // ��������
    DUT_PROTOCOL_CMD_DATA8_INDEX,                       // ��������
    DUT_PROTOCOL_CMD_DATA9_INDEX,                       // ��������
    DUT_PROTOCOL_CMD_DATA10_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA11_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA12_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA13_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA14_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA15_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA16_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA17_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA18_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA19_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA20_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA21_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA22_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA23_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA24_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA25_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA26_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA27_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA28_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA29_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA30_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA31_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA32_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA33_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA34_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA35_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA36_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA37_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA38_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA39_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA40_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA41_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA42_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA43_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA44_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA45_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA46_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA47_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA48_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA49_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA50_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA51_INDEX,                  // ��������
    DUT_PROTOCOL_CMD_DATA52_INDEX,                  // ��������

    DUT_PROTOCOL_CMD_INDEX_MAX


} DUT_PROTOCOL_DATE_FRAME;

// ��������
typedef enum
{
    DUT_PROTOCOL_CMD_NULL = 0,                              // ������

    DUT_PROTOCOL_CMD_HEADLIGHT_CONTROL = 0xEA,              // ��ƿ���
    DUT_PROTOCOL_CMD_GET_THROTTLE_BRAKE_AD = 0xEB,          // ��ȡ����/ɲ����ģ������ֵ
    DUT_PROTOCOL_CMD_TURN_SIGNAL_CONTROL = 0xEC,            // ת��ƿ���
    DUT_PROTOCOL_CMD_ONLINE_DETECTION = 0xED,               // ���߼��
    DUT_PROTOCOL_CMD_GET_PHOTORESISTOR_VALUE = 0x90,        // ��ȡ��������������ֵ
    DUT_PROTOCOL_CMD_VOLTAGE_CALIBRATION = 0x91,            // ���е�ѹУ׼
    DUT_PROTOCOL_CMD_KEY_TEST = 0x05,                       // ��������

    DUT_PROTOCOL_CMD_READ_FLAG_DATA = 0xA6,               // ��ȡ��־������
    DUT_PROTOCOL_CMD_WRITE_FLAG_DATA = 0xA7,              // д���־������
    DUT_PROTOCOL_CMD_WRITE_VERSION_TYPE_DATA = 0xA8,      // д��汾��������
    DUT_PROTOCOL_CMD_READ_VERSION_TYPE_INFO = 0xA9,       // ��ȡ�汾������Ϣ

    DUT_PROTOCOL_CMD_BLUETOOTH_MAC_ADDRESS_READ = 0x0C,   // ��������
    DUT_PROTOCOL_CMD_TEST_LCD = 0x03,                     // LCD��ɫ���ԣ�������=0x03��
    DUT_PROTOCOL_CMD_FLASH_CHECK_TEST = 0x09,             // Flash У�����

    DUT_PROTOCOL_CMD_MAX                                  // ��������
} DUT_PROTOCOL_CMD;

// UART_RX����֡����
typedef struct
{
    uint16  deviceID;
    uint8   buff[DUT_PROTOCOL_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX];   // ����֡������
    uint16  length;                                         // ����֡��Ч���ݸ���
} DUT_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
    uint16  deviceID;
    uint8   buff[DUT_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // ����֡������
    uint16  length;                                         // ����֡��Ч���ݸ���
} DUT_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
    // һ�����ջ�����
    struct
    {
        volatile uint8  buff[DUT_PROTOCOL_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex;                 // ��ǰ��������ֽڵ�λ���±�

        uint16 curCmdFrameLength;                   // ��ǰ����֡����
    } rxFIFO;

    // ����֡���������ݽṹ
    struct
    {
        DUT_PROTOCOL_RX_CMD_FRAME   cmdQueue[DUT_PROTOCOL_RX_QUEUE_SIZE];
        uint16          head;                       // ����ͷ����
        uint16          end;                        // ����β����
    } rx;

    // ����֡���������ݽṹ
    struct
    {
        DUT_PROTOCOL_TX_CMD_FRAME   cmdQueue[DUT_PROTOCOL_TX_QUEUE_SIZE];
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
} DUT_PROTOCOL_CB;

extern DUT_PROTOCOL_CB uartProtocolCB3;
extern BOOL write_success_Flag;
/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void DUT_PROTOCOL_Init(void);

// ��������֡�������������
void DUT_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void DUT_PROTOCOL_TxAddFrame(void);

// UARTЭ�����̴���
void DUT_UART_PROTOCOL_Process(void);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void DUT_PROTOCOL_RxFIFOProcess(DUT_PROTOCOL_CB *pCB);

// UART����֡����������
void DUT_PROTOCOL_CmdFrameProcess(DUT_PROTOCOL_CB *pCB);

// Э��㷢�ʹ������
void DUT_PROTOCOL_TxStateProcess(void);

// ��������ظ�
void DUT_PROTOCOL_SendCmdAck(uint8 ackCmd);

// ��������ظ�����һ������
void DUT_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// ��������ظ���������������
void DUT_PROTOCOL_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam);

// ������DUT������֡ͨ��UARTת����STS
void DutRxFrame_ToSts_Transmit(DUT_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// �ϱ�����״̬
void DUT_PROTOCOL_SendOnlineStart(uint32 param);

// д���־������
void DUT_PROTOCOL_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void DUT_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);
////////////////////////////////////////////////////////////////////////////////

// ����
typedef union
{
    uint16 data[2];
    uint16 value;
} THROTTLE;

// ɲ��
typedef union
{
    uint16 data[2];
    uint16 value;
} BRAKE;

// ���ӱ���
typedef union
{
    uint16 data[2];
    uint16 value;
} DERAILLEUR;

extern uint8 light_cnt;
extern uint8 left_light_cnt ;
extern uint8 right_light_cnt ;
extern uint8 cali_cnt;
extern uint32 online_detection_cnt ;
extern THROTTLE throttle;
extern BRAKE brake;
#endif


