/********************************************************************************
  * �� �� ��: tempCapture.h
  * �� �� ��: ����            
  *******************************************************************************/
#ifndef __METERCONTROL_H__
#define __METERCONTROL_H__

/********************************* ͷ�ļ�************************************/
#include "common.h"
#include "timer.h"

// ������Ϣ����
#define AVOMETER_REV_LENGTH_MAX     16
// ���մ������
#define AVOMETER_REV_ERR_CNT_MAX     3
// �˲����ߴ�
#define AVOMETER_FILTER_BUF_MAX     3
// �˲������ϵ��
#define AVOMETER_FILTER_ERR_COE    0.1
// �Ƿ������ִ���
#define AVOMETER_REV_NUM_ERR_VAL    10000.0

/* �������ýṹ�� */
typedef struct
{
    uint32_t Clock;             /* ʱ�� */
    GPIO_Module* Port;             /* �˿� */
    uint16_t Pin;              /* ���� */
}AVOMETER_PT_PIN;

// ���ñ��ܼ�ö��
typedef enum{
			
	FUNTION_PIN_SELECT = 0x00,								
	FUNTION_PIN_RANGE,								
    FUNTION_PIN_RS232,	
    FUNTION_PIN_TX_EN,    
    FUNTION_PIN_MAX,
}AVOMETER_FUNTION_PIN_E;

// ���ñ���Ե�λö��
typedef enum{
			
	AVOMETER_GEARS_DCV = 0x01,								
	AVOMETER_GEARS_ACV,								
	AVOMETER_GEARS_DCMA,								
	AVOMETER_GEARS_ACMA,								
	AVOMETER_GEARS_OHM,						        
	AVOMETER_GEARS_DCA ,							    
	AVOMETER_GEARS_ACA ,							    
	AVOMETER_GEARS_MAX,
}AVOMETER_GEARS_CMD;


typedef enum
{
    AVOMETER_SYMBOL_CHAR = 0,
    AVOMETER_SYMBOL_P1,
    AVOMETER_SYMBOL_P2,
    AVOMETER_SYMBOL_P3,
    AVOMETER_SYMBOL_DC,
    AVOMETER_SYMBOL_AC,
    AVOMETER_SYMBOL_OHM,
    AVOMETER_SYMBOL_K_OHM,
    AVOMETER_SYMBOL_M_OHM,
	AVOMETER_SYMBOL_MILLI,
    AVOMETER_SYMBOL_MAX,
}AVOMETER_SYMBOL_E;


// ���ñ���Ե�λö��
typedef enum{
			
AVOMETER_STATE_NULL = 0x00,								
AVOMETER_STATE_PT,				// ����PT				
AVOMETER_STATE_CHECK_CHAR,		// ������ͼ���Ƿ���Ҫ��secect				
AVOMETER_STATE_GET_RES,			// ��ȡ���										        						    
AVOMETER_STATE_MAX,
}AVOMETER_STATE_E;


/********************************* �ṹ��***************************/

typedef struct
{
    uint8 length;
    float measureRes[AVOMETER_FILTER_BUF_MAX];
}METER_FILTER_CB;	


typedef struct
{
	uint32 canUse;
	uint8  chl;
	uint8  mesureType; 				
}SEND_RES_PARA;	




typedef struct
{
		uint8 dataDir;
    BOOL revUpdata;
    AVOMETER_STATE_E state;
    AVOMETER_GEARS_CMD gearsMode;
		uint16 revLength;
    uint8 RevBuf[AVOMETER_REV_LENGTH_MAX];
		float  revNewNumRes;		
		float  reportRes;	
    METER_FILTER_CB filter;
	
    SEND_RES_PARA resPara;	
}METER_CONTROL_CB;			





extern METER_CONTROL_CB meterControlCB;						// ��ͷ�ɼ�

/********************************* �����ⲿ����***************************/
// ���ñ��ʼ��
void Avoment_Init(void);
// �л���λ
void Avoment_StitchGear(AVOMETER_GEARS_CMD gear);
// ���ñ����ݴ���
void Avoment_Process(void);
// ��������ϱ�
void Avoment_ReportMeasureResultACK(uint32 canUse);
#endif
