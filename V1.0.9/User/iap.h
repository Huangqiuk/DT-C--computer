/**********************************************************************************************************************************
	Description:
	1.首先在Bootloader（引导程序）的options for target中设置好FLASH的起始位置,如0x08000000~0x08010000,size = 10000;
	2.在引导程序的main主函数中设置中断向量起始地址：NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000);
	3.设置用户app应用程序的起始地址和范围，如0x08010000~0x08090000,size = 80000;
	4.设置用户app程序的中断向量起始地址，NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x10000);

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
#ifndef __IAP_H__
#define __IAP_H__
 
#include "common.h"

// 扇区数从0开始到UI_SECTOR_NUMBER位UI专用
#define UI_SECTOR_NUMBER								1152				// 例如UI用了4.5M=4.5*1024*1024/SPI_FLASH_SECTOR_SIZE=1152个扇区

// 二维码字符串的起始保存地址
//#define PARAM_NVM_QR_START_ADDR							0x0000
//#define PARAM_NVM_QR_SIZE								32					// 没有二维码字符，长度设为0，否则二维码字符串固定32个字符

// 码表类型在E2PROM中的地址
#define PARAM_NVM_HMI_TYPE_START_ADDRESS				(PARAM_NVM_QR_START_ADDR+PARAM_NVM_QR_SIZE)
#define PARAM_NVM_HMI_TYPE_SIZE							6	

// GPS类型在E2PROM中的地址
#define PARAM_NVM_GPS_TYPE_START_ADDRESS				(PARAM_NVM_HMI_TYPE_START_ADDRESS+PARAM_NVM_HMI_TYPE_SIZE)
#define PARAM_NVM_GPS_TYPE_SIZE							1

//===========================================================================================================================
// 用户务必根据自己所选芯片型号设置FLASH大小
#define IAP_GD32_FLASH_SIZE 			256*1024UL			// 所选STM32的FLASH容量大小(单位:字节)



// FLASH地址
#define IAP_GD32_FLASH_BASE  			0x08000000UL 	    	// STM32 FLASH的起始地址
#define IAP_FLASH_ADRESS_APP1_OFFSET	0x8000 					// 地址偏移,为IAP留出32K空间，此地址偏移必须为页IAP_STM32_PAGE_SIZE的整数倍
#define IAP_GD32_APP1_SIZE				(IAP_GD32_FLASH_SIZE-IAP_FLASH_ADRESS_APP1_OFFSET)				// APP1大小
#define IAP_FLASH_APP1_ADDR				(IAP_GD32_FLASH_BASE+IAP_FLASH_ADRESS_APP1_OFFSET)     // APP1起始地址

#define IAP_FLASH_ADRESS_APP2_OFFSET	0x10000 				// 地址偏移,为IAP+APP1留出64K空间，此地址偏移必须为页IAP_STM32_PAGE_SIZE的整数倍
#define IAP_GD32_APP2_SIZE				(IAP_GD32_FLASH_SIZE-IAP_FLASH_ADRESS_APP2_OFFSET)		// APP2大小
#define IAP_FLASH_APP2_ADDR				(IAP_GD32_FLASH_BASE+IAP_FLASH_ADRESS_APP2_OFFSET)     // APP2起始地址


// 以下宏由开发人员修改，用户不需操作
// 判断所使用的FLASH大小，不同型号芯片，库函数每页所占的空间大小也不相同
#if IAP_GD32_FLASH_SIZE <= 256*1024UL
	#define IAP_STM32_PAGE_SIZE 		2048UL // 字节
#else 
	#define IAP_STM32_PAGE_SIZE			4096UL
#endif	

// IAP控制结构体定义
typedef struct
{
	BOOL appValid;			// APP数据是否可用
}IAP_CB;

extern IAP_CB iapCB;


//【外部调用函数声明】
void IAP_Init(void);

BOOL IAP_WriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appSize);    

BOOL IAP_BleWriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appSize);

// 查询Flash是否为空
BOOL IAP_CheckFlashIsBlank(void);

// 擦除APP所有页
BOOL IAP_EraseAPPArea(void);

void IAP_JumpToAppFun(uint32 param);

// 判断指定地址APP是否合法
uint8 IAP_CheckAppRightful(uint32 addr);

// 擦除APP1所有页
BOOL IAP_EraseAPP1Area(void);

// 擦除APP2所有页
BOOL IAP_EraseAPP2Area(void);

// 查询Flash是否为空
BOOL IAP_CheckApp1FlashIsBlank(void);

// 查询Flash是否为空
BOOL IAP_CheckApp2FlashIsBlank(void);

// 带擦除功能的写数据
BOOL IAP_FlashWriteWordArrayWithErase(uint32 writeAddr, uint32 *pBuffer, uint16 NumWordToWrite)	;

// 读出flash进行校验
uint8 IAP_FlashReadForCrc8(uint32 datalen);

#endif

