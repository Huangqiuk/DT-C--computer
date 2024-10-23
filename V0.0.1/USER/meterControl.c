/********************************************************************************
  * 文 件 名: tempCapture.c
  * 版 本 号: 初版            
  *******************************************************************************/
  
/********************************* 头文件************************************/
#include "meterControl.h"
#include "led.h"
#include "delay.h"
#include "math.h"
#include "state.h"
#include "delay.h"
#include "uartProtocol.h"
#include "Source_Relay.h"

/******************************** 全局变量*********************************/
METER_CONTROL_CB meterControlCB;				

/******************************** 内部函数声明*********************************/

// 电子模拟开关控制引脚
AVOMETER_PT_PIN AvomentPtPin[] = {
    
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_14},
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_13},
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_12},
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_1},
    {RCC_APB2_PERIPH_GPIOB, GPIOB, GPIO_PIN_0},  

};
// 电子模拟开关控制引脚
AVOMETER_PT_PIN AvomentFuntionPin[FUNTION_PIN_MAX] = {
    
    {RCC_APB2_PERIPH_GPIOA, GPIOA, GPIO_PIN_5},
    {RCC_APB2_PERIPH_GPIOA, GPIOA, GPIO_PIN_6},
    {RCC_APB2_PERIPH_GPIOA, GPIOA, GPIO_PIN_7},
    {RCC_APB2_PERIPH_GPIOA, GPIOA, GPIO_PIN_2},
};
// 万用表接收串口初始化
void Avoment_Uart2_Init(void)
{
//    GPIO_InitTypeDef    GPIO_InitStructure;
//    USART_InitTypeDef   USART_InitStructure;
//    NVIC_InitTypeDef    NVIC_InitStructure;
//    
//    /* 使能串口3外设时钟 */
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//    
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           // 上拉输入模式
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       // 50MHz速度
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
//    
//    USART_InitStructure.USART_BaudRate = 2400;
//    USART_InitStructure.USART_Mode = USART_Mode_Rx;                     // 发送与接收模式
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                         // 8位数据长度
//    USART_InitStructure.USART_StopBits = USART_StopBits_1_5;                              // 1位停止位
//    USART_InitStructure.USART_Parity = USART_Parity_No;                                 // 无奇偶校验
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;     // 无硬件流
//    USART_Init(USART2, &USART_InitStructure);
//    USART_ITConfig(USART2, USART_IT_RXNE , ENABLE);
//    USART_Cmd(USART2, ENABLE);
//    
//    /* 配置串口1中断参数 */
//    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;               // 串口1中断
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       // 抢占优先级：1
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              // 子优先级3
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 // 中断使能
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
 * 函 数 名：USART2_IRQHandler
 * 函数入参：None
 * 函数出参：None
 * 功能描述：串口2中断执行函数
***************************************************************/
void USART2_IRQHandler(void)
{
    uint8_t Rxdata;
    
    // 接收中断
	if (USART_GetIntStatus(USART2, USART_INT_RXDNE) != RESET)
    {
        /* 接收数据 */
        Rxdata = (uint8_t)USART_ReceiveData(USART2);
        TIMER_ResetTimer(TIMER_ID_GET_AVOMENT);
        // 暂不允许接收数据
        if(!meterControlCB.revUpdata)
        {
            meterControlCB.RevBuf[meterControlCB.revLength++] = Rxdata;
            meterControlCB.revLength %= AVOMETER_REV_LENGTH_MAX;
        }
    }
    /* 查询ORE状态 */
    if( USART_GetFlagStatus(USART2, USART_FLAG_OREF) != RESET)
	{	
			USART_GetIntStatus(USART2, USART_INT_OREF);
			USART_ReceiveData(USART2);
	}
}

// 万用表PT引脚初始化
void Avoment_ControlPin_Init(void)
{
    uint8 i = 0;
    GPIO_InitType    GPIO_InitStructure;
	GPIO_InitStruct(&GPIO_InitStructure);
    // 编码引脚初始化
    for(i = 0; i < sizeof(AvomentPtPin)/sizeof(AvomentPtPin[0]); i++)
    {
        RCC_EnableAPB2PeriphClk(AvomentPtPin[i].Clock, ENABLE);
        GPIO_InitStructure.Pin = AvomentPtPin[i].Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 推挽输出模式
        GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;   // 50MHz速度
        GPIO_InitPeripheral(AvomentPtPin[i].Port, &GPIO_InitStructure); 
    }
    // 功能引脚初始化
    for(i = 0; i < sizeof(AvomentFuntionPin)/sizeof(AvomentFuntionPin[0]); i++)
    {
         RCC_EnableAPB2PeriphClk(AvomentFuntionPin[i].Clock, ENABLE);
        GPIO_InitStructure.Pin = AvomentFuntionPin[i].Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 推挽输出模式
        GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;   // 速度
        GPIO_InitPeripheral(AvomentFuntionPin[i].Port, &GPIO_InitStructure); 
    }
	
    Delayms(250);
	Delayms(250);
	Delayms(250);
	Delayms(250);
	GPIO_WriteBit(AvomentFuntionPin[FUNTION_PIN_TX_EN].Port,AvomentFuntionPin[FUNTION_PIN_TX_EN].Pin,Bit_SET);
    
}

// 接收信息定时器回调
void AvomentRevDataTimeout(uint32 prame)
{
    static uint8 revErrCnt = 0,keyFlip = 0;
    // 收到一帧完整的数据
    if(AVOMETER_REV_LENGTH_MAX - 1 == meterControlCB.revLength)
    {
       // 清空错误计数值
       revErrCnt = 0;
       // 断开开启RS232的引脚
       GPIO_SetBits(AvomentFuntionPin[FUNTION_PIN_RS232].Port,AvomentFuntionPin[FUNTION_PIN_RS232].Pin);
       LED_Reversal();
       // 发生数据更新
       meterControlCB.revUpdata = bTRUE;
    }
    // 长时间没有获得数据
    else if(!meterControlCB.revLength)
    {
        // 达到最大计数
        if(AVOMETER_REV_ERR_CNT_MAX <= revErrCnt++)
        {
            // 3S翻转一次
            if(0 == revErrCnt % 15)
                keyFlip = !keyFlip;
            // 设置RS232 GPIO
            GPIO_WriteBit(AvomentFuntionPin[FUNTION_PIN_RS232].Port,AvomentFuntionPin[FUNTION_PIN_RS232].Pin,(Bit_OperateType)keyFlip);
        }  
    }
    meterControlCB.revLength = 0;
}


// 测量结果上报
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
	// 根据PUCS215A的协议进行了修改 by sl 20211217
	// 上报测量类型
	sendBuf[0]=meterControlCB.resPara.mesureType;
	// 上班测量通道
	sendBuf[1]=meterControlCB.resPara.chl;
					
	// 主观认为数据有效则进行发送有效数据
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
	//开路时超时还没稳定就上报最大值。
	if((canUse==9)&&(sendBuf[0]==MEASURE_OHM))
	{
		IntPara.INT=1000000000;
	    sendBuf[2] = IntPara.data[0];
		sendBuf[3] = IntPara.data[1];
		sendBuf[4] = IntPara.data[2];
		sendBuf[5] = IntPara.data[3];
	 
	}
	// 还没有上报
	if(meterControlCB.dataDir)
	{
		    
			// 进行上报
			MultimeterSendMsgToMaistr(UART_CMD_AVOMETER_ACK,sendBuf,6);
			// 清除有上报的请求
			meterControlCB.dataDir = 0;
		    //复位所有继电器
		    RelayReset();
	}
}

// 万用表PT设置档位
void Avoment_SetPTGear(AVOMETER_GEARS_CMD gear)
{
    // 根据手册查表得到以下测量档位PT脚电平 null DCV ACV DCma ACma OHM DCA ACA
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

// 短按下一个功能键
void Avoment_ShortPressDown(AVOMETER_FUNTION_PIN_E key)
{
    // 拉低功能键200ms后放开
    GPIO_ResetBits(AvomentFuntionPin[key].Port,AvomentFuntionPin[key].Pin);
    Delayms(200);	
    GPIO_SetBits(AvomentFuntionPin[key].Port,AvomentFuntionPin[key].Pin);
    Delayms(250);
	Delayms(250);
	Delayms(250);
	Delayms(250);
	
	
}

// 对数字解码
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
// 获得符号，串口的数据是显示Led玻璃的段位表图。该函数从段位表中查询指定图标是否存在
//存在TRUE不存在FALSE。
BOOL Avoment_FormatGetSymbol(uint8 *msg,AVOMETER_SYMBOL_E symbol)
{
    //uint8 charFormat[AVOMETER_SYMBOL_MAX] = {11,27,43,51,2,3,90,73,81,83};
	uint8 charFormat[AVOMETER_SYMBOL_MAX] = {11,27,43,59,2,3,90,73,81,83};
    uint8 space = 0;
    if(symbol > AVOMETER_SYMBOL_MAX)
        return bFALSE;
    space = charFormat[symbol];
	//msg[space / 8]求得该图标所在数组下标。(1 << (space % 8)))为该图标在改4bit的位置有效位置。
	//charFormat数字为精心设计的。
    if(msg[space / 8] & (1 << (space % 8)))
    {
        return bTRUE;
    }
    return bFALSE;    
}
// 万用表数据解析
void Avoment_RevDataParse(uint8 *msg)
{
    uint8 i = 0,numFormatBuf[4];
    uint8 point = 0;
    uint16 res = 0,temp;
    // 数字缓存
    for(i = 0; i < 4; i++)
    {
        numFormatBuf[i] = ((msg[1 + 2*i] & 0x07) << 4) + (msg[2 + 2*i] & 0x0F);
    }
    // 先转化为数字
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
    // 得到小数点位置
    for(i = AVOMETER_SYMBOL_P3; i >= AVOMETER_SYMBOL_P1; i--)
    {
        if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,(AVOMETER_SYMBOL_E)i))
        {
            point =  AVOMETER_SYMBOL_P3 - i + 1;
        }
    }
    // 获得实际浮点数
    meterControlCB.revNewNumRes = res * 1.0 / pow(10,point);
    // 符号判断
    meterControlCB.revNewNumRes *= Avoment_FormatGetSymbol(meterControlCB.RevBuf,(AVOMETER_SYMBOL_E)AVOMETER_SYMBOL_CHAR)?(-1):1;
    // 显示解析的数值
    printf("%f \r\n",meterControlCB.revNewNumRes);
	//memset(meterControlCB.RevBuf,0,15);
}

// 检查数据稳定并进行上报
void CheckDateStableAndReport(void)
{
		uint8 i = 0;
		float averageMeasureRes = 0;
        float sumMeasureResErr = 0;
        float Coefficient = 0;
		// 求和
		for(i = 0; i < AVOMETER_FILTER_BUF_MAX; i++)
		{
				averageMeasureRes += meterControlCB.filter.measureRes[i];
		}
		// 平均
		averageMeasureRes /= AVOMETER_FILTER_BUF_MAX;
		// 计算总误差
		
		
		for(i = 0; i < AVOMETER_FILTER_BUF_MAX; i++)
		{
				sumMeasureResErr += meterControlCB.filter.measureRes[i] - averageMeasureRes;
		}
		
		
		// 如果满足误差要求        增加了(averageMeasureRes>=0)这个判断条件，消除电压电流值为负数时发送给接口板都是FF的bug  20201023
		if((averageMeasureRes>=0)&&(sumMeasureResErr <= meterControlCB.revNewNumRes * AVOMETER_FILTER_ERR_COE))
		{
				// 对于欧姆档要判断单位 上报欧
				if(AVOMETER_GEARS_OHM == meterControlCB.gearsMode)
				{
						if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_M_OHM))
							Coefficient = 1000000;
						else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_K_OHM))
							Coefficient = 1000;
						else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_OHM))
							Coefficient = 1;
							 
				}
				// 对于电压电流如果显示的是mV或者mA
				else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_MILLI))
				{
						Coefficient = 1;
				}
				else
				{
						Coefficient = 1;
				}
				meterControlCB.reportRes = meterControlCB.revNewNumRes * Coefficient;
				// 测量结果上报有效
				meterControlCB.resPara.canUse=bTRUE;
				Avoment_ReportMeasureResultACK(bTRUE);
		}
		//增加了(averageMeasureRes<0)这个判断条件，消除电压电流值为负数时发送给接口板都是FF的bug  20201023
		else if((averageMeasureRes<0)&&(sumMeasureResErr >= meterControlCB.revNewNumRes * AVOMETER_FILTER_ERR_COE))//若测量数据值小于0时
        {
		       // 对于欧姆档要判断单位 上报欧
				if(AVOMETER_GEARS_OHM == meterControlCB.gearsMode)
				{
						if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_M_OHM))
							Coefficient = 1000000;
						else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_K_OHM))
							Coefficient = 1000;
						else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_OHM))
							Coefficient = 1;
							 
				}
				// 对于电压电流如果显示的是mV或者mA
				else if(Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_MILLI))
				{
						Coefficient = 1;
				}
				else
				{
						Coefficient = 1;
				}
				meterControlCB.reportRes = meterControlCB.revNewNumRes * Coefficient;
				// 测量结果上报有效
				meterControlCB.resPara.canUse=bTRUE;
				Avoment_ReportMeasureResultACK(bTRUE);
		}

}

void Avoment_StartInit(uint32_t para)
{
   Avoment_StitchGear(5);
}

// 万用表初始化
void Avoment_Init(void)
{
    // 接收串口初始化
    Avoment_Uart2_Init();
    // 万用表PT引脚初始化
    Avoment_ControlPin_Init();
    // 添加万用表接收消息定时器
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


// 状态机切换
void Avoment_StateEnter(AVOMETER_STATE_E state)
{
    meterControlCB.state = state;
}

// 切换档位
void Avoment_StitchGear(AVOMETER_GEARS_CMD gear)
{
		if(AVOMETER_GEARS_MAX <= gear)
		{
				return;
		}
		meterControlCB.gearsMode = gear;
		Avoment_StateEnter(AVOMETER_STATE_PT);
}
// 万用表数据处理
void Avoment_Process(void)
{
    uint8 shortPressEnable = 0;
    if(bFALSE ==  meterControlCB.revUpdata)
    {
        return;
    }
    // 万用表数据进行解析
    Avoment_RevDataParse(meterControlCB.RevBuf);
    // 查看运行状态
    switch(meterControlCB.state)
    {
        case AVOMETER_STATE_NULL:
        break;
        // 设置PT
        case AVOMETER_STATE_PT:
						// 清空接收滤波器
						memset(&meterControlCB.filter,0,sizeof(METER_FILTER_CB));
            // 万用表设置档位
            Avoment_SetPTGear(meterControlCB.gearsMode);
            // 状态机切换
            Avoment_StateEnter(AVOMETER_STATE_CHECK_CHAR);
        break;    
        // 检查测量图标是否需要按secect	
        case AVOMETER_STATE_CHECK_CHAR:
            // 交流档位
            if(AVOMETER_GEARS_ACA == meterControlCB.gearsMode || 
                AVOMETER_GEARS_ACMA == meterControlCB.gearsMode || 
                AVOMETER_GEARS_ACV == meterControlCB.gearsMode)
            {
                shortPressEnable = !Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_AC);
            }
            // 直流档位
            else if(AVOMETER_GEARS_DCA == meterControlCB.gearsMode || 
                AVOMETER_GEARS_DCMA == meterControlCB.gearsMode || 
                AVOMETER_GEARS_DCV == meterControlCB.gearsMode)
            {
                shortPressEnable = !Avoment_FormatGetSymbol(meterControlCB.RevBuf,AVOMETER_SYMBOL_DC);
            }
            if(shortPressEnable)
                // 短按一下选择键进行切换
                Avoment_ShortPressDown(FUNTION_PIN_SELECT);
            else
                // 状态机切换
                Avoment_StateEnter(AVOMETER_STATE_GET_RES);
        break;  
        // 获取结果	        
        case AVOMETER_STATE_GET_RES:
						// 数据时有效的
            if(AVOMETER_REV_NUM_ERR_VAL != meterControlCB.revNewNumRes)
						{
								// 添加到队列中用于检查数据是否稳定
								meterControlCB.filter.length++;
								meterControlCB.filter.length %= AVOMETER_FILTER_BUF_MAX;
								meterControlCB.filter.measureRes[meterControlCB.filter.length] = meterControlCB.revNewNumRes;
								// 检查数据稳定并进行上报
								CheckDateStableAndReport();
						}
        break; 
        default: break;
    
    }
    // 可以在次进行接收
    meterControlCB.revUpdata = bFALSE;
}

