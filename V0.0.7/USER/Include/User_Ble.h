
#ifndef _USAR_BLE_
#define _USAR_BLE_

extern unsigned short  m_conn_handle;

//====================�ⲿ����==============
//��������
unsigned char Ble_Send_Data(char* data, unsigned short len);
//��ʼ��Э��ջ
void user_ble_stack_init(void);
//��ʼ�㲥
void user_advertising_start(void);

//���ù㲥���ֲ���ʼ�㲥
//1:���ģʽ 0:��ͨģʽ
void advertising_start_setName(unsigned int param);
//--------------------------------------------
//��ʼ���㲥����
void user_advertising_init(void);
//���Ӳ�����ʼ��
void conn_params_init();
//gap������ʼ��
void gap_params_init(void);
//gatt������ʼ��
void gatt_init();

//�����ʼ��
void services_init();



#endif
