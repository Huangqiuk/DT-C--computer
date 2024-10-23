#include "common.h"
#include "delay.h"
#include "BleUart.h"
#include "BleProtocol.h"
#include "timer.h"
#include "param.h"
#include "Ds1302.h"
#include "State.h"
#include "Record.h"
#include "system.h"
#include "iap.h"

extern BLE_PROTOCOL_CB bleProtocolCB;

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
void BLE_PROTOCOL_SendCmdAck(uint8 ackCmd);

// ���ʹ�Ӧ��Ļظ�
void BLE_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

// �̼��汾�ظ�
void BLE_PROTOCOL_SendCmdFirmVersionAck(void);

// �Ǳ�״̬��Ϣ�ظ�
void BLE_PROTOCOL_SendCmdStatusAck(void);

// �Ǳ��������ظ�
//void BLE_PROTOCOL_SendCmdControlAck(uint8 controlCmd, uint8 paramH, uint8 paramL);

// �Ǳ��ȡʵʱ�˶�״̬�ظ�
void BLE_PROTOCOL_SendCmdGetRuningStateAck(void);

// �Ǳ��ϱ���ʷ����
void BLE_PROTOCOL_SendHistoryData(uint32 param);

// �Ǳ�״̬��1��Ϣ�ظ�
void BLE_PROTOCOL_SendCmdSelectStatus1Ack(void);

// �Ǳ�״̬��2��Ϣ�ظ�
void BLE_PROTOCOL_SendCmdSelectStatus2Ack(void);

// �Ǳ�״̬��3��Ϣ�ظ�
void BLE_PROTOCOL_SendCmdSelectStatus3Ack(void);

//===============================================================================================================
// ����ͨѶ��ʱ�ж�����
void BLE_PROTOCOL_StartTimeoutCheckTask(void);

// UART���߳�ʱ������
void BLE_PROTOCOL_CALLBACK_UartBusError(uint32 param);


void BLE_PROTOCOL_RecordSave(TIME_E time_t, RECORD_ITEM_TMP_CB *recordTmpCB_t, uint32 timeTmpStart_t);

//=======================================================================================

// ���������̼�����Ӧ��
void BLE_PROTOCOL_SendCmdUpDataAck(uint8 ackParam, uint8 ackParam1);

// ���Ǳ��ϱ���ʷ����
void BLE_PROTOCOL_SendHistoryData_NEW(uint32 param);



// ȫ�ֱ�������
BLE_PROTOCOL_CB bleProtocolCB;

// ��������������������������������������������������������������������������������������
// ����ͨѶ��ʱ�ж�����
void BLE_PROTOCOL_StartTimeoutCheckTask(void)
{
}

// ���ͻ�ȡMAC��ַ
void BLE_UART_SendCmdGetBleMacAddr(uint32 param)
{
	uint8 i;
	uint8 bleMacStr[] = "TTM:MAC-?";
	
	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	// ��ѯ����macָ��,��������ʽ����
	for (i = 0; bleMacStr[i] != '\0'; i++)
	{
		// ������ݣ���������
		// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�) 
		USART_ClrFlag(BLE_UART_TYPE_DEF, USART_FLAG_TXC);		

		// �������
		USART_SendData(BLE_UART_TYPE_DEF, bleMacStr[i]);

		// δ�����꣬�����ȴ�
		while(USART_GetFlagStatus(BLE_UART_TYPE_DEF, USART_FLAG_TXC) != SET);
	}	
}

// ��ʱ���ص����ͻ�ȡ������ַ����
void BLE_UART_CALLBALL_GetBleMacAddrRequest(uint32 param)
{
	// ���ͻ�ȡMAC��ַ
	BLE_UART_SendCmdGetBleMacAddr(TRUE);
}


void BLE_UPDataDelay(uint32 param)
{
	// ����Ӧ��
	BLE_PROTOCOL_SendCmdUpDataAck(bleProtocolCB.DeviceStyle, param);
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

	// ע���ѯBLE��MAC��ַ
	TIMER_AddTask(TIMER_ID_BLE_CONTROL,
					200,
					BLE_UART_CALLBALL_GetBleMacAddrRequest,
					TRUE,
					TIMER_LOOP_FOREVER,
					ACTION_MODE_ADD_TO_QUEUE);

	// �ϵ�Ĭ�ϳ����㲥����
	TIMER_AddTask(TIMER_ID_BLE_ADV_RESET,
					1000,
					BLE_PROTOCOL_SendAdv,
					0,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
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

void BLE_PROTOCOL_TxErrorInfo(uint32 param)
{
	uint8 val1,val2;
	uint16 valTemp = (uint16)param;
	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_ERROR_UP); 	
	BLE_PROTOCOL_TxAddData(0x02);
	
	val1 = (uint8)((valTemp >> 8) & 0xFF);
	val2 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	BLE_PROTOCOL_TxAddFrame();	
}

// �Ǳ�����ͬ����ʷ����
void BLE_PROTOCOL_TxStartHistoryData(uint32 param)
{
	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_START_HISTORICAL_DATA_UP); 	
	BLE_PROTOCOL_TxAddData(0x00);
	BLE_PROTOCOL_TxAddFrame();	
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
//	BLE_PROTOCOL_CMD cmd = BLE_CMD_MAX;
//	BLE_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
//	TIME_E time;
//	TIME_E record_d_time;	// �Ѿ���¼ʱ��
//	TIME_E record_c_time;	// DS1302���ڼ�¼��ʱ��
//	RECORD_ITEM_CB track;
//	
//	uint32 bleCheckTime;	// ����У׼��ʱ���
//	uint32 d_time;	// �Ѿ���¼��ʱ���
//	uint32 c_time;	// DS1302���ڼ�¼��ʱ���

//	uint16 count;
//	uint16 i;
//	uint32 startTimeAddr;
	
	BLE_PROTOCOL_CMD cmd = BLE_CMD_MAX;
	BLE_PROTOCOL_RX_CMD_FRAME* pCmdFrame = NULL;
	TIME_E time;
	TIME_E timeBk;	// ʱ�䱸����ʱ����

	uint32 timeCurrent;		// ��ǰʱ���
	uint32 timeTmpStart;	// ��ʱ���ʱ��
	uint32 timeTmp;			// ʱ���ת����ʱ�����ݴ�
	uint32 timeTmpBefore;	// ���֮ǰ��ʱ��
	uint16 tirpNumBk;		// ������������
	uint16 i;
	uint32 bBuff[1];
	
	uint32 temp = 0;
	uint8 tempCrc8 = 0;
	BOOL writeRes;
	uint32 buff[2] = {0};
	static uint32 recDataNum = 0;
	static uint8 recCrc8 = 0;

	static uint32 crcCheck = 0;
	static uint32 fileDataLen = 0;
 	
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
			// APP��������Ӧ��:0x01
			BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_HEARTBEAT_UP,0x01);
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
				PARAM_SetBleConnectState(TRUE);

				// �ϱ�ϵͳ������Ϣ
				TIMER_AddTask(TIMER_ID_BLE_ERROR_INFO,
							1000,
							BLE_PROTOCOL_TxErrorInfo,
							(uint32)PARAM_GetErrorCode(),
							TIMER_LOOP_FOREVER,
							ACTION_MODE_ADD_TO_QUEUE);

				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_AUTHENTICATE_UP,0x00);
			}
			else
			{
				// Ȩ���ж�ʧ��
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_AUTHENTICATE_UP,0x01);
				// �Ͽ��������Ӵ���
				TIMER_AddTask(TIMER_ID_BLE_DISCONNECT,
								1000,
								BLE_MODULE_Reset,
								UINT32_NULL,
								1,
								ACTION_MODE_ADD_TO_QUEUE);
			}
			TIMER_KillTask(TIMER_ID_BLE_CONNECT_VERIFICATE);
			break;

		// ѯ�ʹ̼��汾����
		case BLE_CMD_FIRMVERSION_DOWN:
			// �̼��汾��ѯ�ذ�
			BLE_PROTOCOL_SendCmdFirmVersionAck();
			break;

		// ������ʱ����
		case BLE_CMD_SETTIMER_DOWN:
			time.year = 2000 + pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA2_INDEX];
			time.month= pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA3_INDEX];
			time.day= pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA4_INDEX];
			time.hour= pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA5_INDEX];
			time.minute= pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA6_INDEX];
			time.second= pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA7_INDEX];
			
			// ���õ�ʱ�����ݳ�����ֵ��������ʧ��	
//			DS1302_GetDateCB(&record_c_time);

			if (UINT32_NULL == DS1302_SetDate(time.year, time.month, time.day, time.hour, time.minute, time.second))
			{
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_SETTIMER_UP,0x01);
				break;
			}


			// RECORD������ǰ��¼���������״̬��ͬʱ����ǰRAM���ݱ��浽NVM
			if ( (RECORD_STATE_RECORDING == RECORD_GetState()) && (bleProtocolCB.isTimeCheck != TRUE) )
			{
				RECORD_EnterState(RECORD_STATE_STANDBY);
			}
			
			if (bleProtocolCB.isTimeCheck == FALSE)
			{
				// �رշ���ʱ���ݴ洢��ʱ��
				TIMER_KillTask(TIMER_ID_RECORD_COUNT);
				
				// ������ʱ���������ݴ浽flash��ȥ��Ȼ����շ���ʱ����
				// �������ʱ��¼���ʱ��(ע������ʱ�侭��0�㴦��)
				timeCurrent = (uint32)time.hour * 3600 + (uint32)time.minute * 60 + (uint32)time.second;
				if (timeCurrent > recordTmpCB.tirpTime)
				{
					timeTmpStart = timeCurrent - recordTmpCB.tirpTime;
					
					// �洢�ݴ���������
					BLE_PROTOCOL_RecordSave(time, &recordTmpCB, timeTmpStart);
				}
				else
				{
					// ���й���㴦�������ڣ�
					// ���֮ǰ�������ݴ�
					
					timeTmpBefore = (recordTmpCB.tirpTime - timeCurrent)/60;
					for (i = 0; i < recordTmpCB.tirpNum; i++)
					{
						if (recordTmpCB.recordItemTmp[i].recordparam.min >= timeTmpBefore)
						{
							break;
						}
					}
					// ����ԭʼ������������
					tirpNumBk = recordTmpCB.tirpNum;
					recordTmpCB.tirpNum = i - 1;
					
					// ʱ�����ȥ24СʱȻ��ת��Ϊʱ��
					timeTmp = DS1302_GetTimeStampFromDateTime(time) - 86400;
					DS1302_TimeStampToDateTime(timeTmp, &timeBk);

					timeTmpStart = 86400 - (recordTmpCB.tirpTime - timeCurrent);
					
					BLE_PROTOCOL_RecordSave(timeBk, &recordTmpCB, timeTmpStart);

					// ����Ѿ�����flash������
					memset(&(recordTmpCB.recordItemTmp[0]), 0, (recordTmpCB.tirpNum + 1) * sizeof(RECORD_ITEM_TMP));
					
					// ʣ�µ������ۼƷ�����Ҫ��ȥ0��֮ǰ������
					for (i = recordTmpCB.tirpNum; i < tirpNumBk; i++)
					{
						if (recordTmpCB.recordItemTmp[i].recordparam.min > timeTmpBefore)
						{
							recordTmpCB.recordItemTmp[i].recordparam.min -= timeTmpBefore;
						}
						else
						{
							recordTmpCB.recordItemTmp[i].recordparam.min = 0;
						}
						
					}

					// ���֮��������ݴ�
					recordTmpCB.tirpNum = tirpNumBk;
					timeTmpStart = 0;

					BLE_PROTOCOL_RecordSave(time, &recordTmpCB, timeTmpStart);
				}
				memset(&recordTmpCB, 0, sizeof(RECORD_ITEM_TMP_CB));
			}
			
			// ����У׼�ɹ�
			bleProtocolCB.isTimeCheck = TRUE;

			BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_SETTIMER_UP,0x00);
				
			if (PARAM_GetSpeed() == 0)
			{
				// ����ͬ����ʷ����
				if(RECORD_IsHistoricalData())
				{
					// ����ͬ����ʷ���ݣ����û�н��յ�Ӧ��������ѭ����������ͬ������
					TIMER_AddTask(TIMER_ID_BLE_START_HISTORICAL_DATA_UP,
									5000,
									BLE_PROTOCOL_TxStartHistoryData,
									UINT32_NULL,
									TIMER_LOOP_FOREVER,
									ACTION_MODE_ADD_TO_QUEUE);
				}
			}
			break;

		// ����������Ӧ������
		case BLE_CMD_ERROR_DOWN:
			// ֹͣ�����ϱ�������Ϣ
			TIMER_KillTask(TIMER_ID_BLE_ERROR_INFO);
			break;

		// ��λ����
		case BLE_CMD_RESET_DOWN:
			  
			BLE_PROTOCOL_SendCmdAck(BLE_CMD_RESET_UP);
		
			break;
		

		// �̼���������
		case BLE_CMD_FIRMUPDATE_DOWN:
			
			// �ٶȲ�Ϊ0�����ɽ��д˲���
			if (paramCB.runtime.speed != 0)
			{
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_FIRMUPDATE_UP, 0x00);
				break;
			}
			
			TIMER_KillTask(TIMER_ID_PROTOCOL_PARAM_TX);
		
			// �����ж����ݰ��Ƿ�Ϸ�
			temp  = pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA3_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA4_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA5_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA6_INDEX];
			
			if(temp > IAP_N32_APP1_SIZE)
			{
				// �����洢�ռ䷵��Ӧ��
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_FIRMUPDATE_UP, 0x00);
			}
			else
			{	
				TIMER_Pause(TIMER_ID_PROTOCOL_PARAM_TX);
				TIMER_KillTask(TIMER_ID_CAN_COMMUNICATION_TIME_OUT);
				
				// ����APP2����ַ
				IAP_EraseAPP2Area();	
				
				// Ӧ�ý������ݳ���
				recDataNum = temp;
				recCrc8 = pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA1_INDEX];
	
				// ����Ӧ��
				BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_FIRMUPDATE_UP, 0x01);

				paramCB.runtime.blueUpgradePer =0;
				
				//����״̬����ת
				STATE_EnterState(STATE_BLUE_UPGRADE);
			}	
					
			break;
			
		
		// ���ݰ�д������
		case BLE_CMD_DATAIN_DOWN:
			
			// ������ݳ��Ȳ��Ϸ�
			if (4 > pCmdFrame->buff[BLE_PROTOCOL_CMD_LENGTH_INDEX])
			{
				break;
			}
			
			// ���ݰ����(�̶����ݳ���)
			temp  = pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA1_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA2_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA3_INDEX];
			temp <<= 8;
			temp += pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA4_INDEX];

			// ���������ٷֱ�
			paramCB.runtime.blueUpgradePer = (( temp * 128 )*100) / recDataNum;
			
			// ��APP2���������
			writeRes = IAP_WriteAppBin(IAP_FLASH_APP2_ADDR + temp * 128, &pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA5_INDEX], 128);
			
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
			
				if(recCrc8 == tempCrc8)
				{
					buff[0] = 0xAA000000;// ����������־������BOOTʱ��Ҫ�ж�
					buff[1] = recDataNum;// ���ݰ���С��
					
					// ����������־������ģ���ʼ����־
					IAP_FlashWriteWordArrayWithErase(PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS, buff, 2);
					
					BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_DATAIN_FINISH_UP, 0x01);

					paramCB.runtime.blueUpgradePer =100;

				}
				else
				{
					// ���ݰ�У�鲻�ɹ�����һֱ����boot�У��ȴ��ٴ���������
					BLE_PROTOCOL_SendCmdParamAck(BLE_CMD_DATAIN_FINISH_UP, 0x00);

					paramCB.runtime.blueUpgradePer =0;
				}
				
			break;

		// ��ȡ�Ǳ�״̬����
		case BLE_CMD_GETSTATUS_DOWN:
			BLE_PROTOCOL_SendCmdStatusAck();
			break;

		// �����Ǳ�����
		case BLE_CMD_CONTROL_DOWN:
			BLE_PROTOCOL_SendCmdControlAck(pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA1_INDEX],\
											pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA2_INDEX],\
											pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA3_INDEX]);
			break;
		
		// ѡ���Ի�ȡ�����ϱ�
		case BLE_CMD_SELECT_GETSTATUS_DOWN:
			if (pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA1_INDEX] == 0x01)
			{
				// �Ǳ�״̬��1��Ϣ�ظ�
				BLE_PROTOCOL_SendCmdSelectStatus1Ack();
			}
			else if (pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA1_INDEX] == 0x02)
			{
				// �Ǳ�״̬��2��Ϣ�ظ�
				BLE_PROTOCOL_SendCmdSelectStatus2Ack();
			}
			else if (pCmdFrame->buff[BLE_PROTOCOL_CMD_DATA1_INDEX] == 0x03)
			{
				// �Ǳ�״̬��3��Ϣ�ظ�
				BLE_PROTOCOL_SendCmdSelectStatus3Ack();
			}
			break;

		// ��ȡʵʱ�˶���������
		case BLE_CMD_GETDATA_RUNNING_DOWN:
			BLE_PROTOCOL_SendCmdGetRuningStateAck();
			break;

		// ����ͬ����ʷ�˶�����Ӧ������
		case BLE_CMD_START_HISTORICAL_DATA_DOWN:
			// �ر������Է���������ʷ����ͬ������
			TIMER_KillTask(TIMER_ID_BLE_START_HISTORICAL_DATA_UP);
			
			// �ٶȲ�Ϊ0���������ϱ���ʷ����
			if (PARAM_GetSpeed() == 0)
			{
				// ���͵�һ����ʷ�ϱ�����
				BLE_PROTOCOL_SendHistoryData(UINT32_NULL);

				// ��ʷ����û��ͬ����ɣ���������ͬ����ʷ���ݣ�������ѭ����������ͬ������
				if(RECORD_IsHistoricalData())
				{
					TIMER_AddTask(TIMER_ID_BLE_START_HISTORICAL_DATA_UP,
									5000,
									BLE_PROTOCOL_TxStartHistoryData,
									UINT32_NULL,
									TIMER_LOOP_FOREVER,
									ACTION_MODE_ADD_TO_QUEUE);
				}
			}
			break;

		// �ϱ���ʷ�˶�����Ӧ��
		case BLE_CMD_REPORT_HISTORICAL_DATA_DOWN:
			// �ر������Է���������ʷ����ͬ������
			TIMER_KillTask(TIMER_ID_BLE_START_HISTORICAL_DATA_UP);

			// �ٶȲ�Ϊ0���������ϱ���ʷ����
			if (PARAM_GetSpeed() == 0)
			{
				// �ϴ��ɹ���ˢ�¶�ȡ�ѷ�����ʷ����ֵ
				// ˢ���ѷ�����ʷ��������
				RECORD_RefreashSendCount();
				
				// ���ŷ���һ����ʷ����
				BLE_PROTOCOL_SendHistoryData(UINT32_NULL);

				// ��ʷ����û��ͬ����ɣ���������ͬ����ʷ����
				if(RECORD_IsHistoricalData())
				{
					// ����ͬ����ʷ���ݣ����û�н��յ�Ӧ��������ѭ����������ͬ������
					TIMER_AddTask(TIMER_ID_BLE_START_HISTORICAL_DATA_UP,
									5000,
									BLE_PROTOCOL_TxStartHistoryData,
									UINT32_NULL,
									TIMER_LOOP_FOREVER,
									ACTION_MODE_ADD_TO_QUEUE);
				}
			}
			break;
			
		case BLE_CMD_START_HISTORICAL_DATA_DOWN_NEW:
			
			// �ر������Է�����������ʷ����ͬ������
			TIMER_KillTask(TIMER_ID_BLE_START_HISTORICAL_DATA_UP);

			// �ٶȲ�Ϊ0���������ϱ���ʷ����
			if (PARAM_GetSpeed() == 0)
			{
				// ���͵�һ����ʷ�ϱ�����
				//BLE_PROTOCOL_SendHistoryData_NEW(UINT32_NULL);
			}
			break;
			
		case BLE_CMD_REPORT_HISTORICAL_DATA_DOWN_NEW:
			
			// �ر������Է���������ʷ����ͬ������
			TIMER_KillTask(TIMER_ID_BLE_START_HISTORICAL_DATA_UP);
			
			// �ٶȲ�Ϊ0���������ϱ���ʷ����
			if (PARAM_GetSpeed() == 0)
			{
				//App�ɹ��յ���ˢ���Ѷ�����
				RECORD_IsHistoricalData();
				
				// ���ŷ���һ����ʷ����
				//BLE_PROTOCOL_SendHistoryData_NEW(UINT32_NULL);
			}
			break;
			
		// ��ȡ��ѹ����������ָ��
		case BLE_CMD_GET_VOLCUR_VALUE:
			
			BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
			BLE_PROTOCOL_TxAddData(BLE_CMD_GET_VOLCUR_VALUE_ACK); 	
			BLE_PROTOCOL_TxAddData(0x08);	
		
			// ��ѹ����λmV
			BLE_PROTOCOL_TxAddData((PARAM_GetBatteryVoltage() >> 24) & 0xFF);
			BLE_PROTOCOL_TxAddData((PARAM_GetBatteryVoltage() >> 16) & 0xFF);
			BLE_PROTOCOL_TxAddData((PARAM_GetBatteryVoltage() >> 8) & 0xFF);
			BLE_PROTOCOL_TxAddData(PARAM_GetBatteryVoltage() & 0xFF);
			
			// ��������λmA
			BLE_PROTOCOL_TxAddData((PARAM_GetBatteryCurrent() >> 24) & 0xFF);
			BLE_PROTOCOL_TxAddData((PARAM_GetBatteryCurrent() >> 16) & 0xFF);
			BLE_PROTOCOL_TxAddData((PARAM_GetBatteryCurrent() >> 8) & 0xFF);
			BLE_PROTOCOL_TxAddData(PARAM_GetBatteryCurrent() & 0xFF);
		
			BLE_PROTOCOL_TxAddFrame();
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

void BLE_PROTOCOL_RecordSave(TIME_E time_t, RECORD_ITEM_TMP_CB *recordTmpCB_t, uint32 timeTmpStart_t)
{	
	uint16 i;
	//uint32 timeTmpStart;	// ��ʱ���ʱ��
	uint8 timeTmpHour;		// ��ʱСʱʱ���
	uint32 startTimeAddr;
	RECORD_ITEM_CB track[24] = {0};
	
	uint16 totalCount;
	uint8 bBuff[2];
	
	// ��ȡ���м�¼�������Ѷ�������
	IAP_FlashReadByteArray(RECORD_ADDR_TOTAL, bBuff, 2);
	totalCount = (uint16)(bBuff[1] << 8) + (uint16)bBuff[0];
	
	// ��ȡ�洢����ʱ��¼��ʼ��ַ
	startTimeAddr = RECORD_ADDR_DATA_START + totalCount * sizeof(RECORD_PARAM_CB);
	
	// ɨ���ѯ�ݴ��¼
	for (i = 0; i <= recordTmpCB_t->tirpNum ; i++)
	{
		// У׼Сʱʱ��
		recordTmpCB_t->recordItemTmp[i].recordparam.hour = (timeTmpStart_t + recordTmpCB_t->recordItemTmp[i].recordparam.min * 60)/3600;
		
		timeTmpHour = recordTmpCB_t->recordItemTmp[i].recordparam.hour;
		// �ϲ�ʱ�������
		track[timeTmpHour].param.calories += (uint16)recordTmpCB_t->recordItemTmp[i].recordparam.calories;
		
		track[timeTmpHour].param.dateTime.year = (uint8)(time_t.year - 2000);
		track[timeTmpHour].param.dateTime.month = time_t.month;
		track[timeTmpHour].param.dateTime.day = time_t.day;
		track[timeTmpHour].param.dateTime.hour = recordTmpCB_t->recordItemTmp[i].recordparam.hour;
		track[timeTmpHour].param.dateTime.min = (timeTmpStart_t + recordTmpCB_t->recordItemTmp[i].recordparam.min * 60)%60;
		track[timeTmpHour].param.dateTime.sec = 0;
		
		track[timeTmpHour].param.isTimeCheck = 0x01;
		
		if (track[timeTmpHour].param.speedMax < recordTmpCB_t->recordItemTmp[i].recordparam.speedMax)
		{
			track[timeTmpHour].param.speedMax = recordTmpCB_t->recordItemTmp[i].recordparam.speedMax;
		}
		
		track[timeTmpHour].param.timeSlot = timeTmpHour;
		
		track[timeTmpHour].param.trip += (uint16)recordTmpCB_t->recordItemTmp[i].recordparam.trip;
		
		track[timeTmpHour].param.tripTimer += recordTmpCB_t->recordItemTmp[i].recordparam.ridingTime;
	}
	
	// ���ϲ�������ݴ���flash��
	for (i = 0; i < 24; i++)
	{
		// ��̴���0˵�������м�¼����Ҫ��������
		if (track[i].param.trip > 0)
		{
			// ����У��λ
			track[i].param.packCheck = RECORD_Check(track[i].array, sizeof(RECORD_PARAM_CB) - 1);
			
			// ����flash
			IAP_FLASH_WriteByteWithErase(startTimeAddr, (uint8*)track[i].array, sizeof(RECORD_PARAM_CB));
			startTimeAddr += sizeof(RECORD_PARAM_CB);
			// ˢ����ʷ��������
			RECORD_RefreashSaveCount();
		}
	}
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
void BLE_PROTOCOL_SendCmdAck(uint8 ackCmd)
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
	uint16 valTmp;

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
	valTmp = PARAM_GetFwBinNum();
	val1 = (uint8)((valTmp >> 8) & 0xFF);
	val2 = (uint8)(valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	
	BLE_PROTOCOL_TxAddFrame();	
}

// �Ǳ�״̬��Ϣ�ظ�
void BLE_PROTOCOL_SendCmdStatusAck(void)
{
	uint8 val1, val2, val3, val4;
	uint16 valTmp;
	uint32 valTemp;

	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_GETSTATUS_UP); 	
	BLE_PROTOCOL_TxAddData(0x16);	

	// ���������
	valTemp = paramCB.nvm.param.common.record.total.distance;
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);

	// ��ʱ��
	valTemp = PARAM_GetTotalRidingTime();
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);

	// �ܿ�·��
	valTemp = PARAM_GetTotalCalories();
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);

	// ����ٶ�
	valTmp = paramCB.nvm.param.common.record.trip.speedMax;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ƽ���ٶ�
	valTmp = paramCB.runtime.trip.speedAverage;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// �����ٷֱ�
	val1 = PARAM_GetBatteryPercent();
	BLE_PROTOCOL_TxAddData(val1);

	// ������λ
	val1 = (uint8)PARAM_GetAssistLevel();
	BLE_PROTOCOL_TxAddData(val1);

	// ���״̬
	val1 = (uint8)PARAM_GetLightSwitch();
	BLE_PROTOCOL_TxAddData(val1);

	// �Զ��ػ�ʱ��
	val1 = PARAM_GetPowerOffTime();
	BLE_PROTOCOL_TxAddData(val1);

	// ����ֵ
	val1 = paramCB.nvm.param.common.speed.limitVal/10;
	BLE_PROTOCOL_TxAddData(val1);

	// ��ʾ��λ
	val1 = PARAM_GetUnit();
	BLE_PROTOCOL_TxAddData(val1);
	
	// ����״̬
	BLE_PROTOCOL_TxAddData(0x00);

	// �Ǳ�������
	val1 = PARAM_GetBrightness();
	BLE_PROTOCOL_TxAddData(val1);

	BLE_PROTOCOL_TxAddFrame();	
}

// �Ǳ��������ظ�
void BLE_PROTOCOL_SendCmdControlAck(uint8 controlCmd, uint8 paramH, uint8 paramL)
{
	uint8 val1,val2;
	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_CONTROL_UP); 	
	BLE_PROTOCOL_TxAddData(0x03);	

	switch (controlCmd)
	{
		case LIGHT:
			if(0x00 == paramL)
			{
				// �ص�
				PARAM_SetLightSwitch(FALSE);	
			}
			else
			{
				// ����
				PARAM_SetLightSwitch(TRUE);	
			}
			val1 = 0;
			val2 = paramL;

		break;
		
		case AUTO_SHUTDOWN_TIME:
			PARAM_SetPowerOffTime(paramL);

			TIMER_KillTask(TIMER_ID_AUTO_POWER_OFF);
		
			// �����Զ��ػ�����
			STATE_ResetAutoPowerOffControl();
		
			// NVM���£�����д��
			NVM_SetDirtyFlag(TRUE);
			val1 = 0;
			val2 = paramL;
		break;
		
		case SPEED_LIMIT:
			PARAM_SetSpeedLimit((uint16)paramL * 10);

			// NVM���£�����д��
			NVM_SetDirtyFlag(TRUE);
			val1 = 0;
			val2 = paramL;
		break;
		
		case UNIT:
			PARAM_SetUnit((UNIT_E)paramL);

			// NVM���£�����д��
			NVM_SetDirtyFlag(TRUE);
			val1 = 0;
			val2 = paramL;
		break;
		
		case ASSIST:
			
			// ��ȡ��ǰ����ֵ
			val1 = (uint8)paramCB.runtime.assist;
		
			if(0 == paramL)
			{
				if(val1 > 0)
				{
					val1--;
					PARAM_SetAssistLevel((ASSIST_ID_E)val1);
				}
			}
			else
			{
				if(val1 < PARAM_GetMaxAssist())
				{
					val1++;
					PARAM_SetAssistLevel((ASSIST_ID_E)val1);
				}
			}
			val1 = 0;
			val2 = (uint8)paramCB.runtime.assist;
		break;
		
		case LOCK:
			val1 = 0;
			val2 = 0;
		break;

		case BACKLIGHT:
		
			if (paramL > 5)
			{
				paramL = 5;
				
			}
			// ���ı�������
			PARAM_SetBrightness(paramL);
		
			// NVM���£�����д��
			NVM_SetDirtyFlag(TRUE);
			val1 = 0;
			val2 = paramL;
		break;
		default:
		break;
	}
	BLE_PROTOCOL_TxAddData(controlCmd);	
	BLE_PROTOCOL_TxAddData(val1);	
	BLE_PROTOCOL_TxAddData(val2);

	BLE_PROTOCOL_TxAddFrame();	
}

// �Ǳ�λ��Ϣ�ϱ�
void BLE_PROTOCOL_SendAssistInfor(uint8 param)
{
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_CONTROL_UP); 	
	BLE_PROTOCOL_TxAddData(0x03);	
	
	BLE_PROTOCOL_TxAddData(ASSIST);	
	BLE_PROTOCOL_TxAddData(0x00);	
	BLE_PROTOCOL_TxAddData((ASSIST_ID_E)param);

	BLE_PROTOCOL_TxAddFrame();	
}


// �Ǳ��ȡʵʱ�˶�״̬�ظ�
void BLE_PROTOCOL_SendCmdGetRuningStateAck(void)
{
	uint8 val1, val2, val3, val4;
	uint16 valTmp;
	uint32 valTemp;

	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_GETDATA_RUNNING_UP); 	
	BLE_PROTOCOL_TxAddData(0x0B);	

	// ʵʱ����
	valTmp = PARAM_GetPower();
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// �����
	valTmp = PARAM_GetPowerMax();
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ������λ
	val1 = (uint8)PARAM_GetAssistLevel();
	BLE_PROTOCOL_TxAddData(val1);

	// ʵʱ�ٶ�
	valTmp = paramCB.runtime.speed;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// �����
	valTemp = paramCB.nvm.param.common.record.total.distance;
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);

	BLE_PROTOCOL_TxAddFrame();	
}


// �Ǳ��ϱ���ʷ����
void BLE_PROTOCOL_SendHistoryData(uint32 param)
{
	uint8 val1, val2, val3, val4;
	uint16 valTmp;
	uint32 valTemp;
	TIME_E timeTemp;
	
	RECORD_ITEM_CB record;

	if(!RECORD_ReadFromNVM(&record))
	{
		return;
	}

	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_REPORT_HISTORICAL_DATA_UP); 	
	BLE_PROTOCOL_TxAddData(0x0A);	

	// �˶�����,��ȷ���켴��
	timeTemp.year = (uint16)record.param.dateTime.year + 2000;
	timeTemp.month = record.param.dateTime.month;
	timeTemp.day= record.param.dateTime.day;
	timeTemp.hour= record.param.dateTime.hour;
	timeTemp.minute = record.param.dateTime.min;
	timeTemp.second = record.param.dateTime.sec;
	valTemp = DS1302_GetTimeStampFromDateTime(timeTemp) - 8 * 3600;//Unixʱ���-8H �ϱ�����������ת��Ϊ����ʱ��
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);
	
	// �˶�ʱ��
	val1 = (uint8)(record.param.tripTimer >> 8);
	val2 = (uint8)(record.param.tripTimer & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ����ٶ�
	valTmp = record.param.speedMax;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ��·��
	valTmp = record.param.calories;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ����ʱ���
	val1 = record.param.timeSlot;
	BLE_PROTOCOL_TxAddData(val1);

	// ����ʱ������
	valTemp = record.param.trip;
	val1 = (uint8)((valTemp >> 8) & 0xFF);
	val2 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	BLE_PROTOCOL_TxAddFrame();	
}

// ���ͻ�ȡMAC��ַ
void BLE_PROTOCOL_SendGetMacAddr(uint32 param)
{
	uint8 bleMacStr[] = "TTM:MAC-?";
	
	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	// ��������������ӵ�����㻺����
	BLE_UART_AddTxArray(0xFFFF, bleMacStr, sizeof(bleMacStr));
}


// �Ǳ�״̬��1��Ϣ�ظ�
void BLE_PROTOCOL_SendCmdSelectStatus1Ack(void)
{
	uint8 val1, val2, val3, val4;
	uint16 valTmp;
	uint32 valTemp;

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_SELECT_GETSTATUS_UP); 	
	BLE_PROTOCOL_TxAddData(0x16);

	// ״̬��1���
	BLE_PROTOCOL_TxAddData(0x01);	

	// ���������
	valTemp = paramCB.nvm.param.common.record.total.distance;
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);

	// ��ʱ��
	valTemp = PARAM_GetTotalRidingTime();
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);

	// �ܿ�·��
	valTemp = PARAM_GetTotalCalories();
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);

	// ����ٶ�
	valTmp = paramCB.nvm.param.common.record.trip.speedMax;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ƽ���ٶ�
	valTmp = paramCB.runtime.trip.speedAverage;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// �����ٷֱ�
	val1 = PARAM_GetBatteryPercent();
	BLE_PROTOCOL_TxAddData(val1);

	// ������λ
	val1 = (uint8)PARAM_GetAssistLevel();
	BLE_PROTOCOL_TxAddData(val1);

	// ���״̬
	val1 = (uint8)PARAM_GetLightSwitch();
	BLE_PROTOCOL_TxAddData(val1);

	// ��ʾ��λ
	val1 = PARAM_GetUnit();
	BLE_PROTOCOL_TxAddData(val1);
	
	// ����״̬
	BLE_PROTOCOL_TxAddData(0x00);

	BLE_PROTOCOL_TxAddFrame();	
}

// �Ǳ�״̬��2��Ϣ�ظ�
void BLE_PROTOCOL_SendCmdSelectStatus2Ack(void)
{
	uint8 val1;

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_SELECT_GETSTATUS_UP); 	
	BLE_PROTOCOL_TxAddData(0x0E);	
	
	// ״̬��2���
	BLE_PROTOCOL_TxAddData(0x02);	

	// �Ǳ�������
	val1 = PARAM_GetBrightness();
	BLE_PROTOCOL_TxAddData(val1);
	
	// �Զ��ػ�ʱ��
	val1 = PARAM_GetPowerOffTime();
	BLE_PROTOCOL_TxAddData(val1);

	// ����ֵ
	val1 = paramCB.nvm.param.common.speed.limitVal/10;
	BLE_PROTOCOL_TxAddData(val1);
	
	// ������ʽ
	BLE_PROTOCOL_TxAddData(0x01);
	
	// Ѳ��ģʽ
	BLE_PROTOCOL_TxAddData(0x01);
	
	BLE_PROTOCOL_TxAddFrame();	
}

// �Ǳ�״̬��3��Ϣ�ظ�
void BLE_PROTOCOL_SendCmdSelectStatus3Ack(void)
{
	int i = 0;
	
	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_SELECT_GETSTATUS_UP); 	
	BLE_PROTOCOL_TxAddData(0x2A);	
	
	// ״̬��3���
	BLE_PROTOCOL_TxAddData(0x03);	

	// ���¾��ǲ������
	// ������к�
	for (i = 0;i < 32;i++)
	{
		BLE_PROTOCOL_TxAddData(0);
	}
	
	// ��س�ŵ����
	BLE_PROTOCOL_TxAddData(0);
	BLE_PROTOCOL_TxAddData(0);
	
	// ��������
	BLE_PROTOCOL_TxAddData(0);
	BLE_PROTOCOL_TxAddData(0);

	// �������
	BLE_PROTOCOL_TxAddData(0);
	BLE_PROTOCOL_TxAddData(0);

	// ��������
	BLE_PROTOCOL_TxAddData(0);
	BLE_PROTOCOL_TxAddData(0);

	// ʣ������
	BLE_PROTOCOL_TxAddData(0);
	BLE_PROTOCOL_TxAddData(0);
	
	BLE_PROTOCOL_TxAddFrame();	
}

uint8 MAC_ASCII_To_Byte(uint8 param1, uint8 param2)
{
	uint8 ret1 = 0;
	uint8 ret2 = 0;
	
	if ((param1 >= '0') && (param1 <= '9'))
	{
		ret1 = ((param1 - '0') << 4)&0xF0;
	}
	else if ((param1 >= 'A') && (param1 <= 'Z'))
	{
		ret1 = ((param1 - 'A' + 10) << 4)&0xF0;
	}
	
	if ((param2 >= '0') && (param2 <= '9'))
	{
		ret2 = (param2 - '0')&0x0F;
	}
	else  if ((param2 >= 'A') && (param2 <= 'Z'))
	{
		ret2 = (param2 - 'A' + 10)&0x0F;
	}
	
	return (ret1 + ret2);
}

// crc8
uint8 okay_get_checksum(uint8 * ptr, uint8 len)
{
	uint8 crc; 
	uint8 i; 
	crc = 0; 


	while(len--)
	{
		crc ^= *ptr++; 
		for(i=0; i<8; i++)
		{
			if(crc&0x01)
				crc = (crc>>1) ^ 0x8C;
			else 
				crc >>= 1;
		}
	}
	return crc;
}

// ���͹㲥���ݰ�
void BLE_PROTOCOL_SendAdv(uint32 param)
{
	uint8 i;
	uint8 cc;
	uint8 bleAdvStr[] = "TTM:ADD-";
	uint8 senBleAdvdBuf[24] = {0};
	
	if (0 == param)
	{
		// �㲥�������ݰ�
		senBleAdvdBuf[0] = 'T';
		senBleAdvdBuf[1] = 'T';
		senBleAdvdBuf[2] = 'M';
		senBleAdvdBuf[3] = ':';
		senBleAdvdBuf[4] = 'A';
		senBleAdvdBuf[5] = 'D';
		senBleAdvdBuf[6] = 'D';
		senBleAdvdBuf[7] = '-';
		
		// MAC ��ַ
		senBleAdvdBuf[8] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[0], bleUartCB.rx.macBuff[1]);
		senBleAdvdBuf[9] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[3], bleUartCB.rx.macBuff[4]);
		senBleAdvdBuf[10] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[6], bleUartCB.rx.macBuff[7]);
		senBleAdvdBuf[11] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[9], bleUartCB.rx.macBuff[10]);
		senBleAdvdBuf[12] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[12], bleUartCB.rx.macBuff[13]);
		senBleAdvdBuf[13] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[15], bleUartCB.rx.macBuff[16]);
		
		//55 'A' '1' 01 00 00 25 35 07 55
		senBleAdvdBuf[14] = 0x55;
		senBleAdvdBuf[15] =  paramCB.nvm.param.protocol.carModel[0];
		senBleAdvdBuf[16] =  paramCB.nvm.param.protocol.carModel[1];
		senBleAdvdBuf[17] = '0';
		senBleAdvdBuf[18] = 0x00;
		senBleAdvdBuf[19] = 0x00;
		senBleAdvdBuf[20] = 0x25;
		//senBleAdvdBuf[21] = 0x35;
		senBleAdvdBuf[21] = okay_get_checksum(&senBleAdvdBuf[15],6);
		
		senBleAdvdBuf[22] = 0x07;
		senBleAdvdBuf[23] = 0x55;
		// ��������������ӵ�����㻺����
		BLE_UART_AddTxArray(0xFFFF, senBleAdvdBuf, 24);
	}
	else
	{
		// ��λ�㲥���ݰ������������ݰ���
		senBleAdvdBuf[0] = 'T';
		senBleAdvdBuf[1] = 'T';
		senBleAdvdBuf[2] = 'M';
		senBleAdvdBuf[3] = ':';
		senBleAdvdBuf[4] = 'A';
		senBleAdvdBuf[5] = 'D';
		senBleAdvdBuf[6] = 'D';
		senBleAdvdBuf[7] = '-';
		senBleAdvdBuf[8] = 0x18;
		senBleAdvdBuf[9] = 0x03;
		
		// MAC ��ַ
		senBleAdvdBuf[10] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[0], bleUartCB.rx.macBuff[1]);
		senBleAdvdBuf[11] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[3], bleUartCB.rx.macBuff[4]);
		senBleAdvdBuf[12] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[6], bleUartCB.rx.macBuff[7]);
		senBleAdvdBuf[13] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[9], bleUartCB.rx.macBuff[10]);
		senBleAdvdBuf[14] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[12], bleUartCB.rx.macBuff[13]);
		senBleAdvdBuf[15] = MAC_ASCII_To_Byte(bleUartCB.rx.macBuff[15], bleUartCB.rx.macBuff[16]);
		
		senBleAdvdBuf[16] = 0x01;
		senBleAdvdBuf[17] = 0x05;
		senBleAdvdBuf[18] = 0x00;
		senBleAdvdBuf[19] = 0x01;
		senBleAdvdBuf[20] = 0x00;
		senBleAdvdBuf[21] = 0x00;
		BLE_UART_AddTxArray(0xFFFF, senBleAdvdBuf, 22);
	}
}

// ���������̼�����Ӧ��
void BLE_PROTOCOL_SendCmdUpDataAck(uint8 ackParam, uint8 ackParam1)
{
	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_FIRMUPDATE_UP); 	
	BLE_PROTOCOL_TxAddData(0x03);	
	
	BLE_PROTOCOL_TxAddData(ackParam);
	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_UPDATA_LENGTH_MAX);
	BLE_PROTOCOL_TxAddData(ackParam1);
	BLE_PROTOCOL_TxAddFrame();	
}

// ���Ǳ��ϱ���ʷ����
void BLE_PROTOCOL_SendHistoryData_NEW(uint32 param)
{
/*	uint8 val1, val2, val3, val4 , i;
	uint16 valTmp;
	uint32 valTemp;
	TIME_E timeTemp;
	
	NEW_RECORD_ITEM_CB record_new;

	if(!RECORD_ReadFromNVM(&record_new))
	{
		return;
	}

	BLE_PROTOCOL_TxAddData(BLE_PROTOCOL_CMD_HEAD); 		
	BLE_PROTOCOL_TxAddData(BLE_CMD_REPORT_HISTORICAL_DATA_UP_NEW); 	
	
	BLE_PROTOCOL_TxAddData(record_new.param.p_Arr+22);	 //���ݳ�����Ҫ����

	// ����ʱ��
	timeTemp.year = (uint16)record_new.param.powerontime.year;
	timeTemp.month = record_new.param.powerontime.month;
	timeTemp.day= record_new.param.powerontime.day;
	timeTemp.hour= record_new.param.powerontime.hour;
	timeTemp.minute = record_new.param.powerontime.min;
	timeTemp.second = record_new.param.powerontime.sec;
	valTemp = DS1302_GetTimeStampFromDateTime(timeTemp) - 8 * 3600;//Unixʱ���-8H �ϱ�����������ת��Ϊ����ʱ��
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);
	
	// �˶�����ʱ��
	timeTemp.year = (uint16)record_new.param.powerofftime.year;
	timeTemp.month = record_new.param.powerofftime.month;
	timeTemp.day= record_new.param.powerofftime.day;
	timeTemp.hour= record_new.param.powerofftime.hour;
	timeTemp.minute = record_new.param.powerofftime.min;
	timeTemp.second = record_new.param.powerofftime.sec;
	valTemp = DS1302_GetTimeStampFromDateTime(timeTemp) - 8 * 3600; // Unixʱ���-8H �ϱ�����������ת��Ϊ����ʱ��
	val1 = (uint8)((valTemp >> 24) & 0xFF);
	val2 = (uint8)((valTemp >> 16) & 0xFF);
	val3 = (uint8)((valTemp >> 8) & 0xFF);
	val4 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	BLE_PROTOCOL_TxAddData(val3);
	BLE_PROTOCOL_TxAddData(val4);
	
	// �˶�ʱ��
	val1 = (uint8)(record_new.param.ridingTime >> 8);
	val2 = (uint8)(record_new.param.ridingTime & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ����ٶ�
	valTmp = record_new.param.speedMax;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ƽ���ٶ�
	valTmp = (float)record_new.param.trip * 3600.f / record_new.param.ridingTime;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ��·��
	valTmp = record_new.param.calories;
	val1 = ((valTmp >> 8) & 0xFF);
	val2 = (valTmp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);

	// ��Լ��
	val1 = (uint8)record_new.param.trip * 0.0136f;
	BLE_PROTOCOL_TxAddData(val1);
	
	// ��Լ̼�ŷ�
	valTemp = record_new.param.trip * 0.174f;
	val1 = (uint8)((valTemp >> 8) & 0xFF);
	val2 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	
	// �������
	valTemp = record_new.param.trip;
	val1 = (uint8)((valTemp >> 8) & 0xFF);
	val2 = (uint8)(valTemp & 0xFF);
	BLE_PROTOCOL_TxAddData(val1);
	BLE_PROTOCOL_TxAddData(val2);
	
	// ÿ10�����������
	for(i =0 ; i < record_new.param.p_Arr+1; i++)
	{
		BLE_PROTOCOL_TxAddData(record_new.param.Arr[i]);
	}
	
	BLE_PROTOCOL_TxAddFrame();*/
}

