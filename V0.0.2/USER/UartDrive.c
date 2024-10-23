#include "uartDrive.h"
#include "param.h"
/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void UART_DRIVE_DataStructInit(UART_DRIVE_CB* pCB);

// UART��ʼ��
void UART_DRIVE_HwInit(uint32 baud);

// ���ʹ���
void UART_DRIVE_TxProcess(UART_DRIVE_CB* pCB);

// �����ж��ֽڷ���
void UART_DRIVE_IR_StartSendData(uint8 data);

// ��������һ���ֽ�����
void UART_DRIVE_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void USART3_IRQHandler(void);


// ȫ�ֱ�������
UART_DRIVE_CB uartDriveCB;

// ��������������������������������������������������������������������������������������
// UART��ʼ��
void UART_DRIVE_Init(uint32 baud)
{	
	// Ӳ��UART����
	UART_DRIVE_HwInit(baud);

	// UART���ݽṹ��ʼ��
	UART_DRIVE_DataStructInit(&uartDriveCB);
}

// UARTģ�鴦�����
void UART_DRIVE_Process(void)
{
	// �ڲ�ͨѶ�ӿڵķ��ʹ���
	UART_DRIVE_TxProcess(&uartDriveCB);
}

// ��Э���ע�����ݽ��սӿ�
void UART_DRIVE_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	uartDriveCB.receiveDataThrowService = service;
}

// ���ݽṹ��ʼ��
void UART_DRIVE_DataStructInit(UART_DRIVE_CB* pCB)
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
	for(i = 0; i < UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}
}

// UART��ʼ��
void UART_DRIVE_HwInit(uint32 baud)
{
		GPIO_InitType GPIO_InitStructure;
		USART_InitType USART_InitStructure;
		NVIC_InitType NVIC_InitStructure;
	
		RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA|RCC_APB2_PERIPH_GPIOA, ENABLE);    
		RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_USART1, ENABLE);                       //                                      
	

    // Initialize GPIO_InitStructure
    GPIO_InitStruct(&GPIO_InitStructure);
    // USART Tx 
    GPIO_InitStructure.Pin            = GPIO_PIN_9;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART1;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    // USART Rx 
    GPIO_InitStructure.Pin            = GPIO_PIN_10;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Input;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART1;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    // USART configuration
    USART_InitStructure.BaudRate            = baud;
    USART_InitStructure.WordLength          = USART_WL_8B;
    USART_InitStructure.StopBits            = USART_STPB_1;
    USART_InitStructure.Parity              = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;
    USART_Init(UART_DRIVE_TYPE_DEF, &USART_InitStructure);

    // Enable the USART Interrupt
    NVIC_InitStructure.NVIC_IRQChannel            = UART_DRIVE_IRQn_DEF;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable USART Receive and Transmit interrupts   
    USART_ConfigInt(UART_DRIVE_TYPE_DEF, USART_INT_RXDNE, ENABLE);    
		
    // USART_ConfigInt(UART_DRIVE_TYPE_DEF, USART_INT_TXC, ENABLE);    
    // Enable the USART
    USART_Enable(UART_DRIVE_TYPE_DEF, ENABLE);
}

/*********************************************************
������: void UART0_IRQHandler(void)
��  ��: UART�жϷ������
����ֵ: ��
���ֵ: ��
����ֵ: �� 
**********************************************************/
void USART1_IRQHandler(void)
{
	// �����ж�
	if (USART_GetIntStatus(UART_DRIVE_TYPE_DEF, USART_INT_RXDNE) != RESET)
	{
		uint8 rxdata = 0x00;
		
		// ��������
		rxdata = (uint8)USART_ReceiveData(UART_DRIVE_TYPE_DEF);
		
		if (NULL != uartDriveCB.receiveDataThrowService)
		{			
			(*uartDriveCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}
	}
	
	// �ж�ģʽ����
#if (UART_DRIVE_TX_MODE == UART_DRIVE_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if (USART_GetIntStatus(UART_DRIVE_TYPE_DEF, USART_INT_TXC) != RESET)
	{
		uint16 head = uartDriveCB.tx.head;
		uint16 end;
		uint16 index = uartDriveCB.tx.index;
		uint8 txdata = 0x00;

		// ��������жϱ�־
		USART_ClrIntPendingBit(UART_DRIVE_TYPE_DEF, USART_INT_TXC);

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if (index < uartDriveCB.tx.cmdQueue[head].length)
		{
			txdata = uartDriveCB.tx.cmdQueue[head].buff[uartDriveCB.tx.index++];
			
			// �������
			USART_SendData(UART_DRIVE_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			uartDriveCB.tx.cmdQueue[head].length = 0;
			uartDriveCB.tx.head ++;
			uartDriveCB.tx.head %= UART_DRIVE_TX_QUEUE_SIZE;
			uartDriveCB.tx.index = 0;

			head = uartDriveCB.tx.head;
			end = uartDriveCB.tx.end;
			
			// ����֡���зǿգ�����������һ������֡
			if (head != end)
			{
				txdata = uartDriveCB.tx.cmdQueue[head].buff[uartDriveCB.tx.index++];

				// �������
				USART_SendData(UART_DRIVE_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ�������ж�
				USART_ConfigInt(UART_DRIVE_TYPE_DEF, USART_INT_TXC, DISABLE);
				
				uartDriveCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif

	// Other USARTx interrupts handler can go here ...				 
	if(USART_GetFlagStatus(UART_DRIVE_TYPE_DEF, USART_FLAG_OREF) != RESET) //----------------------- ��������ж� 	
	{
		USART_ClrFlag(UART_DRIVE_TYPE_DEF,USART_FLAG_OREF); 		  //----------------------- �����������жϱ�־λ 
		USART_ReceiveData(UART_DRIVE_TYPE_DEF);						  //----------------------- ��ռĴ���
	}
}

// �����ж��ֽڷ���
void UART_DRIVE_IR_StartSendData(uint8 data)
{
	// �ȶ�SR�������DR���TC��־���
	//UART_GetITStatus(UART_DRIVE_TYPE_DEF, UART_IT_TC);

	// ����һ���ֽ�
	USART_SendData(UART_DRIVE_TYPE_DEF, data);

	// �򿪷�������ж�
	USART_ConfigInt(UART_DRIVE_TYPE_DEF, USART_INT_TXC, ENABLE);
}

// ��������һ���ֽ�����
void UART_DRIVE_BC_SendData(uint8 data)
{	
	// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�) 
	//UART_ClearITPendingBit(UART_DRIVE_TYPE_DEF, UART_CLR_TC);		

	// �������
	USART_SendData(UART_DRIVE_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while(USART_GetFlagStatus(UART_DRIVE_TYPE_DEF, USART_FLAG_TXC) == RESET);	
}

// ������������
void UART_DRIVE_BC_StartTx(UART_DRIVE_CB* pCB)
{
	uint16 index = pCB->tx.index;
	uint16 head = pCB->tx.head;
	uint16 end = pCB->tx.end;

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
	
	// ��ǰ����֡δ������ʱ����������
	while(index < pCB->tx.cmdQueue[head].length)
	{
		// һֱ��䷢��
		UART_DRIVE_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
}

// ���ʹ���,���ַ��ͻ������ǿ�ʱ,�����жϷ���
void UART_DRIVE_TxProcess(UART_DRIVE_CB* pCB)
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
		UART_DRIVE_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		// ���÷���æ״̬
		pCB->tx.txBusy = TRUE;
	}


// ������ʽʱ��������
#else
	UART_DRIVE_BC_StartTx(pCB);
#endif
}

// ���ͻ����������һ������������
BOOL UART_DRIVE_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = uartDriveCB.tx.head;
	uint16 end = uartDriveCB.tx.end;
	
	// ��������
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// ���ͻ������������������
	if ((end + 1) % UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	uartDriveCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		uartDriveCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	uartDriveCB.tx.cmdQueue[end].length = length;
	
	// ���ͻ��ζ��и���λ��
	uartDriveCB.tx.end ++;
	uartDriveCB.tx.end %= UART_DRIVE_TX_QUEUE_SIZE;
	uartDriveCB.tx.cmdQueue[uartDriveCB.tx.end].length = 0;

	return TRUE;
}


