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

    //����dc-dcΪ���緽ʽ
    //Ĭ��LDO
    NRF_POWER->DCDCEN = 1;
    //BLE Init
    user_ble_stack_init();

    NRF_LOG_INFO("Debug logging over RTT started.");



    //��ʼ��SAADC
    saadc_init();


    //��ʱ����ʼTIMER
    TIMER_Init();
    //������ʼ��
    KEY_Init();
    //�������жϳ�ʼ��
    //KEY_GPIO_Init();
    //LED��ʼ��
    Led_Init();
    //���索���ʼ��
    //User_NVM_Init();
    STATE_Init();//״̬����ʼ��


    //������ͨ�㲥
    //user_advertising_start();

    for (;;)
    {
        TIMER_TaskQueueProcess();//��ʱ�����̴���
        KEYMSG_Process();//�������̴���
        STATE_Process();//״̬��
        ADC_Process();//ADC���̴���



        //------------------
        idle_state_handle();
    }
}


/**
 * @}
 */