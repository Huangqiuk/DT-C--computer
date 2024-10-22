#include "main.h"

/*********************************************************
* ����������
*********************************************************/
IAP_CB iapCB;
iapfun jump2app;

// FLASH��д����
BOOL IAP_FlashWriteWordArray(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite);     //��ָ����ַ��ʼд��ָ�����ȵ�����
void IAP_FlashReadWordArray(uint32 readAddr, uint32 *pBuffer, uint16 numToRead);        //��ָ����ַ��ʼ����ָ�����ȵ�����
BOOL IAP_FlashWriteWordArrayAndCheck(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite);
//uint32 IAP_FlashReadWord(uint32 flashAddr);

// APP������
void IAP_RunAPP(uint32 appAddr);

/*******************************************************************************
  * @��������   IAP_RunAPP
  * @����˵��   ��ת��Ӧ�ó����
  * @�������   appAddr:Ӧ�ó������ʼ��ַ
  * @���ز���   ��
*******************************************************************************/
void IAP_RunAPP(uint32 appAddr)
{
    //����һ���������͵Ĳ���
    typedef void (*IAP_Func)(void);
    IAP_Func jumpToApp;

    if (((*(vu32 *)appAddr) & 0x2FFE0000) == 0x20000000) // ���ջ����ַ�Ƿ�Ϸ�.
    {
        if (((*(vu32 *)(appAddr + 4)) & 0xFFF80000) != 0x08000000) // APP��Ч
        {
            return;
        }

        // ��ת֮ǰ�����жϣ���λ����ΪĬ��ֵ
        DI();
//      RCC_EnableAPB2PeriphReset(0xFFFFFFFF, ENABLE);
//      RCC_EnableAPB2PeriphReset(0xFFFFFFFF, DISABLE);
//      RCC_EnableAPB1PeriphReset(0xFFFFFFFF, ENABLE);
//      RCC_EnableAPB1PeriphReset(0xFFFFFFFF, DISABLE);

        jumpToApp = (IAP_Func) * (vu32 *)(appAddr + 4); // �û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
        __set_MSP(*(vu32 *)appAddr);                    // ��ʼ���û�����Ķ�ջָ��
        jumpToApp();                                    // ��ת���û�����APP
    }
}

// ����ʱ�䵽��ת��APP����������
void IAP_JumpToAppFun(uint32 appxaddr)
{
    if(((*(vu32*)appxaddr)&0x0FFFFFFF) < 1024*8)		// ���ջ����ַ�Ƿ�Ϸ�.
    { 
            jump2app = (iapfun)*(vu32*)(appxaddr+4);		
        
            __set_MSP(*(vu32*)appxaddr);						    // ��ʼ����ջָ��
        
            jump2app();										            	// ��ת��APP.
    }
}

uint8 IAP_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len)
{
    uint16 i = 0;

    while (i < len)
    {
        if (*(str1 + i) != *(str2 + i))
        {
            return 0;
        }
        i++;
    }
    return 1;
}

/*******************************************************************************
  * @��������   IAP_Init
  * @����˵��
  * @�������
  * @���ز���   ��
*******************************************************************************/
void IAP_Init(void)
{   
    // �������ȡ������־��������ھ�������ECU�������������������������ھ���APP
    if(1)
    {
    }
    else 
    {
        // ������ʱ����100ms IAP���д�����ʱ��
        TIMER_AddTask(TIMER_ID_IAP_RUN_WINDOW_TIME,
                        100,
                        IAP_JumpToAppFun,
                        IAP_FLASH_APP_ADDR,
                        1,
                        ACTION_MODE_ADD_TO_QUEUE);    
    }
}

/*******************************************************************************
  * @��������   IAP_WriteAppBin
  * @����˵��   �����յ���bin�ļ�����д��ָ����ַ��FLASH����
  * @�������   appAddr:Ӧ�ó������ʼ��ַ appBuf��Ӧ�ó��򻺳����� appSize:Ӧ�ó������ݴ�С����λ���ֽڣ�
  * @���ز���   ��
*******************************************************************************/
BOOL IAP_WriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appSize)
{
    uint16 i = 0;
    uint16 j = 0;
    uint32 curAppAddr = appAddr;    // ��ǰд��ĵ�ַ
    uint32 *iapBuf = NULL;
    BOOL writeStatus;

    // �����ж�
    if (NULL == appBuf)
    {
        return FALSE;
    }

    // ���������Ƿ�Ƿ�����ַ������IAP��������ַ�����Բ����Լ����Լ�����
    if ((appAddr < IAP_FLASH_APP_ADDR) || ((appAddr + appSize) > (IAP_N32_FLASH_BASE + IAP_N32_FLASH_SIZE)))
    {
        return FALSE;       // �Ƿ���ַ
    }

    // ������ʱ�ڴ�
    iapBuf = (uint32 *)malloc(512);
    if (NULL == iapBuf)
    {
        return FALSE;
    }

    // ѭ��д��
    for (i = 0, j = 0; j < appSize; j += 4)
    {
        // �ֽ�����
        iapBuf[i++] = (uint32)(*appBuf) + ((uint32)(*(appBuf + 1)) << 8) + ((uint32)(*(appBuf + 2)) << 16) + ((uint32)(*(appBuf + 3)) << 24);
        // ƫ��4���ֽ�
        appBuf += 4;
    }

    // ֱ��д��FLASH
    writeStatus = IAP_FlashWriteWordArray(curAppAddr, iapBuf, appSize / 4);

    // �ͷ��ڴ�
    free(iapBuf);

    return writeStatus;
}

// ����APP����ҳ
BOOL IAP_EraseAPPArea(void)
{
    uint16 i;
    FLASH_STS status = FLASH_EOP;
    BOOL eraseStatus = TRUE;

    //DI();             // �ر����ж�
    FLASH_Unlock();

    // ������й����־λ
    FLASH_Flag_Status_Clear(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

    // ����APP����ҳ  i = 0x3400 / 512 = 26 ; i < ( 56 * 512 ) / 512 = 56 ; i++
    for (i = (IAP_FLASH_ADRESS_OFFSET / IAP_FLASH_PAGE_SIZE); i < (IAP_N32_FLASH_SIZE / IAP_FLASH_PAGE_SIZE); i++)
    {
        //  
        status = FLASH_One_Page_Erase(i * IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE);

        if (FLASH_EOP != status)
        {
            eraseStatus = FALSE;

            break;
        }
    }

    FLASH_Lock();       // ����
    //EI();                 // �������ж�

    return eraseStatus;
}

/*******************************************************************************
  * @��������   IAP_FlashReadHalfWord
  * @����˵��   ��ȡָ����ַ�İ���(16λ����)
  * @�������   faddr:FLASH��ǰ��ַ
  * @���ز���   �ӵ�ǰFLASH��ַ��ʼ������ȡ16���ֽڣ����֣������ݣ�*(vu16*)faddr��ע���ַ����Ϊ4�ı���
*******************************************************************************/
uint32 IAP_FlashReadWord(uint32 flashAddr)
{
		return *(__IO uint32_t*)flashAddr; 
}

/*******************************************************************************
  * @��������   IAP_FlashWriteHalfWordArrayAndCheck
  * @����˵��   ��FLASH��д��ָ�����ȵ����ݣ�����ʹ�ÿ⺯��IAP_WordProgram��ÿ��д���֣����Ҽ�����д������У��
  * @�������   writeAddr:Ӧ�ó������ʼ��ַ pBuffer���������������� numToWrite:����λ���֣�
  * @���ز���   BOOL,���ΪTRUE,��ò���д���У��ɹ�������ʧ��
----ע��:�˺������ܵ�������ʹ�ã���Ϊû�н�������-------------------------------
*******************************************************************************/
BOOL IAP_FlashWriteWordArrayAndCheck(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite)
{
    uint16 i;
    uint32 readWord;

    // �����ж�
    if (NULL == pBuffer)
    {
        return FALSE;
    }

    // ���������Ƿ�Ƿ�
    if ((writeAddr < IAP_FLASH_APP_ADDR) || ((writeAddr + numToWrite) > (IAP_N32_FLASH_BASE + IAP_N32_FLASH_SIZE + IAP_PARAM_FLASH_SIZE)))
    {
        return FALSE;   // �Ƿ���ַ
    }

    for (i = 0; i < numToWrite; i++)
    {
        FLASH_Word_Program(writeAddr, pBuffer[i]);          // д
        readWord = IAP_FlashReadWord(writeAddr);            // ��
        writeAddr += 4;

        // д��FLASH�еĻ������������ȡ�������Ƿ�ƥ��
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
  * @��������   IAP_FlashWriteWordArray
  * @����˵��   ��ָ����С�Ļ��������ݰ�ҳд��FLASH����
  * @�������   writeAddr:д���ַ pBuffer:�����������׵�ַ numToWrite:��Ҫд��ĳ��ȣ���λ���֣�
  * @���ز���   BOOL�͵ı���writeSucceed
*******************************************************************************/
BOOL IAP_FlashWriteWordArray(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite)
{
    BOOL writeStatus = TRUE;

    // �����ж�
    if (NULL == pBuffer)
    {
        return FALSE;
    }

    // ���������Ƿ�Ƿ�
    if ((writeAddr < IAP_FLASH_APP_ADDR) || ((writeAddr + numToWrite) > (IAP_N32_FLASH_BASE + IAP_N32_FLASH_SIZE)))
    {
        return FALSE;   // �Ƿ���ַ
    }

    DI();             // �ر����ж�
    FLASH_Unlock();     // ����

    // ������й����־λ
    FLASH_Flag_Status_Clear(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

    // д��ҳ
    if (!IAP_FlashWriteWordArrayAndCheck(writeAddr, pBuffer, numToWrite))
    {
        writeStatus = FALSE;
    }

    FLASH_Lock();       // ����
    EI();                 // �������ж�

    return writeStatus;
}

/*******************************************************************************
  * @��������   IAP_FlashReadHalfWordArray
  * @����˵��   ��ָ����ַ��ʼ����ָ�����ȵ�����
  * @�������   readAddr:��ȡ��ַ pBuffer:�����������׵�ַ numToWrite:��Ҫ����ĳ��ȣ���λ�����֣�
  * @���ز���   ��
*******************************************************************************/
void IAP_FlashReadWordArray(uint32 readAddr, uint32 *pBuffer, uint16 numToRead)
{
    uint16 i;
    uint8 tmp;
    uint8 tmpBuf[4];
    uint32 *buf = NULL;

    // �����ж�
    if (NULL == pBuffer)
    {
        return;
    }

    // ���������Ƿ�Ƿ�
    if ((readAddr < IAP_FLASH_APP_ADDR) || ((readAddr + numToRead) > (IAP_N32_FLASH_BASE + IAP_N32_FLASH_SIZE + IAP_PARAM_FLASH_SIZE)))
    {
        return; // �Ƿ���ַ
    }

    // �ж��Ƿ�4�ֽڶ��룬�������4�ֽڶ��룬�����ƫ�ƴ���
    if (readAddr % 4 != 0)
    {
        tmp = readAddr % 4;
        readAddr -= tmp;

        // ��ȡԭʼ����
        for (i = 0; i < numToRead + 1; i++)
        {
            *(buf + i) = IAP_FlashReadWord(readAddr);
            readAddr += 4;
        }
        // �������
        for (i = 0; i < numToRead; i++)
        {
            if (tmp == 1)
            {
                tmpBuf[0] = (uint8)(buf[i] >> 8);
                tmpBuf[1] = (uint8)(buf[i] >> 16);
                tmpBuf[2] = (uint8)(buf[i] >> 24);
                tmpBuf[3] = (uint8)buf[i + 1];
            }
            else if (tmp == 2)
            {
                tmpBuf[0] = (uint8)(buf[i] >> 16);
                tmpBuf[1] = (uint8)(buf[i] >> 24);
                tmpBuf[2] = (uint8)buf[i + 1];
                tmpBuf[3] = (uint8)(buf[i + 1] >> 8);
            }
            else if (tmp == 3)
            {
                tmpBuf[0] = (uint8)(buf[i] >> 24);
                tmpBuf[1] = (uint8)(buf[i + 1] >> 8);
                tmpBuf[2] = (uint8)(buf[i + 1] >> 16);
                tmpBuf[3] = (uint8)(buf[i + 1] >> 24);
            }

            pBuffer[i] = (uint32)tmpBuf[0] + (uint32)(tmpBuf[1] << 8) \
                         + (uint32)(tmpBuf[2] << 16) + (uint32)(tmpBuf[3] << 24);
        }
    }
    else
    {
        for (i = 0; i < numToRead; i++)
        {
            pBuffer[i] = IAP_FlashReadWord(readAddr);   // ��ȡ4���ֽ�.
            readAddr += 4;                              // ƫ��4���ֽ�.
        }
    }
}

/*******************************************************************************
  * @��������   IAP_FlashReadHalfByteArray
  * @����˵��   ��ָ����ַ��ʼ����ָ�����ȵ�����
  * @�������   readAddr:��ȡ��ַ pBuffer:�����������׵�ַ numToWrite:��Ҫ����ĳ��ȣ���λ�����֣�
  * @���ز���   ��
*******************************************************************************/
void IAP_FlashReadByteArray(uint32 readAddr, uint8 *pBuffer, uint16 numToRead)
{

    uint16 i;
    uint8 offsetTmp;
    uint32 readBuf = 0;
    uint8 *tmpBuf = NULL;

    // �����ж�
    if (NULL == pBuffer)
    {
        return;
    }

    // ���������Ƿ�Ƿ�
    if ((readAddr < IAP_FLASH_APP_ADDR) || ((readAddr + numToRead * 4) > (IAP_N32_FLASH_BASE + IAP_N32_FLASH_SIZE + IAP_PARAM_FLASH_SIZE)))
    {
        return; // �Ƿ���ַ
    }

    tmpBuf = (uint8 *)malloc((numToRead / 4 + 2) * 4);

    // �ж��Ƿ�4�ֽڶ��룬�������4�ֽڶ��룬�����ƫ�ƴ���
    if (readAddr % 4 != 0)
    {
        offsetTmp = readAddr % 4;
        readAddr -= offsetTmp;

        // ��ȡԭʼ����
        for (i = 0; i < (numToRead - 1) / 4 + 2; i++)
        {
            readBuf = IAP_FlashReadWord(readAddr);

            tmpBuf[4 * i] = (uint8)readBuf;
            tmpBuf[4 * i + 1] = (uint8)(readBuf >> 8);
            tmpBuf[4 * i + 2] = (uint8)(readBuf >> 16);
            tmpBuf[4 * i + 3] = (uint8)(readBuf >> 24);

            readAddr += 4;
        }

        for (i = 0; i < numToRead ; i++)
        {
            pBuffer[i] = tmpBuf[i + offsetTmp];
        }

    }
    else
    {
        for (i = 0; i < (numToRead - 1) / 4 + 1; i++)
        {
            readBuf = IAP_FlashReadWord(readAddr);      // ��ȡ4���ֽ�.

            tmpBuf[4 * i] = (uint8)readBuf;
            tmpBuf[4 * i + 1] = (uint8)(readBuf >> 8);
            tmpBuf[4 * i + 2] = (uint8)(readBuf >> 16);
            tmpBuf[4 * i + 3] = (uint8)(readBuf >> 24);

            readAddr += 4;                              // ƫ��4���ֽ�.
        }

        for (i = 0; i < numToRead ; i++)
        {
            pBuffer[i] = tmpBuf[i];
        }
    }

    free(tmpBuf);
}

/*******************************************************************************
  * @��������   IAP_FlashWriteWordArrayWithErase
  * @����˵��   ��ָ����С�Ļ��������ݰ�ҳд��FLASH����
  * @�������   writeAddr:д���ַ pBuffer:�����������׵�ַ numToWrite:��Ҫд��ĳ��ȣ���λ���֣�
  * @���ز���   BOOL�͵ı���writeSucceed
*******************************************************************************/
BOOL IAP_FlashWriteWordArrayWithErase(uint32 writeAddr, uint32 *pBuffer, uint16 NumWordToWrite)
{
    BOOL writeStatus = TRUE;
    uint32 pagePos = 0;                     // ������ַ
    uint16 pageOffsent = 0;                 // �������ڵ�ƫ��
    uint16 pageRemain = 0;                  // ����ʣ��ռ��С
    uint32 offaddr;                         // ȥ��0X08000000��ĵ�ַ
    uint16 i = 0;
    uint32 *pFlashBuff = NULL;              // �����ȡ����������
    uint16 numByteToWrite = NumWordToWrite * 4;

    // �����ж�
    if (NULL == pBuffer || (NULL == numByteToWrite))
    {
        return FALSE;
    }

    // ���������Ƿ�Ƿ�
    if ((writeAddr < IAP_FLASH_APP_ADDR) || ((writeAddr + numByteToWrite) > (IAP_N32_FLASH_BASE + IAP_N32_FLASH_SIZE + IAP_PARAM_FLASH_SIZE)))
    {
        return FALSE;   // �Ƿ���ַ
    }

    DI();               // �ر����ж�
    FLASH_Unlock();     // ����

    // ����1024Byte�ռ䱣��ҳ����
    pFlashBuff = (uint32 *)malloc(IAP_FLASH_PAGE_SIZE);
    if (NULL == pFlashBuff)
    {
        return FALSE;
    }

    offaddr = writeAddr - IAP_N32_FLASH_BASE;       // ʵ��ƫ�Ƶ�ַ
    pagePos = offaddr / IAP_FLASH_PAGE_SIZE;        // ҳ��ַ
    pageOffsent = offaddr % IAP_FLASH_PAGE_SIZE;    // ��ҳ��ƫ��
    pageRemain = IAP_FLASH_PAGE_SIZE - pageOffsent; // ҳʣ��ռ��С

    if (numByteToWrite <= pageRemain)
    {
        pageRemain = numByteToWrite;                // �����ڸ�ҳ��Χ
    }

    while (1)
    {
        // ��������ҳ������
        IAP_FlashReadWordArray(pagePos * IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE, pFlashBuff, IAP_FLASH_PAGE_SIZE / 4);

        for (i = 0; i < pageRemain / 4; i++) // У������
        {
            if (pFlashBuff[pageOffsent / 4 + i] != 0xFFFFFFFF)
            {
                // ��������ݲ�ȫ��0xFFFFFFFF��˵����Ҫ����
                break;
            }
        }

        if (i < pageRemain / 4)                 // ��Ҫ����
        {
            // �������ҳ
            FLASH_One_Page_Erase(pagePos * IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE);

            // ����
            for (i = 0; i < pageRemain / 4; i++)
            {
                pFlashBuff[i + pageOffsent / 4] = pBuffer[i];
            }

            // д����ҳ
            if (!IAP_FlashWriteWordArrayAndCheck(pagePos * IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE, pFlashBuff, IAP_FLASH_PAGE_SIZE / 4))
            {
                // �ͷ����ݻ�����
                free(pFlashBuff);

                writeStatus = FALSE;

                break;
                //return writeStatus;
            }
        }
        else
        {
            // д�Ѿ������˵�,ֱ��д������ʣ������.
            if (!IAP_FlashWriteWordArrayAndCheck(writeAddr, pBuffer, pageRemain / 4))
            {
                // �ͷ����ݻ�����
                free(pFlashBuff);

                writeStatus = FALSE;

                break;
                //return writeStatus;
            }
        }

        if (numByteToWrite == pageRemain)
        {
            break;                                  // д�������
        }
        else                                        // д��δ����
        {
            pagePos++;                              // ������ַ��1
            pageOffsent = 0;                        // ƫ��λ��Ϊ0

            pBuffer += (pageRemain / 4);            // ָ��ƫ��
            writeAddr += pageRemain;                // д��ַƫ��
            numByteToWrite -= pageRemain;           // �ֽ����ݼ�

            if (numByteToWrite > IAP_FLASH_PAGE_SIZE)
            {
                pageRemain = IAP_FLASH_PAGE_SIZE;   // ��һ����������д����
            }
            else
            {
                pageRemain = numByteToWrite;        // ��һ����������д����
            }
        }

    }

    // �ͷ����ݻ�����
    free(pFlashBuff);

    FLASH_Lock();       // ����
    EI();               // �������ж�

    return writeStatus;
}

/*********************************************************************
* ��   ��  ��: SPI_FLASH_WriteByteWithErase
* �������: uint8* pBuffer: ���ݴ洢��
            uint32 WriteAddr: ��ʼд��ĵ�ַ(24bit)
            uint16 NumByteToWrite: Ҫд����ֽ���(���65535)
* ��������: ��
* ��   ��  ֵ: void
* ��������: ��ָ����ַ��ʼд��ָ�����ȵ����ݣ��ú�������������!
**********************************************************************/
void IAP_FLASH_WriteByteWithErase(uint32 writeAddr, uint8 *pBuffer, uint16 NumByteToWrite)
{
    BOOL writeStatus = TRUE;
    uint32 pagePos = 0;                     // ������ַ
    uint16 pageOffsent = 0;                 // �������ڵ�ƫ��
    uint16 pageRemain = 0;                  // ����ʣ��ռ��С
    uint32 offaddr;                         // ȥ��0X08000000��ĵ�ַ
    uint16 i = 0;
    uint8 *pFlashBuff = NULL;               // �����ȡ����������
    uint16 numByteToWrite = NumByteToWrite;

    if ((NULL == pBuffer) || (0 == NumByteToWrite))
    {
        return;
    }

    DI();               // �ر����ж�
    FLASH_Unlock();     // ����

    // ����1024Byte�ռ䱣����������
    pFlashBuff = (uint8 *)malloc(IAP_FLASH_PAGE_SIZE);
    if (NULL == pFlashBuff)
    {
        return;
    }

    offaddr = writeAddr - IAP_N32_FLASH_BASE;       // ʵ��ƫ�Ƶ�ַ
    pagePos = offaddr / IAP_FLASH_PAGE_SIZE;        // ҳ��ַ
    pageOffsent = offaddr % IAP_FLASH_PAGE_SIZE;    // ��ҳ��ƫ��
    pageRemain = IAP_FLASH_PAGE_SIZE - pageOffsent; // ҳʣ��ռ��С

    if (NumByteToWrite <= pageRemain)
    {
        pageRemain = NumByteToWrite;                // ������1024���ֽ�
    }

    while (1)
    {
        // ������������������
        IAP_FlashReadByteArray(pagePos * IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE, pFlashBuff, IAP_FLASH_PAGE_SIZE);

        // �������ҳ
        FLASH_One_Page_Erase(pagePos * IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE);


        // ����
        for (i = 0; i < pageRemain; i++)
        {
            pFlashBuff[i + pageOffsent] = pBuffer[i];
        }

        // д����ҳ
        if (!IAP_FlashWriteWordArrayAndCheck(pagePos * IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE, (uint32 *)pFlashBuff, IAP_FLASH_PAGE_SIZE / 4))
        {
            // �ͷ����ݻ�����
            free(pFlashBuff);

            //writeStatus = FALSE;

            break;
            //return writeStatus;
        }


        if (NumByteToWrite == pageRemain)
        {
            break;                                  // д�������
        }
        else                                        // д��δ����
        {
            pagePos++;                              // ������ַ��1
            pageOffsent = 0;                        // ƫ��λ��Ϊ0

            pBuffer += pageRemain;              // ָ��ƫ��
            writeAddr += pageRemain;                // д��ַƫ��
            numByteToWrite -= pageRemain;           // �ֽ����ݼ�

            if (numByteToWrite > IAP_FLASH_PAGE_SIZE)
            {
                pageRemain = IAP_FLASH_PAGE_SIZE;   // ��һ����������д����
            }
            else
            {
                pageRemain = numByteToWrite;        // ��һ����������д����
            }
        }
    }

    // �ͷ����ݻ�����
    free(pFlashBuff);

    FLASH_Lock();       // ����
//  EI();               // �������ж�
}


int32_t app_flag_write(uint32_t data ,uint32_t start_add)
{
		FLASH_Unlock();

		FLASH_One_Page_Erase(start_add);			//д֮ǰ�Ȳ�һ�飬ÿ�β�512Byte
	
		if (FLASH_EOP != FLASH_Word_Program(start_add, data))		//д
		{
				FLASH_Lock();
				return 1;
		}
		FLASH_Lock();
		
		return 0;
}


