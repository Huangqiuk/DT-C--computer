#ifndef __EEPROM__UNIVERSAL_H__
#define __EEPROM__UNIVERSAL_H__

#include "common.h"

// E2PROM单元测试开关
#define E2PROM_UNIT_TEST						0
// 选择硬件EEPROM类型
#define EEPROM_TYPE 							EEPROM_24C64

#define E2PROM_MALLOC_SAVE_SPACE_COUNT			10				// 空间换时间的数量，用户根据计算得出所需寿命对应的空间，比如一个块使用80万次，则总次数=10*80=800万次
#define E2PROM_ERASE_TIMES						800000UL		// 80万次
#define E2PROM_DISPOSABIE_DATA_LENGTH_MAX		EE_TOTAL_SIZE	// 一次性能传的字节个数，根据硬件EEPROM类型进行设置，理论上是EEPROM的最大size
#define E2PROM_LOCKED_KEY						0xA8			// 密匙

typedef enum
{
	E2PROM_ID_NULL = -1,

	E2PROM_ID_WHELL_SIZE1,							// 轮径
	E2PROM_ID_WHELL_SIZE2,							// 轮径
	E2PROM_ID_WHELL_SIZE3,							// 轮径

	E2PROM_ID_WHELL_SIZE4,							// 轮径
	E2PROM_ID_WHELL_SIZE5,							// 轮径
	E2PROM_ID_WHELL_SIZE6,							// 轮径

	E2PROM_ID_MAX
}E2PROM_ID;

typedef enum
{
	E2PROM_ACCESS_MODE_NOT_OFTEN,				// 不经常改写
	E2PROM_ACCESS_MODE_QUITE_OFTEN,				// 经常改写
}E2PROM_ACCESS_MODE_E;

// E2PROM驱动I2C端口定义(用户根据自己端口进行修改)
#define E2PROM_SCL_DIR_OUT()
#define E2PROM_SCL_DIR_IN()
#define E2PROM_SDA_DIR_OUT()
#define E2PROM_SDA_DIR_IN()						E2PROM_SDA_HIGH()	// 开漏模式，输出1后即可读取外部电平

#define E2PROM_SCL_HIGH()						gpio_bit_write(GPIOC, GPIO_PIN_5, SET)
#define E2PROM_SCL_LOW()						gpio_bit_write(GPIOC, GPIO_PIN_5, RESET)
#define E2PROM_SDA_HIGH()						gpio_bit_write(GPIOC, GPIO_PIN_6, SET)
#define E2PROM_SDA_LOW()						gpio_bit_write(GPIOC, GPIO_PIN_6, RESET)
#define E2PROM_SDA_GET()						gpio_input_bit_get(GPIOC, GPIO_PIN_6)
#define E2PROM_SDA_SET(i)						gpio_bit_write(GPIOC, GPIO_PIN_6, (bit_status)(i))


// 记忆块的起始保存地址
#define E2PROM_MEMORY_BLOCK_START_ADDR			0x05
// 记忆块大小，单位:Byte
#define E2PROM_MEMORY_BLOCK_SIZE				5

// 系统参数保存地址的偏移量，发送端一侧认为地址从0开始，接收端一侧从此偏移地址开始为发送端专用空间
#define E2PROM_SYSTEM_PARAM_ADDR_OFFSET			10

// 延时设置(用户根据自己需要进行调整)
#define	E2PROM_DELAY_SWITCH 					1		 // 延时方式选择0:简短方式延时NOP()   1:子函数延时E2PROM_Delayus(i)
#if E2PROM_DELAY_SWITCH
#define	E2PROM_DELAY()							do{\
													E2PROM_Delayus(5);\
												}while(0)
#define	E2PROM_DELAY_MS()						E2PROM_Delayms(5)
#else
#define	E2PROM_DELAY()							do{\
													NOP();\
													NOP();\
												}while(0)
#define	E2PROM_DELAY_MS()						E2PROM_Delayms(5)
#endif

// =======================================================================================
// EEPROM_24CXX支持类型
#define EEPROM_24C01		0
#define EEPROM_24C02		1
#define EEPROM_24C04		2
#define EEPROM_24C08		3
#define EEPROM_24C16		4
#define EEPROM_24C32		5
#define EEPROM_24C64		6
#define EEPROM_24C128		7
#define EEPROM_24C256		8
#define EEPROM_24C512		9

// EEPROM芯片类型选择
#if (EEPROM_24C01 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	8
	#define EE_TOTAL_SIZE	0x007F
#elif (EEPROM_24C02 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	8 
	#define EE_TOTAL_SIZE	0x00FF
#elif (EEPROM_24C04 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	16
	#define EE_TOTAL_SIZE	0x01FF
#elif (EEPROM_24C08 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	16
	#define EE_TOTAL_SIZE	0x03FF
#elif (EEPROM_24C16 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	16
	#define EE_TOTAL_SIZE	0x07FF
#elif (EEPROM_24C32 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	32
	#define EE_TOTAL_SIZE	0x0FFF
#elif (EEPROM_24C64 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	32
	#define EE_TOTAL_SIZE	0x1FFF
#elif (EEPROM_24C128 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	64
	#define EE_TOTAL_SIZE	0x3FFF
#elif (EEPROM_24C256 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	64
	#define EE_TOTAL_SIZE	0x7FFF
#elif (EEPROM_24C512 == EEPROM_TYPE)
	#define EE_PAGE_SIZE	128
	#define EE_TOTAL_SIZE	0xFFFF
#endif 

// 器件地址 EEPROM
#define	EEPROM_DEVICE_ADD	0xA0	
/* I2C write and read command */
#define EEPROM_WRCOMMAND	(EEPROM_DEVICE_ADD & 0xFE)
#define EEPROM_RDCOMMAND	(EEPROM_DEVICE_ADD | 0x01)

#define E2PROM_I2C_OUTPUT	1
#define E2PROM_I2C_INPUT	0
#define E2PROM_I2C_NACK		1
#define E2PROM_I2C_ACK		0

// 记录条
typedef struct{
	uint16 addr;
	uint16 length;
	uint8 index;
	uint32 count;
	E2PROM_ACCESS_MODE_E writeMode;

    BOOL isExist;
	
}E2PROM_RECORD;

typedef struct
{
	E2PROM_RECORD record[E2PROM_ID_MAX];

	uint32 size;
	
}E2PROM_CB;

extern E2PROM_CB e2promCB;


// 单元测试
void E2PROM_UnitTest(void);
// ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

// E2PROM初始化
void E2PROM_Init(void);

// 写数据，整个地址内，无需考虑页问题
BOOL E2PROM_WriteByteArray(uint16 writeAddr, uint8* pBuf, uint16 writeNum);

// 读数据，整个地址内，无需考虑页问题
BOOL E2PROM_ReadByteArray(uint16 ReadAddr, uint8* pBuf, uint16 readNum);

// 读取一个字节
uint8 E2PROM_ReadByte(uint16 ReadAddr);

// 读取一个双字节数值
uint16 E2PROM_ReadInt(uint16 ReadAddr);

// 读取一个四字节数值
uint32 E2PROM_ReadWord(uint16 ReadAddr);

// 写入一个字节
void E2PROM_WriteByte(uint16 writeAddr, uint8 data);

// 写入一个双字节数值
void E2PROM_WriteInt(uint16 writeAddr, uint16 data);

// 写入一个四字节数值
void E2PROM_WriteWord(uint16 writeAddr, uint32 data);


// ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

// ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

// 服务数据初始化
void E2PROM_ServiceInit(void);

// 注册相应ID的长度以及是否需要频繁擦写模式
// 此函数要在大循环之前注册方可使用
void E2PROM_ServiceRegister(E2PROM_ID id, uint16 length, E2PROM_ACCESS_MODE_E mode);

// 业务层写
void E2PROM_ServiceWriteArray(E2PROM_ID id, uint8* pBuf);

// 业务层读
void E2PROM_ServiceReadArray(E2PROM_ID id, uint8* pBuf);

// 业务层获取相应ID存取的地址
uint16 E2PROM_ServiceGetIDAddr(E2PROM_ID id);

// 业务层获取使用的总空间大小
uint32 E2PROM_ServiceGetRomSize(void);

//===================================================================
//=====2017.7.15新增功能=============================================
// 注册长度为一个字节的ID的以及是否需要频繁擦写模式
void E2PROM_ServiceRegister8bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode);

// 注册长度为两个字节的ID的以及是否需要频繁擦写模式
void E2PROM_ServiceRegister16bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode);

// 注册长度为四个字节的ID的以及是否需要频繁擦写模式
void E2PROM_ServiceRegister32bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode);

// 业务层写一个字节
void E2PROM_ServiceWrite8bit(E2PROM_ID id, uint8 data);

// 业务层写一个双字节
void E2PROM_ServiceWrite16bit(E2PROM_ID id, uint16 data);

// 业务层写一个四字节
void E2PROM_ServiceWrite32bit(E2PROM_ID id, uint32 data);

// 业务层读一个字节
uint8 E2PROM_ServiceRead8bit(E2PROM_ID id);

// 业务层读一个双字节
uint16 E2PROM_ServiceRead16bit(E2PROM_ID id);

// 业务层读一个四字节
uint32 E2PROM_ServiceRead32bit(E2PROM_ID id);

// 根据ID读取数值
uint32 E2PROM_ServiceReadByID(E2PROM_ID id);

// 根据ID写入数值
void E2PROM_ServiceWriteByID(E2PROM_ID id, uint32 param);
#endif


