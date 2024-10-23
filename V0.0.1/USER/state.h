#ifndef __STATE_H__
#define __STATE_H__				

#define TIME_WAIT_AFTER_OK						1000ul		// ���óɹ��󡢷����ϼ��˵�ǰ�ĵȴ�ʱ��
#define TIME_WAIT_POWER_ON_OFF					2000ul		// ���ػ��ĵȴ�ʱ��

// ϵͳ״̬����
typedef enum
{
	STATE_NULL = 0,								// ��״̬

	STATE_ENTRY,								// ���״̬
	STATE_LOAD_NVM,								// ��������״̬
	STATE_SHOW_POWER_ON,						// ��������״̬

	STATE_STANDBY,								// ����״̬

	STATE_RESERVED,								// ԤԼ״̬

	STATE_ERROR,								// ����ʹ��״̬

	STATE_NORIDING,								// ����״̬

	STATE_NOPARKING,							// ��ͣ״̬
	
	STATE_LOWSPEED,								// ����״̬

	STATE_RIDING,								// ����״̬

	STATE_CHARGING,								// ���״̬

	STATE_UPDATA,								// ����״̬

	STATE_RIDING_NORMAL_INTERFACE,				// ������������״̬

	STATE_LOGO,									// LOGO����״̬

	STATE_AGING_TEST,							// �ϻ�����״̬

	STATE_FOUCTION_SEG_DATA,					// 

	STATE_MAX									// ״̬��
}STATE_E;

typedef struct
{	
	STATE_E state;					// ��ǰϵͳ״̬
	STATE_E preState;				// ��һ��״̬
	STATE_E persLoveState;			// ����ϲ����״̬
	uint8 charingValue;
	uint8 upDataValue;
	
	uint32 lowSpeedAlarmFlag;
}STATE_CB;

extern STATE_CB stateCB;


// ״̬����ʼ��
void STATE_Init(void);

// �����Զ��ػ���ʱ
void STATE_ResetAutoPowerOffControl(void);

// ״̬Ǩ��
void STATE_EnterState(uint32 state);

// ״̬������
void STATE_Process(void);

// ��������
void UART_PROTOCOL_SendCmdElectronicBell(uint32 param);

// һ������
void UART_PROTOCOL_SendCmdKeyLock(uint32 param);

// ���UI�����仯��ʾ����
void STATE_ShowChargeBatLattice(uint32 param);

void SEGMENT_ShowUpdateGui(BOOL refreshAll);


// ѯ�ʰ忨����Ӧ��
void UartCmdAskBoardTypeACK(uint8 dataDir);
// ����Ӧ��
void UartCmdResetACK(uint8 state,uint8 dataDir);

void MultimeterSendMsgToMaistr(uint8 cmd,uint8 *msg,uint16 length);

// ϵͳ��λ
void SYSTEM_Rst(uint32 param);

#endif


