#include "spiflash.h"
#include "common.h"
#include "Watchdog.h"

spi_parameter_struct spi_init_struct;
dma_parameter_struct dma_tx_init_struct;
dma_parameter_struct dma_rx_init_struct;

SPI_IF_CB spiCB;

// DMA�ж�����
static void NVIC_DMAy_Configuration(void);

// DMAӲ����ʼ��
static void DMAy_SPIx_Configuration(void);


// ��Ԫ����
#if SPI_FLASH_UNIT_TEST
#define SPI_BUFF_SIZE                   256
uint8 SPI_TX_Buff[SPI_BUFF_SIZE];
uint8 SPI_RX_Buff[SPI_BUFF_SIZE];
uint16 DeviceID;
void SPI_FLASH_UnitTest(void)
{
    uint16 i;

    DeviceID = SPI_FLASH_ReadChipID();

    // ��������flash�ռ�
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
    // DMA��ʽ��ȡ����
    //SPIx_DMAy_Read(0, (uint32)SPI_RX_Buff, SPI_BUFF_SIZE);
}
#endif


// ��ʼ��SPI Flash�����˿�
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
//      // ʹ��GPIOʱ��
//      rcu_periph_clock_enable(RCU_GPIOB);
//
//      // SPI0 GPIO config: SCK/PA5-> PB3, MOSI/PA7 - > PB5
//      gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);

//      // SPI0 GPIO config: MISO/PA6 ->PB4
//      gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_4);

//      // SPI0 GPIO config: NSS/PA4 -> PE1
//      gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
//
//      // SPIx����ѡ��
//      rcu_periph_clock_enable(RCU_SPI0);
//
//  }
//  else
//  {
//      // ʹ��GPIOʱ��
//      rcu_periph_clock_enable(RCU_GPIOB);
//
//      // SPI1 GPIO config: SCK/PB13, MOSI/PB15
//      gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 |GPIO_PIN_15);

//      // SPI1 GPIO config: MISO/PB14
//      gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);

//      // SPI1 GPIO config: NSS/PB12
//      gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);

//      // SPIx����ѡ��
//      rcu_periph_clock_enable(RCU_SPI1);
//  }
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOE);
    rcu_periph_clock_enable(RCU_SPI2);
    rcu_periph_clock_enable(RCU_AF);

    //����JTAG���Ź��ܣ���������AF�Ĵ���������Ҫ��ʹ��AFʱ��
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
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;              // ����ʱCLK���ף��ڵ�һ��ʱ�������زɼ���һ������
    spi_init_struct.nss                  = SPI_NSS_SOFT;                        // Ƭѡ�������

    // APB2 - SPI0 - 72M
    // APB1 - SPI1/SPI2 - 36M
    spi_init_struct.prescale             = SPI_PSC_4;                           // 72/4=18M
    spi_init_struct.endian               = SPI_ENDIAN_MSB;                      // �ȷ��͸�λ
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // ʹ��SPIx
    spi_enable(STM32_SPIx_NUM_DEF);

    // ʹ��DMA����
    DMAy_SPIx_Configuration();

    spiCB.dmaFinished = FALSE;

#if SPI_FLASH_UNIT_TEST
    SPI_FLASH_UnitTest();
#endif
}

// ��������
void SPI_FLASH_EraseSector(uint32 SectorAddr)
{
    // ʹ��д�����
    SPI_FLASH_WriteEnable();

    // Ƭѡ
    SPI_FLASH_CS_L();

    // ����������������
    SPI_FLASH_SendByte(CMD_SECTOR_ERASE_4K);

    // ����24λ��ַ
    SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(SectorAddr & 0xFF);

    // �ͷ�
    SPI_FLASH_CS_H();

    // �ȴ�д�����
    SPI_FLASH_WaitForWriteEnd();
}

// ������
void SPI_FLASH_EraseBlock(uint32 BlockAddr)
{
    // ʹ��д�����
    SPI_FLASH_WriteEnable();

    // Ƭѡ
    SPI_FLASH_CS_L();

    // ����64K��������
    SPI_FLASH_SendByte(CMD_BLOCK_ERASE_64K);

    // ����24λ��ַ
    SPI_FLASH_SendByte((BlockAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((BlockAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(BlockAddr & 0xFF);

    // �ͷ�
    SPI_FLASH_CS_H();

    // �ȴ�д�����
    SPI_FLASH_WaitForWriteEnd();
}


// ��������Flash
void SPI_FLASH_EraseChip(void)
{
    // ʹ��д�����
    SPI_FLASH_WriteEnable();

    // Ƭѡ
    SPI_FLASH_CS_L();

    // ����Ƭ��������
    SPI_FLASH_SendByte(CMD_CHIP_ERASE);

    // �ͷ�
    SPI_FLASH_CS_H();

    // �ȴ�д�����
    SPI_FLASH_WaitForWriteEnd();
}

// ҳд��
void SPI_FLASH_WritePage(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    // д��������Ϊ0���˳�
    if (0 == NumByteToWrite)
    {
        return ;
    }

    // ʹ��д�����
    SPI_FLASH_WriteEnable();

    // Ƭѡ
    SPI_FLASH_CS_L();

    // ����ҳд������
    SPI_FLASH_SendByte(CMD_PAGE_WRITE);

    // д��24λ��ʼ��ַ
    SPI_FLASH_SendByte((WriteAddr >> 16) & 0xFF);
    SPI_FLASH_SendByte((WriteAddr >> 8) & 0xFF);
    SPI_FLASH_SendByte(WriteAddr & 0xFF);

    // ����д��ָ������������
    while (NumByteToWrite--)
    {
        /// д��һ���ֽ�
        SPI_FLASH_SendByte(*pBuffer);

        pBuffer++;
    }

    // �ͷ�
    SPI_FLASH_CS_H();

    // �ȴ�д�����
    SPI_FLASH_WaitForWriteEnd();
}

// ��ָ��������������д��ָ����ʼ��ַ������
void SPI_FLASH_WriteArray(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    uint8 NumOfPage = 0, NumOfRestByte = 0, StartAddrOffset = 0, RestSpace = 0, temp = 0;

    // ָ��ǿ��ж�
    if (NULL == pBuffer)
    {
        return;
    }

    // д��������Ϊ0���˳�
    if (0 == NumByteToWrite)
    {
        return ;
    }

    StartAddrOffset = WriteAddr % SPI_FLASH_PAGE_SIZE;
    RestSpace = SPI_FLASH_PAGE_SIZE - StartAddrOffset;
    NumOfPage =  NumByteToWrite / SPI_FLASH_PAGE_SIZE;
    NumOfRestByte = NumByteToWrite % SPI_FLASH_PAGE_SIZE;

    // ��ʼ��ַ�պ�λ�ڵ�ǰҳ�����
    if (StartAddrOffset == 0)
    {
        // ����������һҳ��ֱ��д��
        if (NumOfPage == 0)
        {
            SPI_FLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
        }
        // ����������һҳ
        else
        {
            // ��ҳ�Ĳ��֣�ֱ��д����ҳ
            while (NumOfPage--)
            {
                // �ڵ�ǰҳд��
                SPI_FLASH_WritePage(pBuffer, WriteAddr, SPI_FLASH_PAGE_SIZE);

                WriteAddr += SPI_FLASH_PAGE_SIZE;
                pBuffer += SPI_FLASH_PAGE_SIZE;
            }

            // д�벻��һҳ�Ĳ���
            SPI_FLASH_WritePage(pBuffer, WriteAddr, NumOfRestByte);
        }
    }
    // ��ʼ��ַ���ǵ�ǰҳ����㣬�����м����д��
    else
    {
        // ����������һҳ��ֱ��д��
        if (NumOfPage == 0)
        {
            // Ҫд��������������˵�ǰҳʣ��ռ䣬��ֿ�����д��
            if (NumOfRestByte > RestSpace)
            {
                // �Ƚ���ǰҳ����
                temp = NumOfRestByte - RestSpace;

                SPI_FLASH_WritePage(pBuffer, WriteAddr, RestSpace);
                WriteAddr +=  RestSpace;
                pBuffer += RestSpace;

                // �ٽ�ʣ�������д����һҳ
                SPI_FLASH_WritePage(pBuffer, WriteAddr, temp);
            }
            // Ҫд�����������������ǰҳ��ʣ��ռ䣬��ֱ��д��
            else
            {
                SPI_FLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
            }
        }
        // ����������һҳ
        else
        {
            // �Ƚ���ǰҳ����
            SPI_FLASH_WritePage(pBuffer, WriteAddr, RestSpace);
            WriteAddr +=  RestSpace;
            pBuffer += RestSpace;

            // ʣ������ݣ�д����������һҳ������ˣ��������һ�������ͬ�Ĵ���
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

// ��ȡ������ָ�������������ݸ���������Ϊ��Ƭ����
void SPI_FLASH_ReadArray(uint8 *pBuffer, uint32 ReadAddr, uint16 NumByteToRead)
{
    // ָ��ǿ��ж�
    if (NULL == pBuffer)
    {
        return;
    }

    // Ҫ��ȡ��������Ϊ0���˳�
    if (0 == NumByteToRead)
    {
        return;
    }

    // Ƭѡ
    SPI_FLASH_CS_L();

    // ���Ͷ�ȡ����
    SPI_FLASH_SendByte(CMD_READ_BYTE);

    // ����24λ��ʼ��ַ
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

    // һֱ��ȡ��ָ��������Ϊֹ
    while (NumByteToRead--)
    {
        // ��ȡһ���ֽ�
        *(pBuffer++) = SPI_FLASH_SendByte(0);
    }

    // �ͷ�
    SPI_FLASH_CS_H();
}

// ��ȡоƬID
uint16 SPI_FLASH_ReadChipID(void)
{
    uint16 W25xXX_ID = 0;

    // ʹ��д�����
    SPI_FLASH_WriteEnable();

    SPI_FLASH_CS_L();

    // ���Ͷ�ȡID����
    SPI_FLASH_SendByte(CMD_MANUFACT_DEVICE_ID);

    // ����24λ�ĵ�ַ
    SPI_FLASH_SendByte(0x00);   // dummy
    SPI_FLASH_SendByte(0x00);   // dummy
    SPI_FLASH_SendByte(0x00);   // 0x00

    // ��ȡ���ص�16λֵ
    W25xXX_ID = SPI_FLASH_SendByte(0xFF);   // ��8λ����
    W25xXX_ID <<= 8;
    W25xXX_ID += SPI_FLASH_SendByte(0xFF);  // ��8λ����

    SPI_FLASH_CS_H();

    return W25xXX_ID;
}

// ����һ���ֽ�
uint8 SPI_FLASH_SendByte(uint8 byte)
{
    // ���ͼĴ�����Ϊ�գ��ȴ�
    uint16 spiRetryTime = 0;

    while (spi_i2s_flag_get(STM32_SPIx_NUM_DEF, SPI_STAT_TBE) == RESET)
    {
        if (spiRetryTime++ > SPI_FLASH_RETRY_READ_TIME)
        {
            return 0x00;
        }
    }
    // ����
    spi_i2s_data_transmit(STM32_SPIx_NUM_DEF, byte);

    // ���ռĴ���Ϊ�գ��ȴ�
    spiRetryTime = 0;
    while (spi_i2s_flag_get(STM32_SPIx_NUM_DEF, SPI_STAT_RBNE) == RESET)
    {
        if (spiRetryTime++ > SPI_FLASH_RETRY_READ_TIME)
        {
            return 0x00;
        }
    }

    // ���ؽ��յ���ֵ
    return spi_i2s_data_receive(STM32_SPIx_NUM_DEF);
}

// ����д��ʹ��ָ��
void SPI_FLASH_WriteEnable(void)
{
    // Ƭѡ
    SPI_FLASH_CS_L();

    // ����д��ʹ��ָ��
    SPI_FLASH_SendByte(CMD_WRITE_ENABLE);

    // �ͷ�
    SPI_FLASH_CS_H();
}

// д״̬�Ĵ���
void SPI_FLASH_WriteStatus(uint8 data)
{
    // Ƭѡ
    SPI_FLASH_CS_L();

    // ����д״̬�Ĵ���
    SPI_FLASH_SendByte(CMD_WRITE_REGISTER_ENABLE);

    // �ͷ�
    SPI_FLASH_CS_H();

    // Ƭѡ
    SPI_FLASH_CS_L();

    // ����д״̬�Ĵ���
    SPI_FLASH_SendByte(CMD_WRITE_STATUS_REGISTER);
    SPI_FLASH_SendByte(data);

    // �ͷ�
    SPI_FLASH_CS_H();
}

// �ȴ�д��������
void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8 flashstatus = 0;

    // Ƭѡ
    SPI_FLASH_CS_L();

    // ���Ͷ�ȡ״̬�Ĵ���1������
    SPI_FLASH_SendByte(CMD_READ_STATUS_REGISTER1);

    // д���У������ȴ�
    do
    {
        // ����һ�������ݣ����ɲ���SPIʱ��
        flashstatus = SPI_FLASH_SendByte(0);
    }
    while ((flashstatus & BUSY_FLAG) == BUSY_FLAG);

    // �ͷ�
    SPI_FLASH_CS_H();
}

// ����ָ���ռ� �Ȳ������ٲ����ٲ�����
void SPI_FLASH_EraseRoom(uint32 Addr, uint32 size)
{

    uint16 blockStartNum = 0;       // ���
    uint16 blockCount = 0;          // ��Ҫ�����������
    uint16 blockSectorOffset = 0;   // ��������ƫ����
    uint16 i;

    blockSectorOffset = Addr / SPI_FLASH_SECTOR_SIZE % SPI_FLASH_BLOCK_PER_16SECTORS;

    if (blockSectorOffset)          // ���������ڿ��ڴ���ƫ����
    {
        blockStartNum = Addr / SPI_FLASH_BLOCK_SIZE + 1;
    }
    else                            // ������ʼ��ַ���ڿ����ʼ��ַ��������ƫ��
    {
        blockStartNum = Addr / SPI_FLASH_BLOCK_SIZE;
    }

    // ������ʼ��������ƫ��
    for (i = 0; i < SPI_FLASH_BLOCK_PER_16SECTORS - blockSectorOffset; i++)
    {
        SPI_FLASH_EraseSector((Addr / SPI_FLASH_SECTOR_SIZE + i) * SPI_FLASH_SECTOR_SIZE);

        WDT_Clear();
    }

    // �������Ҫ�����Ŀ������
    blockCount = ((size / SPI_FLASH_SECTOR_SIZE) - (SPI_FLASH_BLOCK_PER_16SECTORS - blockSectorOffset)) / SPI_FLASH_BLOCK_PER_16SECTORS;

    // ������
    for (i = 0; i < blockCount; i++)
    {
        SPI_FLASH_EraseBlock((blockStartNum + i) * SPI_FLASH_BLOCK_SIZE);

        WDT_Clear();
    }

    // ����ʣ������,ʣ�����������һ����
    for (i = 0; i < ((size / SPI_FLASH_SECTOR_SIZE) - (SPI_FLASH_BLOCK_PER_16SECTORS - blockSectorOffset)) % SPI_FLASH_BLOCK_PER_16SECTORS; i++)
    {
        SPI_FLASH_EraseSector((blockStartNum + blockCount) * SPI_FLASH_BLOCK_SIZE + i * SPI_FLASH_SECTOR_SIZE);

        WDT_Clear();
    }
}


// ����д��ӿڣ�����д����
BOOL SPI_FLASH_WriteWithCheck(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    uint16 NumOfInt = 0;
    uint16 NumOfRestByte = 0;
    uint16 i;
    uint8 *pFlashBuff = NULL;               // �����ȡ����������

    if ((NULL == pBuffer) || (0 == NumByteToWrite))
    {
        return FALSE;
    }

    // ����4096Byte�ռ䱣����������
    pFlashBuff = (uint8 *)malloc(SPI_FLASH_SECTOR_SIZE);

    // ָ��ǿ��ж�
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
            // ִ��д��
            SPI_FLASH_WriteArray(pBuffer, WriteAddr, SPI_FLASH_SECTOR_SIZE);
            // ִ�ж�ȡ
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

        // ִ��д��
        SPI_FLASH_WriteArray(pBuffer, WriteAddr, NumOfRestByte);
        // ִ�ж�ȡ
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
        // ִ��д��
        SPI_FLASH_WriteArray(pBuffer, WriteAddr, NumByteToWrite);
        // ִ�ж�ȡ
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
* ��   ��  ��: SPI_FLASH_WriteWithErase
* �������: uint8* pBuffer: ���ݴ洢��
            uint32 WriteAddr: ��ʼд��ĵ�ַ(24bit)
            uint16 NumByteToWrite: Ҫд����ֽ���(���65535)
* ��������: ��
* ��   ��  ֵ: void
* ��������: ��ָ����ַ��ʼд��ָ�����ȵ����ݣ��ú�������������!
***********
* �޸���ʷ:
*   1.�޸�����: ML
*     �޸�����: 2018/7/11
*     �޸�����: �º���
**********************************************************************/
void SPI_FLASH_WriteWithErase(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    uint32 sectPos = 0;
    uint16 sectOff = 0;
    uint16 sectRemain = 0;
    uint16 i = 0;
    uint8 *pFlashBuff = NULL;               // �����ȡ����������

    if ((NULL == pBuffer) || (0 == NumByteToWrite))
    {
        return;
    }

    // ����4096Byte�ռ䱣����������
    pFlashBuff = (uint8 *)malloc(SPI_FLASH_SECTOR_SIZE);

    if (NULL == pFlashBuff)
    {
        return;
    }

    sectPos = WriteAddr / SPI_FLASH_SECTOR_SIZE; // ������ַ
    sectOff = WriteAddr % SPI_FLASH_SECTOR_SIZE; // �������ڵ�ƫ��
    sectRemain = SPI_FLASH_SECTOR_SIZE - sectOff; // ����ʣ��ռ��С

    if (NumByteToWrite <= sectRemain)
    {
        sectRemain = NumByteToWrite;            // ������4096���ֽ�
    }

    while (1)
    {
        // ������������������
        SPI_FLASH_ReadArray(pFlashBuff, sectPos * SPI_FLASH_SECTOR_SIZE, SPI_FLASH_SECTOR_SIZE);

        for (i = 0; i < sectRemain; i++)    // У������
        {
            if (pFlashBuff[sectOff + i] != 0xFF)
            {
                break;                      // ��Ҫ����
            }
        }

        if (i < sectRemain)                 // ��Ҫ����
        {
            SPI_FLASH_EraseSector(sectPos * SPI_FLASH_SECTOR_SIZE);// �����������

            // ����
            for (i = 0; i < sectRemain; i++)
            {
                pFlashBuff[i + sectOff] = pBuffer[i];
            }

            // д����������
            SPI_FLASH_WriteArray(pFlashBuff, sectPos * SPI_FLASH_SECTOR_SIZE, SPI_FLASH_SECTOR_SIZE);
        }
        else
        {
            // д�Ѿ������˵�,ֱ��д������ʣ������.
            SPI_FLASH_WriteArray(pBuffer, WriteAddr, sectRemain);
        }

        if (NumByteToWrite == sectRemain)
        {
            break;                                  // д�������
        }
        else                                        // д��δ����
        {
            sectPos++;                              // ������ַ��1
            sectOff = 0;                            // ƫ��λ��Ϊ0

            pBuffer += sectRemain;                  // ָ��ƫ��
            WriteAddr += sectRemain;                // д��ַƫ��
            NumByteToWrite -= sectRemain;           // �ֽ����ݼ�

            if (NumByteToWrite > SPI_FLASH_SECTOR_SIZE)
            {
                sectRemain = SPI_FLASH_SECTOR_SIZE; // ��һ����������д����
            }
            else
            {
                sectRemain = NumByteToWrite;        // ��һ����������д����
            }
        }
    }

    // �ͷ����ݻ�����
    free(pFlashBuff);
}

// DMAӲ����ʼ��
void DMAy_SPIx_Configuration(void)
{
    static uint16 txByte = 0xFF;    // ��ʼһ�������ַ
    static uint16 rxByte = 0xFF;    // ��ʼһ�������ַ

    // ʹ��ʱ��DMAy
    rcu_periph_clock_enable(RCU_DMA0);

    //SPIx Rx DMA
    dma_deinit(DMA0, DMA_CH1);
    dma_rx_init_struct.periph_addr  = STM32_SPIx_DR_Addr;           // ����SPI���������ַ(Դ��ַ)
    dma_rx_init_struct.memory_addr  = (uint32)(&rxByte);            // ����SRAM�洢��ַ(Ŀ�ĵ�ַ)
    dma_rx_init_struct.direction    = DMA_PERIPHERAL_TO_MEMORY;     // ���䷽������-�ڴ�
    dma_rx_init_struct.number       = 0;                            // ����SPI���ͳ���
    dma_rx_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;  // �����ַ������
    dma_rx_init_struct.memory_inc   = DMA_MEMORY_INCREASE_DISABLE;  // �ڴ��ַ������
    dma_rx_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;   // ����Ĵ������ݿ��16λ
    dma_rx_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;       // �洢���Ĵ������ݿ��16λ
    dma_circulation_disable(DMA0, DMA_CH1);                         // ��ѭ��
    dma_rx_init_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;      // ͨ�����ȼ�Ϊ��
    dma_memory_to_memory_disable(DMA0, DMA_CH1);                    // ʧ���ڴ浽�ڴ�ģʽ
    dma_init(DMA0, DMA_CH1, &dma_rx_init_struct);                   // ��ʼ��DMA0

    //SPIx Tx DMA
    dma_deinit(DMA0, DMA_CH2);
    dma_tx_init_struct.periph_addr  = STM32_SPIx_DR_Addr;           // ����SPI���������ַ(Դ��ַ)
    dma_tx_init_struct.memory_addr  = (uint32)(&txByte);            // ����SRAM�洢��ַ(Ŀ�ĵ�ַ)
    dma_tx_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;     // ���䷽���ڴ�-����
    dma_tx_init_struct.number       = 0;                            // ����SPI���ͳ���
    dma_tx_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;  // �����ַ������
    dma_tx_init_struct.memory_inc   = DMA_MEMORY_INCREASE_DISABLE;  // �ڴ��ַ������
    dma_tx_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;       // �洢���Ĵ������ݿ��16λ
    dma_tx_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;   // ����Ĵ������ݿ��16λ
    dma_circulation_disable(DMA0, DMA_CH2);                         // ��ѭ��
    dma_tx_init_struct.priority     = DMA_PRIORITY_MEDIUM;          // �жϷ�ʽ
    dma_memory_to_memory_disable(DMA0, DMA_CH2);                    // ʧ���ڴ浽�ڴ�ģʽ
    dma_init(DMA0, DMA_CH2, &dma_tx_init_struct);                   // ��ʼ��DMA0


    // �ر�DMA����/�����жϣ���DMA��ǣ���ֹDMA1_TX��RX�Ĵ�������ж�
    spi_dma_disable(STM32_SPIx_NUM_DEF, SPI_DMA_TRANSMIT);
    spi_dma_disable(STM32_SPIx_NUM_DEF, SPI_DMA_RECEIVE);
    dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_G);
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_G);
    dma_interrupt_disable(DMA0, DMA_CH1, DMA_INT_FTF);
    dma_interrupt_disable(DMA0, DMA_CH2, DMA_INT_FTF);

    // ʹ��SPIx��DMA���ͣ���������
    spi_dma_enable(STM32_SPIx_NUM_DEF, SPI_DMA_TRANSMIT);
    spi_dma_enable(STM32_SPIx_NUM_DEF, SPI_DMA_RECEIVE);

    // ʹ��NVIC�ж�
    NVIC_DMAy_Configuration();


}

// ����DMAͨ���ж�
static void NVIC_DMAy_Configuration(void)
{
    nvic_irq_enable(DMA0_Channel1_IRQn, 2, 0);                      // �����ж����ȼ�
    nvic_irq_enable(DMA0_Channel2_IRQn, 2, 0);                      // �����ж����ȼ�
}

// DMA�����ж�RX
void DMA0_Channel1_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA0, DMA_CH1, DMA_INT_FLAG_FTF))
    {
#if STM32_DMA_SPI_DIRECTION
        dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_G);
        dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_G);

        while (spi_i2s_flag_get(STM32_SPIx_NUM_DEF, I2S_FLAG_TRANS) != RESET);

        // ��ֹSPI
        spi_disable(STM32_SPIx_NUM_DEF);
        // �رշ��ͣ�����DMA
        spi_dma_disable(SPI0, SPI_DMA_TRANSMIT);
        spi_dma_disable(SPI0, SPI_DMA_RECEIVE);

        // ��ֹ����TC�ж�
        dma_interrupt_disable(DMA0, DMA_CH1, DMA_INT_FTF);

        SPI_FLASH_CS_H();  // ����Ƭѡ

        // ����DMA��ɱ�ʶ
        spiCB.dmaFinished = TRUE;
#else

        dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_FTF);
        // ע��:������ģʽΪ˫��ֻ���ջ��߽���ģʽ�£������ڴ˼���æBSY���ж����ȣ���ʱ�ñ�־��ʧЧ
        // �������ɾ��Ѿ��ǳ��׵Ľ�����ɣ������Ķ���

        // ��ֹSPI
        spi_disable(STM32_SPIx_NUM_DEF);
        // �رս���DMA
        dma_channel_disable(DMA0, DMA_CH1);
        // ��ֹ����TC�ж�
        dma_interrupt_disable(DMA0, DMA_CH1, DMA_INT_FTF);

        SPI_FLASH_CS_H();  // ����Ƭѡ

        // ����DMA��ɱ�ʶ
        spiCB.dmaFinished = TRUE;
#endif
    }
}

// DMA�����ж�TX
void DMA0_Channel2_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA0, DMA_CH2, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_FTF);

        // ע��:����ΪֹDMA�Ѿ������е����ݰ��˵�SPI_DR�����ǣ����ǣ�����ʵ����SPI���ݻ�ûȫ���ͳ�ȥ
        // ��ˣ�����ȴ�æ��־��ȷ���������ݶ����ͳ�ȥ���ܽ���ر�
        while (spi_i2s_flag_get(STM32_SPIx_NUM_DEF, I2S_FLAG_TRANS) != RESET);

        // �رշ���DMA
        spi_dma_disable(SPI0, SPI_DMA_TRANSMIT);
        // �رշ���TC�ж�
        dma_interrupt_disable(DMA0, DMA_CH2, DMA_INT_FTF);

        SPI_FLASH_CS_H();  // ����Ƭѡ
    }
}

// DMA����(ֻ֧��Ҳд256Byte)
void SPIx_DMAy_SendBuffer(uint8 *sendBuffer, uint32 WriteAddr, uint16 ndtr)
{
#if STM32_DMA_SPI_DIRECTION
    // �ر�DMA
    spi_dma_disable(SPI0, SPI_DMA_TRANSMIT);

    dma_tx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_tx_init_struct.memory_addr = (uint32)sendBuffer;
    dma_tx_init_struct.number = ndtr;
    dma_init(DMA0, DMA_CH2, &dma_tx_init_struct);

    // Ƭѡ
    SPI_FLASH_CS_L();
    // ����ҳд������
    SPI_FLASH_SendByte(CMD_PAGE_WRITE);
    // д��24λ��ʼ��ַ
    SPI_FLASH_SendByte((WriteAddr >> 16) & 0xFF);
    SPI_FLASH_SendByte((WriteAddr >> 8) & 0xFF);
    SPI_FLASH_SendByte(WriteAddr & 0xFF);

    // ��DMA���
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_G);

    // ʹ��TX��TC�ж�
    dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF);

    // ����DMAͨ��DMA0_Channel2
    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);

#else

    // �Ƚ�ֹSPI���޸�����
    spi_disable(STM32_SPIx_NUM_DEF);

    // �ر�DMA
    spi_dma_disable(SPI0, SPI_DMA_TRANSMIT);

    // ȫ˫�����ͽ���ģʽ����ʱ���������յ�����
    // ���ݵĿ�ʼ�������ݽ�SPI_DRΪ����ʱ��
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    dma_tx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_tx_init_struct.memory_addr = (uint32)sendBuffer;
    dma_tx_init_struct.number = ndtr;
    dma_init(DMA0, DMA_CH2, &dma_tx_init_struct);

    // ʹ��SPI��ʹ��˫��ģʽ��������͵�ַ
    spi_enable(STM32_SPIx_NUM_DEF);

    // Ƭѡ
    SPI_FLASH_CS_L();
    // ����ҳд������
    SPI_FLASH_SendByte(CMD_PAGE_WRITE);
    // д��24λ��ʼ��ַ
    SPI_FLASH_SendByte((WriteAddr >> 16) & 0xFF);
    SPI_FLASH_SendByte((WriteAddr >> 8) & 0xFF);
    SPI_FLASH_SendByte(WriteAddr & 0xFF);

    // ��DMA���
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_G);

    // ʹ��TX��TC�ж�
    dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF);

    // ����DMAͨ��DMA0_Channel2
    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);

#endif
}

// ����DMA��ʽ��ȡ
void SPIx_DMAy_Read(uint32 srcAddr, uint32 dstAddr, uint16 count)
{
#if STM32_DMA_SPI_DIRECTION

    static uint16 txByte = 0xFF; // dummy

    // ��μ��
    if (0 == count)
    {
        return;
    }

    // ����DMA��ɱ�ʶ
    spiCB.dmaFinished = FALSE;
    // �Ƚ�ֹSPI���޸�����
    spi_disable(STM32_SPIx_NUM_DEF);

    // �ر�DMA
    dma_channel_disable(DMA0, DMA_CH1);
    dma_channel_disable(DMA1, DMA_CH2);

    // ������ȫ˫��ģʽ������DMA֮ǰд����͵�ַ
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // �������ý��յ�ַ�Լ���������
    dma_rx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE; // �ڴ��ַ�����仯
    dma_rx_init_struct.memory_addr = (uint32)dstAddr;
    dma_rx_init_struct.number = count;
    dma_init(DMA0, DMA_CH1, &dma_rx_init_struct);

    // ����Ϊdummy
    dma_tx_init_struct.memory_inc = DMA_MEMORY_INCREASE_DISABLE;// �ڴ��ַ�������仯
    dma_tx_init_struct.memory_addr = (uint32)(&txByte);
    dma_tx_init_struct.number = count;
    dma_init(DMA0, DMA_CH2, &dma_tx_init_struct);

    // ʹ��SPI��ʹ��˫��ģʽ��������͵�ַ
    spi_enable(STM32_SPIx_NUM_DEF);

    // Ƭѡ
    SPI_FLASH_CS_L();
    // ���Ͷ�ȡ����
    SPI_FLASH_SendByte(CMD_READ_BYTE);
    // ����24λ��ʼ��ַ
    SPI_FLASH_SendByte((srcAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((srcAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(srcAddr & 0xFF);
    // ����ǰ��һ������DR����֤���ջ�����Ϊ��
    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);
    // �ٴν�ֹSPI���޸�����ȫ˫��ģʽ
    spi_disable(STM32_SPIx_NUM_DEF);

    // ����Ϊȫ˫��16λģʽ
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size = SPI_FRAMESIZE_16BIT;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // ��DMA���
    dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_G);
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_G);

    // ʹ��RX��TC�ж�
    dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF);

    // ����ʹ��SPI��SCKʱ�����̿�ʼ�������ʼ���ͽ�������
    spi_enable(STM32_SPIx_NUM_DEF);
    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);

    // ����DMAͨ��DMA1_Channel2��DMA1_Channel3
    dma_channel_enable(DMA0, DMA_CH1);
    dma_channel_enable(DMA1, DMA_CH2);

    while (!spiCB.dmaFinished);

    // DMA�����ģʽ���¸�λ
    SPIx_ConfigureWorkMode(SPI_TRANSMODE_FULLDUPLEX, SPI_FRAMESIZE_8BIT);

#else

    // ��μ��
    if (0 == count)
    {
        return ;
    }

    // ����DMA��ɱ�ʶ
    spiCB.dmaFinished = FALSE;

    // �Ƚ�ֹSPI���޸�����
    spi_disable(STM32_SPIx_NUM_DEF);

    // �ر�DMA
    dma_channel_disable(DMA0, DMA_CH1);

    // ������ȫ˫��ģʽ������DMA֮ǰд����͵�ַ
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // �������ý��յ�ַ�Լ���������
    dma_rx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE; // �ڴ��ַ�����仯
    dma_rx_init_struct.memory_addr = (uint32)dstAddr;
    dma_rx_init_struct.number = count;
    dma_init(DMA0, DMA_CH1, &dma_rx_init_struct);

    // ʹ��SPI��ʹ��˫��ģʽ��������͵�ַ
    spi_enable(STM32_SPIx_NUM_DEF);
    // Ƭѡ
    SPI_FLASH_CS_L();
    // ���Ͷ�ȡ����
    SPI_FLASH_SendByte(CMD_READ_BYTE);
    // ����24λ��ʼ��ַ
    SPI_FLASH_SendByte((srcAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((srcAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(srcAddr & 0xFF);
    // ����ǰ��һ������DR����֤���ջ�����Ϊ��
    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);
    // �ٴν�ֹSPI���޸�����Ϊֻ����ģʽ
    spi_disable(STM32_SPIx_NUM_DEF);
    // ����Ϊֻ����16λģʽ
    spi_init_struct.trans_mode = SPI_TRANSMODE_RECEIVEONLY;
    spi_init_struct.frame_size = SPI_FRAMESIZE_16BIT;
    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);

    // ��DMA���
    dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_G);// ��RXȫ���жϱ�־
    // ʹ��RX��TC�ж�
    dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF);

    // ����ʹ��SPI��SCKʱ�����̿�ʼ�������ʼ��������
    spi_enable(STM32_SPIx_NUM_DEF);
    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);

    // ����DMAͨ��DMA0_Channel1
    dma_channel_enable(DMA0, DMA_CH1);

    while (!spiCB.dmaFinished);

    // DMA�����ģʽ���¸�λ
    SPIx_ConfigureWorkMode(SPI_TRANSMODE_FULLDUPLEX, SPI_FRAMESIZE_8BIT);
#endif
}

// ����SPI����ģʽ
void SPIx_ConfigureWorkMode(uint16 SpiDirection, uint16 SpiDataSize)
{
    spi_init_struct.trans_mode = SpiDirection;
    spi_init_struct.frame_size = SpiDataSize;

    spi_init(STM32_SPIx_NUM_DEF, &spi_init_struct);
    spi_enable(STM32_SPIx_NUM_DEF);

    spi_i2s_data_receive(STM32_SPIx_NUM_DEF);
}


