#include "common.h"
#include "system.h"
#include "timer.h"
#include "watchdog.h"
#include "delay.h"
#include "gsmUart.h"

// UART��ʼ��
void GSM_UART_HwInit(uint32 baud);

// ���ʹ���
void GSM_UART_TxProcess(GSM_UART_CB* pCB);

// UART1�����뷢���жϴ��������ڲ�ͨѶ
void UART3_IRQHandler(void);

// ����ICCID����
void GSM_UART_RcvSimICCID(uint8 data);

void GSM_UART_GpioOnOffControl(uint32 param);

// ȫ�ֱ�������
GSM_UART_CB gsmUartCB;
//===========================================================================================


// ��������������������������������������������������������������������������������������
// UART��ʼ��
void GSM_UART_Init(void)
{
	GSM_UART_HwInit(GSM_UART_BAUD_RATE_CONTROL);
}

// UART��ʼ��
void GSM_UART_HwInit(uint32 baud)
{
	// ��ʱ��
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
	
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_11);

	// UARTʱ������
	rcu_periph_clock_enable(RCU_UART3);
	usart_deinit(GSM_UART_TYPE_DEF);									// ��λ����
		
	// ��������
	usart_baudrate_set(GSM_UART_TYPE_DEF, baud);						// ������
	usart_word_length_set(GSM_UART_TYPE_DEF, USART_WL_8BIT);			// 8λ����λ
	usart_stop_bit_set(GSM_UART_TYPE_DEF, USART_STB_1BIT); 				// һ��ֹͣλ
	usart_parity_config(GSM_UART_TYPE_DEF, USART_PM_NONE); 				// ����żУ��
	usart_hardware_flow_rts_config(GSM_UART_TYPE_DEF, USART_RTS_DISABLE); // ��Ӳ������������
	usart_hardware_flow_cts_config(GSM_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(GSM_UART_TYPE_DEF, USART_TRANSMIT_ENABLE);	// ʹ�ܷ���
	usart_receive_config(GSM_UART_TYPE_DEF, USART_RECEIVE_ENABLE); 		// ʹ�ܽ���

	// �ж�����
	nvic_irq_enable(GSM_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(GSM_UART_TYPE_DEF, USART_INT_RBNE); 			// �����ж�

	usart_enable(GSM_UART_TYPE_DEF);									// ʹ�ܴ��� 

//====================================================================================================================
	// ��ʱ��
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);
	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_9);

	// DTR����
	GSM_DTR_LOW();

	// gsmģ���ϵ�Ĭ�Ϲػ�
	GSM_ONOFF_HIGH();
}

// ��������һ���ֽ�����
void GSM_UART_BC_SendData(uint8 data)
{	
	// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�) 
	usart_flag_get(GSM_UART_TYPE_DEF, USART_FLAG_TC);		

	// �������
	usart_data_transmit(GSM_UART_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while(usart_flag_get(GSM_UART_TYPE_DEF, USART_FLAG_TC) != SET); 		
}

/**************************************************************
 * @brief  void USART1_IRQHandler(void);
 * @input  ����1���պ���������1���жϷ��ͺ���
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
#if 0
void UART3_IRQHandler(void)
{
	// �ж�DR�Ƿ������ݣ��жϽ���
	if(usart_interrupt_flag_get(GSM_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET) 
	{
		uint8 rxData;
		
		// ��������
		rxData = (uint8)usart_data_receive(GSM_UART_TYPE_DEF);

		GSM_UART_RcvSimICCID(rxData);
	}

	// Other USART1 interrupts handler can go here ...				 
	if (usart_interrupt_flag_get(GSM_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- ��������ж� 
	{
		usart_flag_get(GSM_UART_TYPE_DEF, USART_FLAG_ORERR); 		 //----------------------- �����������жϱ�־λ 
		usart_data_receive(GSM_UART_TYPE_DEF);										 //----------------------- ��ռĴ���
	}
	
}
#endif
// ���ͻ�ȡsim��ICCID
void GSM_UART_SendCmdGetICCID(uint32 param)
{
	uint8 i;
	uint8 iccidStr[] = {'A','T','+','I','C','C','I','D','\r','\n','\0'};

	// ��������ʽ��������
	for (i = 0; iccidStr[i] != '\0';)
	{
		// ������ݣ���������
		GSM_UART_BC_SendData(iccidStr[i++]);
	}
}

// ����ICCID����
void GSM_UART_RcvSimICCID(uint8 data)
{
	uint8 i;
	
	// ������������
	gsmUartCB.rx.fifoBuff[gsmUartCB.rx.index++] = data;

	// Խ���ж�
	if (gsmUartCB.rx.index >= sizeof(gsmUartCB.rx.fifoBuff))
	{
		gsmUartCB.rx.index = 0;

		return;
	}
	
	if (data == '\n')			// �յ�������־
	{
		gsmUartCB.rx.index = 0;
		
		// ��������ICCID��ȡ����
		if (strstr((const char*)gsmUartCB.rx.fifoBuff, (const char*)"+ICCID: ") != NULL)
		{
			for (i = 0; i < 20; i++)
			{
				gsmUartCB.rx.iccidBuff[i] = gsmUartCB.rx.fifoBuff[i+8];
			}
			gsmUartCB.rx.iccidBuff[i] = '\0';

			// �رն�ʱ��
			TIMER_KillTask(TIMER_ID_GSM_CONTROL);
			
			// gsmģ�鿪�ػ�����
			GSM_UART_GsmPowerOnOff(FALSE);

			// ICCID�������
			gsmUartCB.rcvIccidOK = TRUE;

			// �ǳ�ʱ����
			gsmUartCB.rcvIccidTimeOut = FALSE;
		}		
	}
}

// ��ʱ���ص����ͻ�ȡsim��ICCID����
void GSM_UART_CALLBALL_GetSimIccidRequest(uint32 param)
{
	uint8 i;
	
	// �Ƿ��Ѿ�����
	if (!gsmUartCB.power.state)
	{
		return;
	}
	
	// ���ͻ�ȡsim��ICCID
	GSM_UART_SendCmdGetICCID(TRUE);

	// ��ʵ���֪5���ڿ��Ի�ȡGSMģ���ICCID��������ʱ���ж�ʧ��
	if (++gsmUartCB.rcvIccidTimeCnt > (5500/GSM_UART_TX_QUERY_ICCID_TIME))
	{
		gsmUartCB.rcvIccidTimeCnt = 0;
		gsmUartCB.rcvIccidTimeOut = TRUE;

		// ICCID����ʧ��
		gsmUartCB.rcvIccidOK = FALSE;

		// �رն�ʱ��
		TIMER_KillTask(TIMER_ID_GSM_CONTROL);

		for (i = 0; i < 20; i++)
		{
			gsmUartCB.rx.iccidBuff[i] = '0';
		}
		gsmUartCB.rx.iccidBuff[i] = '\0';
	}
}

// gsmģ�鿪�ػ�����
void GSM_UART_GsmPowerOnOff(uint32 param)
{
	// ģ��ON/OFF�ܽ�һ������ʱ��1S���ϵĵ͵�ƽģ�鼴�ɿ��ػ�
	GSM_ONOFF_LOW();

	// ������ʱ������ʱ1�뼴��
	TIMER_AddTask(TIMER_ID_GSM_POWER_CONTROL,
					1000,
					GSM_UART_GpioOnOffControl,
					param,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
}

void GSM_UART_GpioOnOffControl(uint32 param)
{
	// �����ͷ�����
	GSM_ONOFF_HIGH();

	// �ϵ����
	gsmUartCB.power.state = (BOOL)param;
}

