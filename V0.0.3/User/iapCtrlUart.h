#ifndef 	__IAP_CTRL_UART_H__
#define 	__IAP_CTRL_UART_H__

#include "common.h"

#define IAP_CTRL_UART_TX_MODE					IAP_CTRL_INTERRUPT_TX_MODE		// ѡ���������ͻ����жϷ�������
#define IAP_CTRL_UART_TYPE_DEF					USART1							// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)
#define IAP_CTRL_UART_IRQn_DEF					USART1_IRQn 					// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)

#define IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��
#define IAP_CTRL_UART_TXRX_TIME_OUT_CHECK_ENABLE	0		// ͨѶ˫����ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define IAP_CTRL_UART_BAUD_RATE					115200	// ͨѶ������

#define IAP_CTRL_UART_RX_QUEUE_SIZE				5		// ����������гߴ�
#define IAP_CTRL_UART_TX_QUEUE_SIZE				5		// ����������гߴ�

#define IAP_CTRL_UART_CMD_LENGTH_MAX 			160		// ��������
#define IAP_CTRL_UART_RX_FIFO_SIZE				250		// ����һ����������С

#define IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN		4		// ����֡��С���ȣ�����:����ͷ�������֡����ݳ��ȡ�У����

#define IAP_CTRL_UART_CMD_HEAD					0x55	// ����ͷ

#define UART_PROTOCOL_MCU_UID_BUFF_LENGTH			12		// SN�ų���

// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define IAP_CTRL_UART_BUS_UNIDIRECTIONAL_TIME_OUT		(uint32)((1000.0/IAP_CTRL_UART_BAUD_RATE*10*IAP_CTRL_UART_CMD_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define IAP_CTRL_UART_BUS_BIDIRECTIONAL_TIME_OUT		(uint32)(((1000.0/IAP_CTRL_UART_BAUD_RATE*10*IAP_CTRL_UART_CMD_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// �������������� ���²��ֲ��������޸� ������������������������������
#define IAP_CTRL_BLOCKING_TX_MODE				0U
#define IAP_CTRL_INTERRUPT_TX_MODE				1U

// UART����Э�鶨��	
typedef enum
{
	IAP_CTRL_UART_CMD_HEAD_INDEX = 0,					// ֡ͷ����
	IAP_CTRL_UART_CMD_CMD_INDEX, 						// ����������
	IAP_CTRL_UART_CMD_LENGTH_INDEX,						// ���ݳ���

	IAP_CTRL_UART_CMD_DATA1_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA2_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA3_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA4_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA5_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA6_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA7_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA8_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA9_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA10_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA11_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA12_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA13_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA14_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA15_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA16_INDEX,						// ��������
	IAP_CTRL_UART_CMD_DATA17_INDEX,						// ��������

	IAP_CTRL_UART_CMD_INDEX_MAX
}IAP_CTRL_UART_DATE_FRAME;


// ��������
typedef enum{
	IAP_CTRL_UART_CMD_NULL = 0,									// ��λ����

	IAP_CTRL_UART_CMD_UP_PROJECT_APPLY = 0x01,					// ����ģʽ��������
	IAP_CTRL_UART_CMD_DOWN_PROJECT_APPLY_ACK = 0x02,			// ����ģʽ��׼Ӧ�� 
	IAP_CTRL_UART_CMD_UP_PROJECT_READY = 0x03,					// ����ģʽ׼������

	IAP_CTRL_UART_CMD_DOWN_FACTORY_RESET = 0x04,				// �ָ���������
	IAP_CTRL_UART_CMD_UP_FACTORY_RESET_ACK = 0x05,				// �ָ���������Ӧ��

	IAP_CTRL_UART_CMD_DOWN_RTC_SET = 0x06,						// ����RTC
	IAP_CTRL_UART_CMD_UP_RTC_SET_RESULT = 0x07,					// ����RTC���
	UART_ECO_CMD_ECO_MCU_UID = 0x08,										// �Ƚ�MCU_UID

	IAP_CTRL_UART_CMD_WRITE_SYS_PARAM = 0x10,					// ϵͳ����д��
	IAP_CTRL_UART_CMD_READ_SYS_PARAM = 0x11,					// ϵͳ������ȡ

	IAP_CTRL_UART_CMD_DOWN_UI_DATA_ERASE = 0x12,				// UI���ݲ���
	IAP_CTRL_UART_CMD_UP_UI_DATA_ERASE_ACK = 0x13,				// UI���ݲ������

	IAP_CTRL_UART_CMD_DOWN_UI_DATA_WRITE = 0x14,				// UI����д��
	IAP_CTRL_UART_CMD_UP_UI_DATA_WRITE_RESULT = 0x15,			// UI����д����

	IAP_CTRL_UART_CMD_DOWN_IAP_ERASE_FLASH = 0x24, 				// ����APP_FLASH����
	IAP_CTRL_UART_CMD_UP_IAP_ERASE_FLASH_RESULT = 0x25, 		// ����APP_FLASH����ϱ�����
	IAP_CTRL_UART_CMD_DOWN_IAP_WRITE_FLASH = 0x26, 				// IAP����д��
	IAP_CTRL_UART_CMD_UP_IAP_WRITE_FLASH_RESULT = 0x27, 		// IAP����д�����ϱ�
	IAP_CTRL_UART_CMD_DOWN_CHECK_FALSH_BLANK = 0x28, 			// ���
	IAP_CTRL_UART_CMD_UP_CHECK_FLASH_BLANK_RESULT = 0x29, 		// ��ս���ϱ�
	IAP_CTRL_UART_CMD_DOWN_UPDATA_FINISH = 0x2A, 				// ϵͳ��������
	IAP_CTRL_UART_CMD_UP_UPDATA_FINISH_RESULT = 0x2B, 			// ϵͳ��������ȷ��

	IAP_CTRL_UART_CMD_DOWN_START_QR_REGISTER = 0x40, 			// ������ά��ע��
	IAP_CTRL_UART_CMD_UP_QR_PARAM_REPORT = 0x41, 				// ��ά��ע������ϱ�
	IAP_CTRL_UART_CMD_DOWN_QR_DATA_WRITE = 0x42,				// д���ά������
	IAP_CTRL_UART_CMD_UP_QR_DATA_WRITE_RESULT = 0x43,			// �����ά�����ݽ���ϱ�

	IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ERESE = 0x60,				// SPI ���Ե�ַ����
	IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_WRITE = 0x61,				// SPI ���Ե�ַд��
	IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_READ = 0x62,				// SPI ���Ե�ַ��ȡ
	IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ER_WR = 0x63,				// SPI ���Ե�ַ��д
	
	IAP_CTRL_UART_CMD_SPI_FLASH_UI_ERESE = 0x70, 					// SPI UI ����
	IAP_CTRL_UART_CMD_SPI_FLASH_UI_WRITE = 0x71,					// SPI UI д��
	IAP_CTRL_UART_CMD_SPI_FLASH_UI_FINISH = 0x72,					// SPI UI д�����
	
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_ERESE = 0x73, 			// SPI ����APP ����
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_WRITE = 0x74,			// SPI ����APP д��
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_FINISH = 0x75,			// SPI ����APP д�����
	
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_ERESE = 0x76, 			// SPI ����APP ����
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_WRITE = 0x77,			// SPI ����APP д��
	IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_FINISH = 0x78,			// SPI ����APP д�����
	
	IAP_CTRL_UART_CMD_SPI_FLASH_WRITE_SPI_TO_MCU = 0x79, 			// ����д��

	IAP_CTRL_UART_CMD_JUMP_TO_BOOT = 0x80, 							// ��λ����
	IAP_CTRL_UART_CMD_JUMP_TO_APP1 = 0x81, 							// ��������ǿ�ƿ���BOOT��ת��APP1
	IAP_CTRL_UART_CMD_JUMP_TO_APP2 = 0x82, 							// ��������ǿ�ƿ���BOOT��ת��APP2

	IAP_CTRL_UART_CMD_CHECK_VERSION = 0x90,							// �汾�˶�����
	
	IAP_CTRL_UART_CMD_FLAG_ARRAY_READ = 0x91,						// ����־������
	IAP_CTRL_UART_CMD_FLAG_ARRAY_WRITE = 0x92,						// д��־������

	IAP_CTRL_UART_CMD_VERSION_TYPE_WRITE = 0x93,					// д��汾��Ϣ
	IAP_CTRL_UART_CMD_VERSION_TYPE_READ = 0x94,						// ��ȡ�汾��Ϣ

	IAP_CTRL_UART_CMD_KEY_TESTING = 0x95,							// ������������
	IAP_CTRL_UART_CMD_TEST_LCD = 0x96,								// LCD��ɫ��ʾ����
	

	IAP_CTRL_UART_CMD_MAX											// ��������
}IAP_CTRL_UART_CMD;

// �汾�˶�ö��
typedef enum{
	IAP_CTRL_UART_CHECK_VERSION_ALL,

	IAP_CTRL_UART_CHECK_VERSION_APP,
	IAP_CTRL_UART_CHECK_VERSION_BOOT,
	IAP_CTRL_UART_CHECK_VERSION_UI,
	IAP_CTRL_UART_CHECK_VERSION_HW,
	IAP_CTRL_UART_CHECK_VERSION_SN_NUM,
	IAP_CTRL_UART_CHECK_VERSION_BLE_MAC,
	IAP_CTRL_UART_CHECK_VERSION_QR_CODE,
}IAP_CTRL_UART_CHECK_VERSION_E;

// UART����֡����
typedef struct
{
	uint8	buff[IAP_CTRL_UART_CMD_LENGTH_MAX];	// ����֡������
	uint16	length; 						// ����֡��Ч���ݸ���
}IAP_CTRL_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8	buff[IAP_CTRL_UART_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;					// ��ǰ��������ֽڵ�λ���±�
	}rxFIFO;
	
	// ����֡���������ݽṹ
	struct{
		IAP_CTRL_CMD_FRAME	cmdQueue[IAP_CTRL_UART_RX_QUEUE_SIZE];
		uint16			head;						// ����ͷ����
		uint16			end;						// ����β����
	}rx;

	// ����֡���������ݽṹ
	struct{
		IAP_CTRL_CMD_FRAME	cmdQueue[IAP_CTRL_UART_TX_QUEUE_SIZE];
		volatile uint16	head;						// ����ͷ����
		volatile uint16	end;						// ����β����
		volatile uint16	index;						// ��ǰ����������������֡�е�������
		volatile BOOL	txBusy;						// ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
	}tx;
	
	uint32 ageFlag;
}IAP_CTRL_UART_CB;

extern IAP_CTRL_UART_CB iapCtrlUartCB;		

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/
// UART��ʼ������main�Ĵ�ѭ��֮ǰ���ã����ģ���ʼ��
void IAP_CTRL_UART_Init(void);

// UARTģ�鴦����ڣ���main�Ĵ�ѭ���е���
void IAP_CTRL_UART_Process(void);

// ���ͻ��������������
void IAP_CTRL_UART_TxAddData(IAP_CTRL_UART_CB* pCB, uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void IAP_CTRL_UART_TxAddFrame(IAP_CTRL_UART_CB* pCB);

// ������������
void IAP_CTRL_UART_BC_StartTx(IAP_CTRL_UART_CB* pCB);

// ͨѶ��ʱ����-˫��
void IAP_CTRL_UART_CALLBACK_TxRxTimeOut(uint32 param);

//==============================================================================
// ����ģʽ����
void IAP_CTRL_UART_SendCmdProjectApply(uint32 para);

// ��ά��ע������ϱ�
void IAP_CTRL_UART_SendCmdQrRegParam(uint8 *param);

// ���������޽��
void IAP_CTRL_UART_SendVersionCheck(uint32 checkNum);

// ������ʾ����
uint8 IAP_CTRL_UART_LcdControl(uint8 lcdCmd);

// ������������
void IAP_CTRL_UART_SendCmdWithResult(uint8 cmdWord, uint8 result);

#endif


