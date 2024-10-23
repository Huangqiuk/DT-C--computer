#include "debug.h"
#include "meterControl.h"
/* 串口重映射 */
#pragma import(__use_no_semihosting)             
/* 标准库需要的支持函数 */
struct __FILE
{ 
	int handle;
};

FILE __stdout;
/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{ 
	x = x;
}

/* 重定义fputc函数 */
int fputc(int ch, FILE *f)
{      	
	// 填充数据
	USART_SendData(USART1, ch);

	// 未发送完，持续等待
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXC) == RESET);
	
	return (ch);
}

/***************************************************************
 * 函 数 名：USART3_IRQHandler
 * 函数入参：None
 * 函数出参：None
 * 功能描述：串口3中断执行函数
***************************************************************/
void USART1_IRQHandler(void)
{
    uint8_t Rxdata;
    
    /* 检查USART1接收中断发生与否 */
    if(USART_GetIntStatus(USART1, USART_INT_RXDNE) != RESET)
    {
        /* 接收数据 */
        Rxdata = (uint8_t)USART_ReceiveData(USART1);
				Rxdata = Rxdata;
    }
    
    /* 查询ORE状态 */
    if( USART_GetFlagStatus(USART1, USART_FLAG_OREF) != RESET)
	{	
			USART_GetIntStatus(USART1, USART_INT_OREF);
			USART_ReceiveData(USART1);
	}
	
		
}

/***************************************************************
 * 函 数 名：UART3_GPIO_Config
 * 函数入参：None
 * 函数出参：None
 * 返 回 值：None
 * 功能描述：串口3引脚配置
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
 * 函 数 名：UART3_Config
 * 函数入参：uint32_t baud -> 串口通信波特率
 * 函数出参：None
 * 返 回 值：None
 * 功能描述：串口3外设参数配置
***************************************************************/
static void UART1_Config(uint32_t baud)
{
//    USART_InitTypeDef   USART_InitStructure;
//    
//    /* 使能串口3外设时钟 */
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
//    
//    /* 配置串口3参数 */
//    USART_InitStructure.USART_BaudRate = baud;
//    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                     // 发送与接收模式
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                         // 8位数据长度
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;                              // 1位停止位
//    USART_InitStructure.USART_Parity = USART_Parity_No;                                 // 无奇偶校验
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;     // 无硬件流
//    USART_Init(USART1, &USART_InitStructure);
//    
//    /* 使能串口1 */
//    USART_Cmd(USART1, ENABLE);
}

/***************************************************************
 * 函 数 名：UART3_NVIC_Config
 * 函数入参：None
 * 函数出参：None
 * 功能描述：串口3中断配置
***************************************************************/
static void UART1_NVIC_Config(void)
{
//    NVIC_InitTypeDef    NVIC_InitStructure;
//    
//    /* 配置串口1接收中断 */
//    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//    
//    /* 配置串口1中断参数 */
//    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;               // 串口1中断
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       // 抢占优先级：1
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              // 子优先级3
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 // 中断使能
//	NVIC_Init(&NVIC_InitStructure);
}

/***************************************************************
 * 函 数 名：UART3_DRIVER_HwInit
 * 函数入参：uint32_t baud -> 串口通信波特率
 * 函数出参：None
 * 功能描述：串口3硬件初始化
***************************************************************/
static void UART1_DRIVER_HwInit(uint32_t baud)
{
    /* 配置串口3的GPIO口 */
    UART1_GPIO_Config(baud);  
}


void Debug_Init(void)
{	
	/* 硬件UART1配置 */
	UART1_DRIVER_HwInit(UART1_DRIVE_BAUD_RATE);
}
