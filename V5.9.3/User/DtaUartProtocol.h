/********************************************************************************************************
		KM5S�������ͨѶЭ��
********************************************************************************************************/
#ifndef 	__DTA_UART_PROTOCOL_H__
#define 	__DTA_UART_PROTOCOL_H__

#include "common.h"

//=====================================================================================================
#define DTA_UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define DTA_UART_PROTOCOL_RX_QUEUE_SIZE					5		// ����������гߴ�
#define DTA_UART_PROTOCOL_TX_QUEUE_SIZE					5		// ����������гߴ�

#define DTA_UART_PROTOCOL_CMD_HEAD						0x55	// ����ͷ

#define DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		150		// ���������֡����
#define DTA_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		150		// ����������֡����
#define DTA_UART_PROTOCOL_RX_FIFO_SIZE					200		// ����һ����������С
#define DTA_UART_PROTOCOL_CMD_FRAME_LENGTH_MIN			4		// ����֡��С���ȣ�����:����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define DTA_UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define DTA_UART_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// UART����Э�鶨��	
typedef enum
{
	DTA_UART_PROTOCOL_CMD_HEAD_INDEX = 0,					// ֡ͷ����
	DTA_UART_PROTOCOL_CMD_CMD_INDEX, 						// ����������
	DTA_UART_PROTOCOL_CMD_LENGTH_INDEX,						// ���ݳ���

	DTA_UART_PROTOCOL_CMD_DATA1_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA2_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA3_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA4_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA5_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA6_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA7_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA8_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA9_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA10_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA11_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA12_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA13_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA14_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA15_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA16_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA17_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA18_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA19_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA20_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA21_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA22_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA23_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA24_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA25_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA26_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA27_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA28_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA29_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA30_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA31_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA32_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA33_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA34_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA35_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA36_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA37_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA38_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA39_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA40_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA41_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA42_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA43_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA44_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA45_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA46_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA47_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA48_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA49_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA50_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA51_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA52_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA53_INDEX,						// ��������
	DTA_UART_PROTOCOL_CMD_DATA54_INDEX,						// ��������

	DTA_UART_PROTOCOL_CMD_INDEX_MAX
}DTA_UART_PROTOCOL_DATE_FRAME;

// ��������
typedef enum{

    DTA_UART_PROTOCOL_CMD_NULL,
    
	DTA_UART_CMD_DUT_BOOT_ERASE_FLASH = 0xD0,					// DUT_BOOT����

	DTA_UART_CMD_DUT_BOOT_WRITE_FLASH = 0xD1,					// DUT_BOOTд��

	DTA_UART_CMD_DUT_UPDATA_FINISH = 0xD2,                      // DUT_BOOTд�����

	DTA_UART_PROTOCOL_CMD_MAX									// ��������
}DTA_UART_PROTOCOL_CMD;

// UART_RX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[DTA_UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}DTA_UART_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[DTA_UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}DTA_UART_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8	buff[DTA_UART_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;					// ��ǰ��������ֽڵ�λ���±�
	}rxFIFO;
	
	// ����֡���������ݽṹ
	struct{
		DTA_UART_PROTOCOL_RX_CMD_FRAME	cmdQueue[DTA_UART_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;						// ����ͷ����
		uint16			end;						// ����β����
	}rx;

	// ����֡���������ݽṹ
	struct{
		DTA_UART_PROTOCOL_TX_CMD_FRAME	cmdQueue[DTA_UART_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;						// ����ͷ����
		uint16	end;						// ����β����
		uint16	index;						// ��ǰ����������������֡�е�������
		BOOL	txBusy;						// ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
	}tx;

	// �������ݽӿ�
	BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	BOOL txPeriodRequest;		// ��������Է�����������
	BOOL txAtOnceRequest;		// ������Ϸ�����������

	BOOL paramSetOK;
}DTA_UART_PROTOCOL_CB;

extern DTA_UART_PROTOCOL_CB dtaUartProtocolCB;		

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void DTA_UART_PROTOCOL_Init(void);

// Э�����̴���
void DTA_UART_PROTOCOL_Process(void);

// ��������֡�������������
void DTA_UART_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void DTA_UART_PROTOCOL_TxAddFrame(void);


// ��������ظ�����һ������
void DTA_UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// ���������޽��
void DTA_UART_PROTOCOL_SendCmdNoResult(uint8 cmdWord);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void DTA_UART_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// �������ݰ�
void dtaUartProtocol_SendOnePacket(uint32 flashAddr,uint32 addr);

// �������ݰ���HEX��
void dtaUartProtocol_SendOnePacket_Hex(uint32 flashAddr);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void DTA_UART_PROTOCOL_RxFIFOProcess(DTA_UART_PROTOCOL_CB* pCB);

// UART����֡����������
void DTA_UART_PROTOCOL_CmdFrameProcess(DTA_UART_PROTOCOL_CB* pCB);

// Э��㷢�ʹ������
void DTA_UART_PROTOCOL_TxStateProcess(void);

#endif

