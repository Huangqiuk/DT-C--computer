/********************************************************************************
  * �� �� ��: tempCapture.c
  * �� �� ��: ����            
  *******************************************************************************/
  
/********************************* ͷ�ļ�************************************/
#include "meterControl.h"
#include "led.h"
#include "delay.h"
#include "math.h"
#include "state.h"
#include "delay.h"
#include "uartProtocol.h"
#include "Source_Relay.h"

/******************************** ȫ�ֱ���*********************************/
METER_CONTROL_CB meterControlCB;				

/******************************** �ڲ���������*********************************/

// ����ģ�⿪�ؿ�������
AVOMETER_PT_PIN AvomentPtPin[] = {
    
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_14},
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_13},
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_12},
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_1},
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_0},  

};
// ����ģ�⿪�ؿ�������
AVOMETER_PT_PIN AvomentFuntionPin[FUNTION_PIN_MAX] = {
    
    {RCC_APB2_PERIPH_GPIOA, GPIOA, GPIO_PIN_5},
    {RCC_APB2_PERIPH_GPIOA, GPIOA, GPIO_PIN_6},
    {RCC_APB2_PERIPH_GPIOA, GPIOA, GPIO_PIN_7},
    {RCC_APB2_PERIPH_GPIOA, GPIOA, GPIO_PIN_2},
};
// ���ñ���մ��ڳ�ʼ��
void Avoment_Uart2_Init(void)
{
//    GPIO_InitTypeDef    GPIO_InitStructure;
//    USART_InitTypeDef   USART_InitStructure;
//    NVIC_InitTypeDef    NVIC_InitStructure;
//    
//    /* ʹ�ܴ���3����ʱ�� */
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//    
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           // ��������ģʽ
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       // 50MHz�ٶ�
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
//    
//    USART_InitStructure.USART_BaudRate = 2400;
//    USART_InitStructure.USART_Mode = USART_Mode_Rx;                     // ���������ģʽ
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                         // 8λ���ݳ���
//    USART_InitStructure.USART_StopBits = USART_StopBits_1_5;                              // 1λֹͣλ
//    USART_InitStructure.USART_Parity = USART_Parity_No;                                 // ����żУ��
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;     // ��Ӳ����
//    USART_Init(USART2, &USART_InitStructure);
//    USART_ITConfig(USART2, USART_IT_RXNE , ENABLE);
//    USART_Cmd(USART2, ENABLE);
//    
//    /* ���ô���1�жϲ��� */
//    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;               // ����1�ж�
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       // ��ռ���ȼ���1
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              // �����ȼ�3
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 // �ж�ʹ��
//		NVIC_Init(&NVIC_InitStructure);

    GPIO_InitType GPIO_InitStructure;
	USART_InitType USART_InitStructure;
	NVIC_InitType NVIC_InitStructure;

	// Enable GPIO clock  
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA|RCC_APB2_PERIPH_AFIO, ENABLE);    
	// Enable USART Clock   
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART2, ENABLE);

    // Initialize GPIO_InitStructure
    GPIO_InitStruct(&GPIO_InitStructure);

    // Configure USART Rx as alternate function push-pull and pull-up
    GPIO_InitStructure.Pin            = GPIO_PIN_3;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Input;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART2;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    // USART configuration
    USART_InitStructure.BaudRate            = 2400;
    USART_InitStructure.WordLength          = USART_WL_8B;
    USART_InitStructure.StopBits            = USART_STPB_1;
    USART_InitStructure.Parity              = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX ;
    USART_Init(USART2, &USART_InitStructure);

    // Enable the USART Interrupt
    NVIC_InitStructure.NVIC_IRQChannel            = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the USART
    USART_Enable(USART2, ENABLE);
    // Enable USART Receive and Transmit interrupts   
    USART_ConfigInt(USART2, USART_INT_RXDNE, ENABLE);    
    //USART_ConfigInt(UART_DRIVE_TYPE_DEF, USART_INT_TXC, ENABLE); 


}

/***************************************************************
 * �� �� ����USART2_IRQHandler
 * ������Σ�None
 * �������Σ�None
 * ��������������2�ж�ִ�к���
***************************************************************/
void USART2_IRQHandler(void)
{
    uint8_t Rxdata;
    
    // �����ж�
	if (USART_GetIntStatus(USART2, USART_INT_RXDNE) != RESET)
    {
        /* �������� */
        Rxdata = (uint8_t)USART_ReceiveData(USART2);
        TIMER_ResetTimer(TIMER_ID_GET_AVOMENT);
        // �ݲ������������
        if(!meterControlCB.revUpdata)
        {
            meterControlCB.RevBuf[meterControlCB.revLength++] = Rxdata;
            meterControlCB.revLength %= AVOMETER_REV_LENGTH_MAX;
        }
    }
    /* ��ѯORE״̬ */
    if( USART_GetFlagStatus(USART2, USART_FLAG_OREF) != RESET)
	{	
			USART_GetIntStatus(USART2, USART_INT_OREF);
			USART_ReceiveData(USART2);
	}
}

// ���ñ�PT���ų�ʼ��
void Avoment_ControlPin_Init(void)
{
    uint8 i = 0;
    GPIO_InitType    GPIO_InitStructure;
	GPIO_InitStruct(&GPIO_InitStructure);
    // �������ų�ʼ��
    for(i = 0; i < sizeof(AvomentPtPin)/sizeof(AvomentPtPin[0]); i++)
    {
        RCC_EnableAPB2PeriphClk(AvomentPtPin[i].Clock, ENABLE);
        GPIO_InitStructure.Pin = AvomentPtPin[i].Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // �������ģʽ
        GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;   // 50MHz�ٶ�
        GPIO_InitPeripheral(AvomentPtPin[i].Port, &GPIO_InitStructure); 
    }
    // �������ų�ʼ��
    for(i = 0; i < sizeof(AvomentFuntionPin)/sizeof(AvomentFuntionPin[0]); i++)
    {
         RCC_EnableAPB2PeriphClk(AvomentFuntionPin[i].Clock, ENABLE);
        GPIO_InitStructure.Pin = AvomentFuntionPin[i].Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // �������ģʽ
        GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;   // �ٶ�
        GPIO_InitPeripheral(AvomentFuntionPin[i].Port, &GPIO_InitStructure); 
    }
	
    Delayms(250);
	Delayms(250);
	Delayms(250);
	Delayms(250);
	GPIO_WriteBit(AvomentFuntionPin[FUNTION_PIN_TX_EN].Port,AvomentFuntionPin[FUNTION_PIN_TX_EN].Pin,Bit_SET);
    
}

// ������Ϣ��ʱ���ص�
void AvomentRevDataTimeout(uint32 prame)
{
    static uint8 revErrCnt = 0,keyFlip = 0;
    // �յ�һ֡����������
    if(AVOMETER_REV_LENGTH_MAX - 1 == meterControlCB.revLength)
    {
       // ��մ������ֵ
       revErrCnt = 0;
       // �Ͽ�����RS232������
       GPIO_SetBits(AvomentFuntionPin[FUNTION_PIN_RS232].Port,AvomentFuntionPin[FUNTION_PIN_RS232].Pin);
       LED_Reversal();
       // �������ݸ���
       meterControlCB.revUpdata = bTRUE;
    }
    // ��ʱ��û�л������
    else if(!meterControlCB.revLength)
    {
        // �ﵽ������
        if(AVOMETER_REV_ERR_CNT_MAX <= revErrCnt++)
        {
            // 3S��תһ��
            if(0 == revErrCnt % 15)
                keyFlip = !keyFlip;
            // ����RS232 GPIO
            GPIO_WriteBit(AvomentFuntionPin[FUNTION_PIN_RS232].Port,AvomentFuntionPin[FUNTION_PIN_RS232].Pin,(Bit_OperateType)keyFlip);
        }  
    }
    meterControlCB.revLength = 0;
}


// ��������ϱ�
void Avoment_ReportMeasureResultACK(uint32 canUse)
{
	union{
	float value;
	uint8 buf[4];
	}Flaot2Byte;
	uint8 sendBuf[6];
	
	INT_PARA IntPara;
	memset(sendBuf,0xFF,6);
	Flaot2Byte.value = meterControlCB.reportRes;
	IntPara.INT=(int)meterControlCB.reportRes;
	// ����PUCS215A��Э��������޸� by sl 20211217
	// �ϱ���������
	sendBuf[0]=meterControlCB.resPara.mesureType;
	// �ϰ����ͨ��
	sendBuf[1]=meterControlCB.resPara.chl;
					
	// ������Ϊ������Ч����з�����Ч����
	if(meterControlCB.resPara.canUse)
	{
		    meterControlCB.resPara.canUse=bFALSE;
		   if(sendBuf[0]==MEASURE_OHM)
		   {
		     sendBuf[2] = IntPara.data[0];
			 sendBuf[3] = IntPara.data[1];
			 sendBuf[4] = IntPara.data[2];
			 sendBuf[5] = IntPara.data[3]; 
		   }
		   else
		   {	   
			 sendBuf[2] = Flaot2Byte.buf[0];
			 sendBuf[3] = Flaot2Byte.buf[1];
			 sendBuf[4] = Flaot2Byte.buf[2];
			 sendBuf[5] = Flaot2Byte.buf[3];
		   }
	}
	//��·ʱ��ʱ��û�ȶ����ϱ����ֵ��
	if((canUse==9)&&(sendBuf[0]==MEASURE_OHM))
	{
		IntPara.INT=1000000000;
	    sendBuf[2] = IntPara.data[0];
		sendBuf[3] = IntPara.data[1];
		sendBuf[4] = IntPara.data[2];
		sendBuf[5] = IntPara.data[3];
	 
	}
	// ��û���ϱ�
	if(meterControlCB.dataDir)
	{
		    
			// �����ϱ�
			MultimeterSendMsgToMaistr(UART_CMD_AVOMETER_ACK,sendBuf,6);
			// ������ϱ�������
			meterControlCB.dataDir = 0;
		    //��λ���м̵���
		    RelayReset();
	}
}

// ���ñ�PT���õ�λ
void Avoment_SetPTGear(AVOMETER_GEARS_CMD gear)
{
    // �����ֲ���õ����²�����λPT�ŵ�ƽ null DCV ACV DCma ACma OHM DCA ACA
    const char gearPinLeavel[] = {00,0x0C,0x0B,0x1A,0x1A,0x1E,0x0A,0x0A};
    uint8 i = 0;
    for(i = 0; i < sizeof(AvomentPtPin)/sizeof(AvomentPtPin[0]); i++)
    {
        if(gearPinLeavel[gear] & (1 << i))
            GPIO_SetBits(AvomentPtPin[i].Port,AvomentPtPin[i].Pin);
        else
            GPIO_ResetBits(AvomentPtPin[i].Port,AvomentPtPin[i].Pin);
    } 
}

// �̰���һ�����ܼ�
void Avoment_ShortPressDown(AVOMETER_FUNTION_PIN_E key)
{
    // ���͹��ܼ�200ms��ſ�
    GPIO_ResetBits(AvomentFuntionPin[key].Port,AvomentFuntionPin[key].Pin);
    Delayms(200);	
    GPIO_SetBits(AvomentFuntionPin[key].Port,AvomentFuntionPin[key].Pin);
    Delayms(250);
	Delayms(250);
	Delayms(250);
	Delayms(250);
	
	
}

// �����ֽ���
uint8 Avoment_FormatGetNum(uint8 format)
{
    uint8 numFormat[] = {0x7D,0x05,0x5B,0x1F,0x27,0x3E,0X7E,0x15,0X7F,0x3F};
    uint8 j = 0;
    for(j = 0; j < sizeof(numFormat)/sizeof(numFormat[0]);j++)
    {
        if(numFormat[j] == format)
        {
            return j;
        }
    }
    return 0xFF;
}
// ��÷��ţ����ڵ���������ʾLed�����Ķ�λ��ͼ���ú����Ӷ�λ���в�ѯָ��ͼ���Ƿ����
//����TRUE������FALSE��
BOOL Avoment_FormatGetSymbol(uint8 *msg,AVOMETER_SYMBOL_E symbol)
{
    //uint8 charFormat[AVOMETER_SYMBOL_MAX] = {11,27,43,51,2,3,90,73,81,83};
	uint8 charFormat[AVOMETER_SYMBOL_MAX] = {11,27,43,59,2,3,90,73,81,83};
    uint8 space = 0;
    if(symbol > AVOMETER_SYMBOL_MAX)
        return bFALSE;
    space = charFormat[symbol];
	//msg[space / 8]��ø�ͼ�����������±ꡣ(1 << (space % 8)))Ϊ��ͼ���ڸ�4bit��λ����Чλ�á�
	//charFormat����Ϊ������Ƶġ�
    if(msg[space / 8] & (1 << (space % 8)))
    {
        return bTRUE;
    }
    return bFALSE;    
}
// ���ñ����ݽ���
void Avoment_RevDataParse(uint8 *msg)
{
    uint8 i = 0,numFormatBuf[4];
    uint8 point = 0;
    uint16 res = 0,temp;
    // ���ֻ���
    for(i = 0; i < 4; i++)
    {
        numFormatBuf[i] = ((msg[1 + 2*i] & 0x07) << 4) + (msg[2 + 2*i] & 0x0F);
    }
    // ��ת��Ϊ����
    for(i = 0; i < 4; i++)
    {
        temp = Avoment_FormatGetNum(numFormatBuf[i]);
        if(0xFF == temp )
        {
           meterControlCB.revNewNumRes = AVOMETER_REV_NUM_ERR_VAL;
           printf("Cnt't Prase Number\r\n");
           return;
        }
        res = 10 * res + temp;
    }
    // �õ�С����λ��
    for(i = AVOMETER_SYMBOL_P3; i >= AVOMETER_SYMBOL_P1; i--)
    {
        if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,(AVOMETER_SYMBOL_E)i))
        {
            point =  AVOMETER_SYMBOL_P3 - i + 1;
        }
    }
    // ���ʵ�ʸ�����
    meterControlCB.revNewNumRes = res * 1.0 / pow(10,point);
    // �����ж�
    meterControlCB.revNewNumRes *= Avoment_FormatGetSymbol(meterControlCB.RevBuf,(AVOMETER_SYMBOL_E)AVOMETER_SYMBOL_CHAR)?(-1):1;
    // ��ʾ��������ֵ
    printf("%f \r\n",meterControlCB.revNewNumRes);
	//memset(meterControlCB.RevBuf,0,15);
}

// ��������ȶ��������ϱ�
void CheckDateStableAndReport(void)
{
		uint8 i = 0;
		float averageMeasureRes = 0;
        float sumMeasureResErr = 0;
        float Coefficient = 0;
		// ���
		for(i = 0; i < AVOMETER_FILTER_BUF_MAX; i++)
		{
				averageMeasureRes += meterControlCB.filter.measureRes[i];
		}
		// ƽ��
		averageMeasureRes /= AVOMETER_FILTER_BUF_MAX;
		// ���������
		
		
		for(i = 0; i < AVOMETER_FILTER_BUF_MAX; i++)
		{
				sumMeasureResErr += meterControlCB.filter.measureRes[i] - averageMeasureRes;
		}
		
		
		// ����������Ҫ��        ������(averageMeasureRes>=0)����ж�������������ѹ����ֵΪ����ʱ���͸��ӿڰ嶼��FF��bug  20201023
		if((averageMeasureRes>=0)&&(sumMeasureResErr <= meterControlCB.revNewNumRes * AVOMETER_FILTER_ERR_COE))
		{
				// ����ŷķ��Ҫ�жϵ�λ �ϱ�ŷ
				if(AVOMETER_GEARS_OHM == meterControlCB.gearsMode)
				{
						if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_M_OHM))
							Coefficient = 1000000;
						else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_K_OHM))
							Coefficient = 1000;
						else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_OHM))
							Coefficient = 1;
							 
				}
				// ���ڵ�ѹ���������ʾ����mV����mA
				else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_MILLI))
				{
						Coefficient = 1;
				}
				else
				{
						Coefficient = 1;
				}
				meterControlCB.reportRes = meterControlCB.revNewNumRes * Coefficient;
				// ��������ϱ���Ч
				meterControlCB.resPara.canUse=bTRUE;
				Avoment_ReportMeasureResultACK(bTRUE);
		}
		//������(averageMeasureRes<0)����ж�������������ѹ����ֵΪ����ʱ���͸��ӿڰ嶼��FF��bug  20201023
		else if((averageMeasureRes<0)&&(sumMeasureResErr >= meterControlCB.revNewNumRes * AVOMETER_FILTER_ERR_COE))//����������ֵС��0ʱ
        {
		       // ����ŷķ��Ҫ�жϵ�λ �ϱ�ŷ
				if(AVOMETER_GEARS_OHM == meterControlCB.gearsMode)
				{
						if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_M_OHM))
							Coefficient = 1000000;
						else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_K_OHM))
							Coefficient = 1000;
						else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_OHM))
							Coefficient = 1;
							 
				}
				// ���ڵ�ѹ���������ʾ����mV����mA
				else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_MILLI))
				{
						Coefficient = 1;
				}
				else
				{
						Coefficient = 1;
				}
				meterControlCB.reportRes = meterControlCB.revNewNumRes * Coefficient;
				// ��������ϱ���Ч
				meterControlCB.resPara.canUse=bTRUE;
				Avoment_ReportMeasureResultACK(bTRUE);
		}

}

void Avoment_StartInit(uint32_t para)
{
   Avoment_StitchGear(5);
}

// ���ñ��ʼ��
void Avoment_Init(void)
{
    // ���մ��ڳ�ʼ��
    Avoment_Uart2_Init();
    // ���ñ�PT���ų�ʼ��
    Avoment_ControlPin_Init();
    // ������ñ������Ϣ��ʱ��
    TIMER_AddTask(TIMER_ID_GET_AVOMENT,
        200,
        AvomentRevDataTimeout,
        0,
        TIMER_LOOP_FOREVER,
        ACTION_MODE_ADD_TO_QUEUE);
	
	TIMER_AddTask(TIMER_ID_OHM_INIT,
        2000,
        Avoment_StartInit,
        0,
        1,
        ACTION_MODE_ADD_TO_QUEUE);
}


// ״̬���л�
void Avoment_StateEnter(AVOMETER_STATE_E state)
{
    meterControlCB.state = state;
}

// �л���λ
void Avoment_StitchGear(AVOMETER_GEARS_CMD gear)
{
		if(AVOMETER_GEARS_MAX <= gear)
		{
				return;
		}
		meterControlCB.gearsMode = gear;
		Avoment_StateEnter(AVOMETER_STATE_PT);
}
// ���ñ����ݴ���
void Avoment_Process(void)
{
    uint8 shortPressEnable = 0;
    if(bFALSE ==  meterControlCB.revUpdata)
    {
        return;
    }
    // ���ñ����ݽ��н���
    Avoment_RevDataParse(meterControlCB.RevBuf);
    // �鿴����״̬
    switch(meterControlCB.state)
    {
        case AVOMETER_STATE_NULL:
        break;
        // ����PT
        case AVOMETER_STATE_PT:
						// ��ս����˲���
						memset(&meterControlCB.filter,0,sizeof(METER_FILTER_CB));
            // ���ñ����õ�λ
            Avoment_SetPTGear(meterControlCB.gearsMode);
            // ״̬���л�
            Avoment_StateEnter(AVOMETER_STATE_CHECK_CHAR);
        break;    
        // ������ͼ���Ƿ���Ҫ��secect	
        case AVOMETER_STATE_CHECK_CHAR:
            // ������λ
            if(AVOMETER_GEARS_ACA == meterControlCB.gearsMode || 
                AVOMETER_GEARS_ACMA == meterControlCB.gearsMode || 
                AVOMETER_GEARS_ACV == meterControlCB.gearsMode)
            {
                shortPressEnable = !Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_AC);
            }
            // ֱ����λ
            else if(AVOMETER_GEARS_DCA == meterControlCB.gearsMode || 
                AVOMETER_GEARS_DCMA == meterControlCB.gearsMode || 
                AVOMETER_GEARS_DCV == meterControlCB.gearsMode)
            {
                shortPressEnable = !Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_DC);
            }
            if(shortPressEnable)
                // �̰�һ��ѡ��������л�
                Avoment_ShortPressDown(FUNTION_PIN_SELECT);
            else
                // ״̬���л�
                Avoment_StateEnter(AVOMETER_STATE_GET_RES);
        break;  
        // ��ȡ���	        
        case AVOMETER_STATE_GET_RES:
						// ����ʱ��Ч��
            if(AVOMETER_REV_NUM_ERR_VAL != meterControlCB.revNewNumRes)
						{
								// ��ӵ����������ڼ�������Ƿ��ȶ�
								meterControlCB.filter.length++;
								meterControlCB.filter.length %= AVOMETER_FILTER_BUF_MAX;
								meterControlCB.filter.measureRes[meterControlCB.filter.length] = meterControlCB.revNewNumRes;
								// ��������ȶ��������ϱ�
								CheckDateStableAndReport();
						}
        break; 
        default: break;
    
    }
    // �����ڴν��н���
    meterControlCB.revUpdata = bFALSE;
}

