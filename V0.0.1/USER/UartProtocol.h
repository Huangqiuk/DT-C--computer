/********************************************************************************************************
		KM5S�������ͨѶЭ��
********************************************************************************************************/
#ifndef 	__UART_PROTOCOL_H__
#define 	__UART_PROTOCOL_H__

#include "common.h"

#define SMOOTH_BASE_TIME							100		// ƽ����׼ʱ�䣬��λ:ms

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define UART_PROTOCOL_COMMUNICATION_TIME_OUT		10000UL	// ��λ:ms

// P��λ
#define PROTOCOL_ASSIST_P							ASSIST_P

#define UART_PROTOCOL_CMD_SEND_TIME					500		// ���������

//=====================================================================================================
#define UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define UART_PROTOCOL_RX_QUEUE_SIZE					5		// ����������гߴ�
#define UART_PROTOCOL_TX_QUEUE_SIZE					5		// ����������гߴ�

#define UART_PROTOCOL_CMD_HEAD1						0x55		// ����ͷ
#define UART_PROTOCOL_CMD_HEAD2						0x00		// ����ͷ1
#define UART_PROTOCOL_CMD_HEAD3						0xAA		// ����ͷ2

#define UART_PROTOCOL_CMD_PROTOCOL_VERSION			0x01	// Э��汾
#define UART_PROTOCOL_CMD_DEVICE_ADDR				0x10	// �豸��
#define UART_PROTOCOL_CMD_NONHEAD 					0xFF	// ������ͷ
#define UART_PROTOCOL_HEAD_BYTE 					3	// ����ͷ�ֽ���
#define UART_PROTOCOL_CHECK_BYTE 					1	// У���ֽ���

#define UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		160		// ���������֡����
#define UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		160		// ����������֡����

#define UART_PROTOCOL_RX_FIFO_SIZE					200		// ����һ����������С
#define UART_PROTOCOL_CMD_FRAME_LENGTH_MIN			6		// ����֡��С���ȣ�����:5������ͷ��Э��汾���豸�š������֡�2�����ݳ��ȡ�У����


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define UART_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// UART����Э�鶨��	
typedef enum
{
	UART_PROTOCOL_CMD_HEAD1_INDEX = 0,					// ֡ͷ1����
	UART_PROTOCOL_CMD_HEAD2_INDEX,						// ֡ͷ2����
	UART_PROTOCOL_CMD_HEAD3_INDEX,						// ֡ͷ3����	
	UART_PROTOCOL_CMD_INDEX,                            /* ���������� */
	UART_PROTOCOL_CMD_LENGTH_INDEX,                     /* ���ݳ��� */
	UART_PROTOCOL_CMD_BOARD_INDEX,	                    /* Ŀ��忨�� */

	UART_PROTOCOL_CMD_DATA1_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA2_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA3_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA4_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA5_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA6_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA7_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA8_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA9_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA10_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA11_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA12_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA13_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA14_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA15_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA16_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA17_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA18_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA19_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA20_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA21_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA22_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA23_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA24_INDEX,						// ��������

	UART_PROTOCOL_CMD_INDEX_MAX
}UART_PROTOCOL_DATE_FRAME;


 /*������*/
typedef enum
{		
	BOARD_ARM =0X00,   /*ARM����*/
	BOARD_MOM =0X01,   /*ĸ�����*/
	BOARD_COM,         /*�ӿڰ����*/
	BOARD_PRO,         /*��¼������*/
	BOARD_AVO,         /*���ñ����*/
	BOARD_MAX     
}UART_PROTOCOL_BDNUM;

// ��������
typedef enum{	
	UART_CMD_NULL        = 0xFF,
	UART_CMD_CALL        = 0x01,						 // ��ѯ�忨��Ϣ��������һ��
	UART_CMD_POWERROR    = 0x02,		                 // ��⵽�쳣����
	UART_CMD_RES         = 0x03, 						 // ��λ��������һ��
	UART_CMD_GO_APP      = 0x04, 						 // ����APP��������һ��	
	UART_CMD_UPGRADE     = 0x05, 						 // �����̼���������һ��
	UART_CMD_WRITE_DATA  = 0x06, 					     // д�����ݣ�������һ��
    //ĸ��ָ�ʼ
    UART_CMD_JS_CTRLKEY  = 0x20,                         //JS�ű����������ư�������
    UART_CMD_POWER_CTRL  = 0x21,                         //��Դ����ڿ�������
    UART_CMD_POWER_ADJ   = 0X22,                         //�ɵ���ѹ��Դ��������
    UART_CMD_ADJVOL_READ = 0X23,                         //ĸ��ɵ���Դ��ǰ��ѹ��ȡ����
	UART_CMD_ADJCUR_READ = 0X24,                         //ĸ��ɵ���Դ��ǰ������ȡ����
	UART_CMD_MOM_DOWM_SWITCH = 0X25,                     //��¼���ؿ���
	//��¼��ָ��
	UART_CMD_DOWN_SEND_DOWNINFO = 0x80,                  // ��¼����������ָ��
    UART_CMD_DOWN_SEND_DOWNINFO_ACK = 0x80,              // ��¼����������ָ��Ӧ��	
	UART_CMD_DOWN_SEND_DOWNFILE = 0x81,                  // ��¼�������ļ�ָ��
    UART_CMD_DOWN_SEND_DOWNFILE_ACK = 0x81,              // ��¼�������ļ�ָ��Ӧ��	
	UART_CMD_DOWN_SEND_DOWNFILE_FINISH = 0x82,           // ��¼�������ļ����
    UART_CMD_DOWN_SEND_DOWNFILE_FINISH_ACK = 0x82,       // ��¼�������ļ�Ӧ��	
	UART_CMD_DOWN_START_DOWNLOARD = 0x83,                // ��¼����¼Ŀ���ָ��
    UART_CMD_DOWN_START_DOWNLOARD_ACK = 0x83,            // ��¼����¼Ŀ���Ӧ��
	
	//���ñ�忨����
	UART_CMD_AVOMETER = 0xb0,
	UART_CMD_AVOMETER_ACK = 0xb0,
	// ���ñ�λ����
	UART_CMD_AVORESET = 0xb1,
	UART_CMD_AVORESET_ACK = 0xb1,
	
	//�㲥����
	//�㲥��λ����
    UART_CMD_RESET_SOURCE  =0xc0,                               //����������Դ���ָ�����ʼ״̬
	UART_CMD_ALL_GO_APP    =0xc1,                               //�㲥������а��ӽ���APP	
	// ��ͷ��λͨ������
	//UART_CMD_MEMET_CAPTURE_GET_GEARS = 0x50, 			// ��ѯ��ͷ��λ����
	//UART_CMD_MEMET_CAPTURE_GET_GEARS_ACK = 0x50,			// ��ѯ��ͷ��λ����Ӧ��
	
	UART_PROTOCOL_CMD_MAX									// ��������
}UART_PROTOCOL_CMD;


 /*������*/
typedef enum
{		
	MEASURE_DCV   =1,  //ֱ����ѹ��V��
    MEASURE_ACV,       //������ѹ��V��
	MEASURE_OHM,       //���裨����
	MEASURE_DCMA,      //ֱ��������mA��
	MEASURE_ACMA,      //����������mA��
	MEASURE_DCA,       //ֱ��������A��
	MEASURE_ACA,       //����������A��		
}AVO_MEASURE_TYPE;



//�������ṹ��
typedef union
{
   uint8_t data[4];
   float fl;
}FLOAT_PARA;


//�������ṹ��
typedef union
{
   uint8_t data[4];
   int INT;
}INT_PARA;





// UART_RX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}UART_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}UART_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8	buff[UART_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;					// ��ǰ��������ֽڵ�λ���±�
	}rxFIFO;
	
	// ����֡���������ݽṹ
	struct{
		UART_PROTOCOL_RX_CMD_FRAME	cmdQueue[UART_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;						// ����ͷ����
		uint16			end;						// ����β����
	}rx;

	// ����֡���������ݽṹ
	struct{
		UART_PROTOCOL_TX_CMD_FRAME	cmdQueue[UART_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;						// ����ͷ����
		uint16	end;						// ����β����
		uint16	index;						// ��ǰ����������������֡�е�������
		BOOL	txBusy;						// ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
	}tx;

	// �������ݽӿ�
	BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	BOOL txPeriodRequest;		// ��������Է�����������
	BOOL txAtOnceRequest;		// ������Ϸ�����������

	BOOL paramSetOK;

	// ƽ���㷨���ݽṹ
	struct{
		uint16 realSpeed;
		uint16 proSpeed;
		uint16 difSpeed;
	}speedFilter;
}UART_PROTOCOL_CB;

extern UART_PROTOCOL_CB uartProtocolCB;		

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

//==================================================================================
// �����·������־���������ò���Ҳ�ǵ��ô˽ӿ�
void UART_PROTOCOL_SetTxAtOnceRequest(uint32 param);


void UART_PROTOCOL_SendCmdLedAck(void);
void UART_LED_SetControlMode(const uint8* buff);

// ����������
void UART_PROTOCOL_SendCmd(uint32 sndCmd);

// ����ECO����
void UART_PROTOCOL_SendCmdEco(uint32 param);

// ��ȡFLASH�ظ�
void UART_PROTOCOL_SendCmdReadFlagCmd(uint32* pBuf);

// ���Ϳ�ID
void UART_PROTOCOL_SendCardId(uint8 cardStyle, uint8 cardIdLen, uint8 *cardId);

// �豸�ϱ�����
void APP_ReportMsg(uint8_t cmd, uint8_t *msg, uint8_t length);
static bool UART_CheckSUM_(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

void Uart2_CmdAskVerACK(void);
#endif


