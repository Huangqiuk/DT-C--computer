#ifndef 	__IAP_CTRL_CAN_H__
#define 	__IAP_CTRL_CAN_H__

#include "common.h"
#include "dutInfo.h"
/** @  *****************************************************************************************************
  * �����򿪷����޸Ľӿڡ� 
  * �û�����ʹ������޸�
************************************************************************************************************/
#define CAN_BAUD_RATE						CAN_BAUD_RATE_500K

// ����ģ�鱾����
#define CAN_DEVICE_SELF_ID					CAN_DEVICE_ID_HMI	
 
// Դ�ڵ�ʹ��(˳���ܴ���,ʹ��ΪTRUE��ʧ��ΪFALSE)
#define CAN_RX_DEVICE_PUC					TRUE
#define CAN_RX_DEVICE_HMI					FALSE

// CAN��Ԫ���Կ���
#define CAN_UNIT_TEST						0

//============================================================
#define CAN_BAUD_RATE_2K					2
#define CAN_BAUD_RATE_3K					3
#define CAN_BAUD_RATE_5K					5
#define CAN_BAUD_RATE_10K					10
#define CAN_BAUD_RATE_20K					20
#define CAN_BAUD_RATE_30K					30
#define CAN_BAUD_RATE_40K					40
#define CAN_BAUD_RATE_50K					50
#define CAN_BAUD_RATE_60K					60
#define CAN_BAUD_RATE_80K					80
#define CAN_BAUD_RATE_90K					90
#define CAN_BAUD_RATE_100K					100
#define CAN_BAUD_RATE_125K					125
#define CAN_BAUD_RATE_150K					150
#define CAN_BAUD_RATE_200K					200
#define CAN_BAUD_RATE_250K					250
#define CAN_BAUD_RATE_300K					300
#define CAN_BAUD_RATE_400K					400
#define CAN_BAUD_RATE_500K					500
#define CAN_BAUD_RATE_600K					600
#define CAN_BAUD_RATE_666K					666
#define CAN_BAUD_RATE_800K					800
#define CAN_BAUD_RATE_900K					900
#define CAN_BAUD_RATE_1000K					1000

/**********************************************************************************************************/


/** @  *****************************************************************************************************
  * �� Can �߼�Ӧ�ò�Э�鶨�塿 
  * ��������Ա����Э���޸�(�����޸���Ҫ��ѯ����������Ա) 
  **********************************************************************************************************/  
	// �������Դ�ڵ���(�û������޸�)
#define CAN_DEVICE_SOURCE_ID_MAX			(CAN_RX_DEVICE_PUC+CAN_RX_DEVICE_HMI)

#define CAN_RX_FIFO_SIZE					150 		// ���ջ������ߴ�
#define CAN_RX_QUEUE_SIZE					11		// ��������֡�ߴ�
#define CAN_TX_QUEUE_SIZE					10		// ��������֡�ߴ�	  

#define CAN_CMD_FRAME_LENGTH_MIN			4		// ����֡��С���ȣ�����4���ֽ�: ����ͷ(1Byte)+������(1Byte)+���ݳ���(1Byte)+У����(1Byte)
#define CAN_CMD_FRAME_LENGTH_MAX			150		// ����CAN���������Ϊ140���ֽ�

#define CAN_CMD_HEAD						0x55	// ����ͷ
#define CAN_CMD_NONHEAD 					0xFF	// ������ͷ
#define CAN_CHECK_BYTE_SIZE 				0x01	// У������ռ�ֽ���
#define CAN_ONCEMESSAGE_MAX_SIZE 			0x08	// CAN������·������������8���ֽ�

// CAN���߽ڵ���
typedef enum{
	CAN_DEVICE_ID_PUC = 0x000,					// PUC�ڵ�	0x000
	CAN_DEVICE_ID_HMI = 0x001,					// HMI�ڵ�	0x001

	CAN_DEVICE_ID_MAX							// ���ڵ���
}CAN_DEVICE_ID;

// CAN����Э�鶨��	
typedef enum
{
	CAN_CMD_HEAD_INDEX = 0, 					// ֡ͷ����
	CAN_CMD_CMD_INDEX,							// ����������
	CAN_CMD_LENGTH_INDEX,						// ���ݳ���

	CAN_CMD_DATA1_INDEX,						// ��������
	CAN_CMD_DATA2_INDEX,						// ��������
	CAN_CMD_DATA3_INDEX,						// ��������
	CAN_CMD_DATA4_INDEX,						// ��������
	CAN_CMD_DATA5_INDEX,						// ��������
	CAN_CMD_DATA6_INDEX,						// ��������
	CAN_CMD_DATA7_INDEX,						// ��������
	CAN_CMD_DATA8_INDEX,						// ��������
	CAN_CMD_DATA9_INDEX,						// ��������
	CAN_CMD_DATA10_INDEX,						// ��������
	CAN_CMD_DATA11_INDEX,						// ��������
	CAN_CMD_DATA12_INDEX,						// ��������
	CAN_CMD_DATA13_INDEX,						// ��������
	CAN_CMD_DATA14_INDEX,						// ��������
	CAN_CMD_DATA15_INDEX,						// ��������
	CAN_CMD_DATA16_INDEX,						// ��������
	CAN_CMD_DATA17_INDEX,						// ��������
	CAN_CMD_DATA18_INDEX,						// ��������
	CAN_CMD_DATA19_INDEX,						// ��������
	CAN_CMD_DATA20_INDEX,						// ��������
	CAN_CMD_DATA21_INDEX,						// ��������
	CAN_CMD_DATA22_INDEX,						// ��������
	CAN_CMD_DATA23_INDEX,						// ��������
	CAN_CMD_DATA24_INDEX,						// ��������
	CAN_CMD_DATA25_INDEX,						// ��������
	CAN_CMD_DATA26_INDEX,						// ��������
	CAN_CMD_DATA27_INDEX,						// ��������
	CAN_CMD_DATA28_INDEX,						// ��������
	CAN_CMD_DATA29_INDEX,						// ��������
	CAN_CMD_DATA30_INDEX,						// ��������
	CAN_CMD_DATA31_INDEX,						// ��������
	CAN_CMD_DATA32_INDEX,						// ��������
	CAN_CMD_DATA33_INDEX,						// ��������
	CAN_CMD_DATA34_INDEX,						// ��������
	CAN_CMD_DATA35_INDEX,						// ��������
	CAN_CMD_DATA36_INDEX,						// ��������
	CAN_CMD_DATA37_INDEX,						// ��������
	CAN_CMD_DATA38_INDEX,						// ��������	

	CAN_CMD_INDEX_MAX,
}CAN_DATE_FRAME;


// CAN��������
typedef enum{
	// ϵͳ����
	IAP_CTRL_CAN_CMD_EMPTY = 0,									// ������

	IAP_CTRL_CAN_CMD_UP_PROJECT_APPLY = 0x01,					// ����ģʽ��������
	IAP_CTRL_CAN_CMD_DOWN_PROJECT_APPLY_ACK = 0x02,				// ����ģʽ��׼Ӧ�� 
	IAP_CTRL_CAN_CMD_UP_PROJECT_READY = 0x03,					// ����ģʽ׼������

	IAP_CTRL_CAN_CMD_SEG_RUN_APP_ACK = 0x04,					// ���������������app���Ӧ��
	IAP_CTRL_CAN_HUAXIN_CMD_APP_EAR = 0x05,						// ��о΢��can��������app����

	IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP = 0x06,					// ��о΢��can����дapp����
	IAP_CTRL_CAN_HUAXIN_CMD_WRITE_APP_COMPLETE = 0x07,			// ��о΢��can����дapp�������

	IAP_CTRL_CAN_CMD_DOWN_UI_DATA_ERASE = 0x12,					// UI���ݲ���
	IAP_CTRL_CAN_CMD_UP_UI_DATA_ERASE_ACK = 0x13,				// UI���ݲ������

	IAP_CTRL_CAN_CMD_DOWN_UI_DATA_WRITE = 0x14,					// UI����д��
	IAP_CTRL_CAN_CMD_UP_UI_DATA_WRITE_RESULT = 0x15,			// UI����д����

	IAP_CTRL_CAN_CMD_DOWN_IAP_ERASE_FLASH = 0x24, 				// ����APP_FLASH����
	IAP_CTRL_CAN_CMD_UP_IAP_ERASE_FLASH_RESULT = 0x25, 			// ����APP_FLASH����ϱ�����
	IAP_CTRL_CAN_CMD_DOWN_IAP_WRITE_FLASH = 0x26, 				// IAP����д��
	IAP_CTRL_CAN_CMD_UP_IAP_WRITE_FLASH_RESULT = 0x27, 			// IAP����д�����ϱ�
	IAP_CTRL_CAN_CMD_DOWN_CHECK_FALSH_BLANK = 0x28, 			// ���
	IAP_CTRL_CAN_CMD_UP_CHECK_FLASH_BLANK_RESULT = 0x29, 		// ��ս���ϱ�
	IAP_CTRL_CAN_CMD_DOWN_UPDATA_FINISH = 0x2A, 				// ��������
	IAP_CTRL_CAN_CMD_UP_UPDATA_FINISH_RESULT = 0x2B, 			// ��������ȷ��	
	
}CAN_CMD;

// CAN����֡����
typedef struct
{
	uint32	deviceID;
	uint8	buff[CAN_CMD_FRAME_LENGTH_MAX]; 	 // ����֡������ 
	uint16	length; 							 // ����֡��Ч���ݸ���
}CAN_CMD_FRAME;

// һ�����ջ�����(����ϲ�Э������)
typedef struct
{
	volatile uint8	buff[CAN_RX_FIFO_SIZE];
	volatile uint16	head;
	volatile uint16	end;
	uint16 currentProcessIndex;					// ��ǰ��������ֽڵ�λ���±�
}CAN_RX_FIFO;

// ����֡���������ݽṹ(�����ɱ�׼CAN��Ϣ���������������)
typedef struct{
	CAN_CMD_FRAME	cmdQueue[CAN_RX_QUEUE_SIZE];
	uint16		head;							// ����ͷ����
	uint16		end;							// ����β����
}CAN_RX;


// CAN���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����(����ϲ�Э������)
	struct{
		CAN_RX_FIFO rxFIFOEachNode[CAN_DEVICE_SOURCE_ID_MAX];
	}rxFIFO;
	
	
	// ����֡���������ݽṹ(�����ɱ�׼CAN��Ϣ���������������)
	struct{
		CAN_RX rxEachNode[CAN_DEVICE_SOURCE_ID_MAX];
	}rx;


	// ����֡���������ݽṹ(�����ɱ�׼CAN��Ϣ)
	struct{
		CAN_CMD_FRAME	cmdQueue[CAN_TX_QUEUE_SIZE];
		volatile uint16 head;						// ����ͷ����
		volatile uint16 end; 						// ����β����
		volatile uint16 index;						// ��ǰ����������������֡�е�������
		volatile BOOL txBusy;						// ��������
	}tx;

}CAN_CB;

extern CAN_CB sysCanCB;

/**********************************************************************************************************/


/** @  *****************************************************************************************************
  * ��CAN����״̬���� 
  *  Can �߼�Ӧ�ò�Э�鶨�壬��������Ա����Э���޸�
  * @{	
  **********************************************************************************************************/

// CAN����ϵͳ״̬����
typedef enum
{
	CAN_TX_STATE_NULL = 0,									// ��״̬
	
	CAN_TX_STATE_ENTRY,										// ���״̬ 
	CAN_TX_STATE_STANDBY,									// ����״̬
	CAN_TX_STATE_FILL_SEND, 								// Tx����뷢��
	CAN_TX_STATE_SENDING,									// Tx������
	CAN_TX_STATE_ERR_PROCESS,								// Tx������
	CAN_TX_STATE_ERR_RESCOVRY,								// Tx�����޸�
	CAN_TX_STATE_ERR_ALARM, 								// Tx���󱨾�
	
	CAN_TX_STATE_MAX,										// Tx״̬��  
	
}CAN_TX_STATE_E;

typedef struct
{
	CAN_TX_STATE_E state;									// ��ǰϵͳ״̬
	CAN_TX_STATE_E preState;								// ��һ��״̬	
	uint8 canTxErrNum;										// CAN��Ϣ���ʹ��������
	uint8 retry;											// CAN��Ϣ�ط�
	uint8 retrySnd; 										// �޸������CAN��Ϣ�ڶ����ط�

	uint8 canIntFlag;										// CAN��Ϣ�жϱ�־λ
	uint8 canErrFlag;										// CAN��Ϣ�����־λ

}CAN_TX_STATE_CB;

extern CAN_TX_STATE_CB canTxStateCB;

//=====�ⲿ���ú�������=======================================================================
// ģ���ʼ���ӿ�
void IAP_CTRL_CAN_Init(uint16 bund);

void CAN_HW_Init(uint16 bund);

// CAN������̺������� main �д�ѭ���ڵ���
void CAN_PROTOCOL_Process_DT(void);
	
// ���ͻ��������������
void IAP_CTRL_CAN_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void IAP_CTRL_CAN_TxAddFrame(void);

//=============================================================================================
// ����ģʽ����
void IAP_CTRL_CAN_SendCmdProjectApply(uint8 *param);

// ���ͻ����������һ������������
BOOL CAN_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length);

void IAP_CTRL_CAN_SendCmdNoAck(uint8 param);

// ������������
void IAP_CTRL_CAN_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_CMD cmd, uint32 falshAddr, uint32 addr);

// CAN ���Ľ��մ�����(ע����ݾ���ģ���޸�)
extern void CAN_PROTOCOL_MacProcess(uint32 standarID, uint8 *pData, uint8 length);

#endif


