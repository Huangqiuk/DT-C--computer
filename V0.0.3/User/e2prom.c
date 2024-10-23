#include "common.h" 
#include "e2prom.h"
#include "Watchdog.h"

E2PROM_CB e2promCB;


// ���������������������ڲ�������������������������������������
void E2PROM_Delayus(volatile uint8 i);
void E2PROM_Delayms(volatile uint8 i);

// I2C��ʼ��
void E2PROM_I2C_Init(void);
// I2C��ʼ�ź�
void E2PROM_I2C_Start(void);
// I2Cֹͣ�ź�
void E2PROM_I2C_Stop(void);
// I2Cдһ���ֽ�
void E2PROM_I2C_WriteByte(uint8 Value);
// I2C��ȡһ���ֽ�
uint8 E2PROM_I2C_ReadByte(uint8 AckOrNot);

// I2C��ҳ��д����
void E2PROM_WritePageBuf(uint16 writeAddr, uint8 *pBuf, uint16 writeNum);

#if E2PROM_UNIT_TEST
static uint8 TempBuf[300] = {0};
uint32 data;
void E2PROM_UnitTest(void)
{
	uint16 i;
	volatile uint8 dataBuf[8];
	volatile uint8 arrtemp[4];

	E2PROM_SCL_HIGH();
	E2PROM_SDA_HIGH();
	E2PROM_SCL_DIR_OUT();
	E2PROM_SDA_DIR_OUT();

	E2PROM_ServiceInit();

	E2PROM_ServiceWriteByID(E2PROM_ID_WHELL_SIZE1, 0x11223344);
	E2PROM_ServiceWriteByID(E2PROM_ID_WHELL_SIZE2, 0x11111111);
	E2PROM_ServiceWriteByID(E2PROM_ID_WHELL_SIZE3, 0x12345678);

	data = E2PROM_ServiceReadByID(E2PROM_ID_WHELL_SIZE1);
	data = E2PROM_ServiceReadByID(E2PROM_ID_WHELL_SIZE2);
	data = E2PROM_ServiceReadByID(E2PROM_ID_WHELL_SIZE3);


	for (i = 0; i < sizeof(TempBuf); i++)
	{
		TempBuf[i] = i;
	}

	E2PROM_WriteByteArray(0, TempBuf, sizeof(TempBuf));

	for (i = 0; i < sizeof(TempBuf); i++)
	{
		TempBuf[i] = 0;
	}

	E2PROM_ReadByteArray(0, TempBuf, sizeof(TempBuf));
}
#endif

/*********************************************************
* ����������
*********************************************************/
// ����ʱ
void E2PROM_Delayus(volatile uint8 i)
{	 
	// �ں�72MHz��1us
	while (i--) 
	{
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();	// 10
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();	// 10
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();	// 10
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();	// 6
	}
}

// ����ʱ
void E2PROM_Delayms(volatile uint8 i)
{
	volatile uint16 x;
	volatile uint8 y;

	while (i--)
	{
		x = 700; 
		while (x--)
		{
			y = 10;
			while (y--)
			{
				NOP(); 
			} 
		}

		// ���Ź�����
		WDT_Clear();
	}
}


////////////////////////////////////////////////////////////////////////////////
// I2C��ʼ��
void E2PROM_I2C_Init(void)
{
	// ��ʱ��
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_5);
	gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_6);
	
	E2PROM_SCL_HIGH();
	E2PROM_SDA_HIGH();
	E2PROM_SCL_DIR_OUT();
	E2PROM_SDA_DIR_OUT();
}

// I2C��ʼ�ź�
void E2PROM_I2C_Start(void)
{
	E2PROM_SDA_HIGH();
	E2PROM_SCL_HIGH();
	E2PROM_DELAY();
	
	E2PROM_SDA_LOW();
	E2PROM_DELAY();
	
	E2PROM_SCL_LOW();
	E2PROM_DELAY();
}

// I2Cֹͣ�ź�
void E2PROM_I2C_Stop(void)
{
	E2PROM_SCL_LOW();
	E2PROM_SDA_LOW();
	E2PROM_DELAY();
	
	E2PROM_SCL_HIGH();
	E2PROM_DELAY();
	
	E2PROM_SDA_HIGH();
	E2PROM_DELAY();
}

// I2Cдһ���ֽ�
void E2PROM_I2C_WriteByte(uint8 Value)
{
	uint8 i;
	
	for (i=0; i<8; i++)
	{
		if(0x80 == (Value & 0x80))
		{
			E2PROM_SDA_HIGH();
		}
		else
		{
			E2PROM_SDA_LOW();
		}
		E2PROM_DELAY();
		
		E2PROM_SCL_HIGH();
		E2PROM_DELAY();
		
		E2PROM_SCL_LOW();
		E2PROM_DELAY(); 
	 
		Value <<= 1;
	}
	
	E2PROM_SDA_DIR_IN();				  // ׼������ACK
	
	E2PROM_SCL_HIGH();	 
	E2PROM_DELAY();  
	
	E2PROM_SCL_LOW();		  
	E2PROM_DELAY();
	
	E2PROM_SDA_DIR_OUT();				  
}									 

// I2C��ȡһ���ֽ�
uint8 E2PROM_I2C_ReadByte(uint8 AckOrNot)
{
	uint8 i;
	uint8 RetValue = 0;
	
	E2PROM_SDA_DIR_IN();					// �˿���Ϊ���뷽ʽ�Զ�ȡ���� 
	 
	for (i=0; i<8; i++)
	{
		E2PROM_SCL_HIGH();					// ʱ��Ϊ��,��ȡλ
		E2PROM_DELAY();
		
		RetValue <<= 1;
		if (E2PROM_SDA_GET())
		{
			RetValue |= 0x01;
		}
		
		E2PROM_SCL_LOW();					// ʱ��Ϊ��,24C02�ı�λ����
		E2PROM_DELAY();    
	}
	
	E2PROM_SDA_DIR_OUT();
	E2PROM_SDA_SET(AckOrNot);				// Ӧ�����
	E2PROM_DELAY();
	
	E2PROM_SCL_HIGH();
	E2PROM_DELAY();
	
	E2PROM_SCL_LOW();
	E2PROM_DELAY();  
	
	return RetValue;
}

// I2C��ҳ��д����
void E2PROM_WritePageBuf(uint16 writeAddr, uint8 *pBuf, uint16 writeNum)
{
	uint16	i = 0;
	uint8 addrH = (uint8)(writeAddr >> 8);	 // ȡ��8λ��ַ
	uint8 addrL = (uint8)writeAddr; 		 // ȡ��8λ��ַ

	// �жϴ洢��ַ�Ƿ񳬹�8λ
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte((addrH<<1) | EEPROM_WRCOMMAND);	// �豸��ַҲ���ֽڵ�ַ�ĸ�8λ
	E2PROM_I2C_WriteByte(addrL);
   
	while (i < writeNum)
	{
		E2PROM_I2C_WriteByte(pBuf[i++]);
	}
	E2PROM_I2C_Stop();

	// ��ʱms����ȴ�buf����д��EEPROM
	E2PROM_DELAY_MS();

#else

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND);
	E2PROM_I2C_WriteByte(addrH);
	E2PROM_I2C_WriteByte(addrL);
	
	while (i < writeNum)
	{
		E2PROM_I2C_WriteByte(pBuf[i++]);
	}
	E2PROM_I2C_Stop();

	// ��ʱms����ȴ�buf����д��EEPROM
	E2PROM_DELAY_MS();

#endif
}


// E2PROM��ʼ��
void E2PROM_Init(void)
{
	// Ӳ����ʼ��
	E2PROM_I2C_Init();

#if E2PROM_UNIT_TEST
	E2PROM_UnitTest();
#endif

	//E2PROM_ServiceInit();
}

// �����ݣ�������ַ�ڣ����迼��ҳ����
BOOL E2PROM_ReadByteArray(uint16 ReadAddr, uint8* pBuf, uint16 readNum)
{
	uint16 i = 0;
	uint8 addrH = (uint8)(ReadAddr >> 8);	// ȡ��8λ��ַ
	uint8 addrL = (uint8)ReadAddr;			// ȡ��8λ��ַ

	// ��ַ���ܳ����ܿռ��С
	if (ReadAddr > EE_TOTAL_SIZE)
	{
		return FALSE;
	}

	// �������������ܿռ��С
	if ((ReadAddr + readNum) > EE_TOTAL_SIZE) 
	{
		return FALSE;
	}

	// ��ָ��
	if (NULL == pBuf)
	{
		return FALSE;
	}

	// �жϴ洢��ַ�Ƿ񳬹�8λ
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(((addrH<<1)&0x0F) | EEPROM_WRCOMMAND);    // �豸��ַҲ���ֽڵ�ַ�ĸ�8λ��дָ��
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// ��ָ��
	while(i < (readNum-1))
	{
		pBuf[i++] = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	}
	pBuf[i] = E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();
	
#else

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND); // дָ��
	E2PROM_I2C_WriteByte(addrH);
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// ��ָ��
	while(i < (readNum-1))
	{
		pBuf[i++] = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	}
	pBuf[i] = E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

#endif
	return TRUE;
}

// д���ݣ�������ַ�ڣ����迼��ҳ����
BOOL E2PROM_WriteByteArray(uint16 writeAddr, uint8* pBuf, uint16 writeNum)
{
	uint16 pageNum = 0;
	uint16 singleNum = 0;
	uint16 part = 0;

	// ��ַ���ܳ����ܿռ��С
	if (writeAddr > EE_TOTAL_SIZE)
	{
		return FALSE;
	}

	// д�����������ܿռ��С
	if ((writeAddr + writeNum) > EE_TOTAL_SIZE) 
	{
		return FALSE;
	}

	// ��ָ��
	if (NULL == pBuf)
	{
		return FALSE;
	}

	// �ж���ʼ��ַ���ڵ�ҳʣ���д�ֽڸ���
	part = writeAddr / EE_PAGE_SIZE;			// ��ʼ��ַ���ڵ�ҳ
	part = EE_PAGE_SIZE*(part+1) - writeAddr;	// ��ҳʣ���д���ֽڸ���

	// �ж��Ƿ���Ҫ��ҳд��
	if (part >= writeNum)	// д������ݸ���С�ڿ�ҳʣ��ĸ�����ֱ��д��
	{
		E2PROM_WritePageBuf(writeAddr, pBuf, writeNum);
	}
	else					// ��ǰҳ�������㣬��Ҫ��ҳ�㷨д��
	{
		// 1.�Ȱѵ�ǰ��ַ���ڵ�ҳ�����
		E2PROM_WritePageBuf(writeAddr, pBuf, part);

		// 2.����ʣ��ҳ��������һҳ���ֽ���
		pageNum = (writeNum - part) / EE_PAGE_SIZE; 
		singleNum = (writeNum - part) % EE_PAGE_SIZE; 

		// 3.����ʣ���ܸ�������һҳ��ʼ��ַ������ָ��
		writeNum -= part;	// ʣ��д�����
		writeAddr += part;	// ��һҳд���ַ
		pBuf += part;		// ��һҳд��ָ��

		// 4.�������������ռҳ����������д��ҳ��
		while (pageNum--)
		{ 
			E2PROM_WritePageBuf(writeAddr, pBuf, EE_PAGE_SIZE);
			
			// ÿд��һҳ����ַ����һҳ������ָ��Ҳ����һҳ��ַ
			writeAddr += EE_PAGE_SIZE;
			pBuf += EE_PAGE_SIZE; 
		}

		// 5.����ҳ��д��󳬳��Ĳ���һҳ������������
		if (0 != singleNum)
		{
			E2PROM_WritePageBuf(writeAddr, pBuf, singleNum);
		}
	}
  
	return TRUE;
}

// ��ȡһ���ֽ�
uint8 E2PROM_ReadByte(uint16 ReadAddr)
{
	uint8 data;
	uint8 addrH = (uint8)(ReadAddr >> 8);	// ȡ��8λ��ַ
	uint8 addrL = (uint8)ReadAddr;			// ȡ��8λ��ַ

	// �жϴ洢��ַ�Ƿ񳬹�8λ
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(((addrH<<1)&0x0F) | EEPROM_WRCOMMAND);    // �豸��ַҲ���ֽڵ�ַ�ĸ�8λ��дָ��
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// ��ָ��

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;
	
#else

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND); // дָ��
	E2PROM_I2C_WriteByte(addrH);
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// ��ָ��

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;

#endif
}

// ��ȡһ��˫�ֽ���ֵ
uint16 E2PROM_ReadInt(uint16 ReadAddr)
{
	uint16 data = 0;
	uint8 addrH = (uint8)(ReadAddr >> 8);	// ȡ��8λ��ַ
	uint8 addrL = (uint8)ReadAddr;			// ȡ��8λ��ַ

	// �жϴ洢��ַ�Ƿ񳬹�8λ
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(((addrH<<1)&0x0F) | EEPROM_WRCOMMAND);    // �豸��ַҲ���ֽڵ�ַ�ĸ�8λ��дָ��
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// ��ָ��

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;
	
#else

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND); // дָ��
	E2PROM_I2C_WriteByte(addrH);
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// ��ָ��

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;

#endif
}

// ��ȡһ�����ֽ���ֵ
uint32 E2PROM_ReadWord(uint16 ReadAddr)
{
	uint32 data = 0;
	uint8 addrH = (uint8)(ReadAddr >> 8);	// ȡ��8λ��ַ
	uint8 addrL = (uint8)ReadAddr;			// ȡ��8λ��ַ

	// �жϴ洢��ַ�Ƿ񳬹�8λ
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(((addrH<<1)&0x0F) | EEPROM_WRCOMMAND);    // �豸��ַҲ���ֽڵ�ַ�ĸ�8λ��дָ��
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// ��ָ��

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;
	
#else

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND); // дָ��
	E2PROM_I2C_WriteByte(addrH);
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// ��ָ��

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;

#endif
}

// д��һ���ֽ�
void E2PROM_WriteByte(uint16 writeAddr, uint8 data)
{
	uint8 dataCheck = ~data;
	uint8 retry = 5;
	uint8 addrH = (uint8)(writeAddr >> 8);	 // ȡ��8λ��ַ
	uint8 addrL = (uint8)writeAddr; 		 // ȡ��8λ��ַ

	// �жϴ洢��ַ�Ƿ񳬹�8λ
#if EEPROM_TYPE <= EEPROM_24C16
	while((dataCheck != data)&&(retry))
	{
		retry--;

		E2PROM_I2C_Start();
		E2PROM_I2C_WriteByte((addrH<<1) | EEPROM_WRCOMMAND);	// �豸��ַҲ���ֽڵ�ַ�ĸ�8λ
		E2PROM_I2C_WriteByte(addrL);
	   
		E2PROM_I2C_WriteByte(data);
		
		E2PROM_I2C_Stop();

		// ��ʱms����ȴ�buf����д��EEPROM
		E2PROM_DELAY_MS();

		dataCheck = E2PROM_ReadByte(writeAddr);
	}

#else
	while((dataCheck != data)&&(retry))
	{
		retry--;

		E2PROM_I2C_Start();
		E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND);
		E2PROM_I2C_WriteByte(addrH);
		E2PROM_I2C_WriteByte(addrL);
		
		E2PROM_I2C_WriteByte(data);

		E2PROM_I2C_Stop();

		// ��ʱms����ȴ�buf����д��EEPROM
		E2PROM_DELAY_MS();

		dataCheck = E2PROM_ReadByte(writeAddr);
	}

#endif
}

// д��һ��˫�ֽ���ֵ
void E2PROM_WriteInt(uint16 writeAddr, uint16 data)
{
	uint8 addrH;
	uint8 addrL;
	uint8 localData;
	uint8 i = 2;
	uint8 page = writeAddr/EE_PAGE_SIZE;

	// �жϴ洢��ַ�Ƿ񳬹�8λ
#if EEPROM_TYPE <= EEPROM_24C16
	while(i)
	{
		addrH = (uint8)(writeAddr >> 8);	// ȡ��8λ��ַ
		addrL = (uint8)writeAddr; 		 	// ȡ��8λ��ַ
		localData = data >> (8*(i-1));
	
		E2PROM_I2C_Start();
		E2PROM_I2C_WriteByte((addrH<<1) | EEPROM_WRCOMMAND);	// �豸��ַҲ���ֽڵ�ַ�ĸ�8λ
		E2PROM_I2C_WriteByte(addrL);

		while(i)
		{
			E2PROM_I2C_WriteByte(localData);
			writeAddr++;
			i--;
			localData = data >> (8*(i-1));

			// ҳ�л������¿�ʼ
			if(page != writeAddr/EE_PAGE_SIZE)
			{
				break;
			}
		}
		
		E2PROM_I2C_Stop();

		// ��ʱms����ȴ�buf����д��EEPROM
		E2PROM_DELAY_MS();
	}

#else
	while(i)
	{
		addrH = (uint8)(writeAddr >> 8);	// ȡ��8λ��ַ
		addrL = (uint8)writeAddr; 		 	// ȡ��8λ��ַ
		localData = data >> (8*(i-1));
	
		E2PROM_I2C_Start();
		E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND);
		E2PROM_I2C_WriteByte(addrH);
		E2PROM_I2C_WriteByte(addrL);

		while(i)
		{
			E2PROM_I2C_WriteByte(localData);
			writeAddr++;
			i--;
			
			if(i != 0)
			{
				localData = data >> (8*(i-1));
			}

			// ҳ�л������¿�ʼ
			if(page != writeAddr/EE_PAGE_SIZE)
			{
				break;
			}
		}

		E2PROM_I2C_Stop();

		// ��ʱms����ȴ�buf����д��EEPROM
		E2PROM_DELAY_MS();
	}

#endif
}

// д��һ�����ֽ���ֵ
void E2PROM_WriteWord(uint16 writeAddr, uint32 data)
{
	uint8 addrH;
	uint8 addrL;
	uint8 localData;
	uint8 i = 4;
	uint8 page = writeAddr/EE_PAGE_SIZE;

	// �жϴ洢��ַ�Ƿ񳬹�8λ
#if EEPROM_TYPE <= EEPROM_24C16
	while(i)
	{
		addrH = (uint8)(writeAddr >> 8);	// ȡ��8λ��ַ
		addrL = (uint8)writeAddr; 		 	// ȡ��8λ��ַ
		localData = data >> (8*(i-1));
	
		E2PROM_I2C_Start();
		E2PROM_I2C_WriteByte((addrH<<1) | EEPROM_WRCOMMAND);	// �豸��ַҲ���ֽڵ�ַ�ĸ�8λ
		E2PROM_I2C_WriteByte(addrL);

		while(i)
		{
			E2PROM_I2C_WriteByte(localData);
			writeAddr++;
			i--;

			if(i != 0)
			{
				localData = data >> (8*(i-1));
			}

			// ҳ�л������¿�ʼ
			if(page != writeAddr/EE_PAGE_SIZE)
			{
				break;
			}
		}
		
		E2PROM_I2C_Stop();

		// ��ʱms����ȴ�buf����д��EEPROM
		E2PROM_DELAY_MS();
	}

#else
	while(i)
	{
		addrH = (uint8)(writeAddr >> 8);	// ȡ��8λ��ַ
		addrL = (uint8)writeAddr; 		 	// ȡ��8λ��ַ
		localData = data >> (8*(i-1));
	
		E2PROM_I2C_Start();
		E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND);
		E2PROM_I2C_WriteByte(addrH);
		E2PROM_I2C_WriteByte(addrL);

		while(i)
		{
			E2PROM_I2C_WriteByte(localData);
			writeAddr++;
			i--;

			if(i != 0)
			{
				localData = data >> (8*(i-1));
			}

			// ҳ�л������¿�ʼ
			if(page != writeAddr/EE_PAGE_SIZE)
			{
				break;
			}
		}

		E2PROM_I2C_Stop();

		// ��ʱms����ȴ�buf����д��EEPROM
		E2PROM_DELAY_MS();
	}

#endif
}


//==================================================================
// �������ݳ�ʼ��
void E2PROM_ServiceInit(void)
{
	uint8 i;

	for (i = 0; i < E2PROM_ID_MAX; i++)
	{
		e2promCB.record[i].addr = 0;
		e2promCB.record[i].length = 0;
		e2promCB.record[i].writeMode = E2PROM_ACCESS_MODE_NOT_OFTEN;

		e2promCB.record[i].isExist = FALSE;
	}
	e2promCB.size = 0;

	// ע��ϵͳ����
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE1, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE2, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE3, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE4, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE5, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE6, E2PROM_ACCESS_MODE_NOT_OFTEN);
}

// ҵ����ȡ��ӦID��ȡ�ĵ�ַ
uint16 E2PROM_ServiceGetIDAddr(E2PROM_ID id)
{	
	return e2promCB.record[id].addr;
}

// ҵ����ȡʹ�õ��ܿռ��С
uint32 E2PROM_ServiceGetRomSize(void)
{
	return e2promCB.size;
}

// ע����ӦID�ĳ����Լ��Ƿ���ҪƵ����дģʽ
void E2PROM_ServiceRegister(E2PROM_ID id, uint16 length, E2PROM_ACCESS_MODE_E mode)
{
	uint8 i;
	uint8 indexBuf[2];
	uint16 addrBuf;
	uint8 dataBuf[sizeof(e2promCB.record[id].count)+1];
	uint8 sumCheck;

	// �����Ϸ��Լ���
	if ((id >= E2PROM_ID_MAX) || (length > E2PROM_DISPOSABIE_DATA_LENGTH_MAX))
	{
		return;
	}

	// ����E2PROM�ռ��С
	if ((E2PROM_SYSTEM_PARAM_ADDR_OFFSET + e2promCB.size) > EE_TOTAL_SIZE)
	{
		return;
	} 
	
	// ��Ҫд�����ݵ�һ����¼���׵�ַ
	e2promCB.record[id].addr = E2PROM_SYSTEM_PARAM_ADDR_OFFSET + e2promCB.size;

	// �Ƿ���ҪƵ��д��
	e2promCB.record[id].writeMode = mode;

	// ��ҪƵ��д��
	if (mode)
	{
		// ��ҪƵ��д������ݽ�Ҫ����һ�������Լ���¼д�����ݴ�����������ܿռ�
		e2promCB.size += (sizeof(e2promCB.record[id].index) + (sizeof(e2promCB.record[id].count) + length) * E2PROM_MALLOC_SAVE_SPACE_COUNT);

		// ����E2PROM�ռ��С
		if ((E2PROM_SYSTEM_PARAM_ADDR_OFFSET + e2promCB.size) > EE_TOTAL_SIZE)
		{
			e2promCB.record[id].isExist = FALSE;
			return;
		}		  

		// ���µ�ǰID��Ӧ�����ݳ���
		e2promCB.record[id].length = length; 

		// ����ֵ
		E2PROM_ReadByteArray(e2promCB.record[id].addr, indexBuf, 2);
		
		// �ж��Ƿ���Ҫ�������ó�ʼֵ
		if (~(indexBuf[0] + E2PROM_LOCKED_KEY) == indexBuf[1])
		{
			e2promCB.record[id].index = indexBuf[0];
		}
		else
		{
			// ��һ���ϵ縳Ĭ��ֵ
			e2promCB.record[id].index = 0;
			indexBuf[0] = e2promCB.record[id].index;
			indexBuf[1] = ~(e2promCB.record[id].index + E2PROM_LOCKED_KEY);
			E2PROM_WriteByteArray(e2promCB.record[id].addr, indexBuf, 2);  // ��������ֵ��������������У��ֵ
		}

		// ��ȡ��д����ֵ
		addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
		E2PROM_ReadByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);

		// У��ֵ
		sumCheck = 0;
		for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
		{
			sumCheck += dataBuf[i];
		}
		sumCheck = ~(sumCheck + E2PROM_LOCKED_KEY);
		
		// �ж��Ƿ���Ҫ�������ó�ʼֵ
		if (sumCheck == dataBuf[i])
		{
			for (i = 0, e2promCB.record[id].count = 0; i < sizeof(e2promCB.record[id].count); i++)
			{
				e2promCB.record[id].count <<= 8;
				e2promCB.record[id].count |= dataBuf[i];
			}
		}
		else
		{
			// ��һ���ϵ縳Ĭ��ֵ
			e2promCB.record[id].count = 0;
			
			// �������ֵ
			sumCheck = 0;
			for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
			{
				dataBuf[i] = 0;
				sumCheck += dataBuf[i];
			}
			dataBuf[i] = ~(sumCheck + E2PROM_LOCKED_KEY);
			
			// ��ǰ������Ӧ�Ĳ�д���������������ƫ�Ƶ�ַ
			addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
			E2PROM_WriteByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);  // ���µ�ǰ������Ӧ�Ĵ���ֵ�������˴���ֵ��У��ֵ
		}
	}
	// ����д�룬���ʱ��Ƚϳ������
	else
	{
		e2promCB.size += length;

		// ����E2PROM�ռ��С
		if ((E2PROM_SYSTEM_PARAM_ADDR_OFFSET + e2promCB.size) > EE_TOTAL_SIZE)
		{
			e2promCB.record[id].isExist = FALSE;
			return;
		}
		
		e2promCB.record[id].length = length;

		e2promCB.record[id].index = 0;

		e2promCB.record[id].count = 0;
	}

	// ִ�е��ˣ�˵��û�г���E2PROM�ռ��С
	e2promCB.record[id].isExist = TRUE;

}

// ע�᳤��Ϊ1���ֽڵĲ�����ָ��ID�ͷ���ģʽ
void E2PROM_ServiceRegister8bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode)
{
	E2PROM_ServiceRegister(id, 1, mode);
}

// ע�᳤��Ϊ2���ֽڵĲ�����ָ��ID�ͷ���ģʽ
void E2PROM_ServiceRegister16bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode)
{
	E2PROM_ServiceRegister(id, 2, mode);
}

// ע�᳤��Ϊ4���ֽڵĲ�����ָ��ID�ͷ���ģʽ
void E2PROM_ServiceRegister32bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode)
{
	E2PROM_ServiceRegister(id, 4, mode);
}

// ҵ���д
void E2PROM_ServiceWriteArray(E2PROM_ID id, uint8* pBuf)
{
	uint8 i;   
	uint16 addrBuf;
	uint8 dataBuf[sizeof(e2promCB.record[id].index)+1+sizeof(e2promCB.record[id].count)+1];
	uint8 sumCheck;

	// �����Ϸ��Լ���
	if ((id >= E2PROM_ID_MAX) || (NULL == pBuf))
	{
		return;
	}

	// ��IDע�᲻�ɹ�
	if (!e2promCB.record[id].isExist)
	{
		return;
	}

	// ��ҪƵ��д��
	if(E2PROM_ACCESS_MODE_QUITE_OFTEN == e2promCB.record[id].writeMode)
	{
		// ��ȡ����ֵ
		addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
		E2PROM_ReadByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count));
		for (i = 0, e2promCB.record[id].count = 0; i < sizeof(e2promCB.record[id].count); i++)
		{
			e2promCB.record[id].count <<= 8;
			e2promCB.record[id].count |= dataBuf[i];
		}

		// ÿдһ��������Ҫ��¼һ�δ���
		e2promCB.record[id].count++;	 
		
		if (e2promCB.record[id].count > E2PROM_ERASE_TIMES) // ��д������������
		{
			// ����ǰ������Ӧ�Ĳ�д�������¹��㲢д��EEPROM
			e2promCB.record[id].count = 0;
			
			// �������ֵ
			sumCheck = 0;
			for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
			{
				dataBuf[i] = 0;
				sumCheck += dataBuf[i];
			}
			dataBuf[i] = ~(sumCheck + E2PROM_LOCKED_KEY);
			
			// ��ǰ������Ӧ�Ĳ�д���������������ƫ�Ƶ�ַ
			addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
			E2PROM_WriteByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);  // ���µ�ǰ������Ӧ�Ĵ���ֵ�������˴���ֵ��У��ֵ
			
			// ����һ����д�ռ�
			e2promCB.record[id].index++;
			e2promCB.record[id].index %= E2PROM_MALLOC_SAVE_SPACE_COUNT;		// ����ָ������Ŀռ���
			
			// ����У��ֵ
			dataBuf[0] = e2promCB.record[id].index;
			dataBuf[1] = ~(e2promCB.record[id].index + E2PROM_LOCKED_KEY);
			E2PROM_WriteByteArray(e2promCB.record[id].addr, dataBuf, 2);  // ��������ֵ��������������У��ֵ

			// ����������Ӧ�Ĵ���ֵҲ����
			sumCheck = 0;
			for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
			{
				dataBuf[i] = 0;
				sumCheck += dataBuf[i];
			}
			dataBuf[i] = ~(sumCheck + E2PROM_LOCKED_KEY);
			
			// У�����������������ƫ�Ƶ�ַ
			addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
			E2PROM_WriteByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);  // ������һ��������Ӧ�Ĵ���ֵ�������˴���ֵ��У��ֵ

			// д����������
			// У���������������������ƫ�Ƶ�ַ
			addrBuf = addrBuf + sizeof(e2promCB.record[id].count);
			E2PROM_WriteByteArray(addrBuf, pBuf, e2promCB.record[id].length);
		}
		else
		{
			// �˴�������ֵ���ֲ���
			
			// д����
			// �����ĵ�ַ��Ҫ����������Ѱַ
			sumCheck = 0;
			for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
			{
				dataBuf[i] = (uint8)(e2promCB.record[id].count >> ((sizeof(e2promCB.record[id].count) - (i + 1)) * 8));
				sumCheck += dataBuf[i];
			}
			dataBuf[i] = ~(sumCheck + E2PROM_LOCKED_KEY);
			
			// У�����������������ƫ�Ƶ�ַ
			addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
			E2PROM_WriteByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);

			// д����������
			// У���������������������ƫ�Ƶ�ַ
			addrBuf = addrBuf + (sizeof(e2promCB.record[id].count)+1);
			E2PROM_WriteByteArray(addrBuf, pBuf, e2promCB.record[id].length);
		}
	}
	// ����д�룬���ʱ��Ƚϳ������
	else
	{
		E2PROM_WriteByteArray(e2promCB.record[id].addr, pBuf, e2promCB.record[id].length);
	}
}

// ҵ���дһ���ֽ�
void E2PROM_ServiceWrite8bit(E2PROM_ID id, uint8 data)
{
	uint8 dataBuf[1];

	dataBuf[0] = data;
	
	E2PROM_ServiceWriteArray(id, dataBuf);
}

// ҵ���дһ��˫�ֽ�
void E2PROM_ServiceWrite16bit(E2PROM_ID id, uint16 data)
{
	uint8 dataBuf[2];

	dataBuf[0] = data>>8;
	dataBuf[1] = data&0x00FF;
	
	E2PROM_ServiceWriteArray(id, dataBuf);
}

// ҵ���дһ�����ֽ�
void E2PROM_ServiceWrite32bit(E2PROM_ID id, uint32 data)
{
	uint8 dataBuf[4];

	dataBuf[0] = data>>24;
	dataBuf[1] = (data>>16)&0x00FF;
	dataBuf[2] = (data>>8)&0x00FF;
	dataBuf[3] = data&0x00FF;
	
	E2PROM_ServiceWriteArray(id, dataBuf);
}

// ҵ����
void E2PROM_ServiceReadArray(E2PROM_ID id, uint8* pBuf)
{ 
	uint16 addrBuf;

	// �����Ϸ��Լ���
	if ((id >= E2PROM_ID_MAX) || (NULL == pBuf))
	{
		return;
	}

	// ��IDע�᲻�ɹ�
	if (!e2promCB.record[id].isExist)
	{
		return;
	}

	// ��ȡƵ����д���ֽ�
	if(E2PROM_ACCESS_MODE_QUITE_OFTEN == e2promCB.record[id].writeMode)
	{
		// ��ȡ����ֵ
		E2PROM_ReadByteArray(e2promCB.record[id].addr, &e2promCB.record[id].index, 1);
		
		// ��ȡ����ֵ
		// �ȼ������ֵ�ĵ�ַ
		addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
		// �������ݵĵ�ַΪ����ֵ��ַ��������ռ���ֽ���
		addrBuf = addrBuf + (sizeof(e2promCB.record[id].count)+1);
		E2PROM_ReadByteArray(addrBuf, pBuf, e2promCB.record[id].length);
	}
	// �����ȡ
	else
	{
		E2PROM_ReadByteArray(e2promCB.record[id].addr, pBuf, e2promCB.record[id].length);
	}
}

// ҵ����һ���ֽ�
uint8 E2PROM_ServiceRead8bit(E2PROM_ID id)
{ 
	uint8 data;

	E2PROM_ServiceReadArray(id, &data);

	return data;
}

// ҵ����һ��˫�ֽ�
uint16 E2PROM_ServiceRead16bit(E2PROM_ID id)
{ 
	uint8 dataBuf[2];
	uint16 data;

	E2PROM_ServiceReadArray(id, dataBuf);

	data = dataBuf[0];
	data <<= 8;
	data |= dataBuf[1];

	return data;
}

// ҵ����һ�����ֽ�
uint32 E2PROM_ServiceRead32bit(E2PROM_ID id)
{ 
	uint8 dataBuf[4];
	uint32 data;

	E2PROM_ServiceReadArray(id, dataBuf);

	data = dataBuf[0];
	data <<= 8;
	data |= dataBuf[1];
	data <<= 8;
	data |= dataBuf[2];
	data <<= 8;
	data |= dataBuf[3];

	return data;
}

// ����ID��ȡ��ֵ
uint32 E2PROM_ServiceReadByID(E2PROM_ID id)
{
	if(1 == e2promCB.record[id].length)
	{
		return E2PROM_ServiceRead8bit(id);
	}
	else if(2 == e2promCB.record[id].length)
	{
		return E2PROM_ServiceRead16bit(id);
	}
	else if(4 == e2promCB.record[id].length)
	{
		return E2PROM_ServiceRead32bit(id);
	}
	else
	{
		return 0;
	}
}

// ����IDд����ֵ
void E2PROM_ServiceWriteByID(E2PROM_ID id, uint32 param)
{
	if(1 == e2promCB.record[id].length)
	{
		E2PROM_ServiceWrite8bit(id, (uint8)param);
	}
	else if(2 == e2promCB.record[id].length)
	{
		E2PROM_ServiceWrite16bit(id, (uint16)param);
	}
	else if(4 == e2promCB.record[id].length)
	{
		E2PROM_ServiceWrite32bit(id, param);
	}
	else
	{}
}


