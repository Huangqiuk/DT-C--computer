/********************************************************************************************************
        BigStone_V9.2Э��
********************************************************************************************************/
#ifndef     __UART_PROTOCOL_XM_H__
#define     __UART_PROTOCOL_XM_H__

#include "common.h"


#define SMOOTH_BASE_TIME                            100     // ƽ����׼ʱ�䣬��λ:ms

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define UART_PROTOCOL_XM_COMMUNICATION_TIME_OUT     10000UL // ��λ:ms

#define UART_PROTOCOL_XM_CMD_SEND_TIME              50     // �����ʱ�䣬��λ:ms

//=====================================================================================================
#define UART_PROTOCOL_XM_RX_TIME_OUT_CHECK_ENABLE       1       // ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define UART_PROTOCOL_XM_RX_QUEUE_SIZE              5       // ����������гߴ�
#define UART_PROTOCOL_XM_TX_QUEUE_SIZE              5      // ����������гߴ�

#define UART_PROTOCOL_XM_CMD_HEAD                       0x5A    // ����ͷ
#define UART_PROTOCOL_XM_CMD_DEVICE					0x12	// �豸��
#define UART_PROTOCOL_XM_CMD_BMS_DEVICE				0x13	// ����豸��

#define UART_PROTOCOL_XM_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX  80      // ������֡�����ճ���

#define UART_PROTOCOL_XM_TX_CMD_FRAME_LENGTH_MAX        150     // ���������֡����
#define UART_PROTOCOL_XM_RX_CMD_FRAME_LENGTH_MAX        150     // ����������֡����

#define UART_PROTOCOL_XM_RX_FIFO_SIZE                   200     // ����һ����������С
#define UART_PROTOCOL_XM_CMD_FRAME_LENGTH_MIN           6       // ����֡��С���ȣ�����:��ʼ����������ֵ��������Ϣ�����У��


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define UART_PROTOCOL_XM_BUS_UNIDIRECTIONAL_TIME_OUT    (uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_XM_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)      // ��λ:ms
#define UART_PROTOCOL_XM_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_XM_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)  // ��λ:ms

// UART����Э�鶨��
typedef enum
{

    UART_PROTOCOL_XM_CMD_HEAD_INDEX = 0,                // ֡ͷ����
    UART_PROTOCOL_XM_CMD_DEVICE_INDEX,					// �豸������
    UART_PROTOCOL_XM_CMD_CMD_INDEX,                     // ����������
    UART_PROTOCOL_XM_CMD_LENGTH_INDEX,                  // ���ݳ���

    UART_PROTOCOL_XM_CMD_DATA1_INDEX,                       // ��������
    UART_PROTOCOL_XM_CMD_DATA2_INDEX,                       // ��������
    UART_PROTOCOL_XM_CMD_DATA3_INDEX,                       // ��������
    UART_PROTOCOL_XM_CMD_DATA4_INDEX,                       // ��������
    UART_PROTOCOL_XM_CMD_DATA5_INDEX,                       // ��������
    UART_PROTOCOL_XM_CMD_DATA6_INDEX,                       // ��������
    UART_PROTOCOL_XM_CMD_DATA7_INDEX,                       // ��������
    UART_PROTOCOL_XM_CMD_DATA8_INDEX,                       // ��������
    UART_PROTOCOL_XM_CMD_DATA9_INDEX,                       // ��������
    UART_PROTOCOL_XM_CMD_DATA10_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA11_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA12_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA13_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA14_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA15_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA16_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA17_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA18_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA19_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA20_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA21_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA22_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA23_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA24_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA25_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA26_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA27_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA28_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA29_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA30_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA31_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA32_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA33_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA34_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA35_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA36_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA37_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA38_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA39_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA40_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA41_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA42_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA43_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA44_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA45_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA46_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA47_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA48_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA49_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA50_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA51_INDEX,                  // ��������
    UART_PROTOCOL_XM_CMD_DATA52_INDEX,                  // ��������

    UART_PROTOCOL_XM_CMD_INDEX_MAX


} UART_PROTOCOL_XM_DATE_FRAME;

// ��������
typedef enum
{
    UART_PROTOCOL_XM_CMD_NULL = 0,                              // ������

    // �������߼��
	UART_PROTOCOL_XM_CMD_CONTORY_UP = 0x20,						// �Ǳ��ϴ������ٶ�����

	// ��Ȩ����
	UART_PROTOCOL_XM_CMD_READ_FLAG_DATA = 0x81,				    // ����־������
	UART_PROTOCOL_XM_CMD_WRITE_FLAG_DATA = 0x82,				// д��־������
	
	UART_PROTOCOL_XM_CMD_WRITE_VERSION_TYPE = 0x83,			    // д��汾��Ϣ
	UART_PROTOCOL_XM_CMD_READ_VERSION_TYPE = 0x84,				// ��ȡ�汾��Ϣ

	// BIST ָ��
	UART_PROTOCOL_XM_CMD_TEST_LED = 0x85,						// �������ʾָ��

	UART_PROTOCOL_XM_CMD_TEST_LIGHT = 0x86,					    // ������Ʋ���

	UART_PROTOCOL_XM_CMD_TEST_Throttle_Break = 0x87,			// ��������ɲ��

	UART_PROTOCOL_XM_CMD_TEST_TURN_LIGHT = 0x88,				// ����ת���
	
	UART_PROTOCOL_XM_CMD_TEST_KEY = 0x89,						// ���԰���
	
	UART_PROTOCOL_XM_CMD_TEST_BEEP = 0x9A,						// ����������
	
	UART_PROTOCOL_XM_CMD_TEST_BLE_VERSION = 0x90,				// ��ȡ�����̼��汾

	UART_PROTOCOL_XM_CMD_TEST_CONTROL_VERSION = 0x91,			// ��ȡ�������̼��汾

	UART_PROTOCOL_XM_CMD_CLEAR_TOTAL_DISTANCE = 0x92,			// ��������ָ��

	UART_PROTOCOL_XM_CMD_TEST_TOTAL_DISTANCE = 0x93,			// ��ȡ������·��

	UART_PROTOCOL_XM_CMD_TEST_RESET_FACTORY = 0x94,			    // �ָ�����ָ��
	
	UART_PROTOCOL_XM_CMD_GET_OOB_PARAM = 0x95,					// ��ȡOOB����ָ��
	
	UART_PROTOCOL_XM_CMD_GET_P_DATA_PARAM = 0x96,				// ��ȡ�������ڲ���ָ��
	
	UART_PROTOCOL_XM_CMD_GET_BIKE_SN_PARAM = 0x97,				// ��ȡ�������кŲ���ָ��
	
	UART_PROTOCOL_XM_CMD_GET_BMS_SN_PARAM = 0x98,				// ͬ���������Ϣָ��

    UART_PROTOCOL_XM_CMD_MAX                                    // ��������
} UART_PROTOCOL_XM_CMD;

// UART_RX����֡����
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL_XM_RX_CMD_WRITE_CONTROL_PARAM_LENGTH_MAX];   // ����֡������
    uint16  length;                                         // ����֡��Ч���ݸ���
} UART_PROTOCOL_XM_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
    uint16  deviceID;
    uint8   buff[UART_PROTOCOL_XM_TX_CMD_FRAME_LENGTH_MAX]; // ����֡������
    uint16  length;                                         // ����֡��Ч���ݸ���
} UART_PROTOCOL_XM_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
    // һ�����ջ�����
    struct
    {
        volatile uint8  buff[UART_PROTOCOL_XM_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex;                 // ��ǰ��������ֽڵ�λ���±�

        uint16 curCmdFrameLength;                   // ��ǰ����֡����
    } rxFIFO;

    // ����֡���������ݽṹ
    struct
    {
        UART_PROTOCOL_XM_RX_CMD_FRAME   cmdQueue[UART_PROTOCOL_XM_RX_QUEUE_SIZE];
        uint16          head;                       // ����ͷ����
        uint16          end;                        // ����β����
    } rx;

    // ����֡���������ݽṹ
    struct
    {
        UART_PROTOCOL_XM_TX_CMD_FRAME   cmdQueue[UART_PROTOCOL_XM_TX_QUEUE_SIZE];
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
} UART_PROTOCOL_XM_CB;

extern UART_PROTOCOL_XM_CB uartProtocolCB5;
extern BOOL write_success_Flag;
/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void UART_PROTOCOL_XM_Init(void);

// ��������֡�������������
void UART_PROTOCOL_XM_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void UART_PROTOCOL_XM_TxAddFrame(void);

// UARTЭ�����̴���
void UART_PROTOCOL_XM_Process(void);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void UART_PROTOCOL_XM_RxFIFOProcess(UART_PROTOCOL_XM_CB *pCB);

// UART����֡����������
void UART_PROTOCOL_XM_CmdFrameProcess(UART_PROTOCOL_XM_CB *pCB);

// Э��㷢�ʹ������
void UART_PROTOCOL_XM_TxStateProcess(void);

// ��������ظ�
void UART_PROTOCOL_XM_SendCmdAck(uint8 ackCmd);

// ��������ظ�����һ������
void UART_PROTOCOL_XM_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// ��������ظ���������������
void UART_PROTOCOL_XM_SendCmdTwoParamAck(uint8 ackCmd, uint8 ackParam, uint8 two_ackParam);

// ������DUT������֡ͨ��UARTת����STS
//void DutRxFrame_ToSts_Transmit(UART_PROTOCOL_XM_RX_CMD_FRAME *pCmdFrame);

// �ϱ�����״̬
void UART_PROTOCOL_XM_SendOnlineStart(uint32 param);

// д���־������
void UART_PROTOCOL_XM_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam);

void UART_PROTOCOL_XM_MacProcess(uint16 standarID, uint8 *pData, uint16 length);

void UART_PROTOCOL_XM_Test(uint32 param);
////////////////////////////////////////////////////////////////////////////////

extern uint8 light_cnt;
extern uint8 left_light_cnt ;
extern uint8 right_light_cnt ;
extern uint8 cali_cnt;
extern uint32 online_detection_cnt ;
extern uint8 cnt;
extern uint8 brake_cnt;


#endif


