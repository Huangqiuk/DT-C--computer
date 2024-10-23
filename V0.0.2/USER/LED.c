#include "led.h"
#include "pwmled.h"
#include "timer.h"
#include "param.h"

extern LEDCOMMON_CB ledCommonCB;

/*********************************************************************
* 函 数 名: LED_AllOnOff
* 函数入参: data
* 函数出参: 无
* 返 回 值: void 
* 功能描述: LED 全亮灭控制
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
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
* 函 数 名: LED_BatLedSwitching
* 函数入参: enable
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 转向灯控制
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
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
* 函 数 名: LED_AssistLedBlink
* 函数入参: enable
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 档位灯个数闪烁 
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
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
* 函 数 名: LED_AllLedBlink
* 函数入参: enable
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 所有灯闪烁
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
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
* 函 数 名: LED_SetLedOnOff
* 函数入参: enable
* 函数出参: 无
* 返 回 值: void 
* 功能描述: LED 亮灭
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
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
* 函 数 名: LED_Assist_Max4
* 函数入参: param
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
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
* 函 数 名: LED_Assist_Max5
* 函数入参: param
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
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
* 函 数 名: LED_AssistClass
* 函数入参: param
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
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
* 函 数 名: LED_BatteryPer
* 函数入参: param
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 电量显示
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
**********************************************************************/
void  LED_BatteryPer(uint32 param)
{
	uint32 ulbuff = param;
	
	// 电量百分比限制
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
* 函 数 名: LED_RidingStateShow
* 函数入参: refreshAll
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 骑行状态显示
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
**********************************************************************/
void LED_RidingStateShow(BOOL  refreshAll)
{

	if ( refreshAll)
	{

	}
	// 无转向灯操作显示电量                  
	/*if ( (!PARAM_GetLeftTurntSwitch()) && (!PARAM_GetRightTurnSwitch()))
	{
		LED_BatteryPer(PARAM_GetBatteryPercent());		
	}*/
	
	LED_BatteryPer(PARAM_GetBatteryPercent());	
	
	// 无推车助力亮档位灯
	if ( !PARAM_IsPushAssistOn())
	{
		LED_AssistClass(PARAM_GetAssistLevel());
	}
	
}


/*********************************************************************
* 函 数 名: LED_RidingStateShow
* 函数入参: refreshAll
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 骑行状态显示
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
**********************************************************************/
void LED_ErrocodeStateShow(BOOL  refreshAll)
{

	if ( refreshAll)
	{	
		if (0X30 == PARAM_GetErrorCode())
		{
			// 1s周期
			LED_AllLedBlink(500);
		}
		else
		{
			// 400ms周期
			LED_AllLedBlink(200);
		}
	}
}
/*********************************************************************
* 函 数 名: LED_RidingStateShow
* 函数入参: refreshAll
* 函数出参: 无
* 返 回 值: void 
* 功能描述:	蓝牙升级状态
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年04月20日
*     修改描述: 新函数 		   
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






















