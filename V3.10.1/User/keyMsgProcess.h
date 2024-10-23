#ifndef 	__KEY_MSG_PROCESS_MODULE_H__
#define 	__KEY_MSG_PROCESS_MODULE_H__

/******************************************************************************
* ���û����Ʋ�������
* ����ϵͳ��Ҫ�޸ģ�����ϸ���ע���������ò���
******************************************************************************/
// ������Ҫ��ģ��ͷ�ļ�

#define KEY_SCAN_TIME			50		// ����ɨ�����ڣ���λ��ms������Ϊ���ⶨʱ���ķֱ��ʵ�N��(N>=1)
#define KEY_HOLD_TIME			1500	// �����������ޣ���λ��ms������Ϊɨ�����ڵ�N��(N>=1)
#define KEY_RETHROW_TIME		500		// �����ظ��׳����ڣ���λ��ms������Ϊɨ�����ڵ�N��(N>=2)

// ����������
typedef enum
{
	// �������ƶ��壬�ɸ���ϵͳ��Ҫ���ж��ƣ���һ��Ҫ��֤�� KEY_NAME_MAX ֮ǰ���
	// ����ÿ��������ռ��λ���ӵ�λ���λ���У��׸�����ö��ֵ����Ϊ0
	KEY_NAME_ADD= 0,
	KEY_NAME_INFO,
	KEY_NAME_DEC,
	KEY_NAME_ADD_DEC,
	
	/*=============================================================
	** ���������ֹ�޸ģ����ұ���λ��������������
	==============================================================*/
	KEY_NAME_MAX,
	KEY_NAME_NULL,
}KEY_NAME_E;

// ���ڼ�¼����״̬���ֽ���
#define KEY_STATE_BYTE_COUNT							((KEY_NAME_MAX+7)/8)

// �����ж����ڵȼ۵�ɨ��������
#define KEY_JUDGE_COUNT								(KEY_JUDGE_TIME/KEY_SCAN_TIME)

// �������붨��
typedef enum
{
	KEY_MASK_DEC			= (1UL<<KEY_NAME_DEC),
	KEY_MASK_INFO			= (1UL<<KEY_NAME_INFO),
	KEY_MASK_ADD			= (1UL<<KEY_NAME_ADD),
	KEY_MASK_ADD_DEC		= (1UL<<KEY_NAME_ADD_DEC),

	KEY_MASK_ALL = 0x000F,
	KEY_MASK_NULL = 0,
}KEY_MASK_E;


// ������ע�⡿����
#define	KEY_MSG_USE_SAME_HOLD_THRESHOLD			0	// �Ƿ�ʹ����ͬ�ĳ����������ޣ�����Ϊ0ʱ����Ҫ������Ҫ�޸�c�ļ��е����޳�����ֵ
#define	KEY_MSG_USE_SAME_REPEAT_TIME			0	// �Ƿ�ʹ����ͬ���ظ����ڣ�����Ϊ0ʱ����Ҫ������Ҫ�޸�c�ļ��ж�Ӧ�ĳ�����ֵ

/******************************************************************************
* ��ϵͳ��������
* ע�⣺�벻Ҫ�޸��������ֵ�����ǶԱ�ģ���ʵ��ԭ��ʮ���˽�
******************************************************************************/
#define	KEY_INFO_SIZE					2		// ������Ϣ�ڵ���гߴ磬�����ɱ��ּ�������״̬
#define	KEY_STATE_MASK					0x8000	// ����״̬����
#define	KEY_MSG_QUEUE_SIZE				10		// ������Ϣ�������ߴ磬�����ɱ��ֶ��ٸ���Ϣ��ע��ʵ���Ͽ���Ԫ��Ϊ��ֵ-1

#define	KEY_MSG_SWITCH_DOWN				1		// ��Ϣ��Ӧ���أ�����������Ϣ
#define KEY_MSG_SWITCH_HOLD				1		// ��Ϣ��Ӧ���أ�����������Ϣ
#define KEY_MSG_SWITCH_REPEAT			1		// ��Ϣ��Ӧ���أ������ظ���Ϣ
#define KEY_MSG_SWITCH_UP				1		// ��Ϣ��Ӧ���أ������ͷ���Ϣ

/******************************************************************************
* ��ϵͳ��������
* �벻Ҫ�޸��������ֵ
******************************************************************************/
#ifndef	NULL
#define	NULL	0
#endif

#define	KEY_MSG_PRIORITY					1	// ��Ϣ������ʱ����0 - ����Ϣ���ȣ��Զ�����������Ϣ��1 - ����Ϣ���ȣ��Զ����Ǿ���Ϣ
#define	KEY_MSG_THROW_KEY_UP_AFTER_REPEAT	1	// �ظ������ͷź��Ƿ��׳������ͷ���Ϣ���˹��ܴ����ƣ��ȶ�֮ǰ��Ҫ��ֵ��Ϊ0��

// ������Ϣ����
typedef enum
{
	// ������Ϣ���ƣ����֧��������Ϣ
#if 1==KEY_MSG_SWITCH_DOWN
	KEY_MSG_DOWN,			// ��������������Ϣ
#endif
#if 1==KEY_MSG_SWITCH_UP
	KEY_MSG_UP,				// �������������Ϣ
#endif
#if 1==KEY_MSG_SWITCH_HOLD
	KEY_MSG_HOLD,			// ����������Ϣ
#endif
#if 1==KEY_MSG_SWITCH_REPEAT
	KEY_MSG_REPEAT,			// �����ظ���Ϣ���ڰ�������֮���������׳�
#endif
	
	/*=============================================================
	** ���������ֹ�޸ģ����ұ���λ��������������
	==============================================================*/
	KEY_MSG_MAX,			// ������Ϣ����
	KEY_MSG_NULL,			// �Ƿ�����Ϣ
}KEY_MSG_E;


// ������Ϣ�ڵ㣬��������ʱ���λ0�������ͷ�ʱ���λΪ1����7λΪ����״̬����ֵ��ʹ��ʱ��Ҫע������¼����
typedef struct
{
	uint8	end;
	uint16	stateCount[KEY_INFO_SIZE];				// ÿ��Ԫ�����λΪ����״̬��0Ϊ���£�1Ϊ�ͷţ�����λΪ��¼����
	void	(*service[KEY_MSG_MAX])(uint32 param);	// ������Ӧ
	uint32	param[KEY_MSG_MAX];						// ������Ӧ����Ĳ���
}KEYMSG_INFO_CB;

// ������Ϣ���У����ڱ����׳�����Ϣ
typedef struct
{
	uint8	head;
	uint8	end;
	struct
	{
		KEY_NAME_E	keyName;	// ������
		KEY_MSG_E	keyMsg;		// ��Ϣ
	}node[KEY_MSG_QUEUE_SIZE];
}KEY_MSG_QUEUE;

// �������ƽṹ��
typedef struct
{
	volatile uint8	scanRequest;					// ����ɨ�������־
	
	uint8	keyValue[KEY_STATE_BYTE_COUNT];			// ����ֵ����ʱ���£�ÿ��bit����һ������״̬��
													// ÿ��������Ӧ��λ�� KEY_NAME_E �еĶ������
													// 1-8������ʱ����Ҫ1���ֽڱ��
													// 9-16������ʱ����Ҫ2���ֽڱ��
													// ��ˣ��ð���ֵ���г��ȼ��㹫ʽΪ (KEY_NAME_MAX + 7)/8
#if 0==KEY_MSG_THROW_KEY_UP_AFTER_REPEAT			// �������׳� KEY_REPEAT ��Ϣ���������Ҫ�׳������ͷ���Ϣ����Ҫ�б�־λ��¼
	uint8	keyUpMsgMaskBit[KEY_STATE_BYTE_COUNT];	// �ο������˵��
													// ��־λΪ1����һ�ν�ֹ�׳� KEY_UP ��Ϣ��
													// ��־λΪ0�������� KEY_UP ��Ϣ
#endif
	KEYMSG_INFO_CB	keyMsgInfo[KEY_NAME_MAX];		// ������Ϣ�ڵ�
	KEY_MSG_QUEUE	msgQueue;						// ������Ϣ������
	
	void(*KeyUpdate)(uint8* p8bitKeyValueArray);	// ����ˢ�º�������Ҫʹ�����ṩ�ص������ӿڣ��ڻص�������Ϊ������ֵ

	// ��Ϣ��������
	BOOL msgMaskRequest[KEY_NAME_MAX][KEY_MSG_MAX];
	BOOL globalMaskRequest;							// ȫ���������󣬴�������λ�󣬱���Ҫ��⵽����ȫ���ͷź��������������׳��κ���Ϣ

	BOOL addDecKeyUpRercoveryRequest;				// +-������ָ�����
	BOOL addKeyInvalidRequest;						// +����Ч����
}KEYMSG_CB;

extern KEYMSG_CB keyMsgCB;


/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/
// ���ӿ����ơ�ģ���ʼ���ӿ�
// ���������ơ�KEYMSG_Init
// ���ӿڹ��ܡ�������Ϣ����ģ���ʼ���ӿ�
// ��ʹ�÷������� main �д�ѭ��֮ǰ���ã�ע�⣬�ڵ��ñ��ӿ�֮ǰ��������ɶ�ʱ��ģ���ʼ��
void KEYMSG_Init(void);

// ���ӿ����ơ�����ɨ��ص�����ע��ӿ�
// ���������ơ�KEYMSG_RegisterKeyScanCallback
// ���ӿڹ��ܡ�ע�ᰴ��ɨ��ص������ӿ�
// ��ʹ�÷������ڰ�����ȡģ���У�����һ�����������βεĺ���������ȡ���İ���ֵ�ӵ��ֽڿ�ʼ����ָ����ָ����
//			   ��ͨ���˽ӿ���ģ��ע�ᣬ��Ҫע����ǣ���������Ϊ0�������ͷ�Ϊ1
void KEYMSG_RegisterKeyScanCallback(void(*fun)(uint8* p8bitKeyValueArray));

// ���ӿ����ơ�������Ϣ��Ӧע��ӿ�
// ���������ơ�KEYMSG_RegisterMsgService
// ���ӿڹ��ܡ�������Ϣע��ӿ�
// ��ʹ�÷���������ע�ᰴ����Ϣ����ִ��֮ǰ������ע������ֱ�ӵ��ñ��ӿڸı���Ϣ����ע��ķ���������ʽ�����뱾�ӿ����β�һ��
void KEYMSG_RegisterMsgService(KEY_NAME_E keyName, KEY_MSG_E keyMsg, void(*service)(uint32 param), uint32 param);

// ���ӿ����ơ�ֹͣ��Ϣ��Ӧ�ӿ�
// ���������ơ�KEYMSG_StopService
// ���ӿڹ��ܡ�ע��������Ϣ����
// ��ʹ�÷�������Ҫֹͣ������Ϣ��Ӧʱ��ͨ�����ӿ�ֹͣ��ָ��������ָ����Ϣ����Ӧ
void KEYMSG_StopService(KEY_NAME_E keyName, KEY_MSG_E keyMsg);

// ���ӿ����ơ�ֹͣ������Ϣ��Ӧ�ӿ�
// ���������ơ�KEYMSG_StopAllService
// ���ӿڹ��ܡ�ע�����а�����Ϣ����
// ��ʹ�÷�����ֹͣ���а���������Ϣ����Ӧ
void KEYMSG_StopAllService(void);

// ���ӿ����ơ���ʱˢ�»ص�����
// ���������ơ�KEYMSG_CALLBACK_Scan
// ���ӿڹ��ܡ���������ص��������ڶ�ʱ�������£������ڵ��ã�ʵ�ְ���ɨ��
// ��ʹ�÷��������˺������붨ʱ���ж���Ӧ�����е��ã�ע�ⶨʱ�����ںͰ���ɨ�����ڵĹ�ϵ
void KEYMSG_CALLBACK_Scan(uint32 param);

// ���ӿ����ơ���������ӿ�
// ���������ơ�KEYMSG_Process
// ���ӿڹ��ܡ���������ӿ�
// ��ʹ�÷������� main �д�ѭ���ڵ���
void KEYMSG_Process(void);

// ����ָ��������ָ����Ϣ����������
void KEYMSG_SetMaskRequest(KEY_NAME_E keyName, KEY_MSG_E keyMsg);

// ���ָ��������ָ����Ϣ����������
void KEYMSG_ClearMaskRequest(KEY_NAME_E keyName, KEY_MSG_E keyMsg);

// ����ȫ����������
void KEYMSG_SetGlobalMaskRequest(void);

// ���ȫ����������
void KEYMSG_ClearGlobalMaskRequest(void);

// ����+-������ָ�����
void KEYMSG_SetAddDecKeyUpRecoveryRequest(BOOL state);

// ��ȡ+-������ָ�����
BOOL KEYMSG_GetAddDecKeyUpRecoveryRequest(void);


#endif

