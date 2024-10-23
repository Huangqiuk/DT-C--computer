#ifndef __BLE_HOST_PROTOCOL_H__
#define __BLE_HOST_PROTOCOL_H__

#include "common.h"

//=====================================================================================================
#define BLE_HOST_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��
#define BLE_HOST_PROTOCOL_TXRX_TIME_OUT_CHECK_ENABLE	0		// ͨѶ˫����ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define BLE_HOST_PROTOCOL_RX_QUEUE_SIZE					8		// ����������гߴ�
#define BLE_HOST_PROTOCOL_TX_QUEUE_SIZE					8		// ����������гߴ�

#define BLE_HOST_PROTOCOL_CMD_HEAD						0x55	// ����ͷ

#define BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		150		// ���������֡����
#define BLE_HOST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		150		// ����������֡����
#define BLE_HOST_PROTOCOL_RX_FIFO_SIZE					300		// ����һ����������С
#define BLE_HOST_PROTOCOL_CMD_FRAME_LENGTH_MIN			4		// ����֡��С���ȣ�����:����ͷ�������֡����ݳ��ȡ�У���

// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define BLE_HOST_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/BLE_UART_BAUD_RATE*10*BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define BLE_HOST_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/BLE_UART_BAUD_RATE*10*BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// ��������ͷ���Ͱ����·�������
#define BLE_OUTFIT_HELMET_ID							0x01
#define BLE_OUTFIT_LOCK_ID								0x02

#define BLE_OUTFIT_CONNECT_TIME							1000	// ���Ӽ��
#define BLE_TIME_OUT					            	6000	// ��ʱʱ��

#define BLE_OUTFIT_PAIR_SCAN_TIME						5		// ���ɨ��ʱ��s
#define BLE_OUTFIT_PAIR_COUNT							-1		// ��Գ��Դ���
#define BLE_SEND_CMD_TIMEOUT							1000	// �����������ʱʱ��

// UART����Э�鶨��	
typedef enum
{
	BLE_HOST_PROTOCOL_CMD_HEAD_INDEX = 0,					// ֡ͷ����
	BLE_HOST_PROTOCOL_CMD_CMD_INDEX, 						// ����������
	BLE_HOST_PROTOCOL_CMD_LENGTH_INDEX,						// ���ݳ�������

	BLE_HOST_PROTOCOL_CMD_DATA1_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA2_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA3_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA4_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA5_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA6_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA7_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA8_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA9_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA10_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA11_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA12_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA13_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA14_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA15_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA16_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA17_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA18_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA19_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA20_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA21_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA22_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA23_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA24_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA25_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA26_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA27_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA28_INDEX,						// ��������
	BLE_HOST_PROTOCOL_CMD_DATA29_INDEX, 					// ��������
	BLE_HOST_PROTOCOL_CMD_DATA30_INDEX, 					// ��������

	BLE_HOST_PROTOCOL_CMD_INDEX_MAX
}BLE_HOST_PROTOCOL_DATE_FRAME;

// ��������
typedef enum{
	
	BLE_OUTFIT_PROTOCOL_CMD_HELMET = 0xB1,				// ͷ����������
	BLE_OUTFIT_PROTOCOL_CMD_HELMET_STATE = 0xB2,		// ͷ��״̬
	BLE_OUTFIT_PROTOCOL_CMD_HELMET_PHONE = 0xB3,		// ͷ��������������
	
	BLE_HOST_CMD_MAX,

}BLE_HOST_PROTOCOL_CMD;

// �������״̬
typedef enum
{
	BLE_PAIR_INIT = 0,									// ��ʼ��״̬
	BLE_PAIR_STANDBY,									// ����״̬
	BLE_PAIR_SCAN,										// ɨ��״̬
	BLE_PAIR_CONNECT,									// ����״̬
	BLE_PAIR_SUCCEED,									// ��Գɹ�״̬
	BLE_PAIR_FAILURE,									// ���ʧ��״̬
	BLE_PAIR_STOP,										// ���ֹͣ״̬
	
	BLE_PAIR_MAX,
}BLE_PAIR_STATE;

typedef enum
{
	BLE_MUSIC_PLAY = 1,								// ����
	BLE_MUSIC_PAUSE = 2,							// ��ͣ
	BLE_MUSIC_UP = 3,								// ��һ��
	BLE_MUSIC_NEXT = 4,								// ��һ��	
	BLE_MUSIC_ADD = 8,								// ����+
	BLE_MUSIC_DEC = 9,								// ����-
	
	BLE_MUSIC_MAX_MODE,
}BLE_MUSIC_MODE;

typedef enum
{
	BLE_NORMAL_MODE = 0,
	BLE_PAIR_MODE,

	BLE_MAX_MODE,
}BLE_MODE;

typedef enum
{
	BLE_HELMET_DEVICE = 0,
	BLE_LOCK_DEVICE,
}BLE_PAIR_DEVICE;

typedef enum
{
	BLE_SEND_CMD_IDE = 0,							// �����������
	BLE_SEND_CMD_SEND,								// �������� ����
	BLE_SEND_CMD_WAIT,								// �������� �ȴ����
	BLE_SEND_CMD_TIMOUT,							// �������� �ȴ���ʱ
	BLE_SEND_CMD_DELAY,								// ��ʱ�ȴ�
	BLE_SEND_CMD_STEP_MAX							
}BLE_SEND_CMD_STEP;

typedef enum
{
	BLE_NO_RCV_DATA = 0,
	BLE_RCV_KEYWORD,
	
	BLE_RCV_MAX
}BLE_RCV_CMD_FRAME_MODE_E;

// UART_RX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[BLE_HOST_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];		// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}BLE_HOST_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[BLE_HOST_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];		// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}BLE_HOST_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8	buff[BLE_HOST_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;											// ��ǰ��������ֽڵ�λ���±�
	}rxFIFO;
	
	// ����֡���������ݽṹ
	struct{
		BLE_HOST_PROTOCOL_RX_CMD_FRAME	cmdQueue[BLE_HOST_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;												// ����ͷ����
		uint16			end;												// ����β����
	}rx;

	// ����֡���������ݽṹ
	struct{
		BLE_HOST_PROTOCOL_TX_CMD_FRAME	cmdQueue[BLE_HOST_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;														// ����ͷ����
		uint16	end;														// ����β����
		uint16	index;														// ��ǰ����������������֡�е�������
		BOOL	txBusy;														// ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
	}tx;

	// �������ݽӿ�
	BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	uint32 ageFlag;
	BOOL isTimeCheck;
	
	// TTM�����֡���������ݽṹ
	struct{
		BLE_HOST_PROTOCOL_TX_CMD_FRAME	cmdQueue[BLE_HOST_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;														// ����ͷ����
		uint16	end;														// ����β����
		uint16	index;														// ��ǰ����������������֡�е�������
	}ttmTx;

	struct
	{
		BLE_PAIR_STATE state;												// ��ǰϵͳ״̬
		BLE_PAIR_STATE preState;											// ��һ��״̬
		BLE_PAIR_DEVICE device;												// ����豸
		uint8 macBuff[20];													// ���mac��ַ
		uint8 count;														// ���Դ���
		BOOL scanFlag;														// ɨ���豸��־
		BLE_MODE mode;														// ģʽѡ��
	}pair;
	
	// ���ͺͽ����ж����ݽṹ
	struct{
		BLE_SEND_CMD_STEP sendCmdStep;
		uint32 timeout;
		BOOL Flag;	
	}sendCmd;
	
}BLE_HOST_PROTOCOL_CB;

extern BLE_HOST_PROTOCOL_CB bleHostProtocolCB;		
extern uint8 connectFlag ;
/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void BLE_HOST_PROTOCOL_Init(void);

// Э�����̴���
void BLE_HOST_PROTOCOL_Process(void);

// ��������֡�������������
void BLE_HOST_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void BLE_HOST_PROTOCOL_TxAddFrame(void);

// ���TTM����
void BLE_HOST_PROTOCOL_TxAddTtmCmd(uint8 data[], uint16 length, uint16 id);

// ���������յ���TTM����
void BLE_HOST_UART_TTM_AnalysisCmd(uint8 buff[], uint8 length);

// ״̬������
void BLE_PairProcess(void);

// �л������ģʽ
BOOL BLE_SwitchPairMode(BLE_PAIR_DEVICE device);

// ȡ�����
void BLE_PairStop(void);

// �ϵ�Ͽ�ͷ������
void BLE_HOST_UART_SendCmdDisconnectHelmet(uint32 param);

// ����ͷ��
void BLE_HOST_UART_SendCmdConnectHelmet(uint32 param);

// �ϵ�Ͽ����߰�������
void BLE_HOST_UART_SendCmdDisconnectKey(uint32 param);

// �������߰���
void BLE_HOST_UART_SendCmdConnectKey(uint32 param);

// ��ʱ���ص������豸����ָ��
void  BLE_HOST_UART_CALLBALL_OutfitConnect(uint32 param);

// ��ת��ƿ���
void BLE_HOST_PROTOCOL_SendCmdLeftTurnLight(uint32 param);

// ��ת��ƿ���
void BLE_HOST_PROTOCOL_SendCmdRightTurnLight(uint32 param);

// ��ý����ƹ���
void BLE_HOST_PROTOCOL_SendCmdControlMp3(uint32 param);

// ������ƹ���
void BLE_HOST_PROTOCOL_SendCmdControlPhone(uint32 param);

// ͷ����Ϣ��ѯ
void BLE_HOST_PROTOCOL_SendCmdGetHeadInfoPhone(uint32 param);

// ����������
void BLE_HOST_PROTOCOL_SendCmdGetKey(uint32 param);

// �򿪵�����
void BLE_HOST_PROTOCOL_SendCmdLockOpen(uint32 param);

// �Ͽ�����
void BLE_DISCONNECT(uint32 param);

// ���͸�λ����
void BLE_RESET(uint32 param);
#endif

