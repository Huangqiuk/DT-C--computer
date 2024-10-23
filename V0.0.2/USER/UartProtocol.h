/********************************************************************************************************
        KM5S�������ͨѶЭ��
********************************************************************************************************/
#ifndef     __UART_PROTOCOL_H__
#define     __UART_PROTOCOL_H__

#include "common.h"

#define VOLTAGE_ERROR                 700   // ���յ����ֵ����λ:mv 
#define CURRENT_ERROR                 700   // ���յ����ֵ����λ:ma  

// ��ѹֵ
#define VOLTAGE_5V 5000
#define VOLTAGE_9V 9000
#define VOLTAGE_12V 12000
#define VOLTAGE_24V 24000
#define VOLTAGE_36V 36000
#define VOLTAGE_48V 48000
#define VOLTAGE_60V 60000
#define VOLTAGE_72V 72000

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define UART_PROTOCOL_COMMUNICATION_TIME_OUT        10000UL // ��λ:ms

// P��λ
#define PROTOCOL_ASSIST_P                           ASSIST_P

#define UART_PROTOCOL_CMD_SEND_TIME                 500     // ���������

//=====================================================================================================
#define UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE      0       // ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define UART_PROTOCOL_RX_QUEUE_SIZE                 3       // ����������гߴ�
#define UART_PROTOCOL_TX_QUEUE_SIZE                 3       // ����������гߴ�

#define UART_PROTOCOL_CMD_HEAD                      0x3A    // ����ͷ
#define UART_PROTOCOL_CMD_DEVICE_ADDR               0x1A    // �豸��ַ

#define UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX       80      // ���������֡����
#define UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX       80      // ����������֡����
#define UART_PROTOCOL_RX_FIFO_SIZE                  200     // ����һ����������С
#define UART_PROTOCOL_CMD_FRAME_LENGTH_MIN          8       // ����֡��С���ȣ�����:����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT   (uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)       // ��λ:ms
#define UART_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT    (uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)   // ��λ:ms

// UART����Э�鶨��
typedef enum
{
    UART_PROTOCOL_CMD_HEAD_INDEX = 0,                   // ֡ͷ����
    UART_PROTOCOL_CMD_DEVICE_ADDR_INDEX,                // �豸��ַ
    UART_PROTOCOL_CMD_CMD_INDEX,                        // ����������
    UART_PROTOCOL_CMD_LENGTH_INDEX,                     // ���ݳ���

    UART_PROTOCOL_CMD_DATA1_INDEX,                      // ��������
    UART_PROTOCOL_CMD_DATA2_INDEX,                      // ��������
    UART_PROTOCOL_CMD_DATA3_INDEX,                      // ��������
    UART_PROTOCOL_CMD_DATA4_INDEX,                      // ��������
    UART_PROTOCOL_CMD_DATA5_INDEX,                      // ��������
    UART_PROTOCOL_CMD_DATA6_INDEX,                      // ��������
    UART_PROTOCOL_CMD_DATA7_INDEX,                      // ��������
    UART_PROTOCOL_CMD_DATA8_INDEX,                      // ��������
    UART_PROTOCOL_CMD_DATA9_INDEX,                      // ��������
    UART_PROTOCOL_CMD_DATA10_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA11_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA12_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA13_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA14_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA15_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA16_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA17_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA18_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA19_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA20_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA21_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA22_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA23_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA24_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA25_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA26_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA27_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA28_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA29_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA30_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA31_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA32_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA33_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA34_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA35_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA36_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA37_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA38_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA39_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA40_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA41_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA42_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA43_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA44_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA45_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA46_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA47_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA48_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA49_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA50_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA51_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA52_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA53_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA54_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA55_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA56_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA57_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA58_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA59_INDEX,                     // ��������
    UART_PROTOCOL_CMD_DATA60_INDEX,                     // ��������

    UART_PROTOCOL_CMD_INDEX_MAX
} UART_PROTOCOL_DATE_FRAME;

// ��������
typedef enum
{
	UART_ECO_CMD_NULL = 0,							 
	UART_ECO_CMD_RST = 0x01,						// ��������
	UART_ECO_CMD_ECO_APPLY = 0x02, 					// ECO����
	UART_ECO_CMD_ECO_READY = 0x03, 					// ECO����
	UART_ECO_CMD_ECO_JUMP_APP = 0x04, 				// APP����汾���
	UART_ECO_CMD_ECO_APP_ERASE = 0x05, 				// APP���ݲ���
	UART_ECO_CMD_ECO_APP_WRITE = 0x06, 				// APP����д��
	UART_ECO_CMD_ECO_APP_WRITE_FINISH = 0x07, 		// APP�������

    UART_ECO_PROTOCOL_CMD_MAX                                   // ��������
} UART_PROTOCOL_CMD;

// UART_RX����֡����
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];    // ����֡������
    uint16  length;                                         // ����֡��Ч���ݸ���
} UART_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];    // ����֡������
    uint16  length;                                         // ����֡��Ч���ݸ���
} UART_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
    // һ�����ջ�����
    struct
    {
        volatile uint8  buff[UART_PROTOCOL_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex;                 // ��ǰ��������ֽڵ�λ���±�
    } rxFIFO;

    // ����֡���������ݽṹ
    struct
    {
        UART_PROTOCOL_RX_CMD_FRAME  cmdQueue[UART_PROTOCOL_RX_QUEUE_SIZE];
        uint16          head;                       // ����ͷ����
        uint16          end;                        // ����β����
    } rx;

    // ����֡���������ݽṹ
    struct
    {
        UART_PROTOCOL_TX_CMD_FRAME  cmdQueue[UART_PROTOCOL_TX_QUEUE_SIZE];
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
    BOOL readMcuUid;

    // ƽ���㷨���ݽṹ
    struct
    {
        uint16 realSpeed;
        uint16 proSpeed;
        uint16 difSpeed;
    } speedFilter;
} UART_PROTOCOL_CB;

typedef union
{
    uint8_t value[4];
    uint32_t raw_data;
} POWERSUPPLYVOLTAGE;

typedef union
{
    uint8_t value[4];
    uint32_t CURRENT;
} VOUT_CURRENT;

extern UART_PROTOCOL_CB uartProtocolCB;
extern uint8 uumacBuff[20];

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void UART_PROTOCOL_Init(void);

// Э�����̴���
void UART_PROTOCOL_Process(void);

// ��������֡�������������
void UART_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void UART_PROTOCOL_TxAddFrame(void);

// �����ϻ��ۼӴ���
void UART_PROTOCOL_SendCmdAging(uint32 param);

void UART_PROTOCOL_SendMcuUid(uint32 param);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

//==================================================================================
// ����ECO����
void UART_PROTOCOL_SendEco(uint32 param);

#define UART_PROTOCOL_RX_PARAM_SIZE         (sizeof(UART_PROTOCOL_RX_PARAM_CB))

#endif
