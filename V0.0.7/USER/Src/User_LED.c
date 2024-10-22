#include "USER_Common.h"

void LED_RED_TRIG(uint32_t param);
void LED_YG_TRIG(uint32_t param);


/*

    TIMER_ID_LED_R,     //RED LED���ƶ�ʱ��
    TIMER_ID_LED_YG,    //YG LED���ƶ�ʱ��
*/
//ģʽ����
uint32_t MODE_NUM[LED_MOD_MAX][3] =
{
    {LED_MODE_PAIR_on,          LED_MODE_PAIR_off,          LED_MODE_PAIR_Count},
    {LED_MODE_CONNECT_on,       LED_MODE_CONNECT_off,   LED_MODE_CONNECT_Count},
    {LED_MODE_AWAKE_on,         LED_MODE_AWAKE_off,         LED_MODE_AWAKE_Count},
    {LED_MODE_LOW_POWER_on, LED_MODE_LOW_POWER_off, LED_MODE_LOW_POWER_Count},
};


//�������� - ��ʱ�� �� ��ʱ��
uint32_t action[LED_MAX][2] = {0};
//���������� - ��ǰ״̬1���� 0����
uint8_t action_status[LED_MAX] = {0};
//�������� ʣ�����������
uint32_t action_sum[LED_MAX] = {0};

void Led_Init()
{

    INIT_LED_R;
    INIT_LED_GY;
    nrf_gpio_cfg_output(USER_LED3);

    CLOSE_LED_R;
    CLOSE_LED_GY;
//  nrf_gpio_pin_clear(USER_LED3);
    nrf_gpio_pin_set(USER_LED3);

}



//ֹͣ��ǰLED
void Led_Uninit()
{

    TIMER_KillTask(TIMER_ID_LED_YG);
    action[LED_YG][0] = 0;
    action[LED_YG][1] = 0;

    action_status[LED_YG] = 0;
    action_sum[LED_YG] = 0;

    CLOSE_LED_GY;

}

void LED_RED_TRIG(uint32_t param)
{

    if (action_status[LED_RED]) //��ǰΪ�� ��ʼ��
    {
        action_status[LED_RED] = 0;
        CLOSE_LED_R;
        TIMER_AddTask(TIMER_ID_LED_R, action[LED_RED][1], LED_RED_TRIG, 0, -1, 0);

        if (0 == action_sum[LED_RED]) //����Ϊ0���رն�ʱ��
        {
            TIMER_KillTask(TIMER_ID_LED_R);
        }
        action_sum[LED_RED]--;
    }
    else//��ǰΪ����ʼ��
    {
        action_status[LED_RED] = 1;
        OPEN_LED_R;
        TIMER_AddTask(TIMER_ID_LED_R, action[LED_RED][0], LED_RED_TRIG, 0, -1, 0);
    }
}

void LED_YG_TRIG(uint32_t param)
{

    if (action_status[LED_YG]) //��ǰΪ�� ��ʼ��
    {
        action_status[LED_YG] = 0;
        CLOSE_LED_GY;
        TIMER_AddTask(TIMER_ID_LED_YG, action[LED_YG][1], LED_YG_TRIG, 0, -1, 0);

        if (0 == action_sum[LED_YG]) //����Ϊ0���رն�ʱ��
        {
            TIMER_KillTask(TIMER_ID_LED_YG);
        }
        action_sum[LED_YG]--;
    }
    else//��ǰΪ����ʼ��
    {
        action_status[LED_YG] = 1;
        OPEN_LED_GY;
        TIMER_AddTask(TIMER_ID_LED_YG, action[LED_YG][0], LED_YG_TRIG, 0, -1, 0);
    }
}

// ����LED
// �����֣�����ʱ�䣻���ʱ�䣻ѭ������
// һ�ο���һ�������һ�����ڣ�ѭ��������ִ��������ڵĴ���
void LED_SetMode(LED_NAME led_name, LED_MODE_NAME ledMode)
{

    if (LED_RED == led_name && action_sum[led_name])
    {
        return;
    }

    action[led_name][0] = MODE_NUM[ledMode][0];//����ʱ��
    action[led_name][1] = MODE_NUM[ledMode][1];

    action_sum[led_name] = MODE_NUM[ledMode][2];//ʣ������

    //��ǰ״̬��
    if (LED_RED == led_name)
    {

        action_status[led_name] = 1;
        OPEN_LED_R;
        TIMER_AddTask(TIMER_ID_LED_R, action[led_name][0], LED_RED_TRIG, 0, -1, 0);
    }
    else
    {
        action_status[led_name] = 1;
        OPEN_LED_GY;
        TIMER_AddTask(TIMER_ID_LED_YG, action[led_name][0], LED_YG_TRIG, 0, -1, 0);
    }


}