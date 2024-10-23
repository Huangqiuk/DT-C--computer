**************************************************************************************************************************
*  ��˵����
*          ɾ���˹켣��¼�Ĺ��ܺ󣬲�����Ҫ��SPI Flash��Ƶ��д��켣�����ݣ�����������DS1302��RAM������Ƶ���޸ĵ����ݣ�
*      һ�����н�������д��Flash
*  DS1302Ƭ��RAMʹ�ö���
*  [ 0] - ʱ�����б�ʶ
*  [ 1] - ���ʶ��Ϊ0x55˵����ǰ����δд��E2PROM��Flash������ֵ˵����ǰ������д��
*  [ 2] - ʱ��:��
*  [ 3] - ʱ��:��
*  [ 4] - ʱ��:��
*  [ 5] - ʱ��:ʱ
*  [ 6] - ʱ��:��
*  [ 7] - ʱ��:��
*  [ 8] - �˶�ʱ��(min)H
*  [ 9] - �˶�ʱ��(min)L
*  [10] - ����ٶ�H
*  [11] - ����ٶ�L
*  [12] - ��·��(KCal)H
*  [13] - ��·��(KCal)L
*  [14] - ���H
*  [15] - ���L
*  [16] - �˶�ʱ���
*  [17] - ����У׼ʱ���־
*  [18] - 
*  [19] - 
*  [20] - 
*  [21] - 
*  [22] - 
*  [23] - 
*  [24] - 
*  [25] - 
*  [26] - 
*  [27] - 
*  [28] - 
*  [29] - 
*  [30] - 
***************************************************************************************************************************/
#include "timer.h"
#include "delay.h"
#include "record.h"
#include "BleProtocol.h"
#include "ds1302.h"
#include "param.h"
#include "iap.h"

RECORD_CB recordCB;

RECORD_ITEM_TMP_CB recordTmpCB = {0};

// ���س������õ���Կ
const uint8 constFormatFlag[] = RECORD_NVM_FORMAT_FLAG;

// �������ʶ
void RECORD_SetDirtyFlag(void)
{
	// дͳ�����ݺ��������ʶ
	DS1302_WriteRAM(RECORD_PARAM_ADDR_DIRTY_FLAG, 0x55);
}

// ������ʶ
void RECORD_ClearDirtyFlag(void)
{
	// ������ʶ
	DS1302_WriteRAM(RECORD_PARAM_ADDR_DIRTY_FLAG, 0);
}

// ��ȡ���ʶ�Ƿ���Ч
BOOL RECORD_IsDirty(void)
{
	if(0x55 == DS1302_ReadRAM(RECORD_PARAM_ADDR_DIRTY_FLAG))
	{
		return TRUE;
	}
	
	return FALSE;
}

// ��ȡ���м�¼����������δͬ��ʱ��ļ�¼
uint16 RECORD_GetRecordCount(void)
{
	uint8 bBuff[2];
	
	IAP_FlashReadByteArray(RECORD_ADDR_TOTAL, bBuff, 2);
	return (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
}

// ��ȡ�ѷ��ͼ�¼����
uint16 RECORD_GetRecordSendCount(void)
{
	uint8 bBuff[2];
	
	IAP_FlashReadByteArray(RECORD_ADDR_SEND_COUNT, bBuff, 2);
	return (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
}

// ���м�¼У��
uint8_t RECORD_Check(uint8_t *ptr, uint8_t len)
{
	uint8_t i;
	uint8_t cc = 0;
	
	for(i=0; i<len; i++)
	{
		cc ^= ptr[i];
	}
	return (~cc) ;
}

// �ӻ������ж�ȡһ������ͳ������
void RECORD_ReadBuffer(RECORD_ITEM_CB* pTrackCB)
{
	uint16 temp;
	
	if(NULL == pTrackCB)
	{
		return;
	}

	// ����ǰ��¼д�뻺��
	pTrackCB->param.dateTime.year = DS1302_ReadRAM(RECORD_PARAM_ADDR_DATE_YEAR);			// ���ֻ����ʮλ�͸�λ
	pTrackCB->param.dateTime.month = DS1302_ReadRAM(RECORD_PARAM_ADDR_DATE_MONTH);
	pTrackCB->param.dateTime.day = DS1302_ReadRAM(RECORD_PARAM_ADDR_DATE_DAY);
	pTrackCB->param.dateTime.hour = DS1302_ReadRAM(RECORD_PARAM_ADDR_DATE_HOUR);
	pTrackCB->param.dateTime.min = DS1302_ReadRAM(RECORD_PARAM_ADDR_DATE_MINUTE);
	pTrackCB->param.dateTime.sec = DS1302_ReadRAM(RECORD_PARAM_ADDR_DATE_SECOND);

	temp = DS1302_ReadRAM(RECODE_PARAM_ADDR_TRIPTIMER_H);
	temp <<= 8;
	temp += DS1302_ReadRAM(RECODE_PARAM_ADDR_TRIPTIMER_L);
	pTrackCB->param.tripTimer = temp;

	temp = DS1302_ReadRAM(RECORD_PARAM_ADDR_MAXSPEED_H);
	temp <<= 8;
	temp += DS1302_ReadRAM(RECORD_PARAM_ADDR_MAXSPEED_L);
	pTrackCB->param.speedMax = temp;

	temp = DS1302_ReadRAM(RECORD_PARAM_ADDR_KCAL_H);
	temp <<= 8;
	temp += DS1302_ReadRAM(RECORD_PARAM_ADDR_KCAL_L);
	pTrackCB->param.calories = temp;

	temp = DS1302_ReadRAM(RECORD_PARAM_ADDR_TRIP_H);
	temp <<= 8;
	temp += DS1302_ReadRAM(RECORD_PARAM_ADDR_TRIP_L);
	pTrackCB->param.trip = temp;

	pTrackCB->param.timeSlot = DS1302_ReadRAM(RECODE_PARAM_ADDR_TIMESLOT);

	pTrackCB->param.isTimeCheck = DS1302_ReadRAM(RECORD_PARAM_ADDR_IS_CHECK_BLE_TIME);
	
	pTrackCB->param.packCheck = RECORD_Check(pTrackCB->array, sizeof(RECORD_ITEM_CB) - 1);
}

// �ж�NVM�Ƿ��Ѹ�ʽ��
BOOL RECORD_IsFormatOK(void)
{
	uint8 i;
	//uint32 bBuff[1];
	
	// ��ȡ��ʽ����־
	IAP_FlashReadWordArray(IAP_MCU_RECORD_ADDR_BASE, (uint32*)recordCB.nvmFormat, RECORD_NVM_FORMAT_FLAG_SIZE/4);

	// ����Ƚϣ�ֻҪ����һ����ͬ������Ϊδ��ʽ��
	for (i = 0; i < RECORD_NVM_FORMAT_FLAG_SIZE; i++)
	{
		if(constFormatFlag[i] != recordCB.nvmFormat[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}

// ��ʽ��NVM
void RECORD_FormatNVM(void)
{
	uint8 i;
	uint8 bBuff[2] = {0};

	// ��ʼ����Ԫ����
	for (i = 0; i < RECORD_NVM_FORMAT_FLAG_SIZE; i++)
	{
		recordCB.nvmFormat[i] = constFormatFlag[i];
	}

	// д��ʽ���ܳ�
	IAP_FlashWriteWordArrayWithErase(IAP_MCU_RECORD_ADDR_BASE, (uint32*)recordCB.nvmFormat, RECORD_NVM_FORMAT_FLAG_SIZE/4);

	// ��ʼ����¼��Ϊ0
	IAP_FLASH_WriteByteWithErase(RECORD_ADDR_TOTAL, bBuff ,2);

	// �Ѷ�·�߱��Ϊ0
	IAP_FLASH_WriteByteWithErase(RECORD_ADDR_SEND_COUNT, bBuff ,2);

	// �������¼�����ݼ�¼��������
	PARAM_SetRecordCount(0);
}

// ������ݼ�¼
void RECORD_ClearRecords(void)
{
	uint8 bBuff[2] = {0};
	
	// ��ʼ����¼��Ϊ0
	IAP_FLASH_WriteByteWithErase(RECORD_ADDR_TOTAL, bBuff ,2);

	// �Ѷ�·�߱��Ϊ0
	IAP_FLASH_WriteByteWithErase(RECORD_ADDR_SEND_COUNT, bBuff ,2);

	// ������ʶ
	RECORD_ClearDirtyFlag();

	// �������¼�����ݼ�¼��������
	PARAM_SetRecordCount(0);
}

// д��һ�����м�¼�㣬�����ǰ�������������ݣ�����¿�ʼʱ�䣻����������������ݣ���ֻ���½���ʱ��
void RECORD_CALLBACK_DoBackup(uint32 param)
{
	TIME_E time;
	uint32 temp1, temp2;

	DS1302_GetDateCB(&time);
	
	// ��ǰ�����ݣ���д�뿪ʼʱ��
	if(!RECORD_IsDirty())
	{
		// ��������㱣����û�����ٶ�
		PARAM_ClearUserMaxSpeed();

		// ������ʼ����
		recordCB.start.ridingTime = PARAM_GetTotalRidingTime();
		recordCB.start.distance = paramCB.nvm.param.common.record.total.distance;
		recordCB.start.calories = PARAM_GetTotalCalories();
		
		// �ж������Ƿ�У׼��ʱ��
		// ����У׼���������ݵ�0���ֽڱ���Ϊ1��
		// ����Уû׼���������ݵ�0���ֽڱ���Ϊ0��
		// ע���ϵ���Ҫ�Ų��У��λ����1���дΪ0xFF��
		DS1302_WriteRAM(RECORD_PARAM_ADDR_IS_CHECK_BLE_TIME, (uint8)bleProtocolCB.isTimeCheck);
		
		DS1302_WriteRAM(RECORD_PARAM_ADDR_DATE_YEAR, time.year%100);			// ���ֻ����ʮλ�͸�λ
		DS1302_WriteRAM(RECORD_PARAM_ADDR_DATE_MONTH, time.month);
		DS1302_WriteRAM(RECORD_PARAM_ADDR_DATE_DAY, time.day);
		DS1302_WriteRAM(RECORD_PARAM_ADDR_DATE_HOUR, time.hour);
		DS1302_WriteRAM(RECORD_PARAM_ADDR_DATE_MINUTE, time.minute);
		DS1302_WriteRAM(RECORD_PARAM_ADDR_DATE_SECOND, time.second);

		// ��������ʱ���
		DS1302_WriteRAM(RECODE_PARAM_ADDR_TIMESLOT, time.hour);

		// д�뿪ʼʱ����������ʶ
		RECORD_SetDirtyFlag();
	}


	// ��ĿǰΪֹ������ʱ�䣬��λ:s
	temp2 = PARAM_GetTotalRidingTime() - recordCB.start.ridingTime;
	
	DS1302_WriteRAM(RECODE_PARAM_ADDR_TRIPTIMER_H, temp2>>8);
	DS1302_WriteRAM(RECODE_PARAM_ADDR_TRIPTIMER_L, temp2&0x00FF);
	
	// ��������ٶ�
	DS1302_WriteRAM(RECORD_PARAM_ADDR_MAXSPEED_H, PARAM_GetUserMaxSpeed()>>8);
	DS1302_WriteRAM(RECORD_PARAM_ADDR_MAXSPEED_L, PARAM_GetUserMaxSpeed()&0x00FF);

	// ���浽ĿǰΪֹ�Ŀ�·������
	temp1 = PARAM_GetTotalCalories() - recordCB.start.calories;
	DS1302_WriteRAM(RECORD_PARAM_ADDR_KCAL_H, temp1>>8);
	DS1302_WriteRAM(RECORD_PARAM_ADDR_KCAL_L, temp1&0x00FF);

	// ���浽ĿǰΪֹ�����о���
	temp1 = paramCB.nvm.param.common.record.total.distance - recordCB.start.distance;
	DS1302_WriteRAM(RECORD_PARAM_ADDR_TRIP_H, temp1>>8);
	DS1302_WriteRAM(RECORD_PARAM_ADDR_TRIP_L, temp1&0x00FF);
}

// �ж��Ƿ���Ҫ���������
void RECORD_CALLBACK_IsDoBackup(uint32 param)
{	
	RECORD_CALLBACK_DoBackup(0);
}

// δ��ʱ���м�¼����
void RECORD_Update(uint32 param)
{
	static uint8 updateFlag;
	//static uint32 ridingTime;
	static uint32 distance;
	static uint32 calories;
	static uint16 tripTimeTmp;
	
	recordTmpCB.tirpTime++;
	tripTimeTmp++;

	// �״ν������ݵ�ǰ��Ϣ
	if (updateFlag == 0)
	{
		// ��������㱣����û�����ٶ�
		PARAM_ClearUserMaxSpeed();
	
		//ridingTime = PARAM_GetTotalRidingTime();
		distance = paramCB.nvm.param.common.record.total.distance;
		calories = PARAM_GetTotalCalories();
		
		updateFlag = 1;
		
		tripTimeTmp = 1;
	}else if ((param == 1) && (updateFlag == 1))
	{
		updateFlag = 0;

		if (recordTmpCB.tirpNum >= RECORD_RIDING_COUNT)
		{
			return;
		}
		
		// ����ʱ���ʱ��,��ʱ����ڵ����Ϊ0���򲻸�������ʱ��
		if (paramCB.nvm.param.common.record.total.distance - distance != 0)
		{
			recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.ridingTime += tripTimeTmp;
		}
		
		// �����ۼƷ���ʱ��
		recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.min = (uint16)(recordTmpCB.tirpTime/60);
		
		// ��������ٶ�
		if (recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.speedMax < PARAM_GetUserMaxSpeed())
		{
			recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.speedMax = PARAM_GetUserMaxSpeed();
		}
		
		// ���п�·��
		recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.calories += PARAM_GetTotalCalories() - calories;;
		
		// �������
		recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.trip += paramCB.nvm.param.common.record.total.distance - distance;
	}
	
	// δ��ʱ����£�һ����ʼ���оͿ�ʼÿ5min��¼һ������
	if (recordTmpCB.tirpTime%RECORD_RIDING_TIME == 0)
	{
		updateFlag = 0;

		if (recordTmpCB.tirpNum >= RECORD_RIDING_COUNT)
		{
			return;
		}
		
		// ����ʱ���ʱ��,��ʱ����ڵ����Ϊ0���򲻸�������ʱ��
		if (paramCB.nvm.param.common.record.total.distance - distance != 0)
		{
			recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.ridingTime += tripTimeTmp;
		}
		
		// �����ۼƷ���ʱ��
		recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.min = (uint16)(recordTmpCB.tirpTime/60);
		
		// ��������ٶ�
		if (recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.speedMax < PARAM_GetUserMaxSpeed())
		{
			recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.speedMax = PARAM_GetUserMaxSpeed();
		}
		
		// ���п�·��
		recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.calories += PARAM_GetTotalCalories() - calories;;
		
		// �������
		recordTmpCB.recordItemTmp[recordTmpCB.tirpNum].recordparam.trip += paramCB.nvm.param.common.record.total.distance - distance;

		recordTmpCB.tirpNum++;
	}
}

// ����ǰ����д��NVM
void RECORD_SaveToNVM(void)
{
	uint16 count;
	uint32 addr;
	RECORD_ITEM_CB track;
	uint8 bBuff[2];
	
	// ��ǰ�޻��棬�˳�
	if(!RECORD_IsDirty())
	{
		return;
	}

	// ���м�¼�����ﵽ�򳬹������������˳�
	IAP_FlashReadByteArray(RECORD_ADDR_TOTAL, bBuff, 2);
	count = (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
	if(count >= (uint16)RECORD_COUNT_MAX)
	{
		return;
	}

	// ��ȡ����
	RECORD_ReadBuffer(&track);

	// �����д��λ�õ��׵�ַ
	addr = sizeof(RECORD_PARAM_CB);
	addr *= count;
	addr += RECORD_ADDR_DATA_START;

	// д������
	IAP_FLASH_WriteByteWithErase(addr, (uint8*)track.array, sizeof(RECORD_PARAM_CB));	

	// ��¼�����ۼ�
	bBuff[0] = ((uint8)(count+1));
	bBuff[1] = ((uint8)(count+1) >> 8);
	IAP_FLASH_WriteByteWithErase(RECORD_ADDR_TOTAL, bBuff, 2);
	
	// �������
	RECORD_ClearDirtyFlag();

	// ���²������¼�����ݼ�¼����
	PARAM_SetRecordCount(count+1);
}

// ��ȡ�����һ���ļ�¼
BOOL RECORD_ReadFromNVM(RECORD_ITEM_CB* pTrack)
{
	uint16 totalCount, readCount;
	uint32 addr;
	uint8 bBuff[2];

	if(NULL == pTrack)
	{
		return FALSE;
	}

	// ��ȡ���м�¼�������Ѷ�������
	IAP_FlashReadByteArray(RECORD_ADDR_TOTAL, bBuff, 2);
	totalCount = (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
	IAP_FlashReadByteArray(RECORD_ADDR_SEND_COUNT, bBuff, 2);
	readCount = (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
	
	// �Ѷ������ﵽ�򳬳����������˳��������ʷ��¼
	if(readCount >= totalCount)
	{
		RECORD_ClearRecords();
		return FALSE;
	}

	// �����ȡ���׵�ַ
	addr = sizeof(RECORD_PARAM_CB);
	addr *= readCount;
	addr += RECORD_ADDR_DATA_START;

	// ��ȡ
	IAP_FlashReadByteArray(addr, (uint8*)pTrack->array, sizeof(RECORD_PARAM_CB));
	// �ж�У��λ�Ƿ�����ȷ������
	if (pTrack->param.packCheck != RECORD_Check(pTrack->array, sizeof(RECORD_ITEM_CB) - 1))
	{
		return FALSE;
	}

	// �ж��Ƿ�������У׼֮������ݣ�������ǣ������ε����������ϴ������·�����������
	if (0x01 != pTrack->param.isTimeCheck)
	{
		// �����ѷ�����ʷ����
		bBuff[0] = (uint8)(readCount+1);
		bBuff[1] = (uint8)((readCount+1) >> 8);
		IAP_FLASH_WriteByteWithErase(RECORD_ADDR_SEND_COUNT, bBuff, 2);
		return FALSE;
	}


	// �����Ѷ�ȡ����
	//SPI_FLASH_WriteHalfWord(RECORD_ADDR_SEND_COUNT, readCount+1);	 
	
	return TRUE;
}

// ˢ���ѷ�����ʷ��������
BOOL RECORD_RefreashSendCount(void)
{
	uint16 totalCount,readCount;
	uint8 bBuff[2];
	
		// ��ȡ���м�¼�������Ѷ�������
	IAP_FlashReadByteArray(RECORD_ADDR_TOTAL, bBuff, 2);
	totalCount = (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
	IAP_FlashReadByteArray(RECORD_ADDR_SEND_COUNT, bBuff, 2);
	readCount = (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
	
	// �Ѷ������ﵽ�򳬳����������˳�
	if(readCount >= totalCount)
	{
		return FALSE;
	}

	// �����ѷ�����ʷ����
	bBuff[0] = (uint8)(readCount+1);
	bBuff[1] = (uint8)((readCount+1) >> 8);
	IAP_FLASH_WriteByteWithErase(RECORD_ADDR_SEND_COUNT, bBuff, 2);
	
	return TRUE;
}

void RECORD_UnitTest(void)
{
	
	uint8 testWriteBuf[50] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
	uint8 testReadBuf[50] = {0x00};
	
	// д������
	IAP_FLASH_WriteByteWithErase(RECORD_ADDR_DATA_START, testWriteBuf, 32);	
	// ��������
	IAP_FlashReadByteArray(RECORD_ADDR_DATA_START, testReadBuf, 32);
}

// ˢ��д��д��������ʷ��������
void RECORD_RefreashSaveCount(void)
{
	uint16 totalCount;
	uint8 bBuff[2];
	
	IAP_FlashReadByteArray(RECORD_ADDR_TOTAL, bBuff, 2);
	totalCount = (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
	
	bBuff[0] = (uint8)(totalCount+1);
	bBuff[1] = (uint8)((totalCount+1) >> 8);
	
	IAP_FLASH_WriteByteWithErase(RECORD_ADDR_TOTAL, bBuff, 2);
}

// ������Ϣģ���ϵ��ʼ��׼��
void RECORD_Init(void)
{
//	int16 i;
//	uint16 count;
//	uint16 tempCount;
//	uint32 addr;

	// δ��ʽ�������ʽ��
	if (!RECORD_IsFormatOK())
	{
		RECORD_FormatNVM();
	}
	else
	{
//		// ��ȡ�ܼ�¼����
//		count = RECORD_GetRecordCount();
//		tempCount = count;
//		if ((count != 0x00) && (count < (uint16)RECORD_COUNT_MAX))
//		{
//			// ��ѯFlash��ַ����IsTimeCheckλΪ0�����Ϊ0xFF
//			for (i = count-1; i >= 0; i--)
//			{
//				addr = RECORD_ADDR_BLE_CHECK_TIME_START + (i * sizeof(RECORD_PARAM_CB));
//				if ((0x00 == SPI_FLASH_ReadByte(addr))		// �ϴο�����ʱ�洢�ļ�¼
//					|| (0x01 != SPI_FLASH_ReadByte(addr)))	// ����ԭ���¸ü�¼��Ч
//				{
//					SPI_FLASH_WriteByte(addr, 0xFF);
//			
//					// ���¼�¼������
//					tempCount--;
//					SPI_FLASH_WriteHalfWord(RECORD_ADDR_TOTAL, tempCount);	
//				}
//			}
//		}
	}

#if RECORD_UNIT_TEST_ON
	RECORD_UnitTest();
#endif

	// Ĭ��״̬Ϊ��
	recordCB.state = RECORD_STATE_NULL;
	recordCB.preState = RECORD_STATE_NULL;

	// ����¼��������������
	PARAM_SetRecordCount(RECORD_GetRecordCount());

	// ָ��ʱ���������
	TIMER_AddTask(TIMER_ID_RECORD_CONTROL,
					500UL,
					RECORD_EnterState,
					RECORD_STATE_STANDBY,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
}

// ÿ��״̬����ڴ���
void RECORD_EnterState(uint32 state)
{
	// �õ�ǰ��״̬��Ϊ��ʷ
	recordCB.preState = recordCB.state;

	// �����µ�״̬
	recordCB.state = (RECORD_STATE_E)state;

	// ��������������
	if (!PARAM_GetExistBle())
	{
		return;
	}
	
	// ��״̬������趨
	switch(state)
	{
		// ���� ��״̬ ����
		case RECORD_STATE_NULL:
			break;

		// ���� ����״̬��ڴ��� ����
		case RECORD_STATE_STANDBY:
		
			// ֹͣ��¼��ʱ��
			TIMER_KillTask(TIMER_ID_RECORD_CONTROL);

			if (bleProtocolCB.isTimeCheck == FALSE)
			{
				RECORD_Update(1);
			}
			else
			{
				// ÿ�ν������״̬��������һ��
				RECORD_SaveToNVM();
			}

			// �ж������Ƿ����ӣ��Ƿ�����ʷ����
			if((PARAM_GetBleConnectState()) && (RECORD_IsHistoricalData()) && (PARAM_GetSpeed() == 0))
			{
				// ����������һ��ͬ����������
//				BLE_PROTOCOL_TxStartHistoryData(TRUE);
				
				// ���û�н��յ�Ӧ��������ѭ����������ͬ������
				TIMER_AddTask(TIMER_ID_BLE_START_HISTORICAL_DATA_UP,
								5000,
								BLE_PROTOCOL_TxStartHistoryData,
								UINT32_NULL,
								TIMER_LOOP_FOREVER,
								ACTION_MODE_ADD_TO_QUEUE);
			}

			break;

		// ���� ��¼��״̬��ڴ��� ����
		case RECORD_STATE_RECORDING:
		
			// ��ʼ�������м�¼,�ϵ�ֻ�ܽ���һ��
			if (bleProtocolCB.isTimeCheck == FALSE)
			{
				if (recordTmpCB.tripFlag == 0)
				{
					recordTmpCB.tripFlag = 1;
					
					TIMER_AddTask(TIMER_ID_RECORD_COUNT,
									1000UL,
									RECORD_Update,
									0,
									TIMER_LOOP_FOREVER,
									ACTION_MODE_ADD_TO_QUEUE);
				}
			}
			else
			{
				// ÿ�ν����״̬������¼һ������
				RECORD_CALLBACK_DoBackup(0);

				// �״ν����״̬��������ʱ������ÿ���¼һ��(��Ҫ����ģ��������)
				if(recordCB.preState != recordCB.state)
				{
					TIMER_AddTask(TIMER_ID_RECORD_CONTROL,
									1000UL,
									RECORD_CALLBACK_IsDoBackup,
									0,
									TIMER_LOOP_FOREVER,
									ACTION_MODE_ADD_TO_QUEUE);
				}
			}

			break;

		default:
			break;
	}
}

// ÿ��״̬�µĹ��̴���
void RECORD_Process(void)
{
	TIME_E time;
	static uint8 lastSecond;
	
	switch(recordCB.state)
	{
		// ���� ��״̬ ����
		case RECORD_STATE_NULL:
			break;

		// ���� ����״̬���̴��� ����
		case RECORD_STATE_STANDBY:
			// �˶�ʱ��ﵽ�򳬹����ޣ���ʼ��¼
			if(PARAM_GetMoveTimeKeepTime() >= RECORD_MOVE_TIME_THRESHOLD)
			{
				// �����¼��״̬
				RECORD_EnterState(RECORD_STATE_RECORDING);
			}
			break;

		// ���� ��¼��״̬���̴��� ����
		case RECORD_STATE_RECORDING:
			// ��ֹʱ��ﵽ�򳬹����ޣ�ֹͣ��¼
			if(PARAM_GetStopTimeKeepTime() >= RECORD_STOP_TIME_THRESHOLD)
			{
				// �������״̬
				RECORD_EnterState(RECORD_STATE_STANDBY);

				// �����ϱ�״̬��Ϣ
				if(PARAM_GetBleConnectState())
				{
					BLE_PROTOCOL_SendCmdStatusAck();
				}
			}

			// ����״̬�����У�����洢һ������
			DS1302_GetDateCB(&time);
			if((0 == time.minute) && (0 == time.second) && (59 == lastSecond))
			{
				// ÿ�ν�������״̬������һ��
				RECORD_SaveToNVM();
			}

			// ������һ���������ظ����������¼����
			lastSecond = time.second;
			break;

		default:
			break;
	}
}

// ��ȡ��¼״̬
RECORD_STATE_E RECORD_GetState(void)
{
	return recordCB.state;
}

// ��ѯ�Ƿ���δͬ����ʷ����
BOOL RECORD_IsHistoricalData(void)
{
	uint16 totalCount, readCount;
	uint8 bBuff[2];
	
	// ��ȡ���м�¼�������Ѷ�������
	IAP_FlashReadByteArray(RECORD_ADDR_TOTAL, bBuff, 2);
	totalCount = (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
	IAP_FlashReadByteArray(RECORD_ADDR_SEND_COUNT, bBuff, 2);
	readCount = (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];

	if(readCount < totalCount)
	{
		return TRUE;  
	}
	else
	{
		return FALSE;
	}
}




