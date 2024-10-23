#include "spiflash.h"
#include "common.h"
#include "Watchdog.h"

spi_parameter_struct spi_init_struct;
dma_parameter_struct dma_tx_init_struct;
dma_parameter_struct dma_rx_init_struct;

SPI_IF_CB spiCB;

// DMA中断配置
static void NVIC_DMAy_Configuration(void);

// DMA硬件初始化
static void DMAy_SPIx_Configuration(void);


// 单元测试
#if SPI_FLASH_UNIT_TEST
#define SPI_BUFF_SIZE                   256
uint8 SPI_TX_Buff[SPI_BUFF_SIZE];
uint8 SPI_RX_Buff[SPI_BUFF_SIZE];
uint16 DeviceID;
void SPI_FLASH_UnitTest(void)
{
    uint16 i;

    DeviceID = SPI_FLASH_ReadChipID();

    // 擦除所有flash空间
    SPI_FLASH_EraseChip();

    for (i = 0; i < SPI_BUFF_SIZE; i++)
    {
        SPI_TX_Buff[i] = 0x58;
        SPI_RX_Buff[i] = 0;
    }
    SPI_FLASH_ReadArray(SPI_RX_Buff, 0, SPI_BUFF_SIZE);
    SPI_FLASH_WriteArray(SPI_TX_Buff, 0, SPI_BUFF_SIZE);
    SPI_FLASH_ReadArray(SPI_RX_Buff, 0, SPI_BUFF_SIZE);


    for (i = 0; i < SPI_BUFF_SIZE; i++)
    {
        SPI_RX_Buff[i] = 0;
    }
    // DMA方式读取数据
    //SPIx_DMAy_Read(0, (uint32)SPI_RX_Buff, SPI_BUFF_SIZE);
}
#endif


// 初始化SPI Flash驱动端口
void SPIx_FLASH_Init(void)
{
//  // GD32 - SPI
//  if(STM32_SPIx_NUM_DEF == SPI0)
//  {
//      /*
//      * PA5 --> SPI1 SCK          Mode: GPIO_Mode_AF_PP
//      * PA7 --> SPI1 MOSI Write   Mode: GPIO_Mode_AF_PP
//      * PA6 --> SPI1 MISO Read    Mode: GPIO_Mode_IPU
//      * PA4 --> SPI1 CS           Mode: GPIO_Mode_Out_PP
//      */
//
//      // 使能GPIO时钟
//      rcu_periph_clock_enable(RCU_GPIOB);
//
//      // SPI0 GPIO config: SCK/PA5-> PB3, MOSI/PA7 - > PB5
//      gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);

//      // SPI0 GPIO config: MISO/PA6 ->PB4
//      gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_4);

//      // SPI0 GPIO config: NSS/PA4 -> PE1
//      gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
//
//      // SPIx配置选项
//      rcu_periph_clock_enable(RCU_SPI0);
//
//  }
//  else
//  {
//      // 使能GPIO时钟
//      rcu_periph_clock_enable(RCU_GPIOB);
//
//      // SPI1 GPIO config: SCK/PB13, MOSI/PB15
//      gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 |GPIO_PIN_15);

//      // SPI1 GPIO config: MISO/PB14
//      gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);

//      // SPI1 GPIO config: NSS/PB12
//      gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);

//      // SPIx配置选项
//      rcu_periph_clock_enable(RCU_SPI1);
//  }
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOE);
    rcu_periph_clock_enable(RCU_SPI2);
    rcu_periph_clock_enable(RCU_AF);

    //禁用JTAG引脚功能，操作的是AF寄存器，所以要先使能AF时钟
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    // SPI0 GPIO config: SCK/PB3, MOSI/PB5
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);

    // SPI0 GPIO config: MISO/PB4
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_4);

    // SPI0 GPIO config: NSS/PE1
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);

    spi_i2s_deinit(STM32_SPIx_NUM_DEF);

    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;              // 空闲时CLK拉底，在第一个时钟跳变沿采集第一个数据
    spi_init_struct.nss                  = SPI_NSS_SOFT;                        // 片选软件控制

    // APB2 - SPI0 - 72M
    // APB1 - SPI1/SPI2 - 36M
    spi_init_struct.prescale             = SPI_PSC_4;                           // 72/4=18M
    spi_init_struct.endian               = SPI_ENDIAN_MSB;                      // 先发送高位
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // 使能SPIx
    spi_enable(STM32_SPIx_NUM_DEF);

    // 使能DMA功能
    DMAy_SPIx_Configuration();

    spiCB.dmaFinished = FALSE;

#if SPI_FLASH_UNIT_TEST
    SPI_FLASH_UnitTest();
#endif
}

// 擦除扇区
void SPI_FLASH_EraseSector(uint32 SectorAddr)
{
    // 使能写入操作
    SPI_FLASH_WriteEnable();

    // 片选
    SPI_FLASH_CS_L();

    // 发送扇区擦除命令
    SPI_FLASH_SendByte(CMD_SECTOR_ERASE_4K);

    // 发送24位地址
    SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(SectorAddr & 0xFF);

    // 释放
    SPI_FLASH_CS_H();

    // 等待写入完成
    SPI_FLASH_WaitForWriteEnd();
}

// 擦除块
void SPI_FLASH_EraseBlock(uint32 BlockAddr)
{
    // 使能写入操作
    SPI_FLASH_WriteEnable();

    // 片选
    SPI_FLASH_CS_L();

    // 发送64K擦除命令
    SPI_FLASH_SendByte(CMD_BLOCK_ERASE_64K);

    // 发送24位地址
    SPI_FLASH_SendByte((BlockAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((BlockAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(BlockAddr & 0xFF);

    // 释放
    SPI_FLASH_CS_H();

    // 等待写入完成
    SPI_FLASH_WaitForWriteEnd();
}


// 擦除整个Flash
void SPI_FLASH_EraseChip(void)
{
    // 使能写入操作
    SPI_FLASH_WriteEnable();

    // 片选
    SPI_FLASH_CS_L();

    // 发送片擦除命令
    SPI_FLASH_SendByte(CMD_CHIP_ERASE);

    // 释放
    SPI_FLASH_CS_H();

    // 等待写入完成
    SPI_FLASH_WaitForWriteEnd();
}

// 页写入
void SPI_FLASH_WritePage(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    // 写入数据量为0，退出
    if (0 == NumByteToWrite)
    {
        return ;
    }

    // 使能写入操作
    SPI_FLASH_WriteEnable();

    // 片选
    SPI_FLASH_CS_L();

    // 发送页写入命令
    SPI_FLASH_SendByte(CMD_PAGE_WRITE);

    // 写入24位起始地址
    SPI_FLASH_SendByte((WriteAddr >> 16) & 0xFF);
    SPI_FLASH_SendByte((WriteAddr >> 8) & 0xFF);
    SPI_FLASH_SendByte(WriteAddr & 0xFF);

    // 连续写入指定数量的数据
    while (NumByteToWrite--)
    {
        /// 写入一个字节
        SPI_FLASH_SendByte(*pBuffer);

        pBuffer++;
    }

    // 释放
    SPI_FLASH_CS_H();

    // 等待写入完成
    SPI_FLASH_WaitForWriteEnd();
}

// 将指定缓冲区的数据写入指定开始地址的区域
void SPI_FLASH_WriteArray(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    uint8 NumOfPage = 0, NumOfRestByte = 0, StartAddrOffset = 0, RestSpace = 0, temp = 0;

    // 指针非空判断
    if (NULL == pBuffer)
    {
        return;
    }

    // 写入数据量为0，退出
    if (0 == NumByteToWrite)
    {
        return ;
    }

    StartAddrOffset = WriteAddr % SPI_FLASH_PAGE_SIZE;
    RestSpace = SPI_FLASH_PAGE_SIZE - StartAddrOffset;
    NumOfPage =  NumByteToWrite / SPI_FLASH_PAGE_SIZE;
    NumOfRestByte = NumByteToWrite % SPI_FLASH_PAGE_SIZE;

    // 起始地址刚好位于当前页的起点
    if (StartAddrOffset == 0)
    {
        // 数据量不足一页，直接写入
        if (NumOfPage == 0)
        {
            SPI_FLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
        }
        // 数据量超出一页
        else
        {
            // 整页的部分，直接写入整页
            while (NumOfPage--)
            {
                // 在当前页写满
                SPI_FLASH_WritePage(pBuffer, WriteAddr, SPI_FLASH_PAGE_SIZE);

                WriteAddr += SPI_FLASH_PAGE_SIZE;
                pBuffer += SPI_FLASH_PAGE_SIZE;
            }

            // 写入不足一页的部分
            SPI_FLASH_WritePage(pBuffer, WriteAddr, NumOfRestByte);
        }
    }
    // 起始地址不是当前页的起点，即从中间插入写入
    else
    {
        // 数据量不足一页，直接写入
        if (NumOfPage == 0)
        {
            // 要写入的数据量超过了当前页剩余空间，则分开两次写入
            if (NumOfRestByte > RestSpace)
            {
                // 先将当前页填满
                temp = NumOfRestByte - RestSpace;

                SPI_FLASH_WritePage(pBuffer, WriteAddr, RestSpace);
                WriteAddr +=  RestSpace;
                pBuffer += RestSpace;

                // 再将剩余的数据写入下一页
                SPI_FLASH_WritePage(pBuffer, WriteAddr, temp);
            }
            // 要写入的数据量不超过当前页的剩余空间，则直接写入
            else
            {
                SPI_FLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
            }
        }
        // 数据量超过一页
        else
        {
            // 先将当前页填满
            SPI_FLASH_WritePage(pBuffer, WriteAddr, RestSpace);
            WriteAddr +=  RestSpace;
            pBuffer += RestSpace;

            // 剩余的数据，写入起点就是下一页的起点了，和上面第一种情况相同的处理
            NumByteToWrite -= RestSpace;
            NumOfPage =  NumByteToWrite / SPI_FLASH_PAGE_SIZE;
            NumOfRestByte = NumByteToWrite % SPI_FLASH_PAGE_SIZE;

            while (NumOfPage--)
            {
                SPI_FLASH_WritePage(pBuffer, WriteAddr, SPI_FLASH_PAGE_SIZE);
                WriteAddr += SPI_FLASH_PAGE_SIZE;
                pBuffer += SPI_FLASH_PAGE_SIZE;
            }

            SPI_FLASH_WritePage(pBuffer, WriteAddr, NumOfRestByte);
        }
    }
}

uint8 SPI_FLASH_ReadByte(uint32 ReadAddr)
{
    uint8 buff[4];

    SPI_FLASH_ReadArray(buff, ReadAddr, 4);

    return buff[0];
}

uint16 SPI_FLASH_ReadHalfWord(uint32 ReadAddr)
{
    uint8 buff[4];

    SPI_FLASH_ReadArray(buff, ReadAddr, 4);

    return ((uint16)buff[0] << 8) + buff[1];
}

uint32 SPI_FLASH_ReadWord(uint32 ReadAddr)
{
    uint8 buff[4];

    SPI_FLASH_ReadArray(buff, ReadAddr, 4);

    return ((uint32)buff[0] << 24) + ((uint32)buff[1] << 16) + ((uint32)buff[2] << 8) + buff[3];
}

// 读取数据至指定缓冲区，数据个数最大可以为整片容量
void SPI_FLASH_ReadArray(uint8 *pBuffer, uint32 ReadAddr, uint16 NumByteToRead)
{
    // 指针非空判断
    if (NULL == pBuffer)
    {
        return;
    }

    // 要读取的数据量为0，退出
    if (0 == NumByteToRead)
    {
        return;
    }

    // 片选
    SPI_FLASH_CS_L();

    // 发送读取命令
    SPI_FLASH_SendByte(CMD_READ_BYTE);

    // 发送24位起始地址
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

    // 一直读取完指定的数量为止
    while (NumByteToRead--)
    {
        // 读取一个字节
        *(pBuffer++) = SPI_FLASH_SendByte(0);
    }

    // 释放
    SPI_FLASH_CS_H();
}

// 读取芯片ID
uint16 SPI_FLASH_ReadChipID(void)
{
    uint16 W25xXX_ID = 0;

    // 使能写入操作
    SPI_FLASH_WriteEnable();

    SPI_FLASH_CS_L();

    // 发送读取ID命令
    SPI_FLASH_SendByte(CMD_MANUFACT_DEVICE_ID);

    // 发送24位的地址
    SPI_FLASH_SendByte(0x00);   // dummy
    SPI_FLASH_SendByte(0x00);   // dummy
    SPI_FLASH_SendByte(0x00);   // 0x00

    // 读取返回的16位值
    W25xXX_ID = SPI_FLASH_SendByte(0xFF);   // 高8位数据
    W25xXX_ID <<= 8;
    W25xXX_ID += SPI_FLASH_SendByte(0xFF);  // 底8位数据

    SPI_FLASH_CS_H();

    return W25xXX_ID;
}

// 发送一个字节
uint8 SPI_FLASH_SendByte(uint8 byte)
{
    // 发送寄存器不为空，等待
    uint16 spiRetryTime = 0;

    while (spi_i2s_flag_get(STM32_SPIx_NUM_DEF, SPI_STAT_TBE) == RESET)
    {
        if (spiRetryTime++ > SPI_FLASH_RETRY_READ_TIME)
        {
            return 0x00;
        }
    }
    // 发送
    spi_i2s_data_transmit(STM32_SPIx_NUM_DEF, byte);

    // 接收寄存器为空，等待
    spiRetryTime = 0;
    while (spi_i2s_flag_get(STM32_SPIx_NUM_DEF, SPI_STAT_RBNE) == RESET)
    {
        if (spiRetryTime++ > SPI_FLASH_RETRY_READ_TIME)
        {
            return 0x00;
        }
    }

    // 返回接收到的值
    return spi_i2s_data_receive(STM32_SPIx_NUM_DEF);
}

// 发送写入使能指令
void SPI_FLASH_WriteEnable(void)
{
    // 片选
    SPI_FLASH_CS_L();

    // 发送写入使能指令
    SPI_FLASH_SendByte(CMD_WRITE_ENABLE);

    // 释放
    SPI_FLASH_CS_H();
}

// 写状态寄存器
void SPI_FLASH_WriteStatus(uint8 data)
{
    // 片选
    SPI_FLASH_CS_L();

    // 允许写状态寄存器
    SPI_FLASH_SendByte(CMD_WRITE_REGISTER_ENABLE);

    // 释放
    SPI_FLASH_CS_H();

    // 片选
    SPI_FLASH_CS_L();

    // 允许写状态寄存器
    SPI_FLASH_SendByte(CMD_WRITE_STATUS_REGISTER);
    SPI_FLASH_SendByte(data);

    // 释放
    SPI_FLASH_CS_H();
}

// 等待写入操作完成
void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8 flashstatus = 0;

    // 片选
    SPI_FLASH_CS_L();

    // 发送读取状态寄存器1的命令
    SPI_FLASH_SendByte(CMD_READ_STATUS_REGISTER1);

    // 写入中，持续等待
    do
    {
        // 发送一个假数据，即可产生SPI时钟
        flashstatus = SPI_FLASH_SendByte(0);
    }
    while ((flashstatus & BUSY_FLAG) == BUSY_FLAG);

    // 释放
    SPI_FLASH_CS_H();
}

// 擦除指定空间 先擦扇区再擦块再擦扇区
void SPI_FLASH_EraseRoom(uint32 Addr, uint32 size)
{

    uint16 blockStartNum = 0;       // 块号
    uint16 blockCount = 0;          // 需要擦除块的数量
    uint16 blockSectorOffset = 0;   // 块内扇区偏移量
    uint16 i;

    blockSectorOffset = Addr / SPI_FLASH_SECTOR_SIZE % SPI_FLASH_BLOCK_PER_16SECTORS;

    if (blockSectorOffset)          // 擦除扇区在块内存在偏移量
    {
        blockStartNum = Addr / SPI_FLASH_BLOCK_SIZE + 1;
    }
    else                            // 擦除开始地址等于块的起始地址，不存在偏移
    {
        blockStartNum = Addr / SPI_FLASH_BLOCK_SIZE;
    }

    // 擦除起始块内扇区偏移
    for (i = 0; i < SPI_FLASH_BLOCK_PER_16SECTORS - blockSectorOffset; i++)
    {
        SPI_FLASH_EraseSector((Addr / SPI_FLASH_SECTOR_SIZE + i) * SPI_FLASH_SECTOR_SIZE);

        WDT_Clear();
    }

    // 计算出需要擦除的块的数量
    blockCount = ((size / SPI_FLASH_SECTOR_SIZE) - (SPI_FLASH_BLOCK_PER_16SECTORS - blockSectorOffset)) / SPI_FLASH_BLOCK_PER_16SECTORS;

    // 擦除块
    for (i = 0; i < blockCount; i++)
    {
        SPI_FLASH_EraseBlock((blockStartNum + i) * SPI_FLASH_BLOCK_SIZE);

        WDT_Clear();
    }

    // 擦除剩余扇区,剩余的扇区不满一个块
    for (i = 0; i < ((size / SPI_FLASH_SECTOR_SIZE) - (SPI_FLASH_BLOCK_PER_16SECTORS - blockSectorOffset)) % SPI_FLASH_BLOCK_PER_16SECTORS; i++)
    {
        SPI_FLASH_EraseSector((blockStartNum + blockCount) * SPI_FLASH_BLOCK_SIZE + i * SPI_FLASH_SECTOR_SIZE);

        WDT_Clear();
    }
}


// 数据写入接口，返回写入结果
BOOL SPI_FLASH_WriteWithCheck(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    uint16 NumOfInt = 0;
    uint16 NumOfRestByte = 0;
    uint16 i;
    uint8 *pFlashBuff = NULL;               // 保存读取扇区的数据

    if ((NULL == pBuffer) || (0 == NumByteToWrite))
    {
        return FALSE;
    }

    // 申请4096Byte空间保存扇区数据
    pFlashBuff = (uint8 *)malloc(SPI_FLASH_SECTOR_SIZE);

    // 指针非空判断
    if (NULL == pFlashBuff)
    {
        return FALSE;
    }

    if (NumByteToWrite > SPI_FLASH_SECTOR_SIZE)
    {
        NumOfInt = NumByteToWrite / SPI_FLASH_SECTOR_SIZE;
        NumOfRestByte = NumByteToWrite % SPI_FLASH_SECTOR_SIZE;

        while (NumOfInt--)
        {
            // 执行写入
            SPI_FLASH_WriteArray(pBuffer, WriteAddr, SPI_FLASH_SECTOR_SIZE);
            // 执行读取
            SPI_FLASH_ReadArray(pFlashBuff, WriteAddr, SPI_FLASH_SECTOR_SIZE);

            for (i = 0; i < SPI_FLASH_SECTOR_SIZE; i++)
            {
                if (pBuffer[i] != pFlashBuff[i])
                {
                    free(pFlashBuff);
                    return FALSE;
                }
            }

            WriteAddr += SPI_FLASH_SECTOR_SIZE;
            pBuffer += SPI_FLASH_SECTOR_SIZE;
        }

        // 执行写入
        SPI_FLASH_WriteArray(pBuffer, WriteAddr, NumOfRestByte);
        // 执行读取
        SPI_FLASH_ReadArray(pFlashBuff, WriteAddr, NumOfRestByte);

        for (i = 0; i < NumOfRestByte; i++)
        {
            if (pBuffer[i] != pFlashBuff[i])
            {
                free(pFlashBuff);
                return FALSE;
            }
        }
    }
    else
    {
        // 执行写入
        SPI_FLASH_WriteArray(pBuffer, WriteAddr, NumByteToWrite);
        // 执行读取
        SPI_FLASH_ReadArray(pFlashBuff, WriteAddr, NumByteToWrite);

        for (i = 0; i < NumByteToWrite; i++)
        {
            if (pBuffer[i] != pFlashBuff[i])
            {
                free(pFlashBuff);
                return FALSE;
            }
        }
    }

    free(pFlashBuff);

    return TRUE;
}

void SPI_FLASH_WriteByte(uint32 WriteAddr, uint8 data)
{
    uint8 buff[4];

    buff[0] = (data) & 0xFF;

    SPI_FLASH_WriteWithErase(buff, WriteAddr, 1);
}

void SPI_FLASH_WriteHalfWord(uint32 WriteAddr, uint16 data)
{
    uint8 buff[4];

    buff[0] = (data >> 8) & 0xFF;
    buff[1] = (data) & 0xFF;

    SPI_FLASH_WriteWithErase(buff, WriteAddr, 2);
}

void SPI_FLASH_WriteWord(uint32 WriteAddr, uint32 data)
{
    uint8 buff[4];

    buff[0] = (data >> 24) & 0xFF;
    buff[1] = (data >> 16) & 0xFF;
    buff[2] = (data >> 8) & 0xFF;
    buff[3] = (data) & 0xFF;

    SPI_FLASH_WriteWithErase(buff, WriteAddr, 4);
}

/*********************************************************************
* 函   数  名: SPI_FLASH_WriteWithErase
* 函数入参: uint8* pBuffer: 数据存储区
            uint32 WriteAddr: 开始写入的地址(24bit)
            uint16 NumByteToWrite: 要写入的字节数(最大65535)
* 函数出参: 无
* 返   回  值: void
* 功能描述: 在指定地址开始写入指定长度的数据，该函数带擦除操作!
***********
* 修改历史:
*   1.修改作者: ML
*     修改日期: 2018/7/11
*     修改描述: 新函数
**********************************************************************/
void SPI_FLASH_WriteWithErase(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    uint32 sectPos = 0;
    uint16 sectOff = 0;
    uint16 sectRemain = 0;
    uint16 i = 0;
    uint8 *pFlashBuff = NULL;               // 保存读取扇区的数据

    if ((NULL == pBuffer) || (0 == NumByteToWrite))
    {
        return;
    }

    // 申请4096Byte空间保存扇区数据
    pFlashBuff = (uint8 *)malloc(SPI_FLASH_SECTOR_SIZE);

    if (NULL == pFlashBuff)
    {
        return;
    }

    sectPos = WriteAddr / SPI_FLASH_SECTOR_SIZE; // 扇区地址
    sectOff = WriteAddr % SPI_FLASH_SECTOR_SIZE; // 在扇区内的偏移
    sectRemain = SPI_FLASH_SECTOR_SIZE - sectOff; // 扇区剩余空间大小

    if (NumByteToWrite <= sectRemain)
    {
        sectRemain = NumByteToWrite;            // 不大于4096个字节
    }

    while (1)
    {
        // 读出整个扇区的内容
        SPI_FLASH_ReadArray(pFlashBuff, sectPos * SPI_FLASH_SECTOR_SIZE, SPI_FLASH_SECTOR_SIZE);

        for (i = 0; i < sectRemain; i++)    // 校验数据
        {
            if (pFlashBuff[sectOff + i] != 0xFF)
            {
                break;                      // 需要擦除
            }
        }

        if (i < sectRemain)                 // 需要擦除
        {
            SPI_FLASH_EraseSector(sectPos * SPI_FLASH_SECTOR_SIZE);// 擦除这个扇区

            // 复制
            for (i = 0; i < sectRemain; i++)
            {
                pFlashBuff[i + sectOff] = pBuffer[i];
            }

            // 写入整个扇区
            SPI_FLASH_WriteArray(pFlashBuff, sectPos * SPI_FLASH_SECTOR_SIZE, SPI_FLASH_SECTOR_SIZE);
        }
        else
        {
            // 写已经擦除了的,直接写入扇区剩余区间.
            SPI_FLASH_WriteArray(pBuffer, WriteAddr, sectRemain);
        }

        if (NumByteToWrite == sectRemain)
        {
            break;                                  // 写入结束了
        }
        else                                        // 写入未结束
        {
            sectPos++;                              // 扇区地址增1
            sectOff = 0;                            // 偏移位置为0

            pBuffer += sectRemain;                  // 指针偏移
            WriteAddr += sectRemain;                // 写地址偏移
            NumByteToWrite -= sectRemain;           // 字节数递减

            if (NumByteToWrite > SPI_FLASH_SECTOR_SIZE)
            {
                sectRemain = SPI_FLASH_SECTOR_SIZE; // 下一个扇区还是写不完
            }
            else
            {
                sectRemain = NumByteToWrite;        // 下一个扇区可以写完了
            }
        }
    }

    // 释放数据缓存区
    free(pFlashBuff);
}

// DMA硬件初始化
void DMAy_SPIx_Configuration(void)
{
    static uint16 txByte = 0xFF;    // 初始一个虚拟地址
    static uint16 rxByte = 0xFF;    // 初始一个虚拟地址

    // 使能时钟DMAy
    rcu_periph_clock_enable(RCU_DMA0);

    //SPIx Rx DMA
    dma_deinit(DMA0, DMA_CH1);
    dma_rx_init_struct.periph_addr  = STM32_SPIx_DR_Addr;           // 设置SPI发送外设地址(源地址)
    dma_rx_init_struct.memory_addr  = (uint32)(&rxByte);            // 设置SRAM存储地址(目的地址)
    dma_rx_init_struct.direction    = DMA_PERIPHERAL_TO_MEMORY;     // 传输方向外设-内存
    dma_rx_init_struct.number       = 0;                            // 设置SPI发送长度
    dma_rx_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;  // 外设地址不增加
    dma_rx_init_struct.memory_inc   = DMA_MEMORY_INCREASE_DISABLE;  // 内存地址不增加
    dma_rx_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;   // 外设的传输数据宽度16位
    dma_rx_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;       // 存储器的传输数据宽度16位
    dma_circulation_disable(DMA0, DMA_CH1);                         // 不循环
    dma_rx_init_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;      // 通道优先级为高
    dma_memory_to_memory_disable(DMA0, DMA_CH1);                    // 失能内存到内存模式
    dma_init(DMA0, DMA_CH1, &dma_rx_init_struct);                   // 初始化DMA0

    //SPIx Tx DMA
    dma_deinit(DMA0, DMA_CH2);
    dma_tx_init_struct.periph_addr  = STM32_SPIx_DR_Addr;           // 设置SPI发送外设地址(源地址)
    dma_tx_init_struct.memory_addr  = (uint32)(&txByte);            // 设置SRAM存储地址(目的地址)
    dma_tx_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;     // 传输方向内存-外设
    dma_tx_init_struct.number       = 0;                            // 设置SPI发送长度
    dma_tx_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;  // 外设地址不增加
    dma_tx_init_struct.memory_inc   = DMA_MEMORY_INCREASE_DISABLE;  // 内存地址不增加
    dma_tx_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;       // 存储器的传输数据宽度16位
    dma_tx_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;   // 外设的传输数据宽度16位
    dma_circulation_disable(DMA0, DMA_CH2);                         // 不循环
    dma_tx_init_struct.priority     = DMA_PRIORITY_MEDIUM;          // 中断方式
    dma_memory_to_memory_disable(DMA0, DMA_CH2);                    // 失能内存到内存模式
    dma_init(DMA0, DMA_CH2, &dma_tx_init_struct);                   // 初始化DMA0


    // 关闭DMA发送/接收中断，清DMA标记，禁止DMA1_TX，RX的传输完成中断
    spi_dma_disable(STM32_SPIx_NUM_DEF, SPI_DMA_TRANSMIT);
    spi_dma_disable(STM32_SPIx_NUM_DEF, SPI_DMA_RECEIVE);
    dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_G);
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_G);
    dma_interrupt_disable(DMA0, DMA_CH1, DMA_INT_FTF);
    dma_interrupt_disable(DMA0, DMA_CH2, DMA_INT_FTF);

    // 使能SPIx的DMA发送，接收请求
    spi_dma_enable(STM32_SPIx_NUM_DEF, SPI_DMA_TRANSMIT);
    spi_dma_enable(STM32_SPIx_NUM_DEF, SPI_DMA_RECEIVE);

    // 使能NVIC中断
    NVIC_DMAy_Configuration();


}

// 配置DMA通道中断
static void NVIC_DMAy_Configuration(void)
{
    nvic_irq_enable(DMA0_Channel1_IRQn, 2, 0);                      // 设置中断优先级
    nvic_irq_enable(DMA0_Channel2_IRQn, 2, 0);                      // 设置中断优先级
}

// DMA接收中断RX
void DMA0_Channel1_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA0, DMA_CH1, DMA_INT_FLAG_FTF))
    {
#if STM32_DMA_SPI_DIRECTION
        dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_G);
        dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_G);

        while (spi_i2s_flag_get(STM32_SPIx_NUM_DEF, I2S_FLAG_TRANS) != RESET);

        // 禁止SPI
        spi_disable(STM32_SPIx_NUM_DEF);
        // 关闭发送，接收DMA
        spi_dma_disable(SPI0, SPI_DMA_TRANSMIT);
        spi_dma_disable(SPI0, SPI_DMA_RECEIVE);

        // 禁止接收TC中断
        dma_interrupt_disable(DMA0, DMA_CH1, DMA_INT_FTF);

        SPI_FLASH_CS_H();  // 结束片选

        // 设置DMA完成标识
        spiCB.dmaFinished = TRUE;
#else

        dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_FTF);
        // 注意:当设置模式为双线只接收或单线接收模式下，不能在此加入忙BSY的判断死等，此时该标志已失效
        // 传输的完成就已经是彻底的接收完成，无需别的动作

        // 禁止SPI
        spi_disable(STM32_SPIx_NUM_DEF);
        // 关闭接收DMA
        dma_channel_disable(DMA0, DMA_CH1);
        // 禁止接收TC中断
        dma_interrupt_disable(DMA0, DMA_CH1, DMA_INT_FTF);

        SPI_FLASH_CS_H();  // 结束片选

        // 设置DMA完成标识
        spiCB.dmaFinished = TRUE;
#endif
    }
}

// DMA发送中断TX
void DMA0_Channel2_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA0, DMA_CH2, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_FTF);

        // 注意:上面为止DMA已经将所有的数据搬运到SPI_DR，但是，但是，但是实际上SPI数据还没全发送出去
        // 因此，软件等待忙标志，确保所有数据都发送出去才能将其关闭
        while (spi_i2s_flag_get(STM32_SPIx_NUM_DEF, I2S_FLAG_TRANS) != RESET);

        // 关闭发送DMA
        spi_dma_disable(SPI0, SPI_DMA_TRANSMIT);
        // 关闭发送TC中断
        dma_interrupt_disable(DMA0, DMA_CH2, DMA_INT_FTF);

        SPI_FLASH_CS_H();  // 结束片选
    }
}

// DMA发送(只支持也写256Byte)
void SPIx_DMAy_SendBuffer(uint8 *sendBuffer, uint32 WriteAddr, uint16 ndtr)
{
#if STM32_DMA_SPI_DIRECTION
    // 关闭DMA
    spi_dma_disable(SPI0, SPI_DMA_TRANSMIT);

    dma_tx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_tx_init_struct.memory_addr = (uint32)sendBuffer;
    dma_tx_init_struct.number = ndtr;
    dma_init(DMA0, DMA_CH2, &dma_tx_init_struct);

    // 片选
    SPI_FLASH_CS_L();
    // 发送页写入命令
    SPI_FLASH_SendByte(CMD_PAGE_WRITE);
    // 写入24位起始地址
    SPI_FLASH_SendByte((WriteAddr >> 16) & 0xFF);
    SPI_FLASH_SendByte((WriteAddr >> 8) & 0xFF);
    SPI_FLASH_SendByte(WriteAddr & 0xFF);

    // 清DMA标记
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_G);

    // 使能TX的TC中断
    dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF);

    // 开启DMA通道DMA0_Channel2
    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);

#else

    // 先禁止SPI以修改配置
    spi_disable(STM32_SPIx_NUM_DEF);

    // 关闭DMA
    spi_dma_disable(SPI0, SPI_DMA_TRANSMIT);

    // 全双工发送接收模式，此时不用理会接收的数据
    // 数据的开始以填数据进SPI_DR为启动时刻
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    dma_tx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_tx_init_struct.memory_addr = (uint32)sendBuffer;
    dma_tx_init_struct.number = ndtr;
    dma_init(DMA0, DMA_CH2, &dma_tx_init_struct);

    // 使能SPI，使用双工模式发送命令和地址
    spi_enable(STM32_SPIx_NUM_DEF);

    // 片选
    SPI_FLASH_CS_L();
    // 发送页写入命令
    SPI_FLASH_SendByte(CMD_PAGE_WRITE);
    // 写入24位起始地址
    SPI_FLASH_SendByte((WriteAddr >> 16) & 0xFF);
    SPI_FLASH_SendByte((WriteAddr >> 8) & 0xFF);
    SPI_FLASH_SendByte(WriteAddr & 0xFF);

    // 清DMA标记
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_G);

    // 使能TX的TC中断
    dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF);

    // 开启DMA通道DMA0_Channel2
    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);

#endif
}

// 启动DMA方式读取
void SPIx_DMAy_Read(uint32 srcAddr, uint32 dstAddr, uint16 count)
{
#if STM32_DMA_SPI_DIRECTION

    static uint16 txByte = 0xFF; // dummy

    // 入参检查
    if (0 == count)
    {
        return;
    }

    // 设置DMA完成标识
    spiCB.dmaFinished = FALSE;
    // 先禁止SPI以修改配置
    spi_disable(STM32_SPIx_NUM_DEF);

    // 关闭DMA
    dma_channel_disable(DMA0, DMA_CH1);
    dma_channel_disable(DMA1, DMA_CH2);

    // 先配置全双工模式，以在DMA之前写命令和地址
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // 重新配置接收地址以及接收数量
    dma_rx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE; // 内存地址增量变化
    dma_rx_init_struct.memory_addr = (uint32)dstAddr;
    dma_rx_init_struct.number = count;
    dma_init(DMA0, DMA_CH1, &dma_rx_init_struct);

    // 发送为dummy
    dma_tx_init_struct.memory_inc = DMA_MEMORY_INCREASE_DISABLE;// 内存地址增量不变化
    dma_tx_init_struct.memory_addr = (uint32)(&txByte);
    dma_tx_init_struct.number = count;
    dma_init(DMA0, DMA_CH2, &dma_tx_init_struct);

    // 使能SPI，使用双工模式发送命令和地址
    spi_enable(STM32_SPIx_NUM_DEF);

    // 片选
    SPI_FLASH_CS_L();
    // 发送读取命令
    SPI_FLASH_SendByte(CMD_READ_BYTE);
    // 发送24位起始地址
    SPI_FLASH_SendByte((srcAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((srcAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(srcAddr & 0xFF);
    // 接收前读一次数据DR，保证接收缓冲区为空
    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);
    // 再次禁止SPI以修改配置全双工模式
    spi_disable(STM32_SPIx_NUM_DEF);

    // 配置为全双工16位模式
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size = SPI_FRAMESIZE_16BIT;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // 清DMA标记
    dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_G);
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_G);

    // 使能RX的TC中断
    dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF);

    // 重新使能SPI，SCK时钟立刻开始输出，开始发送接收数据
    spi_enable(STM32_SPIx_NUM_DEF);
    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);

    // 开启DMA通道DMA1_Channel2，DMA1_Channel3
    dma_channel_enable(DMA0, DMA_CH1);
    dma_channel_enable(DMA1, DMA_CH2);

    while (!spiCB.dmaFinished);

    // DMA读完后，模式重新复位
    SPIx_ConfigureWorkMode(SPI_TRANSMODE_FULLDUPLEX, SPI_FRAMESIZE_8BIT);

#else

    // 入参检查
    if (0 == count)
    {
        return ;
    }

    // 设置DMA完成标识
    spiCB.dmaFinished = FALSE;

    // 先禁止SPI以修改配置
    spi_disable(STM32_SPIx_NUM_DEF);

    // 关闭DMA
    dma_channel_disable(DMA0, DMA_CH1);

    // 先配置全双工模式，以在DMA之前写命令和地址
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // 重新配置接收地址以及接收数量
    dma_rx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE; // 内存地址增量变化
    dma_rx_init_struct.memory_addr = (uint32)dstAddr;
    dma_rx_init_struct.number = count;
    dma_init(DMA0, DMA_CH1, &dma_rx_init_struct);

    // 使能SPI，使用双工模式发送命令和地址
    spi_enable(STM32_SPIx_NUM_DEF);
    // 片选
    SPI_FLASH_CS_L();
    // 发送读取命令
    SPI_FLASH_SendByte(CMD_READ_BYTE);
    // 发送24位起始地址
    SPI_FLASH_SendByte((srcAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((srcAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(srcAddr & 0xFF);
    // 接收前读一次数据DR，保证接收缓冲区为空
    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);
    // 再次禁止SPI以修改配置为只接收模式
    spi_disable(STM32_SPIx_NUM_DEF);
    // 配置为只接收16位模式
    spi_init_struct.trans_mode = SPI_TRANSMODE_RECEIVEONLY;
    spi_init_struct.frame_size = SPI_FRAMESIZE_16BIT;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // 清DMA标记
    dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_G);// 清RX全局中断标志
    // 使能RX的TC中断
    dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF);

    // 重新使能SPI，SCK时钟立刻开始输出，开始接收数据
    spi_enable(STM32_SPIx_NUM_DEF);
    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);

    // 开启DMA通道DMA0_Channel1
    dma_channel_enable(DMA0, DMA_CH1);

    while (!spiCB.dmaFinished);

    // DMA读完后，模式重新复位
    SPIx_ConfigureWorkMode(SPI_TRANSMODE_FULLDUPLEX, SPI_FRAMESIZE_8BIT);
#endif
}

// 配置SPI工作模式
void SPIx_ConfigureWorkMode(uint16 SpiDirection, uint16 SpiDataSize)
{
    spi_init_struct.trans_mode = SpiDirection;
    spi_init_struct.frame_size = SpiDataSize;

    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);
    spi_enable(STM32_SPIx_NUM_DEF);

    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);
}


