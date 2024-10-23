#include "AvoPin.h"
#include "system.h"
#include "timer.h"

// �������ų�ʼ��
void AVO_PIN_Init(void)
{
    // ��ʱ��
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOE);

    // ����Ϊ����̬ģʽ
    gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);      // LED_VCC_TEST_EN
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_12); // LED_HBEAM_TEST_EN
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_13); // LED_LBEAM_TEST_EN

    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);   // LEFT_VCC_TEST_EN_EN
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_4);   // RIGHT_VCC_TEST_EN_EN

    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_12);   // LED_VCC_EN
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_13);   // LED_HBEAM_EN
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);   // LED_LBEAM_EN
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_15);   // LEFT_VCC_EN
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);    // RIGHT_VCC_EN

    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_7);  // THROTTLE_GND_TEST_EN_ON
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_15); // THROTTLE_VCC_TEST_EN_ON
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_7);  // BRAKE_GND_TEST_EN_ON
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);  // BRAKE_VCC_TEST_EN_ON

    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_1);  // DUT_POWER_TEST_EN_ON
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_5);  // DUT_VLK_TEST_EN_ON

    // USB���ز���
    gpio_init(GPIOE, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_4);  //
    gpio_init(GPIOE, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_5);  //
    gpio_init(GPIOE, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);  //

    // UART
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);  // DUT_TO_DTA_OFF
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0);  // UART_TEST_EN

    // VLK_PW_EN
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    
    // KEY_TEST_EN0
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    // KEY_TEST_EN1
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
    // KEY_TEST_EN2
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    // KEY_TEST_EN3
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    // KEY_TEST_EN4
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    // KEY_TEST_EN5
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    // KEY_TEST_EN6
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    // KEY_TEST_EN7
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    
    // ���ӱ���
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);  // DERAILLEUR_VCC_TEST_EN
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_1);  // DERAILLEUR_GND_TEST_EN
    
    // �˷���·VLK5V����ʹ��
    gpio_init(GPIOE, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0);  // 5VOUT_EN    
}

// �������Ÿ�λ
void AVO_PIN_Reset(void)
{
    gpio_bit_reset(GPIOD, GPIO_PIN_11);
    gpio_bit_reset(GPIOD, GPIO_PIN_12);
    gpio_bit_reset(GPIOD, GPIO_PIN_13);

    gpio_bit_reset(GPIOD, GPIO_PIN_3);
    gpio_bit_reset(GPIOD, GPIO_PIN_4);

    gpio_bit_reset(GPIOB, GPIO_PIN_12);
    gpio_bit_reset(GPIOB, GPIO_PIN_13);
    gpio_bit_reset(GPIOB, GPIO_PIN_14);
    gpio_bit_reset(GPIOB, GPIO_PIN_15);
    gpio_bit_reset(GPIOD, GPIO_PIN_8);

    gpio_bit_reset(GPIOC, GPIO_PIN_7);  // THROTTLE_GND_TEST_EN_ON
    gpio_bit_reset(GPIOA, GPIO_PIN_15); // THROTTLE_VCC_TEST_EN_ON
    gpio_bit_reset(GPIOD, GPIO_PIN_7);  // BRAKE_GND_TEST_EN_ON
    gpio_bit_reset(GPIOC, GPIO_PIN_6);  // BRAKE_VCC_TEST_EN_ON

    gpio_bit_reset(GPIOD, GPIO_PIN_1);
    gpio_bit_reset(GPIOD, GPIO_PIN_5);

    gpio_bit_reset(GPIOA, GPIO_PIN_8);  // DUT_TO_DTA_OFF
    gpio_bit_reset(GPIOD, GPIO_PIN_0);  // UART_TEST_EN

}

// ��Դʹ�ܡ��˷��߱�VLK����
void _5VOUT_EN(uint8_t level)
{
    //1.GPIO����ʱ��ʹ��
    rcu_periph_clock_enable(RCU_GPIOE);

    //2.GPIO����Ϊ�������ģʽ
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);

    //3.��������Ϊ�͵�ƽ��ߵ�ƽ
    if (level == 1)
    {
        gpio_bit_set(GPIOE, GPIO_PIN_0);
    }
    else
    {
        gpio_bit_reset(GPIOE, GPIO_PIN_0);
    }
}

void THROTTLE_VCC_TEST_EN(uint8_t level)
{
    //1.GPIO����ʱ��ʹ��
    rcu_periph_clock_enable(RCU_GPIOA);

    //3.GPIO����Ϊ�������ģʽ
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

    //4.��ӳ�����ã�JTAG-DP���ú�SW-DPʹ��
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    //5.��������Ϊ�͵�ƽ��ߵ�ƽ
    if (level == 1)
    {
        gpio_bit_set(GPIOA, GPIO_PIN_15);
    }
    else
    {
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
    }
}

// DAC��ʼ��
void DAC_Init(void)
{
    rcu_periph_clock_enable(RCU_DAC);
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_5);

    // ����DAC1
    dac_trigger_disable(DAC0);
    dac_wave_mode_config(DAC0, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC0);

    // ����DAC2
    dac_trigger_disable(DAC1);
    dac_wave_mode_config(DAC1, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC1);
}

// DACʧ��
void DAC_Uinit(void)
{
    dac_disable(DAC0);
    dac_disable(DAC1);
}

// ����DAC���
void DAC0_output(uint16_t value)
{
    dac_enable(DAC0); //ʹ�� DAC0
    dac_data_set(DAC0, DAC_ALIGN_12B_R, (value / 2 * 4096 / 3300));  // �������ֵ
    dac_software_trigger_enable(DAC0);
}

// ɲ��DAC���
void DAC1_output(uint16_t value)
{
    dac_enable(DAC1); //ʹ�� DAC0

    // �˷ŷŴ����Ŵ�����
    dac_data_set(DAC1, DAC_ALIGN_12B_R, (value / 2 * 4096 / 3300)); // �������ֵ
    dac_software_trigger_enable(DAC1);
}

// ����ʱ
void Delayus(uint16 i)
{
    // �ں�72MHz��1us
    while (i--)
    {
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();  // 10
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();  // 10
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();  // 10
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();  // 6
    }
}

// ����ʱ
void Delayms(uint16 i)
{
    uint16 x;
    uint16 y;

    // �ں�72MHz��1ms
    while (i--)
    {
        x = 1000;
        while (x--)
        {
            y = 10;
            while (y--)
            {
                NOP();
            }
        }
    }
}

// USBӲ��ADC��ʼ��
void ADC_HwInit(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);                                 // ʱ��GPIOCʱ��
    rcu_periph_clock_enable(RCU_ADC0);                                  // ʱ��ADC0ʱ��

    adc_deinit(ADC0);                                                   // ��λADC
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);                        // 72/6 = 12 Mhz(���Ƶ�ʲ��ܳ���40M)

    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);     

    // ADC0 ����
    adc_mode_config(ADC_MODE_FREE);                                     // ����ģʽ
    adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);          // ɨ��ģʽ�ر�
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);    // ����ת��ģʽ�ر�
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);  // ����ͨ�����������
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);               // �����Ҷ���
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);            // ����ͨ������1

    // ����ADC
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_11, ADC_SAMPLETIME_55POINT5);
//    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_12, ADC_SAMPLETIME_55POINT5);

    adc_enable(ADC0);                                                   // ʱ��ADC0
    Delayms(10);
    adc_calibration_enable(ADC0);                                       // ADCУ׼�͸�λУ׼

    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);     // ����ADC0ת��
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
}

/**
 @brief ADC��ȡ
 @return USB�����ѹ����λ��mv
*/
uint16_t USB_ADC_Read(void)
{
    uint16_t real_voltage = 0;
    uint16_t adc_values[10] = {0};
    uint16_t min_value = adc_values[0];
    uint16_t max_value = adc_values[0];
    uint16_t adcValue = 0;
    uint16_t vol = 0;
//    uint16_t sum = 0;
//    int count = 0;
    int i = 0;
    int j = 0;

    // ����ADC 10��
    for (i = 0; i < 10; i++)
    {
		
		// ����ADCͨ��ת��˳�򣬲���ʱ��Ϊ55.5��ʱ������
        adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_55POINT5);
        
        // ����û�в����ⲿ����������ʹ���������ADCת��
        adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);				
				
        while (!adc_flag_get(ADC0, ADC_FLAG_EOC));                      // �ȴ��������
        adc_flag_clear(ADC0, ADC_FLAG_EOC);                             // ���������־
        adc_values[i] = adc_regular_data_read(ADC0);                    // ��ȡADC����
				

    }

    // �������������ȥ��������Сֵ
    for (j = 1; j < 10; j++)
    {
        if (adc_values[j] < min_value)
        {
            min_value = adc_values[j];
        }
        if (adc_values[j] > max_value)
        {
            max_value = adc_values[j];
        }
    }

//    for (j = 0; j < 10; j++)
//    {
//        if (adc_values[j] != min_value && adc_values[j] != max_value)
//        {
//            sum += adc_values[j];
//            count++;
//        }
//    }

//    adcValue = sum / count;
    adcValue = max_value;

    vol = adcValue * 3300 / 4095;                                   // ת���ɵ�ѹֵ����λmv
    real_voltage = 2 * vol;     // ʵ�ʵ�ѹֵ
    return real_voltage;
}

/**
 * @brief ADC��ȡ
 * @return USB���ص�����������
 */
uint16_t USB_CURRENT_Read(void)
{
    uint16_t current = 0;
    uint16_t adc_values[10] = {0};
    uint16_t min_value = adc_values[0];
    uint16_t max_value = adc_values[0];
    uint16_t vol = 0;
    uint16_t adcValue = 0;
    uint16_t sum = 0;
    int count = 0;
    int i = 0;
    int j = 0;

    // ����ADC 10��
    for (i = 0; i < 10; i++)
    {
        // ����ADCͨ��ת��˳�򣬲���ʱ��Ϊ55.5��ʱ������
        adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_11, ADC_SAMPLETIME_55POINT5);

        // ����û�в����ⲿ����������ʹ���������ADCת��
        adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
        while (!adc_flag_get(ADC0, ADC_FLAG_EOC));                      // �ȴ��������
        adc_flag_clear(ADC0, ADC_FLAG_EOC);                             // ���������־
        adc_values[i] = adc_regular_data_read(ADC0);                         // ��ȡADC����
    }

    // �������������ȥ��������Сֵ
    for (j = 1; j < 10; j++)
    {
        if (adc_values[j] < min_value)
        {
            min_value = adc_values[j];
        }
        if (adc_values[j] > max_value)
        {
            max_value = adc_values[j];
        }
    }

    for (j = 0; j < 10; j++)
    {
        if (adc_values[j] != min_value && adc_values[j] != max_value)
        {
            sum += adc_values[j];
            count++;
        }
    }

    adcValue = sum / count;
    vol = adcValue * 3300 / 4095;

    // ʵ�ʵ���ֵ����λ��A��20mR���裬�ɼ�����adcֵ���Ŵ���50����
    current = vol * 1000/ 50 / 20 ;
    return current;
}

/**
 @brief ADC��ȡ
 @return һ��������⣨�߱꣩����λ��mv
*/
uint16_t KEY_ADC_Read(void)
{
    uint16_t real_voltage = 0;
    uint16_t adc_values[10] = {0};
    uint16_t min_value = adc_values[0];
    uint16_t max_value = adc_values[0];
    uint16_t adcValue = 0;
    uint16_t vol = 0;
    uint16_t sum = 0;
    int count = 0;
    int i = 0;
    int j = 0;

    // ����ADC 10��
    for (i = 0; i < 10; i++)
    {
		// ����ADCͨ��ת��˳�򣬲���ʱ��Ϊ55.5��ʱ������
        adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_12, ADC_SAMPLETIME_55POINT5);
        
        // ����û�в����ⲿ����������ʹ���������ADCת��
        adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);				
				
        while (!adc_flag_get(ADC0, ADC_FLAG_EOC));                      // �ȴ��������
        adc_flag_clear(ADC0, ADC_FLAG_EOC);                             // ���������־
        adc_values[i] = adc_regular_data_read(ADC0);                    // ��ȡADC����
    }

    // �������������ȥ��������Сֵ
    for (j = 1; j < 10; j++)
    {
        if (adc_values[j] < min_value)
        {
            min_value = adc_values[j];
        }
        if (adc_values[j] > max_value)
        {
            max_value = adc_values[j];
        }
    }

    for (j = 0; j < 10; j++)
    {
        if (adc_values[j] != min_value && adc_values[j] != max_value)
        {
            sum += adc_values[j];
            count++;
        }
    }

    adcValue = sum / count;
//    adcValue = max_value;

    vol = adcValue * 3300 / 4095;                                   // ת���ɵ�ѹֵ����λmv
    real_voltage = vol ;// 2 * vol;     // ʵ�ʵ�ѹֵ
    return real_voltage;
}


