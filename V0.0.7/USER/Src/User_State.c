#include "USER_Common.h"

//������ֵ - Э��
#define KEY_M_profile 0x04
#define KEY_DEC_profile 0x20
#define KEY_ADD_profile 0x40

//�����
uint8_t CheckSum(uint8_t *data, uint8_t length)
{
    uint8_t cc = 0;
    uint8_t i = 0;
    for (i = 0; i < length; i++)
    {
        cc ^= data[i];
    }
    return ~cc;

}
//���Ӻ�10s��ʱ�Ͽ�����
void TimeOut_10S(uint32_t param)
{

    sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    //STATE_EnterState(STATE_BROADCAST);
}


//�����ص�
void KEY_M_Handler(uint32_t param)
{

    uint8_t buffer[10] = {0x55, 0xA1, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buffer[5] = param;//��䰴��ֵ
    //����ѹֵ
    buffer[6] = (uint8_t)(VOL >> 8);
    buffer[7] = (uint8_t)(VOL & 0xff);
    buffer[8] = Get_Bat_Status();//���ٷֱ�
    buffer[9] = CheckSum(buffer, 9);//У���

    NRF_LOG_INFO("%d - %d", VOL, buffer[8]);

    if (m_conn_handle == BLE_CONN_HANDLE_INVALID) return;
    Ble_Send_Data((char *)buffer, 10);

}
//�����ص�
void KEY_DEC_Handler(uint32_t param)
{

    uint8_t buffer[10] = {0x55, 0xA1, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buffer[5] = param;//��䰴��ֵ
    //����ѹֵ
    buffer[6] = (uint8_t)(VOL >> 8);
    buffer[7] = (uint8_t)(VOL & 0xff);
    buffer[8] = Get_Bat_Status();//���ٷֱ�
    buffer[9] = CheckSum(buffer, 9);//У���

    if (m_conn_handle == BLE_CONN_HANDLE_INVALID) return;
    Ble_Send_Data((char *)buffer, 10);

}
//�����ص�
void KEY_ADD_Handler(uint32_t param)
{

    uint8_t buffer[10] = {0x55, 0xA1, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buffer[5] = param;//��䰴��ֵ
    //����ѹֵ
    buffer[6] = (uint8_t)(VOL >> 8);
    buffer[7] = (uint8_t)(VOL & 0xff);
    buffer[8] = Get_Bat_Status();//���ٷֱ�
    buffer[9] = CheckSum(buffer, 9);//У���

    if (m_conn_handle == BLE_CONN_HANDLE_INVALID) return;
    Ble_Send_Data((char *)buffer, 10);
}
//̧��ص�
void KEY_UP_Handler(uint32_t param)
{

//    uint8_t buffer[10] = {0x55, 0xA1, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//    buffer[5] = param;//��䰴��ֵ
//    //����ѹֵ
//    buffer[6] = (uint8_t)(VOL >> 8);
//    buffer[7] = (uint8_t)(VOL & 0xff);
//    buffer[8] = Get_Bat_Status();//���ٷֱ�
//    buffer[9] = CheckSum(buffer, 9);//У���

//    if (m_conn_handle == BLE_CONN_HANDLE_INVALID) return;
//    Ble_Send_Data((char *)buffer, 10);
    LED_SetMode(LED_YG, LED_MODE_AWAKE);//������Եƹ�

}

// ״̬���ṹ�嶨��
STATE_CB stateCB;


// ״̬����ʼ��
void STATE_Init(void)
{
    // Ĭ��״̬Ϊ��
    stateCB.state       = STATE_NULL;
    stateCB.preState    = STATE_NULL;
    stateCB.persLoveState = STATE_NULL;

    STATE_EnterState(STATE_BROADCAST);

}

// ״̬Ǩ��
void STATE_EnterState(STATE_E state)
{

    stateCB.preState = stateCB.state;
    stateCB.state = state;

    switch (stateCB.state)
    {

    // ������������������������ ��״̬ ������������������������
    case STATE_NULL:

        break;

    case STATE_BROADCAST:   // �㲥״̬


        Led_Uninit();//��λLED
        //60S��ʱ��ʱ�� TIMER_ID_1MIN_TIMEOUT
        TIMER_AddTask(TIMER_ID_1MIN_TIMEOUT, 60000, STATE_EnterState, STATE_SLEEP, 1, ACTION_MODE_ADD_TO_QUEUE);
        //������ϼ��������״̬
        KEYMSG_StopAllService();
        //̧��
        KEYMSG_RegisterMsgService(BUTTON_M, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_DEC, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_ADD, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_M_DEC, KEY_MSG_HOLD, STATE_EnterState, STATE_PAIR);

        //�Ƿ��Ǵ���Թ㲥��ʱ�ص���ͨ�㲥
        if (STATE_PAIR == stateCB.preState)
        {
            //�ı�㲥����
            advertising_start_setName(0);
        }
        //������״̬�˻ص��㲥״̬
        else if (STATE_CONNECT == stateCB.preState)
        {
            //������ͨ�㲥
            user_advertising_start();
            advertising_start_setName(0);
        }
        //�͹��Ļ���
        else
        {
            user_advertising_start();//Ĭ����ͨ�㲥��
            LED_SetMode(LED_YG, LED_MODE_AWAKE);
        }
        break;

    case STATE_PAIR:            // ���״̬
        //�ı�㲥��
        advertising_start_setName(1);
        //60S��ʱ��ʱ�� TIMER_ID_1MIN_TIMEOUT
        TIMER_AddTask(TIMER_ID_1MIN_TIMEOUT, 60000, STATE_EnterState, STATE_BROADCAST, 1, ACTION_MODE_ADD_TO_QUEUE);
        KEYMSG_StopAllService();//ע������

        LED_SetMode(LED_YG, LED_MODE_PAIR);//������Եƹ�

        break;

    case STATE_CONNECT:     // ����״̬
        //ɾ����ʱ��ʱ��
        TIMER_KillTask(TIMER_ID_1MIN_TIMEOUT);
        //����10S��֤��ʱ��
        //TimeOut_10S
        TIMER_AddTask(TIMER_ID_10SEC_TIMEOUT, 10000, TimeOut_10S, 0, 1, ACTION_MODE_ADD_TO_QUEUE);

        //ע�ᰴ��������̧��
        KEYMSG_StopAllService();    //ע�ᰴ��
        KEYMSG_RegisterMsgService(BUTTON_M, KEY_MSG_DOWN, KEY_M_Handler, KEY_M_profile);
        KEYMSG_RegisterMsgService(BUTTON_DEC, KEY_MSG_DOWN, KEY_DEC_Handler, KEY_DEC_profile);
        KEYMSG_RegisterMsgService(BUTTON_ADD, KEY_MSG_DOWN, KEY_ADD_Handler, KEY_ADD_profile);
        //̧��
        KEYMSG_RegisterMsgService(BUTTON_M, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_DEC, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_ADD, KEY_MSG_UP, KEY_UP_Handler, 0);

        LED_SetMode(LED_YG, LED_MODE_CONNECT);
        //�涨ʱ���ڽ������ݰ���ʱ��

        break;

    case STATE_SLEEP:           //�͹���״̬

        CLOSE_LED_R;
        CLOSE_LED_GY;

        KEY_GPIO_Init();//������������
        sd_power_system_off();
        break;

    default:
        break;
    }
}

// ״̬������
void STATE_Process(void)
{

    switch (stateCB.state)
    {
    // ������������������������ ��״̬ ������������������������
    case STATE_NULL:


        break;

    case STATE_BROADCAST:   // �㲥״̬

        break;

    case STATE_PAIR:            // ���״̬

        break;

    case STATE_CONNECT:     // ����״̬

        break;


    default:

        break;


    }
}