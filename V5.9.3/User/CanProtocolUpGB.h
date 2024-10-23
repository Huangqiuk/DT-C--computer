/********************************************************************************************************
	���ڶ�����������ϵͳͨ��Э��V3r2_20170821
********************************************************************************************************/
#ifndef __CAN_PROTOCOL_UP_GB_H__
#define __CAN_PROTOCOL_UP_GB_H__

#include "common.h"

/*******************************************************************************************************
 * �����򿪷����޸Ľӿڡ�
 * �û�����ʹ������޸�
 *******************************************************************************************************/
// CAN����ͨѶ����ʱ��
#define CAN_PROTOCOL_COMMUNICATION_TIME_OUT 30000UL // ��λ:ms

// ����ģ�鱾����
#define CAN_PROTOCOL_DEVICE_SELF_ID CAN_PROTOCOL_DEVICE_ID_HMI

// Դ�ڵ�ʹ��(˳���ܴ���,ʹ��ΪTRUE��ʧ��ΪFALSE)
#define CAN_PROTOCOL_RX_DEVICE_HMI FALSE
#define CAN_PROTOCOL_RX_DEVICE_BATTERY TRUE
#define CAN_PROTOCOL_RX_DEVICE_DRIVER TRUE
#define CAN_PROTOCOL_RX_DEVICE_PUC TRUE

/*******************************************************************************************************
 * �� Can �߼�Ӧ�ò�Э�鶨�塿
 * ��������Ա����Э���޸�(�����޸���Ҫ��ѯ����������Ա)
 *******************************************************************************************************/
// �������Դ�ڵ���(�û������޸�)
#define CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX (CAN_PROTOCOL_RX_DEVICE_HMI + CAN_PROTOCOL_RX_DEVICE_DRIVER + CAN_PROTOCOL_RX_DEVICE_BATTERY + CAN_PROTOCOL_RX_DEVICE_PUC)

#define CAN_PROTOCOL_RX_FIFO_SIZE 150  // ���ջ������ߴ�
#define CAN_PROTOCOL_RX_QUEUE_SIZE 40 // ��������֡�ߴ�
#define CAN_PROTOCOL_TX_QUEUE_SIZE 30 // ��������֡�ߴ�

#define CAN_PROTOCOL_RX_CMD_FRAME_LENGTH_MIN 6 // ��������֡��С���ȣ�����6���ֽ�: ����ͷ(3Byte) + ���ݳ���(1Byte) + У����(1Byte) + ֡β(1Byte)
#define CAN_PROTOCOL_TX_CMD_FRAME_LENGTH_MIN 6 // ��������֡��С���ȣ�����6���ֽ�: ����ͷ(3Byte) + ���ݳ���(1Byte) + У����(1Byte) + ֡β(1Byte)
#define CAN_PROTOCOL_CMD_FRAME_LENGTH_MAX 8	   // ����CAN���������

#define CAN_PROTOCOL_CMD_HEAD 0x55	  // ����ͷ
#define CAN_PROTOCOL_CMD_TAIL 0xF0	  // ֡β
#define CAN_PROTOCOL_CMD_NONHEAD 0xFF // ������ͷ
#define CAN_PROTOCOL_CHECK_BYTE 0x01  // У���ֽ���
#define CAN_PROTOCOL_TAIL_BYTE 0x01	  // ֡β�ֽ���

#define CAN_PROTOCOL_ONCE_MESSAGE_MAX_SIZE 0x08 // CAN������·������������8���ֽ�

// CAN����֡Э�鶨��
typedef enum
{
	CAN_PROTOCOL_RX_CMD_HEAD1_INDEX = 0, // RX֡ͷ����1
	CAN_PROTOCOL_RX_CMD_HEAD2_INDEX,	 // RX֡ͷ����2
	CAN_PROTOCOL_RX_CMD_HEAD3_INDEX,	 // RX֡ͷ����3
	CAN_PROTOCOL_RX_CMD_LENGTH_INDEX,	 // RX���ݳ���
	CAN_PROTOCOL_RX_CMD_CMD_H_INDEX,	 // RX�������������ֽ�
	CAN_PROTOCOL_RX_CMD_CMD_L_INDEX,	 // RX�������������ֽ�

	CAN_PROTOCOL_RX_CMD_DATA1_INDEX, // RX��������
	CAN_PROTOCOL_RX_CMD_DATA2_INDEX, // RX��������
	CAN_PROTOCOL_RX_CMD_DATA3_INDEX, // RX��������
	CAN_PROTOCOL_RX_CMD_DATA4_INDEX, // RX��������
	CAN_PROTOCOL_RX_CMD_DATA5_INDEX, // RX��������
	CAN_PROTOCOL_RX_CMD_DATA6_INDEX, // RX��������
	CAN_PROTOCOL_RX_CMD_DATA7_INDEX, // RX��������
	CAN_PROTOCOL_RX_CMD_DATA8_INDEX, // RX��������
	CAN_PROTOCOL_RX_CMD_DATA9_INDEX, // RX��������

	//=======================================================================
	CAN_PROTOCOL_TX_CMD_HEAD1_INDEX = 0, // TX֡ͷ����1
	CAN_PROTOCOL_TX_CMD_HEAD2_INDEX,	 // TX֡ͷ����2
	CAN_PROTOCOL_TX_CMD_HEAD3_INDEX,	 // TX֡ͷ����3
	CAN_PROTOCOL_TX_CMD_LENGTH_INDEX,	 // TX���ݳ���
	CAN_PROTOCOL_TX_CMD_CMD_H_INDEX,	 // TX�������������ֽ�
	CAN_PROTOCOL_TX_CMD_CMD_L_INDEX,	 // TX�������������ֽ�

	CAN_PROTOCOL_TX_CMD_DATA1_INDEX, // TX��������
	CAN_PROTOCOL_TX_CMD_DATA2_INDEX, // TX��������
	CAN_PROTOCOL_TX_CMD_DATA3_INDEX, // TX��������
	CAN_PROTOCOL_TX_CMD_DATA4_INDEX, // TX��������
	CAN_PROTOCOL_TX_CMD_DATA5_INDEX, // TX��������
	CAN_PROTOCOL_TX_CMD_DATA6_INDEX, // TX��������
	CAN_PROTOCOL_TX_CMD_DATA7_INDEX, // TX��������
	CAN_PROTOCOL_TX_CMD_DATA8_INDEX, // TX��������
	CAN_PROTOCOL_TX_CMD_DATA9_INDEX, // TX��������

	CAN_PROTOCOL_CMD_INDEX_MAX
} CAN_PROTOCOL_DATE_FRAME;


// PGN 1939����״̬
typedef enum
{
	STATE_PGN_NULL = 0,

	STATE_PGN_SEND_1, // ������
	STATE_PGN_SEND_2, // ������
	STATE_PGN_SEND_3, // ���ͽ���
	STATE_PGN_SEND_4,

	STATE_PGN_GET_1,
	STATE_PGN_GET_2,
	STATE_PGN_GET_3,
	STATE_PGN_GET_4,
	STATE_PGN_GET_5,
	STATE_PGN_GET_6,
} STATE_PGN;

// ���� �������ϱ������Ĳ��� ����
typedef struct
{
	uint8 ucBatMcuState : 1; // ���MCU״̬
	uint8 reserved : 3;		 // Ԥ��
	uint8 ucBatState : 4;	 // BMS״̬

	uint8 ucBatPercent1; // ��ص�������λ:0.1%
	uint8 ucBatPercent2 : 2;

	uint8 ucBatHealthState1 : 6; // ��ؽ���״̬����λ:0.1%
	uint8 ucBatHealthState2 : 4;

	uint8 ucBatTotalVol1 : 4; // �������ѹ����λ:0.1V
	uint8 ucBatTotalVol2;

	uint8 ucBatOutputlVol1; // ��������ѹ����λ:0.1V
	uint8 ucBatOutputlVol2 : 4;

	uint8 ucBatCurrent1 : 4; // ��ǰ��ص�������λ:0.1A
	uint8 ucBatCurrent2;

} CAN_PROTOCOL_RX_BAT_PARAM_CB;
#define CAN_PROTOCOL_RX_BAT_PARAM_SIZE (sizeof(CAN_PROTOCOL_RX_BAT_PARAM_CB))

// ���� �������ϱ������Ĳ��� ����
typedef struct
{
	uint8 ucHmiMcuState : 1; // �Ǳ�MCU����״̬
	uint8 ucBluMcuState : 1; // �Ǳ�BLE����״̬
	uint8 reserved1 : 2;	 // Ԥ��
	uint8 ucHmiState : 4;	 // �Ǳ�״̬

	uint8 ucHmiError1;
	uint8 ucHmiError2; // �Ǳ������

	uint8 ucHmiSetMode : 2; // �Ǳ�����ģʽ
	uint8 ucHmiLight : 2;	// �Ǳ�ƿ�״̬
	uint8 ucHmiAssist : 4;	// �Ǳ�����ģʽ����

	uint8 ucBluKeyEnable : 2; // ����Կ��ʹ��״̬
	uint8 ucBluKeyUnLock : 2; // ����Կ�׽���״̬
	uint8 ucHmiPowerCol : 2;  // �Ǳ��ϵ����
	uint8 reserved2 : 2;

	uint8 reserved3; // Ԥ��
	uint8 bikeTypeL; // ��������L
	uint8 bikeTypeH; // ��������H
} CAN_PROTOCOL_TX_HMI_PARAM_CB;
#define CAN_PROTOCOL_TX_HMI_PARAM_SIZE (sizeof(CAN_PROTOCOL_TX_HMI_PARAM_CB))

// CAN����֡����
typedef struct
{
	union
	{
		uint32 deviceID;
		struct
		{
			uint32 source : 8;
			uint32 destination : 8;
			uint32 pgn : 10;
			uint32 priority : 3;
			uint32 nHeard : 3;
		} sDet;
	} EXTID;
	uint8 buff[CAN_PROTOCOL_CMD_FRAME_LENGTH_MAX]; // ����֡������
	uint16 length;								   // ����֡��Ч���ݸ���
} CAN_PROTOCOL_CMD_FRAME;

// һ�����ջ�����(����ϲ�Э������)
typedef struct
{
	volatile uint8 buff[CAN_PROTOCOL_RX_FIFO_SIZE];
	volatile uint16 head;
	volatile uint16 end;
	uint16 currentProcessIndex; // ��ǰ��������ֽڵ�λ���±�
} CAN_PROTOCOL_RX_FIFO;

// canһ֡����������
typedef struct
{
	uint32 StdId;
	union
	{
		uint32 ExtId;
		struct
		{
			uint32 nFn : 8;
			uint32 nAfn : 8;
			uint32 nCmd : 3;
			uint32 nDestId : 5;
			uint32 nSrcId : 5;
			uint32 nHeard : 3;
		} sDet;
	} uExtId;
	uint8 IDE;
	uint8 RTR;
	uint8 DLC;
	uint8 Data[8];
	uint8 FMI;
} CAN_RX_FRAME_MAP;

// ����֡���������ݽṹ(�����ɱ�׼CAN��Ϣ���������������)
typedef struct
{
	CAN_RX_FRAME_MAP cmdQueue[CAN_PROTOCOL_RX_QUEUE_SIZE];
	uint16 head; // ����ͷ����
	uint16 end;	 // ����β����
} CAN_PROTOCOL_RX;

// CAN���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����(����ϲ�Э������)
	struct
	{
		CAN_PROTOCOL_RX_FIFO rxFIFOEachNode[CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX];
	} rxFIFO;

	// ����֡���������ݽṹ(�����ɱ�׼CAN��Ϣ���������������)
	struct
	{
		CAN_PROTOCOL_RX rxEachNode[CAN_PROTOCOL_DEVICE_SOURCE_ID_MAX];
	} rx;

	// ����֡���������ݽṹ(�����ɱ�׼CAN��Ϣ)
	struct
	{
		CAN_PROTOCOL_CMD_FRAME cmdQueue[CAN_PROTOCOL_TX_QUEUE_SIZE];
		uint16 head;  // ����ͷ����
		uint16 end;	  // ����β����
		uint16 index; // ��ǰ����������������֡�е�������
		BOOL txBusy;  // ��������
	} tx;

	// �������ݽӿ�
	BOOL(*sendDataThrowService)
	(uint32 id, uint8 *pData, uint8 length);

	BOOL txPeriodRequest; // ��������Է�����������
	BOOL txAtOnceRequest; // ������Ϸ�����������
	BOOL controlFlag;

	// ƽ���㷨���ݽṹ
	struct
	{
		uint16 realSpeed;
		uint16 proSpeed;
	} smooth;

	struct
	{
		uint32 distance;
		uint32 times;
	} tripData;

	STATE_PGN state;		   // ��ǰϵͳ״̬
	STATE_PGN preState;		   // ��һ��״̬
	uint8 sendBuff[200];	   // ��������buff
	uint8 keyPasswordBuff[20]; // ����KEY
	BOOL hmiState;
	// PGN���մ���
	struct
	{

		uint8 numbers;		// ���հ�����
		uint16 byteNumbers; // �ֽ���
		uint8 PGN;			// ��ǰ���յ�PGN

		uint8 dataBuff[200]; // ��������buff
	} PGN_RX;

	struct
	{
		uint16 magicNumber;
		uint8 funCode;
		uint8 data[40];
	} encodeData;

	BOOL UPDATA;
	BOOL UPDATA1;
} CAN_PROTOCOL_CB;

// �������������ݽ����ṹ�嶨��
typedef struct
{
	// �������ϱ������Ĳ���
	union
	{
		uint8 buff[CAN_PROTOCOL_RX_BAT_PARAM_SIZE];
		CAN_PROTOCOL_RX_BAT_PARAM_CB param;
	} rxBat;

	// �Ǳ�������
	union
	{
		uint8 buff[CAN_PROTOCOL_TX_HMI_PARAM_SIZE];
		CAN_PROTOCOL_TX_HMI_PARAM_CB param;
	} tx;

} CAN_PROTOCOL_PARAM_RX_CB;

extern CAN_PROTOCOL_PARAM_RX_CB canRxDriveCB;
extern CAN_PROTOCOL_CB canProtocolCB;

//=======================================================================================
// Э���ʼ��
void CAN_PROTOCOL_Init(void);

// Э�����̴���
void CAN_PROTOCOL_Process_GB(void);

// ��������֡���������֡Դ��֡Ŀ��
void CAN_PROTOCOL_TxAddNewFrame(uint8 priority, uint16 pgn, uint8 destinationID, uint8 sourceID);

// ��������֡�������������
void CAN_PROTOCOL_TxAddData(uint8 data);

// CAN��Ϣ����֡�����ӿ�
void CAN_PROTOCOL_TxAddFrame(void);

//==================================================================================
// �����·������־���������ò���Ҳ�ǵ��ô˽ӿ�
void CAN_PROTOCOL_SetTxAtOnceRequest(uint32 param);

// state : ��ǰ״̬   DestAddress:Ŀ�ĵ�ַ   SourceAddress:Դ��ַ
void CAN_PGN_RequestSendData(STATE_PGN state, uint8 PGN, uint8 DestAddress, uint8 SourceAddress);

// state : ��ǰ״̬   DestAddress:Ŀ�ĵ�ַ   SourceAddress:Դ��ַ
void CAN_PGN_RequestGetData(STATE_PGN state, uint8 PGN, uint8 DestAddress, uint8 SourceAddress);

// �豸������λid  0x1803FF28
void CMD_CAN_HmiToDriverSendResetData(uint32 param);

// ��λ������������ 0x184FEF28
void CMD_CAN_HmiStAssistParam(uint8 assist, uint8 param1, uint8 param2, uint8 param3);

// �־��������� 0x184FEFF9
void CMD_CAN_HmiSetWheelSizeParam(void);

// ���ػ�����ȷ��0x18221828
void CMD_CAN_HmiPowerOffParam(uint32 param);

// ���ò���״̬ 0x18041828
void CMD_CAN_HmiSettingStateParam(uint32 param);

// ����״̬ 0x1825FF28
void CMD_CAN_HmiWorkStateParam(uint32 param);

// �������͹�����Ϣ����
void CAN_SendErrorRequest(uint32 param);

void CAN_UP_DATA(uint32 param);

void CAN_UP_RESET(uint32 param);

extern CAN_PROTOCOL_RX canProtocolRX;
#endif
