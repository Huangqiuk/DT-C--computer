#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#include "common.h"

// SPIx选择
#define STM32_SPIx_NUM_DEF              SPI2                    // 选择SPI(中断和时钟都需要修改)
#define STM32_SPIx_IRQn_DEF             SPI2_IRQn               // 选择SPI(中断和时钟都需要修改)
#define STM32_SPIx_DR_Addr              ((uint32)(&SPI_DATA(SPI2)))         // SPI数据寄存器地址

// 选择全双工还是双线只接收
#define STM32_DMA_SPI_DIRECTION         1           // 0: 2Lines_RxOnly，1: 2Lines_FullDuplex

// SPI_FLASH单元测试开关
#define SPI_FLASH_UNIT_TEST             0           // 0禁止，1使能

#define SPI_FLASH_TYPE                  W25X128     // spi_flash类型选择

// 擦除芯片模式选择
#define SPI_FLASH_ERASE_MODE            2           // 0扇区，1整片，2块(整片擦除慎用，因为会把UI数据也擦掉，其它两种方式会自动避开UI数据擦除)

// 以下宏开发人员修改
#define SPI_FLASH_RETRY_READ_TIME       200U        // 重试次数，避免死循环
#define SPI_FLASH_PAGE_SIZE             256         // 页尺寸，单位:Byte
#define SPI_FLASH_SECTOR_SIZE           4096UL      // 扇尺寸，单位:Byte
#define SPI_FLASH_BLOCK_SIZE            65536UL     // 块尺寸，单位:Byte
#define SPI_FLASH_BLOCK_PER_16SECTORS   16          // 每16个扇区为一个块

#define SPI_FLASH_SECTOR_START_ADDR(X)  ((X)*SPI_FLASH_SECTOR_SIZE) // 第n个扇区地址 
#define SPI_FLASH_BLOCK_START_ADDR(X)   ((X)*SPI_FLASH_BLOCK_SIZE)  // 第n个块区地址

// Flash命令
#define CMD_WRITE_ENABLE                0x06    // 写入使能指令
#define CMD_WRITE_REGISTER_ENABLE       0x50    // 允许写状态寄存器
#define CMD_WRITE_STATUS_REGISTER       0x01    // 写入状态寄存器
#define CMD_SECTOR_ERASE_4K             0x20    // Sector Erase instruction
#define CMD_CHIP_ERASE                  0xC7    // Chip Erase instruction
#define CMD_PAGE_WRITE                  0x02    // 页写入命令
#define CMD_READ_BYTE                   0x03    // Read from Memory instruction
#define CMD_READ_STATUS_REGISTER1       0x05    // Read Status Register instruction
#define BUSY_FLAG                       0x01    // BUSY flag
// 后加
#define CMD_FAST_READ_BYTE              0x0B    // 快读
#define CMD_BLOCK_ERASE_32K             0x52    // 32K块擦除
#define CMD_BLOCK_ERASE_64K             0xD8    // 64K块擦除
#define CMD_WRITE_DISABLE               0x04    // 写禁止
#define CMD_POWER_DOWN                  0xB9    // 掉电
#define CMD_RELEASE_POWERDOWN_DEVICE_ID 0xAB    // 释放掉电/器件ID
#define CMD_MANUFACT_DEVICE_ID          0x90    // 制造/器件ID
#define CMD_JEDEC_DEVICE_ID             0x9F    // JEDEC ID

#define SPI_FLASH_CS_L()                gpio_bit_reset(GPIOE, GPIO_PIN_1)
#define SPI_FLASH_CS_H()                gpio_bit_set(GPIOE, GPIO_PIN_1)

//工具app首地址地址
#define SPI_FLASH_TOOL_APP_ADDEESS                  0x00000000

//dut的boot首地址
#define SPI_FLASH_DUT_BOOT_ADDEESS                  (SPI_FLASH_TOOL_APP_ADDEESS + 0x00040000)   // dut_boot区

//dut的app首地址
#define SPI_FLASH_DUT_APP_ADDEESS                   (SPI_FLASH_TOOL_APP_ADDEESS + 0x00060000)   // dut_app区

//dut的config首地址
#define SPI_FLASH_DUT_CONFIG_ADDEESS                (SPI_FLASH_TOOL_APP_ADDEESS + 0x00160000)   // dut_config区

/* other区 */
#define SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS          (SPI_FLASH_TOOL_APP_ADDEESS + 0x00161000)   // other区
// 工具升级数据区大小
#define SPI_FLASH_TOOL_APP_DATA_SIZE                (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 4*1024)
//dut机型信息
#define SPI_FLASH_DUT_INFO                          (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 8*1024)
// BOOT版本存放地址
#define SPI_FLASH_BOOT_VERSION_ADDEESS              (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 12*1024)
// APP版本存放地址
#define SPI_FLASH_APP_VERSION_ADDEESS               (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 16*1024)
//DUT_BOOT大小存放
#define SPI_FLASH_DUT_BOOT_SIZE_ADDEESS             (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 20*1024)
//DUT_APP大小存放
#define SPI_FLASH_DUT_APP_SIZE_ADDEESS              (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 24*1024)
//DUT_UI大小存放
#define SPI_FLASH_DUT_UI_SIZE_ADDEESS               (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 28*1024)
//tool排序信息
#define SPI_FLASH_TOOL_RANK_ADDEESS                 (SPI_FLASH_TOOL_UPDATA_FLAG_ADDEESS + 32*1024)


//dut的ui首地址
#define SPI_FLASH_DUT_UI_ADDEESS                   0x00200000//ui区

// SPI_FLASH支持类型
#define W25X16      0
#define W25X32      1
#define W25X64      2
#define W25X128     3

// SPI_FLASH芯片类型选择
#if (W25X16 == SPI_FLASH_TYPE)
    #define SPI_FLASH_PAGE_NUM      8192        // 页数量
    #define SPI_FLASH_SECTOR_NUM    512         // 扇数量
    #define SPI_FLASH_BLOCK_NUM     32          // 块数量
    #define SPI_FLASH_TOTAL_SIZE    0x200000UL  // 总flash空间大小，单位:Byte
    #define SPI_FLASH_DEVICE_ID     0xEF14      // 设备ID
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

// SPI相关的数据结构
typedef struct
{
    volatile BOOL dmaFinished;
    volatile BOOL tx_dmaFinished;
} SPI_IF_CB;

extern SPI_IF_CB spiCB;

// 初始化SPI Flash驱动端口
void SPIx_FLASH_Init(void);

// 擦除扇区
void SPI_FLASH_EraseSector(uint32 SectorAddr);

// 擦除块
void SPI_FLASH_EraseBlock(uint32 BlockAddr);

// 擦除整个Flash
void SPI_FLASH_EraseChip(void);

// 页写入，地址只限于一页地址内
void SPI_FLASH_WritePage(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);


// 读取数据至指定缓冲区，数据个数最大可以为整片容量
void SPI_FLASH_ReadArray(uint8 *pBuffer, uint32 ReadAddr, uint16 NumByteToRead);

// 将指定缓冲区的数据写入指定开始地址的区域，不带擦除功能，前提是写入的数据区域为空数据
void SPI_FLASH_WriteArray(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);

// 将指定缓冲区的数据写入指定开始地址的区域，不带擦除功能，带数据校验功能，返回写入结果
BOOL SPI_FLASH_WriteWithCheck(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);

// 将指定缓冲区的数据写入指定开始地址的区域，带擦除功能，指定数据非空时将先读出该扇区数据后擦除该扇区最后再写入
void SPI_FLASH_WriteWithErase(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);

// 擦除指定空间
void SPI_FLASH_EraseRoom(uint32 Addr, uint32 size);


// 读取芯片ID
uint16 SPI_FLASH_ReadChipID(void);

// 发送一个字节
uint8 SPI_FLASH_SendByte(uint8 byte);

// 发送写入使能指令
void SPI_FLASH_WriteEnable(void);

// 写状态寄存器
void SPI_FLASH_WriteStatus(uint8 data);

// 等待写入操作完成
void SPI_FLASH_WaitForWriteEnd(void);

// DMA发送(只支持也写256Byte)
void SPIx_DMAy_SendBuffer(uint8 *sendBuffer, uint32 WriteAddr, uint16 ndtr);

// DMA接收
void SPIx_DMAy_Read(uint32 srcAddr, uint32 dstAddr, uint16 count);

// 配置SPI工作模式
void SPIx_ConfigureWorkMode(uint16 SpiDirection, uint16 SpiDataSize);

uint8 SPI_FLASH_ReadByte(uint32 ReadAddr);

uint16 SPI_FLASH_ReadHalfWord(uint32 ReadAddr);

uint32 SPI_FLASH_ReadWord(uint32 ReadAddr);


void SPI_FLASH_WriteByte(uint32 WriteAddr, uint8 data);

void SPI_FLASH_WriteHalfWord(uint32 WriteAddr, uint16 data);

void SPI_FLASH_WriteWord(uint32 WriteAddr, uint32 data);

uint8 SPI_FlashReadForCrc8(uint32 datalen);

#endif

