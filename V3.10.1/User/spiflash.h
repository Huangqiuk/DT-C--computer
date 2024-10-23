#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#include "common.h"

// SPIxѡ��
#define STM32_SPIx_NUM_DEF				SPI0					// ѡ��SPI(�жϺ�ʱ�Ӷ���Ҫ�޸�)
#define STM32_SPIx_IRQn_DEF				SPI0_IRQn				// ѡ��SPI(�жϺ�ʱ�Ӷ���Ҫ�޸�)
#define STM32_SPIx_DR_Addr				((uint32)(&SPI_DATA(SPI0)))			// SPI���ݼĴ�����ַ

// ѡ��ȫ˫������˫��ֻ����
#define STM32_DMA_SPI_DIRECTION			0			// 0: 2Lines_RxOnly��1: 2Lines_FullDuplex

// SPI_FLASH��Ԫ���Կ���
#define SPI_FLASH_UNIT_TEST				0			// 0��ֹ��1ʹ��

#define SPI_FLASH_TYPE					W25X32		// spi_flash����ѡ��

// ����оƬģʽѡ��
#define SPI_FLASH_ERASE_MODE			2			// 0������1��Ƭ��2��(��Ƭ�������ã���Ϊ���UI����Ҳ�������������ַ�ʽ���Զ��ܿ�UI���ݲ���)

// ���º꿪����Ա�޸�
#define SPI_FLASH_RETRY_READ_TIME		200U		// ���Դ�����������ѭ��
#define SPI_FLASH_PAGE_SIZE				256			// ҳ�ߴ磬��λ:Byte
#define SPI_FLASH_SECTOR_SIZE			4096UL		// �ȳߴ磬��λ:Byte
#define SPI_FLASH_BLOCK_SIZE			65536UL		// ��ߴ磬��λ:Byte
#define SPI_FLASH_BLOCK_PER_16SECTORS	16			// ÿ16������Ϊһ����

#define SPI_FLASH_SECTOR_START_ADDR(X)	((X)*SPI_FLASH_SECTOR_SIZE) // ��n��������ַ
#define SPI_FLASH_BLOCK_START_ADDR(X)	((X)*SPI_FLASH_BLOCK_SIZE)	// ��n��������ַ

// Flash����
#define CMD_WRITE_ENABLE				0x06	// д��ʹ��ָ��
#define CMD_WRITE_REGISTER_ENABLE		0x50	// ����д״̬�Ĵ���
#define CMD_WRITE_STATUS_REGISTER		0x01	// д��״̬�Ĵ���
#define CMD_SECTOR_ERASE_4K				0x20	// Sector Erase instruction
#define CMD_CHIP_ERASE					0xC7	// Chip Erase instruction
#define CMD_PAGE_WRITE					0x02	// ҳд������
#define CMD_READ_BYTE					0x03	// Read from Memory instruction
#define CMD_READ_STATUS_REGISTER1		0x05	// Read Status Register instruction
#define BUSY_FLAG						0x01	// BUSY flag
// ���
#define CMD_FAST_READ_BYTE				0x0B	// ���
#define CMD_BLOCK_ERASE_32K 			0x52	// 32K�����
#define CMD_BLOCK_ERASE_64K 			0xD8	// 64K�����
#define CMD_WRITE_DISABLE				0x04	// д��ֹ
#define CMD_POWER_DOWN					0xB9	// ����
#define CMD_RELEASE_POWERDOWN_DEVICE_ID	0xAB	// �ͷŵ���/����ID
#define CMD_MANUFACT_DEVICE_ID			0x90	// ����/����ID
#define CMD_JEDEC_DEVICE_ID 			0x9F	// JEDEC ID

#define SPI_FLASH_CS_L()				gpio_bit_reset(GPIOA, GPIO_PIN_4)
#define SPI_FLASH_CS_H()				gpio_bit_set(GPIOA, GPIO_PIN_4)

// �������־
#define SPI_FLASH_TEST_FLAG_ADDEESS			((SPI_FLASH_SECTOR_NUM - 1)*SPI_FLASH_SECTOR_SIZE)
// BOOT�汾��ŵ�ַ
#define SPI_FLASH_BOOT_ADDEESS				((SPI_FLASH_SECTOR_NUM - 1)*SPI_FLASH_SECTOR_SIZE + 64)	
// APP��ŵ�ַ
#define SPI_FLASH_APP_ADDEESS				((SPI_FLASH_SECTOR_NUM - 1)*SPI_FLASH_SECTOR_SIZE + 96)	
// BLE������־��ַ
#define SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS	((SPI_FLASH_SECTOR_NUM - 1)*SPI_FLASH_SECTOR_SIZE + 128)
// BLE����CRC��ַ
#define SPI_FLASH_BLE_UPDATA_CRC_ADDEESS	((SPI_FLASH_SECTOR_NUM - 1)*SPI_FLASH_SECTOR_SIZE + 136)

// UI�汾����
#define SPI_FLASH_UI_VERSION_ADDEESS		((SPI_FLASH_SECTOR_NUM - 2)*SPI_FLASH_SECTOR_SIZE + 32)	
// Ӳ���汾����
#define SPI_FLASH_HW_VERSION_ADDEESS		((SPI_FLASH_SECTOR_NUM - 2)*SPI_FLASH_SECTOR_SIZE + 64)	
// SN
#define SPI_FLASH_SN_ADDEESS				((SPI_FLASH_SECTOR_NUM - 2)*SPI_FLASH_SECTOR_SIZE + 96)	
// ����MAC
#define SPI_FLASH_BLE_MAC_ADDEESS			((SPI_FLASH_SECTOR_NUM - 2)*SPI_FLASH_SECTOR_SIZE + 128)	
// ��ά���ŵ�ַ
#define SPI_FLASH_QR_CODE_ADDEESS			((SPI_FLASH_SECTOR_NUM - 2)*SPI_FLASH_SECTOR_SIZE + 160)	
// �ͻ�SN��ַ
#define SPI_FLASH_CUSTOMER_SN_ADDEESS		((SPI_FLASH_SECTOR_NUM - 2)*SPI_FLASH_SECTOR_SIZE + 224)
// ��ά���ŵ�ַ
#define SPI_FLASH_NEW_QR_CODE_ADDEESS		((SPI_FLASH_SECTOR_NUM - 2)*SPI_FLASH_SECTOR_SIZE + 256)

// ϵͳ������
#define SPI_FLASH_SYS_PARAM_ADDEESS			((SPI_FLASH_SECTOR_NUM - 3)*SPI_FLASH_SECTOR_SIZE)

// ���Բ�д��
#define SPI_FLASH_TEST_ERASE_WD_ADDEESS		((SPI_FLASH_SECTOR_NUM - 4)*SPI_FLASH_SECTOR_SIZE)

// ����������
#define SPI_FLASH_FACTORY_PARAM_ADDEESS		((SPI_FLASH_SECTOR_NUM - 5)*SPI_FLASH_SECTOR_SIZE)

// �����������ݱ����ַ
#define SPI_FLASH_APP_DATA_ADDEESS			0x380000

// ����������������С
#define SPI_FLASH_APP_DATA_SIZE				256*1024

// ����������־��ַ
#define SPI_FLASH_APP_UPFLAG_ADDEESS		(SPI_FLASH_APP_DATA_ADDEESS + 256*1024)

#define SPI_FLASH_APP_READ_UPDATA_SIZE 	1024





// ������BOOTƫ�Ƶ�ַ
#define SPI_FLASH_HOST_BOOT_OFFSET_ADDR	        0

// ������BOOT��С
#define SPI_FLASH_HOST_BOOT_SIZE			    (100 * 1024)

// ������APPƫ�Ƶ�ַ
#define SPI_FLASH_HOST_APP_OFFSET_ADDR	        (SPI_FLASH_HOST_BOOT_OFFSET_ADDR + SPI_FLASH_HOST_BOOT_SIZE)

// ������APP��С
#define SPI_FLASH_HOST_APP_SIZE			        (260 * 1024)

// ������UIƫ�Ƶ�ַ
#define SPI_FLASH_HOST_UI_OFFSET_ADDR	        (SPI_FLASH_HOST_APP_OFFSET_ADDR + SPI_FLASH_HOST_APP_SIZE)

// ������UI��С
#define SPI_FLASH_HOST_UI_SIZE			        (3 * 1024 * 1024)

// ����������ƫ�Ƶ�ַ
#define SPI_FLASH_HOST_INFO_OFFSET_ADDR	        (SPI_FLASH_HOST_UI_OFFSET_ADDR + SPI_FLASH_HOST_UI_SIZE)

// ������������С
#define SPI_FLASH_HOST_INFO_SIZE			    (4 * 1024)






// SPI_FLASH֧������
#define W25X16		0
#define W25X32		1
#define W25X64		2
#define W25X128		3

// SPI_FLASHоƬ����ѡ��
#if (W25X16 == SPI_FLASH_TYPE)
	#define SPI_FLASH_PAGE_NUM		8192		// ҳ����
	#define SPI_FLASH_SECTOR_NUM	512 		// ������
	#define SPI_FLASH_BLOCK_NUM		32			// ������
	#define SPI_FLASH_TOTAL_SIZE	0x200000UL	// ��flash�ռ��С����λ:Byte
	#define SPI_FLASH_DEVICE_ID		0xEF14		// �豸ID
#elif (W25X32 == SPI_FLASH_TYPE)
	#define SPI_FLASH_PAGE_NUM		16384
	#define SPI_FLASH_SECTOR_NUM	1024
	#define SPI_FLASH_BLOCK_NUM		64
	#define SPI_FLASH_TOTAL_SIZE	0x400000UL
	#define SPI_FLASH_DEVICE_ID		0xEF15
#elif (W25X64 == SPI_FLASH_TYPE)
	#define SPI_FLASH_PAGE_NUM		32768
	#define SPI_FLASH_SECTOR_NUM	2048
	#define SPI_FLASH_BLOCK_NUM		128
	#define SPI_FLASH_TOTAL_SIZE	0x800000UL
	#define SPI_FLASH_DEVICE_ID		0xEF16
#elif (W25X128 == SPI_FLASH_TYPE)
	#define SPI_FLASH_PAGE_NUM		65536
	#define SPI_FLASH_SECTOR_NUM	4096
	#define SPI_FLASH_BLOCK_NUM		256
	#define SPI_FLASH_TOTAL_SIZE	0x1000000UL
	#define SPI_FLASH_DEVICE_ID		0xEF17
#endif

// UIƫ�Ƶ�ַ
#define SPI_FLASH_UI_OFFSET_ADDR	        0

// ����UI��С
#define SPI_FLASH_UI_SIZE			        (3 * 1024 * 1024)


// ����APPƫ�Ƶ�ַ
#define SPI_FLASH_APP_FACTORY_OFFSET_ADDR	(3 * 1024 * 1024)

// ����APP��С,��λΪ2���ݴη�,����1, 2, 4, 8, 0.5, 0.25, 0.125, 0.0625.....
#define SPI_FLASH_APP_FACTORY_SIZE			(256 * 1024)

// ����APPƫ�Ƶ�ַ
#define SPI_FLASH_APP_UPDATE_OFFSET_ADDR	((3 * 1024 * 1024) + (256 * 1024))

// ����APP��С,��λΪ2���ݴη�,����1, 2, 4, 8, 0.5, 0.25, 0.125, 0.0625.....
#define SPI_FLASH_APP_UPDATE_SIZE			(256 * 1024)


// SPI��ص����ݽṹ
typedef struct
{
	volatile BOOL dmaFinished;
}SPI_IF_CB;

extern SPI_IF_CB spiCB;

// ��ʼ��SPI Flash�����˿�
void SPIx_FLASH_Init(void);

// ��������
void SPI_FLASH_EraseSector(uint32 SectorAddr);

// ������
void SPI_FLASH_EraseBlock(uint32 BlockAddr);

// ��������Flash
void SPI_FLASH_EraseChip(void);

// ҳд�룬��ַֻ����һҳ��ַ��
void SPI_FLASH_WritePage(uint8* pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);


// ��ȡ������ָ�������������ݸ���������Ϊ��Ƭ����
void SPI_FLASH_ReadArray(uint8* pBuffer, uint32 ReadAddr, uint16 NumByteToRead);

// ��ָ��������������д��ָ����ʼ��ַ�����򣬲����������ܣ�ǰ����д�����������Ϊ������
void SPI_FLASH_WriteArray(uint8* pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);

// ��ָ��������������д��ָ����ʼ��ַ�����򣬲����������ܣ�������У�鹦�ܣ�����д����
BOOL SPI_FLASH_WriteWithCheck(uint8* pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);

// ��ָ��������������д��ָ����ʼ��ַ�����򣬴��������ܣ�ָ�����ݷǿ�ʱ���ȶ������������ݺ���������������д��
void SPI_FLASH_WriteWithErase(uint8* pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);

// ����ָ���ռ�
void SPI_FLASH_EraseRoom(uint32 Addr, uint32 size);


// ��ȡоƬID
uint16 SPI_FLASH_ReadChipID(void);

// ����һ���ֽ�
uint8 SPI_FLASH_SendByte(uint8 byte);

// ����д��ʹ��ָ��
void SPI_FLASH_WriteEnable(void);

// д״̬�Ĵ���
void SPI_FLASH_WriteStatus(uint8 data);

// �ȴ�д��������
void SPI_FLASH_WaitForWriteEnd(void);

// DMA����(ֻ֧��Ҳд256Byte)
void SPIx_DMAy_SendBuffer(uint8 *sendBuffer, uint32 WriteAddr, uint16 ndtr);

// DMA����
void SPIx_DMAy_Read(uint32 srcAddr, uint32 dstAddr, uint16 count);

// ����SPI����ģʽ
void SPIx_ConfigureWorkMode(uint16 SpiDirection, uint16 SpiDataSize);

uint8 SPI_FLASH_ReadByte(uint32 ReadAddr);

uint16 SPI_FLASH_ReadHalfWord(uint32 ReadAddr);

uint32 SPI_FLASH_ReadWord(uint32 ReadAddr);


void SPI_FLASH_WriteByte(uint32 WriteAddr, uint8 data);

void SPI_FLASH_WriteHalfWord(uint32 WriteAddr, uint16 data);

void SPI_FLASH_WriteWord(uint32 WriteAddr, uint32 data);


#endif

