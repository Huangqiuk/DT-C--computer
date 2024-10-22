#include "USER_Common.h"

//按键键值 - 协议
#define KEY_M_profile 0x04
#define KEY_DEC_profile 0x20
#define KEY_ADD_profile 0x40

//检验和
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
//连接后10s超时断开连接
void TimeOut_10S(uint32_t param)
{

    sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    //STATE_EnterState(STATE_BROADCAST);
}


//按键回调
void KEY_M_Handler(uint32_t param)
{

    uint8_t buffer[10] = {0x55, 0xA1, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buffer[5] = param;//填充按键值
    //填充电压值
    buffer[6] = (uint8_t)(VOL >> 8);
    buffer[7] = (uint8_t)(VOL & 0xff);
    buffer[8] = Get_Bat_Status();//填充百分比
    buffer[9] = CheckSum(buffer, 9);//校验和

    NRF_LOG_INFO("%d - %d", VOL, buffer[8]);

    if (m_conn_handle == BLE_CONN_HANDLE_INVALID) return;
    Ble_Send_Data((char *)buffer, 10);

}
//按键回调
void KEY_DEC_Handler(uint32_t param)
{

    uint8_t buffer[10] = {0x55, 0xA1, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buffer[5] = param;//填充按键值
    //填充电压值
    buffer[6] = (uint8_t)(VOL >> 8);
    buffer[7] = (uint8_t)(VOL & 0xff);
    buffer[8] = Get_Bat_Status();//填充百分比
    buffer[9] = CheckSum(buffer, 9);//校验和

    if (m_conn_handle == BLE_CONN_HANDLE_INVALID) return;
    Ble_Send_Data((char *)buffer, 10);

}
//按键回调
void KEY_ADD_Handler(uint32_t param)
{

    uint8_t buffer[10] = {0x55, 0xA1, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buffer[5] = param;//填充按键值
    //填充电压值
    buffer[6] = (uint8_t)(VOL >> 8);
    buffer[7] = (uint8_t)(VOL & 0xff);
    buffer[8] = Get_Bat_Status();//填充百分比
    buffer[9] = CheckSum(buffer, 9);//校验和

    if (m_conn_handle == BLE_CONN_HANDLE_INVALID) return;
    Ble_Send_Data((char *)buffer, 10);
}
//抬起回调
void KEY_UP_Handler(uint32_t param)
{

//    uint8_t buffer[10] = {0x55, 0xA1, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//    buffer[5] = param;//填充按键值
//    //填充电压值
//    buffer[6] = (uint8_t)(VOL >> 8);
//    buffer[7] = (uint8_t)(VOL & 0xff);
//    buffer[8] = Get_Bat_Status();//填充百分比
//    buffer[9] = CheckSum(buffer, 9);//校验和

//    if (m_conn_handle == BLE_CONN_HANDLE_INVALID) return;
//    Ble_Send_Data((char *)buffer, 10);
    LED_SetMode(LED_YG, LED_MODE_AWAKE);//设置配对灯光

}

// 状态机结构体定义
STATE_CB stateCB;


// 状态机初始化
void STATE_Init(void)
{
    // 默认状态为空
    stateCB.state       = STATE_NULL;
    stateCB.preState    = STATE_NULL;
    stateCB.persLoveState = STATE_NULL;

    STATE_EnterState(STATE_BROADCAST);

}

// 状态迁移
void STATE_EnterState(STATE_E state)
{

    stateCB.preState = stateCB.state;
    stateCB.state = state;

    switch (stateCB.state)
    {

    // ■■■■■■■■■■■■ 空状态 ■■■■■■■■■■■■
    case STATE_NULL:

        break;

    case STATE_BROADCAST:   // 广播状态


        Led_Uninit();//复位LED
        //60S超时定时器 TIMER_ID_1MIN_TIMEOUT
        TIMER_AddTask(TIMER_ID_1MIN_TIMEOUT, 60000, STATE_EnterState, STATE_SLEEP, 1, ACTION_MODE_ADD_TO_QUEUE);
        //长按组合键进入配对状态
        KEYMSG_StopAllService();
        //抬起
        KEYMSG_RegisterMsgService(BUTTON_M, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_DEC, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_ADD, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_M_DEC, KEY_MSG_HOLD, STATE_EnterState, STATE_PAIR);

        //是否是从配对广播超时回到普通广播
        if (STATE_PAIR == stateCB.preState)
        {
            //改变广播名字
            advertising_start_setName(0);
        }
        //从连接状态退回到广播状态
        else if (STATE_CONNECT == stateCB.preState)
        {
            //启动普通广播
            user_advertising_start();
            advertising_start_setName(0);
        }
        //低功耗唤醒
        else
        {
            user_advertising_start();//默认普通广播名
            LED_SetMode(LED_YG, LED_MODE_AWAKE);
        }
        break;

    case STATE_PAIR:            // 配对状态
        //改变广播名
        advertising_start_setName(1);
        //60S超时定时器 TIMER_ID_1MIN_TIMEOUT
        TIMER_AddTask(TIMER_ID_1MIN_TIMEOUT, 60000, STATE_EnterState, STATE_BROADCAST, 1, ACTION_MODE_ADD_TO_QUEUE);
        KEYMSG_StopAllService();//注销按键

        LED_SetMode(LED_YG, LED_MODE_PAIR);//设置配对灯光

        break;

    case STATE_CONNECT:     // 连接状态
        //删除超时定时器
        TIMER_KillTask(TIMER_ID_1MIN_TIMEOUT);
        //开启10S认证定时器
        //TimeOut_10S
        TIMER_AddTask(TIMER_ID_10SEC_TIMEOUT, 10000, TimeOut_10S, 0, 1, ACTION_MODE_ADD_TO_QUEUE);

        //注册按键触发；抬起
        KEYMSG_StopAllService();    //注册按键
        KEYMSG_RegisterMsgService(BUTTON_M, KEY_MSG_DOWN, KEY_M_Handler, KEY_M_profile);
        KEYMSG_RegisterMsgService(BUTTON_DEC, KEY_MSG_DOWN, KEY_DEC_Handler, KEY_DEC_profile);
        KEYMSG_RegisterMsgService(BUTTON_ADD, KEY_MSG_DOWN, KEY_ADD_Handler, KEY_ADD_profile);
        //抬起
        KEYMSG_RegisterMsgService(BUTTON_M, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_DEC, KEY_MSG_UP, KEY_UP_Handler, 0);
        KEYMSG_RegisterMsgService(BUTTON_ADD, KEY_MSG_UP, KEY_UP_Handler, 0);

        LED_SetMode(LED_YG, LED_MODE_CONNECT);
        //规定时间内接收数据包定时器

        break;

    case STATE_SLEEP:           //低功耗状态

        CLOSE_LED_R;
        CLOSE_LED_GY;

        KEY_GPIO_Init();//开启按键唤醒
        sd_power_system_off();
        break;

    default:
        break;
    }
}

// 状态机处理
void STATE_Process(void)
{

    switch (stateCB.state)
    {
    // ■■■■■■■■■■■■ 空状态 ■■■■■■■■■■■■
    case STATE_NULL:


        break;

    case STATE_BROADCAST:   // 广播状态

        break;

    case STATE_PAIR:            // 配对状态

        break;

    case STATE_CONNECT:     // 连接状态

        break;


    default:

        break;


    }
}