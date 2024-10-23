#include "common.h"
#include "system.h"
#include "timer.h"
#include "delay.h"
#include "state.h"
#include "param.h"
#include "segment.h"
#include "iap.h"
#include "uartprotocol.h"

// ״̬���ṹ�嶨��
STATE_CB stateCB;

void STATE_FunctionTest(uint32 param);

// ״̬����ʼ��
void STATE_Init(void)
{
	// Ĭ��״̬Ϊ��
	stateCB.state = STATE_NULL;
	stateCB.preState = STATE_NULL;

	stateCB.charingValue = 0;
	
	
}




#define EXCHANGE_H4B_L4B(s)  (((s << 4)&0xf0) + (s >> 4))



void MultimeterSendMsgToMaistr(uint8 cmd,uint8 *msg,uint16 length)
{
	
	//	����ͷ1
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD1);
	//	����ͷ2
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD2);
	//	����ͷ3
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD3);
	//	������
	UART_PROTOCOL_TxAddData(cmd);
	// 	���ȣ����Ȼ���UART2_TxAddData�������Զ�����
	UART_PROTOCOL_TxAddData(0);
	//	���ӷ���Ŀ�ĵ�����
	UART_PROTOCOL_TxAddData(BOARD_ARM);
	
	//	д������
	for(uint8_t i = 0; i < length; ++i)
	{
		UART_PROTOCOL_TxAddData(msg[i]);
	}
	
	// ��װ����֡
	UART_PROTOCOL_TxAddFrame();
		
		
}

// ѯ�ʰ忨����Ӧ��
void UartCmdAskBoardTypeACK(uint8 dataDir)
{
	//uint8 sendBuf[6];
	//MultimeterSendMsgToMaistr(UART_CMD_ASK_BOARD_TYPE_ACK,dataDir,NULL,6);
}
// ����Ӧ��
void UartCmdResetACK(uint8 state,uint8 dataDir)
{
	//uint8 sendBuf[1];
	//sendBuf[0] = state;
	//MultimeterSendMsgToMaistr(UART_CMD_RESTART_ACK,dataDir,sendBuf,1);
}


// ϵͳ�ܸ�λ
//void SYSTEM_Rst(uint32 param)
//{
//	//__disable_fault_irq();
//	NVIC_SystemReset();
//}

