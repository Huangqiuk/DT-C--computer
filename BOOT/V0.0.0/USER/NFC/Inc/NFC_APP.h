#ifndef __NFC_APP_H__
#define __NFC_APP_H__	 

#include "main.h"


//NFC定时器状态
typedef enum{
	NFC_TIMER_NULL =0,
	NFC_TIMER_END  =1,		
}NFC_TIMER_STA;


typedef enum{
	STEP_IDLE =0,
	STEP_WORKING =1,
	
	STEP_WAITING =2,	
}NFC_WORK_STEP;

//NFC权限状态
typedef enum{
	VERIFY_WAITING =0,
	VERIFY_PASSED =1,	
}VERIFY_STA;




typedef struct
{
  NFC_TIMER_STA  TSta;	
  NFC_WORK_STEP  Step;	
  NFC_WORK_STEP  PreStep;		
  VERIFY_STA     Verify;
	
	
}NFC_APP_CB;

#define NFC_Default {NFC_TIMER_NULL,STEP_IDLE,STEP_IDLE,VERIFY_WAITING  }

void NFC_Init(void);
void NFC_Task(void);

extern  NFC_APP_CB NFC_Cb;
#define GET_NfcTimerSta()   (NFC_Cb.TSta)
#define SET_NfcTimerSta(A)  (NFC_Cb.TSta=(A))

#define GET_NfcVerifySta()   (NFC_Cb.Verify)
#define SET_NfcVerifySta(A)  (NFC_Cb.Verify=(A))


void NFC_PowerOn(void);
#endif

