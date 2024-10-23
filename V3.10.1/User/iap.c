/**********************************************************************************************************************************
	Description:
	1.首先在Bootloader（引导程序）的options for target中设置好FLASH的起始位置,如0x08000000~0x08010000,size = 10000;
	2.在引导程序的main主函数中设置中断向量起始地址：NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000);
	3.设置用户app应用程序的起始地址和范围，如0x08010000~0x08090000,size = 80000;
	4.设置用户app程序的中断向量起始地址，篘VIC_SetVectorTable(NVIC_VectTab_FLASH, 0x10000);

***************************************IAP封装接口函数使用步骤****************************************************
	1.首先按照以上方法对引导程序和用户程序进行相应配置；
	2.通过USART/I2C/CAN总线/SPI等通信方式将应用程序生成的bin文件发送到烧有Bootloader引导程序的目标板上，以数组的方式对缓冲区数据进行存储；
	3.设置好接收成功标志位flag,当判断接收成功之后，即可使用本接口函数对程序进行升级；
	4.接口函数1：BOOL IAP_WriteAppBin(u32 appAddr,u8 *appBuf,u32 appLen)————将应用程序的起始地址，数据首地址和应用程序的长度作为输入，
	  将数据写入FLASE当中,若写入后校验成功返回TRUE,否则，返回FALSE,提示写入失败;对appLen赋0即可对固件进行擦除；
	5.接口函数2：void IAP_UpdataReset(void)————当判断函数1返回为TRUE之后，即可对程序进行软件复位；
	6.接口函数3：void IAP_RunAPP(u32 appAddr)————程序跳转，执行应用程序；
	注：为了防止程序跑飞，在进行程序跳转时要对程序判断：if(((*(vu32*)(IAP_FLASH_APP1_ADDR+4))&0xFF000000)==IAP_STM32_FLASH_BASE),
	    如果不满足条件，则不能跳转至应用程序。
	Others: 		//
	Function List:	// 
	1. ....
	History:		// 
					// 
	1. Date:
	   Author:
	   Modification:
	2. ...
**************************************************************************************************************************************/

// IAP通用头文件
#include "common.h"
#include "iap.h"
#include "iapctrluart.h"
#include "timer.h"
#include "e2prom.h"
#include "Spiflash.h"
#include "lcd.h"
#include "BleProtocol.h"
#include "Watchdog.h" 

/*********************************************************
* 变量定义区
*********************************************************/
IAP_CB iapCB;

// FLASH读写操作
// 半字操作
BOOL IAP_FlashWriteHalfWordArray(uint32 writeAddr, uint16 *pBuffer, uint16 numToWrite);		//从指定地址开始写入指定长度的数据
void IAP_FlashReadHalfWordArray(uint32 readAddr, uint16 *pBuffer, uint16 numToRead);   		//从指定地址开始读出指定长度的数据
BOOL IAP_FlashWriteHalfWordArrayAndCheck(uint32 writeAddr, uint16 *pBuffer, uint16 numToWrite);
uint16 IAP_FlashReadHalfWord(uint32 flashAddr);
// 字操作
BOOL IAP_FlashWriteWordArray(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite);		//从指定地址开始写入指定长度的数据
void IAP_FlashReadWordArray(uint32 readAddr, uint32 *pBuffer, uint16 numToRead);   		//从指定地址开始读出指定长度的数据
BOOL IAP_FlashWriteWordArrayAndCheck(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite);
uint32 IAP_FlashReadWord(uint32 flashAddr);

// APP处理函数
void IAP_RunAPP(uint32 appAddr);

/*******************************************************************************
  * @函数名称	IAP_RunAPP
  * @函数说明   跳转到应用程序段
  * @输入参数   appAddr:应用程序的起始地址
  * @返回参数   无
*******************************************************************************/
void IAP_RunAPP(uint32 appAddr)
{
    // appAddr存放的是用户程序Flash的首地址，(*(vu32*)appAddr的意思是取用户程序首地址里面的数据，该数据是用户程序的堆栈地址
    // 堆栈地址指向RAM,而RAM的起始地址是0x20000000,因此上面的判断语句执行:判断用户代码的堆栈地址是否落在:0x20000000~0x2001ffff区间中，
    // 这个区间的大小较多落在<128K RAM的区间，因此需要进行下面的判断

	//定义一个函数类型的参数	
	typedef void (*IAP_Func)(void);
	IAP_Func jumpToApp;

	if(((*(uint32*)appAddr) & 0x2FFE0000) == 0x20000000)	// 检查栈顶地址是否合法.
	{ 
		if (((*(uint32*)(appAddr+4)) & 0xFFF80000) != 0x08000000)   // APP无效
		{
			return;
		}

		// 跳转之前关总中断，复位外设为默认值
		DI();
		RCU_APB2RST |= 0xFFFFFFFF;
		RCU_APB2RST &= ~0xFFFFFFFF;
		RCU_APB1RST |= 0xFFFFFFFF;
		RCU_APB1RST &= ~0xFFFFFFFF;
		
		jumpToApp = (IAP_Func)*(uint32*)(appAddr+4);		// 用户代码区第二个字为程序开始地址(复位地址)		
	  	__set_MSP(*(uint32*)appAddr);						// 初始化用户程序的堆栈指针
		jumpToApp();										// 跳转到用户程序APP
	}
}

// 窗口时间到跳转到APP代码区运行
void IAP_JumpToAppFun(uint32 param)
{
	// 检查栈顶地址是否合法.
	if(((*(uint32*)param) & 0x2FFE0000) != 0x20000000)	
	{
		return;
	}

	// APP无效
	if (((*(uint32*)(param+4)) & 0xFFF80000) != 0x08000000)
	{
		return;
	}
	
	// 执行APP程序
	IAP_RunAPP(param);
}

// 判断指定地址APP是否合法
uint8 IAP_CheckAppRightful(uint32 addr)
{
	// 检查栈顶地址是否合法.
	if(((*(uint32*)addr) & 0x2FFE0000) != 0x20000000)	
	{
		return 0;
	}

	// APP无效
	if (((*(uint32*)(addr+4)) & 0xFFF80000) != 0x08000000)
	{
		return 0;
	}

	return 1;
}


/*******************************************************************************
  * @函数名称	IAP_Init
  * @函数说明   
  * @输入参数   
  * @返回参数   无
*******************************************************************************/
void IAP_Init(void)
{		
	iapCB.mcuUID.sn0 = *(uint32*)(0x1FFFF7E8); 
	iapCB.mcuUID.sn1 = *(uint32*)(0x1FFFF7EC); 
	iapCB.mcuUID.sn2 = *(uint32*)(0x1FFFF7F0);
	
	// 设置中断向量起始位置
	nvic_vector_table_set(NVIC_VECTTAB_FLASH,0x0000);
	
	TIMER_AddTask(TIMER_ID_IAP_ECO_REQUEST,
					50,
					IAP_CTRL_UART_SendCmdProjectApply,
					IAP_CTRL_UART_CMD_UP_PROJECT_APPLY,
					3,
					ACTION_MODE_ADD_TO_QUEUE);
		
}

/*******************************************************************************
  * @函数名称	IAP_WriteAppBin
  * @函数说明   将接收到的bin文件数据写入指定地址的FLASH当中
  * @输入参数   appAddr:应用程序的起始地址 appBuf：应用程序缓冲数据 appSize:应用程序数据大小（单位：字节）
  * @返回参数   无
*******************************************************************************/
BOOL IAP_WriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appSize)
{
	uint16 i = 0;
	uint16 j = 0;
	uint32 curAppAddr = appAddr; 	// 当前写入的地址
	uint16 iapBuf[SPI_FLASH_APP_READ_UPDATA_SIZE];			// 此16位数据需要和串口通讯协议一帧最长字节有关126*2=252个字节
	BOOL writeStatus;
	
	// 参数判断
	if (NULL == appBuf)
	{
		return FALSE;
	}

	// 参数检验是否非法，地址不能是IAP本身代码地址，绝对不能自己把自己擦掉
	if ((appAddr < IAP_FLASH_APP1_ADDR) || (appAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// 非法地址
	}
	
	// 循环写入
	for (j = 0; j < appSize; j += 2)
	{
		// 字节整合
		iapBuf[i++] = (uint16)(*appBuf) + ((uint16)(*(appBuf+1)) << 8);
		// 偏移2个字节
		appBuf += 2;
	}

	// 直接写入FLASH
	writeStatus = IAP_FlashWriteHalfWordArray(curAppAddr, iapBuf, appSize/2);	

	return writeStatus;
}


/*******************************************************************************
  * @函数名称	IAP_BleWriteAppBin
  * @函数说明   将接收到的bin文件数据写入指定地址的FLASH当中
  * @输入参数   appAddr:应用程序的起始地址 appBuf：应用程序缓冲数据 appSize:应用程序数据大小（单位：字节）
  * @返回参数   无
*******************************************************************************/
BOOL IAP_BleWriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appSize)
{
	uint16 i = 0;
	uint16 j = 0;
	uint32 curAppAddr = appAddr; 	// 当前写入的地址
	uint32 *iapBuf = NULL;
	BOOL writeStatus;
	
	// 参数判断
	if (NULL == appBuf)
	{
		return FALSE;
	}

	// 参数检验是否非法，地址不能是IAP本身代码地址，绝对不能自己把自己擦掉
	if ((appAddr < IAP_FLASH_APP1_ADDR) || ((appAddr + appSize) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;		// 非法地址
	}

	// 开辟临时内存
	iapBuf = (uint32 *)malloc(1024);
	if (NULL == iapBuf)
	{
		return FALSE;
	}
	
	// 循环写入
	for (i = 0, j = 0; j < appSize; j += 4)
	{
		// 字节整合
		iapBuf[i++] = (uint32)(*appBuf) + ((uint32)(*(appBuf+1)) << 8) + ((uint32)(*(appBuf+2)) << 16) + ((uint32)(*(appBuf+3)) << 24);
		// 偏移4个字节
		appBuf += 4;
	}      
	
	// 直接写入FLASH
	writeStatus = IAP_FlashWriteWordArray(curAppAddr, iapBuf, appSize/4);	
	
	// 释放内存
	free(iapBuf);
	
	return writeStatus;
}


// 查询Flash是否为空
BOOL IAP_CheckFlashIsBlank(void)
{
	uint16 i;
	uint16 j;
	uint16 readHalfWord;
	uint32 readAddr;
	
	for (i = (IAP_FLASH_ADRESS_APP1_OFFSET/IAP_STM32_PAGE_SIZE); i < (IAP_GD32_FLASH_SIZE/IAP_STM32_PAGE_SIZE); i++)
	{
		// 页地址
		readAddr = i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE;

		for (j = 0; j < IAP_STM32_PAGE_SIZE; j += 2)
		{
			readHalfWord = IAP_FlashReadHalfWord(readAddr);	// 读
		  	readAddr += 2;

			// 全为0xFFFF说明为空
			if (readHalfWord == 0xFFFF)
			{
				continue;
			}
			else
			{
				return FALSE;
			}
		}  
		
	}

	return TRUE;
}

// 查询Flash是否为空
BOOL IAP_CheckApp1FlashIsBlank(void)
{
	uint16 i;
	uint16 j;
	uint16 readHalfWord;
	uint32 readAddr;
	
	for (i = (IAP_FLASH_ADRESS_APP1_OFFSET/IAP_STM32_PAGE_SIZE); i < ((IAP_FLASH_ADRESS_APP1_OFFSET + IAP_GD32_APP1_SIZE)/IAP_STM32_PAGE_SIZE); i++)
	{
		// 页地址
		readAddr = i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE;

		for (j = 0; j < IAP_STM32_PAGE_SIZE; j += 2)
		{
			readHalfWord = IAP_FlashReadHalfWord(readAddr);	// 读
		  	readAddr += 2;

			// 全为0xFFFF说明为空
			if (readHalfWord == 0xFFFF)
			{
				continue;
			}
			else
			{
				return FALSE;
			}
		}  
		
	}

	return TRUE;
}

// 查询Flash是否为空
BOOL IAP_CheckApp2FlashIsBlank(void)
{
	uint16 i;
	uint16 j;
	uint16 readHalfWord;
	uint32 readAddr;
	
	for (i = (IAP_FLASH_ADRESS_APP2_OFFSET/IAP_STM32_PAGE_SIZE); i < ((IAP_FLASH_ADRESS_APP2_OFFSET + IAP_GD32_APP2_SIZE)/IAP_STM32_PAGE_SIZE); i++)
	{
		// 页地址
		readAddr = i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE;

		for (j = 0; j < IAP_STM32_PAGE_SIZE; j += 2)
		{
			readHalfWord = IAP_FlashReadHalfWord(readAddr);	// 读
		  	readAddr += 2;

			// 全为0xFFFF说明为空
			if (readHalfWord == 0xFFFF)
			{
				continue;
			}
			else
			{
				return FALSE;
			}
		}
		
	}

	return TRUE;
}


// 擦除APP所有页
BOOL IAP_EraseAPPArea(void)
{
	uint16 i;
	fmc_state_enum status = FMC_READY;
	BOOL eraseStatus = TRUE;

	DI();				// 关闭总中断
	fmc_unlock();		// 解锁

	// 清除所有挂起标志位
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	// 擦除APP所有页
	for (i = (IAP_FLASH_ADRESS_APP1_OFFSET/IAP_STM32_PAGE_SIZE); i < (IAP_GD32_FLASH_SIZE/IAP_STM32_PAGE_SIZE); i++)
	{
		status = fmc_page_erase(i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
		
		if (FMC_READY != status)
		{
			eraseStatus = FALSE;

			break;
		}

		// 清看门狗
		WDT_Clear();
	}

	fmc_lock();		// 上锁
	EI(); 				// 开启总中断

	return eraseStatus;
}

// 擦除APP1所有页
BOOL IAP_EraseAPP1Area(void)
{
	uint16 i;
	fmc_state_enum status = FMC_READY;
	BOOL eraseStatus = TRUE;

	DI();				// 关闭总中断
	fmc_unlock();		// 解锁

	// 清除所有挂起标志位
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	// 擦除APP所有页
	for (i = (IAP_FLASH_ADRESS_APP1_OFFSET/IAP_STM32_PAGE_SIZE); i < ((IAP_FLASH_ADRESS_APP1_OFFSET + IAP_GD32_APP1_SIZE)/IAP_STM32_PAGE_SIZE); i++)
	{
		status = fmc_page_erase(i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
		
		if (FMC_READY != status)
		{
			eraseStatus = FALSE;

			break;
		}

		// 清看门狗
		WDT_Clear();
	}

	fmc_lock();			// 上锁
	EI(); 				// 开启总中断

	return eraseStatus;
}

// 擦除APP2所有页
BOOL IAP_EraseAPP2Area(void)
{
	uint16 i;
	fmc_state_enum status = FMC_READY;
	BOOL eraseStatus = TRUE;

	DI();				// 关闭总中断
	fmc_unlock();		// 解锁

	// 清除所有挂起标志位
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	// 擦除APP所有页
	for (i = (IAP_FLASH_ADRESS_APP2_OFFSET/IAP_STM32_PAGE_SIZE); i < ((IAP_FLASH_ADRESS_APP2_OFFSET + IAP_GD32_APP2_SIZE)/IAP_STM32_PAGE_SIZE); i++)
	{
		status = fmc_page_erase(i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

		if (FMC_READY != status)
		{
			eraseStatus = FALSE;

			break;
		}
		
		// 清看门狗
		WDT_Clear();
	}

	fmc_lock();			// 上锁
	EI(); 				// 开启总中断

	return eraseStatus;
}

/*******************************************************************************
  * @函数名称	IAP_FlashReadHalfWord
  * @函数说明   读取指定地址的半字(16位数据)
  * @输入参数   faddr:FLASH当前地址
  * @返回参数   从当前FLASH地址开始，向后读取16个字节（半字）的数据：*(vu16*)faddr，注意地址必须为2的倍数
*******************************************************************************/
uint16 IAP_FlashReadHalfWord(uint32 flashAddr)
{
	return *(volatile uint16*)flashAddr; 
}

/*******************************************************************************
  * @函数名称	IAP_FlashWriteHalfWordArrayAndCheck
  * @函数说明   向FLASH中写入指定长度的数据，辅助使用库函数FLASH_ProgramHalfWord，每次写入半字，并且加入了写入数据校验
  * @输入参数   writeAddr:应用程序的起始地址 pBuffer：整个扇区的内容 numToWrite:写入半字的数目
  * @返回参数   BOOL,如果为TRUE,则该部分写入和校验成功，否则失败
----注意:此函数不能单独调用使用，因为没有解锁上锁-------------------------------
*******************************************************************************/
BOOL IAP_FlashWriteHalfWordArrayAndCheck(uint32 writeAddr, uint16 *pBuffer, uint16 numToWrite)   
{ 			 		 
	uint16 i;
	uint16 readHalfWord;

	// 参数判断
	if (NULL == pBuffer)
	{
		return FALSE;
	}

	// 参数检验是否非法
	if ((writeAddr < IAP_GD32_FLASH_BASE) || (writeAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// 非法地址
	}
	
	for (i = 0; i < numToWrite; i++)
	{
		fmc_halfword_program(writeAddr, pBuffer[i]);		// 写
		readHalfWord = IAP_FlashReadHalfWord(writeAddr);	// 读
	  	writeAddr += 2;

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 
		
	  	// 写入FLASH中的缓冲区数据与读取的数据是否匹配
		if (readHalfWord == pBuffer[i])
		{
			continue;
		}
		else
		{
			return FALSE;
		}
	}  
	return TRUE;
} 

/*******************************************************************************
  * @函数名称	IAP_FlashWriteHalfWordArray
  * @函数说明   将指定大小的缓冲区数据按页写入FLASH当中
  * @输入参数   writeAddr:写入地址 pBuffer:缓冲区数据首地址 numToWrite:需要写入的长度（单位：半字）
  * @返回参数   BOOL型的变量writeSucceed
*******************************************************************************/
BOOL IAP_FlashWriteHalfWordArray(uint32 writeAddr, uint16 *pBuffer, uint16 numToWrite)	
{
#if 0

	uint32 secpos;	    // 扇区地址
	uint16 secoff;	    // 扇区内偏移地址(16位字计算)
	uint16 secremain;  	// 扇区内剩余地址(16位字计算)	   
 	uint16 i;    
	uint32 offaddr;    	// 去掉0X08000000后的地址
	BOOL writeStatus = FALSE;
	FLASH_Status status = FLASH_COMPLETE;
	static uint16 iapFlashBuf[IAP_STM32_PAGE_SIZE/2];	// 最多是2K字节

	// 参数检验是否非法
	if ((writeAddr < IAP_GD32_FLASH_BASE) || (writeAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// 非法地址
	}
	
	DI();			// 关闭总中断
	FLASH_Unlock();	// 解锁

	// 清除所有挂起标志位
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	
	offaddr = writeAddr-IAP_GD32_FLASH_BASE;			// 实际偏移地址.
	secpos = offaddr/IAP_STM32_PAGE_SIZE;				// 扇区地址  0~127 for STM32F103RBT6
	secoff = (offaddr%IAP_STM32_PAGE_SIZE)/2;			// 在扇区内的偏移(2个字节为基本单位.)
	secremain = IAP_STM32_PAGE_SIZE/2 - secoff;			// 扇区剩余空间大小 

	// 不大于该扇区范围
	if (numToWrite <= secremain) 
	{
		secremain = numToWrite;
	}
	
	while (1)
	{
		// 读出整个扇区的内容
		IAP_FlashReadHalfWordArray((secpos*IAP_STM32_PAGE_SIZE+IAP_GD32_FLASH_BASE),
									iapFlashBuf,
									IAP_STM32_PAGE_SIZE/2);

		// 校验数据
		for (i = 0; i < secremain; i++)
		{
			if (iapFlashBuf[secoff + i] != 0xFFFF)
			{
				// 里面的数据不全是0xFFFF，说明需要擦除
				break;
			}
		}

		// 需要擦除
		if (i < secremain)
		{
			// 擦除这个扇区
			status = FLASH_ErasePage(secpos*IAP_STM32_PAGE_SIZE+IAP_STM32_FLASH_BASE);

			if (FLASH_COMPLETE != status)
			{
				return FALSE;
			}
			
			for (i = 0; i < secremain; i++) // 复制
			{
				iapFlashBuf[i + secoff] = pBuffer[i];	  
			}

			// 写入整个扇区
			writeStatus = IAP_FlashWriteHalfWordArrayAndCheck((secpos*IAP_STM32_PAGE_SIZE+IAP_GD32_FLASH_BASE), 
																iapFlashBuf, 
																IAP_STM32_PAGE_SIZE/2);
		}
		// 不需要擦除
		else
		{
			writeStatus = IAP_FlashWriteHalfWordArrayAndCheck(writeAddr,pBuffer,secremain); //写已经擦除了的,直接写入扇区剩余区间. 
		}
		
		if (!writeStatus)
		{
			break;
		}
		
		// 写入结束了
		if (numToWrite == secremain)
		{
			break;
		}
		// 写入未结束
		else 
		{
			secpos++;							// 扇区地址增1
			secoff = 0;							// 偏移位置为0 	 
		 	pBuffer += secremain;  	     		// 指针偏移
			writeAddr += secremain;				// 写地址偏移	   
		  	numToWrite -= secremain;			// 字节(16位)数递减

		  	// 下一个扇区还是写不完
			if (numToWrite > (IAP_STM32_PAGE_SIZE/2))
			{
				secremain = IAP_STM32_PAGE_SIZE/2;	
			}
			// 下一个扇区可以写完了
			else 
			{
				secremain = numToWrite;
			}
		}	 
	}
	
	FLASH_Lock();		// 上锁
	EI(); 				// 开启总中断
	
	return writeStatus;

#else
	BOOL writeStatus = TRUE;
	
	// 参数判断
	if (NULL == pBuffer)
	{
		return FALSE;
	}

	// 参数检验是否非法
	if ((writeAddr < IAP_GD32_FLASH_BASE) || (writeAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// 非法地址
	}
	
	DI();				// 关闭总中断
	fmc_unlock();		// 解锁

	// 清除所有挂起标志位
	fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	// 写入页
	if (!IAP_FlashWriteHalfWordArrayAndCheck(writeAddr, pBuffer, numToWrite))	
	{
		writeStatus = FALSE;
	}

	fmc_lock();		// 上锁
	EI(); 			// 开启总中断
	
	return writeStatus;

#endif
}

/*******************************************************************************
  * @函数名称	IAP_FlashReadHalfWordArray
  * @函数说明   从指定地址开始读出指定长度的数据
  * @输入参数   readAddr:读取地址 pBuffer:缓冲区数据首地址 numToWrite:需要读入的长度（单位：半字）
  * @返回参数   无
*******************************************************************************/
void IAP_FlashReadHalfWordArray(uint32 readAddr, uint16 *pBuffer, uint16 numToRead)   	
{
	uint16 i;

	// 参数判断
	if (NULL == pBuffer)
	{
		return;
	}

	// 参数检验是否非法
	if ((readAddr < IAP_GD32_FLASH_BASE) || (readAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return;	// 非法地址
	}
	
	for (i = 0; i < numToRead; i++)
	{
		pBuffer[i] = IAP_FlashReadHalfWord(readAddr);	// 读取2个字节.
		readAddr += 2; 									// 偏移2个字节.	
	}
}


/*******************************************************************************
  * @函数名称	IAP_FlashWriteWordArray
  * @函数说明   将指定大小的缓冲区数据按页写入FLASH当中
  * @输入参数   writeAddr:写入地址 pBuffer:缓冲区数据首地址 numToWrite:需要写入的长度（单位：字）
  * @返回参数   BOOL型的变量writeSucceed
*******************************************************************************/
BOOL IAP_FlashWriteWordArray(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite)	
{
	BOOL writeStatus = TRUE;
	
	// 参数判断
	if (NULL == pBuffer)
	{
		return FALSE;
	}

	// 参数检验是否非法
	if ((writeAddr < IAP_GD32_FLASH_BASE) || ((writeAddr + numToWrite) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// 非法地址
	}
	
	DI();				// 关闭总中断
	fmc_unlock();		// 解锁

	// 清除所有挂起标志位
	fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);


	// 写入页
	if (!IAP_FlashWriteWordArrayAndCheck(writeAddr, pBuffer, numToWrite))	
	{
		writeStatus = FALSE;
	}

	fmc_lock();			// 上锁
	EI(); 				// 开启总中断
	
	return writeStatus;
}

/*******************************************************************************
  * @函数名称	IAP_FlashWriteHalfWordArrayAndCheck
  * @函数说明   向FLASH中写入指定长度的数据，辅助使用库函数IAP_WordProgram，每次写入字，并且加入了写入数据校验
  * @输入参数   writeAddr:应用程序的起始地址 pBuffer：整个扇区的内容 numToWrite:（单位：字）
  * @返回参数   BOOL,如果为TRUE,则该部分写入和校验成功，否则失败
----注意:此函数不能单独调用使用，因为没有解锁上锁-------------------------------
*******************************************************************************/
BOOL IAP_FlashWriteWordArrayAndCheck(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite)   
{ 			 		 
	uint16 i;
	uint32 readWord;

	// 参数判断
	if (NULL == pBuffer)
	{
		return FALSE;
	}

	// 参数检验是否非法
	if ((writeAddr < IAP_GD32_FLASH_BASE) || ((writeAddr + numToWrite) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE+1024UL)))
	{
		return FALSE;	// 非法地址
	}
	
	for (i = 0; i < numToWrite; i++)
	{
		fmc_word_program(writeAddr, pBuffer[i]);		// 写
		readWord = IAP_FlashReadWord(writeAddr);		// 读
	  	writeAddr += 4;

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 

	  	// 写入FLASH中的缓冲区数据与读取的数据是否匹配
		if (readWord == pBuffer[i])
		{
			continue;
		}
		else
		{
			return FALSE;
		}
	}  
	return TRUE;
} 

/*******************************************************************************
  * @函数名称	IAP_FlashReadHalfWordArray
  * @函数说明   从指定地址开始读出指定长度的数据
  * @输入参数   readAddr:读取地址 pBuffer:缓冲区数据首地址 numToWrite:需要读入的长度（单位：半字）
  * @返回参数   无
*******************************************************************************/
void IAP_FlashReadWordArray(uint32 readAddr, uint32 *pBuffer, uint16 numToRead)   	
{
	uint16 i;

	// 参数判断
	if (NULL == pBuffer)
	{
		return;
	}

	// 参数检验是否非法
	if ((readAddr < IAP_GD32_FLASH_BASE) || ((readAddr + numToRead) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE+1024UL)))
	{
		return;	// 非法地址
	}
	
	for (i = 0; i < numToRead; i++)
	{
		pBuffer[i] = IAP_FlashReadWord(readAddr);	// 读取4个字节.
		readAddr += 4;								// 偏移4个字节.	
	}
}

/*******************************************************************************
  * @函数名称	IAP_FlashReadHalfWord
  * @函数说明   读取指定地址的字(32位数据)
  * @输入参数   faddr:FLASH当前地址
  * @返回参数   从当前FLASH地址开始，向后读取32个位（字）的数据：*(uint32*)faddr，注意地址必须为4的倍数
*******************************************************************************/
uint32 IAP_FlashReadWord(uint32 flashAddr)
{
	return *(volatile uint32*)flashAddr; 
}

/*******************************************************************************
  * @函数名称	IAP_FlashWriteWordArrayWithErase
  * @函数说明   将指定大小的缓冲区数据按页写入FLASH当中
  * @输入参数   writeAddr:写入地址 pBuffer:缓冲区数据首地址 numToWrite:需要写入的长度（单位：字）
  * @返回参数   BOOL型的变量writeSucceed
*******************************************************************************/
BOOL IAP_FlashWriteWordArrayWithErase(uint32 writeAddr, uint32 *pBuffer, uint16 NumWordToWrite)	
{
	BOOL writeStatus = TRUE;
	uint32 pagePos = 0;						// 扇区地址
	uint16 pageOffsent = 0;					// 在扇区内的偏移
	uint16 pageRemain = 0;  				// 扇区剩余空间大小 
	uint32 offaddr;    						// 去掉0X08000000后的地址
 	uint16 i = 0;
 	uint32 * pFlashBuff = NULL;				// 保存读取扇区的数据
 	uint16 numByteToWrite = NumWordToWrite * 4;
	
	// 参数判断
	if (NULL == pBuffer|| (NULL == numByteToWrite))
	{
		return FALSE;
	}

	// 参数检验是否非法
	if ((writeAddr < IAP_GD32_FLASH_BASE) || ((writeAddr + numByteToWrite) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE + 1024UL)))
	{
		return FALSE;	// 非法地址
	}

	// 申请1024Byte空间保存页数据
	pFlashBuff = (uint32 *)malloc(IAP_STM32_PAGE_SIZE);
	if (NULL == pFlashBuff)
	{
		return FALSE;
	}

	offaddr = writeAddr - IAP_GD32_FLASH_BASE;		// 实际偏移地址
	pagePos = offaddr / IAP_STM32_PAGE_SIZE;		// 页地址
	pageOffsent = offaddr % IAP_STM32_PAGE_SIZE;	// 在页内偏移
	pageRemain = IAP_STM32_PAGE_SIZE - pageOffsent;	// 页剩余空间大小

	if (numByteToWrite <= pageRemain)
	{
		pageRemain = numByteToWrite;				// 不大于该页范围
	}
	
	DI();				// 关闭总中断
	fmc_unlock();		// 解锁

	// 清除所有挂起标志位
	fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

	
	while(1)
	{
		// 读出整个页的内容
		IAP_FlashReadWordArray(pagePos*IAP_STM32_PAGE_SIZE+IAP_STM32_PAGE_SIZE, pFlashBuff, IAP_STM32_PAGE_SIZE/4);

		for (i = 0; i < pageRemain/4; i++)	// 校验数据
		{
			if (pFlashBuff[pageOffsent/4 + i] != 0xFFFFFFFF)
			{
				// 里面的数据不全是0xFFFFFFFF，说明需要擦除
				break;
			}
		}

		if (i < pageRemain/4)					// 需要擦除
		{
			// 擦除这个页
			//IAP_PageErase(pagePos * IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);
			fmc_page_erase(pagePos * IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);

			// 复制
			for (i = 0; i < pageRemain/4; i++)
			{
				pFlashBuff[i + pageOffsent/4] = pBuffer[i]; 
			}

			// 写入整页
			if (!IAP_FlashWriteWordArrayAndCheck(pagePos * IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE, pFlashBuff, IAP_STM32_PAGE_SIZE/4))	
			{
				// 释放数据缓存区
				free(pFlashBuff);

				fmc_lock();			// 上锁
				EI();				// 开启总中断
				writeStatus = FALSE;
				return writeStatus;
			}
		}
		else 
		{
			// 写已经擦除了的,直接写入扇区剩余区间.
			if (!IAP_FlashWriteWordArrayAndCheck(writeAddr, pBuffer, pageRemain/4))	
			{
				// 释放数据缓存区
				free(pFlashBuff);

				fmc_lock();			// 上锁
				EI(); 				// 开启总中断
				writeStatus = FALSE;
				return writeStatus;
			}
		}
		
		if (numByteToWrite == pageRemain)
		{
			break;									// 写入结束了
		}
		else										// 写入未结束
		{
			pagePos++;								// 扇区地址增1
			pageOffsent = 0;						// 偏移位置为0 	 

		   	pBuffer += (pageRemain/4);  			// 指针偏移
			writeAddr += pageRemain;				// 写地址偏移	   
		   	numByteToWrite -= pageRemain;			// 字节数递减
		   	
			if (numByteToWrite > IAP_STM32_PAGE_SIZE)
			{
				pageRemain = IAP_STM32_PAGE_SIZE;	// 下一个扇区还是写不完
			}
			else
			{
				pageRemain = numByteToWrite;		// 下一个扇区可以写完了	
			}
		}	
		
	}

	// 释放数据缓存区
	free(pFlashBuff);	
	fmc_lock();			// 上锁
	EI(); 				// 开启总中断
	return writeStatus;
}

/*******************************************************************************
  * @函数名称	IAP_FlashReadForCrc8
  * @函数说明   读出写入flash的数据进行crc8校验
  * @输入参数   数据长度，只能是4的倍数
  * @返回参数   返回crc8的校验结果
*******************************************************************************/
uint8 IAP_FlashReadForCrc8(uint32 datalen)
{
	uint8 crc = 0;
	uint8 i,j;
	uint8 buf[4] = {0};
	uint32 addrIndex = 0;
	uint32 dataTemp;
	uint32 len = datalen/4;

	while (len--)
	{
		dataTemp = *(volatile uint32*)(addrIndex + IAP_FLASH_APP2_ADDR);
		addrIndex += 4;
		// 小端模式
		buf[3] = (uint8)(dataTemp >> 24);
		buf[2] = (uint8)(dataTemp >> 16);
		buf[1] = (uint8)(dataTemp >> 8);
		buf[0] = (uint8)dataTemp;
		
		for(j = 0; j < 4; j++)
		{
			crc ^= buf[j];
			
			for (i = 0; i < 8; i++)
			{
				if (crc & 0x01)
				{
					crc = (crc >> 1) ^ 0x8C;
				}
				else
				{
					crc >>= 1;
				}
			}
		}
	}
	return crc;
}


