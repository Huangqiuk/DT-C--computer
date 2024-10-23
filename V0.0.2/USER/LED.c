#include "led.h"
#include "pwmled.h"
#include "timer.h"
#include "param.h"

extern LEDCOMMON_CB ledCommonCB;

/*********************************************************************
* �� �� ��: LED_AllOnOff
* �������: data
* ��������: ��
* �� �� ֵ: void 
* ��������: LED ȫ�������
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void LED_AllOnOff(uint32 data)
{

	uint8 i;
	
	for( i = 0; i < LED_NAME_MAX;i++ )
	{
		LED_Set((LED_NAME)i, 1000, 100, data, 0, 0, 0, -1);
	}
		
}

/*********************************************************************
* �� �� ��: LED_BatLedSwitching
* �������: enable
* ��������: ��
* �� �� ֵ: void 
* ��������: ת��ƿ���
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void  LED_BatLedSwitching(BOOL enable)
{
	LED_SetLedOnOff(LED_NAME_ORANGE_1,enable);
	LED_SetLedOnOff(LED_NAME_ORANGE_2,enable);
	LED_SetLedOnOff(LED_NAME_ORANGE_3,enable);
	LED_SetLedOnOff(LED_NAME_ORANGE_4,enable);
	LED_SetLedOnOff(LED_NAME_ORANGE_5,enable);

} 

/*********************************************************************
* �� �� ��: LED_AssistLedBlink
* �������: enable
* ��������: ��
* �� �� ֵ: void 
* ��������: ��λ�Ƹ�����˸ 
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void  LED_AssistLedBlink(uint32 param)
{
	uint8 i;
	
	for( i = 5+param; i > 4; i-- )
	{
		LED_Set((LED_NAME)i, 300, 50, 100, 0, 0, 0, -1);	
	}

}

/*********************************************************************
* �� �� ��: LED_AllLedBlink
* �������: enable
* ��������: ��
* �� �� ֵ: void 
* ��������: ���е���˸
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void  LED_AllLedBlink(uint32 param)
{
	uint8 i;
	
	for( i = 0; i < LED_NAME_MAX;i++ )
	{
		LED_Set((LED_NAME)i, param, 50, 100, 0, 0, 0, -1);	
	}

}

/*********************************************************************
* �� �� ��: LED_SetLedOnOff
* �������: enable
* ��������: ��
* �� �� ֵ: void 
* ��������: LED ����
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void LED_SetLedOnOff(LED_NAME seg, BOOL onoff)
{
	if ( onoff )
	{
		LED_Set((LED_NAME)seg, 1000, 100, LED_PWMBUFF, 0, 0, 0, -1);
	}
	else
	{
		LED_Set((LED_NAME)seg, 1000, 100,LED_PWMBUFF0, 0, 0, 0, -1);
	}
     
}
/*********************************************************************
* �� �� ��: LED_Assist_Max4
* �������: param
* ��������: ��
* �� �� ֵ: void 
* ��������: 
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void LED_Assist_Max4(uint32 param)
{
	switch (param)
	{
		case  ASSIST_0:
		
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;


		case  ASSIST_1:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;

		case  ASSIST_2:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;
			
		case  ASSIST_3:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;

		case  ASSIST_4:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,TRUE);
			
			break;
		
		case  ASSIST_P:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;
		
		default:
		
			break;
	
	}
}
/*********************************************************************
* �� �� ��: LED_Assist_Max5
* �������: param
* ��������: ��
* �� �� ֵ: void 
* ��������: 
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void LED_Assist_Max5(uint32 param)
{
	switch (param)
	{
		case  ASSIST_0:
			LED_SetLedOnOff(LED_NAME_BLUE_1,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			break;


		case  ASSIST_1:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			break;

		case  ASSIST_2:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;
			
		case  ASSIST_3:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;

		case  ASSIST_4:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;
			
		case  ASSIST_5:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,TRUE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;	
		
		case  ASSIST_P:
			
			LED_SetLedOnOff(LED_NAME_BLUE_1,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_2,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_BLUE_5,FALSE);
			
			break;
		
		default:
		
			break;
	
	}
}

/*********************************************************************
* �� �� ��: LED_AssistClass
* �������: param
* ��������: ��
* �� �� ֵ: void 
* ��������: 
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void LED_AssistClass(uint32 param)
{
	
	switch(PARAM_GetMaxAssist())
	{
		case ASSISTMAX_4:

			LED_Assist_Max4(param);

			break;

		case ASSISTMAX_5:
		
			LED_Assist_Max5(param);
			
			break;
	}		
}
/*********************************************************************
* �� �� ��: LED_BatteryPer
* �������: param
* ��������: ��
* �� �� ֵ: void 
* ��������: ������ʾ
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void  LED_BatteryPer(uint32 param)
{
	uint32 ulbuff = param;
	
	// �����ٷֱ�����
	if ( ulbuff >= 100)
	{
		ulbuff =100;
	}
	
	if ( ulbuff <= 5 )
	{
		ledCommonCB.Enblink2 = FALSE;
		if ( !ledCommonCB.Enblink)
		{
			ledCommonCB.Enblink = TRUE;	
			LED_Set(LED_NAME_ORANGE_1, 600, 50, LED_PWMBUFF, 0, 0, 0, -1);
			LED_SetLedOnOff(LED_NAME_ORANGE_2,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_5,FALSE);
		}
		
	}
	else if ( ulbuff <= 10 )
	{
		ledCommonCB.Enblink = FALSE;
		if ( !ledCommonCB.Enblink2)
		{
			ledCommonCB.Enblink2 = TRUE;	
			LED_Set(LED_NAME_ORANGE_1, 1200, 50, LED_PWMBUFF, 0, 0, 0, -1);
			LED_SetLedOnOff(LED_NAME_ORANGE_2,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_5,FALSE);

		}
	}
	else
	{		  	
		ledCommonCB.Enblink2 = FALSE;
		ledCommonCB.Enblink = FALSE;	
		if (ulbuff <= 20)
		{

			LED_SetLedOnOff(LED_NAME_ORANGE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_2,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_5,FALSE);

		}
		else if (ulbuff <= 40)
		{

			LED_SetLedOnOff(LED_NAME_ORANGE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_3,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_5,FALSE);
		}
		else if (ulbuff <= 60)
		{

			LED_SetLedOnOff(LED_NAME_ORANGE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_3,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_4,FALSE);
			LED_SetLedOnOff(LED_NAME_ORANGE_5,FALSE);


		}
		else if (ulbuff<= 80)
		{

			LED_SetLedOnOff(LED_NAME_ORANGE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_3,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_4,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_5,FALSE);

		}
		else if (ulbuff <= 100)
		{

			LED_SetLedOnOff(LED_NAME_ORANGE_1,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_2,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_3,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_4,TRUE);
			LED_SetLedOnOff(LED_NAME_ORANGE_5,TRUE);
		}
	}
}

/*********************************************************************
* �� �� ��: LED_RidingStateShow
* �������: refreshAll
* ��������: ��
* �� �� ֵ: void 
* ��������: ����״̬��ʾ
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void LED_RidingStateShow(BOOL  refreshAll)
{

	if ( refreshAll)
	{

	}
	// ��ת��Ʋ�����ʾ����                  
	/*if ( (!PARAM_GetLeftTurntSwitch()) && (!PARAM_GetRightTurnSwitch()))
	{
		LED_BatteryPer(PARAM_GetBatteryPercent());		
	}*/
	
	LED_BatteryPer(PARAM_GetBatteryPercent());	
	
	// ���Ƴ���������λ��
	if ( !PARAM_IsPushAssistOn())
	{
		LED_AssistClass(PARAM_GetAssistLevel());
	}
	
}


/*********************************************************************
* �� �� ��: LED_RidingStateShow
* �������: refreshAll
* ��������: ��
* �� �� ֵ: void 
* ��������: ����״̬��ʾ
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void LED_ErrocodeStateShow(BOOL  refreshAll)
{

	if ( refreshAll)
	{	
		if (0X30 == PARAM_GetErrorCode())
		{
			// 1s����
			LED_AllLedBlink(500);
		}
		else
		{
			// 400ms����
			LED_AllLedBlink(200);
		}
	}
}
/*********************************************************************
* �� �� ��: LED_RidingStateShow
* �������: refreshAll
* ��������: ��
* �� �� ֵ: void 
* ��������:	��������״̬
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��04��20��
*     �޸�����: �º��� 		   
**********************************************************************/
void  LED_UpradeStateShow(uint32 param)
{

	if (paramCB.runtime.blueUpgradePer == 1)
	{
		LED_Set(LED_NAME_ORANGE_1, 500, 50, LED_PWMBUFF, 0, 0, 0, -1);
	}
	else if (paramCB.runtime.blueUpgradePer == 20)
	{
		LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF, 0, 0, 0, -1);	
		LED_Set(LED_NAME_ORANGE_2, 500,  50, LED_PWMBUFF,  0, 0, 0, -1);
	}
	else if (paramCB.runtime.blueUpgradePer == 40)
	{
		LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);	
		LED_Set(LED_NAME_ORANGE_2, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
		LED_Set(LED_NAME_ORANGE_3, 500,  50,  LED_PWMBUFF,  0, 0, 0, -1);
	}
	else if (paramCB.runtime.blueUpgradePer == 60)
	{
		LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);	
		LED_Set(LED_NAME_ORANGE_2, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);	
		LED_Set(LED_NAME_ORANGE_3, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
		LED_Set(LED_NAME_ORANGE_4, 500, 50,  LED_PWMBUFF,  0, 0, 0, -1);
	}
	else if (paramCB.runtime.blueUpgradePer == 80)
	{
		LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);	
		LED_Set(LED_NAME_ORANGE_2, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);	
		LED_Set(LED_NAME_ORANGE_3, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
		LED_Set(LED_NAME_ORANGE_4, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
		LED_Set(LED_NAME_ORANGE_5, 500, 50, LED_PWMBUFF,  0, 0, 0, -1);
	}
	else if (paramCB.runtime.blueUpgradePer == 99)
	{
		LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);	
		LED_Set(LED_NAME_ORANGE_2, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);	
		LED_Set(LED_NAME_ORANGE_3, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
		LED_Set(LED_NAME_ORANGE_4, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
		LED_Set(LED_NAME_ORANGE_5, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
	}

}






















