/********************************************************************************************************
		KM5S�������ͨѶЭ��
********************************************************************************************************/
#ifndef 	__UART_PROTOCOL_H__
#define 	__UART_PROTOCOL_H__

#include "common.h"

//=====================================================================================================
#define UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define UART_PROTOCOL_RX_QUEUE_SIZE					5		// ����������гߴ�
#define UART_PROTOCOL_TX_QUEUE_SIZE					5		// ����������гߴ�

#define UART_PROTOCOL_CMD_HEAD						0x55	// ����ͷ
#define UART_PROTOCOL_CMD_HEAD_3A					0x3A	// ����ͷ

#define UART_PROTOCOL_CMD_DEVICE_ADDR				0x1A	// �豸��ַ

#define UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		150		// ���������֡����
#define UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		150		// ����������֡����
#define UART_PROTOCOL_RX_FIFO_SIZE					200		// ����һ����������С
#define UART_PROTOCOL_CMD_FRAME_LENGTH_MIN			4		// ����֡��С���ȣ�����:����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define UART_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// UART����Э�鶨��	
typedef enum
{
	UART_PROTOCOL_CMD_HEAD_INDEX = 0,					// ֡ͷ����
	UART_PROTOCOL_CMD_CMD_INDEX, 						// ����������
	UART_PROTOCOL_CMD_LENGTH_INDEX,						// ���ݳ���

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
	UART_PROTOCOL_CMD_DATA25_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA26_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA27_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA28_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA29_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA30_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA31_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA32_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA33_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA34_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA35_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA36_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA37_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA38_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA39_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA40_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA41_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA42_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA43_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA44_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA45_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA46_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA47_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA48_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA49_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA50_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA51_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA52_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA53_INDEX,						// ��������
	UART_PROTOCOL_CMD_DATA54_INDEX,						// ��������

	UART_PROTOCOL_CMD_INDEX_MAX
}UART_PROTOCOL_DATE_FRAME;

// ��������
typedef enum{
	UART_PROTOCOL_CMD_NULL = 0,								// ������

	UART_CMD_UP_PROJECT_APPLY = 0x01,						// DUT����ģʽ����,bc280ul

	UART_CMD_UP_PROJECT_ALLOW = 0x02,						// DUT����ģʽ��׼,bc352

	UART_CMD_UP_PROJECT_READY = 0x03,						// DUT����ģʽ׼������

	UART_ECO_CMD_ECO_JUMP_APP = 0x04,						// ������ɺ�����app

	UART_CMD_UP_APP_EAR = 0x05,								// 352app������ʼ

	UART_CMD_UP_APP_UP = 0x06,								// 352app����

	UART_CMD_UP_APP_UP_OVER = 0x07,							// 352app��������

	UART_CMD_WRITE_SYS_PARAM = 0x10,						// DUT_configд�루����Ҫ��״̬���з����ͣ�

	UART_CMD_READ_SYS_PARAM = 0x11,							// DUT_config��ȡ

	UART_CMD_DUT_UI_DATA_ERASE = 0x12,						// DUT_UI��д

	UART_CMD_DUT_UI_DATA_ERASE_ACK = 0x13,					// DUT_UI��дӦ��

	UART_CMD_DUT_UI_DATA_WRITE = 0x14,						// DUT_UIд��

	UART_CMD_DUT_UI_DATA_WRITE_RES = 0x15,					// DUT_UIд����

	UART_CMD_DUT_APP_ERASE_FLASH = 0x24,					// DUT_APP��д

	UART_CMD_DUT_APP_ERASE_RESULT = 0x25,					// DUT_APP��д�����ѯ

	UART_CMD_DUT_APP_WRITE_FLASH = 0x26,					// DUT_APPд��

	UART_CMD_DUT_APP_WRITE_FLASH_RES = 0x27,				// DUT_APPд����

	UART_CMD_DUT_UPDATA_FINISH = 0x2A,                      // DUT_APPд�����

	UART_CMD_UP_UPDATA_FINISH_RESULT = 0x2B,		     	// DUT_APPд����ȷ��
	
	
	// ��������
	UART_PROTOCOL_CMD_HMI_CTRL_RUN = 0x52,					// ��������״̬

	UART_PROTOCOL_CMD_HMI_CTRL_PARAM_SET = 0x53,			// ��������

	UART_PROTOCOL_CMD_RST = 0x80,							// ��λ����

	UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM = 0xC0,			// д����������
	UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM_RESULT = 0xC1,	// д�������������Ӧ��
	UART_PROTOCOL_CMD_READ_CONTROL_PARAM = 0xC2,			// ��ȡ����������
	UART_PROTOCOL_CMD_READ_CONTROL_PARAM_REPORT = 0xC3,		// �����������ϱ�

	UART_PROTOCOL_CMD_FLAG_ARRAY_READ = 0xA6,				// ����־������
	UART_PROTOCOL_CMD_FLAG_ARRAY_WRITE = 0xA7,				// д��־������

	UART_PROTOCOL_CMD_VERSION_TYPE_WRITE = 0xA8,			// д��汾��Ϣ
	UART_PROTOCOL_CMD_VERSION_TYPE_READ = 0xA9,				// ��ȡ�汾��Ϣ

	UART_PROTOCOL_CMD_TEST_LCD = 0xAA,						// LCD��ɫ��ʾ����
	UART_PROTOCOL_CMD_PROTOCOL_SWITCH = 0xAB,				// Э���л�����

	UART_PROTOCOL_CMD_AD_RATIO_REF = 0xAC,					// ldo���ѹ��ѹ��У׼ֵ
	UART_PROTOCOL_CMD_KEY_CHECK_TEST = 0xAD,				// ����������
	UART_PROTOCOL_CMD_CLOCK_CHECK_TEST = 0xAE,				// ʱ�Ӽ�����
	UART_PROTOCOL_CMD_SENSOR_CHECK_TEST = 0xAF,				// ����������
	UART_PROTOCOL_CMD_BLUE_MACADDR_TEST = 0xB0,				// ����MAC��ַ��ȡ
	UART_PROTOCOL_CMD_CLEAR_MILEAGE = 0xB5,					// ����������
	

	UART_PROTOCOL_CMD_MAX									// ��������
}UART_PROTOCOL_CMD;

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

void  UART_PROTOCOL_TxAddFrame_3A(void);

// ��������ظ�����һ������
void UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// �������ݰ�
void uartProtocol_SendOnePacket(uint32 flashAddr,uint32 addr);
// �������ݰ���HEX��
void uartProtocol_SendOnePacket_Hex(uint32 flashAddr);
#endif

