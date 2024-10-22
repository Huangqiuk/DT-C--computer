#include "USER_Common.h"

void in_pin_Handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) { 
	
	//nrf_gpio_pin_toggle(USER_LED1);
	
}

void KEY_GPIO_Init(){

	ret_code_t err_code; 
	err_code = nrf_drv_gpiote_init();//初始化GPIOTE APP_ERR	
	APP_ERROR_CHECK(err_code);
	
	//设置输出初始化
	nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(1);
	err_code = nrf_drv_gpiote_out_init(USER_LED1, &out_config);
	
	//设置输入初始化
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);//参数为是否配置为高精度时钟
	in_config.pull = NRF_GPIO_PIN_PULLUP;
	//设置GPIOTE输入,极性,模式
	err_code = nrf_drv_gpiote_in_init(KEY_12 , &in_config, in_pin_Handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_gpiote_in_init(KEY_14 , &in_config, in_pin_Handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_gpiote_in_init(KEY_15 , &in_config, in_pin_Handler);
	APP_ERROR_CHECK(err_code);
	
	//使能GPIOTE
	nrf_drv_gpiote_in_event_enable(KEY_12, true);
	nrf_drv_gpiote_in_event_enable(KEY_14, true);
	nrf_drv_gpiote_in_event_enable(KEY_15, true);
	
}
