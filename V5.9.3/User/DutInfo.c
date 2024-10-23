#include "DutInfo.h"
#include "Spiflash.h"
#include "string.h"
#include "common.h"

// ����ȫ�ֱ���
DUT_INFO dut_info;

// ��ʼ��������Ϣ����flash�ж�ȡ�������ݻ������ú�id
void DutInfoUpdata()
{
    uint8 configInfo[40] = {0};
    uint8 bootSize[4] = {0};
    uint8 appSize[4] = {0};
    uint8 uiSize[4] = {0};
    uint8 i = 0;

    char names[10] = {0};
    char names2[10] = {0};
    char names3[10] = {0};
    char names4[10] = {0};
    char names5[10] = {0};
    char names6[13] = {0};
    char names7[10] = {0};
    
    SPI_FLASH_ReadArray(configInfo, SPI_FLASH_DUT_INFO, 40);           // ��������Ϣ
    SPI_FLASH_ReadArray(bootSize, SPI_FLASH_DUT_BOOT_SIZE_ADDEESS, 4); // ��dut_boot��С
    SPI_FLASH_ReadArray(appSize, SPI_FLASH_DUT_APP_SIZE_ADDEESS, 4);   // ��dut_app��С
    SPI_FLASH_ReadArray(uiSize, SPI_FLASH_DUT_UI_SIZE_ADDEESS, 4);     // ��dut_ui��С

    // ��ȡ�ͺų���
    dut_info.nameLen = configInfo[20];

    // ��ȡ��ѹ
    dut_info.voltage = configInfo[21];

    // ��ȡ��dut��ͨѶ����
    dut_info.dutBusType = configInfo[22];

    // ��ȡ�ļ�����
    //  dut_info.bootType = configInfo[23]; // bootType
    //  dut_info.appType = configInfo[24];  // appType
    //  dut_info.uiType = configInfo[25];       // uiType
    //  dut_info.configType = configInfo[26]; // configType

    dut_info.bootSize = (bootSize[0] << 24) | (bootSize[1] << 16) | (bootSize[2] << 8) | (bootSize[3]);
    dut_info.appSize = (appSize[0] << 24) | (appSize[1] << 16) | (appSize[2] << 8) | (appSize[3]);
    dut_info.uiSize = (uiSize[0] << 24) | (uiSize[1] << 16) | (uiSize[2] << 8) | (uiSize[3]);

    // ����Ϊ��
    if (0xFF == configInfo[20])
    {
        dut_info.nameLen = 0;
    }
    // ��СΪ��
    if (0xFF == dut_info.bootSize)
    {
        dut_info.bootSize = 0;
    }

    if (0xFF == dut_info.appSize)
    {
        dut_info.appSize = 0;
    }

    if (0xFF == dut_info.uiSize)
    {
        dut_info.uiSize = 0;
    }

    for (i = 0; i < dut_info.nameLen; i++)
    {
        dut_info.name[i] = configInfo[i];
    }

    // ��ʼ������
    if (dut_info.name[0] == 'L')
    {
        snprintf(names, 5, "%s", dut_info.name);
        snprintf(names2, 6, "%s", dut_info.name);
        if (0 == strcmp(names, "L400"))
        {
            dut_info.ID = DUT_TYPE_SEG;
            dut_info.nam = DUT_NAME_NULL;
        }
        else if (0 == strcmp(names, "L401"))
        {
            dut_info.ID = DUT_TYPE_SEG;
            dut_info.nam = DUT_NAME_NULL;
        }
        else if ((0 == strcmp(names, "L182")) || (0 == strcmp(names, "L200")) || (0 == strcmp(names, "L300")) || (0 == strcmp(names, "L301")) || (0 == strcmp(names, "L401")) || (0 == strcmp(names, "L402")))
        {
            dut_info.ID = DUT_TYPE_SEG;
            dut_info.nam = DUT_NAME_NULL;
        }
        else if (0 == strcmp(names2, "LE18D"))
        {
            dut_info.ID = DUT_TYPE_LIME;
            dut_info.nam = DUT_NAME_NULL;
        }
        else if (0 == strcmp(names2, "LE18U"))
        {
            dut_info.ID = DUT_TYPE_LIME;
            dut_info.nam = DUT_NAME_NULL;
        }
    }
    else
    {
        snprintf(names, 3, "%s", &dut_info.name[6]);
        if (0 == strcmp(names, "GB"))
        {
            dut_info.ID = DUT_TYPE_GB;
            dut_info.nam = DUT_NAME_NULL;
        }
        else
        {
            snprintf(names7, 7, "%s", dut_info.name);          
            snprintf(names6, 12, "%s", dut_info.name);        
            snprintf(names5, 8, "%s", dut_info.name);
            snprintf(names4, 5, "%s", dut_info.name);
            snprintf(names3, 3, "%s", dut_info.name);
            snprintf(names2, 4, "%s", dut_info.name);
            snprintf(names, 6, "%s", dut_info.name);
            if (0 == strcmp(names, "BC201"))
            {
                dut_info.ID = DUT_TYPE_CM;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BN610"))
            {
                dut_info.ID = DUT_TYPE_CM;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BC280"))
            {
                dut_info.ID = DUT_TYPE_CM;
                dut_info.nam = DUT_NAME_BC280;
            }
            else if (0 == strcmp(names, "BC110"))
            {
                dut_info.ID = DUT_TYPE_CM;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BC281"))
            {
                if(0 == strcmp(names6, "BC281EVL-S1"))
                {
                    dut_info.ID = DUT_TYPE_CM;
                    dut_info.nam = DUT_NAME_BC281EVL;                
                }
                else
                {
                    dut_info.ID = DUT_TYPE_CM;
                    dut_info.nam = DUT_NAME_BC281;               
                }
            }
            else if (0 == strcmp(names, "BC182"))
            {
                dut_info.ID = DUT_TYPE_CM;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BC300"))
            {
                dut_info.ID = DUT_TYPE_HUAXIN;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BC302"))
            {
                dut_info.ID = DUT_TYPE_HUAXIN;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BC351"))
            {
                dut_info.ID = DUT_TYPE_CM;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BC352"))
            {
                dut_info.ID = DUT_TYPE_HUAXIN;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BC240"))
            {
                dut_info.ID = DUT_TYPE_HUAXIN;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BC147"))
            {
                dut_info.ID = DUT_TYPE_CM;
                dut_info.nam = DUT_NAME_BC147;
            }
            else if (0 == strcmp(names, "BC550"))
            {
                dut_info.ID = DUT_TYPE_KAIYANG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BN135"))
            {
                dut_info.ID = DUT_TYPE_SEG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BN132"))
            {
                dut_info.ID = DUT_TYPE_SEG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "BN136"))
            {
                dut_info.ID = DUT_TYPE_SEG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "EH001"))
            {
                dut_info.ID = DUT_TYPE_SEG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "Citra"))
            {
                dut_info.ID = DUT_TYPE_LIME;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names2, "3KM"))
            {
                dut_info.ID = DUT_TYPE_SEG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names3, "A5"))
            {
                dut_info.ID = DUT_TYPE_SEG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names3, "A8"))
            {
                dut_info.ID = DUT_TYPE_SEG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names3, "A6"))
            {
                dut_info.ID = DUT_TYPE_SEG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names4, "A8CC"))
            {
                dut_info.ID = DUT_TYPE_SEG;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names5, "Sparrow"))
            {
                dut_info.ID = DUT_TYPE_SPARROW;
                dut_info.nam = DUT_NAME_NULL;
            }
            else if (0 == strcmp(names, "CS505"))
            {
                dut_info.ID = DUT_TYPE_MEIDI;
                dut_info.nam = DUT_NAME_NULL;
            } 
            else if (0 == strcmp(names7, "IOT550"))
            {
                dut_info.ID = DUT_TYPE_IOT;
                dut_info.nam = DUT_NAME_NULL;
            }              
            else
            {
                dut_info.ID = DUT_TYPE_NULL;
                dut_info.nam = DUT_NAME_NULL;
            }

        }
    }


}