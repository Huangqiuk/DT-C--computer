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
* 【内部接口声明】
******************************************************************************/

// 数据结构初始化
void IAP_CTRL_UART_DataStructureInit(IAP_CTRL_UART_CB* pCB);

// UART初始化
void IAP_CTRL_UART_HwInit(uint32 baud);

// 发送处理
void IAP_CTRL_UART_TxProcess(IAP_CTRL_UART_CB* pCB);

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL IAP_CTRL_UART_ConfirmTempCmdFrameBuff(IAP_CTRL_UART_CB* pCB);

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void IAP_CTRL_UART_RxFIFOProcess(IAP_CTRL_UART_CB* pCB);

// 对传入的命令帧进行校验，返回校验结果
BOOL IAP_CTRL_UART_CheckSUM(IAP_CTRL_CMD_FRAME* pCmdFrame);

// UART命令帧缓冲区处理
void IAP_CTRL_UART_CmdFrameProcess(IAP_CTRL_UART_CB* pCB);

// 通讯超时处理-单向
void IAP_CTRL_UART_CALLBACK_RxTimeOut(uint32 param);

// 停止RX通讯超时检测任务
void IAP_CTRL_UART_StopRxTimeOutCheck(void);

// TXRX通讯超时处理-双向
void IAP_CTRL_UART_CALLBACK_TxRxTimeOut(uint32 param);

// 停止TXRX通讯超时检测任务
void IAP_CTRL_UART_StopTxRxTimeOutCheck(void);

// 启动中断字节发送
void IAP_CTRL_UART_IR_StartSendData(uint8 data);

// 阻塞发送一个字节数据
void IAP_CTRL_UART_BC_SendData(uint8 data);

// UART接收与发送中断处理，用于内部通讯
void USART2_IRQHandler(void);

// 工程模式准备就绪报告
void IAP_CTRL_UART_SendCmdProjectReady(uint8 param);
// 擦除FLASH结果上报
void IAP_CTRL_UART_SendCmdEraseFlashResult(uint8 param);
// IAP数据写入结果上报
void IAP_CTRL_UART_SendCmdWriteFlashResult(uint8 param);
// 查空结果上报
void IAP_CTRL_UART_SendCmdCheckFlashBlankResult(uint8 param);
// 系统升级结束确认
void IAP_CTRL_UART_SendCmdUpdataFinishResult(uint8 param);
// 发送UI数据擦除结果
void IAP_CTRL_UART_SendCmdUIEraseResultReport(uint8 param);
// 发送UI数据写入结果
void IAP_CTRL_UART_SendCmdUIWriteDataResultReport(uint8 param);
// 发送写入二维码数据结果
void IAP_CTRL_UART_SendCmdQrWriteResultReport(uint8 param);

// 发送命令带结果
void IAP_CTRL_UART_SendCmdWithResult(uint8 cmdWord, uint8 result);
// 发送命令无结果
void IAP_CTRL_UART_SendCmdNoResult(uint8 cmdWord);
// 读取数据上报
void IAP_CTRL_UART_SendSpiFlashData(uint32 ReadAddr, uint8 NumByteToRead);

void IAP_CTRL_UART_SendFlagCheck(uint32 checkNum);

// 发送自身MCU的唯一序列号，防止误入老化
void UART_PROTOCOL_SendMcuUid(uint32 param);


//=================================================================================

// 全局变量定义
IAP_CTRL_UART_CB iapCtrlUartCB;

// ■■函数定义区■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
// UART初始化
void IAP_CTRL_UART_Init(void)
{	
	// 硬件UART配置
	IAP_CTRL_UART_HwInit(IAP_CTRL_UART_BAUD_RATE);

	// UART数据结构初始化
	IAP_CTRL_UART_DataStructureInit(&iapCtrlUartCB);
}

// 数据结构初始化
void IAP_CTRL_UART_DataStructureInit(IAP_CTRL_UART_CB* pCB)
{
	uint8 i;
	
	// 参数合法性检验
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

// UART初始化
void IAP_CTRL_UART_HwInit(uint32 baud)
{
	// GD32
	// 时钟配置
	rcu_periph_clock_enable(RCU_GPIOA);
	
	// GPIO配置
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);   //复用推挽

	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, GPIO_PIN_3); 	// 上拉输入
	
	// UART时钟配置
	rcu_periph_clock_enable(RCU_USART1);								// 时能USART时钟
	usart_deinit(IAP_CTRL_UART_TYPE_DEF);									// 复位串口

	// 串口配置
	usart_baudrate_set(IAP_CTRL_UART_TYPE_DEF, baud);						// 波特率
	usart_word_length_set(IAP_CTRL_UART_TYPE_DEF, USART_WL_8BIT);			// 8位数据位
	usart_stop_bit_set(IAP_CTRL_UART_TYPE_DEF, USART_STB_1BIT);			// 一个停止位
	usart_parity_config(IAP_CTRL_UART_TYPE_DEF, USART_PM_NONE);			// 无奇偶校验
	usart_hardware_flow_rts_config(IAP_CTRL_UART_TYPE_DEF, USART_RTS_DISABLE); // 无硬件数据流控制
	usart_hardware_flow_cts_config(IAP_CTRL_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(IAP_CTRL_UART_TYPE_DEF, USART_TRANSMIT_ENABLE);	// 使能发射
	usart_receive_config(IAP_CTRL_UART_TYPE_DEF, USART_RECEIVE_ENABLE);		// 使能接收

	// 中断配置
	nvic_irq_enable(IAP_CTRL_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(IAP_CTRL_UART_TYPE_DEF, USART_INT_RBNE);			// 开启中断

	usart_enable(IAP_CTRL_UART_TYPE_DEF);									// 使能串口 

}

// 将临时缓冲区添加到命令帧缓冲区中，其本质操作是承认临时缓冲区数据有效
BOOL IAP_CTRL_UART_ConfirmTempCmdFrameBuff(IAP_CTRL_UART_CB* pCB)
{
	IAP_CTRL_CMD_FRAME* pCmdFrame = NULL;
	
	// 参数合法性检验
	if(NULL == pCB)
	{
		return FALSE;
	}

	// 临时缓冲区为空，不予添加
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	if(0 == pCmdFrame->length)
	{
		return FALSE;
	}

	// 添加
	pCB->rx.end ++;
	pCB->rx.end %= IAP_CTRL_UART_RX_QUEUE_SIZE;
	pCB->rx.cmdQueue[pCB->rx.end].length = 0;	// 该行的作用是将新的添加位置有效数据个数清零，以便将这个位置当做临时帧缓冲区
	
	return TRUE;
}

// 一级接收缓冲区处理，从一级接收缓冲区中取出一个字节添加到命令帧缓冲区中
void IAP_CTRL_UART_RxFIFOProcess(IAP_CTRL_UART_CB* pCB)
{
	uint16 end = pCB->rxFIFO.end;
	uint16 head = pCB->rxFIFO.head;
	IAP_CTRL_CMD_FRAME* pCmdFrame = NULL;
	uint16 length = 0;
	uint8 currentData = 0;
	
	// 参数合法性检验
	if(NULL == pCB)
	{
		return;
	}
	
	// 一级缓冲区为空，退出
	if(head == end)
	{
		return;
	}

	// 获取临时缓冲区指针
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.end];
	// 取出当前要处理的字节
	currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
	
	// 临时缓冲区长度为0时，搜索首字节
	if(0 == pCmdFrame->length)
	{
		// 命令头错误，删除当前字节并退出
		if(IAP_CTRL_UART_CMD_HEAD != currentData)
		{
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}
		
		// 命令头正确，但无临时缓冲区可用，退出
		if((pCB->rx.end + 1)%IAP_CTRL_UART_RX_QUEUE_SIZE == pCB->rx.head)
		{
			return;
		}

		// 添加UART通讯超时时间设置-2016.1.5增加
		#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
		TIMER_AddTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL, IAP_CTRL_UART_BUS_UNIDIRECTIONAL_TIME_OUT, IAP_CTRL_UART_CALLBACK_RxTimeOut, 0, 1, ACTION_MODE_ADD_TO_QUEUE);
		#endif
		
		// 命令头正确，有临时缓冲区可用，则将其添加到命令帧临时缓冲区中
		pCmdFrame->buff[pCmdFrame->length++]= currentData;
		pCB->rxFIFO.currentProcessIndex ++;
		pCB->rxFIFO.currentProcessIndex %= IAP_CTRL_UART_RX_FIFO_SIZE;
	}
	// 非首字节，将数据添加到命令帧临时缓冲区中，但暂不删除当前数据
	else
	{
		// 临时缓冲区溢出，说明当前正在接收的命令帧是错误的，正确的命令帧不会出现长度溢出的情况
		if(pCmdFrame->length >= IAP_CTRL_UART_CMD_LENGTH_MAX)
		{
			#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			IAP_CTRL_UART_StopRxTimeOutCheck();
			#endif

			// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
			pCmdFrame->length = 0;	// 2016.1.5增加
			// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
			pCB->rxFIFO.head ++;
			pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

			return;
		}

		// 一直取到末尾
		while(end != pCB->rxFIFO.currentProcessIndex)
		{
			// 取出当前要处理的字节
			currentData = pCB->rxFIFO.buff[pCB->rxFIFO.currentProcessIndex];
			// 缓冲区未溢出，正常接收，将数据添加到临时缓冲区中
			pCmdFrame->buff[pCmdFrame->length++]= currentData;
			pCB->rxFIFO.currentProcessIndex ++;
			pCB->rxFIFO.currentProcessIndex %= IAP_CTRL_UART_RX_FIFO_SIZE;

			// ■■接下来，需要检查命令帧是否完整，如果完整，则将命令帧临时缓冲区扶正■■
			
			// 首先判断命令帧最小长度，一个完整的命令字至少包括4个字节: 命令头 + 命令字 + 数据长度 + 校验码，因此不足4个字节的必定不完整
			if(pCmdFrame->length < IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN)
			{
				// 继续接收
				continue;
			}

			// 命令帧长度数值越界，说明当前命令帧错误，停止接收
			if(pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] > IAP_CTRL_UART_CMD_LENGTH_MAX-IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN)
			{
				#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				IAP_CTRL_UART_StopRxTimeOutCheck();
				#endif
			
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;

				return;
			}

			// 命令帧长度校验，在命令长度描述字的数值上，增加命令头、命令字、数据长度、校验码，即为命令帧实际长度
			length = pCmdFrame->length;
			if(length < pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] + IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN)
			{
				// 长度要求不一致，说明未接收完毕，退出继续
				continue;
			}

			// 命令帧长度OK，则进行校验，失败时删除命令头
			if(!IAP_CTRL_UART_CheckSUM(pCmdFrame))
			{
				#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
				// 停止RX通讯超时检测
				IAP_CTRL_UART_StopRxTimeOutCheck();
				#endif
				
				// 校验失败，将命令帧长度清零，即认为抛弃该命令帧
				pCmdFrame->length = 0;
				// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
				pCB->rxFIFO.head ++;
				pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
				pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
				
				return;
			}

			#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
			// 停止RX通讯超时检测
			IAP_CTRL_UART_StopRxTimeOutCheck();
			#endif
			
			// 执行到这里，即说明接收到了一个完整并且正确的命令帧，此时需将处理过的数据从一级缓冲区中删除，并将该命令帧扶正
			pCB->rxFIFO.head += length;		// head必须要用16位，否则这一行有时会溢出
			pCB->rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
			pCB->rxFIFO.currentProcessIndex = pCB->rxFIFO.head;
			IAP_CTRL_UART_ConfirmTempCmdFrameBuff(pCB);

			return;
		}
	}
}

// 对传入的命令帧进行校验，返回校验结果
BOOL IAP_CTRL_UART_CheckSUM(IAP_CTRL_CMD_FRAME* pCmdFrame)
{
	uint8 cc = 0;
	uint16 i = 0;
	
	if(NULL == pCmdFrame)
	{
		return FALSE;
	}

	// 从命令头开始，到校验码之前的一个字节，依次进行异或运算
	for(i=0; i<pCmdFrame->length-1; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}

	cc = ~cc;

	// 判断计算得到的校验码与命令帧中的校验码是否相同
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

// UART命令帧缓冲区处理
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
	// 参数合法性检验
	if(NULL == pCB)
	{
		return;
	}

	// 命令帧缓冲区为空，退出
	if(pCB->rx.head == pCB->rx.end)
	{
		return;
	}

	// 获取当前要处理的命令帧指针
	pCmdFrame = &pCB->rx.cmdQueue[pCB->rx.head];
	
	// 命令头非法，退出
	if(IAP_CTRL_UART_CMD_HEAD != pCmdFrame->buff[IAP_CTRL_UART_CMD_HEAD_INDEX])
	{
		// 删除命令帧
		pCB->rx.head ++;
		pCB->rx.head %= IAP_CTRL_UART_RX_QUEUE_SIZE;
		return;
	}

	// 命令头合法，则提取命令
	cmd = (IAP_CTRL_UART_CMD)pCmdFrame->buff[IAP_CTRL_UART_CMD_CMD_INDEX];
	
	// 执行命令帧
	switch(cmd)
	{		
		case IAP_CTRL_UART_CMD_NULL:
			break;

		// 收到对发ECO请求
		case IAP_CTRL_UART_CMD_UP_PROJECT_APPLY:
//			// 关闭跳入APP1定时器
//			TIMER_KillTask(TIMER_ID_IAP_RUN_WINDOW_TIME);
//			
//			// 如果成功跳入指定APP,如果指定地址APP非法，上报失败
//			errStatus = IAP_CheckAppRightful(IAP_FLASH_APP1_ADDR);
//			if(0 == paramCB.runtime.jumpFlag)
//			{
//				paramCB.runtime.jumpFlag = (BOOL)1;
//				
//				// 升级完成延时回复后跳转
//				TIMER_AddTask(TIMER_ID_ECO_JUMP,
//								200,
//								IAP_JumpToAppFun,
//								IAP_FLASH_APP1_ADDR,
//								1,
//								ACTION_MODE_ADD_TO_QUEUE);
//			}
			
			// 关闭跳入APP1定时器
				TIMER_KillTask(TIMER_ID_IAP_RUN_WINDOW_TIME);
				
	
				TIMER_AddTask(TIMER_ID_IAP_RUN_WINDOW_TIME,
									500,
									IAP_JumpToAppFun,
									IAP_FLASH_APP2_ADDR,
									1,
									ACTION_MODE_ADD_TO_QUEUE);
									
				UART_PROTOCOL_SendMcuUid(1);
				
				// 发送MCU_UID
				TIMER_AddTask(TIMER_ID_MCU_UID,
								50,
								UART_PROTOCOL_SendMcuUid,
								1,
								4,
								ACTION_MODE_ADD_TO_QUEUE);		
			// 发送结果
			//IAP_CTRL_UART_SendCmdWithResult(0x0F, errStatus);
			break;
			
		// 发送MCU_UID
		case UART_ECO_CMD_ECO_MCU_UID:
		
			// 收到此MCU_UID说明ECO申请的命令已经收到了，可以停止该命令发送
			TIMER_KillTask(TIMER_ID_IAP_ECO_REQUEST);
			TIMER_KillTask(TIMER_ID_MCU_UID);
		
			// 接受的MCU_UID长度是否等于12
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
			
				// MCU_UID一致则不理会
				if ((iapCB.mcuUID.sn0 == temp3) && (iapCB.mcuUID.sn1 == temp4) && (iapCB.mcuUID.sn2 == temp5))
				{
					break;
				}
				// MCU_UID不同则写入老化标志
				else
				{
					// 做条件判断是为了只进入一次，防止多次进入
					if (0x55AA55BB != iapCtrlUartCB.ageFlag)
					{
						// 写入老化进入标志
						iapCtrlUartCB.ageFlag = 0x55AA55BB;
						
						SPI_FLASH_WriteWord(SPI_FLASH_TEST_FLAG_ADDEESS, iapCtrlUartCB.ageFlag);
					}
				}
			}
			break;

		// 工程模式批准
		case IAP_CTRL_UART_CMD_DOWN_PROJECT_APPLY_ACK:
			// 关闭此定时器
			TIMER_KillTask(TIMER_ID_IAP_RUN_WINDOW_TIME);
			TIMER_KillTask(TIMER_ID_IAP_ECO_REQUEST);

			// 发送结果
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_PROJECT_READY, TRUE);
			break;

		// 恢复出厂设置
		case IAP_CTRL_UART_CMD_DOWN_FACTORY_RESET:
			break;

		// 设置RTC
		case IAP_CTRL_UART_CMD_DOWN_RTC_SET:
			break;

		// 系统参数写入
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

					// 单位
					paramCB.factoryNvm.param.sys.unit = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX];

					// 限速
					paramCB.factoryNvm.param.sys.speedLimitVal = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA7_INDEX];

					// 测速磁钢
					paramCB.factoryNvm.param.sys.steelNumOfSpeedSensor = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA8_INDEX];

					// 背光亮度
					paramCB.factoryNvm.param.sys.brightness = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA9_INDEX];

					// 自动关机时间
					paramCB.factoryNvm.param.sys.powerOffTime = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA10_INDEX];

					// 协议
					paramCB.factoryNvm.param.sys.protocol = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA11_INDEX];

					// 系统电压
					paramCB.factoryNvm.param.sys.batVoltage = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA12_INDEX];

					// 电量方式
					paramCB.factoryNvm.param.sys.percentageMethod = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA13_INDEX];

					// 推车助力
					paramCB.factoryNvm.param.sys.pushAssistSwitch = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA14_INDEX];

					// 默认档位
					paramCB.factoryNvm.param.sys.defaultAssist = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA15_INDEX];

					// 最大档位
					paramCB.factoryNvm.param.sys.maxAssist = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA16_INDEX];

					// 开关机logo
					paramCB.factoryNvm.param.sys.logo = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA17_INDEX];

					// 设置写入标志
					FACTORY_NVM_SetDirtyFlag(TRUE);

					// 发送应答信号
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_WRITE_SYS_PARAM);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 2);		// 数据长度
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);		// 参数名称
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
					// 出厂参数
					SPI_FLASH_ReadArray(paramCB.factoryNvm.array, SPI_FLASH_FACTORY_PARAM_ADDEESS, PARAM_FACTORY_NVM_DATA_SIZE);
					SPI_FLASH_ReadArray(paramCB.preFactoryValue, SPI_FLASH_FACTORY_PARAM_ADDEESS, PARAM_FACTORY_NVM_DATA_SIZE);
					
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_READ_SYS_PARAM);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);		// 参数名称

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

		// UI数据擦除
		case IAP_CTRL_UART_CMD_DOWN_UI_DATA_ERASE:
			// 以块方式擦除
#if (0 == SPI_FLASH_ERASE_MODE)
			// 扇区擦除
			SPI_FLASH_EraseRoom(SPI_FLASH_UI_OFFSET_ADDR, SPI_FLASH_UI_SIZE);
			
#elif (1 == SPI_FLASH_ERASE_MODE)
			// 擦除整个FLASH
			SPI_FLASH_EraseChip();
			
#elif (2 == SPI_FLASH_ERASE_MODE)
			SPI_FLASH_EraseRoom(SPI_FLASH_UI_OFFSET_ADDR, SPI_FLASH_UI_SIZE);
#endif

			// 发送结果
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_UI_DATA_ERASE_ACK, TRUE);
			break;

		// UI数据写入
		case IAP_CTRL_UART_CMD_DOWN_UI_DATA_WRITE:
			// 禁止SPI以修改配置
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

			// 写入数据到flash里面
			errStatus = SPI_FLASH_WriteWithCheck(SPI_FLASH_UI_OFFSET_ADDR + (uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX], temp, length);

			// 发送结果
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_UI_DATA_WRITE_RESULT, errStatus);
			break;

		// APP擦除
		case IAP_CTRL_UART_CMD_DOWN_IAP_ERASE_FLASH:
			// 显示uart升级提示
			
			// 先擦除APP代码区
			IAP_EraseAPP1Area();

			// 擦除完后需要读出来检查是否为空
			errStatus = IAP_CheckApp1FlashIsBlank();

			// 发送结果
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_IAP_ERASE_FLASH_RESULT, errStatus);

			break;
			
		// APP数据写入
		case IAP_CTRL_UART_CMD_DOWN_IAP_WRITE_FLASH:
			{
				uint8 encryptionMode;
				uint8 key;
				static uint8 arrbuf[IAP_CTRL_UART_CMD_LENGTH_MAX];
				
				encryptionMode = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];
				key 		   = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX];

				switch (encryptionMode)
				{
					case 0x00:	// 无加密，【密钥】无意义
						temp  = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA3_INDEX];
						temp <<= 8;
						temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA4_INDEX];
						temp <<= 8;
						temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
						temp <<= 8;
						temp += pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX];

						length = pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] - 6;

						// 写入数据
						errStatus = IAP_WriteAppBin(temp, (uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA7_INDEX], length);
						break;

					case 0x01:	// 加密，地址与数据均与【密钥】相亦或后使用
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

						// 写入数据
						errStatus = IAP_WriteAppBin(temp, arrbuf, length);
						break;

					case 0x02:
						errStatus = FALSE;
						break;

					default:
						errStatus = FALSE;
						break;
				}

				// 发送结果
				IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_IAP_WRITE_FLASH_RESULT, errStatus);
			}

			break;

		// 查空
		case IAP_CTRL_UART_CMD_DOWN_CHECK_FALSH_BLANK:
			errStatus = IAP_CheckApp2FlashIsBlank();

			// 发送结果
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_CHECK_FLASH_BLANK_RESULT, errStatus);
			break;

		// APP升级完成
		case IAP_CTRL_UART_CMD_DOWN_UPDATA_FINISH:
			// 串口升级完成需要擦除蓝牙升级标志，否则不会进入应用APP
			// 清除蓝牙升级标志
			SPI_FLASH_WriteByte(SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS,0xFF);

			// 串口升级完成提示
			
			
			// 发送应答
			IAP_CTRL_UART_SendCmdNoResult(IAP_CTRL_UART_CMD_UP_UPDATA_FINISH_RESULT);
			break;


		// 启动二维码注册
		case IAP_CTRL_UART_CMD_DOWN_START_QR_REGISTER:
			// 使能开始二维码注册
			paramCB.runtime.qrRegEnable = TRUE;
			
			// 注册查询BLE的MAC地址
			TIMER_AddTask(TIMER_ID_BLE_CONTROL,
							200,
							BLE_UART_CALLBALL_GetBleMacAddrRequest,
							TRUE,
							TIMER_LOOP_FOREVER,
							ACTION_MODE_ADD_TO_QUEUE);

			// 屏蔽ICCID注册定时器				
			gsmUartCB.rcvIccidTimeOut = TRUE;
			
			/*
			// 注册查询ICCID定时器
			TIMER_AddTask(TIMER_ID_GSM_CONTROL,
							GSM_UART_TX_QUERY_ICCID_TIME,
							GSM_UART_CALLBALL_GetSimIccidRequest,
							TRUE,
							TIMER_LOOP_FOREVER,
							ACTION_MODE_ADD_TO_QUEUE);
			*/

			// gsm模块开关机控制
			GSM_UART_GsmPowerOnOff(TRUE);
			break;

		// 写入二维码数据
		case IAP_CTRL_UART_CMD_DOWN_QR_DATA_WRITE:
			// 将QR写入指定位置的EEPROM
			//E2PROM_WriteByteArray(PARAM_NVM_QR_START_ADDR, (uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX], PARAM_NVM_QR_SIZE);

			SPI_FLASH_WriteWithErase((uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX], SPI_FLASH_NEW_QR_CODE_ADDEESS, PARAM_NVM_QR_SIZE + 1);

			// 读出来进行判断
			SPI_FLASH_ReadArray(localBuf, SPI_FLASH_NEW_QR_CODE_ADDEESS, PARAM_NVM_QR_SIZE + 1);

			// 比较写入的和读出来的是否一致
			errStatus = PARAM_CmpareN((const uint8*)(uint8*)&pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX], (const uint8*)localBuf, PARAM_NVM_QR_SIZE + 1);
			
			// 发送结果
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_UP_QR_DATA_WRITE_RESULT, errStatus);
			break;

//==========================================================================
//==========================================================================
		// SPI 绝对地址擦除
		case IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ERESE:
			// 地址偏移字节数
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			// 字节数
			temp2 = 0;
			for (i = 0; i < 4; i++)
			{
				temp2 <<= 8;
				temp2 |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX + i];
			}
			
			// 擦除指定空间数据
			SPI_FLASH_EraseRoom(temp, temp2);
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ERESE, TRUE);
			break;

		// SPI 绝对地址写入
		case IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_WRITE:
			// 地址偏移字节数
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			
			// 字节数
			length = 0;
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// 写入数据到flash里面
			bTemp = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], temp, length);		
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_WRITE, bTemp);
			break;

		// SPI 绝对地址读取
		case IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_READ:
			// 地址偏移字节数
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			
			// 字节数
			length = 0;
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// 从外部flash读取数据上报
			IAP_CTRL_UART_SendSpiFlashData(temp, length);
			break;

		// SPI 绝对地址擦写
		case IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ER_WR:
			// 地址偏移字节数
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			
			// 字节数
			length = 0;
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// 写入数据到flash里面
			SPI_FLASH_WriteWithErase(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], temp, length);		
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_ER_WR, TRUE);
			break;
			
		// UI数据擦除
		case IAP_CTRL_UART_CMD_SPI_FLASH_UI_ERESE:
			// 擦除指定空间大小
			SPI_FLASH_EraseRoom(SPI_FLASH_UI_OFFSET_ADDR, SPI_FLASH_UI_SIZE);
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_UI_ERESE, TRUE);
			break;
			
		// UI数据写入
		case IAP_CTRL_UART_CMD_SPI_FLASH_UI_WRITE:
			// 地址偏移字节数
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			// 字节数
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// 写入数据到flash里面
			bTemp = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], (SPI_FLASH_UI_OFFSET_ADDR + temp), length);		
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_UI_WRITE, bTemp);
			break;

		// UI 写入完成
		case IAP_CTRL_UART_CMD_SPI_FLASH_UI_FINISH:
			IAP_CTRL_UART_SendCmdNoResult(IAP_CTRL_UART_CMD_SPI_FLASH_UI_FINISH);
			break;

		// 出厂APP数据擦除
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_ERESE:
			// 擦除出厂APP数据
			SPI_FLASH_EraseRoom(SPI_FLASH_APP_FACTORY_OFFSET_ADDR, SPI_FLASH_APP_FACTORY_SIZE);
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_ERESE, TRUE);
			break;

		// 出厂APP数据写入
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_WRITE:
			// 地址偏移字节数
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			// 字节数
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// 写入数据到flash里面
			bTemp = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], (SPI_FLASH_APP_FACTORY_OFFSET_ADDR + temp), length);				
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_WRITE, bTemp);
			break;

		// 出厂APP数据写入完成
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_FINISH:	
			IAP_CTRL_UART_SendCmdNoResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_FACTORY_FINISH);
			break;

		// 升级APP数据擦除
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_ERESE:
			// 擦除文件系统
			SPI_FLASH_EraseRoom(SPI_FLASH_APP_UPDATE_OFFSET_ADDR, SPI_FLASH_APP_UPDATE_SIZE);
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_ERESE, TRUE);
			break;

		// 升级APP数据写入
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_WRITE:
			// 地址偏移字节数
			temp = 0;
			for (i = 0; i < 4; i++)
			{
				temp <<= 8;
				temp |= pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX + i];
			}
			// 字节数
			length = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA5_INDEX];
			
			// 写入数据到flash里面
			bTemp = SPI_FLASH_WriteWithCheck(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA6_INDEX], (SPI_FLASH_APP_UPDATE_OFFSET_ADDR + temp), length);		
				
			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_WRITE, bTemp);
			break;

		// 升级APP数据写入完成
		case IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_FINISH:	
			IAP_CTRL_UART_SendCmdNoResult(IAP_CTRL_UART_CMD_SPI_FLASH_APP_UPDATE_FINISH);
			break;

		case IAP_CTRL_UART_CMD_SPI_FLASH_WRITE_SPI_TO_MCU:
			// 先擦除APP代码区
			IAP_EraseAPP2Area();

#define WR_APP_BYTES_ONE_TIME		128	
			// APP数据写入
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

			// 限于IAP_WriteAppBin()函数一次只能写252个数据，因此读取与写入只能小于等于252
			for (i = 0; i < SPI_FLASH_APP_UPDATE_SIZE/WR_APP_BYTES_ONE_TIME; i++)
			{
				SPI_FLASH_ReadArray(updateBuff, (temp + i * WR_APP_BYTES_ONE_TIME), WR_APP_BYTES_ONE_TIME);
				IAP_WriteAppBin((IAP_FLASH_APP2_ADDR + i * WR_APP_BYTES_ONE_TIME), updateBuff, WR_APP_BYTES_ONE_TIME);
			}
#undef WR_APP_BYTES_ONE_TIME

			// 释放该内存
			free(updateBuff);
			
			// 升级完成延时回复后跳转
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
			
		// 串口命令强制控制BOOT跳转到APP1
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

		// 串口命令强制控制BOOT跳转到APP2
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

		// 版本检测命令
		case IAP_CTRL_UART_CMD_CHECK_VERSION:
			if(IAP_CTRL_UART_CHECK_VERSION_ALL == pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX])
			{			
				checkFlag = 0;
				// 升级完成延时回复后跳转
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

			// 写入标志位置
			temp = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];

			// 写入数据到spiflash
			SPI_FLASH_WriteWithErase(&pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX], SPI_FLASH_TEST_FLAG_ADDEESS + temp*4, 4);

			// 更新标志
			PARAM_Init();

			IAP_CTRL_UART_SendCmdWithResult(IAP_CTRL_UART_CMD_FLAG_ARRAY_WRITE, TRUE);
			break;

		case IAP_CTRL_UART_CMD_VERSION_TYPE_WRITE:
			// 第一个写入类型
			temp = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX];
			
			// 第二个为信息长度
			temp2 = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA2_INDEX];

			updateBuff = (uint8 *)malloc(128);
			if (NULL == updateBuff)
			{
				break;
			}
			
			// 清零
			for (i = 0; i < 128; i++)
			{
				updateBuff[i] = 0;
			}
			// 类型信息
			for (i = 0; i < temp2 + 1; i++)
			{
				updateBuff[i] = pCmdFrame->buff[IAP_CTRL_UART_CMD_DATA1_INDEX+1+i];
			}
			
			// 判断类型种类是否合法
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
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, temp);	// 类型
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
			// 清零
			for (i = 0; i < 128; i++)
			{
				updateBuff[i] = 0;
			}

			// 读取数据
			SPI_FLASH_ReadArray(updateBuff, PARAM_MCU_VERSION_ADDRESS[temp], 64+1);

			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_VERSION_TYPE_READ);
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, temp);	// 类型

			// 根据类型有不同的长度
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
						// 信息长度
						IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[0]);

						// 类型信息
						for (i = 0; i < updateBuff[0]; i++)
						{
							IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[i+1]);
						}
					}
					else
					{
						// 信息长度
						IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);
					}
					break;

				case 5:
				case 6:
					// 信息长度
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 4);
					
					// 类型信息，高字节在前，低字节在后
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[1]);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[2]);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[3]);
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, updateBuff[4]);
					break;
					
				case 7:
					// 信息长度
					IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 32);

					// 类型信息
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

		// 按键测试
		case IAP_CTRL_UART_CMD_KEY_TESTING:
		
			break;

		// 显示测试
		case IAP_CTRL_UART_CMD_TEST_LCD:
			// 长度合法
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
	
	// 删除命令帧
	pCB->rx.head ++;
	pCB->rx.head %= IAP_CTRL_UART_RX_QUEUE_SIZE;
}




// 串口显示控制
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
			

// 向发送命令帧队列中添加数据
void IAP_CTRL_UART_TxAddData(IAP_CTRL_UART_CB* pCB, uint8 data)
{
	IAP_CTRL_CMD_FRAME* pCmdFrame = &pCB->tx.cmdQueue[pCB->tx.end];
	uint16 head = pCB->tx.head;
	uint16 end =  pCB->tx.end;	// 2015.12.29

	// 参数合法性检验
	if(NULL == pCB)
	{
		return;
	}

	// 发送缓冲区已满，不予接收
	if((end + 1) % IAP_CTRL_UART_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 队尾命令帧已满，退出
	if(pCmdFrame->length >= IAP_CTRL_UART_CMD_LENGTH_MAX)
	{
		return;
	}

	// 数据添加到帧末尾，并更新帧长度
	pCmdFrame->buff[pCmdFrame->length] = data;
	pCmdFrame->length ++;
}

// 确认添加命令帧，即承认之前填充的数据为命令帧，将其添加到发送队列中，由main进行调度发送，本函数内会自动校正命令长度，并添加校验码
void IAP_CTRL_UART_TxAddFrame(IAP_CTRL_UART_CB* pCB)
{
	uint8 cc = 0;
	uint16 i = 0;
	IAP_CTRL_CMD_FRAME* pCmdFrame = &pCB->tx.cmdQueue[pCB->tx.end];
	uint16 length = pCmdFrame->length;
	uint16 head = pCB->tx.head;
	uint16 end =  pCB->tx.end;	// 2015.12.29

	// 参数合法性检验
	if(NULL == pCB)
	{
		return;
	}

	// 发送缓冲区已满，不予接收
	if((end + 1) % IAP_CTRL_UART_TX_QUEUE_SIZE == head)
	{
		return;
	}
	
	// 命令帧长度不足，清除已填充的数据，退出
	if(IAP_CTRL_UART_CMD_FRAME_LENGTH_MIN-1 > length)//2016.1.5修改减1
	{
		pCmdFrame->length = 0;
		
		return;
	}

	// 重新设置数据长度，系统在准备数据时，填充的"数据长度"可以为任意值，并且不需要添加校验码，在这里重新设置为正确的值
	pCmdFrame->buff[IAP_CTRL_UART_CMD_LENGTH_INDEX] = length - 3;	// 重设数据长度
	for(i=0; i<length; i++)
	{
		cc ^= pCmdFrame->buff[i];
	}
	pCmdFrame->buff[length] = ~cc;
	pCmdFrame->length ++;

	pCB->tx.end ++;
	pCB->tx.end %= IAP_CTRL_UART_TX_QUEUE_SIZE;
	//pCB->tx.cmdQueue[pCB->tx.end].length = 0;   //2015.12.2修改

	// 阻塞方式时启动发送
#if (IAP_CTRL_UART_TX_MODE == IAP_CTRL_BLOCKING_TX_MODE)
	IAP_CTRL_UART_BC_StartTx(pCB);
#endif

}

// UART模块处理入口
void IAP_CTRL_UART_Process(void)
{
	// 内部通讯接口的一级接收缓冲区处理
	IAP_CTRL_UART_RxFIFOProcess(&iapCtrlUartCB);

	// 内部通讯接口的命令帧缓冲区处理
	IAP_CTRL_UART_CmdFrameProcess(&iapCtrlUartCB);
	
	//中断发送方式
#if (IAP_CTRL_UART_TX_MODE == IAP_CTRL_INTERRUPT_TX_MODE)
	// 内部通讯接口的发送处理
	IAP_CTRL_UART_TxProcess(&iapCtrlUartCB);
#endif

}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  串口接收函数，串口的中断发送函数
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
void USART1_IRQHandler(void)
{
	// 判断DR是否有数据，中断接收
	if(usart_interrupt_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET) 	 
	{
		uint16 end = iapCtrlUartCB.rxFIFO.end;
		uint16 head = iapCtrlUartCB.rxFIFO.head;
		uint8 rxdata = 0x00;
		
		// 接收数据
		rxdata = (uint8)usart_data_receive(IAP_CTRL_UART_TYPE_DEF);

		// 一级缓冲区已满，不予接收
		if((end + 1)%IAP_CTRL_UART_RX_FIFO_SIZE == head)
		{
			return;
		}
		// 一级缓冲区未满，接收 
		else
		{
			// 将接收到的数据放到临时缓冲区中
			iapCtrlUartCB.rxFIFO.buff[end] = rxdata;
			iapCtrlUartCB.rxFIFO.end ++;
			iapCtrlUartCB.rxFIFO.end %= IAP_CTRL_UART_RX_FIFO_SIZE;
		}	

	}

// 中断模式发送
#if (IAP_CTRL_UART_TX_MODE == IAP_CTRL_INTERRUPT_TX_MODE)
	// 判断DR是否有数据，中断发送
	if(usart_interrupt_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = iapCtrlUartCB.tx.head;
		uint16 end;
		uint16 index = iapCtrlUartCB.tx.index;
		uint8 txdata = 0x00;

		// 执行到这里，说明上一个数据已经发送完毕，当前命令帧未发送完时，取出一个字节放到发送寄存器中
		if(index < iapCtrlUartCB.tx.cmdQueue[head].length)
		{
			txdata = iapCtrlUartCB.tx.cmdQueue[head].buff[iapCtrlUartCB.tx.index++];
			
			// 填充数据
			usart_data_transmit(IAP_CTRL_UART_TYPE_DEF, txdata);
		}
		// 当前命令帧发送完时，删除之
		else
		{
			iapCtrlUartCB.tx.cmdQueue[head].length = 0;
			iapCtrlUartCB.tx.head ++;
			iapCtrlUartCB.tx.head %= IAP_CTRL_UART_TX_QUEUE_SIZE;
			iapCtrlUartCB.tx.index = 0;

			head = iapCtrlUartCB.tx.head;
			end = iapCtrlUartCB.tx.end;
			
			// 命令帧队列非空，继续发送下一个命令帧
			if(head != end)
			{
				txdata = iapCtrlUartCB.tx.cmdQueue[head].buff[iapCtrlUartCB.tx.index++];

				// 填充数据
				usart_data_transmit(IAP_CTRL_UART_TYPE_DEF, txdata);
			}
			// 命令帧队列为空停止发送，设置空闲
			else
			{
				// 关闭发送空中断
				usart_interrupt_disable(IAP_CTRL_UART_TYPE_DEF, USART_INT_TC);
				
				iapCtrlUartCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif

	// Other USARTx interrupts handler can go here ...				 
	if(usart_interrupt_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- 接收溢出中断 	
	{
		// 用户手册 434  --- 软件先读USART_STAT0，再读USART_DATA可清除该位
		usart_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_FLAG_ORERR);		
		usart_data_receive(IAP_CTRL_UART_TYPE_DEF);						  //----------------------- 清空寄存器
	}
}

#if (IAP_CTRL_UART_TX_MODE == IAP_CTRL_INTERRUPT_TX_MODE)
// 启动中断字节发送
void IAP_CTRL_UART_IR_StartSendData(uint8 data)
{
	// 先读SR，再填充DR会把TC标志清掉
	usart_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_FLAG_TC);

	// 发送一个字节
	usart_data_transmit(IAP_CTRL_UART_TYPE_DEF, data);

	// 打开发送完成中断
	usart_interrupt_enable(IAP_CTRL_UART_TYPE_DEF, USART_INT_TC);	
}

#else
// 阻塞发送一个字节数据
void IAP_CTRL_UART_BC_SendData(uint8 data)
{	
	// 防止丢失第一个字节(清除空标志位,务必加) 
	usart_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_FLAG_TC);		

	// 填充数据
	usart_data_transmit(IAP_CTRL_UART_TYPE_DEF, data);

	// 未发送完，持续等待
	while(usart_flag_get(IAP_CTRL_UART_TYPE_DEF, USART_FLAG_TC) != SET);
}

// 启动阻塞发送
void IAP_CTRL_UART_BC_StartTx(IAP_CTRL_UART_CB* pCB)
{
	uint16 index = pCB->tx.index;
	uint16 head = pCB->tx.head;

	// 参数合法性检验
	if(NULL == pCB)
	{
		return;
	}
	
	// 当前命令帧未发送完时，持续发送
	while(index < pCB->tx.cmdQueue[head].length)
	{
		// 一直填充发送
		IAP_CTRL_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// 当前命令帧发送完时，删除之
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= IAP_CTRL_UART_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
}
#endif

// 发送处理,发现发送缓冲区非空时,启动中断发送
void IAP_CTRL_UART_TxProcess(IAP_CTRL_UART_CB* pCB)
{
	uint16 index = pCB->tx.index;							// 当前发送数据的索引号
	uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length;	// 当前发送的命令帧的长度
	uint16 head = pCB->tx.head;								// 发送命令帧队列头索引号
	uint16 end = pCB->tx.end;								// 发送命令帧队列尾索引号

	// 参数合法性检验
	if(NULL == pCB)
	{
		return;
	}
	
	// 队列为空，不处理
	if(head == end)
	{
		return;
	}

	// ■■执行到这里，说明队列非空■■
	
	// 当前命令帧未发送完时，取出一个字节放到发送寄存器中
	if(index < length)
	{
		// 发送忙，退出
		if(pCB->tx.txBusy)
		{
			return;
		}
		
		IAP_CTRL_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		// 设置发送忙状态
		pCB->tx.txBusy = TRUE;

	}
	// 当前命令帧发送完时，删除之
	else
	{
	//	pCB->tx.head ++;
	//	pCB->tx.head %= IAP_CTRL_UART_TX_QUEUE_SIZE;
	//	pCB->tx.index = 0;
	}
}

//===============================================================================
// 详细命令设计
// ==============================================================================
// 工程模式启用
void IAP_CTRL_UART_SendCmdProjectApply(uint32 para)
{
	uint8 param[] = IAP_VERSION;
	uint8 len = strlen((char*)param);
	uint8 i;
	
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_PROJECT_APPLY);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, len);		// 数据长度

	for (i = 0; i < len; i++)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param[i]);
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 工程模式准备就绪报告
void IAP_CTRL_UART_SendCmdProjectReady(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_PROJECT_READY);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 擦除FLASH结果上报
void IAP_CTRL_UART_SendCmdEraseFlashResult(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_IAP_ERASE_FLASH_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// IAP数据写入结果上报
void IAP_CTRL_UART_SendCmdWriteFlashResult(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_IAP_WRITE_FLASH_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 查空结果上报
void IAP_CTRL_UART_SendCmdCheckFlashBlankResult(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_CHECK_FLASH_BLANK_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 系统升级结束确认
void IAP_CTRL_UART_SendCmdUpdataFinishResult(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_UPDATA_FINISH_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);		// 数据长度
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 发送UI数据擦除结果
void IAP_CTRL_UART_SendCmdUIEraseResultReport(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_UI_DATA_ERASE_ACK);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 发送UI数据写入结果
void IAP_CTRL_UART_SendCmdUIWriteDataResultReport(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_UI_DATA_WRITE_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 二维码注册参数上报
void IAP_CTRL_UART_SendCmdQrRegParam(uint8 *param)
{
	uint8 len = strlen((char*)param);
	uint8 i;
	
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_QR_PARAM_REPORT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, len);		// 数据长度

	for (i = 0; i < len; i++)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param[i]);
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 发送写入二维码数据结果
void IAP_CTRL_UART_SendCmdQrWriteResultReport(uint8 param)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_UP_QR_DATA_WRITE_RESULT);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, param);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}


// 发送指定核对的版本
void IAP_CTRL_UART_SendVersionCheck(uint32 checkNum)
{
	uint8 *buff;
	uint8 i;
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_CHECK_VERSION);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);				// 数据长度

	// 根据版本号，选择发送的版本
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
			
		// 硬件版本
		case IAP_CTRL_UART_CHECK_VERSION_HW:
			buff = paramCB.runtime.hardVersion;
			break;
			
		// SN版本
		case IAP_CTRL_UART_CHECK_VERSION_SN_NUM:
			buff = paramCB.runtime.snCode;
			break;
			
		// 蓝牙MAC
		case IAP_CTRL_UART_CHECK_VERSION_BLE_MAC:
			buff = paramCB.runtime.bluMac;
			break;
			
		// 二维码字符
		case IAP_CTRL_UART_CHECK_VERSION_QR_CODE:
			buff = paramCB.runtime.qrCodeStr;
			break;

		default:
			buff = NULL;
			break;
	
	}

	// 判断指针是否合法
	if (NULL == buff)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, checkNum & 0xFF);		// 比对号
	}
	else
	{		
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, checkNum & 0xFF);		// 比对号
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, buff[0]);		// 版本长度

		for (i = 0; i < buff[0]; i++)
		{
			IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, buff[1 + i]);		// 版本字符内容
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
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);				// 数据长度

	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 64);			// 标志区长度

	buff = paramCB.runtime.flagArr;
	for (i = 0; i < 64; i++)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, buff[i]);		// 版本字符内容
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}


// 发送命令带结果
void IAP_CTRL_UART_SendCmdWithResult(uint8 cmdWord, uint8 result)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, cmdWord);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 1);		// 数据长度
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, result);
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 发送命令无结果
void IAP_CTRL_UART_SendCmdNoResult(uint8 cmdWord)
{
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, cmdWord);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, 0);		// 数据长度
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);
}

// 读取数据上报
void IAP_CTRL_UART_SendSpiFlashData(uint32 ReadAddr, uint8 NumByteToRead)
{
	uint16 i;
	uint8* pFlashBuff = NULL;				// 保存读取扇区的数据

	pFlashBuff = (uint8 *)malloc(NumByteToRead);
	if (NULL == pFlashBuff)
	{
		return;
	}
	
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_HEAD);
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, IAP_CTRL_UART_CMD_SPI_FLASH_ABSOLUTE_READ);
	
	// 发送数据长度，随意填写即可
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, (128+4));

	// 发送读取的地址
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, ((ReadAddr & 0xFF000000) >> 24));
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, ((ReadAddr & 0x00FF0000) >> 16));
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, ((ReadAddr & 0x0000FF00) >> 8));
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, ((ReadAddr & 0x000000FF) >> 0));

	// 发送读取数据字节数
	IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, (NumByteToRead & 0xFF));

	// 从外部spiflash读取数据
	SPI_FLASH_ReadArray(pFlashBuff, ReadAddr, NumByteToRead);
	// 填充数据到命令帧数据中
	for (i = 0; i < NumByteToRead; i++)
	{
		IAP_CTRL_UART_TxAddData(&iapCtrlUartCB, pFlashBuff[i]);
	}
	
	IAP_CTRL_UART_TxAddFrame(&iapCtrlUartCB);

	free(pFlashBuff);
}


#if IAP_CTRL_UART_RX_TIME_OUT_CHECK_ENABLE
// RX通讯超时处理-单向
void IAP_CTRL_UART_CALLBACK_RxTimeOut(uint32 param)
{
	IAP_CTRL_CMD_FRAME* pCmdFrame = NULL;

	pCmdFrame = &iapCtrlUartCB.rx.cmdQueue[iapCtrlUartCB.rx.end];

	// 超时错误，将命令帧长度清零，即认为抛弃该命令帧
	pCmdFrame->length = 0;	// 2016.1.6增加
	// 删除当前的命令头，而不是删除已分析完的所有数据，因为数据中可能会有命令头
	iapCtrlUartCB.rxFIFO.head ++;
	iapCtrlUartCB.rxFIFO.head %= IAP_CTRL_UART_RX_FIFO_SIZE;
	iapCtrlUartCB.rxFIFO.currentProcessIndex = iapCtrlUartCB.rxFIFO.head;
}

// 停止Rx通讯超时检测任务
void IAP_CTRL_UART_StopRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_RX_TIME_OUT_CONTROL);
}
#endif

#if IAP_CTRL_UART_TXRX_TIME_OUT_CHECK_ENABLE
// TXRX通讯超时处理-双向
void IAP_CTRL_UART_CALLBACK_TxRxTimeOut(uint32 param)
{
	
}

// 停止TxRX通讯超时检测任务
void IAP_CTRL_UART_StopTxRxTimeOutCheck(void)
{
	TIMER_KillTask(TIMER_ID_UART_TXRX_TIME_OUT_CONTROL);
}
#endif

// 发送自身MCU的唯一序列号，防止误入老化
void UART_PROTOCOL_SendMcuUid(uint32 param)
{
	uint8 buff[UART_PROTOCOL_MCU_UID_BUFF_LENGTH];
	uint8 i;

	// 按位发送sn0
	buff[0] = iapCB.mcuUID.sn0 & 0xff;
	buff[1] = (iapCB.mcuUID.sn0 >> 8)  & 0xff;
	buff[2] = (iapCB.mcuUID.sn0 >> 16) & 0xff;
	buff[3] = (iapCB.mcuUID.sn0 >> 24) & 0xff;

	// 按位发送sn1
	buff[4] = iapCB.mcuUID.sn1 & 0xff;
	buff[5] = (iapCB.mcuUID.sn1 >> 8)  & 0xff;
	buff[6] = (iapCB.mcuUID.sn1 >> 16) & 0xff;
	buff[7] = (iapCB.mcuUID.sn1 >> 24) & 0xff;

	// 按位发送sn2
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


