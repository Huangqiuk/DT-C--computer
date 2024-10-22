#include "main.h"

//BOOST - PA2 - 低电平有效（外接上拉）

//按键初始化
void Button_Init()
{

    GPIO_InitType GPIO_InitStructure;
    RCC_APB_Peripheral_Clock_Enable(RCC_APB_PERIPH_IOPA);
    GPIO_Structure_Initialize(&GPIO_InitStructure);

    GPIO_InitStructure.Pin          = GPIO_PIN_2;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_INPUT;
    GPIO_InitStructure.GPIO_Pull        = GPIO_NO_PULL;
    //Boost按键初始化
    GPIO_Peripheral_Initialize(GPIOA, &GPIO_InitStructure);

    //ADC按键初始化
    Adc_Init();

}

//读取键值
KEY_NUM ADC_Button_Read()
{

    KEY_NUM keyNum = BUTTON_NULL;//键值
    uint16_t adc = Get_Stable_Key_Adc();//获取ADC

    if (adc >= BUTTON_ADD_MIN && adc <= BUTTON_ADD_MAX)
    {
        keyNum = BUTTON_ADD;//加
    }
    else if (adc >= BUTTON_DEC_MIN && adc <= BUTTON_DEC_MAX)
    {
        keyNum = BUTTON_DEC;    //减
    }
    else if (adc >= BUTTON_TRUMPET_MIN && adc <= BUTTON_TRUMPET_MAX)
    {
        keyNum = BUTTON_TRUMPET;    //喇叭
    }
    else if (adc >= BUTTON_LIGHT_MIN && adc <= BUTTON_LIGHT_MAX)
    {
        keyNum = BUTTON_LIGHT;  //大灯
    }
    else if (adc >= BUTTON_ADDNFC_MIN && adc <= BUTTON_ADDNFC_MAX)
    {
        keyNum = BUTTON_ADDNFC; //加NFC - （+灯）
    }
    else if (adc >= BUTTON_DELNFC_MIN && adc <= BUTTON_DELNFC_MAX)
    {
        keyNum = BUTTON_DELNFC; //删NFC - （-灯）
    }
    else if (adc >= BUTTON_BLE_MIN && adc <= BUTTON_BLE_MAX)
    {
        keyNum = BUTTON_BLE;    //蓝牙 -  （+- ）
    }
    else
    {
        keyNum = BUTTON_NULL;
    }

    return keyNum;


}





