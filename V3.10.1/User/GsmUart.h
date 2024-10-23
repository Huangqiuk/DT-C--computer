#ifndef __GSM_UART_H__
#define __GSM_UART_H__

#define GSM_UART_TX_QUERY_ICCID_TIME		200		// ���Ͳ�ѯICCIDʱ����

#define GSM_UART_TYPE_DEF					UART3					 // ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)
#define GSM_UART_IRQn_DEF					UART3_IRQn               // ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)

#define GSM_UART_RX_TIME_OUT_CHECK_ENABLE	1		// ͨѶ��ʱ��⹦�ܿ���:0��ֹ��1ʹ��
#define GSM_UART_BAUD_RATE_CONTROL			115200	// ����������������֮��ͨѶ������

#define GSM_DTR_HIGH()						gpio_bit_write(GPIOB, GPIO_PIN_8, SET)
#define GSM_DTR_LOW()						gpio_bit_write(GPIOB, GPIO_PIN_8, RESET)
#define GSM_ONOFF_HIGH()					gpio_bit_write(GPIOB, GPIO_PIN_9, RESET)	// ����NPN�����ܣ������Ҫ������
#define GSM_ONOFF_LOW()						gpio_bit_write(GPIOB, GPIO_PIN_9, SET)


// UART���ƽṹ�嶨��
typedef struct
{
	// ����֡���������ݽṹ
	struct{	
		uint8 fifoBuff[50];
		uint8 iccidBuff[21];
		uint8 index;
	}rx;

	struct{	
		BOOL state;
	}power;

	BOOL rcvIccidOK;
	BOOL rcvIccidTimeOut;

	uint8 rcvIccidTimeCnt;

}GSM_UART_CB;

extern GSM_UART_CB gsmUartCB;		


//==============================================================================================================
/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/
// UART��ʼ������main�Ĵ�ѭ��֮ǰ���ã����ģ���ʼ��
void GSM_UART_Init(void);

// UARTģ�鴦����ڣ���main�Ĵ�ѭ���е���
void GSM_UART_Process(void);

// ���ͻ�ȡsim��ICCID
void GSM_UART_SendCmdGetICCID(uint32 param);

// ��ʱ���ص����ͻ�ȡsim��ICCID����
void GSM_UART_CALLBALL_GetSimIccidRequest(uint32 param);

// gsmģ�鿪�ػ�����
void GSM_UART_GsmPowerOnOff(uint32 param);


#endif



