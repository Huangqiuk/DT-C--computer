#include "AvoPin.h"
#include "system.h"
#include "timer.h"

// 测试引脚初始化
void AVO_PIN_Init(void)
{
    // 打开时钟
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOE);

    // 配置为高阻态模式
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

    // USB负载测试
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
    
    // 电子变速
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);  // DERAILLEUR_VCC_TEST_EN
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_1);  // DERAILLEUR_GND_TEST_EN
    
    // 八方电路VLK5V供电使能
    gpio_init(GPIOE, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0);  // 5VOUT_EN    
}

// 测试引脚复位
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

// 电源使能、八方高标VLK供电
void _5VOUT_EN(uint8_t level)
{
    //1.GPIO引脚时钟使能
    rcu_periph_clock_enable(RCU_GPIOE);

    //2.GPIO配置为推完输出模式
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);

    //3.引脚配置为低电平或高电平
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
    //1.GPIO引脚时钟使能
    rcu_periph_clock_enable(RCU_GPIOA);

    //3.GPIO配置为推完输出模式
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

    //4.重映射配置：JTAG-DP禁用和SW-DP使能
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    //5.引脚配置为低电平或高电平
    if (level == 1)
    {
        gpio_bit_set(GPIOA, GPIO_PIN_15);
    }
    else
    {
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
    }
}

// DAC初始化
void DAC_Init(void)
{
    rcu_periph_clock_enable(RCU_DAC);
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_5);

    // 配置DAC1
    dac_trigger_disable(DAC0);
    dac_wave_mode_config(DAC0, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC0);

    // 配置DAC2
    dac_trigger_disable(DAC1);
    dac_wave_mode_config(DAC1, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC1);
}

// DAC失能
void DAC_Uinit(void)
{
    dac_disable(DAC0);
    dac_disable(DAC1);
}

// 油门DAC输出
void DAC0_output(uint16_t value)
{
    dac_enable(DAC0); //使能 DAC0
    dac_data_set(DAC0, DAC_ALIGN_12B_R, (value / 2 * 4096 / 3300));  // 设置输出值
    dac_software_trigger_enable(DAC0);
}

// 刹车DAC输出
void DAC1_output(uint16_t value)
{
    dac_enable(DAC1); //使能 DAC0

    // 运放放大器放大两倍
    dac_data_set(DAC1, DAC_ALIGN_12B_R, (value / 2 * 4096 / 3300)); // 设置输出值
    dac_software_trigger_enable(DAC1);
}

// 简单延时
void Delayus(uint16 i)
{
    // 内核72MHz，1us
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

// 简单延时
void Delayms(uint16 i)
{
    uint16 x;
    uint16 y;

    // 内核72MHz，1ms
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

// USB硬件ADC初始化
void ADC_HwInit(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);                                 // 时能GPIOC时钟
    rcu_periph_clock_enable(RCU_ADC0);                                  // 时能ADC0时钟

    adc_deinit(ADC0);                                                   // 复位ADC
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);                        // 72/6 = 12 Mhz(最大频率不能超过40M)

    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);     

    // ADC0 配置
    adc_mode_config(ADC_MODE_FREE);                                     // 独立模式
    adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);          // 扫描模式关闭
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);    // 连续转换模式关闭
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);  // 常规通道，软件触发
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);               // 数据右对齐
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);            // 常规通道数量1

    // 设置ADC
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_11, ADC_SAMPLETIME_55POINT5);
//    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_12, ADC_SAMPLETIME_55POINT5);

    adc_enable(ADC0);                                                   // 时能ADC0
    Delayms(10);
    adc_calibration_enable(ADC0);                                       // ADC校准和复位校准

    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);     // 启动ADC0转换
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
}

/**
 @brief ADC读取
 @return USB输入电压，单位：mv
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

    // 采样ADC 10次
    for (i = 0; i < 10; i++)
    {
		
		// 配置ADC通道转换顺序，采样时间为55.5个时钟周期
        adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_55POINT5);
        
        // 由于没有采用外部触发，所以使用软件触发ADC转换
        adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);				
				
        while (!adc_flag_get(ADC0, ADC_FLAG_EOC));                      // 等待采样完成
        adc_flag_clear(ADC0, ADC_FLAG_EOC);                             // 清除结束标志
        adc_values[i] = adc_regular_data_read(ADC0);                    // 读取ADC数据
				

    }

    // 对数组进行排序并去掉最大和最小值
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

    vol = adcValue * 3300 / 4095;                                   // 转换成电压值，单位mv
    real_voltage = 2 * vol;     // 实际电压值
    return real_voltage;
}

/**
 * @brief ADC读取
 * @return USB负载电流（毫安）
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

    // 采样ADC 10次
    for (i = 0; i < 10; i++)
    {
        // 配置ADC通道转换顺序，采样时间为55.5个时钟周期
        adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_11, ADC_SAMPLETIME_55POINT5);

        // 由于没有采用外部触发，所以使用软件触发ADC转换
        adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
        while (!adc_flag_get(ADC0, ADC_FLAG_EOC));                      // 等待采样完成
        adc_flag_clear(ADC0, ADC_FLAG_EOC);                             // 清除结束标志
        adc_values[i] = adc_regular_data_read(ADC0);                         // 读取ADC数据
    }

    // 对数组进行排序并去掉最大和最小值
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

    // 实际电流值，单位：A（20mR电阻，采集到的adc值被放大了50倍）
    current = vol * 1000/ 50 / 20 ;
    return current;
}

/**
 @brief ADC读取
 @return 一键开机检测（高标），单位：mv
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

    // 采样ADC 10次
    for (i = 0; i < 10; i++)
    {
		// 配置ADC通道转换顺序，采样时间为55.5个时钟周期
        adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_12, ADC_SAMPLETIME_55POINT5);
        
        // 由于没有采用外部触发，所以使用软件触发ADC转换
        adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);				
				
        while (!adc_flag_get(ADC0, ADC_FLAG_EOC));                      // 等待采样完成
        adc_flag_clear(ADC0, ADC_FLAG_EOC);                             // 清除结束标志
        adc_values[i] = adc_regular_data_read(ADC0);                    // 读取ADC数据
    }

    // 对数组进行排序并去掉最大和最小值
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

    vol = adcValue * 3300 / 4095;                                   // 转换成电压值，单位mv
    real_voltage = vol ;// 2 * vol;     // 实际电压值
    return real_voltage;
}


