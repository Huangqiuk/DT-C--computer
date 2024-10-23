#include "iapctrluart.h"
#include "timer.h"
#include "delay.h"
#include "spiflash.h"
#include "iap.h"
#include "bleUart.h"
#include "gsmUart.h"
#include "sysinfo.h"
#include "e2prom.h"
#include "system.h"
#include "spiFlash.h"

#include "param.h"
//#include "lcd.h"

#include "state.h"


/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void IAP_CTRL_UART_DataStructureInit(IAP_CTRL_UART_CB* pCB);

// UART��ʼ��
void IAP_CTRL_UART_HwInit(uint32 baud);

// ���ʹ���
void IAP_CTRL_UART_TxProcess(IAP_CTRL_UART_CB* pCB);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL IAP_CTRL_UART_ConfirmTempCmdFrameBuff(IAP_CTRL_UART_CB* pCB);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void IAP_CTRL_UART_RxFIFOProcess(IAP_CTRL_UART_CB* pCB);

// �Դ��������֡����У�飬����У����
BOOL IAP_CTRL_UART_CheckSUM(IAP_CTRL_CMD_FRAME* pCmdFrame);

// UART����֡����������
void IAP_CTRL_UART_CmdFrameProcess(IAP_CTRL_UART_CB* pCB);

// ͨѶ��ʱ����-����
void IAP_CTRL_UART_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void IAP_CTRL_UART_StopRxTimeOutCheck(void);

// TXRXͨѶ��ʱ����-˫��
void IAP_CTRL_UART_CALLBACK_TxRxTimeOut(uint32 param);

// ֹͣTXRXͨѶ��ʱ�������
void IAP_CTRL_UART_StopTxRxTimeOutCheck(void);

// �����ж��ֽڷ���
void IAP_CTRL_UART_IR_StartSendData(uint8 data);

// ��������һ���ֽ�����
void IAP_CTRL_UART_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void USART2_IRQHandler(void);

// ����ģʽ׼����������
void IAP_CTRL_UART_SendCmdProjectReady(uint8 param);
// ����FLASH����ϱ�
void IAP_CTRL_UART_SendCmdEraseFlashResult(uint8 param);
// IAP����д�����ϱ�
void IAP_CTRL_UART_SendCmdWriteFlashResult(uint8 param);
// ��ս���ϱ�
void IAP_CTRL_UART_SendCmdCheckFlashBlankResult(uint8 param);
// ϵͳ��������ȷ��
void IAP_CTRL_UART_SendCmdUpdataFinishResult(uint8 param);
// ����UI���ݲ������
void IAP_CTRL_UART_SendCmdUIEraseResultReport(uint8 param);
// ����UI����д����
void IAP_CTRL_UART_SendCmdUIWriteDataResultReport(uint8 param);
// ����д���ά�����ݽ��
void IAP_CTRL_UART_SendCmdQrWriteResultReport(uint8 param);

// ������������
void IAP_CTRL_UART_SendCmdWithResult(uint8 cmdWord, uint8 result);
// ���������޽��
void IAP_CTRL_UART_SendCmdNoResult(uint8 cmdWord);
// ��ȡ�����ϱ�
void IAP_CTRL_UART_SendSpiFlashData(uint32 ReadAddr, uint8 NumByteToRead);

void IAP_CTRL_UART_SendFlagCheck(uint32 checkNum);

// ��������MCU��Ψһ���кţ���ֹ�����ϻ�
void UART_PROTOCOL_SendMcuUid(uint32 param);


//=================================================================================

// ȫ�ֱ�������
IAP_CTRL_UART_CB iapCtrlUartCB;

// ��������������������������������������������������������������������������������������
// UART��ʼ��
void IAP_CTRL_UART_Init(void)
{	
	// Ӳ��UART����
	IAP_CTRL_UART_HwInit(IAP_CTRL_UART_BAUD_RATE);

	// UART���ݽṹ��ʼ��
	IAP_CTRL_UART_DataStructureInit(&iapCtrlUartCB);
}

// ���ݽṹ��ʼ��
void IAP_CTRL_UART_DataStructureInit(IAP_CTRL_UART_CB* pCB)
{
	uint8 i;
	
	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}

	pCB->tx.txBusy = FALSE;
	pCB->tx.index = 0;
	pCB->tx.head = 0;
	pCB->tx.end = 0;
	for(i=0; i<IAP_CTRL_UART_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}
	
	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.currentProcessIndex = 0;
	pCB->rxFIFO.end  = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<IAP_CTRL_UART_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART��ʼ��
void IAP_CTRL_UART_HwInit(uint32 baud)
{
	// GD32
	// ʱ������
	rcu_periph_clock_enable(RCU_GPIOA);
	
	// GPIO����
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);   //��������

	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, GPIO_PIN_3); 	// ��������
	
	// UARTʱ������
	rcu_periph_clock_enable(RCU_USART1);								// ʱ��USARTʱ��
	usart_deinit(IAP_CTRL_UART_TYPE_DEF);									// ��λ����

	// ��������
	usart_baudrate_set(IAP_CTRL_UART_TYPE_DEF, baud);						// ������
	usart_word_length_set(IAP_CTRL_UART_TYPE_DEF, USART_WL_8BIT);			// 8λ����λ
	usart_stop_bit_set(IAP_CTRL_UART_TYPE_DEF, USART_STB_1BIT);			// һ��ֹͣλ
	usart_parity_config(IAP_CTRL_UART_TYPE_DEF, USART_PM_NONE);			// ����żУ��
	usart_hardware_flow_rts_config(IAP_CTRL_UART_TYPE_DEF, USART_RTS_DISABLE); // ��Ӳ������������
	usart_hardware_flow_cts_config(IAP_CTRL_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(IAP_CTRL_UART_TYPE_DEF, USART_TRANSMIT_ENABLE);	// ʹ�ܷ���
	usart_receive_config(IAP_CTRL_UART_TYPE_DEF, USART_RECEIVE_ENABLE);		// ʹ�ܽ���

	// �ж�����
	nvic_irq_enable(IAP_CTRL_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(IAP_CTRL_UART_TYPE_DEF, USART_INT_RBNE);			// �����ж�

	usart_enable(IAP_CTRL_UART_TYPE_DEF);									// ʹ�ܴ��� 

}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL IAP_CTRL_UART_ConfirmTempCmdFrameBuff(IAP_CTRL_UART_CB* pCB)
{
	IAP_CTRL_CMD_FRAME* pCmdFrame = NULL;
	
	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return FALSE;
	}

	// ��ʱ������Ϊ�գ��������
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	if(0 == pCmdFrame->length)
	{
		return FALSE;
	}

	// ���
	pCB->rx.end ++;
	pCB->rx.end %= IAP_CTRL_UART_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
	
	return TRUE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void IAP_CTRL_UART_RxFIFOProcess(IAP_CTRL_UART_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	IAP_CTRL_CMD_FRAME* pCmdFrame = NULL;
	uint16 length = 0;
	uint8 currentData = 0;
	
	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}
	
	// һ��������Ϊ�գ��˳�
	if(head == end)
	{
		return;
	}

	// ��ȡ��ʱ������ָ��
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	// ȡ����ǰҪ������ֽ�
	currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
	
	// ��ʱ����������Ϊ0ʱ���������ֽ�
	if(0 == pCmdFrame->length)
	{
		// ����ͷ����ɾ����ǰ�ֽڲ��˳�
		if(IAP_CTRL_UART_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if((pCB->rx.end + 1)%IAP_CTRL_UART_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// ���UARTͨѶ��ʱʱ������-2016.1.5����
		#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL, IAP_CTRL_UART_BUS_UNIDIRECTIONAL_TIME_OUT, IAP_CTRL_UART_CALLBACK_RxTimeOut, 0, 1, ACTION_MODE_ADD_TO_QUEUE);
		#endif
		
		// ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= IAP_CTRL_UART_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if(pCmdFrame->length >= IAP_CTRL_UART_CMD_LENGTH_MAX)
		{
			#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			IAP_CTRL_UART_StopRxTimeOutCheck();
			#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0;	// 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// һֱȡ��ĩβ
		while(end != pCB->rxFIFO.currentProcessIndex)
		{
			// ȡ����ǰҪ������ֽ�
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			// ������δ������������գ���������ӵ���ʱ��������
			pCmdFrame->buff[pCmdFrame->length++]= currentData;
			pCB->rxFIFO.currentProcessIndex ++;
			pCB->rxFIFO.currentProcessIndex %= IAP_CTRL_UART_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ��������������
			
			// �����ж�����֡��С���ȣ�һ�����������������ٰ���4���ֽ�: ����ͷ + ������ + ���ݳ��� + У���룬��˲���4���ֽڵıض�������
			if(pCmdFrame->length < IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
			if(pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] > IAP_CTRL_UART_CMD_LENGTH_MAX-IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN)
			{
				#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				IAP_CTRL_UART_StopRxTimeOutCheck();
				#endif
			
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// ����֡����У�飬������������ֵ���ֵ�ϣ���������ͷ�������֡����ݳ��ȡ�У���룬��Ϊ����֡ʵ�ʳ���
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] + IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN)
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if(!IAP_CTRL_UART_CheckSUM(pCmdFrame))
			{
				#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				IAP_CTRL_UART_StopRxTimeOutCheck();
				#endif
				
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

			#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			IAP_CTRL_UART_StopRxTimeOutCheck();
			#endif
			
			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;		// head����Ҫ��16λ��������һ����ʱ�����
			pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			IAP_CTRL_UART_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// �Դ��������֡����У�飬����У����
BOOL IAP_CTRL_UART_CheckSUM(IAP_CTRL_CMD_FRAME* pCmdFrame)
{
	uint8 cc = 0;
	uint16 i = 0;
	
	if(NULL == pCmdFrame)
	{
		return FALSE;
	}

	// ������ͷ��ʼ����У����֮ǰ��һ���ֽڣ����ν����������
	for(i=0; i<pCmdFrame->length-1; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}

	cc = ~cc;

	// �жϼ���õ���У����������֡�е�У�����Ƿ���ͬ
	if(pCmdFrame->buff[pCmdFrame->length-1] != cc)
	{
		return FALSE;
	}
	
	return TRUE;
}

uint8 checkFlag = 0;

void checkVersion(uint32 param)
{
	IAP_CTRL_UART_SendVersionCheck(checkFlag);
	checkFlag++;
}

// UART����֡����������
void IAP_CTRL_UART_CmdFrameProcess(IAP_CTRL_UART_CB* pCB)
{
	IAP_CTRL_UART_CMD cmd = IAP_CTRL_UART_CMD_NULL;
	IAP_CTRL_CMD_FRAME* pCmdFrame = NULL;
	uint32 temp = 0;
	uint32 temp2= 0;
	uint16 length = 0;
	uint8 errStatus;
	uint16 i;
	BOOL bTemp;
	uint8* updateBuff = NULL;
	uint8 localBuf[50];
	uint32 temp3,temp4,temp5;
//	IAP_CTRL_UART_SendCmdWithResult(0x78, TRUE);
	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}

	// ����֡������Ϊ�գ��˳�
	if(pCB->rx.head == pCB->rx.end)
	{
		return;
	}

	// ��ȡ��ǰҪ���������ָ֡��
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.head];
	
	// ����ͷ�Ƿ����˳�
	if(IAP_CTRL_UART_CMD_HEAD != pCmdFrame->buff[IAP_CTRL_UART_CMD_HEAD_INDEX])
	{
		// ɾ������֡
		pCB->rx.head ++;
		pCB->rx.head %= IAP_CTRL_UART_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (IAP_CTRL_UART_CMD)pCmdFrame->buff[IAP_CTRL_UART_CMD_CMD_INDEX];
	
	// ִ������֡
	switch(cmd)
	{		
		case IAP_CTRL_UART_CMD_NULL:
			break;

		// �յ��Է�ECO����
		case IAP_CTRL_UART_CMD_UP_PROJECT_APPLY:
//			// �ر�����APP1��ʱ��
//			TIMER_KillTask(TIMER_ID_IAP_RUN_WINDOW_TIME);
//			
//			// ����ɹ�����ָ��APP,���ָ����ַAPP�Ƿ����ϱ�ʧ��
//			errStatus = IAP_CheckAppRightful(IAP_FLASH_APP1_ADDR);
//			if(0 == paramCB.runtime.jumpFlag)
//			{
//				paramCB.runtime.jumpFlag = (BOOL)1;
//				
//				// ���������ʱ�ظ�����ת
//				TIMER_AddTask(TIMER_ID_ECO_JUMP,
//								200,
//								IAP_JumpToAppFun,
//								IAP_FLASH_APP1_ADDR,
//								1,
//								ACTION_MODE_ADD_TO_QUEUE);
//			}
			
			// �ر�����APP1��ʱ��
				TIMER_KillTask(TIMER_ID_IAP_RUN_WINDOW_TIME);
				
	
				TIMER_AddTask(TIMER_ID_IAP_RUN_WINDOW_TIME,
									500,
									IAP_JumpToAppFun,
									IAP_FLASH_APP2_ADDR,
									1,
									ACTION_MODE_ADD_TO_QUEUE);
									
				UART_PROTOCOL_SendMcuUid(1);
				
				// ����MCU_UID
				TIMER_AddTask(TIMER_ID_MCU_UID,
								50,
								UART_PROTOCOL_SendMcuUid,
								1,
								4,
								ACTION_MODE_ADD_TO_QUEUE);		
			// ���ͽ��
			//IAP_CTRL_UART_SendCmdWithResult(0x0F, errStatus);
			break;
			
		// ����MCU_UID
		case UART_ECO_CMD_ECO_MCU_UID:
		
			// �յ���MCU_UID˵��ECO����������Ѿ��յ��ˣ�����ֹͣ�������
			TIMER_KillTask(TIMER_ID_IAP_ECO_REQUEST);
			TIMER_KillTask(TIMER_ID_MCU_UID);
		
			// ���ܵ�MCU_UID�����Ƿ����12
			if (UART_PROTOCOL_MCU_UID_BUFF_LENGTH == pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX])
			{
				
				temp3 = ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA4_INDEX]) << 24) 
						+ ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA3_INDEX]) << 16) 
						+ ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX]) << 8) 
						+ ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX]));

				temp4 = ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA8_INDEX]) << 24) 
						+ ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA7_INDEX]) << 16) 
						+ ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX]) << 8) 
						+ ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX]));

				temp5 = ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA12_INDEX]) << 24) 
						+ ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA11_INDEX]) << 16) 
						+ ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA10_INDEX]) << 8) 
						+ ((uint32)(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA9_INDEX]));
			
				// MCU_UIDһ�������
				if ((iapCB.mcuUID.sn0 == temp3) && (iapCB.mcuUID.sn1 == temp4) && (iapCB.mcuUID.sn2 == temp5))
				{
					break;
				}
				// MCU_UID��ͬ��д���ϻ���־
				else
				{
					// �������ж���Ϊ��ֻ����һ�Σ���ֹ��ν���
					if (0x55AA55BB != iapCtrlUartCB.ageFlag)
					{
						// д���ϻ������־
						iapCtrlUartCB.ageFlag = 0x55AA55BB;
						
						SPI_FLASH_WriteWord(SPI_FLASH_TEST_FLAG_ADDEESS, iapCtrlUartCB.ageFlag);
					}
				}
			}
			break;

		// ����ģʽ��׼
		case IAP_CTRL_UART_CMD_DOWN_PROJECT_APPLY_ACK:
			// �رմ˶�ʱ��
			TIMER_KillTask(TIMER_ID_IAP_RUN_WINDOW_TIME);
			TIMER_KillTask(TIMER_ID_IAP_ECO_REQUEST);

			// ���ͽ��
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_PROJECT_READY, TRUE);
			break;

		// �ָ���������
		case IAP_CTRL_UART_CMD_DOWN_FACTORY_RESET:
			break;

		// ����RTC
		case IAP_CTRL_UART_CMD_DOWN_RTC_SET:
			break;

		// ϵͳ����д��
		case IAP_CTRL_UART_CMD_WRITE_SYS_PARAM:
			switch (pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX])
			{
				case 0:
					temp = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX];
					temp <<= 8;
					temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA3_INDEX];
					paramCB.factoryNvm.param.sys.newWheelSize = temp;

					temp = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA4_INDEX];
					temp <<= 8;
					temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
					paramCB.factoryNvm.param.sys.newPerimeter = temp;

					// ��λ
					paramCB.factoryNvm.param.sys.unit = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX];

					// ����
					paramCB.factoryNvm.param.sys.speedLimitVal = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA7_INDEX];

					// ���ٴŸ�
					paramCB.factoryNvm.param.sys.steelNumOfSpeedSensor = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA8_INDEX];

					// ��������
					paramCB.factoryNvm.param.sys.brightness = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA9_INDEX];

					// �Զ��ػ�ʱ��
					paramCB.factoryNvm.param.sys.powerOffTime = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA10_INDEX];

					// Э��
					paramCB.factoryNvm.param.sys.protocol = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA11_INDEX];

					// ϵͳ��ѹ
					paramCB.factoryNvm.param.sys.batVoltage = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA12_INDEX];

					// ������ʽ
					paramCB.factoryNvm.param.sys.percentageMethod = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA13_INDEX];

					// �Ƴ�����
					paramCB.factoryNvm.param.sys.pushAssistSwitch = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA14_INDEX];

					// Ĭ�ϵ�λ
					paramCB.factoryNvm.param.sys.defaultAssist = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA15_INDEX];

					// ���λ
					paramCB.factoryNvm.param.sys.maxAssist = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA16_INDEX];

					// ���ػ�logo
					paramCB.factoryNvm.param.sys.logo = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA17_INDEX];

					// ����д���־
					FACTORY_NVM_SetDirtyFlag(TRUE);

					// ����Ӧ���ź�
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_WRITE_SYS_PARAM);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 2);		// ���ݳ���
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);		// ��������
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, TRUE);
					IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
					break;

				default:
					break;
			}
			break;

		case IAP_CTRL_UART_CMD_READ_SYS_PARAM:
			switch (pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX])
			{
				case 0:
					// ��������
					SPI_FLASH_ReadArray(paramCB.factoryNvm.array, SPI_FLASH_FACTORY_PARAM_ADDEESS, PARAM_FACTORY_NVM_DATA_SIZE);
					SPI_FLASH_ReadArray(paramCB.preFactoryValue, SPI_FLASH_FACTORY_PARAM_ADDEESS, PARAM_FACTORY_NVM_DATA_SIZE);
					
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_READ_SYS_PARAM);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);		// ��������

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.newWheelSize>>8);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.newWheelSize>>0);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.newPerimeter>>8);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.newPerimeter>>0);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.unit);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.speedLimitVal);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.steelNumOfSpeedSensor);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.brightness);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.powerOffTime);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.protocol);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.batVoltage);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.percentageMethod);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.pushAssistSwitch);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.defaultAssist);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.maxAssist);

					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, paramCB.factoryNvm.param.sys.logo);
					
					
					IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
					
					break;

				default:
					break;
			}
			break;

		// UI���ݲ���
		case IAP_CTRL_UART_CMD_DOWN_UI_DATA_ERASE:
			// �Կ鷽ʽ����
#if (0 == SPI_FLASH_ERASE_MODE)
			// ��������
			SPI_FLASH_EraseRoom(SPI_FLASH_UI_OFFSET_ADDR, SPI_FLASH_UI_SIZE);
			
#elif (1 == SPI_FLASH_ERASE_MODE)
			// ��������FLASH
			SPI_FLASH_EraseChip();
			
#elif (2 == SPI_FLASH_ERASE_MODE)
			SPI_FLASH_EraseRoom(SPI_FLASH_UI_OFFSET_ADDR, SPI_FLASH_UI_SIZE);
#endif

			// ���ͽ��
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_UI_DATA_ERASE_ACK, TRUE);
			break;

		// UI����д��
		case IAP_CTRL_UART_CMD_DOWN_UI_DATA_WRITE:
			// ��ֹSPI���޸�����
			spi_disable(STM32_SPIx_NUM_DEF);
			SPIx_ConfigureWorkMode(SPI_TRANSMODE_FULLDUPLEX, SPI_FRAMESIZE_8BIT);
			spi_enable(STM32_SPIx_NUM_DEF);
			temp  = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA3_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA4_INDEX];

			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] - 4;

			// д�����ݵ�flash����
			errStatus = SPI_FLASH_WriteWithCheck(SPI_FLASH_UI_OFFSET_ADDR + (uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX], temp, length);

			// ���ͽ��
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_UI_DATA_WRITE_RESULT, errStatus);
			break;

		// APP����
		case IAP_CTRL_UART_CMD_DOWN_IAP_ERASE_FLASH:
			// ��ʾuart������ʾ
			
			// �Ȳ���APP������
			IAP_EraseAPP1Area();

			// ���������Ҫ����������Ƿ�Ϊ��
			errStatus = IAP_CheckApp1FlashIsBlank();

			// ���ͽ��
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_IAP_ERASE_FLASH_RESULT, errStatus);

			break;
			
		// APP����д��
		case IAP_CTRL_UART_CMD_DOWN_IAP_WRITE_FLASH:
			{
				uint8 encryptionMode;
				uint8 key;
				static uint8 arrbuf[IAP_CTRL_UART_CMD_LENGTH_MAX];
				
				encryptionMode = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];
				key 		   = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX];

				switch (encryptionMode)
				{
					case 0x00:	// �޼��ܣ�����Կ��������
						temp  = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA3_INDEX];
						temp <<= 8;
						temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA4_INDEX];
						temp <<= 8;
						temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
						temp <<= 8;
						temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX];

						length = pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] - 6;

						// д������
						errStatus = IAP_WriteAppBin(temp, (uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA7_INDEX], length);
						break;

					case 0x01:	// ���ܣ���ַ�����ݾ��롾��Կ��������ʹ��
						temp  = (pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA3_INDEX] ^ key);
						temp <<= 8;
						temp += (pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA4_INDEX] ^ key);
						temp <<= 8;
						temp += (pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX] ^ key);
						temp <<= 8;
						temp += (pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX] ^ key);

						length = pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] - 6;

						for (i = 0; i < length; i++)
						{
							arrbuf[i] = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA7_INDEX + i] ^ key;
						}

						// д������
						errStatus = IAP_WriteAppBin(temp, arrbuf, length);
						break;

					case 0x02:
						errStatus = FALSE;
						break;

					default:
						errStatus = FALSE;
						break;
				}

				// ���ͽ��
				IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_IAP_WRITE_FLASH_RESULT, errStatus);
			}

			break;

		// ���
		case IAP_CTRL_UART_CMD_DOWN_CHECK_FALSH_BLANK:
			errStatus = IAP_CheckApp2FlashIsBlank();

			// ���ͽ��
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_CHECK_FLASH_BLANK_RESULT, errStatus);
			break;

		// APP�������
		case IAP_CTRL_UART_CMD_DOWN_UPDATA_FINISH:
			// �������������Ҫ��������������־�����򲻻����Ӧ��APP
			// �������������־
			SPI_FLASH_WriteByte(SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS,0xFF);

			// �������������ʾ
			
			
			// ����Ӧ��
			IAP_CTRL_UART_SendCmdNoResult(IAP_CTRL_UART_CMD_UP_UPDATA_FINISH_RESULT);
			break;


		// ������ά��ע��
		case IAP_CTRL_UART_CMD_DOWN_START_QR_REGISTER:
			// ʹ�ܿ�ʼ��ά��ע��
			paramCB.runtime.qrRegEnable = TRUE;
			
			// ע���ѯBLE��MAC��ַ
			TIMER_AddTask(TIMER_ID_BLE_CONTROL,
							200,
							BLE_UART_CALLBALL_GetBleMacAddrRequest,
							TRUE,
							TIMER_LOOP_FOREVER,
							ACTION_MODE_ADD_TO_QUEUE);

			// ����ICCIDע�ᶨʱ��				
			gsmUartCB.rcvIccidTimeOut = TRUE;
			
			/*
			// ע���ѯICCID��ʱ��
			TIMER_AddTask(TIMER_ID_GSM_CONTROL,
							GSM_UART_TX_QUERY_ICCID_TIME,
							GSM_UART_CALLBALL_GetSimIccidRequest,
							TRUE,
							TIMER_LOOP_FOREVER,
							ACTION_MODE_ADD_TO_QUEUE);
			*/

			// gsmģ�鿪�ػ�����
			GSM_UART_GsmPowerOnOff(TRUE);
			break;

		// д���ά������
		case IAP_CTRL_UART_CMD_DOWN_QR_DATA_WRITE:
			// ��QRд��ָ��λ�õ�EEPROM
			//E2PROM_WriteByteArray(PARAM_NVM_QR_START_ADDR, (uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX], PARAM_NVM_QR_SIZE);

			SPI_FLASH_WriteWithErase((uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX], SPI_FLASH_NEW_QR_CODE_ADDEESS, PARAM_NVM_QR_SIZE + 1);

			// �����������ж�
			SPI_FLASH_ReadArray(localBuf, SPI_FLASH_NEW_QR_CODE_ADDEESS, PARAM_NVM_QR_SIZE + 1);

			// �Ƚ�д��ĺͶ��������Ƿ�һ��
			errStatus = PARAM_CmpareN((const uint8*)(uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX], (const uint8*)localBuf, PARAM_NVM_QR_SIZE + 1);
			
			// ���ͽ��
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_QR_DATA_WRITE_RESULT, errStatus);
			break;

//==========================================================================
//==========================================================================
		// SPI ���Ե�ַ����
		case IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ERESE:
			// ��ַƫ���ֽ���
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			// �ֽ���
			temp2 = 0;
			for (i = 0; i < 4; i++)
			{
				temp2 <<= 8;
				temp2 |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX + i];
			}
			
			// ����ָ���ռ�����
			SPI_FLASH_EraseRoom(temp, temp2);
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ERESE, TRUE);
			break;

		// SPI ���Ե�ַд��
		case IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_WRITE:
			// ��ַƫ���ֽ���
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			
			// �ֽ���
			length = 0;
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// д�����ݵ�flash����
			bTemp = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], temp, length);		
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_WRITE, bTemp);
			break;

		// SPI ���Ե�ַ��ȡ
		case IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_READ:
			// ��ַƫ���ֽ���
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			
			// �ֽ���
			length = 0;
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// ���ⲿflash��ȡ�����ϱ�
			IAP_CTRL_UART_SendSpiFlashData(temp, length);
			break;

		// SPI ���Ե�ַ��д
		case IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ER_WR:
			// ��ַƫ���ֽ���
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			
			// �ֽ���
			length = 0;
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// д�����ݵ�flash����
			SPI_FLASH_WriteWithErase(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], temp, length);		
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ER_WR, TRUE);
			break;
			
		// UI���ݲ���
		case IAP_CTRL_UART_CMD_SPI_FLASH_UI_ERESE:
			// ����ָ���ռ��С
			SPI_FLASH_EraseRoom(SPI_FLASH_UI_OFFSET_ADDR, SPI_FLASH_UI_SIZE);
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_UI_ERESE, TRUE);
			break;
			
		// UI����д��
		case IAP_CTRL_UART_CMD_SPI_FLASH_UI_WRITE:
			// ��ַƫ���ֽ���
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			// �ֽ���
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// д�����ݵ�flash����
			bTemp = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], (SPI_FLASH_UI_OFFSET_ADDR + temp), length);		
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_UI_WRITE, bTemp);
			break;

		// UI д�����
		case IAP_CTRL_UART_CMD_SPI_FLASH_UI_FINISH:
			IAP_CTRL_UART_SendCmdNoResult(IAP_CTRL_UART_CMD_SPI_FLASH_UI_FINISH);
			break;

		// ����APP���ݲ���
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_ERESE:
			// ��������APP����
			SPI_FLASH_EraseRoom(SPI_FLASH_APP_FACTORY_OFFSET_ADDR, SPI_FLASH_APP_FACTORY_SIZE);
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_ERESE, TRUE);
			break;

		// ����APP����д��
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_WRITE:
			// ��ַƫ���ֽ���
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			// �ֽ���
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// д�����ݵ�flash����
			bTemp = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], (SPI_FLASH_APP_FACTORY_OFFSET_ADDR + temp), length);				
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_WRITE, bTemp);
			break;

		// ����APP����д�����
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_FINISH:	
			IAP_CTRL_UART_SendCmdNoResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_FINISH);
			break;

		// ����APP���ݲ���
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_ERESE:
			// �����ļ�ϵͳ
			SPI_FLASH_EraseRoom(SPI_FLASH_APP_UPDATE_OFFSET_ADDR, SPI_FLASH_APP_UPDATE_SIZE);
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_ERESE, TRUE);
			break;

		// ����APP����д��
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_WRITE:
			// ��ַƫ���ֽ���
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			// �ֽ���
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// д�����ݵ�flash����
			bTemp = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], (SPI_FLASH_APP_UPDATE_OFFSET_ADDR + temp), length);		
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_WRITE, bTemp);
			break;

		// ����APP����д�����
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_FINISH:	
			IAP_CTRL_UART_SendCmdNoResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_FINISH);
			break;

		case IAP_CTRL_UART_CMD_SPI_FLASH_WRITE_SPI_TO_MCU:
			// �Ȳ���APP������
			IAP_EraseAPP2Area();

#define WR_APP_BYTES_ONE_TIME		128	
			// APP����д��
			updateBuff = (uint8 *)malloc(WR_APP_BYTES_ONE_TIME);
			if (NULL == updateBuff)
			{
				return;
			}
			
			if (1 == pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX])
			{
				temp = SPI_FLASH_APP_FACTORY_OFFSET_ADDR;
			}
			else if(2 == pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX])
			{
				temp = SPI_FLASH_APP_UPDATE_OFFSET_ADDR;
			}
			else
			{
				break;
			}

			// ����IAP_WriteAppBin()����һ��ֻ��д252�����ݣ���˶�ȡ��д��ֻ��С�ڵ���252
			for (i = 0; i < SPI_FLASH_APP_UPDATE_SIZE/WR_APP_BYTES_ONE_TIME; i++)
			{
				SPI_FLASH_ReadArray(updateBuff, (temp + i * WR_APP_BYTES_ONE_TIME), WR_APP_BYTES_ONE_TIME);
				IAP_WriteAppBin((IAP_FLASH_APP2_ADDR + i * WR_APP_BYTES_ONE_TIME), updateBuff, WR_APP_BYTES_ONE_TIME);
			}
#undef WR_APP_BYTES_ONE_TIME

			// �ͷŸ��ڴ�
			free(updateBuff);
			
			// ���������ʱ�ظ�����ת
			TIMER_AddTask(TIMER_ID_ECO_JUMP,
							200,
							IAP_JumpToAppFun,
							IAP_FLASH_ADRESS_APP2_OFFSET,
							1,
							ACTION_MODE_ADD_TO_QUEUE);

			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_WRITE_SPI_TO_MCU, TRUE);
			break;

		case IAP_CTRL_UART_CMD_JUMP_TO_BOOT:			
			TIMER_AddTask(TIMER_ID_ECO_JUMP,
							200,
							SYSTEM_Rst,
							temp,
							1,
							ACTION_MODE_ADD_TO_QUEUE);
							
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_JUMP_TO_BOOT, TRUE);
			
			break;
			
		// ��������ǿ�ƿ���BOOT��ת��APP1
		case IAP_CTRL_UART_CMD_JUMP_TO_APP1:
			temp = IAP_FLASH_ADRESS_APP1_OFFSET;
			
			TIMER_AddTask(TIMER_ID_ECO_JUMP,
							200,
							IAP_JumpToAppFun,
							temp,
							1,
							ACTION_MODE_ADD_TO_QUEUE);
							
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_JUMP_TO_APP1, IAP_CheckAppRightful(temp));
			break;

		// ��������ǿ�ƿ���BOOT��ת��APP2
		case IAP_CTRL_UART_CMD_JUMP_TO_APP2:
			temp = IAP_FLASH_ADRESS_APP2_OFFSET;
			
			TIMER_AddTask(TIMER_ID_ECO_JUMP,
							200,
							IAP_JumpToAppFun,
							temp,
							1,
							ACTION_MODE_ADD_TO_QUEUE);
							
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_JUMP_TO_APP2, IAP_CheckAppRightful(temp));
			break;

		// �汾�������
		case IAP_CTRL_UART_CMD_CHECK_VERSION:
			if(IAP_CTRL_UART_CHECK_VERSION_ALL == pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX])
			{			
				checkFlag = 0;
				// ���������ʱ�ظ�����ת
				TIMER_AddTask(TIMER_ID_ECO_JUMP,
								200,
								checkVersion,
								0,
								8,
								ACTION_MODE_ADD_TO_QUEUE);
			}
			else
			{
				IAP_CTRL_UART_SendVersionCheck(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX]);
			}							
			break;

		case IAP_CTRL_UART_CMD_FLAG_ARRAY_READ:
			IAP_CTRL_UART_SendFlagCheck(1);
			break;

		case IAP_CTRL_UART_CMD_FLAG_ARRAY_WRITE:

			// д���־λ��
			temp = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];

			// д�����ݵ�spiflash
			SPI_FLASH_WriteWithErase(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX], SPI_FLASH_TEST_FLAG_ADDEESS + temp*4, 4);

			// ���±�־
			PARAM_Init();

			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_FLAG_ARRAY_WRITE, TRUE);
			break;

		case IAP_CTRL_UART_CMD_VERSION_TYPE_WRITE:
			// ��һ��д������
			temp = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];
			
			// �ڶ���Ϊ��Ϣ����
			temp2 = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX];

			updateBuff = (uint8 *)malloc(128);
			if (NULL == updateBuff)
			{
				break;
			}
			
			// ����
			for (i = 0; i < 128; i++)
			{
				updateBuff[i] = 0;
			}
			// ������Ϣ
			for (i = 0; i < temp2 + 1; i++)
			{
				updateBuff[i] = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX+1+i];
			}
			
			// �ж����������Ƿ�Ϸ�
			if (16 >= temp)
			{
				SPI_FLASH_WriteWithErase(updateBuff, PARAM_MCU_VERSION_ADDRESS[temp], temp2+1);
				PARAM_Init();

				bTemp = TRUE;
			}
			else
			{
				bTemp = FALSE;
			}

			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_VERSION_TYPE_WRITE);
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, temp);	// ����
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, bTemp);
			IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
			break;

		case IAP_CTRL_UART_CMD_VERSION_TYPE_READ:
			temp = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];
			
			updateBuff = (uint8 *)malloc(128);
			if (NULL == updateBuff)
			{
				break;
			}
			// ����
			for (i = 0; i < 128; i++)
			{
				updateBuff[i] = 0;
			}

			// ��ȡ����
			SPI_FLASH_ReadArray(updateBuff, PARAM_MCU_VERSION_ADDRESS[temp], 64+1);

			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_VERSION_TYPE_READ);
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, temp);	// ����

			// ���������в�ͬ�ĳ���
			switch (temp)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 8:
					if ((updateBuff[0] > 0) && (updateBuff[0] <= 64))
					{
						// ��Ϣ����
						IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[0]);

						// ������Ϣ
						for (i = 0; i < updateBuff[0]; i++)
						{
							IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[i+1]);
						}
					}
					else
					{
						// ��Ϣ����
						IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);
					}
					break;

				case 5:
				case 6:
					// ��Ϣ����
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 4);
					
					// ������Ϣ�����ֽ���ǰ�����ֽ��ں�
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[1]);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[2]);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[3]);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[4]);
					break;
					
				case 7:
					// ��Ϣ����
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 32);

					// ������Ϣ
					for (i = 0; i < 32; i++)
					{
						IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[i+1]);
					}
					break;

				default:
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);
					break;
			}
			IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);

			free(updateBuff);
			break;

		// ��������
		case IAP_CTRL_UART_CMD_KEY_TESTING:
		
			break;

		// ��ʾ����
		case IAP_CTRL_UART_CMD_TEST_LCD:
			// ���ȺϷ�
			if (1 == pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX])
			{
				IAP_CTRL_UART_LcdControl(pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX]);
			}
			else
			{
				IAP_CTRL_UART_LcdControl(0);
			}
			IAP_CTRL_UART_SendCmdNoResult(IAP_CTRL_UART_CMD_TEST_LCD);
			break;
		
		default:
			break;
	}
	
	// ɾ������֡
	pCB->rx.head ++;
	pCB->rx.head %= IAP_CTRL_UART_RX_QUEUE_SIZE;
}




// ������ʾ����
uint8 IAP_CTRL_UART_LcdControl(uint8 lcdCmd)
{
	switch (lcdCmd)
	{
		case 0:
			gpio_bit_set(GPIOA, GPIO_PIN_8);
			//LCD_FillColor(LCD_COLOR_RED);
			
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			gpio_bit_set(GPIOA, GPIO_PIN_8);
		
			break;

		case 7:
			gpio_bit_reset(GPIOA, GPIO_PIN_8);
			break;

		default:		
			break;
	}
	return 0;
}
			

// ��������֡�������������
void IAP_CTRL_UART_TxAddData(IAP_CTRL_UART_CB* pCB, uint8 data)
{
	IAP_CTRL_CMD_FRAME* pCmdFrame = &pCB->tx.cmdQueue[pCB->tx.end];
	uint16 head = pCB->tx.head;
	uint16 end =  pCB->tx.end;	// 2015.12.29

	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}

	// ���ͻ������������������
	if((end + 1) % IAP_CTRL_UART_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ��β����֡�������˳�
	if(pCmdFrame->length >= IAP_CTRL_UART_CMD_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void IAP_CTRL_UART_TxAddFrame(IAP_CTRL_UART_CB* pCB)
{
	uint8 cc = 0;
	uint16 i = 0;
	IAP_CTRL_CMD_FRAME* pCmdFrame = &pCB->tx.cmdQueue[pCB->tx.end];
	uint16 length = pCmdFrame->length;
	uint16 head = pCB->tx.head;
	uint16 end =  pCB->tx.end;	// 2015.12.29

	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}

	// ���ͻ������������������
	if((end + 1) % IAP_CTRL_UART_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ����֡���Ȳ��㣬������������ݣ��˳�
	if(IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN-1 > length)//2016.1.5�޸ļ�1
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] = length - 3;	// �������ݳ���
	for(i=0; i<length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[length] = ~cc;
	pCmdFrame->length ++;

	pCB->tx.end ++;
	pCB->tx.end %= IAP_CTRL_UART_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�

	// ������ʽʱ��������
#if (IAP_CTRL_UART_TX_MODE == IAP_CTRL_BLOCKING_TX_MODE)
	IAP_CTRL_UART_BC_StartTx(pCB);
#endif

}

// UARTģ�鴦�����
void IAP_CTRL_UART_Process(void)
{
	// �ڲ�ͨѶ�ӿڵ�һ�����ջ���������
	IAP_CTRL_UART_RxFIFOProcess(&iapCtrlUartCB);

	// �ڲ�ͨѶ�ӿڵ�����֡����������
	IAP_CTRL_UART_CmdFrameProcess(&iapCtrlUartCB);
	
	//�жϷ��ͷ�ʽ
#if (IAP_CTRL_UART_TX_MODE == IAP_CTRL_INTERRUPT_TX_MODE)
	// �ڲ�ͨѶ�ӿڵķ��ʹ���
	IAP_CTRL_UART_TxProcess(&iapCtrlUartCB);
#endif

}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ���ڽ��պ��������ڵ��жϷ��ͺ���
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
void USART1_IRQHandler(void)
{
	// �ж�DR�Ƿ������ݣ��жϽ���
	if(usart_interrupt_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET) 	 
	{
		uint16 end = iapCtrlUartCB.rxFIFO.end;
		uint16 head = iapCtrlUartCB.rxFIFO.head;
		uint8 rxdata = 0x00;
		
		// ��������
		rxdata = (uint8)usart_data_receive(IAP_CTRL_UART_TYPE_DEF);

		// һ���������������������
		if((end + 1)%IAP_CTRL_UART_RX_FIFO_SIZE == head)
		{
			return;
		}
		// һ��������δ�������� 
		else
		{
			// �����յ������ݷŵ���ʱ��������
			iapCtrlUartCB.rxFIFO.buff[end] = rxdata;
			iapCtrlUartCB.rxFIFO.end ++;
			iapCtrlUartCB.rxFIFO.end %= IAP_CTRL_UART_RX_FIFO_SIZE;
		}	

	}

// �ж�ģʽ����
#if (IAP_CTRL_UART_TX_MODE == IAP_CTRL_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if(usart_interrupt_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = iapCtrlUartCB.tx.head;
		uint16 end;
		uint16 index = iapCtrlUartCB.tx.index;
		uint8 txdata = 0x00;

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if(index < iapCtrlUartCB.tx.cmdQueue[head].length)
		{
			txdata = iapCtrlUartCB.tx.cmdQueue[head].buff[iapCtrlUartCB.tx.index++];
			
			// �������
			usart_data_transmit(IAP_CTRL_UART_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			iapCtrlUartCB.tx.cmdQueue[head].length = 0;
			iapCtrlUartCB.tx.head ++;
			iapCtrlUartCB.tx.head %= IAP_CTRL_UART_TX_QUEUE_SIZE;
			iapCtrlUartCB.tx.index = 0;

			head = iapCtrlUartCB.tx.head;
			end = iapCtrlUartCB.tx.end;
			
			// ����֡���зǿգ�����������һ������֡
			if(head != end)
			{
				txdata = iapCtrlUartCB.tx.cmdQueue[head].buff[iapCtrlUartCB.tx.index++];

				// �������
				usart_data_transmit(IAP_CTRL_UART_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ��Ϳ��ж�
				usart_interrupt_disable(IAP_CTRL_UART_TYPE_DEF, USART_INT_TC);
				
				iapCtrlUartCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif

	// Other USARTx interrupts handler can go here ...				 
	if(usart_interrupt_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- ��������ж� 	
	{
		// �û��ֲ� 434  --- ����ȶ�USART_STAT0���ٶ�USART_DATA�������λ
		usart_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_FLAG_ORERR);		
		usart_data_receive(IAP_CTRL_UART_TYPE_DEF);						  //----------------------- ��ռĴ���
	}
}

#if (IAP_CTRL_UART_TX_MODE == IAP_CTRL_INTERRUPT_TX_MODE)
// �����ж��ֽڷ���
void IAP_CTRL_UART_IR_StartSendData(uint8 data)
{
	// �ȶ�SR�������DR���TC��־���
	usart_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_FLAG_TC);

	// ����һ���ֽ�
	usart_data_transmit(IAP_CTRL_UART_TYPE_DEF, data);

	// �򿪷�������ж�
	usart_interrupt_enable(IAP_CTRL_UART_TYPE_DEF, USART_INT_TC);	
}

#else
// ��������һ���ֽ�����
void IAP_CTRL_UART_BC_SendData(uint8 data)
{	
	// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�) 
	usart_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_FLAG_TC);		

	// �������
	usart_data_transmit(IAP_CTRL_UART_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while(usart_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_FLAG_TC) != SET);
}

// ������������
void IAP_CTRL_UART_BC_StartTx(IAP_CTRL_UART_CB* pCB)
{
	uint16 index = pCB->tx.index;
	uint16 head = pCB->tx.head;

	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}
	
	// ��ǰ����֡δ������ʱ����������
	while(index < pCB->tx.cmdQueue[head].length)
	{
		// һֱ��䷢��
		IAP_CTRL_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= IAP_CTRL_UART_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
}
#endif

// ���ʹ���,���ַ��ͻ������ǿ�ʱ,�����жϷ���
void IAP_CTRL_UART_TxProcess(IAP_CTRL_UART_CB* pCB)
{
	uint16 index = pCB->tx.index;							// ��ǰ�������ݵ�������
	uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length;	// ��ǰ���͵�����֡�ĳ���
	uint16 head = pCB->tx.head;								// ��������֡����ͷ������
	uint16 end = pCB->tx.end;								// ��������֡����β������

	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}
	
	// ����Ϊ�գ�������
	if(head == end)
	{
		return;
	}

	// ����ִ�е����˵�����зǿա���
	
	// ��ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
	if(index < length)
	{
		// ����æ���˳�
		if(pCB->tx.txBusy)
		{
			return;
		}
		
		IAP_CTRL_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		// ���÷���æ״̬
		pCB->tx.txBusy = TRUE;

	}
	// ��ǰ����֡������ʱ��ɾ��֮
	else
	{
	//	pCB->tx.head ++;
	//	pCB->tx.head %= IAP_CTRL_UART_TX_QUEUE_SIZE;
	//	pCB->tx.index = 0;
	}
}

//===============================================================================
// ��ϸ�������
// ==============================================================================
// ����ģʽ����
void IAP_CTRL_UART_SendCmdProjectApply(uint32 para)
{
	uint8 param[] = IAP_VERSION;
	uint8 len = strlen((char*)param);
	uint8 i;
	
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_PROJECT_APPLY);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, len);		// ���ݳ���

	for (i = 0; i < len; i++)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param[i]);
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ����ģʽ׼����������
void IAP_CTRL_UART_SendCmdProjectReady(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_PROJECT_READY);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ����FLASH����ϱ�
void IAP_CTRL_UART_SendCmdEraseFlashResult(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_IAP_ERASE_FLASH_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// IAP����д�����ϱ�
void IAP_CTRL_UART_SendCmdWriteFlashResult(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_IAP_WRITE_FLASH_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ��ս���ϱ�
void IAP_CTRL_UART_SendCmdCheckFlashBlankResult(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_CHECK_FLASH_BLANK_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ϵͳ��������ȷ��
void IAP_CTRL_UART_SendCmdUpdataFinishResult(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_UPDATA_FINISH_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);		// ���ݳ���
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ����UI���ݲ������
void IAP_CTRL_UART_SendCmdUIEraseResultReport(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_UI_DATA_ERASE_ACK);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ����UI����д����
void IAP_CTRL_UART_SendCmdUIWriteDataResultReport(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_UI_DATA_WRITE_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ��ά��ע������ϱ�
void IAP_CTRL_UART_SendCmdQrRegParam(uint8 *param)
{
	uint8 len = strlen((char*)param);
	uint8 i;
	
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_QR_PARAM_REPORT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, len);		// ���ݳ���

	for (i = 0; i < len; i++)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param[i]);
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ����д���ά�����ݽ��
void IAP_CTRL_UART_SendCmdQrWriteResultReport(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_QR_DATA_WRITE_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}


// ����ָ���˶Եİ汾
void IAP_CTRL_UART_SendVersionCheck(uint32 checkNum)
{
	uint8 *buff;
	uint8 i;
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_CHECK_VERSION);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);				// ���ݳ���

	// ���ݰ汾�ţ�ѡ���͵İ汾
	switch ((IAP_CTRL_UART_CHECK_VERSION_E)checkNum)
	{
		// APP
		case IAP_CTRL_UART_CHECK_VERSION_APP:
			buff = paramCB.runtime.appVersion;
			break;

		// BOOT
		case IAP_CTRL_UART_CHECK_VERSION_BOOT:
			buff = paramCB.runtime.bootVersion;
			break;

		// UI
		case IAP_CTRL_UART_CHECK_VERSION_UI:
			buff = paramCB.runtime.uiVersion;
			break;
			
		// Ӳ���汾
		case IAP_CTRL_UART_CHECK_VERSION_HW:
			buff = paramCB.runtime.hardVersion;
			break;
			
		// SN�汾
		case IAP_CTRL_UART_CHECK_VERSION_SN_NUM:
			buff = paramCB.runtime.snCode;
			break;
			
		// ����MAC
		case IAP_CTRL_UART_CHECK_VERSION_BLE_MAC:
			buff = paramCB.runtime.bluMac;
			break;
			
		// ��ά���ַ�
		case IAP_CTRL_UART_CHECK_VERSION_QR_CODE:
			buff = paramCB.runtime.qrCodeStr;
			break;

		default:
			buff = NULL;
			break;
	
	}

	// �ж�ָ���Ƿ�Ϸ�
	if (NULL == buff)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, checkNum & 0xFF);		// �ȶԺ�
	}
	else
	{		
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, checkNum & 0xFF);		// �ȶԺ�
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, buff[0]);		// �汾����

		for (i = 0; i < buff[0]; i++)
		{
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, buff[1 + i]);		// �汾�ַ�����
		}
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

void IAP_CTRL_UART_SendFlagCheck(uint32 checkNum)
{
	uint8 *buff;
	uint8 i;
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_FLAG_ARRAY_READ);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);				// ���ݳ���

	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 64);			// ��־������

	buff = paramCB.runtime.flagArr;
	for (i = 0; i < 64; i++)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, buff[i]);		// �汾�ַ�����
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}


// ������������
void IAP_CTRL_UART_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, cmdWord);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// ���ݳ���
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, result);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ���������޽��
void IAP_CTRL_UART_SendCmdNoResult(uint8 cmdWord)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, cmdWord);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);		// ���ݳ���
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// ��ȡ�����ϱ�
void IAP_CTRL_UART_SendSpiFlashData(uint32 ReadAddr, uint8 NumByteToRead)
{
	uint16 i;
	uint8* pFlashBuff = NULL;				// �����ȡ����������

	pFlashBuff = (uint8 *)malloc(NumByteToRead);
	if (NULL == pFlashBuff)
	{
		return;
	}
	
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_READ);
	
	// �������ݳ��ȣ�������д����
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, (128+4));

	// ���Ͷ�ȡ�ĵ�ַ
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, ((ReadAddr & 0xFF000000) >> 24));
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, ((ReadAddr & 0x00FF0000) >> 16));
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, ((ReadAddr & 0x0000FF00) >> 8));
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, ((ReadAddr & 0x000000FF) >> 0));

	// ���Ͷ�ȡ�����ֽ���
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, (NumByteToRead & 0xFF));

	// ���ⲿspiflash��ȡ����
	SPI_FLASH_ReadArray(pFlashBuff, ReadAddr, NumByteToRead);
	// ������ݵ�����֡������
	for (i = 0; i < NumByteToRead; i++)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, pFlashBuff[i]);
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);

	free(pFlashBuff);
}


#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
// RXͨѶ��ʱ����-����
void IAP_CTRL_UART_CALLBACK_RxTimeOut(uint32 param)
{
	IAP_CTRL_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &iapCtrlUartCB.rx.cmdQueue[iapCtrlUartCB.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0;	// 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	iapCtrlUartCB.rxFIFO.head ++;
	iapCtrlUartCB.rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
	iapCtrlUartCB.rxFIFO.currentProcessIndex = iapCtrlUartCB.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void IAP_CTRL_UART_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

#if IAP_CTRL_UART_TXRX_TIME_OUT_CHECK_ENABLE
// TXRXͨѶ��ʱ����-˫��
void IAP_CTRL_UART_CALLBACK_TxRxTimeOut(uint32 param)
{
	
}

// ֹͣTxRXͨѶ��ʱ�������
void IAP_CTRL_UART_StopTxRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_TXRX_TIME_OUT_CONTROL);
}
#endif

// ��������MCU��Ψһ���кţ���ֹ�����ϻ�
void UART_PROTOCOL_SendMcuUid(uint32 param)
{
	uint8 buff[UART_PROTOCOL_MCU_UID_BUFF_LENGTH];
	uint8 i;

	// ��λ����sn0
	buff[0] = iapCB.mcuUID.sn0 & 0xff;
	buff[1] = (iapCB.mcuUID.sn0 >> 8)  & 0xff;
	buff[2] = (iapCB.mcuUID.sn0 >> 16) & 0xff;
	buff[3] = (iapCB.mcuUID.sn0 >> 24) & 0xff;

	// ��λ����sn1
	buff[4] = iapCB.mcuUID.sn1 & 0xff;
	buff[5] = (iapCB.mcuUID.sn1 >> 8)  & 0xff;
	buff[6] = (iapCB.mcuUID.sn1 >> 16) & 0xff;
	buff[7] = (iapCB.mcuUID.sn1 >> 24) & 0xff;

	// ��λ����sn2
	buff[8]  = iapCB.mcuUID.sn2 & 0xff;
	buff[9]  = (iapCB.mcuUID.sn2 >> 8)  & 0xff;
	buff[10] = (iapCB.mcuUID.sn2 >> 16) & 0xff;
	buff[11] = (iapCB.mcuUID.sn2 >> 24) & 0xff;

	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	
	// MCU_UID
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, UART_ECO_CMD_ECO_MCU_UID);	
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0x00);	
	
	for (i = 0; i < UART_PROTOCOL_MCU_UID_BUFF_LENGTH; i++)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, buff[i]);
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}


