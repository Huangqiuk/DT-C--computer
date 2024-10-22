#include "USER_Common.h"
#include "nrf_saadc.h"

unsigned short VOL = 0;

//================================================================
//SAADC事件回调函数，只有一个缓存填满后才会进入事件回调函数
void saadc_callback(nrfx_saadc_evt_t const *p_event)
{

}
//初始化SAADC，配置使用的SAADC通道的参数和缓存
void saadc_init(void)
{
    ret_code_t err_code;
    //定义ADC通道配置结构体，并使用单端采样配置宏初始化
    //因为要采样的是芯片的VDD，所以单端采样配置宏的通道正极为：NRF_SAADC_INPUT_VDD
    nrf_saadc_channel_config_t channel_config =
        NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);
    //初始化SAADC，注册事件回调函数。
    nrfx_saadc_config_t  p_config = NRFX_SAADC_DEFAULT_CONFIG;
    err_code = nrf_drv_saadc_init(&p_config, saadc_callback);
    APP_ERROR_CHECK(err_code);
    //初始化SAADC通道0
    err_code = nrfx_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

}
//ADC过程处理
void ADC_Process()
{

    static uint8_t count = 0;

    if (5 == count++)
    {
        count = 0;
        VOL = Get_ADC();///获取电压值

        //电量是否低于10%
        if (BAT_LOW_PER > Get_Bat_Status())
        {
            LED_SetMode(LED_RED, LED_MODE_LOW_POWER);
        }
    }
}


//获取电量值
unsigned int Get_ADC()
{

    nrf_saadc_value_t  saadc_val;
    uint8_t buffer[50] = {0};

    //启动一次ADC采样（阻塞模式）。
    nrfx_saadc_sample_convert(0, &saadc_val);
    //NRF_LOG_INFO("Sample value is:  %d\r\n", saadc_val);
    //串口输出采样值计算得到的电压值。电压值 = 采样值 * 3.6 /1024
    //memset(buffer, 0, 50);
    //sprintf((char*)buffer ,"%.3fV-%d", saadc_val * 3.6 /1024, saadc_val);
    //NRF_LOG_INFO("%s", (char*)buffer);
    //Ble_Send_Data(buffer, strlen(buffer));
    return saadc_val * 3600 / 1024;

}


//获取电量百分比
//0-100
uint8_t Get_Bat_Status()
{
    //电量比例
    uint8_t Bat_percent = 0;
    if (VOL >= FULL_VOL)
    {
        Bat_percent = 100;
        //return 100;
    }
    else if (VOL <= LOW_VOL)
    {
        Bat_percent = 0;
        //return 0;
    }
    else
    {
        Bat_percent = (VOL - LOW_VOL) * 100 / (FULL_VOL - LOW_VOL);
        //return (VOL - LOW_VOL)*100 / (FULL_VOL - LOW_VOL);
    }
    return Bat_percent;
}

