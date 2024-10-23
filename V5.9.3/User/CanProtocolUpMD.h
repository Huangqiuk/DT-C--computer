#ifndef 	__CAN_MD_PROTOCOL_H__
#define 	__CAN_MD_PROTOCOL_H__

#include "common.h"
#include "DutInfo.h"

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define CAN_MD_PROTOCOL_COMMUNICATION_TIME_OUT		10000UL	// ��λ:ms

//=====================================================================================================
#define CAN_MD_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define CAN_MD_PROTOCOL_RX_QUEUE_SIZE					3		// ����������гߴ�
#define CAN_MD_PROTOCOL_TX_QUEUE_SIZE					3		// ����������гߴ�

#define CAN_MD_PROTOCOL_CMD_HEAD						0x55	// ����ͷ
#define CAN_MD_PROTOCOL_CMD_HEAD2						0xAA	// ����ͷ

#define CAN_MD_PROTOCOL_CMD_TAIL			       	    0xF0	// ֡β
#define CAN_MD_PROTOCOL_CMD_NONHEAD 					0xFF	// ������ͷ
#define CAN_MD_PROTOCOL_CMD_CHECK_BYTE_SIZE 			0x04	// У������ռ�ֽ���
#define CAN_MD_PROTOCOL_CMD_TAIL_BYTE 					0x01	// ֡β�ֽ���

#define CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		160		// ���������֡����
#define CAN_MD_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		160		// ����������֡����
#define CAN_MD_PROTOCOL_RX_FIFO_SIZE					180		// ����һ����������С
#define CAN_MD_PROTOCOL_CMD_FRAME_LENGTH_MIN			9		// ����֡��С���ȣ�����:��ʼ���������֡����ݳ��ȡ�У���


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define CAN_MD_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define CAN_MD_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// CAN����Э�鶨��	
typedef enum
{
	CAN_MD_PROTOCOL_CMD_HEAD_INDEX = 0,					    // ֡ͷ����
	CAN_MD_PROTOCOL_CMD_HEAD_INDEX2,				        // ֡ͷ���� 
	CAN_MD_PROTOCOL_CMD_MODE_INDEX, 						// ֡ģʽ
	CAN_MD_PROTOCOL_CMD_LENGTH_INDEX,						// ���ݳ���
	CAN_MD_PROTOCOL_CMD_CMD_H_INDEX,						// ����������
	CAN_MD_PROTOCOL_CMD_CMD_L_INDEX,						// ����������
    
	CAN_MD_PROTOCOL_CMD_DATA1_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA2_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA3_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA4_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA5_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA6_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA7_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA8_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA9_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA10_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA11_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA12_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA13_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA14_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA15_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA16_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA17_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA18_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA19_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA20_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA21_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA22_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA23_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA24_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA25_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA26_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA27_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA28_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA29_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA30_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA31_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA32_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA33_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA34_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA35_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA36_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA37_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA38_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA39_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA40_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA41_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA42_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA43_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA44_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA45_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA46_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA47_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA48_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA49_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA50_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA51_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA52_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA53_INDEX,						// ��������
	CAN_MD_PROTOCOL_CMD_DATA54_INDEX,						// ��������

	CAN_MD_PROTOCOL_CMD_INDEX_MAX
}CAN_MD_PROTOCOL_DATE_FRAME;

// CAN���߽ڵ���
typedef enum{
	CAN_MD_PROTOCOL_MODE_READ = 0x11,					// ��ģʽ 
	CAN_MD_PROTOCOL_MODE_WRITE = 0x16,					// дģʽ 
	CAN_MD_PROTOCOL_MODE_ACK = 0x0C,					// �ϱ�ģʽ 

	CAN_MD_PROTOCOL_MODE_MAX							// ���ڵ���
}CAN_MD_PROTOCOL_MODE;

// ��������
typedef enum{
	CAN_MD_PROTOCOL_CMD_NULL = 0,								// ������

	CAN_MD_PROTOCOL_CMD_UP_PROJECT_APPLY = 0xC109,						// ����ģʽ��������
	CAN_MD_PROTOCOL_CMD_DOWN_PROJECT_APPLY_ACK = 0xA10B,				// ����ģʽ��׼Ӧ�� 
	CAN_MD_PROTOCOL_CMD_UP_PROJECT_READY = 0xC402,						// ����ģʽ׼������
	
	CAN_MD_PROTOCOL_CMD_DOWN_IAP_WRITE_FLASH = 0xA385, 					// IAP����д��
	CAN_MD_PROTOCOL_CMD_UP_IAP_WRITE_RIGHT_RESULT = 0xC202, 			// IAP����д�����ϱ�,��ȷӦ��
	CAN_MD_PROTOCOL_CMD_UP_IAP_WRITE_ERROR_RESULT = 0xC302, 			// IAP����д�����ϱ�������Ӧ��
	CAN_MD_PROTOCOL_CMD_DOWN_UPDATA_FINISH = 0xA401, 					// ϵͳ��������
	CAN_MD_PROTOCOL_CMD_UP_UPDATA_FINISH_RESULT = 0xC502, 				// ϵͳ��������ȷ��	

	CAN_MD_PROTOCOL_CMD_MAX									// ��������
}CAN_MD_PROTOCOL_CMD;

// UART_RX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[CAN_MD_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										    // ����֡��Ч���ݸ���
}CAN_MD_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[CAN_MD_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}CAN_MD_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8	buff[CAN_MD_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
        volatile uint16	deviceID;
		uint16 currentProcessIndex;					// ��ǰ��������ֽڵ�λ���±�
	}rxFIFO;
	
	// ����֡���������ݽṹ
	struct{
		CAN_MD_PROTOCOL_RX_CMD_FRAME	cmdQueue[CAN_MD_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;						// ����ͷ����
		uint16			end;						// ����β����
	}rx;

	// ����֡���������ݽṹ
	struct{
		CAN_MD_PROTOCOL_TX_CMD_FRAME	cmdQueue[CAN_MD_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;						// ����ͷ����
		uint16	end;						// ����β����
		uint16	index;						// ��ǰ����������������֡�е�������
		BOOL	txBusy;						// ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
	}tx;

	// �������ݽӿ�
	BOOL (*sendDataThrowService)(uint32 id, uint8 *pData, uint8 length);
	
}CAN_MD_PROTOCOL_CB;

extern CAN_MD_PROTOCOL_CB canMDProtocolCB;	

//=====�ⲿ���ú�������=======================================================================
// Э���ʼ��
void CAN_MD_PROTOCOL_Init(void);

// CAN������̺������� main �д�ѭ���ڵ���
void CAN_PROTOCOL_Process_DT(void);
	
// ���ͻ��������������
void CAN_MD_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void CAN_MD_PROTOCOL_TxAddFrame(void);

//============================================================================================

// ���ͻ����������һ������������
BOOL CAN_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length);

// ������������
void CAN_MD_PROTOCOL_SendCmdWithResult(uint16 cmdWord, uint8 result);

// ���������޽��
void CAN_MD_PROTOCOL_SendCmdNoResult(uint16 cmdWord);

// ������������
void CAN_MD_PROTOCOL_SendUpDataPacket(DUT_FILE_TYPE upDataType, CAN_MD_PROTOCOL_CMD cmd, uint32 falshAddr, uint32 addr);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_MD_PROTOCOL_MacProcess(uint32 standarID, uint8* pData, uint16 length);

// UARTЭ�����̴���
void CAN_MD_PROTOCOL_Process(void);
#endif


