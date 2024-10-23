#include "common.h"
#include "delay.h"
#include "BleUart.h"
#include "BleProtocol.h"
#include "timer.h"
#include "param.h"
#include "State.h"
#include "Iap.h"
#include "iapctrluart.h"
#include "Spiflash.h"
//#include "lcd.h"

// ����Ȩ����
const uint8 Authentication_Table[5][256] = {
 {0x09, 0x4c, 0xd7, 0x6e, 0x2e, 0x6d, 0x28, 0x5f, 0x03, 0xc6, 0xd1, 0x59, 0xab, 0xe3, 0xd5, 0x97, 
  0x99, 0x37, 0x3d, 0x3d, 0x37, 0x63, 0xe7, 0x80, 0x69, 0x53, 0xe4, 0xc9, 0x1c, 0x60, 0x89, 0x5c, 
  0xe8, 0x19, 0xce, 0x51, 0x8d, 0x9b, 0xdb, 0xae, 0xa4, 0x3c, 0xf7, 0xf1, 0x7c, 0xee, 0xe0, 0xae, 
  0x7b, 0xcb, 0x07, 0xb2, 0xb6, 0x44, 0x9f, 0xd4, 0xbf, 0xc7, 0xaf, 0x9d, 0xcb, 0x00, 0xbf, 0x09, 
  0x56, 0x83, 0x9f, 0xfc, 0x82, 0x5e, 0x22, 0x52, 0x32, 0xf3, 0x42, 0x37, 0xbe, 0x8e, 0x5d, 0x06,
  0xc6, 0x56, 0xac, 0xfc, 0x3a, 0x95, 0x08, 0xf1, 0x51, 0xdc, 0x69, 0xba, 0x67, 0x2e, 0x44, 0xec, 
  0x1b, 0x2c, 0x15, 0x91, 0x5c, 0x3f, 0xf2, 0xc1, 0xfd, 0xeb, 0xee, 0x30, 0xaa, 0x94, 0x7c, 0x79, 
  0xde, 0xb8, 0xcb, 0x63, 0x6e, 0xf8, 0x28, 0x5e, 0xf5, 0x32, 0x2b, 0x34, 0x0f, 0xe4, 0x56, 0x4e, 
  0xf3, 0xf0, 0x3e, 0xe1, 0x68, 0x53, 0x26, 0x2d, 0x70, 0x98, 0x1e, 0x13, 0x61, 0x6f, 0x5f, 0x91, 
  0x39, 0x75, 0xa2, 0x11, 0x2c, 0x34, 0x96, 0xfd, 0x5e, 0x2f, 0x76, 0xe6, 0xa7, 0x3c, 0xf7, 0x37, 
  0x17, 0x88, 0x6e, 0xa5, 0x9f, 0x3d, 0x24, 0x9b, 0xe6, 0x54, 0x2d, 0x49, 0xf1, 0x00, 0x0d, 0x77, 
  0x87, 0xf0, 0xaa, 0xe6, 0xca, 0xdb, 0xc1, 0xcc, 0x97, 0x08, 0x22, 0x16, 0x78, 0xe5, 0x83, 0xf8, 
  0x0a, 0x63, 0x7c, 0xe6, 0x9a, 0x76, 0xc3, 0x3d, 0xc9, 0x06, 0x3b, 0xae, 0x98, 0xb4, 0xae, 0x18, 
  0x1e, 0xeb, 0x7a, 0x84, 0xb1, 0x30, 0x73, 0xd8, 0xbe, 0x14, 0x8b, 0x54, 0x15, 0xdb, 0x88, 0xf5, 
  0xa0, 0xc4, 0x43, 0xb6, 0xc8, 0xd2, 0x75, 0x0c, 0xeb, 0x15, 0xfd, 0x10, 0x45, 0xd1, 0xfc, 0x90, 
  0xa6, 0x41, 0xe0, 0xa7, 0x30, 0x48, 0x9f, 0x7c, 0x01, 0x54, 0x03, 0x9f, 0x7d, 0x58, 0xde, 0xac},

 {0xC8, 0xed, 0x36, 0xab, 0xa6, 0x36, 0x8e, 0x68, 0x26,	0x2f, 0x42, 0x30, 0xb0, 0x79, 0x56, 0xbc,
  0xfd, 0xbe, 0x7e, 0x2a, 0xb8, 0xea, 0xcf, 0x3e, 0x78, 0xb4, 0x22, 0x4f, 0x75, 0xb9, 0xfd, 0x31,
  0x0a, 0xbe, 0x08, 0x81, 0x87, 0xe8, 0xec, 0x2d, 0xbd, 0x96, 0xb1, 0xb7, 0x11, 0x5b, 0x8a, 0xc8,
  0xe7, 0xf3, 0x53, 0xcb, 0xa0, 0x7b, 0xf6, 0xfa, 0x93, 0x2e, 0x41, 0xf2, 0xbb, 0xbf, 0xb6, 0x2b,
  0x31, 0x1b, 0x5f, 0xea, 0xe3, 0xb4, 0x9a, 0x14, 0x5c, 0xb6, 0x78, 0x26, 0xe8, 0x65, 0x4c, 0xd9,
  0xc9, 0xc6, 0x9a, 0x84, 0xc3, 0x2d, 0xd6, 0x09, 0xae, 0x84, 0xdd, 0x4c, 0x71, 0x64, 0x8b, 0x64,
  0x36, 0xdb, 0x2c, 0x8e, 0x72, 0x5f, 0x30, 0xa0, 0xce, 0xf6, 0xfd, 0x23, 0xe7, 0xbd, 0x49, 0x85,
  0x5b, 0x08, 0x12, 0xcf, 0x7c, 0x07, 0xe1, 0x13, 0xb5, 0x94, 0xa6, 0x67, 0x8c, 0xf8, 0x47, 0x9a,
  0xe7, 0xae, 0xa1, 0xe4, 0x3e, 0x1f, 0x78, 0xe7, 0x1a, 0xdf, 0xc1, 0xc8, 0x73, 0x91, 0x48, 0xfc,
  0x1f, 0xba, 0xd0, 0x4a, 0xd3, 0xe6, 0x79, 0xe8, 0x0b, 0x4d, 0x41, 0x1f, 0x50, 0xbe, 0x54, 0xc0,
  0x5f, 0xea, 0xec, 0xe4, 0xe4, 0x7a, 0x72, 0xca, 0x8b, 0x02, 0xac, 0x6b, 0x6c, 0xfb, 0xc2, 0x53,
  0xf5, 0x0b, 0x1a, 0x86, 0xde, 0x7f, 0x0c, 0xdd, 0xb7, 0xac, 0xba, 0x06, 0x40, 0xf3, 0x6e, 0x75,
  0xba, 0xa0, 0x36, 0xff, 0x17, 0x5e, 0x95, 0x77, 0xfa, 0x19, 0x96, 0xa2, 0x40, 0x2f, 0xb4, 0x1a,
  0xf8, 0x83, 0x89, 0x23, 0x5c, 0x91, 0x7d, 0x6a, 0xc5, 0xfd, 0x2b, 0x87, 0x4a, 0x1d, 0x98, 0xa9,
  0x16, 0xec, 0xdc, 0x58, 0x5f, 0x7e, 0x57, 0x29, 0x53, 0x67, 0x17, 0x8b, 0x3e, 0xe5, 0xb3, 0x20,
  0x93, 0x76, 0x5f, 0x22, 0x9b, 0x66, 0xc6, 0x0a, 0xf4, 0x64, 0xac, 0x52, 0x3f, 0x91, 0x4e, 0x92},

 {0xf5, 0x8e, 0xd1, 0x99, 0x36, 0x25, 0xc5, 0x46, 0x48, 0xde, 0x34, 0x7f, 0xf8, 0x5b, 0x22, 0xd2,
  0x13, 0x57, 0x81, 0xd6, 0x2d, 0x63, 0x86, 0xa9, 0x1f, 0xe2, 0xe4, 0xad, 0x69, 0x07, 0xd7, 0xb2, 
  0x8d, 0x17, 0x13, 0x7c, 0xfa, 0xe2, 0x0c, 0x51, 0xb8, 0x21, 0xea, 0x54, 0x72, 0x63, 0xcf, 0x86, 
  0xd9, 0x4e, 0xb4, 0xa4, 0x8f, 0x38, 0x3f, 0x7f, 0x00, 0x8f, 0x6f, 0x24, 0xa6, 0x0c, 0xda, 0xce, 
  0x08, 0x79, 0x99, 0xb8, 0xe7, 0xb0, 0x9b, 0xc1, 0x8e, 0x75, 0x47, 0x63, 0x1d, 0x1e, 0xd4, 0x86, 
  0x30, 0x39, 0xa7, 0xa4, 0xbe, 0xf9, 0x68, 0x1e, 0x7f, 0xcc, 0x2d, 0xc3, 0x75, 0x6c, 0x4c, 0x47, 
  0x6d, 0x8a, 0xf4, 0xf5, 0x75, 0x8f, 0x84, 0xd6, 0x35, 0x00, 0x31, 0x63, 0x3e, 0xb8, 0x04, 0x3d, 
  0xf1, 0x9d, 0xb2, 0x62, 0xac, 0x5c, 0x26, 0x27, 0x62, 0x7e, 0xc3, 0x84, 0x66, 0x64, 0xa2, 0x66, 
  0xab, 0xd2, 0x01, 0x42, 0x1a, 0x1e, 0x2d, 0xa7, 0xea, 0xb0, 0xe6, 0x64, 0x12, 0x27, 0x36, 0x9f, 
  0x08, 0x49, 0x22, 0x72, 0x0d, 0x03, 0x6f, 0xa7, 0x1c, 0xd0, 0x05, 0xc9, 0x6f, 0x35, 0xfd, 0xff, 
  0x42, 0x93, 0x91, 0x26, 0x29, 0x08, 0x96, 0x25, 0xbe, 0x10, 0xf3, 0xbb, 0xf8, 0x6c, 0xe7, 0xfe, 
  0xc9, 0x00, 0x84, 0x28, 0xd6, 0x8d, 0xf6, 0xc3, 0x6d, 0xae, 0x89, 0xe6, 0xb4, 0xfd, 0x6b, 0xe5, 
  0x3d, 0x0a, 0x4f, 0x00, 0xe8, 0xaf, 0xf1, 0x5b, 0xd7, 0x58, 0x6d, 0x2c, 0xdf, 0x0c, 0x15, 0x17, 
  0x81, 0x5d, 0x2c, 0x91, 0xf7, 0xdb, 0x5b, 0x8f, 0x45, 0x75, 0x7d, 0xe6, 0x97, 0xd7, 0x65, 0x96, 
  0x62, 0xf9, 0xe9, 0x9c, 0xef, 0x1c, 0x6a, 0xf2, 0xfa, 0xc8, 0x63, 0x4e, 0xef, 0xcf, 0x64, 0x67, 
  0x4d, 0xf7, 0xfa, 0xb0, 0x57, 0xa7, 0x9d, 0x36, 0xeb, 0xed, 0xc3, 0x99, 0x02, 0x38, 0x8f, 0x31},

 {0x26, 0x2e, 0x92, 0x04, 0x9e, 0x3b, 0x51, 0xaf, 0xc9, 0x66, 0x48, 0x25, 0x88, 0x1b, 0x24, 0xba,
  0x97, 0xae, 0x88, 0x05, 0x6e, 0x16, 0xb5, 0x59, 0x6c, 0x20, 0xf3, 0x07, 0xb3, 0x86, 0x28, 0xe0, 
  0xf7, 0x28, 0xa1, 0x7c, 0xed, 0xf7, 0x7c, 0xf3, 0xec, 0x65, 0xd5, 0x2c, 0xd4, 0x32, 0x51, 0xb7, 
  0x42, 0x2a, 0x69, 0x29, 0xaa, 0x04, 0x5a, 0x43, 0xef, 0xe6, 0x70, 0xd8, 0xb9, 0x31, 0x16, 0x40, 
  0xd5, 0x1b, 0xc2, 0x47, 0xa6, 0x62, 0xc1, 0x1b, 0xc8, 0xb8, 0x91, 0x51, 0xc2, 0xf1, 0x88, 0x41, 
  0xf2, 0xe6, 0x21, 0x29, 0x8c, 0xd5, 0x76, 0x32, 0xe5, 0xb4, 0x41, 0x38, 0x11, 0x2b, 0xa8, 0x75, 
  0x35, 0x2c, 0x43, 0x7a, 0xec, 0x0e, 0xb9, 0xba, 0x64, 0xab, 0x61, 0xc5, 0xf8, 0xb9, 0x82, 0xad, 
  0x6c, 0x9a, 0xe4, 0x97, 0x08, 0x30, 0x84, 0x3f, 0x50, 0xe5, 0x89, 0x66, 0x39, 0xe2, 0xb2, 0x7f, 
  0x58, 0xba, 0xde, 0x8d, 0xa2, 0x05, 0x68, 0x3a, 0x08, 0x78, 0x9f, 0x41, 0x88, 0x9a, 0xbb, 0xf1, 
  0xda, 0xe3, 0x51, 0x3d, 0x53, 0x60, 0x81, 0xf3, 0x52, 0xed, 0xa2, 0x0c, 0xdd, 0x26, 0xb2, 0xa1, 
  0x05, 0xb9, 0x3d, 0x16, 0xf3, 0x33, 0x01, 0x1d, 0x8f, 0xb1, 0x33, 0xd7, 0x09, 0xc5, 0xcf, 0xf9, 
  0xa1, 0xb8, 0x13, 0xfb, 0x83, 0xd8, 0xda, 0xaf, 0xa2, 0xe4, 0x55, 0x29, 0x16, 0xb7, 0x4a, 0xd8, 
  0x8c, 0x53, 0xc3, 0xc6, 0x1e, 0x05, 0xf8, 0x88, 0xf7, 0xea, 0xe7, 0x02, 0xf0, 0x41, 0x18, 0x41, 
  0x8b, 0x27, 0xc3, 0xe7, 0x7d, 0xf7, 0x9c, 0x4d, 0x3a, 0x55, 0x52, 0x43, 0xda, 0x1c, 0xff, 0x8f, 
  0x05, 0xba, 0x8b, 0xac, 0x79, 0x4a, 0x4b, 0x08, 0x32, 0xa1, 0xf9, 0xef, 0x27, 0xd8, 0x84, 0xa3, 
  0x22, 0x4e, 0x10, 0xa6, 0x1d, 0x06, 0xda, 0x0c, 0x46, 0x3e, 0xdb, 0xd3, 0xbd, 0xb1, 0x33, 0x99},

 {0xd7, 0x0e, 0xb3, 0xef, 0xd0, 0x7e, 0xe3, 0x38, 0x1b, 0xed, 0xa6, 0x78, 0xa0, 0x33, 0x07, 0x65, 
  0xf3, 0x84, 0x8c, 0x16, 0x8a, 0x5a, 0xeb, 0x58, 0x91, 0x86, 0x5b, 0x83, 0x65, 0x58, 0x16, 0xb1, 
  0xfb, 0x8d, 0xd0, 0x2f, 0xda, 0xb6, 0x73, 0x42, 0x3c, 0xf5, 0x6c, 0x92, 0x20, 0x26, 0x2c, 0x3e, 
  0xf6, 0x12, 0x3c, 0x07, 0x5a, 0xd3, 0x90, 0x0a, 0x32, 0x63, 0x29, 0x3e, 0x14, 0x32, 0x49, 0x51, 
  0x77, 0xab, 0xb6, 0x6a, 0xa0, 0xea, 0x6e, 0x30, 0xab, 0x14, 0x5f, 0xa2, 0x4d, 0x8e, 0x48, 0x78, 
  0x2f, 0xc2, 0x00, 0x2f, 0xb4, 0x31, 0xc2, 0x82, 0x3b, 0x77, 0xed, 0xb8, 0x98, 0x17, 0xef, 0x08, 
  0x79, 0x38, 0xb9, 0x84, 0x1a, 0x09, 0x16, 0xe5, 0x49, 0x00, 0xc8, 0x42, 0x3e, 0x18, 0x23, 0xd6, 
  0x54, 0x0b, 0x1d, 0x11, 0xf2, 0xc1, 0x84, 0x88, 0x33, 0x36, 0x18, 0xbe, 0x26, 0xd1, 0xc1, 0x8b, 
  0x5c, 0x76, 0x0a, 0x63, 0xa8, 0x78, 0x53, 0x07, 0xae, 0x7f, 0xcd, 0xec, 0xbf, 0x59, 0x9b, 0x33, 
  0x2b, 0x06, 0xce, 0x25, 0xc5, 0x9f, 0xf1, 0x00, 0xd7, 0x31, 0x3b, 0x59, 0x44, 0x57, 0x1f, 0x71, 
  0xba, 0x3c, 0x34, 0xa4, 0x44, 0x94, 0xe0, 0x8c, 0x7d, 0x53, 0x41, 0x7f, 0xd6, 0x24, 0x1b, 0xe5, 
  0x31, 0x20, 0x5e, 0x27, 0x0d, 0xdc, 0xfd, 0x2b, 0x1b, 0xa6, 0x6e, 0xed, 0xea, 0xbd, 0x2c, 0x3b, 
  0xb0, 0x52, 0xdc, 0x89, 0xf6, 0x79, 0xb0, 0x9b, 0x0a, 0x66, 0xb2, 0xfe, 0x88, 0x1c, 0x51, 0x69, 
  0x88, 0x20, 0x8b, 0xab, 0xe8, 0xe0, 0x81, 0x1c, 0x65, 0x4a, 0x09, 0x9b, 0xe7, 0x6b, 0x1f, 0x9d, 
  0x6d, 0x0e, 0xbe, 0x2b, 0x89, 0x07, 0x94, 0xa2, 0x29, 0x3d, 0xb9, 0x90, 0xd8, 0x8e, 0x24, 0x4f,
  0x1f, 0x63, 0x21, 0xec, 0x09, 0x14, 0x82, 0x74, 0x04, 0x59, 0x8a, 0xe4, 0x86, 0x94, 0xe1, 0x0a}
};


/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/

// ���ݽṹ��ʼ��
void BLE_PROTOCOL_DataStructInit(BLE_PROTOCOL_CB* pCB);

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void BLE_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length);

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void BLE_PROTOCOL_RxFIFOProcess(BLE_PROTOCOL_CB* pCB);

// UART����֡����������
void BLE_PROTOCOL_CmdFrameProcess(BLE_PROTOCOL_CB* pCB);

// �Դ��������֡����У�飬����У����
BOOL BLE_PROTOCOL_CheckSUM(BLE_PROTOCOL_RX_CMD_FRAME* pCmdFrame);

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL BLE_PROTOCOL_ConfirmTempCmdFrameBuff(BLE_PROTOCOL_CB* pCB);

// ͨѶ��ʱ����-����
void BLE_PROTOCOL_CALLBACK_RxTimeOut(uint32 param);

// ֹͣRXͨѶ��ʱ�������
void BLE_PROTOCOL_StopRxTimeOutCheck(void);

// TXRXͨѶ��ʱ����-˫��
void BLE_PROTOCOL_CALLBACK_TxRxTimeOut(uint32 param);

// ֹͣTXRXͨѶ��ʱ�������
void BLE_PROTOCOL_StopTxRxTimeOutCheck(void);

// Э��㷢�ʹ������
void BLE_PROTOCOL_TxStateProcess(void);

// UARTЭ�����������ע�����ݷ��ͽӿ�
void BLE_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

//===============================================================================================================
// ��������ظ�
void BLE_PROTOCOL_SendCmdAck(uint32 ackCmd);

// ��������ظ�����һ������
void BLE_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// ��ѯ�̼��汾�ظ�
void BLE_PROTOCOL_SendCmdFirmVersionAck(void);

//===============================================================================================================
// ����ͨѶ��ʱ�ж�����
void BLE_PROTOCOL_StartTimeoutCheckTask(void);

// UART���߳�ʱ������
void BLE_PROTOCOL_CALLBACK_UartBusError(uint32 param);

//=======================================================================================
// ȫ�ֱ�������
BLE_PROTOCOL_CB bleProtocolCB;

// ��������������������������������������������������������������������������������������
// ����ͨѶ��ʱ�ж�����
void BLE_PROTOCOL_StartTimeoutCheckTask(void)
{
}

// Э���ʼ��
void BLE_PROTOCOL_Init(void)
{
	// Э������ݽṹ��ʼ��
	BLE_PROTOCOL_DataStructInit(&bleProtocolCB);

	// ��������ע�����ݽ��սӿ�
	BLE_UART_RegisterDataSendService(BLE_PROTOCOL_MacProcess);

	// ��������ע�����ݷ��ͽӿ�
	BLE_PROTOCOL_RegisterDataSendService(BLE_UART_AddTxArray);
}

// BLEЭ�����̴���
void BLE_PROTOCOL_Process(void)
{
	// BLE����FIFO����������
	BLE_PROTOCOL_RxFIFOProcess(&bleProtocolCB);

	// BLE���������������
	BLE_PROTOCOL_CmdFrameProcess(&bleProtocolCB);
	
	// BLEЭ��㷢�ʹ������
	BLE_PROTOCOL_TxStateProcess();
}

// ��������֡�������������
void BLE_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = bleProtocolCB.tx.head;
	uint16 end =  bleProtocolCB.tx.end;
	BLE_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &bleProtocolCB.tx.cmdQueue[bleProtocolCB.tx.end];

	// ���ͻ������������������
	if((end + 1) % BLE_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ��β����֡�������˳�
	if(pCmdFrame->length >= BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// ������ӵ�֡ĩβ��������֡����
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// ȷ���������֡��������֮ǰ��������Ϊ����֡��������ӵ����Ͷ����У���main���е��ȷ��ͣ��������ڻ��Զ�У������ȣ������У����
void BLE_PROTOCOL_TxAddFrame(void)
{
	uint16 cc = 0;
	uint16 i = 0;
	uint16 head = bleProtocolCB.tx.head;
	uint16 end  = bleProtocolCB.tx.end;
	BLE_PROTOCOL_TX_CMD_FRAME* pCmdFrame = &bleProtocolCB.tx.cmdQueue[bleProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// ���ͻ������������������
	if((end + 1) % BLE_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// ����֡���Ȳ��㣬������������ݣ��˳�
	if(BLE_PROTOCOL_CMD_FRAME_LENGTH_MIN-1 > length)	// ��ȥ"У���"1���ֽ�
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// ��β����֡�������˳�
	if(length >= BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// �����������ݳ��ȣ�ϵͳ��׼������ʱ������"���ݳ���"����Ϊ����ֵ�����Ҳ���Ҫ���У���룬��������������Ϊ��ȷ��ֵ
	pCmdFrame->buff[BLE_PROTOCOL_CMD_LENGTH_INDEX] = length - 3;	// �������ݳ���

	for(i=0; i<length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = ~cc ;

	bleProtocolCB.tx.end ++;
	bleProtocolCB.tx.end %= BLE_PROTOCOL_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2�޸�
}


// ���ݽṹ��ʼ��
void BLE_PROTOCOL_DataStructInit(BLE_PROTOCOL_CB* pCB)
{
	uint16 i;
	
	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}

	pCB->tx.txBusy = FALSE;
	pCB->tx.index = 0;
	pCB->tx.head = 0;
	pCB->tx.end = 0;
	for(i = 0; i < BLE_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end  = 0;
	for(i=0; i<BLE_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}

	pCB->isTimeCheck = FALSE;
}

// UART���Ľ��մ�����(ע����ݾ���ģ���޸�)
void BLE_PROTOCOL_MacProcess(uint16 standarID, uint8* pData, uint16 length)
{
	uint16 end = bleProtocolCB.rxFIFO.end;
	uint16 head = bleProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;
	
	// ��������
	rxdata = *pData;

	// һ���������������������
	if((end + 1)%BLE_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// һ��������δ�������� 
	else
	{
		// �����յ������ݷŵ���ʱ��������
		bleProtocolCB.rxFIFO.buff[end] = rxdata;
		bleProtocolCB.rxFIFO.end ++;
		bleProtocolCB.rxFIFO.end %= BLE_PROTOCOL_RX_FIFO_SIZE;
	}	
}

// UARTЭ�����������ע�����ݷ��ͽӿ�
void BLE_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{		
	bleProtocolCB.sendDataThrowService = service;
}

// ����ʱ��������ӵ�����֡�������У��䱾�ʲ����ǳ�����ʱ������������Ч
BOOL BLE_PROTOCOL_ConfirmTempCmdFrameBuff(BLE_PROTOCOL_CB* pCB)
{
	BLE_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	
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
	pCB->rx.end %= BLE_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// ���е������ǽ��µ����λ����Ч���ݸ������㣬�Ա㽫���λ�õ�����ʱ֡������
	
	return TRUE;
}

// Э��㷢�ʹ������
void BLE_PROTOCOL_TxStateProcess(void)
{
	uint16 head = bleProtocolCB.tx.head;
	uint16 end =  bleProtocolCB.tx.end;
	uint16 length = bleProtocolCB.tx.cmdQueue[head].length;
	uint8* pCmd = bleProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = bleProtocolCB.tx.cmdQueue[head].deviceID;

	// ���ͻ�����Ϊ�գ�˵��������
	if (head == end)
	{
		return;
	}

	// ���ͺ���û��ע��ֱ�ӷ���
	if (NULL == bleProtocolCB.sendDataThrowService)
	{
		return;
	}

	// Э�����������Ҫ���͵�������		
	if (!(*bleProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// ���ͻ��ζ��и���λ��
	bleProtocolCB.tx.cmdQueue[head].length = 0;
	bleProtocolCB.tx.head ++;
	bleProtocolCB.tx.head %= BLE_PROTOCOL_TX_QUEUE_SIZE;
}

// һ�����ջ�����������һ�����ջ�������ȡ��һ���ֽ���ӵ�����֡��������
void BLE_PROTOCOL_RxFIFOProcess(BLE_PROTOCOL_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	BLE_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
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
		if(BLE_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= BLE_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// ����ͷ��ȷ��������ʱ���������ã��˳�
		if((pCB->rx.end + 1)%BLE_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// ���UARTͨѶ��ʱʱ������-2016.1.5����
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
						BLE_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
						BLE_PROTOCOL_CALLBACK_RxTimeOut,
						0,
						1,
						ACTION_MODE_ADD_TO_QUEUE);
#endif
		
		// ����ͷ��ȷ������ʱ���������ã�������ӵ�����֡��ʱ��������
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= BLE_PROTOCOL_RX_FIFO_SIZE;
	}
	// �����ֽڣ���������ӵ�����֡��ʱ�������У����ݲ�ɾ����ǰ����
	else
	{
		// ��ʱ�����������˵����ǰ���ڽ��յ�����֡�Ǵ���ģ���ȷ������֡������ֳ�����������
		if(pCmdFrame->length >= BLE_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			BLE_PROTOCOL_StopRxTimeOutCheck();
#endif

			// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
			pCmdFrame->length = 0;	// 2016.1.5����
			// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= BLE_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// һֱȡ��ĩβ
		while(end != pCB->rxFIFO.currentProcessIndex)
		{
			// ȡ����ǰҪ������ֽ�
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			// ������δ������������գ���������ӵ���ʱ��������
			pCmdFrame->buff[pCmdFrame->length++] = currentData;
			pCB->rxFIFO.currentProcessIndex ++;
			pCB->rxFIFO.currentProcessIndex %= BLE_PROTOCOL_RX_FIFO_SIZE;

			// ��������������Ҫ�������֡�Ƿ����������������������֡��ʱ���������� ����
						
			// �����ж�����֡��С���ȣ�һ�����������������ٰ���4���ֽ�: ����֡��С���ȣ�����:����ͷ�������֡����ݳ��ȡ�У��ͣ���˲���4���ֽڵıض�������
			if(pCmdFrame->length < BLE_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ��������
				continue;
			}

			// ����֡������ֵԽ�磬˵����ǰ����֡����ֹͣ����
			if(pCmdFrame->buff[BLE_PROTOCOL_CMD_LENGTH_INDEX] > (BLE_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - BLE_PROTOCOL_CMD_FRAME_LENGTH_MIN))
			{
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				BLE_PROTOCOL_StopRxTimeOutCheck();
#endif
			
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= BLE_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// ����֡����У��
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[BLE_PROTOCOL_CMD_LENGTH_INDEX] + BLE_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// ����Ҫ��һ�£�˵��δ������ϣ��˳�����
				continue;
			}

			// ����֡����OK�������У�飬ʧ��ʱɾ������ͷ
			if(!BLE_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// ֹͣRXͨѶ��ʱ���
				BLE_PROTOCOL_StopRxTimeOutCheck();
#endif
				
				// У��ʧ�ܣ�������֡�������㣬����Ϊ����������֡
				pCmdFrame->length = 0;
				// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= BLE_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// ֹͣRXͨѶ��ʱ���
			BLE_PROTOCOL_StopRxTimeOutCheck();
#endif
			
			// ִ�е������˵�����յ���һ������������ȷ������֡����ʱ�轫����������ݴ�һ����������ɾ��������������֡����
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= BLE_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			BLE_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// �Դ��������֡����У�飬����У����
BOOL BLE_PROTOCOL_CheckSUM(BLE_PROTOCOL_RX_CMD_FRAME* pCmdFrame)
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

// BLE����֡����������
void BLE_PROTOCOL_CmdFrameProcess(BLE_PROTOCOL_CB* pCB)
{
	static uint32 recDataNum = 0;
	static uint8 recCrc8 = 0;
	BLE_PROTOCOL_CMD cmd = BLE_CMD_MAX;
	BLE_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	uint32 temp = 0;
	uint8 tempCrc8 = 0;
	BOOL errStatus;
	BOOL writeRes;

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
	if(BLE_PROTOCOL_CMD_HEAD != pCmdFrame->buff[BLE_PROTOCOL_CMD_HEAD_INDEX])
	{
		// ɾ������֡
		pCB->rx.head ++;
		pCB->rx.head %= BLE_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// ����ͷ�Ϸ�������ȡ����
	cmd = (BLE_PROTOCOL_CMD)pCmdFrame->buff[BLE_PROTOCOL_CMD_CMD_INDEX];
	
	// ִ������֡
	switch (cmd)
	{
		// ��������
		case BLE_CMD_HEARTBEAT_DOWN:
			// BOOT������Ӧ:0x00
			BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_HEARTBEAT_UP,0x00);
			break;

		// Ȩ������
		case BLE_CMD_AUTHENTICATE_DOWN:
			if ((Authentication_Table[pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA1_INDEX] - 1][pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA2_INDEX]] \
			== pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA3_INDEX]) && \
					(Authentication_Table[pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA4_INDEX] - 1][pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA5_INDEX]] \
			== pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA6_INDEX]) && \
					(Authentication_Table[pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA7_INDEX] - 1][pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA8_INDEX]] \
			== pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA9_INDEX]))
			{
				// Ȩ���жϳɹ�
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_AUTHENTICATE_UP,0x00);
			}
			else
			{
				// Ȩ���ж�ʧ��
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_AUTHENTICATE_UP,0x01);
				// �Ͽ��������Ӵ���
				//BLE_MODULE_Reset(UINT32_NULL);
			}
			TIMER_KillTask(TIMER_ID_BLE_CONNECT_VERIFICATE);
			break;

		// ѯ�ʹ̼��汾����
		case BLE_CMD_FIRMVERSION_DOWN:
			BLE_PROTOCOL_SendCmdFirmVersionAck();
			break;

		// ������ʱ����
		case BLE_CMD_SETTIMER_DOWN:
			BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_SETTIMER_UP, 0x00);
			break;

		// ����������Ӧ������
		case BLE_CMD_ERROR_DOWN:
			break;

		// ��λ����
		case BLE_CMD_RESET_DOWN:
			// �����λ����
			BLE_PROTOCOL_SendCmdAck(BLE_CMD_RESET_UP);
			break;

		// �̼���������
		case BLE_CMD_FIRMUPDATE_DOWN:
			// �����ж����ݰ��Ƿ�Ϸ�
			temp  = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA3_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA4_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			if(temp > IAP_GD32_APP1_SIZE)
			{
				// �����洢�ռ䷵��Ӧ��
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_FIRMUPDATE_UP, 0x00);
			}
			else
			{
				// ����֮ǰ��Ӧ��APP
				IAP_EraseAPP1Area();
				
				// �������֮����Ҫ��ȡ�����ж�
				errStatus = IAP_CheckApp1FlashIsBlank();
				if(!errStatus)
				{
					BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_FIRMUPDATE_UP, 0x00);
					break;
				}
				// Ӧ�ý������ݳ���
				recDataNum = temp;
				recCrc8 = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];
				// ����Ӧ��
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_FIRMUPDATE_UP, 0x01);
			}
			
			break;

		// ���ݰ�д������
		case BLE_CMD_DATAIN_DOWN:
			// ������ݳ��Ȳ��Ϸ�
			if (4 > pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX])
			{
				break;
			}

			// ���ݰ����(�̶����ݳ���)
			temp  = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA3_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA4_INDEX];
			writeRes = IAP_BleWriteAppBin(IAP_FLASH_APP1_ADDR + temp * 128, &pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX], 128);
			if(writeRes)
			{
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_DATAIN_UP, 0x00);
			}
			else
			{
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_DATAIN_UP, 0x01);
			}
			break;

		// ���ݰ�д���������
		case BLE_CMD_DATAIN_FINISH_DOWN:
			// ���ݰ�У��
			tempCrc8 = IAP_FlashReadForCrc8(recDataNum);
		
			TIMER_KillTask(TIMER_ID_BLE_UPDATE_STATE);
			if(recCrc8 == tempCrc8)
			{
				// �������������־����������ģ���ʼ����־
				SPI_FLASH_WriteHalfWord(SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS,0xFFFF);

				
				
				temp = IAP_FLASH_APP1_ADDR;
				
				TIMER_AddTask(TIMER_ID_ECO_JUMP,
								2000,
								IAP_JumpToAppFun,
								temp,
								1,
								ACTION_MODE_ADD_TO_QUEUE);
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_DATAIN_FINISH_UP, 0x01);
			}
			else
			{
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_DATAIN_FINISH_UP, 0x00);
			}
			break;

		// ��ȡ�Ǳ�״̬����
		case BLE_CMD_GETSTATUS_DOWN:
			break;

		// �����Ǳ�����
		case BLE_CMD_CONTROL_DOWN:
			break;

		// ��ȡʵʱ�˶���������
		case BLE_CMD_GETDATA_RUNNING_DOWN:
			break;

		// ����ͬ����ʷ�˶�����Ӧ������
		case BLE_CMD_START_HISTORICAL_DATA_DOWN:
			break;

		// �ϱ���ʷ�˶�����Ӧ��
		case BLE_CMD_REPORT_HISTORICAL_DATA_DOWN:
			break;
			
		default:
			break;
	}

	// ����UART����ͨѶ��ʱ�ж�
	BLE_PROTOCOL_StartTimeoutCheckTask();
	
	// ɾ������֡
	pCB->rx.head ++;
	pCB->rx.head %= BLE_PROTOCOL_RX_QUEUE_SIZE;
}

// RXͨѶ��ʱ����-����
#if BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void BLE_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	BLE_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &bleProtocolCB.rx.cmdQueue[bleProtocolCB.rx.end];

	// ��ʱ���󣬽�����֡�������㣬����Ϊ����������֡
	pCmdFrame->length = 0;	// 2016.1.6����
	// ɾ����ǰ������ͷ��������ɾ���ѷ�������������ݣ���Ϊ�����п��ܻ�������ͷ
	bleProtocolCB.rxFIFO.head ++;
	bleProtocolCB.rxFIFO.head %= BLE_PROTOCOL_RX_FIFO_SIZE;
	bleProtocolCB.rxFIFO.currentProcessIndex = bleProtocolCB.rxFIFO.head;
}

// ֹͣRxͨѶ��ʱ�������
void BLE_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

#if BLE_PROTOCOL_TXRX_TIME_OUT_CHECK_ENABLE
// TXRXͨѶ��ʱ����-˫��
void BLE_PROTOCOL_CALLBACK_TxRxTimeOut(uint32 param)
{
	
}

// ֹͣTxRXͨѶ��ʱ�������
void BLE_PROTOCOL_StopTxRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_TXRX_TIME_OUT_CONTROL);
}
#endif

// ��������ظ�
void BLE_PROTOCOL_SendCmdAck(uint32 ackCmd)
{
	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(ackCmd); 	
	BLE_PROTOCOL_TxAddData(0x00);	
	BLE_PROTOCOL_TxAddFrame();	
}

// ��������ظ�����һ������
void BLE_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(ackCmd); 	
	BLE_PROTOCOL_TxAddData(0x01);	
	
	BLE_PROTOCOL_TxAddData(ackParam);	
	BLE_PROTOCOL_TxAddFrame();	
}

// ��ѯ�̼��汾�ظ�
void BLE_PROTOCOL_SendCmdFirmVersionAck(void)
{
	uint8 val1, val2, val3, val4;
	uint32 valTemp;

	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();
	
	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_FIRMVERSION_UP); 	
	BLE_PROTOCOL_TxAddData(0x07);	

	// �Ǳ��ͺű���
	BLE_PROTOCOL_TxAddData(paramCB.runtime.devVersion[0]);
	BLE_PROTOCOL_TxAddData(paramCB.runtime.devVersion[1]);
	BLE_PROTOCOL_TxAddData(paramCB.runtime.devVersion[2]);
	
	// �̼��汾
	val1 = PARAM_GetDevVersionRemain();
	val2 = PARAM_Get_DevReleaseVersion();
	val3 = PARAM_Get_FwUpdateVersion();
	val4 = PARAM_Get_FwDebugReleaseVersion();
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);

	// �̼��汾���
	valTemp = PARAM_GetFwBinNum();
	val1 = (valTemp / 10) % 10;
	val2 = (valTemp) % 10;
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	
	BLE_PROTOCOL_TxAddFrame();	
}



