#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#include "common.h"

// SPIxѡ��
#define STM32_SPIx_NUM_DEF              SPI2                    // ѡ��SPI(�жϺ�ʱ�Ӷ���Ҫ�޸�)
#define STM32_SPIx_IRQn_DEF             SPI2_IRQn               // ѡ��SPI(�жϺ�ʱ�Ӷ���Ҫ�޸�)
#define STM32_SPIx_DR_Addr              ((uint32)(&SPI_DATA(SPI2)))         // SPI���ݼĴ�����ַ

// ѡ��ȫ˫������˫��ֻ����
#define STM32_DMA_SPI_DIRECTION         1           // 0: 2Lines_RxOnly��1: 2Lines_FullDuplex

// SPI_FLASH��Ԫ���Կ���
#define SPI_FLASH_UNIT_TEST             0           // 0��ֹ��1ʹ��

#define SPI_FLASH_TYPE                  W25X128     // spi_flash����ѡ��

// ����оƬģʽѡ��
#define SPI_FLASH_ERASE_MODE            2           // 0������1��Ƭ��2��(��Ƭ�������ã���Ϊ���UI����Ҳ�������������ַ�ʽ���Զ��ܿ�UI���ݲ���)

// ���º꿪����Ա�޸�
#define SPI_FLASH_RETRY_READ_TIME       200U        // ���Դ�����������ѭ��
#define SPI_FLASH_PAGE_SIZE             256         // ҳ�ߴ磬��λ:Byte
#define SPI_FLASH_SECTOR_SIZE           4096UL      // �ȳߴ磬��λ:Byte
#define SPI_FLASH_BLOCK_SIZE            65536UL     // ��ߴ磬��λ:Byte
#define SPI_FLASH_BLOCK_PER_16SECTORS   16          // ÿ16������Ϊһ����

#define SPI_FLASH_SECTOR_START_ADDR(X)  ((X)*SPI_FLASH_SECTOR_SIZE) // ��n��������ַ 
#define SPI_FLASH_BLOCK_START_ADDR(X)   ((X)*SPI_FLASH_BLOCK_SIZE)  // ��n��������ַ

// Flash����
#define CMD_WRITE_ENABLE                0x06    // д��ʹ��ָ��
#define CMD_WRITE_REGISTER_ENABLE       0x50    // ����д״̬�Ĵ���
#define CMD_WRITE_STATUS_REGISTER       0x01    // д��״̬�Ĵ���
#define CMD_SECTOR_ERASE_4K             0x20    // Sector Erase instruction
#define CMD_CHIP_ERASE                  0xC7    // Chip Erase instruction
#define CMD_PAGE_WRITE                  0x02    // ҳд������
#define CMD_READ_BYTE                   0x03    // Read from Memory instruction
#define CMD_READ_STATUS_REGISTER1       0x05    // Read Status Register instruction
#define BUSY_FLAG                       0x01    // BUSY flag
// ���
#define CMD_FAST_READ_BYTE              0x0B    // ���
#define CMD_BLOCK_ERASE_32K             0x52    // 32K�����
#define CMD_BLOCK_ERASE_64K             0xD8    // 64K�����
#define CMD_WRITE_DISABLE               0x04    // д��ֹ
#define CMD_POWER_DOWN                  0xB9    // ����
#define CMD_RELEASE_POWERDOWN_DEVICE_ID 0xAB    // �ͷŵ���/����ID
#define CMD_MANUFACT_DEVICE_ID          0x90    // ����/����ID
#define CMD_JEDEC_DEVICE_ID             0x9F    // JEDEC ID

#define SPI_FLASH_CS_L()                gpio_bit_reset(GPIOE, GPIO_PIN_1)
#define SPI_FLASH_CS_H()                gpio_bit_set(GPIOE, GPIO_PIN_1)

//����app�׵�ַ��ַ
#define SPI_FLASH_TOOL_APP_ADDEESS                  0x00000000

//dut��boot�׵�ַ
#define SPI_FLASH_DUT_BOOT_ADDEESS                  (SPI_FLASH_TOOL_APP_ADDEESS + 0x00040000)   // dut_boot��

//dut��app�׵�ַ
#define SPI_FLASH_DUT_APP_ADDEESS                   (SPI_FLASH_TOOL_APP_ADDEESS + 0x00060000)   // dut_app��

//dut��config�׵�ַ
#define SPI_FLASH_DUT_CONFIG_ADDEESS                (SPI_FLASH_TOOL_APP_ADDEESS + 0x00160000)   // dut_config��

/* other�� */
#define SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS          (SPI_FLASH_TOOL_APP_ADDEESS + 0x00161000)   // other��
// ����������������С
#define SPI_FLASH_TOOL_APP_DATA_SIZE                (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 4*1024)
//dut������Ϣ
#define SPI_FLASH_DUT_INFO                          (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 8*1024)
// BOOT�汾��ŵ�ַ
#define SPI_FLASH_BOOT_VERSION_ADDEESS              (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 12*1024)
// APP�汾��ŵ�ַ
#define SPI_FLASH_APP_VERSION_ADDEESS               (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 16*1024)
//DUT_BOOT��С���
#define SPI_FLASH_DUT_BOOT_SIZE_ADDEESS             (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 20*1024)
//DUT_APP��С���
#define SPI_FLASH_DUT_APP_SIZE_ADDEESS              (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 24*1024)
//DUT_UI��С���
#define SPI_FLASH_DUT_UI_SIZE_ADDEESS               (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 28*1024)
//tool������Ϣ
#define SPI_FLASH_TOOL_RANK_ADDEESS                 (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 32*1024)


//dut��ui�׵�ַ
#define SPI_FLASH_DUT_UI_ADDEESS                   0x00200000//ui��

// SPI_FLASH֧������
#define W25X16      0
#define W25X32      1
#define W25X64      2
#define W25X128     3

// SPI_FLASHоƬ����ѡ��
#if (W25X16 == SPI_FLASH_TYPE)
    #define SPI_FLASH_PAGE_NUM      8192        // ҳ����
    #define SPI_FLASH_SECTOR_NUM    512         // ������
    #define SPI_FLASH_BLOCK_NUM     32          // ������
    #define SPI_FLASH_TOTAL_SIZE    0x200000UL  // ��flash�ռ��С����λ:Byte
    #define SPI_FLASH_DEVICE_ID     0xEF14      // �豸ID
#elif (W25X32 == SPI_FLASH_TYPE)
    #define SPI_FLASH_PAGE_NUM      16384
    #define SPI_FLASH_SECTOR_NUM    1024
    #define SPI_FLASH_BLOCK_NUM     64
    #define SPI_FLASH_TOTAL_SIZE    0x400000UL
    #define SPI_FLASH_DEVICE_ID     0xEF15
#elif (W25X64 == SPI_FLASH_TYPE)
    #define SPI_FLASH_PAGE_NUM      32768
    #define SPI_FLASH_SECTOR_NUM    2048
    #define SPI_FLASH_BLOCK_NUM     128
    #define SPI_FLASH_TOTAL_SIZE    0x800000UL
    #define SPI_FLASH_DEVICE_ID     0xEF16
#elif (W25X128 == SPI_FLASH_TYPE)
    #define SPI_FLASH_PAGE_NUM      65536
    #define SPI_FLASH_SECTOR_NUM    4096
    #define SPI_FLASH_BLOCK_NUM     256
    #define SPI_FLASH_TOTAL_SIZE    0x1000000UL
    #define SPI_FLASH_DEVICE_ID     0xEF17
#endif

// SPI��ص����ݽṹ
typedef struct
{
    volatile BOOL dmaFinished;
    volatile BOOL tx_dmaFinished;
} SPI_IF_CB;

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
void SPI_FLASH_WritePage(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);


// ��ȡ������ָ�������������ݸ���������Ϊ��Ƭ����
void SPI_FLASH_ReadArray(uint8 *pBuffer, uint32 ReadAddr, uint16 NumByteToRead);

// ��ָ��������������д��ָ����ʼ��ַ�����򣬲����������ܣ�ǰ����д�����������Ϊ������
void SPI_FLASH_WriteArray(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);

// ��ָ��������������д��ָ����ʼ��ַ�����򣬲����������ܣ�������У�鹦�ܣ�����д����
BOOL SPI_FLASH_WriteWithCheck(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);

// ��ָ��������������д��ָ����ʼ��ַ�����򣬴��������ܣ�ָ�����ݷǿ�ʱ���ȶ������������ݺ���������������д��
void SPI_FLASH_WriteWithErase(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);

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

uint8 SPI_FlashReadForCrc8(uint32 datalen);

#endif

