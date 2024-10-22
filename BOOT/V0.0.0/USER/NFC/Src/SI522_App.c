
//#include "systick.h"
#include "main.h"
//***********************************//修改新增内容
extern uint8_t Si522_IRQ_flagA ;

//NFC DEBUG
#define NFC_DEBUG(frm,...)	//printf(frm,##__VA_ARGS__)


/***********************************************
 * 函数名：PcdAntennaOn
 * 描述  ：开启天线  每次启动或关闭天险发射之间应至少有1ms的间隔
 * 输入  ：无
 * 返回  : 无
 * 调用  ：外部调用
 **********************************************/

void PcdAntennaOn(void)
{
    unsigned char i;
    i = I_SI522_IO_Read(TxControlReg);
    if (!(i & 0x03))
    {
        I_SI522_SetBitMask(TxControlReg, 0x03); // bit1 bit0 置1
    }
}


/***********************************************
 * 函数名：PcdAntennaOff
 * 描述  ：关闭天线
 * 输入  ：无
 * 返回  : 无
 * 调用  ：外部调用
 **********************************************/
void PcdAntennaOff(void)
{
	I_SI522_ClearBitMask(TxControlReg, 0x03);  // bit1 bit0 清0
}

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////

/***********************************************
 * 函数名：CalulateCRC
 * 描述  ：用MF522计算CRC16函数
 * 输入  ：
 * 返回  : 无
 * 调用  ：外部调用
 **********************************************/
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    I_SI522_ClearBitMask(DivIrqReg,0x04);
    I_SI522_IO_Write(CommandReg,PCD_IDLE);
    I_SI522_SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   
			I_SI522_IO_Write(FIFODataReg, *(pIndata+i));   
		}
    I_SI522_IO_Write(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = I_SI522_IO_Read(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = I_SI522_IO_Read(CRCResultRegL);
    pOutData[1] = I_SI522_IO_Read(CRCResultRegH);
}


unsigned char aaa = 0;



/**************************************************************************************************
 * 函数名：PcdComMF522
 * 描述  ：通过RC522和ISO14443卡通讯
 * 输入  ：Command[IN]:RC522命令字              pInData[IN]:通过RC522发送到卡片的数据    InLenByte[IN]:发送数据的字节长度
 *       ：pOutData[OUT]:接收到的卡片返回数据   pOutLenBit[OUT]:返回数据的位长度
 * 返回  : 无 
 * 调用  ：外部调用
 *************************************************************************************************/
char PcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
        case PCD_AUTHENT:
			irqEn   = 0x12;
			waitFor = 0x10;
			break;
		case PCD_TRANSCEIVE:
			irqEn   = 0x77;
			waitFor = 0x30;
			break;
		default:
			break;
    }
   
    //I_SI522_IO_Write(ComIEnReg,irqEn|0x80);
    I_SI522_ClearBitMask(ComIrqReg,0x80);
    I_SI522_IO_Write(CommandReg,PCD_IDLE);
    I_SI522_SetBitMask(FIFOLevelReg,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   
		I_SI522_IO_Write(FIFODataReg, pInData[i]);    
	}
    I_SI522_IO_Write(CommandReg, Command);
   
    if (Command == PCD_TRANSCEIVE)
    {    
		I_SI522_SetBitMask(BitFramingReg,0x80);  
	}
    
    //i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
	i = 2000;
    do 
    {
        n = I_SI522_IO_Read(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    I_SI522_ClearBitMask(BitFramingReg,0x80);

    if (i!=0)
    {   
		aaa = I_SI522_IO_Read(ErrorReg);
		
        if(!(I_SI522_IO_Read(ErrorReg)&0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   status = MI_NOTAGERR;   }
            if (Command == PCD_TRANSCEIVE)
            {
               	n = I_SI522_IO_Read(FIFOLevelReg);
              	lastBits = I_SI522_IO_Read(ControlReg) & 0x07;
                if (lastBits)
                {   
					*pOutLenBit = (n-1)*8 + lastBits;   
				}
                else
                {   
					*pOutLenBit = n*8;   
				}
                if (n == 0)
                {   
					n = 1;    
				}
                if (n > MAXRLEN)
                {   
					n = MAXRLEN;   
				}
                for (i=0; i<n; i++)
                {   
					pOutData[i] = I_SI522_IO_Read(FIFODataReg);    
				}
            }
        }
        else
        {   
			status = MI_ERR;   
		}
        
    }
   
    I_SI522_SetBitMask(ControlReg,0x80);           // stop timer now
    I_SI522_IO_Write(CommandReg,PCD_IDLE); 
    return status;
}
                     
/********************************************************************
 * 函数功能：寻卡
 * 参数说明: req_code[IN]:寻卡方式
 *                0x52 = 寻感应区内所有符合14443A标准的卡
 *                0x26 = 寻未进入休眠状态的卡
 *          pTagType[OUT]：卡片类型代码
 *                0x4400 = Mifare_UltraLight
 *                0x0400 = Mifare_One(S50)
 *                0x0200 = Mifare_One(S70)
 *                0x0800 = Mifare_Pro(X)
 *                0x4403 = Mifare_DESFire
 * 返回值 : 成功返回MI_OK
 ********************************************************************/
char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
	char status;  
	unsigned int unLen;
	unsigned char ucComMF522Buf[MAXRLEN]; 

	I_SI522_ClearBitMask(Status2Reg,0x08);
	I_SI522_IO_Write(BitFramingReg,0x07);
	I_SI522_SetBitMask(TxControlReg,0x03);
 
	ucComMF522Buf[0] = req_code;

	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
	if ((status == MI_OK) && (unLen == 0x10))
	{    
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
	}
	else
	{   
		status = MI_ERR;   
	}
   
	return status;
}


/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////  
char PcdAnticoll(unsigned char *pSnr, unsigned char anticollision_level)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    

    I_SI522_ClearBitMask(Status2Reg,0x08);
    I_SI522_IO_Write(BitFramingReg,0x00);
    I_SI522_ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = anticollision_level;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
	{
		for (i=0; i<4; i++)
		{   
			*(pSnr+i)  = ucComMF522Buf[i];
			snr_check ^= ucComMF522Buf[i];
		}
		if (snr_check != ucComMF522Buf[i])
   		{   
			status = MI_ERR;    
		}
    }
    
    I_SI522_SetBitMask(CollReg,0x80);
    return status;
}


/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
////////////////////////////////////////////////////////////////////
char PcdSelect (unsigned char * pSnr, unsigned char *sak)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);                                                                      
  
    I_SI522_ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   
		*sak = ucComMF522Buf[0];
		status = MI_OK;  
	}
    else
    {   
		status = MI_ERR;    
	}

    return status;
}

char PcdSelect1 (unsigned char * pSnr, unsigned char *sak)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    I_SI522_ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   
		*sak = ucComMF522Buf[0];
		status = MI_OK;  
	}
    else
    {   
		status = MI_ERR;    
	}

    return status;
}

char PcdSelect2 (unsigned char * pSnr, unsigned char *sak)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL2;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    I_SI522_ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   
		*sak = ucComMF522Buf[0];
		status = MI_OK;  
	}
    else
    {   
		status = MI_ERR;    
	}

    return status;
}

char PcdSelect3 (unsigned char * pSnr, unsigned char *sak)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL2;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    I_SI522_ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   
		*sak = ucComMF522Buf[0];
		status = MI_OK;  
	}
    else
    {   
		status = MI_ERR;    
	}

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////               
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
	memcpy(&ucComMF522Buf[2], pKey, 6); 
	memcpy(&ucComMF522Buf[8], pSnr, 6); 
    
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(I_SI522_IO_Read(Status2Reg) & 0x08)))
    {
		status = MI_ERR;   
	}
    
    return status;
}



/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
///////////////////////////////////////////////////////////////////// 
char PcdRead(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
   	{   
		memcpy(pData, ucComMF522Buf, 16);   
	}
    else
    {   
		status = MI_ERR;   
	}
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////                  
char PcdWrite(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   
		status = MI_ERR;   
	}
        
    if (status == MI_OK)
    {
        memcpy(ucComMF522Buf, pData, 16);
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   
			status = MI_ERR;   
		}
    }
    
    return status;
}

/*=================================
 函数功能：循环读取A卡UID

=================================*/
void PCD_SI522_TypeA(void)
{
	//LED_OperaSuccess();	// LED indicator
	
	while(1)
	{
		PCD_SI522_TypeA_GetUID();		//读A卡
		
		//PCD_SI522_TypeA_rw_block();		//读A卡扇区

		delay_ms(500);
	}
}


/*===============================
 函数功能：读A卡初始化配置

 ================================*/
void PCD_SI522_TypeA_Init(void)
{
	I_SI522_IO_Init();	// Initializes the interface with Si522	
	delay_ms(50);
	
	I_SI522_ClearBitMask(Status2Reg, 0x08);  
	// Reset baud rates
	I_SI522_IO_Write(TxModeReg, 0x00);
	I_SI522_IO_Write(RxModeReg, 0x00);
	// Reset ModWidthReg
	I_SI522_IO_Write(ModWidthReg, 0x26);
	// RxGain:110,43dB by default;
	I_SI522_IO_Write(RFCfgReg, RFCfgReg_Val);
	// When communicating with a PICC we need a timeout if something goes wrong.
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	I_SI522_IO_Write(TModeReg, 0x80);// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	I_SI522_IO_Write(TPrescalerReg, 0xa9);// TPreScaler = TModeReg[3..0]:TPrescalerReg
	I_SI522_IO_Write(TReloadRegH, 0x03); // Reload timer 
	I_SI522_IO_Write(TReloadRegL, 0xe8); // Reload timer 
	I_SI522_IO_Write(TxASKReg, 0x40);	// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
	I_SI522_IO_Write(ModeReg, 0x3D);	// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
	I_SI522_IO_Write(CommandReg, 0x00);  // Turn on the analog part of receiver   

	PcdAntennaOn();
}

char PCD_SI522_TypeA_GetUID(void)
{
	unsigned char ATQA[2];
	unsigned char UID[12];
	unsigned char SAK = 0;
	unsigned char UID_complate1 = 0;
	unsigned char UID_complate2 = 0;
  static unsigned char delay=0;
	
	uint8_t j = 0;
	uint8_t i = 0;
	
	
	UID2HMI[0] = 0xFF;
	UID2HMI[1] = 0xFF;
	UID2HMI[2] = 0xFF;
	UID2HMI[3] = 0xFF;
	
	//	printf("\r\nTest_Si522_GetUID");
	I_SI522_IO_Write(RFCfgReg, RFCfgReg_Val); //复位接收增益
		
	//if(++delay>=1)
	{
	    delay=0;	
		//寻卡
		if( PcdRequest( PICC_REQIDL, ATQA) != MI_OK )  //寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
		{
			I_SI522_IO_Write(RFCfgReg, 0x48);
			if(PcdRequest( PICC_REQIDL, ATQA) != MI_OK)
			{
				I_SI522_IO_Write(RFCfgReg, 0x58);
				if(PcdRequest( PICC_REQIDL, ATQA) != MI_OK)
				{//寻卡失败
					NFC_DEBUG("\r\nRequest:fail");
					return 1;
				}
				else
				{//寻卡成功
					NFC_DEBUG("\r\nRequest1:ok  ATQA:%02x %02x",ATQA[0],ATQA[1]);
				}	
			}
			else
			{//寻卡成功
				NFC_DEBUG("\r\nRequest2:ok  ATQA:%02x %02x",ATQA[0],ATQA[1]);
			}		
		}
		else
		{
			NFC_DEBUG("\r\nRequest3:ok  ATQA:%02x %02x",ATQA[0],ATQA[1]);
		}
	
	
		//UID长度=4
		//Anticoll 冲突检测 level1
		if(PcdAnticoll(UID, PICC_ANTICOLL1)!= MI_OK) 
		{
			NFC_DEBUG("\r\nAnticoll1:fail");
			return 1;		
		}
		else
		{
			if(PcdSelect1(UID,&SAK)!= MI_OK)
			{
				NFC_DEBUG("\r\nSelect1:fail");
				return 1;		
			}
			else
			{
				NFC_DEBUG("\r\nSelect1:ok  SAK1:%02x",SAK);
				if(SAK&0x04)                         
				{
					
					
					UID_complate1 = 0;
					
					//UID长度=7
					if(UID_complate1 == 0)    
					{
						//Anticoll 冲突检测 level2
						if(PcdAnticoll(UID+4, PICC_ANTICOLL2)!= MI_OK) 
						{
							NFC_DEBUG("\r\nAnticoll2:fail");
							
							return 1;		
						}
						else
						{
							if(PcdSelect2(UID+4,&SAK)!= MI_OK)  
							{
								NFC_DEBUG("\r\nSelect2:fail");
								return 1;		
							}
							else
							{
								NFC_DEBUG("\r\nSelect2:ok  SAK2:%02x",SAK);
								if(SAK&0x04)                         
								{
									UID_complate2 = 0;
									
									//UID长度=10
									if(UID_complate2 == 0)     
									{
										//Anticoll 冲突检测 level3
										if(PcdAnticoll(UID+8, PICC_ANTICOLL3)!= MI_OK) 
										{
											NFC_DEBUG("\r\nAnticoll3:fail");
											return 1;		
										}
										else
										{
											if(PcdSelect3(UID+8,&SAK)!= MI_OK)  
											{
												NFC_DEBUG("\r\nSelect3:fail");
												return 1;		
											}
											else
											{
												NFC_DEBUG("\r\nSelect3:ok  SAK3:%02x",SAK);
												if(SAK&0x04)                          
												{
	//												UID_complate3 = 0;
												}
												else 
												{
		//											UID_complate3 = 1; 
													SET_NfcVerifySta(VERIFY_PASSED); 
                             						NFC_PowerOn();													
													NFC_DEBUG("\r\nAnticoll3:ok  UID:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
													UID[1],UID[2],UID[3],UID[5],UID[6],UID[7],UID[8],UID[9],UID[10],UID[11]);									
												}					
											}							
										}
									}
								}
								else 
								{
									//=================检测到卡-读取UUID==================
									 UID_complate2 = 1; 
									 SET_NfcVerifySta(VERIFY_PASSED);
									 NFC_PowerOn();
									NFC_DEBUG("\r\nAnticoll2:ok  UID:%02x %02x %02x %02x %02x %02x %02x",
									UID[1],UID[2],UID[3],UID[4],UID[5],UID[6],UID[7]);
								}	
							}			
						}
					}
				}
				else{//-------------------读到了卡------------------------
					NFC_DEBUG("\r\nUID:%02x %02x %02x %02x ",UID[0],UID[1],UID[2],UID[3]);				
					
					UID2HMI[0] = UID[0];
					UID2HMI[1] = UID[1];
					UID2HMI[2] = UID[2];
					UID2HMI[3] = UID[3];
					
				} 
			}		
		}




	
	}
		return 0;
}

char PCD_SI522_TypeA_rw_block(void)
{
	unsigned char ATQA[2];
	unsigned char UID[12];
	unsigned char SAK = 0;
	unsigned char CardReadBuf[16] = {0};
	unsigned char CardWriteBuf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	unsigned char DefaultKeyABuf[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	uint8_t i = 0;
	
//	printf("\r\n\r\nTest_Si522_GetCard");
	
	//request 寻卡
	if( PcdRequest( PICC_REQIDL, ATQA) != MI_OK )  //寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
	{
//		printf("\r\nRequest:fail");
		return 1;		
	}
	else
	{
//		printf("\r\nRequest:ok  ATQA:%02x %02x",ATQA[0],ATQA[1]);
	}
	

	//Anticoll 冲突检测
	if(PcdAnticoll(UID, PICC_ANTICOLL1)!= MI_OK)
	{		
//		printf("\r\nAnticoll:fail");
		return 1;		
	}
	else
	{	
//		printf("\r\nAnticoll:ok  UID:%02x %02x %02x %02x",UID[0],UID[1],UID[2],UID[3]);
	}
	
	//Select 选卡
	if(PcdSelect1(UID,&SAK)!= MI_OK)
	{
//		printf("\r\nSelect:fail");
		return 1;		
	}
	else
	{
//		printf("\r\nSelect:ok  SAK:%02x",SAK);
	}

	//Authenticate 验证密码
	if(PcdAuthState( PICC_AUTHENT1A, 4, DefaultKeyABuf, UID ) != MI_OK )
	{
		//printf("\r\nAuthenticate:fail");
		return 1;		
	}
	else
	{
		//printf("\r\nAuthenticate:ok");
	}

	//读BLOCK原始数据
	if( PcdRead( 4, CardReadBuf ) != MI_OK )
	{
		//printf("\r\nPcdRead:fail");
		return 1;		
	}
	else
	{
		//printf("\r\nPcdRead:ok  ");
		for(i=0;i<16;i++)
		{
			//printf(" %02x",CardReadBuf[i]);
		}
	}

	//产生随机数
	for(i=0;i<16;i++)
		CardWriteBuf[i] = rand();

	//写BLOCK 写入新的数据
	if( PcdWrite( 4, CardWriteBuf ) != MI_OK )
	{
//		printf("\r\nPcdWrite:fail");
		return 1;	
	}
	else
	{
//		printf("\r\nPcdWrite:ok  ");
		for(i=0;i<16;i++)
		{
//			printf(" %02x",CardWriteBuf[i]);
		}
	}
		
	//读BLOCK 读出新写入的数据
	if( PcdRead( 4, CardReadBuf ) != MI_OK )
	{
//		printf("\r\nPcdRead:fail");
		return 1;		
	}
	else
	{
//		printf("\r\nPcdRead:ok  ");
		for(i=0;i<16;i++)
		{
//			printf(" %02x",CardReadBuf[i]);
		}
	}
		
//	//Halt
//	if(PcdHalt() != MI_OK)
//	{
//		printf("\r\nHalt:fail");
//		return 1;		
//	}
//	else
//	{
//		printf("\r\nHalt:ok");
//	}	
	
	return 0;
}

//***********************************//修改新增内容

/***********************************************
 * 函数名：PcdReset
 * 描述  ：复位RC522 
 * 输入  ：无
 * 返回  : 无
 * 调用  ：外部调用
 **********************************************/
void PcdReset ( void )
{
	//hard reset
//	HAL_GPIO_WritePin(S52_NRSTPD_GPIO_Port,S52_NRSTPD_Pin,GPIO_PIN_RESET);
//	delay_us(100);
//	HAL_GPIO_WritePin(S52_NRSTPD_GPIO_Port,S52_NRSTPD_Pin,GPIO_PIN_SET);
//	delay_us(100);
	
	I_SI522_IO_Write(CommandReg, 0x0f);			      //向CommandReg 写入 0x0f	作用是使RC522复位
	while(I_SI522_IO_Read(CommandReg) & 0x10 );	  //Powerdown位为0时，表示RC522已准备好
	delay_us(100);
}




void PcdPowerdown (void)
{
//	GPIO_ResetBits( GPIOA , GPIO_Pin_2 );
//	delay_ms(5);
//	GPIO_SetBits( GPIOA , GPIO_Pin_2 );
//	delay_ms(5);
}


//SI522_interfaces
void I_SI522_ClearBitMask(unsigned char reg,unsigned char mask)  
{
	char tmp = 0x00;
	tmp = I_SI522_IO_Read(reg);
	I_SI522_IO_Write(reg, tmp & ~mask);  // clear bit mask
} 

void I_SI522_SetBitMask(unsigned char reg,unsigned char mask)  
{
	char tmp = 0x00;
	tmp = I_SI522_IO_Read(reg);
	I_SI522_IO_Write(reg,tmp | mask);  // set bit mask
}

void I_SI522_SiModifyReg(unsigned char RegAddr, unsigned char ModifyVal, unsigned char MaskByte)
{
	unsigned char RegVal;
	RegVal = I_SI522_IO_Read(RegAddr);
	if(ModifyVal)
	{
			RegVal |= MaskByte;
	}
	else
	{
			RegVal &= (~MaskByte);
	}
	I_SI522_IO_Write(RegAddr, RegVal);
}
