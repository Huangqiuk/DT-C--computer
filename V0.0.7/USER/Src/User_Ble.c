
#include "USER_Common.h"

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define DEVICE_NAME                     "zzao"                               /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

//广播时长 10ms
#define APP_ADV_DURATION                10000                       //60s                /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(50, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */


BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
    char buffer[15] = {0};
    char addr[3] = {0};

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    //待连接广播名
    addr[0] = *((uint8_t *)(0x10000000 + 0x0a6));
    addr[1] = *((uint8_t *)(0x10000000 + 0x0a5));
    addr[2] = *((uint8_t *)(0x10000000 + 0x0a4));

    sprintf(addr, "K31W-%02x%02X%02X", addr[0], addr[1], addr[2]);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) addr,
                                          strlen(addr));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t *p_evt)
{
#define SLEEP_CMD "sleep"
    uint8_t arr[10] = {"sleep off"};
    uint8_t receive_data[200] = {0};
    static int i = 0;
    static uint8_t count = 0;
    uint16_t len = strlen((char *)arr);
    memset(receive_data, 0, 200);
    strncpy(receive_data, p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        uint32_t err_code;

        NRF_LOG_INFO("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
        //p_evt->params.rx_data.length
        //p_evt->params.rx_data.p_data[i]

        if (0 == strcmp((char *)SLEEP_CMD, (char *)receive_data))
        {
            ble_nus_data_send(&m_nus, arr, &len, m_conn_handle);
            sd_power_system_off();
        }
        else if (0 == strcmp((char *)"read", (char *)receive_data))
        {
            NRF_LOG_INFO("READNVM");
            read_NVM_record(arr, (uint8_t *)&i);
        }
        else if (0 == strcmp((char *)"up", (char *)receive_data))
        {
            NRF_LOG_INFO("UPDATANVM");
            sprintf(arr, "nvm:%d", count++);
            updata_NVM_Record(arr, strlen(arr));
        }
        else if (0 == strcmp((char *)"LED1", (char *)receive_data))
        {

//              nrf_gpio_pin_clear(16);
//              nrf_gpio_pin_clear(18);
//              nrf_gpio_pin_clear(20);
//              nrf_gpio_pin_set(16);
            nrf_gpio_pin_set(16);
            nrf_gpio_pin_set(18);
            nrf_gpio_pin_set(20);
            nrf_gpio_pin_clear(16);

        }
        else if (0 == strcmp((char *)"LED2", (char *)receive_data))
        {

//              nrf_gpio_pin_clear(16);
//              nrf_gpio_pin_clear(18);
//              nrf_gpio_pin_clear(20);
//              nrf_gpio_pin_set(18);
            nrf_gpio_pin_set(16);
            nrf_gpio_pin_set(18);
            nrf_gpio_pin_set(20);
            nrf_gpio_pin_clear(18);

        }
        else if (0 == strcmp((char *)"LED3", (char *)receive_data))
        {

//              nrf_gpio_pin_clear(16);
//              nrf_gpio_pin_clear(18);
//              nrf_gpio_pin_clear(20);
//              nrf_gpio_pin_set(20);
            nrf_gpio_pin_set(16);
            nrf_gpio_pin_set(18);
            nrf_gpio_pin_set(20);
            nrf_gpio_pin_clear(20);

        }
        else if (receive_data[0] == 0x55 &&
                 receive_data[1] == 0xA2 &&
                 receive_data[2] == 0x01)
        {

            TIMER_KillTask(TIMER_ID_10SEC_TIMEOUT);
        }
        else
        {
            ble_nus_data_send(&m_nus, (uint8_t *)p_evt->params.rx_data.p_data, &p_evt->params.rx_data.length, m_conn_handle);
        }
    }
}


/**@snippet [Handling the data received over BLE] */
/**@brief Function for initializing services that will be used by the application.
 */
void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();

    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
    case BLE_ADV_EVT_FAST:
        //err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        //APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_IDLE:
        //广播超时
        //sleep_mode_enter();
        break;
    default:
        break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        NRF_LOG_INFO("Connected");
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
        APP_ERROR_CHECK(err_code);
        STATE_EnterState(STATE_CONNECT);//进入连接状态
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        NRF_LOG_INFO("Disconnected");
        // LED indication will be changed when advertising starts.
        m_conn_handle = BLE_CONN_HANDLE_INVALID;
        STATE_EnterState(STATE_BROADCAST);//进入广播状态
        break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
        NRF_LOG_DEBUG("PHY update request.");
        ble_gap_phys_t const phys =
        {
            .rx_phys = BLE_GAP_PHY_AUTO,
            .tx_phys = BLE_GAP_PHY_AUTO,
        };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
    }
    break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        // Pairing not supported
        err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        // No system attributes have been stored.
        err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);

        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    default:
        // No implementation needed.
        break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
void user_ble_stack_init(void)
{

    //定义地址结构体变量my_addr
    static ble_gap_addr_t  my_addr;
    uint8_t addr_mac[6] = {0};//读取 全球唯一MAC

    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
    //------------------BLE-初始化配置--------------------

    gap_params_init();
    gatt_init();
    services_init();

    user_advertising_init();

    //设置功率为+4dbm
    err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_advertising.adv_handle, 4);
    APP_ERROR_CHECK(err_code);

    //连接参数初始化
    conn_params_init();


    /*---------------------------设置设备地址:随机静态地址-----------------------*/
    //初始化地址和地址类型
    my_addr.addr[0] = *((uint8_t *)(0x10000000 + 0x0a9)) | 0x40 | 0x80;
    my_addr.addr[1] = *((uint8_t *)(0x10000000 + 0x0a8));
    my_addr.addr[2] = *((uint8_t *)(0x10000000 + 0x0a7));
    my_addr.addr[3] = *((uint8_t *)(0x10000000 + 0x0a6));
    my_addr.addr[4] = *((uint8_t *)(0x10000000 + 0x0a5));
    my_addr.addr[5] = *((uint8_t *)(0x10000000 + 0x0a4)); //注意地址最高两个位必须为1，其它所有的位不能同时为0，也不能同时为1
    my_addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;//地址类型设置为随机静态地址
    //写入地址
    err_code = sd_ble_gap_addr_set(&my_addr);//
    if (err_code != NRF_SUCCESS)
    {
        //打印设置地址失败
        NRF_LOG_INFO("Set Address Failed!");
    }
    /*---------------------------------END---------------------------------*/
    /*------------------------读出设备地址和地址类型------------------------*/
    err_code = sd_ble_gap_addr_get(&my_addr);
    //地址读取成功，串口打印出地址
    if (err_code == NRF_SUCCESS)
    {
        //打印地址类型
        NRF_LOG_INFO("Address Type: %02X\r\n", my_addr.addr_type);
        //打印地址
        NRF_LOG_INFO("Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                     my_addr.addr[0], my_addr.addr[1],
                     my_addr.addr[2], my_addr.addr[3],
                     my_addr.addr[4], my_addr.addr[5]);

    }
    //广播名设置
    //待绑定模式 PK31W-MAC后6个字符
    //待lian
    /*---------------------------------END---------------------------------*/
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    return;
    switch (event)
    {
    case BSP_EVENT_SLEEP:
        sleep_mode_enter();
        break;

    case BSP_EVENT_DISCONNECT:
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;

    case BSP_EVENT_WHITELIST_OFF:
        if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
        {
            err_code = ble_advertising_restart_without_whitelist(&m_advertising);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
        }
        break;

    default:
        break;
    }
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t *p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
    case APP_UART_DATA_READY:
        UNUSED_VARIABLE(app_uart_get(&data_array[index]));
        index++;

        if ((data_array[index - 1] == '\n') ||
                (data_array[index - 1] == '\r') ||
                (index >= m_ble_nus_max_data_len))
        {
            if (index > 1)
            {
                NRF_LOG_DEBUG("Ready to send data over BLE NUS");
                NRF_LOG_HEXDUMP_DEBUG(data_array, index);

                do
                {
                    uint16_t length = (uint16_t)index;
                    err_code = ble_nus_data_send(&m_nus, data_array, &length, m_conn_handle);
                    if ((err_code != NRF_ERROR_INVALID_STATE) &&
                            (err_code != NRF_ERROR_RESOURCES) &&
                            (err_code != NRF_ERROR_NOT_FOUND))
                    {
                        APP_ERROR_CHECK(err_code);
                    }
                }
                while (err_code == NRF_ERROR_RESOURCES);
            }

            index = 0;
        }
        break;

    case APP_UART_COMMUNICATION_ERROR:
        APP_ERROR_HANDLER(p_event->data.error_communication);
        break;

    case APP_UART_FIFO_ERROR:
        APP_ERROR_HANDLER(p_event->data.error_code);
        break;

    default:
        break;
    }
}
/**@snippet [Handling the data received over UART] */


/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
#if defined (UART_PRESENT)
        .baud_rate    = NRF_UART_BAUDRATE_115200
#else
        .baud_rate    = NRF_UARTE_BAUDRATE_115200
#endif
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}
/**@snippet [UART Initialization] */


/**@brief Function for initializing the Advertising functionality.
 */
void user_advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

//  init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
//  init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.config.ble_adv_on_disconnect_disabled  = true;//断开不自动广播
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}
/**@brief Function for starting advertising.
 */
void user_advertising_start(void)
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}
//-------------------------------------------
//===========================================
/*
    发送数据
*/
//发送数据
//返回值 - 0：成功   非零：错误码
unsigned char Ble_Send_Data(char *data, unsigned short len)
{

    uint32_t ret_err;

    if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return 1;
    }
    else
    {

        //发送给主机
        ret_err = ble_nus_data_send(&m_nus, (uint8_t *)data, &len, m_conn_handle);

        if (NRF_SUCCESS == ret_err)
        {
            return NRF_SUCCESS;
        }
        else
        {
            return ret_err;
        }

    }
}
//--------------改变广播名字------------------
//设置广播名字并开始广播
//param: 1配对模式 0普通模式
void advertising_start_setName(uint32_t param)
{

    uint32_t                err_code;
    ble_gap_conn_sec_mode_t sec_mode;
    char adv_name[15] = {0};
    uint8_t MAC[3] = {0};

    //获取MAC
    MAC[0] = *((uint8_t *)(0x10000000 + 0x0a6));
    MAC[1] = *((uint8_t *)(0x10000000 + 0x0a5));
    MAC[2] = *((uint8_t *)(0x10000000 + 0x0a4));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    //待绑定模式：PK31W-MAC地址后6个字符
    //待连接状态：K31W-MAC地址后6个字符
    if (param) //待绑定
    {
        sprintf(adv_name, "PK31W-%x%x%x", MAC[0], MAC[1], MAC[2]);
    }
    else //待连接
    {
        sprintf(adv_name, "K31W-%x%x%x", MAC[0], MAC[1], MAC[2]);
    }

    //停止广播 - 判断是否需要停止广播

    err_code = sd_ble_gap_adv_stop(m_advertising.adv_handle);
    APP_ERROR_CHECK(err_code);
    //改变广播名
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) adv_name,
                                          strlen(adv_name));
    APP_ERROR_CHECK(err_code);

    user_advertising_init();//想改变广播名字，必须加这个，初始化广播数据
    //开始广播
    user_advertising_start();

    NRF_LOG_INFO("adv name update...");

}
