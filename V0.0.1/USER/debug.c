#include "debug.h"
#include "meterControl.h"
/* ������ӳ�� */
#pragma import(__use_no_semihosting)             
/* ��׼����Ҫ��֧�ֺ��� */
struct __FILE
{ 
	int handle;
};

FILE __stdout;
/* ����_sys_exit()�Ա���ʹ�ð�����ģʽ */
void _sys_exit(int x)
{ 
	x = x;
}

/* �ض���fputc���� */
int fputc(int ch, FILE *f)
{      	
	// �������
	USART_SendData(USART1, ch);

	// δ�����꣬�����ȴ�
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXC) == RESET);
	
	return (ch);
}

/***************************************************************
 * �� �� ����USART3_IRQHandler
 * ������Σ�None
 * �������Σ�None
 * ��������������3�ж�ִ�к���
***************************************************************/
void USART1_IRQHandler(void)
{
    uint8_t Rxdata;
    
    /* ���USART1�����жϷ������ */
    if(USART_GetIntStatus(USART1, USART_INT_RXDNE) != RESET)
    {
        /* �������� */
        Rxdata = (uint8_t)USART_ReceiveData(USART1);
				Rxdata = Rxdata;
    }
    
    /* ��ѯORE״̬ */
    if( USART_GetFlagStatus(USART1, USART_FLAG_OREF) != RESET)
	{	
			USART_GetIntStatus(USART1, USART_INT_OREF);
			USART_ReceiveData(USART1);
	}
	
		
}

/***************************************************************
 * �� �� ����UART3_GPIO_Config
 * ������Σ�None
 * �������Σ�None
 * �� �� ֵ��None
 * ��������������3��������
***************************************************************/
static void UART1_GPIO_Config(uint32_t baud)
{
	GPIO_InitType GPIO_InitStructure;
	USART_InitType USART_InitStructure;
	NVIC_InitType NVIC_InitStructure;

	// Enable GPIO clock  
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB|RCC_APB2_PERIPH_AFIO, ENABLE);    
	// Enable USART Clock   
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_USART1, ENABLE);

    // Initialize GPIO_InitStructure
    GPIO_InitStruct(&GPIO_InitStructure);

    // Configure USART Tx as alternate function push-pull
    GPIO_InitStructure.Pin            = GPIO_PIN_9;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART1;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    // Configure USART Rx as alternate function push-pull and pull-up
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
    USART_Init(USART1, &USART_InitStructure);

    // Enable the USART Interrupt
    NVIC_InitStructure.NVIC_IRQChannel            = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the USART
    USART_Enable(USART1, ENABLE);
    // Enable USART Receive and Transmit interrupts   
    USART_ConfigInt(USART1, USART_INT_RXDNE, ENABLE);    
    //USART_ConfigInt(UART_DRIVE_TYPE_DEF, USART_INT_TXC, ENABLE);    
	
}

/***************************************************************
 * �� �� ����UART3_Config
 * ������Σ�uint32_t baud -> ����ͨ�Ų�����
 * �������Σ�None
 * �� �� ֵ��None
 * ��������������3�����������
***************************************************************/
static void UART1_Config(uint32_t baud)
{
//    USART_InitTypeDef   USART_InitStructure;
//    
//    /* ʹ�ܴ���3����ʱ�� */
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
//    
//    /* ���ô���3���� */
//    USART_InitStructure.USART_BaudRate = baud;
//    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                     // ���������ģʽ
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                         // 8λ���ݳ���
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;                              // 1λֹͣλ
//    USART_InitStructure.USART_Parity = USART_Parity_No;                                 // ����żУ��
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;     // ��Ӳ����
//    USART_Init(USART1, &USART_InitStructure);
//    
//    /* ʹ�ܴ���1 */
//    USART_Cmd(USART1, ENABLE);
}

/***************************************************************
 * �� �� ����UART3_NVIC_Config
 * ������Σ�None
 * �������Σ�None
 * ��������������3�ж�����
***************************************************************/
static void UART1_NVIC_Config(void)
{
//    NVIC_InitTypeDef    NVIC_InitStructure;
//    
//    /* ���ô���1�����ж� */
//    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//    
//    /* ���ô���1�жϲ��� */
//    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;               // ����1�ж�
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       // ��ռ���ȼ���1
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              // �����ȼ�3
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 // �ж�ʹ��
//	NVIC_Init(&NVIC_InitStructure);
}

/***************************************************************
 * �� �� ����UART3_DRIVER_HwInit
 * ������Σ�uint32_t baud -> ����ͨ�Ų�����
 * �������Σ�None
 * ��������������3Ӳ����ʼ��
***************************************************************/
static void UART1_DRIVER_HwInit(uint32_t baud)
{
    /* ���ô���3��GPIO�� */
    UART1_GPIO_Config(baud);  
}


void Debug_Init(void)
{	
	/* Ӳ��UART1���� */
	UART1_DRIVER_HwInit(UART1_DRIVE_BAUD_RATE);
}
