#include "USER_Common.h"

void LED_RED_TRIG(uint32_t param);
void LED_YG_TRIG(uint32_t param);


/*

    TIMER_ID_LED_R,     //RED LED控制定时器
    TIMER_ID_LED_YG,    //YG LED控制定时器
*/
//模式数量
uint32_t MODE_NUM[LED_MOD_MAX][3] =
{
    {LED_MODE_PAIR_on,          LED_MODE_PAIR_off,          LED_MODE_PAIR_Count},
    {LED_MODE_CONNECT_on,       LED_MODE_CONNECT_off,   LED_MODE_CONNECT_Count},
    {LED_MODE_AWAKE_on,         LED_MODE_AWAKE_off,         LED_MODE_AWAKE_Count},
    {LED_MODE_LOW_POWER_on, LED_MODE_LOW_POWER_off, LED_MODE_LOW_POWER_Count},
};


//动作队列 - 亮时长 ； 灭时长
uint32_t action[LED_MAX][2] = {0};
//动作计数器 - 当前状态1：亮 0：灭
uint8_t action_status[LED_MAX] = {0};
//动作周期 剩余的周期数量
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



//停止当前LED
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

    if (action_status[LED_RED]) //当前为亮 则开始灭
    {
        action_status[LED_RED] = 0;
        CLOSE_LED_R;
        TIMER_AddTask(TIMER_ID_LED_R, action[LED_RED][1], LED_RED_TRIG, 0, -1, 0);

        if (0 == action_sum[LED_RED]) //周期为0，关闭定时器
        {
            TIMER_KillTask(TIMER_ID_LED_R);
        }
        action_sum[LED_RED]--;
    }
    else//当前为灭则开始亮
    {
        action_status[LED_RED] = 1;
        OPEN_LED_R;
        TIMER_AddTask(TIMER_ID_LED_R, action[LED_RED][0], LED_RED_TRIG, 0, -1, 0);
    }
}

void LED_YG_TRIG(uint32_t param)
{

    if (action_status[LED_YG]) //当前为亮 则开始灭
    {
        action_status[LED_YG] = 0;
        CLOSE_LED_GY;
        TIMER_AddTask(TIMER_ID_LED_YG, action[LED_YG][1], LED_YG_TRIG, 0, -1, 0);

        if (0 == action_sum[LED_YG]) //周期为0，关闭定时器
        {
            TIMER_KillTask(TIMER_ID_LED_YG);
        }
        action_sum[LED_YG]--;
    }
    else//当前为灭则开始亮
    {
        action_status[LED_YG] = 1;
        OPEN_LED_GY;
        TIMER_AddTask(TIMER_ID_LED_YG, action[LED_YG][0], LED_YG_TRIG, 0, -1, 0);
    }
}

// 设置LED
// 灯名字；开灯时间；灭灯时间；循环次数
// 一次开灯一次灭灯是一个周期；循环次数是执行这个周期的次数
void LED_SetMode(LED_NAME led_name, LED_MODE_NAME ledMode)
{

    if (LED_RED == led_name && action_sum[led_name])
    {
        return;
    }

    action[led_name][0] = MODE_NUM[ledMode][0];//亮灭时长
    action[led_name][1] = MODE_NUM[ledMode][1];

    action_sum[led_name] = MODE_NUM[ledMode][2];//剩余周期

    //当前状态亮
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