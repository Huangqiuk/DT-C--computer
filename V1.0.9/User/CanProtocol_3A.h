/********************************************************************************************************
        KM5S�������ͨѶЭ��
********************************************************************************************************/
#ifndef __CAN_PROTOCOL1_H__
#define __CAN_PROTOCOL1_H__

#include "common.h"

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define CAN_PROTOCOL1_COMMUNICATION_TIME_OUT 10000UL // ��λ:ms

//=====================================================================================================
#define CAN_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE 0 // ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define CAN_PROTOCOL1_RX_QUEUE_SIZE 5 // ����������гߴ�
#define CAN_PROTOCOL1_TX_QUEUE_SIZE 5 // ����������гߴ�

#define CAN_PROTOCOL1_CMD_HEAD 0x3A        // ����ͷ
#define CAN_PROTOCOL1_CMD_DEVICE_ADDR 0x1A // �豸��ַ

#define CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX 150 // ���������֡����
#define CAN_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX 150 // ����������֡����
#define CAN_PROTOCOL1_RX_FIFO_SIZE 200            // ����һ����������С
#define CAN_PROTOCOL1_CMD_FRAME_LENGTH_MIN 8      // ����֡��С���ȣ�����:����ͷ���豸��ַ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA

// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define CAN_PROTOCOL1_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / UART_DRIVE_BAUD_RATE * 10 * CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)      // ��λ:ms
#define CAN_PROTOCOL1_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / UART_DRIVE_BAUD_RATE * 10 * CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // ��λ:ms

// UART����Э�鶨��
typedef enum
{
    CAN_PROTOCOL1_CMD_HEAD_INDEX = 0,    // ֡ͷ����
    CAN_PROTOCOL1_CMD_DEVICE_ADDR_INDEX, // �豸��ַ
    CAN_PROTOCOL1_CMD_CMD_INDEX,         // ����������
    CAN_PROTOCOL1_CMD_LENGTH_INDEX,      // ���ݳ���

    CAN_PROTOCOL1_CMD_DATA1_INDEX,  // ��������
    CAN_PROTOCOL1_CMD_DATA2_INDEX,  // ��������
    CAN_PROTOCOL1_CMD_DATA3_INDEX,  // ��������
    CAN_PROTOCOL1_CMD_DATA4_INDEX,  // ��������
    CAN_PROTOCOL1_CMD_DATA5_INDEX,  // ��������
    CAN_PROTOCOL1_CMD_DATA6_INDEX,  // ��������
    CAN_PROTOCOL1_CMD_DATA7_INDEX,  // ��������
    CAN_PROTOCOL1_CMD_DATA8_INDEX,  // ��������
    CAN_PROTOCOL1_CMD_DATA9_INDEX,  // ��������
    CAN_PROTOCOL1_CMD_DATA10_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA11_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA12_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA13_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA14_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA15_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA16_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA17_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA18_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA19_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA20_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA21_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA22_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA23_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA24_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA25_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA26_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA27_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA28_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA29_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA30_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA31_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA32_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA33_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA34_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA35_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA36_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA37_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA38_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA39_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA40_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA41_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA42_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA43_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA44_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA45_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA46_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA47_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA48_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA49_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA50_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA51_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA52_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA53_INDEX, // ��������
    CAN_PROTOCOL1_CMD_DATA54_INDEX, // ��������

    CAN_PROTOCOL1_CMD_INDEX_MAX
} CAN_PROTOCOL1_DATE_FRAME;

// ��������
typedef enum
{
    CAN_PROTOCOL1_CMD_NULL = 0, // ������

    CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM = 0xC0,        // д����������
    CAN_PROTOCOL1_CMD_WRITE_CONTROL_PARAM_RESULT = 0xC1, // д�������������Ӧ��
    CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM = 0xC2,               // ��ȡ����������
    CAN_PROTOCOL1_CMD_READ_CONTROL_PARAM_REPORT = 0xC3,        // �����������ϱ�
    CAN_PROTOCOL1_CMD_NEW_WRITE_CONTROL_PARAM = 0x50,          // д�����ò���
	CAN_PROTOCOL1_CMD_TEST_LCD = 0xAA,						   // LCD��ɫ��ʾ����

    CAN_PROTOCOL1_CMD_MAX // ��������
} CAN_PROTOCOL1_CMD;

// UART_RX����֡����
typedef struct
{
    uint16 deviceID;
    uint8 buff[CAN_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX]; // ����֡������
    uint16 length;                                     // ����֡��Ч���ݸ���
} CAN_PROTOCOL1_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
    uint16 deviceID;
    uint8 buff[CAN_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX]; // ����֡������
    uint16 length;                                     // ����֡��Ч���ݸ���
} CAN_PROTOCOL1_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
    // һ�����ջ�����
    struct
    {
        volatile uint8 buff[CAN_PROTOCOL1_RX_FIFO_SIZE];
        volatile uint16 head;
        volatile uint16 end;
        uint16 currentProcessIndex; // ��ǰ��������ֽڵ�λ���±�
    } rxFIFO;

    // ����֡���������ݽṹ
    struct
    {
        CAN_PROTOCOL1_RX_CMD_FRAME cmdQueue[CAN_PROTOCOL1_RX_QUEUE_SIZE];
        uint16 head; // ����ͷ����
        uint16 end;  // ����β����
    } rx;

    // ����֡���������ݽṹ
    struct
    {
        CAN_PROTOCOL1_TX_CMD_FRAME cmdQueue[CAN_PROTOCOL1_TX_QUEUE_SIZE];
        uint16 head;  // ����ͷ����
        uint16 end;   // ����β����
        uint16 index; // ��ǰ����������������֡�е�������
        BOOL txBusy;  // ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
    } tx;

    // �������ݽӿ�
    BOOL (*sendDataThrowService)
    (uint32 id, uint8 *pData, uint8 length);

} CAN_PROTOCOL1_CB;

extern CAN_PROTOCOL1_CB canProtocol1CB;

/******************************************************************************
 * ���ⲿ�ӿ�������
 ******************************************************************************/
// Э���ʼ��
void CAN_PROTOCOL1_Init(void);

// Э�����̴���
void CAN_PROTOCOL_Process_3A(void);

// ��������֡�������������
void CAN_PROTOCOL1_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void CAN_PROTOCOL1_TxAddFrame(void);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void CAN_PROTOCOL1_MacProcess(uint32 standarID, uint8 *pData, uint16 length);

// ���ͻ����������һ������������
BOOL CAN_DRIVE_AddTxArray(uint32 id, uint8 *pArray, uint8 length);

extern void CAN_HW_Init(uint16 bund);

// �����ɹ��任��Ļ��ɫ
void CAN_PROTOCOL1_SendCmdChangeColor(uint32 param);

#endif
