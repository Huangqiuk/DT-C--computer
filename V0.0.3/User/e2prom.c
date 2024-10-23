#include "common.h" 
#include "e2prom.h"
#include "Watchdog.h"

E2PROM_CB e2promCB;


// ■■■■■■■■■■内部函数声明■■■■■■■■■■■■■■
void E2PROM_Delayus(volatile uint8 i);
void E2PROM_Delayms(volatile uint8 i);

// I2C初始化
void E2PROM_I2C_Init(void);
// I2C起始信号
void E2PROM_I2C_Start(void);
// I2C停止信号
void E2PROM_I2C_Stop(void);
// I2C写一个字节
void E2PROM_I2C_WriteByte(uint8 Value);
// I2C读取一个字节
uint8 E2PROM_I2C_ReadByte(uint8 AckOrNot);

// I2C在页内写数据
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
* 函数定义区
*********************************************************/
// 简单延时
void E2PROM_Delayus(volatile uint8 i)
{	 
	// 内核72MHz，1us
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

// 简单延时
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

		// 看门狗处理
		WDT_Clear();
	}
}


////////////////////////////////////////////////////////////////////////////////
// I2C初始化
void E2PROM_I2C_Init(void)
{
	// 打开时钟
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_5);
	gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_6);
	
	E2PROM_SCL_HIGH();
	E2PROM_SDA_HIGH();
	E2PROM_SCL_DIR_OUT();
	E2PROM_SDA_DIR_OUT();
}

// I2C起始信号
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

// I2C停止信号
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

// I2C写一个字节
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
	
	E2PROM_SDA_DIR_IN();				  // 准备接收ACK
	
	E2PROM_SCL_HIGH();	 
	E2PROM_DELAY();  
	
	E2PROM_SCL_LOW();		  
	E2PROM_DELAY();
	
	E2PROM_SDA_DIR_OUT();				  
}									 

// I2C读取一个字节
uint8 E2PROM_I2C_ReadByte(uint8 AckOrNot)
{
	uint8 i;
	uint8 RetValue = 0;
	
	E2PROM_SDA_DIR_IN();					// 端口设为输入方式以读取数据 
	 
	for (i=0; i<8; i++)
	{
		E2PROM_SCL_HIGH();					// 时钟为高,读取位
		E2PROM_DELAY();
		
		RetValue <<= 1;
		if (E2PROM_SDA_GET())
		{
			RetValue |= 0x01;
		}
		
		E2PROM_SCL_LOW();					// 时钟为低,24C02改变位数据
		E2PROM_DELAY();    
	}
	
	E2PROM_SDA_DIR_OUT();
	E2PROM_SDA_SET(AckOrNot);				// 应答与否
	E2PROM_DELAY();
	
	E2PROM_SCL_HIGH();
	E2PROM_DELAY();
	
	E2PROM_SCL_LOW();
	E2PROM_DELAY();  
	
	return RetValue;
}

// I2C在页内写数据
void E2PROM_WritePageBuf(uint16 writeAddr, uint8 *pBuf, uint16 writeNum)
{
	uint16	i = 0;
	uint8 addrH = (uint8)(writeAddr >> 8);	 // 取高8位地址
	uint8 addrL = (uint8)writeAddr; 		 // 取低8位地址

	// 判断存储地址是否超过8位
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte((addrH<<1) | EEPROM_WRCOMMAND);	// 设备地址也是字节地址的高8位
	E2PROM_I2C_WriteByte(addrL);
   
	while (i < writeNum)
	{
		E2PROM_I2C_WriteByte(pBuf[i++]);
	}
	E2PROM_I2C_Stop();

	// 延时ms级别等待buf数据写入EEPROM
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

	// 延时ms级别等待buf数据写入EEPROM
	E2PROM_DELAY_MS();

#endif
}


// E2PROM初始化
void E2PROM_Init(void)
{
	// 硬件初始化
	E2PROM_I2C_Init();

#if E2PROM_UNIT_TEST
	E2PROM_UnitTest();
#endif

	//E2PROM_ServiceInit();
}

// 读数据，整个地址内，无需考虑页问题
BOOL E2PROM_ReadByteArray(uint16 ReadAddr, uint8* pBuf, uint16 readNum)
{
	uint16 i = 0;
	uint8 addrH = (uint8)(ReadAddr >> 8);	// 取高8位地址
	uint8 addrL = (uint8)ReadAddr;			// 取低8位地址

	// 地址不能超出总空间大小
	if (ReadAddr > EE_TOTAL_SIZE)
	{
		return FALSE;
	}

	// 读的数量超出总空间大小
	if ((ReadAddr + readNum) > EE_TOTAL_SIZE) 
	{
		return FALSE;
	}

	// 空指针
	if (NULL == pBuf)
	{
		return FALSE;
	}

	// 判断存储地址是否超过8位
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(((addrH<<1)&0x0F) | EEPROM_WRCOMMAND);    // 设备地址也是字节地址的高8位，写指令
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// 读指令
	while(i < (readNum-1))
	{
		pBuf[i++] = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	}
	pBuf[i] = E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();
	
#else

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND); // 写指令
	E2PROM_I2C_WriteByte(addrH);
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// 读指令
	while(i < (readNum-1))
	{
		pBuf[i++] = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	}
	pBuf[i] = E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

#endif
	return TRUE;
}

// 写数据，整个地址内，无需考虑页问题
BOOL E2PROM_WriteByteArray(uint16 writeAddr, uint8* pBuf, uint16 writeNum)
{
	uint16 pageNum = 0;
	uint16 singleNum = 0;
	uint16 part = 0;

	// 地址不能超出总空间大小
	if (writeAddr > EE_TOTAL_SIZE)
	{
		return FALSE;
	}

	// 写的数量超出总空间大小
	if ((writeAddr + writeNum) > EE_TOTAL_SIZE) 
	{
		return FALSE;
	}

	// 空指针
	if (NULL == pBuf)
	{
		return FALSE;
	}

	// 判断起始地址所在的页剩余可写字节个数
	part = writeAddr / EE_PAGE_SIZE;			// 起始地址所在的页
	part = EE_PAGE_SIZE*(part+1) - writeAddr;	// 此页剩余可写入字节个数

	// 判断是否需要跨页写入
	if (part >= writeNum)	// 写入的数据个数小于跨页剩余的个数可直接写入
	{
		E2PROM_WritePageBuf(writeAddr, pBuf, writeNum);
	}
	else					// 当前页个数不足，需要跨页算法写入
	{
		// 1.先把当前地址所在的页填充满
		E2PROM_WritePageBuf(writeAddr, pBuf, part);

		// 2.计算剩余页数及不足一页的字节数
		pageNum = (writeNum - part) / EE_PAGE_SIZE; 
		singleNum = (writeNum - part) % EE_PAGE_SIZE; 

		// 3.计算剩余总个数及下一页开始地址和数据指针
		writeNum -= part;	// 剩余写入个数
		writeAddr += part;	// 下一页写入地址
		pBuf += part;		// 下一页写入指针

		// 4.按计算的数据量占页面数，连续写入页面
		while (pageNum--)
		{ 
			E2PROM_WritePageBuf(writeAddr, pBuf, EE_PAGE_SIZE);
			
			// 每写满一页，地址自增一页，数据指针也自增一页地址
			writeAddr += EE_PAGE_SIZE;
			pBuf += EE_PAGE_SIZE; 
		}

		// 5.补充页面写完后超出的不足一页数据量的数据
		if (0 != singleNum)
		{
			E2PROM_WritePageBuf(writeAddr, pBuf, singleNum);
		}
	}
  
	return TRUE;
}

// 读取一个字节
uint8 E2PROM_ReadByte(uint16 ReadAddr)
{
	uint8 data;
	uint8 addrH = (uint8)(ReadAddr >> 8);	// 取高8位地址
	uint8 addrL = (uint8)ReadAddr;			// 取低8位地址

	// 判断存储地址是否超过8位
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(((addrH<<1)&0x0F) | EEPROM_WRCOMMAND);    // 设备地址也是字节地址的高8位，写指令
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// 读指令

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;
	
#else

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND); // 写指令
	E2PROM_I2C_WriteByte(addrH);
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// 读指令

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;

#endif
}

// 读取一个双字节数值
uint16 E2PROM_ReadInt(uint16 ReadAddr)
{
	uint16 data = 0;
	uint8 addrH = (uint8)(ReadAddr >> 8);	// 取高8位地址
	uint8 addrL = (uint8)ReadAddr;			// 取低8位地址

	// 判断存储地址是否超过8位
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(((addrH<<1)&0x0F) | EEPROM_WRCOMMAND);    // 设备地址也是字节地址的高8位，写指令
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// 读指令

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;
	
#else

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND); // 写指令
	E2PROM_I2C_WriteByte(addrH);
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// 读指令

	data = E2PROM_I2C_ReadByte(E2PROM_I2C_ACK);
	data <<= 8;
	data |= E2PROM_I2C_ReadByte(E2PROM_I2C_NACK);

	E2PROM_I2C_Stop();

	return data;

#endif
}

// 读取一个四字节数值
uint32 E2PROM_ReadWord(uint16 ReadAddr)
{
	uint32 data = 0;
	uint8 addrH = (uint8)(ReadAddr >> 8);	// 取高8位地址
	uint8 addrL = (uint8)ReadAddr;			// 取低8位地址

	// 判断存储地址是否超过8位
#if EEPROM_TYPE <= EEPROM_24C16

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(((addrH<<1)&0x0F) | EEPROM_WRCOMMAND);    // 设备地址也是字节地址的高8位，写指令
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// 读指令

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
	E2PROM_I2C_WriteByte(EEPROM_WRCOMMAND); // 写指令
	E2PROM_I2C_WriteByte(addrH);
	E2PROM_I2C_WriteByte(addrL);

	E2PROM_I2C_Start();
	E2PROM_I2C_WriteByte(EEPROM_RDCOMMAND);	// 读指令

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

// 写入一个字节
void E2PROM_WriteByte(uint16 writeAddr, uint8 data)
{
	uint8 dataCheck = ~data;
	uint8 retry = 5;
	uint8 addrH = (uint8)(writeAddr >> 8);	 // 取高8位地址
	uint8 addrL = (uint8)writeAddr; 		 // 取低8位地址

	// 判断存储地址是否超过8位
#if EEPROM_TYPE <= EEPROM_24C16
	while((dataCheck != data)&&(retry))
	{
		retry--;

		E2PROM_I2C_Start();
		E2PROM_I2C_WriteByte((addrH<<1) | EEPROM_WRCOMMAND);	// 设备地址也是字节地址的高8位
		E2PROM_I2C_WriteByte(addrL);
	   
		E2PROM_I2C_WriteByte(data);
		
		E2PROM_I2C_Stop();

		// 延时ms级别等待buf数据写入EEPROM
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

		// 延时ms级别等待buf数据写入EEPROM
		E2PROM_DELAY_MS();

		dataCheck = E2PROM_ReadByte(writeAddr);
	}

#endif
}

// 写入一个双字节数值
void E2PROM_WriteInt(uint16 writeAddr, uint16 data)
{
	uint8 addrH;
	uint8 addrL;
	uint8 localData;
	uint8 i = 2;
	uint8 page = writeAddr/EE_PAGE_SIZE;

	// 判断存储地址是否超过8位
#if EEPROM_TYPE <= EEPROM_24C16
	while(i)
	{
		addrH = (uint8)(writeAddr >> 8);	// 取高8位地址
		addrL = (uint8)writeAddr; 		 	// 取低8位地址
		localData = data >> (8*(i-1));
	
		E2PROM_I2C_Start();
		E2PROM_I2C_WriteByte((addrH<<1) | EEPROM_WRCOMMAND);	// 设备地址也是字节地址的高8位
		E2PROM_I2C_WriteByte(addrL);

		while(i)
		{
			E2PROM_I2C_WriteByte(localData);
			writeAddr++;
			i--;
			localData = data >> (8*(i-1));

			// 页切换，重新开始
			if(page != writeAddr/EE_PAGE_SIZE)
			{
				break;
			}
		}
		
		E2PROM_I2C_Stop();

		// 延时ms级别等待buf数据写入EEPROM
		E2PROM_DELAY_MS();
	}

#else
	while(i)
	{
		addrH = (uint8)(writeAddr >> 8);	// 取高8位地址
		addrL = (uint8)writeAddr; 		 	// 取低8位地址
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

			// 页切换，重新开始
			if(page != writeAddr/EE_PAGE_SIZE)
			{
				break;
			}
		}

		E2PROM_I2C_Stop();

		// 延时ms级别等待buf数据写入EEPROM
		E2PROM_DELAY_MS();
	}

#endif
}

// 写入一个四字节数值
void E2PROM_WriteWord(uint16 writeAddr, uint32 data)
{
	uint8 addrH;
	uint8 addrL;
	uint8 localData;
	uint8 i = 4;
	uint8 page = writeAddr/EE_PAGE_SIZE;

	// 判断存储地址是否超过8位
#if EEPROM_TYPE <= EEPROM_24C16
	while(i)
	{
		addrH = (uint8)(writeAddr >> 8);	// 取高8位地址
		addrL = (uint8)writeAddr; 		 	// 取低8位地址
		localData = data >> (8*(i-1));
	
		E2PROM_I2C_Start();
		E2PROM_I2C_WriteByte((addrH<<1) | EEPROM_WRCOMMAND);	// 设备地址也是字节地址的高8位
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

			// 页切换，重新开始
			if(page != writeAddr/EE_PAGE_SIZE)
			{
				break;
			}
		}
		
		E2PROM_I2C_Stop();

		// 延时ms级别等待buf数据写入EEPROM
		E2PROM_DELAY_MS();
	}

#else
	while(i)
	{
		addrH = (uint8)(writeAddr >> 8);	// 取高8位地址
		addrL = (uint8)writeAddr; 		 	// 取低8位地址
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

			// 页切换，重新开始
			if(page != writeAddr/EE_PAGE_SIZE)
			{
				break;
			}
		}

		E2PROM_I2C_Stop();

		// 延时ms级别等待buf数据写入EEPROM
		E2PROM_DELAY_MS();
	}

#endif
}


//==================================================================
// 服务数据初始化
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

	// 注册系统参数
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE1, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE2, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE3, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE4, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE5, E2PROM_ACCESS_MODE_NOT_OFTEN);
	E2PROM_ServiceRegister32bit(E2PROM_ID_WHELL_SIZE6, E2PROM_ACCESS_MODE_NOT_OFTEN);
}

// 业务层获取相应ID存取的地址
uint16 E2PROM_ServiceGetIDAddr(E2PROM_ID id)
{	
	return e2promCB.record[id].addr;
}

// 业务层获取使用的总空间大小
uint32 E2PROM_ServiceGetRomSize(void)
{
	return e2promCB.size;
}

// 注册相应ID的长度以及是否需要频繁擦写模式
void E2PROM_ServiceRegister(E2PROM_ID id, uint16 length, E2PROM_ACCESS_MODE_E mode)
{
	uint8 i;
	uint8 indexBuf[2];
	uint16 addrBuf;
	uint8 dataBuf[sizeof(e2promCB.record[id].count)+1];
	uint8 sumCheck;

	// 参数合法性检验
	if ((id >= E2PROM_ID_MAX) || (length > E2PROM_DISPOSABIE_DATA_LENGTH_MAX))
	{
		return;
	}

	// 超出E2PROM空间大小
	if ((E2PROM_SYSTEM_PARAM_ADDR_OFFSET + e2promCB.size) > EE_TOTAL_SIZE)
	{
		return;
	} 
	
	// 需要写入数据的一个记录条首地址
	e2promCB.record[id].addr = E2PROM_SYSTEM_PARAM_ADDR_OFFSET + e2promCB.size;

	// 是否需要频繁写入
	e2promCB.record[id].writeMode = mode;

	// 需要频繁写入
	if (mode)
	{
		// 需要频繁写入的数据将要增加一个索引以及记录写入数据次数所分配的总空间
		e2promCB.size += (sizeof(e2promCB.record[id].index) + (sizeof(e2promCB.record[id].count) + length) * E2PROM_MALLOC_SAVE_SPACE_COUNT);

		// 超出E2PROM空间大小
		if ((E2PROM_SYSTEM_PARAM_ADDR_OFFSET + e2promCB.size) > EE_TOTAL_SIZE)
		{
			e2promCB.record[id].isExist = FALSE;
			return;
		}		  

		// 更新当前ID对应的数据长度
		e2promCB.record[id].length = length; 

		// 索引值
		E2PROM_ReadByteArray(e2promCB.record[id].addr, indexBuf, 2);
		
		// 判断是否需要给其配置初始值
		if (~(indexBuf[0] + E2PROM_LOCKED_KEY) == indexBuf[1])
		{
			e2promCB.record[id].index = indexBuf[0];
		}
		else
		{
			// 第一次上电赋默认值
			e2promCB.record[id].index = 0;
			indexBuf[0] = e2promCB.record[id].index;
			indexBuf[1] = ~(e2promCB.record[id].index + E2PROM_LOCKED_KEY);
			E2PROM_WriteByteArray(e2promCB.record[id].addr, indexBuf, 2);  // 更新索引值，包含了索引的校验值
		}

		// 读取擦写次数值
		addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
		E2PROM_ReadByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);

		// 校验值
		sumCheck = 0;
		for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
		{
			sumCheck += dataBuf[i];
		}
		sumCheck = ~(sumCheck + E2PROM_LOCKED_KEY);
		
		// 判断是否需要给其配置初始值
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
			// 第一次上电赋默认值
			e2promCB.record[id].count = 0;
			
			// 计算检验值
			sumCheck = 0;
			for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
			{
				dataBuf[i] = 0;
				sumCheck += dataBuf[i];
			}
			dataBuf[i] = ~(sumCheck + E2PROM_LOCKED_KEY);
			
			// 当前索引对应的擦写次数相对于索引的偏移地址
			addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
			E2PROM_WriteByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);  // 更新当前索引对应的次数值，包含了次数值的校验值
		}
	}
	// 常规写入，间隔时间比较长的情况
	else
	{
		e2promCB.size += length;

		// 超出E2PROM空间大小
		if ((E2PROM_SYSTEM_PARAM_ADDR_OFFSET + e2promCB.size) > EE_TOTAL_SIZE)
		{
			e2promCB.record[id].isExist = FALSE;
			return;
		}
		
		e2promCB.record[id].length = length;

		e2promCB.record[id].index = 0;

		e2promCB.record[id].count = 0;
	}

	// 执行到此，说明没有超出E2PROM空间大小
	e2promCB.record[id].isExist = TRUE;

}

// 注册长度为1个字节的参数，指定ID和访问模式
void E2PROM_ServiceRegister8bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode)
{
	E2PROM_ServiceRegister(id, 1, mode);
}

// 注册长度为2个字节的参数，指定ID和访问模式
void E2PROM_ServiceRegister16bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode)
{
	E2PROM_ServiceRegister(id, 2, mode);
}

// 注册长度为4个字节的参数，指定ID和访问模式
void E2PROM_ServiceRegister32bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode)
{
	E2PROM_ServiceRegister(id, 4, mode);
}

// 业务层写
void E2PROM_ServiceWriteArray(E2PROM_ID id, uint8* pBuf)
{
	uint8 i;   
	uint16 addrBuf;
	uint8 dataBuf[sizeof(e2promCB.record[id].index)+1+sizeof(e2promCB.record[id].count)+1];
	uint8 sumCheck;

	// 参数合法性检验
	if ((id >= E2PROM_ID_MAX) || (NULL == pBuf))
	{
		return;
	}

	// 此ID注册不成功
	if (!e2promCB.record[id].isExist)
	{
		return;
	}

	// 需要频繁写入
	if(E2PROM_ACCESS_MODE_QUITE_OFTEN == e2promCB.record[id].writeMode)
	{
		// 读取次数值
		addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
		E2PROM_ReadByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count));
		for (i = 0, e2promCB.record[id].count = 0; i < sizeof(e2promCB.record[id].count); i++)
		{
			e2promCB.record[id].count <<= 8;
			e2promCB.record[id].count |= dataBuf[i];
		}

		// 每写一次数据需要记录一次次数
		e2promCB.record[id].count++;	 
		
		if (e2promCB.record[id].count > E2PROM_ERASE_TIMES) // 擦写次数超过限制
		{
			// 将当前索引对应的擦写次数重新归零并写入EEPROM
			e2promCB.record[id].count = 0;
			
			// 计算检验值
			sumCheck = 0;
			for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
			{
				dataBuf[i] = 0;
				sumCheck += dataBuf[i];
			}
			dataBuf[i] = ~(sumCheck + E2PROM_LOCKED_KEY);
			
			// 当前索引对应的擦写次数相对于索引的偏移地址
			addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
			E2PROM_WriteByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);  // 更新当前索引对应的次数值，包含了次数值的校验值
			
			// 换下一个擦写空间
			e2promCB.record[id].index++;
			e2promCB.record[id].index %= E2PROM_MALLOC_SAVE_SPACE_COUNT;		// 超过指定分配的空间数
			
			// 计算校验值
			dataBuf[0] = e2promCB.record[id].index;
			dataBuf[1] = ~(e2promCB.record[id].index + E2PROM_LOCKED_KEY);
			E2PROM_WriteByteArray(e2promCB.record[id].addr, dataBuf, 2);  // 更新索引值，包含了索引的校验值

			// 将新索引对应的次数值也清零
			sumCheck = 0;
			for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
			{
				dataBuf[i] = 0;
				sumCheck += dataBuf[i];
			}
			dataBuf[i] = ~(sumCheck + E2PROM_LOCKED_KEY);
			
			// 校正次数相对于索引的偏移地址
			addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
			E2PROM_WriteByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);  // 更新下一个索引对应的次数值，包含了次数值的校验值

			// 写真正的数据
			// 校正真正数据相对于索引的偏移地址
			addrBuf = addrBuf + sizeof(e2promCB.record[id].count);
			E2PROM_WriteByteArray(addrBuf, pBuf, e2promCB.record[id].length);
		}
		else
		{
			// 此处的索引值保持不变
			
			// 写次数
			// 次数的地址需要根据索引来寻址
			sumCheck = 0;
			for (i = 0; i < sizeof(e2promCB.record[id].count); i++)
			{
				dataBuf[i] = (uint8)(e2promCB.record[id].count >> ((sizeof(e2promCB.record[id].count) - (i + 1)) * 8));
				sumCheck += dataBuf[i];
			}
			dataBuf[i] = ~(sumCheck + E2PROM_LOCKED_KEY);
			
			// 校正次数相对于索引的偏移地址
			addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
			E2PROM_WriteByteArray(addrBuf, dataBuf, sizeof(e2promCB.record[id].count)+1);

			// 写真正的数据
			// 校正真正数据相对于索引的偏移地址
			addrBuf = addrBuf + (sizeof(e2promCB.record[id].count)+1);
			E2PROM_WriteByteArray(addrBuf, pBuf, e2promCB.record[id].length);
		}
	}
	// 常规写入，间隔时间比较长的情况
	else
	{
		E2PROM_WriteByteArray(e2promCB.record[id].addr, pBuf, e2promCB.record[id].length);
	}
}

// 业务层写一个字节
void E2PROM_ServiceWrite8bit(E2PROM_ID id, uint8 data)
{
	uint8 dataBuf[1];

	dataBuf[0] = data;
	
	E2PROM_ServiceWriteArray(id, dataBuf);
}

// 业务层写一个双字节
void E2PROM_ServiceWrite16bit(E2PROM_ID id, uint16 data)
{
	uint8 dataBuf[2];

	dataBuf[0] = data>>8;
	dataBuf[1] = data&0x00FF;
	
	E2PROM_ServiceWriteArray(id, dataBuf);
}

// 业务层写一个四字节
void E2PROM_ServiceWrite32bit(E2PROM_ID id, uint32 data)
{
	uint8 dataBuf[4];

	dataBuf[0] = data>>24;
	dataBuf[1] = (data>>16)&0x00FF;
	dataBuf[2] = (data>>8)&0x00FF;
	dataBuf[3] = data&0x00FF;
	
	E2PROM_ServiceWriteArray(id, dataBuf);
}

// 业务层读
void E2PROM_ServiceReadArray(E2PROM_ID id, uint8* pBuf)
{ 
	uint16 addrBuf;

	// 参数合法性检验
	if ((id >= E2PROM_ID_MAX) || (NULL == pBuf))
	{
		return;
	}

	// 此ID注册不成功
	if (!e2promCB.record[id].isExist)
	{
		return;
	}

	// 读取频繁擦写的字节
	if(E2PROM_ACCESS_MODE_QUITE_OFTEN == e2promCB.record[id].writeMode)
	{
		// 读取索引值
		E2PROM_ReadByteArray(e2promCB.record[id].addr, &e2promCB.record[id].index, 1);
		
		// 读取数据值
		// 先计算次数值的地址
		addrBuf = (e2promCB.record[id].addr+2) + (e2promCB.record[id].index * ((sizeof(e2promCB.record[id].count)+1) + e2promCB.record[id].length));
		// 真正数据的地址为次数值地址加上其所占的字节数
		addrBuf = addrBuf + (sizeof(e2promCB.record[id].count)+1);
		E2PROM_ReadByteArray(addrBuf, pBuf, e2promCB.record[id].length);
	}
	// 常规读取
	else
	{
		E2PROM_ReadByteArray(e2promCB.record[id].addr, pBuf, e2promCB.record[id].length);
	}
}

// 业务层读一个字节
uint8 E2PROM_ServiceRead8bit(E2PROM_ID id)
{ 
	uint8 data;

	E2PROM_ServiceReadArray(id, &data);

	return data;
}

// 业务层读一个双字节
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

// 业务层读一个四字节
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

// 根据ID读取数值
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

// 根据ID写入数值
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


