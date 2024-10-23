#include "common.h"
#include "delay.h"
#include "system.h"
#include "timer.h"
#include "BleUart.h"
#include "Param.h"
#include "BleProtocol.h"
#include "stringOperation.h"

/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/
// UART��ʼ��
void BLE_UART_HwInit(uint32 baud);

// ���ݽṹ��ʼ��
void BLE_UART_DataStructureInit(BLE_UART_CB* pCB);

// ���ʹ���
void BLE_UART_TxProcess(BLE_UART_CB* pCB);

// �����ж��ֽڷ���
void BLE_UART_IR_StartSendData(uint8 data);

// ��������һ���ֽ�����
//void BLE_UART_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void USART3_IRQHandler(void);

// BLEģ�鸨����������
void BLE_CMD_GPIO_CtrlPin(void);

// ȫ�ֱ�������
BLE_UART_CB bleUartCB;

// UART��ʼ��
void BLE_UART_HwInit(uint32 baud)
{
	GPIO_InitType  GPIO_InitStructure;
	USART_InitType USART_InitStructure;
	NVIC_InitType  NVIC_InitStructure;	
		
	// ʱ������ 
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);		// ʹ��GPIOʱ��

	// Initialize GPIO_InitStructure
	GPIO_InitStruct(&GPIO_InitStructure);

	// Configure USART Tx as alternate function push-pull
	GPIO_InitStructure.Pin            = GPIO_PIN_2;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART2;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	// Configure USART Rx as alternate function push-pull and pull-up
	GPIO_InitStructure.Pin            = GPIO_PIN_3;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
	GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Input;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART2;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);// ��������

	// UARTʱ��ʹ��
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART2, ENABLE);		// ʹ��USARTʱ��
	
	RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_USART2, ENABLE);		// ��λ����
	RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_USART2, DISABLE); 	// ֹͣ��λ

	// ��������
	USART_InitStructure.BaudRate = baud;							// һ������Ϊ9600;
	USART_InitStructure.WordLength = USART_WL_8B; 					// 8λ���ݳ���
	USART_InitStructure.StopBits = USART_STPB_1;					// һ��ֹͣλ
	USART_InitStructure.Parity = USART_PE_NO; 						// ����żУ��λ
	USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE; 	// ��Ӳ������������
	USART_InitStructure.Mode = USART_MODE_RX | USART_MODE_TX; 		// �շ�ģʽ
	USART_Init(BLE_UART_TYPE_DEF, &USART_InitStructure);			// ��ʼ������

	// �ж�����
	NVIC_InitStructure.NVIC_IRQChannel = BLE_UART_IRQn_DEF;		// ʹ�ܴ����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	// ��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			// �����ȼ�1��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			// ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); 							// ����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
 
	USART_ConfigInt(BLE_UART_TYPE_DEF, USART_INT_RXDNE, ENABLE);	// �����ж�
 
	USART_Enable(BLE_UART_TYPE_DEF, ENABLE);						// ʹ�ܴ���
	
}

// ���ݽṹ��ʼ��
void BLE_UART_DataStructureInit(BLE_UART_CB* pCB)
{
	uint16 i;
	
	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}

	pCB->tx.txBusy = FALSE;
	pCB->tx.index = 0;
	pCB->tx.head = 0;
	pCB->tx.end = 0;
	for(i = 0; i < BLE_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}	

	bleUartCB.rcvBleMacOK = FALSE;
}

// ���ʹ���
void BLE_UART_TxProcess(BLE_UART_CB* pCB)
{
// �жϷ�ʽ
#if (UART_DRIVE_TX_MODE == UART_DRIVE_INTERRUPT_TX_MODE)
	uint16 index = pCB->tx.index;							// ��ǰ�������ݵ�������
	uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length;	// ��ǰ���͵�����֡�ĳ���
	uint16 head = pCB->tx.head;								// ��������֡����ͷ������
	uint16 end = pCB->tx.end;								// ��������֡����β������

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}
	
	// ����Ϊ�գ�������
	if (head == end)
	{
		return;
	}

	// ����æ���˳�
	if (pCB->tx.txBusy)
	{
		return;
	}

	// ����ִ�е����˵�����зǿա���
	
	// ��ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
	if (index < length)
	{		
		BLE_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		// ���÷���æ״̬
		pCB->tx.txBusy = TRUE;
	}


// ������ʽʱ��������
#else
	uint16 index = pCB->tx.index;
	uint16 head = pCB->tx.head;
	uint16 end = pCB->tx.end;

	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}

	// ����Ϊ�գ�������
	if(head == end)
	{
		return;
	}
	
	// ��ǰ����֡δ������ʱ����������
	while(index < pCB->tx.cmdQueue[head].length)
	{
		// һֱ��䷢��
		BLE_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= BLE_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

#if (BLE_UART_TX_MODE == BLE_UART_INTERRUPT_TX_MODE)
// �����ж��ֽڷ���
void BLE_UART_IR_StartSendData(uint8 data)
{
	// �ȶ�SR�������DR���TC��־���
	USART_GetFlagStatus(BLE_UART_TYPE_DEF, USART_FLAG_TXC);

	// ����һ���ֽ�
	USART_SendData(BLE_UART_TYPE_DEF, data);

	// �򿪷�������ж�
	USART_ConfigInt(BLE_UART_TYPE_DEF, USART_INT_TXC, ENABLE);	
}

#else
// ��������һ���ֽ�����
void BLE_UART_BC_SendData(uint8 data)
{	
	// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�) 
	USART_ClrFlag(BLE_UART_TYPE_DEF, USART_FLAG_TXC);		

	// �������
	USART_SendData(BLE_UART_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while(USART_GetFlagStatus(BLE_UART_TYPE_DEF, USART_FLAG_TXC) != SET);		
}
#endif

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
uint8 bleRxBuff[100] = {0};
uint8 bleIndex = 0;
void BLE_UART_BleConnectState(uint32 param)
{	
	bleIndex = 0;

	if (-1 != STRING_Find(bleRxBuff, (uint8 *)"TTM:CONNECTED"))
	{
		PARAM_SetBleConnectState(TRUE);
		
		// ������Ȩ��ʱ����10s�������û�м�Ȩ�ɹ����˳���������
		TIMER_AddTask(TIMER_ID_BLE_CONNECT_VERIFICATE,
					10000,
					BLE_MODULE_Reset,
					0,
					1,
					ACTION_MODE_ADD_TO_QUEUE);

	}

	if (-1 != STRING_Find(bleRxBuff, (uint8 *)"TTM:DISCONNECT"))
	{
		PARAM_SetBleConnectState(FALSE);
	}
}

// ��������mac��ַ
void BLE_UART_AtChannelDataProcess(uint8 data)
{
	uint8 i;
	
	if (!bleUartCB.rx.startFlag)
	{
		if (data == 'T')				// ��ʼ��־
		{
			bleUartCB.rx.startFlag = TRUE;

			bleUartCB.rx.index = 0;

			bleUartCB.rx.fifoBuff[bleUartCB.rx.index++] = data;		
			
			return;
		}
	}
	else
	{
		// ������������
		bleUartCB.rx.fifoBuff[bleUartCB.rx.index++] = data;

		// Խ���ж�
		if (bleUartCB.rx.index >= sizeof(bleUartCB.rx.fifoBuff))
		{
			bleUartCB.rx.startFlag = TRUE;
			bleUartCB.rx.index = 0;

			return;
		}
		
		if (data == '\n')			// �յ�������־
		{
			bleUartCB.rx.startFlag = FALSE;
			bleUartCB.rx.index = 0;

			bleUartCB.rx.macBuff[2] = ':';
			bleUartCB.rx.macBuff[5] = ':';
			bleUartCB.rx.macBuff[8] = ':';
			bleUartCB.rx.macBuff[11] = ':';
			bleUartCB.rx.macBuff[14] = ':';
			bleUartCB.rx.macBuff[17] = '\0';

			// ���˻���ָ��
			if ((strstr((const char*)bleUartCB.rx.fifoBuff, (const char*)"TTM:MAC-?") != NULL))
			{
				return ;
			}

			// ������������mac��ַ��ȡ����
			if ((strstr((const char*)bleUartCB.rx.fifoBuff, (const char*)"TTM:MAC-") != NULL) 
			&& (bleUartCB.rx.fifoBuff[0] == 'T') && (bleUartCB.rx.fifoBuff[1] == 'T') && (bleUartCB.rx.fifoBuff[2] == 'M'))
			{
				bleUartCB.rx.macBuff[0] = bleUartCB.rx.fifoBuff[8];
				bleUartCB.rx.macBuff[1] = bleUartCB.rx.fifoBuff[9];

				bleUartCB.rx.macBuff[3] = bleUartCB.rx.fifoBuff[10];
				bleUartCB.rx.macBuff[4] = bleUartCB.rx.fifoBuff[11];

				bleUartCB.rx.macBuff[6] = bleUartCB.rx.fifoBuff[12];
				bleUartCB.rx.macBuff[7] = bleUartCB.rx.fifoBuff[13];

				bleUartCB.rx.macBuff[9] = bleUartCB.rx.fifoBuff[14];
				bleUartCB.rx.macBuff[10] = bleUartCB.rx.fifoBuff[15];

				bleUartCB.rx.macBuff[12] = bleUartCB.rx.fifoBuff[16];
				bleUartCB.rx.macBuff[13] = bleUartCB.rx.fifoBuff[17];

				bleUartCB.rx.macBuff[15] = bleUartCB.rx.fifoBuff[18];
				bleUartCB.rx.macBuff[16] = bleUartCB.rx.fifoBuff[19];

				//for (i = 0; i < 12; i++)
				//{
				//	bleUartCB.rx.macBuff[i] = bleUartCB.rx.fifoBuff[i+10];
				//}
				//bleUartCB.rx.macBuff[i] = '\0';
				
				for (i = 0; i < sizeof(bleUartCB.rx.fifoBuff); i++)
				{
					bleUartCB.rx.fifoBuff[i] = 0;
				}				

				// ע���ö�ʱ��
				TIMER_KillTask(TIMER_ID_BLE_CONTROL);
				
				// ������ַ�������
				bleUartCB.rcvBleMacOK = TRUE;
			}	
		}
	}
}


/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
void USART2_IRQHandler(void)
{
	// �ж�DR�Ƿ������ݣ��жϽ���
	if (USART_GetIntStatus(BLE_UART_TYPE_DEF, USART_INT_RXDNE) != RESET) 
	{
		uint8 rxdata = 0x00;
		
		// ��������
		rxdata = (uint8)USART_ReceiveData(BLE_UART_TYPE_DEF);
		
		if (NULL != bleUartCB.receiveDataThrowService)
		{			
			(*bleUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}

		if (!bleUartCB.rcvBleMacOK)
		{
			BLE_UART_AtChannelDataProcess(rxdata);
		}

//=================================================================
		// ���������ж�
		if ((bleIndex + 1)< sizeof(bleRxBuff)/sizeof(bleRxBuff[0]))
		{
			bleRxBuff[bleIndex++] = rxdata;
			bleRxBuff[bleIndex] = '\0';
		}

		TIMER_AddTask(TIMER_ID_BLE_CONNECT_STATE,
					100,
					BLE_UART_BleConnectState,
					0,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
	}


// �ж�ģʽ����
#if (BLE_UART_TX_MODE == BLE_UART_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if (USART_GetIntStatus(BLE_UART_TYPE_DEF, USART_INT_TXC) != RESET)
	{
		uint16 head = bleUartCB.tx.head;
		uint16 end;
		uint16 index = bleUartCB.tx.index;
		uint8 txdata = 0x00;

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if (index < bleUartCB.tx.cmdQueue[head].length)
		{
			txdata = bleUartCB.tx.cmdQueue[head].buff[bleUartCB.tx.index++];
			
			// �������
			USART_SendData(BLE_UART_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			bleUartCB.tx.cmdQueue[head].length = 0;
			bleUartCB.tx.head ++;
			bleUartCB.tx.head %= BLE_UART_DRIVE_TX_QUEUE_SIZE;
			bleUartCB.tx.index = 0;

			head = bleUartCB.tx.head;
			end = bleUartCB.tx.end;
			
			// ����֡���зǿգ�����������һ������֡
			if (head != end)
			{
				txdata = bleUartCB.tx.cmdQueue[head].buff[bleUartCB.tx.index++];

				// �������
				USART_SendData(BLE_UART_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ�������ж�
				USART_ConfigInt(BLE_UART_TYPE_DEF, USART_INT_TXC, DISABLE);
				
				bleUartCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif

	// Other USART3 interrupts handler can go here ...				 
	if (USART_GetIntStatus(BLE_UART_TYPE_DEF, USART_INT_OREF) != RESET)	//----------------------- ��������ж� 
	{
		USART_GetIntStatus(BLE_UART_TYPE_DEF, USART_INT_OREF); 			//----------------------- �����������жϱ�־λ 
		USART_ReceiveData(BLE_UART_TYPE_DEF);							//----------------------- ��ռĴ���
	}
}

// BLEģ�鸨����������
void BLE_CMD_GPIO_CtrlPin(void)
{
	GPIO_InitType GPIO_InitStructure;
	
	// Enable GPIO clock  
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE); 
	
	// GPIO����
	// Initialize GPIO_InitStructure
	GPIO_InitStruct(&GPIO_InitStructure);

	GPIO_InitStructure.Pin            = GPIO_PIN_4;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA, GPIO_PIN_4);
	
}

// UART��ʼ��
void BLE_UART_Init(void)
{
	// BLEģ�鸨���������ų�ʼ��
	BLE_CMD_GPIO_CtrlPin();

	// ���ڳ�ʼ��
	BLE_UART_HwInit(BLE_UART_BAUD_RATE);

	// UART���ݽṹ��ʼ��
	BLE_UART_DataStructureInit(&bleUartCB);
}

// UARTģ�鴦�����
void BLE_UART_Process(void)
{
	// ���ʹ���
	BLE_UART_TxProcess(&bleUartCB);
}

// ע�������׳��ӿڷ���
void BLE_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	bleUartCB.receiveDataThrowService = service;
}

// ���ͻ����������һ������������
BOOL BLE_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = bleUartCB.tx.head;
	uint16 end = bleUartCB.tx.end;
	
	// ��������
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// ���ͻ������������������
	if((end + 1) % BLE_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	bleUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		bleUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	bleUartCB.tx.cmdQueue[end].length = length;
	
	// ���ͻ��ζ��и���λ��
	bleUartCB.tx.end ++;
	bleUartCB.tx.end %= BLE_UART_DRIVE_TX_QUEUE_SIZE;
	bleUartCB.tx.cmdQueue[bleUartCB.tx.end].length = 0;

	return TRUE;
}

// BLEģ�鸴λ
void BLE_MODULE_Reset(uint32 param)
{
	// ������λ
	BLE_RST_RESET();
	Delayms(10);
	
	// �ͷŸ�λ
	BLE_RST_RELEASE();

	// ����ʼ�㲥
	BLE_EN_ENABLE();
}


