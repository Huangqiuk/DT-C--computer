#include "USER_Common.h"


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}




/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


int main(void)
{


    uint32_t err_code = 0;
    char buff[10];

    bool erase_bonds = 0;

    // Initialize.
    //uart_init();
    log_init();
    timers_init();
    //buttons_leds_init(&erase_bonds);
    power_management_init();

    //设置dc-dc为供电方式
    //默认LDO
    NRF_POWER->DCDCEN = 1;
    //BLE Init
    user_ble_stack_init();

    NRF_LOG_INFO("Debug logging over RTT started.");



    //初始化SAADC
    saadc_init();


    //定时器初始TIMER
    TIMER_Init();
    //按键初始化
    KEY_Init();
    //按键外中断初始化
    //KEY_GPIO_Init();
    //LED初始化
    Led_Init();
    //掉电储存初始化
    //User_NVM_Init();
    STATE_Init();//状态机初始化


    //启动普通广播
    //user_advertising_start();

    for (;;)
    {
        TIMER_TaskQueueProcess();//定时器过程处理
        KEYMSG_Process();//按键过程处理
        STATE_Process();//状态机
        ADC_Process();//ADC过程处理



        //------------------
        idle_state_handle();
    }
}


/**
 * @}
 */
