#include "common.h"
#include "timer.h"
#include "uartDrive.h"
#include "uartProtocol.h"
#include "state.h"
#include "spiFlash.h"
#include "ArmUart.h"
#include "DutInfo.h"
#include "dutCtl.h"
#include "uartProtocol3.h"
#include "DtaUartProtocol.h"

/******************************************************************************
 * 【内部接口声明】
 ******************************************************************************/

// 数据结构初始化
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB *pCB);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB *pCB);

// UART命令帧缓冲区处理
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB *pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB *pCB);

// 协议层发送处理过程
void UART_PROTOCOL_TxStateProcess(void);

// UART协议层向驱动层注册数据发送接口
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length));

// 发送命令带结果
void UART_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result);

// 发送命令无结果
void UART_PROTOCOL_SendCmdNoResult(uint8 cmdWord);

//==================================================================================
// 速度平滑滤波处理
void UART_PROTOCOL_CALLBACK_SpeedFilterProcess(uint32 param);

// 发送命令带结果
void UART_PROTOCOL_SendLdoV(uint8 cmdWord, uint32 result);

// 全局变量定义
UART_PROTOCOL_CB uartProtocolCB;

// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

// 协议初始化
void UART_PROTOCOL_Init(void)
{
	// 协议层数据结构初始化
	UART_PROTOCOL_DataStructInit(&uartProtocolCB);

	// 向驱动层注册数据接收接口
	// UART_DRIVE_RegisterDataSendService(UART_PROTOCOL_MacProcess);

	// 向驱动层注册数据发送接口
	UART_PROTOCOL_RegisterDataSendService(UART_DRIVE_AddTxArray);

	// 注册间隔参数设置时间
	//	TIMER_AddTask(TIMER_ID_PROTOCOL_55_PARAM_TX,
	//					300,
	//					UART_PROTOCOL_SendCmdAging,
	//					TRUE,
	//					TIMER_LOOP_FOREVER,
	//					ACTION_MODE_ADD_TO_QUEUE);
}

// UART协议层过程处理
void UART_PROTOCOL_Process(void)
{
	// 55升级协议
	//  UART接收FIFO缓冲区处理
	UART_PROTOCOL_RxFIFOProcess(&uartProtocolCB);

	// UART接收命令缓冲区处理
	UART_PROTOCOL_CmdFrameProcess(&uartProtocolCB);

	// UART协议层发送处理过程
	UART_PROTOCOL_TxStateProcess();

	// 3A配置协议
	// UART接收FIFO缓冲区处理
	UART_PROTOCOL3_RxFIFOProcess(&uartProtocolCB3);

	// UART接收命令缓冲区处理
	UART_PROTOCOL3_CmdFrameProcess(&uartProtocolCB3); // km5s协议，在app中升级config

	// UART协议层发送处理过程
	UART_PROTOCOL3_TxStateProcess();

    // DTA测试协议
	//  UART接收FIFO缓冲区处理
	DTA_UART_PROTOCOL_RxFIFOProcess(&dtaUartProtocolCB);

	// UART接收命令缓冲区处理
	DTA_UART_PROTOCOL_CmdFrameProcess(&dtaUartProtocolCB);

	// UART协议层发送处理过程
	DTA_UART_PROTOCOL_TxStateProcess();    
}

// 向发送命令帧队列中添加数据
void UART_PROTOCOL_TxAddData(uint8 data)
{
	uint16 head = uartProtocolCB.tx.head;
	uint16 end = uartProtocolCB.tx.end;
	UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];

	// 发送缓冲区已满，不予接收
	if ((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// 队尾命令帧已满，退出
	if (pCmdFrame->length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}
	// 数据添加到帧末尾，并更新帧长度
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void UART_PROTOCOL_TxAddFrame(void)
{
	uint16 cc = 0;
	uint16 i = 0;
	uint16 head = uartProtocolCB.tx.head;
	uint16 end = uartProtocolCB.tx.end;
	UART_PROTOCOL_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB.tx.cmdQueue[uartProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// 发送缓冲区已满，不予接收
	if ((end + 1) % UART_PROTOCOL_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// 命令帧长度不足，清除已填充的数据，退出
	if (UART_PROTOCOL_CMD_FRAME_LENGTH_MIN - 1 > length) // 减去"校验和"1个字节
	{
		pCmdFrame->length = 0;

		return;
	}

	// 队尾命令帧已满，退出
	if (length >= UART_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX)
	{
		return;
	}

	// 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
	pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] = length - 3; // 重设数据长度

	for (i = 0; i < length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = ~cc;

	uartProtocolCB.tx.end++;
	uartProtocolCB.tx.end %= UART_PROTOCOL_TX_QUEUE_SIZE;

	// pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改
}

void UART_PROTOCOL_TxAddFrame_3A(void)
{
	uint16 checkSum = 0;
	uint16 i = 0;
	uint16 head = uartProtocolCB3.tx.head;
	uint16 end = uartProtocolCB3.tx.end;
	UART_PROTOCOL3_TX_CMD_FRAME *pCmdFrame = &uartProtocolCB3.tx.cmdQueue[uartProtocolCB.tx.end];
	uint16 length = pCmdFrame->length;

	// 发送缓冲区已满，不予接收
	if ((end + 1) % UART_PROTOCOL3_TX_QUEUE_SIZE == head)
	{
		return;
	}

	// 命令帧长度不足，清除已填充的数据，退出
	if (UART_PROTOCOL3_CMD_FRAME_LENGTH_MIN - 4 > length) // 减去"校验和L、校验和H、结束标识0xD、结束标识OxA"4个字节
	{
		pCmdFrame->length = 0;

		return;
	}

	// 队尾命令帧已满，退出
	if ((length >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX) || (length + 1 >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX) || (length + 2 >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX) || (length + 3 >= UART_PROTOCOL3_TX_CMD_FRAME_LENGTH_MAX))
	{
		return;
	}

	// 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
	pCmdFrame->buff[UART_PROTOCOL3_CMD_LENGTH_INDEX] = length - 4; // 重设数据长度，减去"命令头、设备地址、命令字、数据长度"4个字节
	for (i = 1; i < length; i++)
	{
		checkSum += pCmdFrame->buff[i];
	}
	pCmdFrame->buff[pCmdFrame->length++] = (checkSum & 0x00FF);		   // 低字节在前
	pCmdFrame->buff[pCmdFrame->length++] = ((checkSum >> 8) & 0x00FF); // 高字节在后

	// 结束标识
	pCmdFrame->buff[pCmdFrame->length++] = 0x0D;
	pCmdFrame->buff[pCmdFrame->length++] = 0x0A;

	uartProtocolCB3.tx.end++;
	uartProtocolCB3.tx.end %= UART_PROTOCOL3_TX_QUEUE_SIZE;
}

// 数据结构初始化
void UART_PROTOCOL_DataStructInit(UART_PROTOCOL_CB *pCB)
{
	uint16 i;

	// 参数合法性检验
	if (NULL == pCB)
	{
		return;
	}

	pCB->tx.txBusy = FALSE;
	pCB->tx.index = 0;
	pCB->tx.head = 0;
	pCB->tx.end = 0;
	for (i = 0; i < UART_PROTOCOL_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	pCB->rxFIFO.head = 0;
	pCB->rxFIFO.end = 0;
	pCB->rxFIFO.currentProcessIndex = 0;

	pCB->rx.head = 0;
	pCB->rx.end = 0;
	for (i = 0; i < UART_PROTOCOL_RX_QUEUE_SIZE; i++)
	{
		pCB->rx.cmdQueue[i].length = 0;
	}
}

// UART报文接收处理函数(注意根据具体模块修改)
void UART_PROTOCOL_MacProcess(uint16 standarID, uint8 *pData, uint16 length)
{
	uint16 end = uartProtocolCB.rxFIFO.end;
	uint16 head = uartProtocolCB.rxFIFO.head;
	uint8 rxdata = 0x00;

	// 接收数据
	rxdata = *pData;

	// 一级缓冲区已满，不予接收
	if ((end + 1) % UART_PROTOCOL_RX_FIFO_SIZE == head)
	{
		return;
	}
	// 一级缓冲区未满，接收
	else
	{
		// 将接收到的数据放到临时缓冲区中
		uartProtocolCB.rxFIFO.buff[end] = rxdata;
		uartProtocolCB.rxFIFO.end++;
		uartProtocolCB.rxFIFO.end %= UART_PROTOCOL_RX_FIFO_SIZE;
	}
}

// UART协议层向驱动层注册数据发送接口
void UART_PROTOCOL_RegisterDataSendService(BOOL (*service)(uint16 id, uint8 *pData, uint16 length))
{
	uartProtocolCB.sendDataThrowService = service;
}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL UART_PROTOCOL_ConfirmTempCmdFrameBuff(UART_PROTOCOL_CB *pCB)
{
	UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	// 参数合法性检验
	if (NULL == pCB)
	{
		return FALSE;
	}

	// 临时缓冲区为空，不予添加
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	if (0 == pCmdFrame->length)
	{
		return FALSE;
	}

	// 添加
	pCB->rx.end++;
	pCB->rx.end %= UART_PROTOCOL_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0; // 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区

	return TRUE;
}

// 协议层发送处理过程
void UART_PROTOCOL_TxStateProcess(void)
{
	uint16 head = uartProtocolCB.tx.head;
	uint16 end = uartProtocolCB.tx.end;
	uint16 length = uartProtocolCB.tx.cmdQueue[head].length;
	uint8 *pCmd = uartProtocolCB.tx.cmdQueue[head].buff;
	uint16 localDeviceID = uartProtocolCB.tx.cmdQueue[head].deviceID;

	// 发送缓冲区为空，说明无数据
	if (head == end)
	{
		return;
	}

	// 发送函数没有注册直接返回
	if (NULL == uartProtocolCB.sendDataThrowService)
	{
		return;
	}

	// 协议层有数据需要发送到驱动层
	if (!(*uartProtocolCB.sendDataThrowService)(localDeviceID, pCmd, length))
	{
		return;
	}

	// 发送环形队列更新位置
	uartProtocolCB.tx.cmdQueue[head].length = 0;
	uartProtocolCB.tx.head++;
	uartProtocolCB.tx.head %= UART_PROTOCOL_TX_QUEUE_SIZE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void UART_PROTOCOL_RxFIFOProcess(UART_PROTOCOL_CB *pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;
	uint16 length = 0;
	uint8 currentData = 0;

	// 参数合法性检验
	if (NULL == pCB)
	{
		return;
	}

	// 一级缓冲区为空，退出
	if (head == end)
	{
		return;
	}

	// 获取临时缓冲区指针
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	// 取出当前要处理的字节
	currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];

	// 临时缓冲区长度为0时，搜索首字节
	if (0 == pCmdFrame->length)
	{
		// 命令头错误，删除当前字节并退出
		if (UART_PROTOCOL_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// 命令头正确，但无临时缓冲区可用，退出
		if ((pCB->rx.end + 1) % UART_PROTOCOL_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// 添加UART通讯超时时间设置-2016.1.5增加
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL,
					  UART_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT,
					  UART_PROTOCOL_CALLBACK_RxTimeOut,
					  0,
					  1,
					  ACTION_MODE_ADD_TO_QUEUE);
#endif

		// 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
		pCmdFrame->buff[pCmdFrame->length++] = currentData;
		pCB->rxFIFO.currentProcessIndex++;
		pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;
	}
	// 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
	else
	{
		// 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
		if (pCmdFrame->length >= UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX)
		{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			UART_PROTOCOL_StopRxTimeOutCheck();
#endif

			// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
			pCmdFrame->length = 0; // 2016.1.5增加
			// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
			pCB->rxFIFO.head++;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// 一直取到末尾
		while (end != pCB->rxFIFO.currentProcessIndex)
		{
			// 取出当前要处理的字节
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			// 缓冲区未溢出，正常接收，将数据添加到临时缓冲区中
			pCmdFrame->buff[pCmdFrame->length++] = currentData;
			pCB->rxFIFO.currentProcessIndex++;
			pCB->rxFIFO.currentProcessIndex %= UART_PROTOCOL_RX_FIFO_SIZE;

			// ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■

			// 首先判断命令帧最小长度，一个完整的命令字至少包括8个字节: 命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，因此不足8个字节的必定不完整
			if (pCmdFrame->length < UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 继续接收
				continue;
			}

			// 命令帧长度数值越界，说明当前命令帧错误，停止接收
			if (pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] > UART_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX - UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				UART_PROTOCOL_StopRxTimeOutCheck();
#endif

				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// 命令帧长度校验，在命令长度描述字的数值上，增加命令头、设备地址、命令字、数据长度、校验和L、校验和H、结束标识0xD、结束标识OxA，即为命令帧实际长度
			length = pCmdFrame->length;
			if (length < pCmdFrame->buff[UART_PROTOCOL_CMD_LENGTH_INDEX] + UART_PROTOCOL_CMD_FRAME_LENGTH_MIN)
			{
				// 长度要求不一致，说明未接收完毕，退出继续
				continue;
			}

			// 命令帧长度OK，则进行校验，失败时删除命令头
			if (!UART_PROTOCOL_CheckSUM(pCmdFrame))
			{
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				UART_PROTOCOL_StopRxTimeOutCheck();
#endif

				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head++;
				pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			UART_PROTOCOL_StopRxTimeOutCheck();
#endif

			// 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
			pCB->rxFIFO.head += length;
			pCB->rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			UART_PROTOCOL_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// 对传入的命令帧进行校验，返回校验结果
BOOL UART_PROTOCOL_CheckSUM(UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame)
{
	uint8 cc = 0;
	uint16 i = 0;

	if (NULL == pCmdFrame)
	{
		return FALSE;
	}

	// 从命令头开始，到校验码之前的一个字节，依次进行异或运算
	for (i = 0; i < pCmdFrame->length - 1; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}

	cc = ~cc;

	// 判断计算得到的校验码与命令帧中的校验码是否相同
	if (pCmdFrame->buff[pCmdFrame->length - 1] != cc)
	{
		return FALSE;
	}

	return TRUE;
}

void setDutConfig(uint32 temp)
{
	/*发送dut配置信息*/
	uint8 configs[100] = {0};
	uint8 i;

	SPI_FLASH_ReadArray(configs, SPI_FLASH_DUT_CONFIG_ADDEESS, 60); // 实际只有54个字节

	// DUT上电
	DUTCTRL_PowerOnOff(1);

	// 添加命令头
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL3_CMD_HEAD);

	// 添加设备地址
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// 添加命令字
	UART_PROTOCOL3_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM); // 0xc0

	// 添加数据长度
	UART_PROTOCOL3_TxAddData(54);

	// 写入结果
	// UART_PROTOCOL3_TxAddData(1);//背光亮度1，看现象
	for (i = 0; i < 54; i++)
	{
		UART_PROTOCOL3_TxAddData(configs[i]);
	}
	// 添加检验和与结束符，并添加至发送
	UART_PROTOCOL3_TxAddFrame();
}

void uartProtocol_SendOnePacket(uint32 flashAddr, uint32 addr)
{
	uint8 addr1, addr2, addr3, addr4;
	uint8 appUpdateOnePacket[150] = {0};
	uint8 appUpdateOnePacket_i = 0;
	addr1 = (addr & 0xFF000000) >> 24;
	addr2 = (addr & 0x00FF0000) >> 16;
	addr3 = (addr & 0x0000FF00) >> 8;
	addr4 = (addr & 0x000000FF);
	SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + addr, 128); // 工具读取128字节

	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD); // 头
	if (dut_info.appUpFlag)
	{
		UART_PROTOCOL_TxAddData(UART_CMD_UP_APP_UP); // 命令0x06
	}
	else if (dut_info.uiUpFlag)
	{
		UART_PROTOCOL_TxAddData(UART_CMD_DUT_UI_DATA_WRITE); // 命令0x14
	}
	UART_PROTOCOL_TxAddData(0x00);													   // 长度暂时为0
	UART_PROTOCOL_TxAddData(addr1);													   // 添加地址
	UART_PROTOCOL_TxAddData(addr2);													   // 添加地址
	UART_PROTOCOL_TxAddData(addr3);													   // 添加地址
	UART_PROTOCOL_TxAddData(addr4);													   // 添加地址
	for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 128; appUpdateOnePacket_i++) // 添加数据
	{
		UART_PROTOCOL_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i]);
	}
	UART_PROTOCOL_TxAddFrame(); // 调整帧格式,修改长度和添加校验
}

void uartProtocol_SendOnePacket_Hex(uint32 flashAddr)
{
	uint8 appUpdateOnePacket[150] = {0};
	uint8 appUpdateOnePacket_i = 0;															 // app起始地址
	SPI_FLASH_ReadArray(appUpdateOnePacket, flashAddr + dut_info.currentAppSize * 116, 116); // 工具读取 4+112 个字节
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);										 // 添加命令头;//头
	UART_PROTOCOL_TxAddData(UART_CMD_DUT_APP_WRITE_FLASH);									 // 命令
	UART_PROTOCOL_TxAddData(0x00);															 // 添加临时数据长度
	UART_PROTOCOL_TxAddData(0);																 // 添加是否加密
	UART_PROTOCOL_TxAddData(0);																 // 添加密钥
	UART_PROTOCOL_TxAddData(appUpdateOnePacket[0]);											 // 添加地址
	UART_PROTOCOL_TxAddData(appUpdateOnePacket[1]);											 // 添加地址
	UART_PROTOCOL_TxAddData(appUpdateOnePacket[2]);											 // 添加地址
	UART_PROTOCOL_TxAddData(appUpdateOnePacket[3]);											 // 添加地址

	for (appUpdateOnePacket_i = 0; appUpdateOnePacket_i < 112; appUpdateOnePacket_i++) // 添加数据
	{
		UART_PROTOCOL_TxAddData(appUpdateOnePacket[appUpdateOnePacket_i + 4]);
	}
	UART_PROTOCOL_TxAddFrame(); // 调整帧格式,修改长度和添加校验
}

// UART命令帧缓冲区处理
void UART_PROTOCOL_CmdFrameProcess(UART_PROTOCOL_CB *pCB)
{
	UART_PROTOCOL_CMD cmd = UART_PROTOCOL_CMD_NULL;
	UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	// 我的变量
	uint8 eraResual = 0;

	// 参数合法性检验
	if (NULL == pCB)
	{
		return;
	}

	// 命令帧缓冲区为空，退出
	if (pCB->rx.head == pCB->rx.end)
	{
		return;
	}

	// 获取当前要处理的命令帧指针
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.head];

	// 命令头非法，退出
	if (UART_PROTOCOL_CMD_HEAD != pCmdFrame->buff[UART_PROTOCOL_CMD_HEAD_INDEX])
	{
		// 删除命令帧
		pCB->rx.head++;
		pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
		return;
	}

	// 命令头合法，则提取命令
	cmd = (UART_PROTOCOL_CMD)pCmdFrame->buff[UART_PROTOCOL_CMD_CMD_INDEX];
	switch (dut_info.ID)
	{
	case DUT_TYPE_NULL:
		break;

	case DUT_TYPE_CM: // 串口通用升级
	
		switch (cmd)
		{
		case UART_PROTOCOL_CMD_NULL: // = 0				// 空命令
			break;

		case UART_CMD_UP_PROJECT_APPLY: // 0x01			//DUT工程模式申请,bc280ul
			// 发送允许进入工程模式
			if (dut_info.bootUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_BOOT_ENTRY);
			}
			else if (!dut_info.dutPowerOnAllow)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_UP_ALLOW);
			}
			else
			{
				/*不操作，仪表自动跳过工程模式*/
			}
			break;

		case UART_CMD_UP_PROJECT_READY: // 0x03			//DUT工程模式准备就绪
			// 选择升级
//			if (dut_info.bootUpFlag)
//			{
//				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_BOOT_ENTRY);
//			}
            if (dut_info.appUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR);
			}            
			else if (dut_info.uiUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR);
			}
			else if (dut_info.configUpFlag)
			{
				STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
			}  
			break;

		case UART_CMD_DUT_UI_DATA_ERASE_ACK: // 0x13		//DUT_UI擦写成功
			// 提取DUT_UI擦除结果
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			// 擦除失败
			if (0 == eraResual)
			{
				// 擦除失败再擦除一次
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_EAR);
				break;
			}
			// 擦除成功
			dut_info.currentUiSize = 0;
			STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET);
			break;

		case UART_CMD_DUT_UI_DATA_WRITE_RES: // 0x15			//DUT_UI写入结果
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			if (0 == eraResual) // 如果仪表写入失败就退出
			{
				dut_info.currentUiSize--;
			}

			if (dut_info.currentUiSize < (dut_info.uiSize + 1))
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_UI_PACKET);
			}
			else
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_UI_UP_SUCCESS);
			}
			break;

		case UART_CMD_DUT_APP_ERASE_FLASH: // 0x24		//DUT_APP擦写
			break;

		case UART_CMD_DUT_APP_ERASE_RESULT: // 0x25			//DUT_APP擦写结果查询
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];
			// 擦除失败
			if (0 == eraResual)
			{
				// 擦除失败再擦除一次
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_APP_EAR);
				break;
			}

			// 发送第一帧数据
			dut_info.currentAppSize = 0;
			STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET);
			break;

		case UART_CMD_DUT_APP_WRITE_FLASH: // 0x26		//DUT_APP写入
			break;

		case UART_CMD_DUT_APP_WRITE_FLASH_RES: // 0x27		//DUT_APP写入结果
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			if (0 == eraResual) // 如果仪表写入失败就退出
			{
				dut_info.currentAppSize--;
			}
			if (dut_info.currentAppSize < dut_info.appSize)
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_SEND_APP_PACKET);
			}
			else
			{
				STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_APP_UP_SUCCESS);
			}
			break;

		case UART_CMD_UP_UPDATA_FINISH_RESULT: // 0x2B  			//DUT写入结果确定
			STATE_SwitchStep(STEP_CM_UART_BC_UPGRADE_ITEM_FINISH);
			break;

		default:
			break;
		}
		break;
		
	case DUT_TYPE_HUAXIN: // 华芯微特串口升级
		// 执行命令帧
		switch (cmd)
		{
		// 收到仪表发送的ECO请求
		case UART_CMD_UP_PROJECT_ALLOW:	   // 02
			if (!dut_info.dutPowerOnAllow) // 允许开机就不应答
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_UP_ALLOW);
			}
			break;

		// 仪表回应准备就绪
		case UART_CMD_UP_PROJECT_READY: // 03

			/*判断还有是否要升级的标志位*/
			if (dut_info.appUpFlag)
			{
				// 向仪表发送擦除app区命令
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_EAR);
			}
			else if (dut_info.uiUpFlag)
			{
				// 向仪表发送擦除ui区命令
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR);
			}
			else if (dut_info.configUpFlag)
			{
				STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
			}
			break;

		case UART_CMD_UP_APP_EAR: // 05
			// 提取仪表擦除结果
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			// 擦除失败
			if (0 == eraResual)
			{
				// 擦除失败再擦除一次
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_EAR);
				break;
			}
			// 擦除成功
			STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_APP_PACKET);
			break;

		case UART_CMD_UP_APP_UP: // 06,app写入
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];
			if (0 == eraResual) // 如果仪表写入失败
			{
				dut_info.currentAppSize--;
			}

			if (dut_info.currentAppSize < dut_info.appSize)
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_APP_PACKET);
			}
			else
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_APP_UP_SUCCESS);
			}

			break;

		case UART_CMD_UP_APP_UP_OVER: // 07，升级项结束，开始升级下一项
			STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_ITEM_FINISH);

			break;

		case UART_CMD_DUT_UI_DATA_ERASE: // 0x12,ui擦除

			break;

		case UART_CMD_DUT_UI_DATA_ERASE_ACK: // 0x13,ui擦除应答
			// 提取DUT_UI擦除结果
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			// 擦除失败
			if (0 == eraResual)
			{
				// 擦除失败再擦除一次
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_EAR);
				break;
			}
			// 擦除成功
			dut_info.currentUiSize = 0;
			STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_FIRST_UI_PACKET);
			break;
		case UART_CMD_DUT_UI_DATA_WRITE: // 0x14,ui写入

			break;
		case UART_CMD_DUT_UI_DATA_WRITE_RES: // 0x15,ui写入结果

			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			if (0 == eraResual) // 如果仪表写入失败就退出
			{
				dut_info.currentUiSize--;
			}

			if (dut_info.currentUiSize < (dut_info.uiSize + 1))
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_SEND_UI_PACKET);
			}
			else
			{
				STATE_SwitchStep(STEP_HUAXIN_UART_BC_UPGRADE_UI_UP_SUCCESS);
			}
			break;
		default:
			break;
		}

		break;

	// 串口段码屏升级
	case DUT_TYPE_SEG:
		switch (cmd)
		{
		// 收到仪表发送的ECO请求
		case UART_CMD_UP_PROJECT_ALLOW: // 02
			// 发送允许进入工程模式
			STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_UP_ALLOW);
			break;
		case UART_CMD_UP_PROJECT_READY: // 03
			// 发送app擦除指令
			if (dut_info.appUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR);
			}
			else if (dut_info.configUpFlag)
			{
				STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH);
			}
			break;

		// dut擦除结果查询
		case UART_CMD_UP_APP_EAR:
			// 提取仪表擦除结果
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			// 擦除失败
			if (0 == eraResual)
			{
				// 擦除失败再擦除一次
				STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_SEND_APP_EAR);
				break;
			}
			// 擦除成功
			// 发送第一包数据
			STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_SEND_FIRST_APP_PACKET);

			break;
		case UART_CMD_UP_APP_UP: // 06,app数据写入
			eraResual = pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX];

			if (0 == eraResual) // 如果仪表写入失败
			{
				break;
			}
			else
			{
				if (dut_info.currentAppSize < dut_info.appSize)
				{
					STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_SEND_APP_PACKET); // 发送app数据包
				}
				else
				{
					STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_APP_UP_SUCCESS);
				}
			}
			break;
		case UART_CMD_UP_APP_UP_OVER: // 07，app升级完毕
			STATE_SwitchStep(STEP_CM_UART_SEG_UPGRADE_ITEM_FINISH);
			break;
		default:
			break;
		}
		break;
		
	// 开阳升级流程
	case DUT_TYPE_KAIYANG:
		// 执行命令帧
		switch (cmd)
		{
		// 收到仪表发送的ECO请求
		case UART_CMD_UP_PROJECT_APPLY: // 01
			if (!dut_info.dutPowerOnAllow)
			{
				if (dut_info.uiUpFlag || dut_info.appUpFlag)
				{
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_UP_ALLOW);
				}
				else if (dut_info.configUpFlag)
				{
					STATE_SwitchStep(STEP_UART_SET_CONFIG_ENTRY);
				}
				dut_info.dutPowerOnAllow = TRUE; // 不在接收eco请求
			}
			break;

		// 仪表回应准备就绪
		case UART_CMD_UP_PROJECT_ALLOW: // 02
			if (dut_info.appUpFlag)
			{
				dut_info.currentAppSize = 0;
				STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE);
				TIMER_AddTask(TIMER_ID_RECONNECTION, 
							1000, 
							STATE_SwitchStep, 
							STEP_KAIYANG_UART_UPGRADE_SEND_APP_AGAIN, 
							TIMER_LOOP_FOREVER, 
							ACTION_MODE_ADD_TO_QUEUE);
			}
			else if (dut_info.uiUpFlag)
			{
				dut_info.currentUiSize = 0;
				STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE);
				TIMER_AddTask(TIMER_ID_RECONNECTION, 
							1000, 
							STATE_SwitchStep, 
							STEP_KAIYANG_UART_UPGRADE_SEND_UI_AGAIN, 
							TIMER_LOOP_FOREVER, 
							ACTION_MODE_ADD_TO_QUEUE);
			}
			break;

		case UART_CMD_UP_PROJECT_READY: // 03
			TIMER_ResetTimer(TIMER_ID_DUT_TIMEOUT);	// 喂狗
			if (dut_info.appUpFlag)
			{
				if (dut_info.currentAppSize < dut_info.appSize - 1)
				{
					if (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX])
					{
						dut_info.currentAppSize++;
					}
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_APP_WRITE);
				}
				else
				{
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_APP_UP_SUCCESS);
				}
			}
			else if (dut_info.uiUpFlag)
			{
				if (dut_info.currentUiSize < dut_info.uiSize - 1)
				{
					if (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX])
					{
						dut_info.currentUiSize++;
					}
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_SEND_UI_WRITE);
				}
				else
				{
					STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_APP_UP_SUCCESS);
				}
			}

			break;
		case UART_ECO_CMD_ECO_JUMP_APP: // 04
			if (pCmdFrame->buff[UART_PROTOCOL_CMD_DATA1_INDEX])
			{
				// 升级成功
				if (dut_info.appUpFlag)
				{
					dut_info.appUpSuccesss = TRUE;
					dut_info.appUpFlag = FALSE;
				}
				else if (dut_info.uiUpFlag)
				{
					dut_info.uiUpSuccesss = TRUE;
					dut_info.uiUpFlag = FALSE;
				}
			}
			else // 升级失败
			{
				if (dut_info.appUpFlag)
				{
					dut_info.appUpFaile = TRUE;
					dut_info.appUpFlag = FALSE;
				}
				else if (dut_info.uiUpFlag)
				{
					dut_info.appUpFaile = TRUE;
					dut_info.uiUpFlag = FALSE;
				}
			}
			TIMER_KillTask(TIMER_ID_DUT_TIMEOUT);
			STATE_SwitchStep(STEP_KAIYANG_UART_UPGRADE_ITEM_FINISH);
			break;

		default:
			break;
		}
		break;
		
	default:
		break;
	}

	// 删除命令帧
	pCB->rx.head++;
	pCB->rx.head %= UART_PROTOCOL_RX_QUEUE_SIZE;
}

// 发送命令回复，带一个参数
void UART_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam)
{
	UART_PROTOCOL_TxAddData(0x55);
	UART_PROTOCOL_TxAddData(ackCmd);
	UART_PROTOCOL_TxAddData(0x01);
	UART_PROTOCOL_TxAddData(ackParam);
	UART_PROTOCOL_TxAddFrame();
}

// RX通讯超时处理-单向
#if UART_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE
void UART_PROTOCOL_CALLBACK_RxTimeOut(uint32 param)
{
	UART_PROTOCOL_RX_CMD_FRAME *pCmdFrame = NULL;

	pCmdFrame = &uartProtocolCB.rx.cmdQueue[uartProtocolCB.rx.end];

	// 超时错误，将命令帧长度清零，即认为抛弃该命令帧
	pCmdFrame->length = 0; // 2016.1.6增加
	// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
	uartProtocolCB.rxFIFO.head++;
	uartProtocolCB.rxFIFO.head %= UART_PROTOCOL_RX_FIFO_SIZE;
	uartProtocolCB.rxFIFO.currentProcessIndex = uartProtocolCB.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void UART_PROTOCOL_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

// 码表设置立刻下发数据请求标志
void UART_PROTOCOL_SetTxAtOnceRequest(uint32 param)
{
	uartProtocolCB.txAtOnceRequest = (BOOL)param;
}

// 码表设置周期下发数据请求标志
void UART_PROTOCOL_CALLBACK_SetTxPeriodRequest(uint32 param)
{
	uartProtocolCB.txPeriodRequest = (BOOL)param;
}

// 	UART总线超时错误处理
void UART_PROTOCOL_CALLBACK_UartBusError(uint32 param)
{
}

// 上报写配置参数结果
void UART_PROTOCOL_ReportWriteParamResult(uint32 param)
{
	// 添加命令头
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

	// 添加设备地址
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// 添加命令字
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_WRITE_CONTROL_PARAM_RESULT);

	// 添加数据长度
	UART_PROTOCOL_TxAddData(1);

	// 写入结果
	UART_PROTOCOL_TxAddData(param);

	// 添加检验和与结束符，并添加至发送
	UART_PROTOCOL_TxAddFrame();
}

// 发送命令带结果
void UART_PROTOCOL_SendLdoV(uint8 cmdWord, uint32 result)
{
	// 添加命令头
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

	// 添加设备地址
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// 添加命令字
	UART_PROTOCOL_TxAddData(cmdWord);

	// 添加数据长度
	UART_PROTOCOL_TxAddData(4);

	UART_PROTOCOL_TxAddData((result >> 24) & 0xFF);
	UART_PROTOCOL_TxAddData((result >> 16) & 0xFF);
	UART_PROTOCOL_TxAddData((result >> 8) & 0xFF);
	UART_PROTOCOL_TxAddData((result >> 0) & 0xFF);

	UART_PROTOCOL_TxAddFrame();
}

// 发送命令带结果
void UART_PROTOCOL_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
	// 添加命令头
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

	// 添加设备地址
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// 添加命令字
	UART_PROTOCOL_TxAddData(cmdWord);

	// 添加数据长度
	UART_PROTOCOL_TxAddData(1);

	UART_PROTOCOL_TxAddData(result);

	UART_PROTOCOL_TxAddFrame();
}

// 发送命令无结果
void UART_PROTOCOL_SendCmdNoResult(uint8 cmdWord)
{
	// 添加命令头
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_HEAD);

	// 添加设备地址
	UART_PROTOCOL_TxAddData(UART_PROTOCOL_CMD_DEVICE_ADDR);

	// 添加命令字
	UART_PROTOCOL_TxAddData(cmdWord);

	UART_PROTOCOL_TxAddData(0); // 数据长度

	UART_PROTOCOL_TxAddFrame();
}
