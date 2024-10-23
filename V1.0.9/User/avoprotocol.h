#ifndef __AVO_PROTOCOL_H__
#define __AVO_PROTOCOL_H__

#include "common.h"

////==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
//// UART����ͨѶ����ʱ��
//#define UART_PROTOCOL_COMMUNICATION_TIME_OUT      10000UL // ��λ:ms
//#define UART_PROTOCOL_CMD_SEND_TIME                   500     // ���������

//=====================================================================================================
#define AVO_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE       0       // ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define AVO_PROTOCOL_RX_QUEUE_SIZE 3 // ����������гߴ�
#define AVO_PROTOCOL_TX_QUEUE_SIZE 3 // ����������гߴ�

#define AVO_PROTOCOL_CMD_HEAD1                      0x55        // ����ͷ
#define AVO_PROTOCOL_CMD_HEAD2                      0x00        // ����ͷ1
#define AVO_PROTOCOL_CMD_HEAD3                      0xAA        // ����ͷ2

#define AVO_PROTOCOL_CMD_PROTOCOL_VERSION           0x01    // Э��汾
#define AVO_PROTOCOL_CMD_DEVICE_ADDR                0x10    // �豸��
#define AVO_PROTOCOL_CMD_NONHEAD                    0xFF    // ������ͷ
#define AVO_PROTOCOL_HEAD_BYTE                      3   // ����ͷ�ֽ���
#define AVO_PROTOCOL_CHECK_BYTE                     1   // У���ֽ�

#define AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 160 // ���������֡����
#define AVO_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 160 // ����������֡����

#define AVO_PROTOCOL_RX_FIFO_SIZE 200            // ����һ����������С
#define AVO_PROTOCOL_CMD_FRAME_LENGTH_MIN 6      // ����֡��С���ȣ�����:5������ͷ��Э��汾���豸�š������֡�2�����ݳ��ȡ�У����

// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define AVO_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / AVO_UART_BAUD_RATE * 10 * AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)      // ��λ:ms
#define AVO_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / AVO_UART_BAUD_RATE * 10 * AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // ��λ:ms

// UART����Э�鶨��
typedef enum
{
    AVO_PROTOCOL_CMD_HEAD1_INDEX = 0,                 // ֡ͷ1����
    AVO_PROTOCOL_CMD_HEAD2_INDEX,                     // ֡ͷ2����
    AVO_PROTOCOL_CMD_HEAD3_INDEX,                     // ֡ͷ3����
    AVO_PROTOCOL_CMD_INDEX,                           /* ���������� */
    AVO_PROTOCOL_CMD_LENGTH_INDEX,                    /* ���ݳ��� */
    AVO_PROTOCOL_CMD_BOARD_INDEX,                     /* Ŀ��忨�� ���忨��ţ�*/

    AVO_PROTOCOL_CMD_DATA1_INDEX,                     // ��������(��������)
    AVO_PROTOCOL_CMD_DATA2_INDEX,                     // ��������(����ͨ��)
    AVO_PROTOCOL_CMD_DATA3_INDEX,                     // ��������(�������byte1)
    AVO_PROTOCOL_CMD_DATA4_INDEX,                     // ��������(�������byte2)
    AVO_PROTOCOL_CMD_DATA5_INDEX,                     // ��������(�������byte3)
    AVO_PROTOCOL_CMD_DATA6_INDEX,                     // ��������(�������byte4)
    AVO_PROTOCOL_CMD_DATA7_INDEX,                     // ��������
    AVO_PROTOCOL_CMD_DATA8_INDEX,                     // ��������
    AVO_PROTOCOL_CMD_DATA9_INDEX,                     // ��������
    AVO_PROTOCOL_CMD_DATA10_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA11_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA12_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA13_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA14_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA15_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA16_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA17_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA18_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA19_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA20_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA21_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA22_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA23_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_DATA24_INDEX,                    // ��������
    AVO_PROTOCOL_CMD_INDEX_MAX

} AVO_PROTOCOL_DATE_FRAME;


/*������*/
typedef enum
{
    BOARD_ARM = 0X00,  /*ARM����*/
    BOARD_MOM = 0X01,  /*ĸ�����*/
    BOARD_COM,         /*�ӿڰ����*/
    BOARD_PRO,         /*��¼������*/
    BOARD_AVO,         /*���ñ����*/
    BOARD_MAX
} AVO_PROTOCOL_BDNUM;

// ��������
typedef enum
{
    AVO_CMD_NULL        = 0xFF,

    //���ñ��������
    AVO_CMD_AVOMETER = 0xb0,
    AVO_CMD_AVOMETER_ACK = 0xb0,
    // ���ñ�λ����
    AVO_CMD_AVORESET = 0xb1,
    AVO_CMD_AVORESET_ACK = 0xb1,

    // ��ͷ��λͨ������
    //AVO_CMD_MEMET_CAPTURE_GET_GEARS = 0x50,           // ��ѯ��ͷ��λ����
    //AVO_CMD_MEMET_CAPTURE_GET_GEARS_ACK = 0x50,       // ��ѯ��ͷ��λ����Ӧ��

    AVO_PROTOCOL_CMD_MAX                                 // ��������
} AVO_PROTOCOL_CMD;


// ���ñ���Ե�λö��
typedef enum
{

    AVO_GEARS_DCV = 0x01,
    AVO_GEARS_ACV,
    AVO_GEARS_DCMA,
    AVO_GEARS_ACMA,
    AVO_GEARS_OHM,
    AVO_GEARS_DCA,
    AVO_GEARS_ACA,
    AVO_GEARS_MAX,
} AVO_GEARS_E;

typedef enum
{
    MEASURE_DCV   = 1, //ֱ����ѹ��V��
    MEASURE_ACV,       //������ѹ��V��
    MEASURE_OHM,       //���裨����
    MEASURE_DCMA,      //ֱ��������mA��
    MEASURE_ACMA,      //����������mA��
    MEASURE_DCA,       //ֱ��������A��
    MEASURE_ACA,       //����������A��
} AVO_MEASURE_TYPE;

// UART_RX����֡����
typedef struct
{
    uint16 deviceID;
    uint8 buff[AVO_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX]; // ����֡������
    uint16 length;                                    // ����֡��Ч���ݸ���
} AVO_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
    uint16 deviceID;
    uint8 buff[AVO_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // ����֡������
    uint16 length;                                    // ����֡��Ч���ݸ���
} AVO_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
    // һ�����ջ�����
    struct
    {
        volatile uint8 buff[AVO_PROTOCOL_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex; // ��ǰ��������ֽڵ�λ���±�
    } rxFIFO;

    // ����֡���������ݽṹ
    struct
    {
        AVO_PROTOCOL_RX_CMD_FRAME cmdQueue[AVO_PROTOCOL_RX_QUEUE_SIZE];
        uint16 head; // ����ͷ����
        uint16 end;  // ����β����
    } rx;

    // ����֡���������ݽṹ
    struct
    {
        AVO_PROTOCOL_TX_CMD_FRAME cmdQueue[AVO_PROTOCOL_TX_QUEUE_SIZE];
        uint16 head;  // ����ͷ����
        uint16 end;   // ����β����
        uint16 index; // ��ǰ����������������֡�е�������
        BOOL txBusy;  // ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
    } tx;

    // �������ݽӿ�
    BOOL(*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

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

    uint32 ageFlag;
    BOOL isTimeCheck;
} AVO_PROTOCOL_CB;

extern AVO_PROTOCOL_CB AVOProtocolCB;

/******************************************************************************
 * ���ⲿ�ӿ�������
 ******************************************************************************/

// Э���ʼ��
void AVO_PROTOCOL_Init(void);

// Э�����̴���
void AVO_PROTOCOL_Process(void);

// ��������֡�������������
void AVO_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void AVO_PROTOCOL_TxAddFrame(void);

// �·�������������, ͨ��Ĭ��Ϊ0
void AVO_PROTOCOL_Send_Mesuretype(uint32 mesuretype);

// �·��������ͺͲ���ͨ������
void AVO_PROTOCOL_Send_Type_Chl(uint32 mesuretype, uint32 mesurechl);

// �·���λ����
void AVO_PROTOCOL_Send_Reset(void);
#endif

/******************************************************************************
 * ���ⲿ���ݻ�ȡ��
 ******************************************************************************/

typedef union
{
    uint8 data[4];
    float result;
} MEASURE_RESULT;

typedef union
{
   uint8_t data[4];
   int INT;
} OHM;

extern MEASURE_RESULT measure;
extern uint8 headlight_cnt ;
extern uint8 gnd_cnt ;
extern OHM ohm;
