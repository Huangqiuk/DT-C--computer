/********************************************************************************************************
		KM5S�������ͨѶЭ��
********************************************************************************************************/
#ifndef 	__LIME_UART_PROTOCOL_H__
#define 	__LIME_UART_PROTOCOL_H__

#include "common.h"

#define SMOOTH_BASE_TIME							100		// ƽ����׼ʱ�䣬��λ:ms

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define LIME_UART_PROTOCOL_COMMUNICATION_TIME_OUT		10000UL	// ��λ:ms

// P��λ
#define PROTOCOL_ASSIST_P							ASSIST_P

#define LIME_UART_PROTOCOL_CMD_SEND_TIME					500		// ���������

//=====================================================================================================
#define LIME_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define LIME_UART_PROTOCOL_RX_QUEUE_SIZE					4		// ����������гߴ�
#define LIME_UART_PROTOCOL_TX_QUEUE_SIZE					4		// ����������гߴ�

#define LIME_UART_PROTOCOL_CMD_HEAD						'L'		// ����ͷ
#define LIME_UART_PROTOCOL_CMD_HEAD1						'L'		// ����ͷ1
#define LIME_UART_PROTOCOL_CMD_HEAD2						'B'		// ����ͷ2
#define LIME_UART_PROTOCOL_CMD_HEAD3						'D'		// ����ͷ3
#define LIME_UART_PROTOCOL_CMD_HEAD4						'C'		// ����ͷ4
#define LIME_UART_PROTOCOL_CMD_HEAD5						'P'		// ����ͷ5
#define LIME_UART_PROTOCOL_CMD_PROTOCOL_VERSION			0x01	// Э��汾
#define LIME_UART_PROTOCOL_CMD_DEVICE_ADDR				0x10	// �豸��
#define LIME_UART_PROTOCOL_CMD_NONHEAD 					0xFF	// ������ͷ
#define LIME_UART_PROTOCOL_HEAD_BYTE 					5	// ����ͷ�ֽ���
#define LIME_UART_PROTOCOL_CHECK_BYTE 					1	// У���ֽ���

#define LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		160		// ���������֡����
#define LIME_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		160		// ����������֡����

#define LIME_UART_PROTOCOL_RX_FIFO_SIZE					200		// ����һ����������С
#define LIME_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN			11		// ����֡��С���ȣ�����:5������ͷ��Э��汾���豸�š������֡�2�����ݳ��ȡ�У����


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define LIME_UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define LIME_UART_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// UART����Э�鶨��	
typedef enum
{
	LIME_UART_PROTOCOL_CMD_HEAD1_INDEX = 0,					// ֡ͷ1����
	LIME_UART_PROTOCOL_CMD_HEAD2_INDEX,						// ֡ͷ2����
	LIME_UART_PROTOCOL_CMD_HEAD3_INDEX,						// ֡ͷ3����
	LIME_UART_PROTOCOL_CMD_HEAD4_INDEX,						// ֡ͷ4����
	LIME_UART_PROTOCOL_CMD_HEAD5_INDEX,						// ֡ͷ5����
	LIME_UART_PROTOCOL_CMD_VERSION_INDEX,							// Э��汾����
	LIME_UART_PROTOCOL_CMD_DEVICE_ADDR_INDEX,				// �豸��ַ
	LIME_UART_PROTOCOL_CMD_CMD_INDEX, 						// ����������
	LIME_UART_PROTOCOL_CMD_LENGTH_H_INDEX,					// ���ݳ���H
	LIME_UART_PROTOCOL_CMD_LENGTH_L_INDEX,					// ���ݳ���L

	LIME_UART_PROTOCOL_CMD_DATA1_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA2_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA3_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA4_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA5_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA6_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA7_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA8_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA9_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA10_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA11_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA12_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA13_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA14_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA15_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA16_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA17_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA18_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA19_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA20_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA21_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA22_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA23_INDEX,						// ��������
	LIME_UART_PROTOCOL_CMD_DATA24_INDEX,						// ��������

	LIME_UART_PROTOCOL_CMD_INDEX_MAX
}LIME_UART_PROTOCOL_DATE_FRAME;

// ��������
typedef enum{
	LIME_UART_PROTOCOL_CMD_NULL = 0,								// ������

	// ECOģʽ����
	LIME_UART_PROTOCOL_ECO_CMD_REQUEST = 0x01,					// ECO����
	LIME_UART_PROTOCOL_ECO_CMD_READY = 0x02,						// ECOģʽ׼������
	LIME_UART_PROTOCOL_ECO_CMD_FACTORY_RST = 0x03,				// �ָ���������
	LIME_UART_PROTOCOL_ECO_CMD_APP_ERASE = 0x04,					// APP���ݲ���
	LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE = 0x05, 				// APP����д��
	LIME_UART_PROTOCOL_ECO_CMD_APP_WRITE_FINISH = 0x06, 			// APP����д�����

	// ����ģʽ����
	LIME_UART_PROTOCOL_RUN_CMD_RST = 0x10,						// �Ǳ�λ����
	LIME_UART_PROTOCOL_RUN_CMD_SEG_DISPLAY = 0x11,				// �������ʾ
	LIME_UART_PROTOCOL_RUN_CMD_BRIGHTNESS = 0x12,				// ���ȵ���
	LIME_UART_PROTOCOL_RUN_CMD_APP_VERSION = 0x16,				// APP����汾
	
	LIME_UART_PROTOCOL_RUN_CMD_LIGHT_BELT = 0x1B,				// �ƴ�����
	LIME_UART_PROTOCOL_RUN_CMD_READ_SN = 0x1C,					// ��ȡSN
	LIME_UART_PROTOCOL_RUN_CMD_BOOT_VERSION = 0x1D,				// ��ȡBOOT�汾
	LIME_UART_PROTOCOL_RUN_CMD_HW_VERSION = 0x1E,				// ��ȡӲ���汾
	LIME_UART_PROTOCOL_RUN_CMD_READ_LIME_SN = 0x1F,				// ��ȡLIMESN
	LIME_UART_PROTOCOL_RUN_CMD_MATCHING = 0x31,					// ���
	LIME_UART_PROTOCOL_RUN_CMD_WRITE_CARD_ID = 0x32,				// д��ID

	// ��Ȩ����
	LIME_UART_PROTOCOL_PRIVILEGE_READ_FLASH_DATA = 0xA4,			// FLASH���ݶ�ȡ
	LIME_UART_PROTOCOL_PRIVILEGE_WRITE_FLASH_DATA = 0xA5,		// FLASH����д��	
	LIME_UART_PROTOCOL_PRIVILEGE_READ_FLAG_DATA = 0xA6,			// FLAG ���ݶ�ȡ
	LIME_UART_PROTOCOL_PRIVILEGE_WRITE_FLAG_DATA = 0xA7,			// FLAG ����д��
	LIME_UART_PROTOCOL_PRIVILEGE_WRITE_VERSION_DATA = 0xA8,		// �汾����д��

	// �������ʾ
	LIME_UART_PROTOCOL_SEG_DISPLAY_SEG_TEST = 0xB0,				// ����������
	LIME_UART_PROTOCOL_SEG_DISPLAY_SEG_CONTROL = 0xB1,			// ����ܿ���	
	
	LIME_UART_PROTOCOL_CMD_MAX									// ��������
}LIME_UART_PROTOCOL_CMD;


// UART_RX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[LIME_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}LIME_UART_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[LIME_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}LIME_UART_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8	buff[LIME_UART_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;					// ��ǰ��������ֽڵ�λ���±�
	}rxFIFO;
	
	// ����֡���������ݽṹ
	struct{
		LIME_UART_PROTOCOL_RX_CMD_FRAME	cmdQueue[LIME_UART_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;						// ����ͷ����
		uint16			end;						// ����β����
	}rx;

	// ����֡���������ݽṹ
	struct{
		LIME_UART_PROTOCOL_TX_CMD_FRAME	cmdQueue[LIME_UART_PROTOCOL_TX_QUEUE_SIZE];
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
}LIME_UART_PROTOCOL_CB;

extern LIME_UART_PROTOCOL_CB uartProtocolCB4;		

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void LIME_UART_PROTOCOL_Init(void);

// Э�����̴���
void LIME_UART_PROTOCOL_Process(void);

// ��������֡�������������
void LIME_UART_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void LIME_UART_PROTOCOL_TxAddFrame(void);

//==================================================================================
// �����·������־���������ò���Ҳ�ǵ��ô˽ӿ�
void LIME_UART_PROTOCOL_SetTxAtOnceRequest(uint32 param);

void LIME_UART_PROTOCOL_SendCmdLedAck(void);
void UART_LED_SetControlMode(const uint8* buff);

// ����������
void LIME_UART_PROTOCOL_SendCmd(uint32 sndCmd);

// ����ECO����
void LIME_UART_PROTOCOL_SendCmdEco(uint32 param);

// ��ȡFLASH�ظ�
void LIME_UART_PROTOCOL_SendCmdReadFlagCmd(uint32* pBuf);

// ���Ϳ�ID
void LIME_UART_PROTOCOL_SendCardId(uint8 cardStyle, uint8 cardIdLen, uint8 *cardId);

// ���������ּ��ظ�һ���ֽ�
void LIME_UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

void LIME_Protocol_SendOnePacket(uint32 flashAddr, uint32 addr);
#endif


