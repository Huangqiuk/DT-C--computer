#ifndef __STATE_H__
#define __STATE_H__				

#include "common.h"

#define TIME_WAIT_AFTER_OK						1000ul		// ���óɹ��󡢷����ϼ��˵�ǰ�ĵȴ�ʱ��
#define TIME_WAIT_POWER_ON_OFF					2000ul		// ���ػ��ĵȴ�ʱ��
#define TIME_WAIT_MEUN_AUTO_BACK_RIDING			60000ul		// ��ͨ�˵�����͸߼��˵������Զ��������н���ĵȴ�ʱ��

#define TIME_WAIT_NO_POSITON_RECORD				(600000ul)	// ��ʼ���а���¼����λ�ü�¼�ĳ�ʱʱ��

// ϵͳ״̬����
typedef enum
{
	STATE_NULL = 0,								// ��״̬

	STATE_STANDBY,
	
	STATE_GAOBIAO_CAN_UPGRADE,					// �߱�CAN����
	
	STATE_CM_CAN_UPGRADE,      					// ͨ��can����

	STATE_HUAXIN_CAN_UPGRADE,      				// ��о΢��can����
	
	STATE_SEG_CAN_UPGRADE,						// ����ܡ�������can����

	STATE_KAIYANG_CAN_UPGRADE,					// ����can����

	STATE_SPARROW_CAN_UPGRADE,					// SPARROW CAN����
	
    STATE_MEIDI_CAN_UPGRADE,                    // MEIDI CAN����
    
	STATE_CM_UART_SEG_UPGRADE,  				// ������������ܵ�ͨ�ô�������

	STATE_CM_UART_BC_UPGRADE,  					// ͨ�ò�����ͨ�ô�������

	STATE_HUAXIN_UART_BC_UPGRADE,  				// ��оά�ز����Ĵ�������

	STATE_KAIYANG_UART_BC_UPGRADE,  			// ���������Ĵ�������
	
	STATE_LIME_UART_BC_UPGRADE,                 // LIME�Ĵ�������

    STATE_IOT_UART_UPGRADE,                     // IOT�Ĵ�������

    STATE_IOT_CAN_UPGRADE,                      // IOT�Ĵ�������
	
	STATE_MAX									// ״̬��
}STATE_E;


// ϵͳ״̬����
typedef enum
{
	STEP_NULL = 0,								// ��

	// �߱�can����
	STEP_GAOBIAO_CAN_UPGRADE_ENTRY,			// ��ڲ���
	STEP_GAOBIAO_CAN_UPGRADE_CAN_INIT,		// CAN��ʼ��
	STEP_GAOBIAO_CAN_UPGRADE_POWER_ON,
	STEP_GAOBIAO_CAN_UPGRADE_WAIT_3S,
	STEP_GAOBIAO_CAN_UPGRADE_SEND_KEEP_ELECTRICITY,
	STEP_GAOBIAO_CAN_UPGRADE_SEND_BOOT_CMD,
	STEP_GAOBIAO_CAN_UPGRADE_ARE_YOU_READY,			// ѯ���Ƿ���Խ�������
	STEP_GAOBIAO_CAN_UPGRADE_WAITING_FOR_READY,
	STEP_GAOBIAO_CAN_UPGRADE_SEND_DATA,
	STEP_GAOBIAO_CAN_UPGRADE_WRITE_ERROR,
	STEP_GAOBIAO_CAN_UPGRADE_FINISH,
	STEP_GAOBIAO_CAN_UPGRADE_POWER_OFF,
	STEP_GAOBIAO_CAN_UPGRADE_TIME_OUT,
	STEP_GAOBIAO_CAN_UPGRADE_FAILE,
	
	// ͨ��can��������
	STEP_CM_CAN_UPGRADE_ENTRY,								// ��ڲ���
	STEP_CM_CAN_UPGRADE_WAIT_FOR_ACK,    					// �ȴ�dut��Ӧ
	STEP_CM_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// �յ�dut��eco����
	STEP_CM_CAN_UPGRADE_UP_ALLOW,							// ����ģʽ����

	STEP_CM_CAN_UPGRADE_SEND_UI_EAR,						// ����ui����ָ��
	STEP_CM_CAN_UPGRADE_UI_EAR_RESUALT,						// ���յ�dut���ص�ui�������
	STEP_CM_CAN_UPGRADE_SEND_UI_WRITE,						// ����ui��������
	STEP_CM_CAN_UPGRADE_UI_UP_SUCCESS,						// ui�������
	STEP_CM_CAN_WRITE_UI_VER,                               // дUI�汾��
    STEP_CM_CAN_READ_UI_VER,                                // ��UI�汾��
    STEP_CM_CAN_WRITE_UI_VER_SUCCESS,                       // дUI�汾�ųɹ�
    
	STEP_CM_CAN_UPGRADE_SEND_APP_EAR,						// ����app����ָ��
	STEP_CM_CAN_UPGRADE_APP_EAR_RESUALT,					// ���յ�dut���ص�app�������
	STEP_CM_CAN_UPGRADE_SEND_APP_WRITE,						// ����app��������
	STEP_CM_CAN_UPGRADE_APP_UP_SUCCESS,						// app�������

	STEP_CM_CAN_UPGRADE_ITEM_FINISH,						// �������������
	
	STEP_CM_CAN_UPGRADE_COMMUNICATION_TIME_OUT,				// ���ֳ�ʱ
	STEP_CM_CAN_UPGRADE_RECONNECTION,						// ��������
	STEP_CM_CAN_UPGRADE_RECONNECTION_TIME_OUT,				// �������ӳ�ʱ

	// ��о΢��can��������
	STEP_HUAXIN_CAN_UPGRADE_ENTRY,							// ��ڲ���
	STEP_HUAXIN_CAN_UPGRADE_WAIT_FOR_ACK,					// �ȴ�dut��Ӧ
	STEP_HUAXIN_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_HUAXIN_CAN_UPGRADE_UP_ALLOW,						// ����ģʽ����

	STEP_HUAXIN_CAN_UPGRADE_SEND_APP_EAR,					// ����app����ָ��
	STEP_HUAXIN_CAN_UPGRADE_APP_EAR_RESUALT,				// ���յ�dut���ص�app�������
	STEP_HUAXIN_CAN_UPGRADE_SEND_APP_WRITE,					// ����app��������
	STEP_HUAXIN_CAN_UPGRADE_APP_UP_SUCCESS,					// app�������

	STEP_HUAXIN_CAN_UPGRADE_SEND_UI_EAR,					// ����ui����ָ��
	STEP_HUAXIN_CAN_UPGRADE_UI_EAR_RESUALT,					// ���յ�dut���ص�ui�������
	STEP_HUAXIN_CAN_UPGRADE_SEND_UI_WRITE,					// ����ui��������
	STEP_HUAXIN_CAN_UPGRADE_UI_UP_SUCCESS,					// ui�������

	STEP_HUAXIN_CAN_UPGRADE_ITEM_FINISH,					// �������������

	STEP_HUAXIN_CAN_UPGRADE_COMMUNICATION_TIME_OUT,			// ���ֳ�ʱ


	// can�����������������
	STEP_SEG_CAN_UPGRADE_ENTRY,								// ��ڲ���
	STEP_SEG_CAN_UPGRADE_WAIT_FOR_ACK,    					// �ȴ�dut��Ӧ
	STEP_SEG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// �յ�dut��eco����
	STEP_SEG_CAN_UPGRADE_UP_ALLOW,							// ����ģʽ����

	STEP_SEG_CAN_UPGRADE_SEND_APP_EAR,						// ����app����ָ��
	STEP_SEG_CAN_UPGRADE_APP_EAR_RESUALT,					// ���յ�dut���ص�app�������
	STEP_SEG_CAN_UPGRADE_SEND_APP_WRITE,					// ����app��������
	STEP_SEG_CAN_UPGRADE_APP_UP_SUCCESS,					// app�������

	STEP_SEG_CAN_UPGRADE_ITEM_FINISH,						// �������������

	STEP_SEG_CAN_UPGRADE_COMMUNICATION_TIME_OUT, 			// ���ֳ�ʱ

	// can��������
	STEP_KAIYANG_CAN_UPGRADE_ENTRY,								// ��ڲ���
	STEP_KAIYANG_CAN_UPGRADE_WAIT_FOR_ACK,    					// �ȴ�dut��Ӧ
	STEP_KAIYANG_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,			// �յ�dut��eco����
	STEP_KAIYANG_CAN_UPGRADE_UP_ALLOW,							// ����ģʽ����	
	STEP_KAIYANG_CAN_UPGRADE_SEND_APP_WRITE,					// ����app��������
	STEP_KAIYANG_CAN_UPGRADE_SEND_APP_AGAIN,					// �ֶ�������һ֡����
	STEP_KAIYANG_CAN_UPGRADE_APP_UP_SUCCESS,					// app�������
	STEP_KAIYANG_CAN_UPGRADE_SEND_UI_WRITE,					// ����ui��������
	STEP_KAIYANG_CAN_UPGRADE_SEND_UI_AGAIN,					// ���·���ui���ݰ�
	STEP_KAIYANG_CAN_UPGRADE_UI_UP_SUCCESS,					// ui�������
	STEP_KAIYANG_CAN_UPGRADE_ITEM_FINISH,						// �������������

	// ����IOT����
	STEP_IOT_CAN_UPGRADE_ENTRY,							// ��ڲ���
	STEP_IOT_CAN_UPGRADE_WAIT_FOR_ACK,    			// �ȴ�dut��Ӧ
	STEP_IOT_CAN_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_IOT_CAN_UPGRADE_UP_ALLOW,					// ����ģʽ����
	STEP_IOT_CAN_UPGRADE_SEND_APP_EAR,				    // ����app����ָ��
	STEP_IOT_CAN_UPGRADE_SEND_FIRST_APP_PACKET,			// ���͵�һ������
	STEP_IOT_CAN_UPGRADE_SEND_APP_PACKET,				    // ����app��������
	STEP_IOT_CAN_UPGRADE_APP_UP_SUCCESS,				    // app�������	
	STEP_IOT_CAN_UPGRADE_ITEM_FINISH,				       	// �������������	
	STEP_IOT_CAN_UPGRADE_COMMUNICATION_TIME_OUT,		        // ���ֳ�ʱ
    
 	// can����������Ϣ
 	STEP_CAN_SET_CONFIG_ENTRY,								// ����config���
	STEP_CAN_SEND_SET_CONFIG,								// ����config����		
	STEP_CAN_SET_CONFIG_SUCCESS,							// ����config�ɹ�
    STEP_CAN_READ_CONFIG,	                                // ��ȡ��������
	//STEP_CAN_VERIFY_CONFIG,								// ��֤������Ϣ�Ƿ�д�ɹ�
	STEP_CAN_DCD_FLAG_WRITE,							    // д�����ñ�־λ
	STEP_CAN_DCD_FLAG_READ,						        	// ��ȡ���ñ�־λ
    
	STEP_KAIYANG_CAN_UPGRADE_COMMUNICATION_TIME_OUT,		// ���ֳ�ʱ


	//ͨ�ò�������
	STEP_CM_UART_UPGRADE_ENTRY,							// ��ڲ���
	STEP_CM_UART_BC_UPGRADE_WAIT_FOR_ACK,    			// �ȴ�dut��Ӧ
	STEP_CM_UART_BC_UPGRADE_UP_ALLOW,					// ����ģʽ����

	STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR,				// ����app����ָ��
	STEP_CM_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET,		// ���͵�һ������
	STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET,			// ����app��������
	STEP_CM_UART_BC_UPGRADE_APP_UP_SUCCESS,				// app�������

	STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR,				// ����ui����ָ��
	STEP_CM_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET,		// ���͵�һ������
	STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET,				// ����ui��������
	STEP_CM_UART_BC_UPGRADE_UI_UP_SUCCESS,				// ui�������
	STEP_CM_UART_BC_WRITE_UI_VER,                       // дUI�汾��
	STEP_CM_UART_BC_READ_UI_VER,                        // ��UI�汾��    
    STEP_CM_UART_BC_WRITE_UI_VER_SUCCESS,               // дUI�汾�ųɹ�

	STEP_CM_UART_BC_UPGRADE_ITEM_FINISH,				// �������������

	// ��оά�ز�����������
	STEP_HUAXIN_UART_UPGRADE_ENTRY,							// ��ڲ���
	STEP_HUAXIN_UART_BC_UPGRADE_WAIT_FOR_ACK,    				// �ȴ�dut��Ӧ
	STEP_HUAXIN_UART_BC_UPGRADE_UP_ALLOW,						// ����ģʽ����

	STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_EAR,					// ����app����ָ��
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET,			// ���͵�һ������
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_PACKET,				// ����app��������
	STEP_HUAXIN_UART_BC_UPGRADE_APP_UP_SUCCESS,					// app�������

	STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR,					// ����ui����ָ��
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET,			// ���͵�һ������
	STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_PACKET,					// ����ui��������
	STEP_HUAXIN_UART_BC_UPGRADE_UI_UP_SUCCESS,					// ui�������

	STEP_HUAXIN_UART_BC_UPGRADE_ITEM_FINISH,					// �������������
	
	// �����/��������������
	STEP_SEG_UART_UPGRADE_ENTRY,							// ��ڲ���
	STEP_CM_UART_SEG_UPGRADE_WAIT_FOR_ACK,    				// �ȴ�dut��Ӧ
	STEP_CM_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_CM_UART_SEG_UPGRADE_UP_ALLOW,						// ����ģʽ����

	STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR,					// ����app����ָ��
	STEP_CM_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET,			// ���͵�һ������
	STEP_CM_UART_SEG_UPGRADE_SEND_APP_PACKET,				// ����app��������
	STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS,				// app�������
	
	STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH,					// �������������

	// ������������
	STEP_KAIYANG_UART_UPGRADE_ENTRY,						// ��ڲ���
	STEP_KAIYANG_UART_UPGRADE_WAIT_FOR_ACK,    				// �ȴ�dut��Ӧ
	STEP_KAIYANG_UART_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_KAIYANG_UART_UPGRADE_UP_ALLOW,						// ����ģʽ����

	
	STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE,				// ����app��������
	STEP_KAIYANG_UART_UPGRADE_SEND_APP_AGAIN,				// �ֶ��ط�
	STEP_KAIYANG_UART_UPGRADE_APP_UP_SUCCESS,				// app�������

	STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE,				// ����ui��������
	STEP_KAIYANG_UART_UPGRADE_SEND_UI_AGAIN,				// �ֶ��ط�
	STEP_KAIYANG_UART_UPGRADE_UI_UP_SUCCESS,				// ui�������

	STEP_KAIYANG_UART_UPGRADE_ITEM_FINISH,					// �������������
	
	STEP_KAIYANG_UART_UPGRADE_COMMUNICATION_TIME_OUT,		// ���ֳ�ʱ

	// ����LIME����
	STEP_LIME_UART_UPGRADE_ENTRY,							// ��ڲ���
	STEP_LIME_UART_SEG_UPGRADE_WAIT_FOR_ACK,    			// �ȴ�dut��Ӧ
	STEP_LIME_UART_SEG_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_LIME_UART_SEG_UPGRADE_UP_ALLOW,					// ����ģʽ����
	STEP_LIME_UART_SEG_UPGRADE_SEND_APP_EAR,				    // ����app����ָ��
	STEP_LIME_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET,			// ���͵�һ������
	STEP_LIME_UART_SEG_UPGRADE_SEND_APP_PACKET,				    // ����app��������
	STEP_LIME_UART_SEG_UPGRADE_APP_UP_SUCCESS,				    // app�������	
	STEP_LIME_UART_SEG_UPGRADE_ITEM_FINISH,				       	// �������������	
	STEP_LIME_UART_UPGRADE_COMMUNICATION_TIME_OUT,		        // ���ֳ�ʱ

	// ����IOT����
	STEP_IOT_UART_UPGRADE_ENTRY,							// ��ڲ���
	STEP_IOT_UART_UPGRADE_WAIT_FOR_ACK,    			// �ȴ�dut��Ӧ
	STEP_IOT_UART_UPGRADE_RECEIVE_DUT_ECO_REQUEST,		// �յ�dut��eco����
	STEP_IOT_UART_UPGRADE_UP_ALLOW,					// ����ģʽ����
	STEP_IOT_UART_UPGRADE_SEND_APP_EAR,				    // ����app����ָ��
	STEP_IOT_UART_UPGRADE_SEND_FIRST_APP_PACKET,			// ���͵�һ������
	STEP_IOT_UART_UPGRADE_SEND_APP_PACKET,				    // ����app��������
	STEP_IOT_UART_UPGRADE_APP_UP_SUCCESS,				    // app�������	
	STEP_IOT_UART_UPGRADE_ITEM_FINISH,				       	// �������������	
	STEP_IOT_UART_UPGRADE_COMMUNICATION_TIME_OUT,		        // ���ֳ�ʱ
	
	// ����������Ϣ����
	STEP_UART_SET_CONFIG_ENTRY,								// ����config���
	STEP_UART_PROTOCOL_SWITCCH,                             // Э���л�
	STEP_UART_SEND_SET_CONFIG,								// ����config����
    STEP_UART_READ_CONFIG,	                                // ��ȡ��������
	STEP_UART_SET_CONFIG_SUCCESS,							// ����config�ɹ�
	STEP_UART_DCD_FLAG_WRITE,							// д�����ñ�־λ
	STEP_UART_DCD_FLAG_READ,							// ��ȡ���ñ�־λ

    // ͨ�ò���BOOT����    
	STEP_CM_UART_BC_UPGRADE_BOOT_ENTRY,						// ����BOOT���
	STEP_CM_UART_BC_UPGRADE_SEND_BOOT_EAR,					// ����BOOT����ָ��
	STEP_CM_UART_BC_UPGRADE_SEND_FIRST_BOOT_PACKET,			// ���͵�һ������
	STEP_CM_UART_BC_UPGRADE_SEND_BOOT_PACKET,				// ����BOOT��������
	STEP_CM_UART_BC_UPGRADE_BOOT_UP_SUCCESS,				// BOOT�������
	
 	STEP_CM_CAN_BC_UPGRADE_BOOT_ENTRY,						// ����BOOT���
	STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_EAR,					// ����BOOT����ָ��
	STEP_CM_CAN_BC_UPGRADE_SEND_FIRST_BOOT_PACKET,			// ���͵�һ������
	STEP_CM_CAN_BC_UPGRADE_SEND_BOOT_PACKET,				// ����BOOT��������
	STEP_CM_CAN_BC_UPGRADE_BOOT_UP_SUCCESS,				    // BOOT�������
	     
	STEP_MAX
}STEP_E;

typedef struct
{	
	STATE_E state;					// ��ǰϵͳ״̬
	STATE_E preState;				// ��һ��״̬

	STEP_E step;
    
 
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

// �����л�
void STATE_SwitchStep(uint32 param);

extern uint8 verifiedBuff[150];
extern uint8 uiVerifiedBuff[50];
extern BOOL writeUiFlag;
extern BOOL writeUiVerFlag;  
#endif


