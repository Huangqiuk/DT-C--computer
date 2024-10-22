#ifndef _USER_COMMON_H_
#define _USER_COMMON_H_

/*
#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef uint16_t
#define uint16_t unsigned short int
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif
*/

#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_gpiote.h"
#include "nrf_saadc.h"
#include "nrf_delay.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#include "fds.h"
#include "stdint.h"
#include "User_Ble.h"
#include "User_ADC.h"
#include "User_Button.h"
#include "User_LowPower.h"
#include "User_NVM.h"
#include "User_LED.h"
#include "User_keyMsgProcess.h"
#include "User_key.h"
#include "User_State.h"
#include "User_Timer.h"
#endif
