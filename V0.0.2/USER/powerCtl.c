#include "powerCtl.h"

// 检测结果电压
volatile uint32 realtimeVoltage;

// 引脚初始化
void Power_Init(void)
{
    GPIO_InitType GPIO_InitStructure;
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);

                                     // VOUT_SW0      VOUT_SW1      VOUT_EN
    GPIO_InitStructure.Pin            = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_12 ;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

                                     // BUCK_OFF     BOOST_OFF
    GPIO_InitStructure.Pin            = GPIO_PIN_8 | GPIO_PIN_9  ;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
		
                                      // A
    GPIO_InitStructure.Pin            = GPIO_PIN_13;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOC, &GPIO_InitStructure);

                                     // B            C            G2A          G2B          G1
    GPIO_InitStructure.Pin            = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 ;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
}



// 电源引脚ADC初始化
void ADC_Vin(void)
{
    GPIO_InitType GPIO_InitStructure;
    ADC_InitType ADC_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin            = GPIO_PIN_5;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Analog;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source

    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

    ADC_ConfigRegularChannel(ADC, ADC_CH_6, 1, ADC_SAMP_TIME_55CYCLES5);
    ADC_Enable(ADC, ENABLE);
    while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

    ADC_StartCalibration(ADC);
    while (ADC_GetCalibrationStatus(ADC));

    // 启动ADC转换
    ADC_EnableSoftwareStartConv(ADC, ENABLE);

}

// 获取电源板输入电压,单位：mv
uint16_t Get_Vin_Voltage(void)
{
    float voltage = 0.1f;
    uint16_t real_voltage = 0;
    uint16_t adc_value = 0;
    uint16_t adc_values[10] = {0};
    uint16_t min_value = adc_values[0];
    uint16_t max_value = adc_values[0];
    uint16_t sum = 0;
    int i = 0, j = 0;
    int count = 0;

    // 采样ADC 10次
    for (i = 0; i < 10; i++)
    {
        ADC_ConfigRegularChannel(ADC, ADC_CH_6, 1, ADC_SAMP_TIME_55CYCLES5);

        // 启动ADC转换
        ADC_EnableSoftwareStartConv(ADC, ENABLE);

        // 等待转换完成
        while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

        // 获取ADC值
        adc_values[i] = ADC_GetDat(ADC);
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

    // 对数组进行排序
    for (j = 0; j < 10; j++)
    {
        if (adc_values[j] != min_value && adc_values[j] != max_value)
        {
            sum += adc_values[j];
            count++;
        }
    }

    // 去掉最大和最小值并相加
    adc_value = sum / count;

    // 计算电压
    voltage = (float)adc_value / 4095 * 3300;

    // 输入12V电阻分压缩小6倍给ADC采样，故实际电压值应该为6倍
    real_voltage = 6 * voltage;
		
		// 检测结果电压
		realtimeVoltage = real_voltage;
    return real_voltage;
}


// 升压引脚ADC初始化
void ADC_Boost(void)
{
    GPIO_InitType GPIO_InitStructure;
    ADC_InitType ADC_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin            = GPIO_PIN_6;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Analog;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

    ADC_ConfigRegularChannel(ADC, ADC_CH_7, 1, ADC_SAMP_TIME_55CYCLES5);

    ADC_Enable(ADC, ENABLE);
    while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);
    ADC_StartCalibration(ADC);
    while (ADC_GetCalibrationStatus(ADC));

    // 启动ADC转换
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
}

// 获取升压后的电压
uint32_t Get_Boost_Voltage(void)
{
    float voltage = 0.1f;
    uint32_t real_voltage = 0;
    uint16_t adc_value = 0;
    uint16_t adc_values[10] = {0};
    uint16_t min_value = adc_values[0];
    uint16_t max_value = adc_values[0];
    uint16_t sum = 0;
    int i = 0, j = 0;
    int count = 0;

    // 采样ADC 10次
    for (i = 0; i < 10; i++)
    {
        ADC_ConfigRegularChannel(ADC, ADC_CH_7, 1, ADC_SAMP_TIME_55CYCLES5);

        // 启动ADC转换
        ADC_EnableSoftwareStartConv(ADC, ENABLE);

        // 等待转换完成
        while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

        // 获取ADC值
        adc_values[i] = ADC_GetDat(ADC);
    }

    // 对数组进行排序
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

    // 去掉最大和最小值并相加
    for (j = 0; j < 10; j++)
    {
        if (adc_values[j] != min_value && adc_values[j] != max_value)
        {
            sum += adc_values[j];
            count++;
        }
    }

    adc_value = sum / count;

    // 计算电压
    voltage = (float)adc_value / 4095 * 3300;

    // 电阻分压缩小37倍给ADC采样，故实际电压值应该为28倍
    real_voltage = 37 * voltage;
		
		// 检测结果电压
    realtimeVoltage = real_voltage;
    return real_voltage;
}

// 降压引脚ADC初始化
void ADC_Buck(void)
{
    GPIO_InitType GPIO_InitStructure;
    ADC_InitType ADC_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin            = GPIO_PIN_7;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Analog;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    // ADC的输入时钟不得超过64MHz
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source

    // ADC配置
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

    ADC_ConfigRegularChannel(ADC, ADC_CH_8, 1, ADC_SAMP_TIME_55CYCLES5);
    ADC_Enable(ADC, ENABLE);
    while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

    // 建议每次上电执行一次校准
    ADC_StartCalibration(ADC);
    while (ADC_GetCalibrationStatus(ADC));

    // 启动ADC转换
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
}


// 获取降压后的电压
uint16_t Get_Buck_Voltage(void)
{
    float voltage = 0.0f;
    uint16_t real_voltage = 0;
    uint16_t adc_value = 0;
    uint16_t adc_values[10] = {0};
    uint16_t min_value = adc_values[0];
    uint16_t max_value = adc_values[0];
    uint16_t sum = 0;
    int i = 0, j = 0;
    int count = 0;

    // 采样ADC 10次
    for (i = 0; i < 10; i++)
    {
        ADC_ConfigRegularChannel(ADC, ADC_CH_8, 1, ADC_SAMP_TIME_55CYCLES5);
        // 启动ADC转换
        ADC_EnableSoftwareStartConv(ADC, ENABLE);

        // 等待转换完成
        while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

        // 获取ADC值
        adc_values[i] = ADC_GetDat(ADC);
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

    adc_value = sum / count;

    // 计算电压
    voltage = (float)adc_value / 4095 * 3300;

    // 输入12V电阻分压缩小6倍给ADC采样，故实际电压值应该为6倍
    real_voltage = 6 * voltage;
		
		// 检测结果电压
    realtimeVoltage = real_voltage;
    return real_voltage;
}

// 电流引脚ADC初始化检测输出电流
void ADC_Current(void)
{
    GPIO_InitType GPIO_InitStructure;
    ADC_InitType ADC_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin            = GPIO_PIN_0;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Analog;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    // ADC的输入时钟不得超过64MHz
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source

    // ADC配置
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

    ADC_ConfigRegularChannel(ADC, ADC_CH_9, 1, ADC_SAMP_TIME_55CYCLES5);
    ADC_Enable(ADC, ENABLE);
    while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

    // 建议每次上电执行一次校准
    ADC_StartCalibration(ADC);
    while (ADC_GetCalibrationStatus(ADC));

    // 启动ADC转换
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
}


// 获取ADC值之后转化为电压值\100\20MR，通过计算得到电流值
uint16_t Get_Vout_Current(void)
{
    uint16_t current = 0;
    uint16_t adc_value = 0;
    uint16_t adc_values[10] = {0};
    float voltage = 0.0f;
    uint16_t min_value = adc_values[0];
    uint16_t max_value = adc_values[0];
    uint16_t sum = 0;
    int i = 0, j = 0;
    int count = 0;

    // 采样ADC 10次
    for (i = 0; i < 10; i++)
    {
        ADC_ConfigRegularChannel(ADC, ADC_CH_9, 1, ADC_SAMP_TIME_55CYCLES5);

        // 启动ADC转换
        ADC_EnableSoftwareStartConv(ADC, ENABLE);

        // 等待转换完成
        while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

        // 获取ADC值
        adc_values[i] = ADC_GetDat(ADC);
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

    // 计算平均值
    adc_value = sum / count;

    // 计算电压
    voltage = (float)adc_value / 4095 * 3300;

    /* 
       计算电流（单位：mA） INA168可变增益电流感应放大器放大25倍，运放放大倍数为52倍,
       换算得出R2512电阻器两端的电压，除以其自身20mR的阻值，得电流值。再减去20补偿值，
       即得与万用表串联进去电路测得的电流相差不大。  
       一直检测，若50ms检测的电流一直大于500mA则断电。
    */
    current = ((voltage * 1000)  / (25 * 52 * 20)) - 20;
		
	// 检测结果电流
    realtimeVoltage = current;
    return current;
}


// ADC引脚初始化
void Power_ADC(void)
{
    ADC_Vin();
    ADC_Boost();
    ADC_Buck();
    ADC_Current();
}
