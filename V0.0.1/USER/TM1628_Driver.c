
#include "TM1628_Driver.h"
#include "Delay.h" 
                           
/***********************************************************
�������ƣ� TM1628_Init
�������ܣ� ��ʼ��TM1628�˿�  
��ڲ����� ��
���ڲ����� ��
��    ע�� 
***********************************************************/
void TM1628_Init(void)
{
    GPIO_InitType GPIO_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA|RCC_APB2_PERIPH_GPIOB, ENABLE);
	
	GPIO_InitStruct(&GPIO_InitStructure);
    
    GPIO_InitStructure.Pin            = GPIO_PIN_0 |GPIO_PIN_2| GPIO_PIN_10;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    

    TM1628_DIO_LOW();
    TM1628_CLK_LOW();
    TM1628_STB_HIGH();  									 //�˿�ֵ��ʼ
    
    TM1628_WriteData(BITS_MODE_8, DIS_MODE_SET);  			 // 7λ10��
    TM1628_STB_HIGH();
} 

/***********************************************************
�������ƣ� TM1628_ReadByte
�������ܣ� ��TM1628��ȡһ���ֽ�  
��ڲ����� ��
���ڲ����� ���ض�ȡֵ
��    ע�� ��ʱ�ӵ���������TM16xx������
***********************************************************/
#ifdef KEY
UINT8 TM1628_ReadByte(void)
{
    
}
#endif

/***********************************************************
�������ƣ� TM1628_WriteByte
�������ܣ� ��TM1628д��һ���ֽ�  
��ڲ����� Data: Ҫд������
���ڲ����� ��
��    ע�� ��ʱ�ӵ�������ͨ��MCU��LED����IC����TM16xxд����
***********************************************************/
#if SEGMENT_TYPE == SEGMENT_FOR_8
void TM1628_WriteByte(UINT8 Data)
{
    UINT8 i;
    
    TM1628_STB_LOW();           //��֤"STB"Ϊ�͵�ƽ
    
    for (i = 0; i < 8; i++)
    {
        TM1628_CLK_LOW();       //�Ƚ�"CLK"����    
        TM1628_DELAY();
        
        if ((Data & 0x01) == 0x01)
        {
            TM1628_DIO_HIGH();         
        }
        else
        {
            TM1628_DIO_LOW();          
        }
        TM1628_DELAY(); 
        TM1628_CLK_HIGH();     //ʱ�������ؽ�����д��
        TM1628_DELAY();        //��ʱ����0.1us��֤�����ȶ�д��
        Data >>= 1;            //׼������һ��BIT
    }                          //����һ���ֽں��˳�ѭ��
}

/***********************************************************
�������ƣ� TM1628_DisplayFixAddr
�������ܣ� �ڲ���ַ�̶�ģʽ��ʾ����  
��ڲ����� Addr:  ��ʾ��ַ  
           Data�� Ҫ��ʾ������
���ڲ����� ��
��    ע�� 
***********************************************************/
void TM1628_DisplayFixAddr(UINT8 Addr, UINT8 Data)
{    
    TM1628_DIO_HIGH();                             //ͨѶ��ʼǰͨѶ�˿�ȫ����ʼ��Ϊ"1"
    TM1628_CLK_HIGH();
    TM1628_STB_HIGH();
    
    TM1628_WriteByte(WRITE_MODE_ADDR_FIX);      //дģʽ����,���ò��ù̶���ַ��ʽд��ʾ����
    TM1628_STB_HIGH();                             //����������������Ҫ��"STB"��"1"
    TM1628_WriteByte(START_ADDR + Addr);           //д��ʾ��ַ  
    //��ַ������"STB"����Ϊ"0"��������Ҫ��ʾ������
    TM1628_WriteByte(Data);                     //д��ʾ����
    
    TM1628_STB_HIGH();                             //��ʾ���ݺ󣬽�"STB"��"1"
    TM1628_WriteByte(DIS_CONTROL_ALL_ON);       //����ʾ������������_�������
    TM1628_STB_HIGH();                             //��ʾ����������"STB"��"1"
}

/***********************************************************
�������ƣ� TM1628_DisplayAutoAddr
�������ܣ� �ڲ���ַ�Զ�����ģʽ��ʾ����
��ڲ����� Addr:             ��ʾ����ʼ��ַ
           p_DataDisplayBuf��ָ����ʾ���ݵĵ�ַ
           Size:             ��ʾ���ݳ���
���ڲ����� 
��    ע�� 
***********************************************************/
void TM1628_DisplayAutoAddr(UINT8 Addr, UINT8 *p_DataDisplayBuf, UINT8 Size)
{
    UINT8 i; 
    
    TM1628_DIO_HIGH();                             //ͨѶ��ʼǰͨѶ�˿�ȫ����ʼ��Ϊ"1"
    TM1628_CLK_HIGH();
    TM1628_STB_HIGH();
    
    TM1628_WriteByte(WRITE_MODE_ADDR_INC);      //дģʽ����,���ò��õ�ַ�Զ���1��ʽд��ʾ����
    TM1628_STB_HIGH();                             //����������������Ҫ��"STB"��"1"
    TM1628_WriteByte(START_ADDR + Addr);           //��Ҫ��ʾ����ʼ��ַ
    //��ַ������"STB"����Ϊ"0"��������Ҫ��ʾ������
    for (i = 0; i < Size; i++)
    {
        TM1628_WriteByte(*p_DataDisplayBuf++);  //����������ʾ�����ݹ�����,"STB"һֱ����Ϊ"0"
    }
    
    TM1628_STB_HIGH();                             //�������е���ʾ���ݺ󣬽�"STB"��"1"
    //TM1628_WriteByte(DIS_CONTROL_ALL_ON);       //����ʾ������������_�������
    TM1628_WriteByte(DIS_CONTROL_ALL_ON); 
    
    TM1628_STB_HIGH();                             //��ʾ����������"STB"��"1"
}

#else
/***********************************************************
�������ƣ� TM1628_WriteData
�������ܣ� ��TM1628д��һ����  
��ڲ����� BitsMode: һ����д��λ��ѡ��     
           Data��    Ҫд������ 
���ڲ����� ��
��    ע�� ��ʱ�ӵ�������ͨ��MCU��LED����IC����TM16xxд����
           BitsMode-8λ��16λ
***********************************************************/
void TM1628_WriteData(UINT8 BitsMode, UINT16 Data)
{
    UINT8 i;
    
    TM1628_STB_LOW();           //��֤"STB"Ϊ�͵�ƽ
    
    for (i = 0; i < BitsMode; i++)
    {
        TM1628_CLK_LOW();       //�Ƚ�"CLK"����    
        TM1628_DELAY();
        
        if ((Data & 0x0001) == 0x0001)
        {
            TM1628_DIO_HIGH();         
        }
        else
        {
            TM1628_DIO_LOW();          
        }
        TM1628_DELAY(); 
        TM1628_CLK_HIGH();     //ʱ�������ؽ�����д��
        TM1628_DELAY();        //��ʱ����0.1us��֤�����ȶ�д��
        Data >>= 1;            //׼������һ��BIT
    }                          //����һ���ֽں��˳�ѭ��
}

/***********************************************************
�������ƣ� TM1628_DisplayFixAddr
�������ܣ� �ڲ���ַ�̶�ģʽ��ʾ����  
��ڲ����� Addr: ��ʾ��ַ  
           Data��Ҫ��ʾ������
���ڲ����� ��
��    ע�� 
***********************************************************/
void TM1628_DisplayFixAddr(UINT8 Addr, UINT16 Data)
{
    TM1628_DIO_HIGH();                                      //ͨѶ��ʼǰͨѶ�˿�ȫ����ʼ��Ϊ"1"
    TM1628_CLK_HIGH();
    TM1628_STB_HIGH();
    
    TM1628_WriteData(BITS_MODE_8, WRITE_MODE_ADDR_FIX);  //дģʽ����,���ò��ù̶���ַ��ʽд��ʾ����
    TM1628_STB_HIGH();                                      //����������������Ҫ��"STB"��"1"
    TM1628_WriteData(BITS_MODE_8, START_ADDR + Addr);       //д��ʾ��ַ  
    //��ַ������"STB"����Ϊ"0"��������Ҫ��ʾ������
    TM1628_WriteData(BITS_MODE_16, Data);                //д��ʾ����
    
    TM1628_STB_HIGH();                                      //��ʾ���ݺ󣬽�"STB"��"1"
    TM1628_WriteData(BITS_MODE_8, DIS_CONTROL_ALL_ON);   //����ʾ������������_�������
    TM1628_STB_HIGH();                                      //��ʾ����������"STB"��"1"
}

/***********************************************************
�������ƣ� TM1628_DisplayAutoAddr
�������ܣ� �ڲ���ַ�Զ�����ģʽ��ʾ����
��ڲ����� Addr:             ��ʾ����ʼ��ַ
           p_DataDisplayBuf��ָ����ʾ���ݵĵ�ַ
           Size:             ��ʾ���ݳ���
���ڲ����� 
��    ע�� д��8��λ����ʱ��ַ�Զ���1
***********************************************************/
void TM1628_DisplayAutoAddr(UINT8 Addr, UINT16 *p_DataDisplayBuf, UINT8 Size)
{
    UINT8 i; 
    
    TM1628_DIO_HIGH();                                           //ͨѶ��ʼǰͨѶ�˿�ȫ����ʼ��Ϊ"1"
    TM1628_CLK_HIGH();
    TM1628_STB_HIGH();
    
    TM1628_WriteData(BITS_MODE_8, WRITE_MODE_ADDR_INC);       //дģʽ����,���ò��õ�ַ�Զ���1��ʽд��ʾ����
    TM1628_STB_HIGH();                                           //����������������Ҫ��"STB"��"1"
    TM1628_WriteData(BITS_MODE_8, START_ADDR + Addr);            //��Ҫ��ʾ����ʼ��ַ
    //��ַ������"STB"����Ϊ"0"��������Ҫ��ʾ������
    for (i = 0; i < Size; i++)
    {
        TM1628_WriteData(BITS_MODE_16, *p_DataDisplayBuf++);  //����������ʾ�����ݹ�����,"STB"һֱ����Ϊ"0"
    }
    
    TM1628_STB_HIGH();                                           //�������е���ʾ���ݺ󣬽�"STB"��"1"
    //TM1628_WriteData(BITS_MODE_8, DIS_CONTROL_ALL_ON);        //����ʾ������������_������� 
    TM1628_WriteData(BITS_MODE_8, DIS_CONTROL_ALL_ON);
    TM1628_STB_HIGH();                                           //��ʾ����������"STB"��"1"
}


#endif 



