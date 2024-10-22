#include "USER_Common.h"
#include "nrf_saadc.h"

unsigned short VOL = 0;

//================================================================
//SAADC�¼��ص�������ֻ��һ������������Ż�����¼��ص�����
void saadc_callback(nrfx_saadc_evt_t const *p_event)
{

}
//��ʼ��SAADC������ʹ�õ�SAADCͨ���Ĳ����ͻ���
void saadc_init(void)
{
    ret_code_t err_code;
    //����ADCͨ�����ýṹ�壬��ʹ�õ��˲������ú��ʼ��
    //��ΪҪ��������оƬ��VDD�����Ե��˲������ú��ͨ������Ϊ��NRF_SAADC_INPUT_VDD
    nrf_saadc_channel_config_t channel_config =
        NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);
    //��ʼ��SAADC��ע���¼��ص�������
    nrfx_saadc_config_t  p_config = NRFX_SAADC_DEFAULT_CONFIG;
    err_code = nrf_drv_saadc_init(&p_config, saadc_callback);
    APP_ERROR_CHECK(err_code);
    //��ʼ��SAADCͨ��0
    err_code = nrfx_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

}
//ADC���̴���
void ADC_Process()
{

    static uint8_t count = 0;

    if (5 == count++)
    {
        count = 0;
        VOL = Get_ADC();///��ȡ��ѹֵ

        //�����Ƿ����10%
        if (BAT_LOW_PER > Get_Bat_Status())
        {
            LED_SetMode(LED_RED, LED_MODE_LOW_POWER);
        }
    }
}


//��ȡ����ֵ
unsigned int Get_ADC()
{

    nrf_saadc_value_t  saadc_val;
    uint8_t buffer[50] = {0};

    //����һ��ADC����������ģʽ����
    nrfx_saadc_sample_convert(0, &saadc_val);
    //NRF_LOG_INFO("Sample value is:  %d\r\n", saadc_val);
    //�����������ֵ����õ��ĵ�ѹֵ����ѹֵ = ����ֵ * 3.6 /1024
    //memset(buffer, 0, 50);
    //sprintf((char*)buffer ,"%.3fV-%d", saadc_val * 3.6 /1024, saadc_val);
    //NRF_LOG_INFO("%s", (char*)buffer);
    //Ble_Send_Data(buffer, strlen(buffer));
    return saadc_val * 3600 / 1024;

}


//��ȡ�����ٷֱ�
//0-100
uint8_t Get_Bat_Status()
{
    //��������
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

