#ifndef 	__CAN_PROTOCOL_TEST_H__
#define 	__CAN_PROTOCOL_TEST_H__

#include "common.h"
#include "dutInfo.h"
/** @  *****************************************************************************************************
  * �����򿪷����޸Ľӿڡ� 
  * �û�����ʹ������޸�
************************************************************************************************************/
#define CAN_BAUD_RATE						CAN_BAUD_RATE_500K

// ����ģ�鱾����
#define CAN_TEST_DEVICE_SELF_ID					CAN_TEST_DEVICE_ID_HMI	
 
// Դ�ڵ�ʹ��(˳���ܴ���,ʹ��ΪTRUE��ʧ��ΪFALSE)
#define CAN_TEST_RX_DEVICE_PUC					TRUE
#define CAN_TEST_RX_DEVICE_HMI					FALSE

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
#define CAN_TEST_DEVICE_SOURCE_ID_MAX			(CAN_TEST_RX_DEVICE_PUC + CAN_TEST_RX_DEVICE_HMI)

#define CAN_TEST_RX_FIFO_SIZE					80	    // ���ջ������ߴ�
#define CAN_TEST_RX_QUEUE_SIZE					80	    // ��������֡�ߴ�
#define CAN_TEST_TX_QUEUE_SIZE					80		// ��������֡�ߴ�	  

#define CAN_PROTOCOL_TEST_FRAME_LENGTH_MIN			4		// ����֡��С���ȣ�����4���ֽ�: ����ͷ(1Byte)+������(1Byte)+���ݳ���(1Byte)+У����(1Byte)
#define CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX			140		// ����CAN���������Ϊ140���ֽ�

#define CAN_PROTOCOL_TEST_HEAD						0x55	// ����ͷ
#define CAN_PROTOCOL_TEST_NONHEAD 					0xFF	// ������ͷ
#define CAN_CHECK_BYTE_SIZE 				0x01	// У������ռ�ֽ���
#define CAN_ONCEMESSAGE_MAX_SIZE 			0x08	// CAN������·������������8���ֽ�

// CAN���߽ڵ���
typedef enum{
	CAN_TEST_DEVICE_ID_PUC = 0x000,					// PUC�ڵ�	0x000
	CAN_TEST_DEVICE_ID_HMI = 0x001,					// HMI�ڵ�	0x001

	CAN_TEST_DEVICE_ID_MAX							// ���ڵ���
}CAN_TEST_DEVICE_ID;

// CAN����Э�鶨��	
typedef enum
{
	CAN_PROTOCOL_TEST_HEAD_INDEX = 0, 					// ֡ͷ����
	CAN_PROTOCOL_TEST_CMD_INDEX,						// ����������
	CAN_PROTOCOL_TEST_LENGTH_INDEX,						// ���ݳ���

	CAN_PROTOCOL_TEST_DATA1_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA2_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA3_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA4_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA5_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA6_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA7_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA8_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA9_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA10_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA11_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA12_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA13_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA14_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA15_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA16_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA17_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA18_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA19_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA20_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA21_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA22_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA23_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA24_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA25_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA26_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA27_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA28_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA29_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA30_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA31_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA32_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA33_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA34_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA35_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA36_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA37_INDEX,						// ��������
	CAN_PROTOCOL_TEST_DATA38_INDEX,						// ��������	

	CAN_PROTOCOL_TEST_INDEX_MAX,
}CAN_TEST_DATE_FRAME;


// CAN��������
typedef enum{
	// ϵͳ����
	CAN_PROTOCOL_TEST_CMD_NULL = 0,                              // ������

    CAN_PROTOCOL_TEST_CMD_HEADLIGHT_CONTROL = 0xEA,              // ��ƿ���
    CAN_PROTOCOL_TEST_CMD_GET_THROTTLE_BRAKE_AD = 0xEB,          // ��ȡ����/ɲ����ģ������ֵ
    CAN_PROTOCOL_TEST_CMD_TURN_SIGNAL_CONTROL = 0xEC,            // ת��ƿ���
	CAN_PROTOCOL_TEST_CMD_ONLINE_DETECTION = 0xED,               // ���߼��
    CAN_PROTOCOL_TEST_CMD_GET_PHOTORESISTOR_VALUE = 0x90,        // ��ȡ��������������ֵ
    CAN_PROTOCOL_TEST_CMD_VOLTAGE_CALIBRATION = 0x91,            // ���е�ѹУ׼
    CAN_PROTOCOL_TEST_CMD_KEY_TEST = 0x05,                       // ��������

    CAN_PROTOCOL_TEST_CMD_READ_FLAG_DATA = 0xA6,               // ��ȡ��־������
    CAN_PROTOCOL_TEST_CMD_WRITE_FLAG_DATA = 0xA7,              // д���־������
    CAN_PROTOCOL_TEST_CMD_WRITE_VERSION_TYPE_DATA = 0xA8,      // д��汾��������
    CAN_PROTOCOL_TEST_CMD_READ_VERSION_TYPE_INFO = 0xA9,       // ��ȡ�汾������Ϣ
		
	CAN_PROTOCOL_TEST_CMD_BLUETOOTH_MAC_ADDRESS_READ = 0x0C,   // ��������  
//	CAN_PROTOCOL_TEST_CMD_BLUETOOTH_TEST = 0x0A,               // ��������     
    CAN_PROTOCOL_TEST_CMD_TEST_LCD = 0x03,                     // LCD��ɫ���ԣ�������=0x03��
    CAN_PROTOCOL_TEST_CMD_FLASH_CHECK_TEST = 0x09,             // Flash У�����

    CAN_PROTOCOL_TEST_CMD_MAX                                  // ��������


}CAN_PROTOCOL_TEST;

// CAN����֡����
typedef struct
{
	uint32	deviceID;
	uint8	buff[CAN_PROTOCOL_TEST_FRAME_LENGTH_MAX]; 	 // ����֡������ 
	uint16	length; 							 // ����֡��Ч���ݸ���
}CAN_PROTOCOL_TEST_FRAME;

// һ�����ջ�����(����ϲ�Э������)
typedef struct
{
	volatile uint8	buff[CAN_TEST_RX_FIFO_SIZE];
	volatile uint16	head;
	volatile uint16	end;
	uint16 currentProcessIndex;					// ��ǰ��������ֽڵ�λ���±�
}CAN_TEST_RX_FIFO;

// ����֡���������ݽṹ(�����ɱ�׼CAN��Ϣ���������������)
typedef struct{
	CAN_PROTOCOL_TEST_FRAME	cmdQueue[CAN_TEST_RX_QUEUE_SIZE];
	uint16		head;							// ����ͷ����
	uint16		end;							// ����β����
}CAN_TEST_RX;


// CAN���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����(����ϲ�Э������)
	struct{
		CAN_TEST_RX_FIFO rxFIFOEachNode[CAN_TEST_DEVICE_SOURCE_ID_MAX];
	}rxFIFO;
	
	
	// ����֡���������ݽṹ(�����ɱ�׼CAN��Ϣ���������������)
	struct{
		CAN_TEST_RX rxEachNode[CAN_TEST_DEVICE_SOURCE_ID_MAX];
	}rx;


	// ����֡���������ݽṹ(�����ɱ�׼CAN��Ϣ)
	struct{
		CAN_PROTOCOL_TEST_FRAME	cmdQueue[CAN_TEST_TX_QUEUE_SIZE];
		volatile uint16 head;						// ����ͷ����
		volatile uint16 end; 						// ����β����
		volatile uint16 index;						// ��ǰ����������������֡�е�������
		volatile BOOL txBusy;						// ��������
	}tx;

}CAN_TEST_CB;

extern CAN_TEST_CB testCanCB;

/**********************************************************************************************************/


/** @  *****************************************************************************************************
  * ��CAN����״̬���� 
  *  Can �߼�Ӧ�ò�Э�鶨�壬��������Ա����Э���޸�
  * @{	
  **********************************************************************************************************/

// CAN����ϵͳ״̬����
typedef enum
{
	CAN_TEST_TX_STATE_NULL = 0,									// ��״̬
	
	CAN_TEST_TX_STATE_ENTRY,										// ���״̬ 
	CAN_TEST_TX_STATE_STANDBY,									// ����״̬
	CAN_TEST_TX_STATE_FILL_SEND, 								// Tx����뷢��
	CAN_TEST_TX_STATE_SENDING,									// Tx������
	CAN_TEST_TX_STATE_ERR_PROCESS,								// Tx������
	CAN_TEST_TX_STATE_ERR_RESCOVRY,								// Tx�����޸�
	CAN_TEST_TX_STATE_ERR_ALARM, 								// Tx���󱨾�
	
	CAN_TEST_TX_STATE_MAX,										// Tx״̬��  
	
}CAN_TEST_TX_STATE_E;

typedef struct
{
	CAN_TEST_TX_STATE_E state;									// ��ǰϵͳ״̬
	CAN_TEST_TX_STATE_E preState;								// ��һ��״̬	
	uint8 canTxErrNum;										// CAN��Ϣ���ʹ��������
	uint8 retry;											// CAN��Ϣ�ط�
	uint8 retrySnd; 										// �޸������CAN��Ϣ�ڶ����ط�

	uint8 canIntFlag;										// CAN��Ϣ�жϱ�־λ
	uint8 canErrFlag;										// CAN��Ϣ�����־λ

}CAN_TEST_TX_STATE_CB;

extern CAN_TEST_TX_STATE_CB canTestTxStateCB;

//=====�ⲿ���ú�������=======================================================================
// ģ���ʼ���ӿ�
void CAN_PROTOCOL_TEST_Init(uint16 bund);

void CAN_TEST_HW_Init(uint16 bund);

// CAN������̺������� main �д�ѭ���ڵ���
void CAN_PROTOCOL_Process_Test(void);
	
// ���ͻ��������������
void CAN_PROTOCOL_TEST_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void CAN_PROTOCOL_TEST_TxAddFrame(void);

// д���־������
void CAN_PROTOCOL_TEST_WriteFlag(uint8 placeParam, uint8 shutdownFlagParam);

// ���������Ӧ��
void CAN_PROTOCOL_TEST_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);
//=============================================================================================
// ����ģʽ����
void CAN_PROTOCOL_TEST_SendCmdProjectApply(uint8 *param);

// ���ͻ����������һ������������
BOOL CAN_TEST_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length);

void CAN_PROTOCOL_TEST_SendCmdAck(uint8 param);

// ������������
void CAN_PROTOCOL_TEST_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_PROTOCOL_TEST cmd, uint32 falshAddr, uint32 addr);

// CAN ���Ľ��մ�����(ע����ݾ���ģ���޸�)
extern void CAN_TEST_PROTOCOL_MacProcess(uint32 standarID, uint8 *pData, uint8 length);

// CAN ���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_TEST_MacProcess(uint16 standarID, uint8 *data, uint8 length);

// �����������
void CAN_PROTOCOL_TEST_SendCmdTest(uint32 param);

//// ���ͳɹ�������һ֡
//BOOL CAN_TEST_Tx_NextFram(CAN_TEST_CB *pCB);

//// ����뷢�ʹ���:TRUE,����뷢�ͣ�FALSE�����ζ�����ǰ����.
//BOOL CAN_TEST_Tx_FillAndSend(CAN_TEST_CB *pCB);
#endif


