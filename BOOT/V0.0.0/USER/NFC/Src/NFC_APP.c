#include "SI522_App.h"
#include "NFC_APP.h"
#include "main.h"

NFC_APP_CB NFC_Cb=NFC_Default;


// Global variable definition
uint8_t  Si522_IRQ_flagA = 0;
uint8_t  Si522_IRQ_flag = 0;

extern unsigned char ValSet;
extern unsigned int	 none;
extern unsigned char Reader_type;//用于确定工作芯片
extern unsigned char Reader_mode;//用于确定读卡模式
extern unsigned char ALPPL_mode; //用于确定ALPPL模式
extern unsigned char ACD_IRQ;
extern unsigned char ACDTIMER_IRQ;
extern unsigned char OSCMon_IRQ;
extern unsigned char ACC_IRQ;
extern unsigned char RFLowDetect_IRQ;

//SI522芯片初始化
void NFC_Init(void)
{
    PCD_SI522_TypeA_Init();	
}

//NFC延时到时回调
void NFC_TimeCallBack(uint32_t param)
{
  SET_NfcTimerSta(NFC_TIMER_END);
}

//NFC延时启动函数
void NFC_DelayStart(uint32_t  xtime)
{
	TIMER_AddTask(TIMER_ID_NFC_DELAY,
					xtime,
					NFC_TimeCallBack,
					0,
					1,
					ACTION_MODE_DO_AT_ONCE);
	 SET_NfcTimerSta(NFC_TIMER_NULL);
}

void NFC_DelayKill(void)
{
	 TIMER_KillTask(TIMER_ID_NFC_DELAY);
}

 void NFC_EnterState(NFC_WORK_STEP NFCState)
 {	 
	NFC_Cb.PreStep=NFC_Cb.Step; 
    NFC_Cb.Step=NFCState;
 }


void NFC_Task(void)
{
	 switch (NFC_Cb.Step)
	{
		case STEP_IDLE :
			
		    NFC_EnterState(STEP_WORKING);
		     break;
		//NFC读卡任务
	    case STEP_WORKING :
			  PCD_SI522_TypeA_GetUID();
		    NFC_EnterState(STEP_WAITING);
		     break;
		//NFC延时等待
	    case STEP_WAITING :
			 if(NFC_Cb.PreStep!=(NFC_Cb.Step))
		     {
			    NFC_DelayStart(500);
			    NFC_EnterState(STEP_WAITING);
			 }
			 else 
			 {
			   if(NFC_Cb.TSta==NFC_TIMER_END)
			   {
			      NFC_EnterState(STEP_WORKING);				   
			   } 			 
			 }
		     break;	
		  default :
              break;			
	}
	 
}

//NFC扫卡启动函数
void NFC_PowerOn(void)
{
	/*
	static BOOL Flag = FALSE;
	
	if(STATE_PASSWORD_SIGN_IN==stateCB.state)	
	{
		STATE_EnterState(STATE_TRIP_MAIN_SPD_MILE);
	}
	
	Flag = !Flag;
	*/
}


