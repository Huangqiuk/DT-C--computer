#include "common.h"
#include "system.h"
#include "timer.h"
#include "delay.h"
#include "state.h"
#include "param.h"
#include "segment.h"
#include "iap.h"
#include "uartprotocol.h"

// 状态机结构体定义
STATE_CB stateCB;

void STATE_FunctionTest(uint32 param);

// 状态机初始化
void STATE_Init(void)
{
	// 默认状态为空
	stateCB.state = STATE_NULL;
	stateCB.preState = STATE_NULL;

	stateCB.charingValue = 0;
	
	
}




#define EXCHANGE_H4B_L4B(s)  (((s << 4)&0xf0) + (s >> 4))



void MultimeterSendMsgToMaistr(uint8 cmd,uint8 *msg,uint16 length)
{
	
	//	命令头1
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD1);
	//	命令头2
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD2);
	//	命令头3
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD3);
	//	命令字
	UART_PROTOCOL_TxAddData(cmd);
	// 	长度，长度会在UART2_TxAddData函数里自动更新
	UART_PROTOCOL_TxAddData(0);
	//	增加发送目的单板编号
	UART_PROTOCOL_TxAddData(BOARD_ARM);
	
	//	写入数据
	for(uint8_t i = 0; i < length; ++i)
	{
		UART_PROTOCOL_TxAddData(msg[i]);
	}
	
	// 组装命令帧
	UART_PROTOCOL_TxAddFrame();
		
		
}

// 询问板卡类型应答
void UartCmdAskBoardTypeACK(uint8 dataDir)
{
	//uint8 sendBuf[6];
	//MultimeterSendMsgToMaistr(UART_CMD_ASK_BOARD_TYPE_ACK,dataDir,NULL,6);
}
// 重启应答
void UartCmdResetACK(uint8 state,uint8 dataDir)
{
	//uint8 sendBuf[1];
	//sendBuf[0] = state;
	//MultimeterSendMsgToMaistr(UART_CMD_RESTART_ACK,dataDir,sendBuf,1);
}


// 系统总复位
//void SYSTEM_Rst(uint32 param)
//{
//	//__disable_fault_irq();
//	NVIC_SystemReset();
//}

