#ifndef __EEPROM__UNIVERSAL_H__
#define __EEPROM__UNIVERSAL_H__

#include "common.h"

// E2PROM��Ԫ���Կ���
#define E2PROM_UNIT_TEST						0
// ѡ��Ӳ��EEPROM����
#define EEPROM_TYPE 							EEPROM_24C64

#define E2PROM_MALLOC_SAVE_SPACE_COUNT			10				// �ռ任ʱ����������û����ݼ���ó�����������Ӧ�Ŀռ䣬����һ����ʹ��80��Σ����ܴ���=10*80=800���
#define E2PROM_ERASE_TIMES						800000UL		// 80���
#define E2PROM_DISPOSABIE_DATA_LENGTH_MAX		EE_TOTAL_SIZE	// һ�����ܴ����ֽڸ���������Ӳ��EEPROM���ͽ������ã���������EEPROM�����size
#define E2PROM_LOCKED_KEY						0xA8			// �ܳ�

typedef enum
{
	E2PROM_ID_NULL = -1,

	E2PROM_ID_WHELL_SIZE1,							// �־�
	E2PROM_ID_WHELL_SIZE2,							// �־�
	E2PROM_ID_WHELL_SIZE3,							// �־�

	E2PROM_ID_WHELL_SIZE4,							// �־�
	E2PROM_ID_WHELL_SIZE5,							// �־�
	E2PROM_ID_WHELL_SIZE6,							// �־�

	E2PROM_ID_MAX
}E2PROM_ID;

typedef enum
{
	E2PROM_ACCESS_MODE_NOT_OFTEN,				// ��������д
	E2PROM_ACCESS_MODE_QUITE_OFTEN,				// ������д
}E2PROM_ACCESS_MODE_E;

// E2PROM����I2C�˿ڶ���(�û������Լ��˿ڽ����޸�)
#define E2PROM_SCL_DIR_OUT()
#define E2PROM_SCL_DIR_IN()
#define E2PROM_SDA_DIR_OUT()
#define E2PROM_SDA_DIR_IN()						E2PROM_SDA_HIGH()	// ��©ģʽ�����1�󼴿ɶ�ȡ�ⲿ��ƽ

#define E2PROM_SCL_HIGH()						gpio_bit_write(GPIOC, GPIO_PIN_5, SET)
#define E2PROM_SCL_LOW()						gpio_bit_write(GPIOC, GPIO_PIN_5, RESET)
#define E2PROM_SDA_HIGH()						gpio_bit_write(GPIOC, GPIO_PIN_6, SET)
#define E2PROM_SDA_LOW()						gpio_bit_write(GPIOC, GPIO_PIN_6, RESET)
#define E2PROM_SDA_GET()						gpio_input_bit_get(GPIOC, GPIO_PIN_6)
#define E2PROM_SDA_SET(i)						gpio_bit_write(GPIOC, GPIO_PIN_6, (bit_status)(i))


// ��������ʼ�����ַ
#define E2PROM_MEMORY_BLOCK_START_ADDR			0x05
// ������С����λ:Byte
#define E2PROM_MEMORY_BLOCK_SIZE				5

// ϵͳ���������ַ��ƫ���������Ͷ�һ����Ϊ��ַ��0��ʼ�����ն�һ��Ӵ�ƫ�Ƶ�ַ��ʼΪ���Ͷ�ר�ÿռ�
#define E2PROM_SYSTEM_PARAM_ADDR_OFFSET			10

// ��ʱ����(�û������Լ���Ҫ���е���)
#define	E2PROM_DELAY_SWITCH 					1		 // ��ʱ��ʽѡ��0:��̷�ʽ��ʱNOP()   1:�Ӻ�����ʱE2PROM_Delayus(i)
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
// EEPROM_24CXX֧������
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

// EEPROMоƬ����ѡ��
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

// ������ַ EEPROM
#define	EEPROM_DEVICE_ADD	0xA0	
/* I2C write and read command */
#define EEPROM_WRCOMMAND	(EEPROM_DEVICE_ADD & 0xFE)
#define EEPROM_RDCOMMAND	(EEPROM_DEVICE_ADD | 0x01)

#define E2PROM_I2C_OUTPUT	1
#define E2PROM_I2C_INPUT	0
#define E2PROM_I2C_NACK		1
#define E2PROM_I2C_ACK		0

// ��¼��
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


// ��Ԫ����
void E2PROM_UnitTest(void);
// ��������������������������������������������������������������������������������������
// ��������������������������������������������������������������������������������������

// E2PROM��ʼ��
void E2PROM_Init(void);

// д���ݣ�������ַ�ڣ����迼��ҳ����
BOOL E2PROM_WriteByteArray(uint16 writeAddr, uint8* pBuf, uint16 writeNum);

// �����ݣ�������ַ�ڣ����迼��ҳ����
BOOL E2PROM_ReadByteArray(uint16 ReadAddr, uint8* pBuf, uint16 readNum);

// ��ȡһ���ֽ�
uint8 E2PROM_ReadByte(uint16 ReadAddr);

// ��ȡһ��˫�ֽ���ֵ
uint16 E2PROM_ReadInt(uint16 ReadAddr);

// ��ȡһ�����ֽ���ֵ
uint32 E2PROM_ReadWord(uint16 ReadAddr);

// д��һ���ֽ�
void E2PROM_WriteByte(uint16 writeAddr, uint8 data);

// д��һ��˫�ֽ���ֵ
void E2PROM_WriteInt(uint16 writeAddr, uint16 data);

// д��һ�����ֽ���ֵ
void E2PROM_WriteWord(uint16 writeAddr, uint32 data);


// ��������������������������������������������������������������������������������������

// ��������������������������������������������������������������������������������������

// �������ݳ�ʼ��
void E2PROM_ServiceInit(void);

// ע����ӦID�ĳ����Լ��Ƿ���ҪƵ����дģʽ
// �˺���Ҫ�ڴ�ѭ��֮ǰע�᷽��ʹ��
void E2PROM_ServiceRegister(E2PROM_ID id, uint16 length, E2PROM_ACCESS_MODE_E mode);

// ҵ���д
void E2PROM_ServiceWriteArray(E2PROM_ID id, uint8* pBuf);

// ҵ����
void E2PROM_ServiceReadArray(E2PROM_ID id, uint8* pBuf);

// ҵ����ȡ��ӦID��ȡ�ĵ�ַ
uint16 E2PROM_ServiceGetIDAddr(E2PROM_ID id);

// ҵ����ȡʹ�õ��ܿռ��С
uint32 E2PROM_ServiceGetRomSize(void);

//===================================================================
//=====2017.7.15��������=============================================
// ע�᳤��Ϊһ���ֽڵ�ID���Լ��Ƿ���ҪƵ����дģʽ
void E2PROM_ServiceRegister8bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode);

// ע�᳤��Ϊ�����ֽڵ�ID���Լ��Ƿ���ҪƵ����дģʽ
void E2PROM_ServiceRegister16bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode);

// ע�᳤��Ϊ�ĸ��ֽڵ�ID���Լ��Ƿ���ҪƵ����дģʽ
void E2PROM_ServiceRegister32bit(E2PROM_ID id, E2PROM_ACCESS_MODE_E mode);

// ҵ���дһ���ֽ�
void E2PROM_ServiceWrite8bit(E2PROM_ID id, uint8 data);

// ҵ���дһ��˫�ֽ�
void E2PROM_ServiceWrite16bit(E2PROM_ID id, uint16 data);

// ҵ���дһ�����ֽ�
void E2PROM_ServiceWrite32bit(E2PROM_ID id, uint32 data);

// ҵ����һ���ֽ�
uint8 E2PROM_ServiceRead8bit(E2PROM_ID id);

// ҵ����һ��˫�ֽ�
uint16 E2PROM_ServiceRead16bit(E2PROM_ID id);

// ҵ����һ�����ֽ�
uint32 E2PROM_ServiceRead32bit(E2PROM_ID id);

// ����ID��ȡ��ֵ
uint32 E2PROM_ServiceReadByID(E2PROM_ID id);

// ����IDд����ֵ
void E2PROM_ServiceWriteByID(E2PROM_ID id, uint32 param);
#endif


