
#ifndef _USAR_BLE_
#define _USAR_BLE_

extern unsigned short  m_conn_handle;

//====================外部调用==============
//发送数据
unsigned char Ble_Send_Data(char* data, unsigned short len);
//初始化协议栈
void user_ble_stack_init(void);
//开始广播
void user_advertising_start(void);

//设置广播名字并开始广播
//1:配对模式 0:普通模式
void advertising_start_setName(unsigned int param);
//--------------------------------------------
//初始化广播数据
void user_advertising_init(void);
//连接参数初始化
void conn_params_init();
//gap参数初始化
void gap_params_init(void);
//gatt参数初始化
void gatt_init();

//服务初始化
void services_init();



#endif
