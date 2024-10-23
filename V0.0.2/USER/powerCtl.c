#include "powerCtl.h"

// �������ѹ
volatile uint32 realtimeVoltage;

// ���ų�ʼ��
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



// ��Դ����ADC��ʼ��
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

    // ����ADCת��
    ADC_EnableSoftwareStartConv(ADC, ENABLE);

}

// ��ȡ��Դ�������ѹ,��λ��mv
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

    // ����ADC 10��
    for (i = 0; i < 10; i++)
    {
        ADC_ConfigRegularChannel(ADC, ADC_CH_6, 1, ADC_SAMP_TIME_55CYCLES5);

        // ����ADCת��
        ADC_EnableSoftwareStartConv(ADC, ENABLE);

        // �ȴ�ת�����
        while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

        // ��ȡADCֵ
        adc_values[i] = ADC_GetDat(ADC);
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

    // �������������
    for (j = 0; j < 10; j++)
    {
        if (adc_values[j] != min_value && adc_values[j] != max_value)
        {
            sum += adc_values[j];
            count++;
        }
    }

    // ȥ��������Сֵ�����
    adc_value = sum / count;

    // �����ѹ
    voltage = (float)adc_value / 4095 * 3300;

    // ����12V�����ѹ��С6����ADC��������ʵ�ʵ�ѹֵӦ��Ϊ6��
    real_voltage = 6 * voltage;
		
		// �������ѹ
		realtimeVoltage = real_voltage;
    return real_voltage;
}


// ��ѹ����ADC��ʼ��
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

    // ����ADCת��
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
}

// ��ȡ��ѹ��ĵ�ѹ
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

    // ����ADC 10��
    for (i = 0; i < 10; i++)
    {
        ADC_ConfigRegularChannel(ADC, ADC_CH_7, 1, ADC_SAMP_TIME_55CYCLES5);

        // ����ADCת��
        ADC_EnableSoftwareStartConv(ADC, ENABLE);

        // �ȴ�ת�����
        while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

        // ��ȡADCֵ
        adc_values[i] = ADC_GetDat(ADC);
    }

    // �������������
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

    // ȥ��������Сֵ�����
    for (j = 0; j < 10; j++)
    {
        if (adc_values[j] != min_value && adc_values[j] != max_value)
        {
            sum += adc_values[j];
            count++;
        }
    }

    adc_value = sum / count;

    // �����ѹ
    voltage = (float)adc_value / 4095 * 3300;

    // �����ѹ��С37����ADC��������ʵ�ʵ�ѹֵӦ��Ϊ28��
    real_voltage = 37 * voltage;
		
		// �������ѹ
    realtimeVoltage = real_voltage;
    return real_voltage;
}

// ��ѹ����ADC��ʼ��
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

    // ADC������ʱ�Ӳ��ó���64MHz
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source

    // ADC����
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

    ADC_ConfigRegularChannel(ADC, ADC_CH_8, 1, ADC_SAMP_TIME_55CYCLES5);
    ADC_Enable(ADC, ENABLE);
    while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

    // ����ÿ���ϵ�ִ��һ��У׼
    ADC_StartCalibration(ADC);
    while (ADC_GetCalibrationStatus(ADC));

    // ����ADCת��
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
}


// ��ȡ��ѹ��ĵ�ѹ
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

    // ����ADC 10��
    for (i = 0; i < 10; i++)
    {
        ADC_ConfigRegularChannel(ADC, ADC_CH_8, 1, ADC_SAMP_TIME_55CYCLES5);
        // ����ADCת��
        ADC_EnableSoftwareStartConv(ADC, ENABLE);

        // �ȴ�ת�����
        while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

        // ��ȡADCֵ
        adc_values[i] = ADC_GetDat(ADC);
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

    adc_value = sum / count;

    // �����ѹ
    voltage = (float)adc_value / 4095 * 3300;

    // ����12V�����ѹ��С6����ADC��������ʵ�ʵ�ѹֵӦ��Ϊ6��
    real_voltage = 6 * voltage;
		
		// �������ѹ
    realtimeVoltage = real_voltage;
    return real_voltage;
}

// ��������ADC��ʼ������������
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

    // ADC������ʱ�Ӳ��ó���64MHz
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source

    // ADC����
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

    ADC_ConfigRegularChannel(ADC, ADC_CH_9, 1, ADC_SAMP_TIME_55CYCLES5);
    ADC_Enable(ADC, ENABLE);
    while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

    // ����ÿ���ϵ�ִ��һ��У׼
    ADC_StartCalibration(ADC);
    while (ADC_GetCalibrationStatus(ADC));

    // ����ADCת��
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
}


// ��ȡADCֵ֮��ת��Ϊ��ѹֵ\100\20MR��ͨ������õ�����ֵ
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

    // ����ADC 10��
    for (i = 0; i < 10; i++)
    {
        ADC_ConfigRegularChannel(ADC, ADC_CH_9, 1, ADC_SAMP_TIME_55CYCLES5);

        // ����ADCת��
        ADC_EnableSoftwareStartConv(ADC, ENABLE);

        // �ȴ�ת�����
        while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);

        // ��ȡADCֵ
        adc_values[i] = ADC_GetDat(ADC);
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

    // ����ƽ��ֵ
    adc_value = sum / count;

    // �����ѹ
    voltage = (float)adc_value / 4095 * 3300;

    /* 
       �����������λ��mA�� INA168�ɱ����������Ӧ�Ŵ����Ŵ�25�����˷ŷŴ���Ϊ52��,
       ����ó�R2512���������˵ĵ�ѹ������������20mR����ֵ���õ���ֵ���ټ�ȥ20����ֵ��
       ���������ñ�����ȥ��·��õĵ�������  
       һֱ��⣬��50ms���ĵ���һֱ����500mA��ϵ硣
    */
    current = ((voltage * 1000)  / (25 * 52 * 20)) - 20;
		
	// ���������
    realtimeVoltage = current;
    return current;
}


// ADC���ų�ʼ��
void Power_ADC(void)
{
    ADC_Vin();
    ADC_Boost();
    ADC_Buck();
    ADC_Current();
}
