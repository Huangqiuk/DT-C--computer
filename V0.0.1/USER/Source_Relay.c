#include "Source_Relay.h"

RELAY_CHL OHMrelayChl[RELAY_OHM_MAX]={
                                     {GPIO_PIN_9,GPIOB,RCC_APB2_PERIPH_GPIOB},{GPIO_PIN_8,GPIOB,RCC_APB2_PERIPH_GPIOB},{GPIO_PIN_8,GPIOA,RCC_APB2_PERIPH_GPIOA},{GPIO_PIN_15,GPIOB,RCC_APB2_PERIPH_GPIOB},
									 {GPIO_PIN_13,GPIOC,RCC_APB2_PERIPH_GPIOC},
                                   };

RELAY_CHL VINrelayChl[RELAY_VIN_MAX]={
                                     {GPIO_PIN_15,GPIOA,RCC_APB2_PERIPH_GPIOA},{GPIO_PIN_3,GPIOB,RCC_APB2_PERIPH_GPIOB},{GPIO_PIN_4,GPIOB,RCC_APB2_PERIPH_GPIOB},{GPIO_PIN_5,GPIOB,RCC_APB2_PERIPH_GPIOB},
									 {GPIO_PIN_6,GPIOB,RCC_APB2_PERIPH_GPIOB}, {GPIO_PIN_7,GPIOB,RCC_APB2_PERIPH_GPIOB},
                                   };								   

// �̵������ƽų�ʼ��
void Relay_HwInit(void)
{
	GPIO_InitType GPIO_InitStructure;
	uint8 i = 0;    
	GPIO_InitStruct(&GPIO_InitStructure);
	RTC_TamperCmd(RTC_TAMPER_1, DISABLE);
    // ŷķͨ���̵�����ʼ��
    for(i = 0; i < sizeof(OHMrelayChl)/sizeof(OHMrelayChl[0]); i++)
    {
        RCC_EnableAPB2PeriphClk(OHMrelayChl[i].Clock, ENABLE);
        GPIO_InitStructure.Pin = OHMrelayChl[i].Pin;
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Down;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // �������ģʽ
        GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;   // 50MHz�ٶ�
        GPIO_InitPeripheral(OHMrelayChl[i].GPIOx, &GPIO_InitStructure); 
    }
	
	  // ��ѹͨ���̵�����ʼ��
    for(i = 0; i < sizeof(VINrelayChl)/sizeof(VINrelayChl[0]); i++)
    {
        RCC_EnableAPB2PeriphClk(VINrelayChl[i].Clock, ENABLE);
        GPIO_InitStructure.Pin = VINrelayChl[i].Pin;
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Down;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // �������ģʽ
        GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;   // 50MHz�ٶ�
        GPIO_InitPeripheral(VINrelayChl[i].GPIOx, &GPIO_InitStructure); 
    }
	
	//COM��    
	GPIO_InitStructure.Pin = COM_PIN;
	GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Down;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // �������ģʽ
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;   // 50MHz�ٶ�
	GPIO_InitPeripheral(COM_PORT, &GPIO_InitStructure); 
	
	//�����˺���
	COM_CHL_OFF();
	
}

/*******************************************************************************
* Function Name  : OHM_RelayCtrl(OHM_RELAY_CHL relayCh,RELAY_STA OnOff)
* Description    : ŷķͨ��ѡ��
* Input          : relayCh ��ŷķ����ͨ����   OnOff: OFF����ON
* Output         : None
* Return         : None
*******************************************************************************/
void OHM_RelayCtrl(OHM_RELAY_CHL relayCh,RELAY_STA OnOff)
{
  if((relayCh>=RELAY_OHM_MAX)||(OnOff>1))
  {
     return;
  }
  GPIO_WriteBit(OHMrelayChl[relayCh].GPIOx, OHMrelayChl[relayCh].Pin,(Bit_OperateType) OnOff);

}


/*******************************************************************************
* Function Name  : VIN_RelayCtrl(VIN_RELAY_CHL relayCh,RELAY_STA OnOff)
* Description    : ��ѹͨ��ѡ��
* Input          : relayCh ����ѹ����ͨ����   OnOff: OFF����ON
* Output         : None
* Return         : None
*******************************************************************************/
void VIN_RelayCtrl(VIN_RELAY_CHL relayCh,RELAY_STA OnOff)
{
  if((relayCh>=RELAY_VIN_MAX)||(OnOff>1))
  {
     return;
  }
  GPIO_WriteBit(VINrelayChl[relayCh].GPIOx, VINrelayChl[relayCh].Pin, (Bit_OperateType) OnOff);
}

void RelayReset(void )
{
    for(uint8 i = 0; i < sizeof(OHMrelayChl)/sizeof(OHMrelayChl[0]); i++)
    {
        GPIO_WriteBit(OHMrelayChl[i].GPIOx, OHMrelayChl[i].Pin,(Bit_OperateType) OFF);
    }	
	  // ��ѹͨ���̵�����ʼ��
    for(uint8 i = 0; i < sizeof(VINrelayChl)/sizeof(VINrelayChl[0]); i++)
    {
        GPIO_WriteBit(VINrelayChl[i].GPIOx, VINrelayChl[i].Pin, (Bit_OperateType) OFF);
    }
	COM_CHL_OFF(); 	
}


