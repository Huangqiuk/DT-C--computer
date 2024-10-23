#include "common.h"
#include "system.h"
#include "timer.h"
#include "PowerDrive.h"
// #include "Param.h"
#include "POWERProtocol.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/
// UART��ʼ��
void POWER_UART_HwInit(uint32 baud);

// ���ݽṹ��ʼ��
void POWER_UART_DataStructureInit(POWER_UART_CB *pCB);

// ���ʹ���
void POWER_UART_TxProcess(POWER_UART_CB *pCB);

// �����ж��ֽڷ���
void POWER_UART_IR_StartSendData(uint8 data);

// ��������һ���ֽ�����
void POWER_UART_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void USART2_IRQHandler(void);

// BLEģ�鸨����������
void POWER_PROTOCOL_CMD_GPIO_CtrlPin(void);

// ȫ�ֱ�������
POWER_UART_CB POWERUartCB;

// UART��ʼ��
void POWER_UART_HwInit(uint32 baud)
{
    // ��ʱ��
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10); // TX

    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_11);  //RX

    // UARTʱ������
    rcu_periph_clock_enable(RCU_USART2);
    usart_deinit(POWER_UART_TYPE_DEF); // ��λ����

    // ��������
    usart_baudrate_set(POWER_UART_TYPE_DEF, baud);                        // ������
    usart_word_length_set(POWER_UART_TYPE_DEF, USART_WL_8BIT);            // 8λ����λ
    usart_stop_bit_set(POWER_UART_TYPE_DEF, USART_STB_1BIT);                  // һ��ֹͣλ
    usart_parity_config(POWER_UART_TYPE_DEF, USART_PM_NONE);                  // ����żУ��
    usart_hardware_flow_rts_config(POWER_UART_TYPE_DEF, USART_RTS_DISABLE); // ��Ӳ������������
    usart_hardware_flow_cts_config(POWER_UART_TYPE_DEF, USART_CTS_DISABLE);
    usart_transmit_config(POWER_UART_TYPE_DEF, USART_TRANSMIT_ENABLE); // ʹ�ܷ���
    usart_receive_config(POWER_UART_TYPE_DEF, USART_RECEIVE_ENABLE);     // ʹ�ܽ���

    // �ж�����
    nvic_irq_enable(POWER_UART_IRQn_DEF, 2, 1);

    usart_interrupt_enable(POWER_UART_TYPE_DEF, USART_INT_RBNE); // �����ж�

    usart_enable(POWER_UART_TYPE_DEF); // ʹ�ܴ���
}

// ���ݽṹ��ʼ��
void POWER_UART_DataStructureInit(POWER_UART_CB *pCB)
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
    for (i = 0; i < POWER_UART_DRIVE_TX_QUEUE_SIZE; i++)
    {
        pCB->tx.cmdQueue[i].length = 0;
    }

    POWERUartCB.rcvBleMacOK = FALSE;
}

// ���ʹ���
void POWER_UART_TxProcess(POWER_UART_CB *pCB)
{
// �жϷ�ʽ
#if (POWER_UART_TX_MODE == POWER_UART_INTERRUPT_TX_MODE)
    uint16 index = pCB->tx.index;                          // ��ǰ�������ݵ�������
    uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length; // ��ǰ���͵�����֡�ĳ���
    uint16 head = pCB->tx.head;                            // ��������֡����ͷ������
    uint16 end = pCB->tx.end;                              // ��������֡����β������

    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return;
    }

    // ����Ϊ�գ�������
    if (head == end)
    {
        return;
    }

    // ����æ���˳�
    if (pCB->tx.txBusy)
    {
        return;
    }

    // ����ִ�е����˵�����зǿա���

    // ��ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
    if (index < length)
    {
        POWER_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

        // ���÷���æ״̬
        pCB->tx.txBusy = TRUE;
    }

// ������ʽʱ��������
#else
    uint16 index = pCB->tx.index;
    uint16 head = pCB->tx.head;
    uint16 end = pCB->tx.end;

    // �����Ϸ��Լ���
    if (NULL == pCB)
    {
        return;
    }

    // ����Ϊ�գ�������
    if (head == end)
    {
        return;
    }

    // ��ǰ����֡δ������ʱ����������
    while (index < pCB->tx.cmdQueue[head].length)
    {
        // һֱ��䷢��
        POWER_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

        index = pCB->tx.index;
    }

    // ��ǰ����֡������ʱ��ɾ��֮
    pCB->tx.cmdQueue[head].length = 0;
    pCB->tx.head++;
    pCB->tx.head %= POWER_UART_DRIVE_TX_QUEUE_SIZE;
    pCB->tx.index = 0;
#endif
}

// �����ж��ֽڷ���
void POWER_UART_IR_StartSendData(uint8 data)
{
    // �ȶ�SR�������DR���TC��־���
    usart_flag_get(POWER_UART_TYPE_DEF, USART_FLAG_TC);

    // ����һ���ֽ�
    usart_data_transmit(POWER_UART_TYPE_DEF, data);

    // �򿪷�������ж�
    usart_interrupt_enable(POWER_UART_TYPE_DEF, USART_INT_TC);
}

// ��������һ���ֽ�����
void POWER_UART_BC_SendData(uint8 data)
{
    // ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�)
    usart_flag_get(POWER_UART_TYPE_DEF, USART_FLAG_TC);

    // �������
    usart_data_transmit(POWER_UART_TYPE_DEF, data);

    // δ�����꣬�����ȴ�
    while (usart_flag_get(POWER_UART_TYPE_DEF, USART_FLAG_TC) != SET);
}


/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None
 * @Notes
 *************************************************************/
uint8 POWERRxBuff[200] = {0};
//uint8 callringBuff[20] = {0};
//uint8 bleIndex = 0;

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None
 * @Notes
 *************************************************************/
void USART2_IRQHandler(void)
{
    // �ж�DR�Ƿ������ݣ��жϽ���
    if (usart_interrupt_flag_get(POWER_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET)
    {
        uint8 rxdata = 0x00;

        // ��������
        rxdata = (uint8)usart_data_receive(POWER_UART_TYPE_DEF);

        if (NULL != POWERUartCB.receiveDataThrowService)
        {
            (*POWERUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
        }

        if (!POWERUartCB.rcvBleMacOK)
        {
            //          BLE_UART_AtChannelDataProcess(rxdata);
        }

    }

// �ж�ģʽ����
#if (POWER_UART_TX_MODE == POWER_UART_INTERRUPT_TX_MODE)
    // �ж�DR�Ƿ������ݣ��жϷ���
    if (usart_interrupt_flag_get(POWER_UART_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
    {
        uint16 head = POWERUartCB.tx.head;
        uint16 end;
        uint16 index = POWERUartCB.tx.index;
        uint8 txdata = 0x00;

        // ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
        if (index < POWERUartCB.tx.cmdQueue[head].length)
        {
            txdata = POWERUartCB.tx.cmdQueue[head].buff[POWERUartCB.tx.index++];

            // �������
            usart_data_transmit(POWER_UART_TYPE_DEF, txdata);
        }
        // ��ǰ����֡������ʱ��ɾ��֮
        else
        {
            POWERUartCB.tx.cmdQueue[head].length = 0;
            POWERUartCB.tx.head++;
            POWERUartCB.tx.head %= POWER_UART_DRIVE_TX_QUEUE_SIZE;
            POWERUartCB.tx.index = 0;

            head = POWERUartCB.tx.head;
            end = POWERUartCB.tx.end;

            // ����֡���зǿգ�����������һ������֡
            if (head != end)
            {
                txdata = POWERUartCB.tx.cmdQueue[head].buff[POWERUartCB.tx.index++];

                // �������
                usart_data_transmit(POWER_UART_TYPE_DEF, txdata);
            }
            // ����֡����Ϊ��ֹͣ���ͣ����ÿ���
            else
            {
                // �رշ�������ж�
                usart_interrupt_disable(POWER_UART_TYPE_DEF, USART_INT_TC);

                POWERUartCB.tx.txBusy = FALSE;
            }
        }
    }
#endif

    // Other USARTx interrupts handler can go here ...
    if (usart_interrupt_flag_get(POWER_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- ��������ж�
    {
        usart_flag_get(POWER_UART_TYPE_DEF, USART_FLAG_ORERR);                          //----------------------- �����������жϱ�־λ
        usart_data_receive(POWER_UART_TYPE_DEF);                                            //----------------------- ��ռĴ���
    }
}

// UART��ʼ��
void POWER_UART_Init(void)
{
    // POWER���ڳ�ʼ��
    POWER_UART_HwInit(POWER_UART_BAUD_RATE);

    // POWER���ݽṹ��ʼ��
    POWER_UART_DataStructureInit(&POWERUartCB);
}

// UARTģ�鴦�����
void POWER_UART_Process(void)
{
    // ���ʹ���
    POWER_UART_TxProcess(&POWERUartCB);
}

// ע�������׳��ӿڷ���
void POWER_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
    POWERUartCB.receiveDataThrowService = service;
}

// ���ͻ����������һ������������
BOOL POWER_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
    uint16 i;
    uint16 head = POWERUartCB.tx.head;
    uint16 end = POWERUartCB.tx.end;

    // ��������
    if ((NULL == pArray) || (0 == length))
    {
        return FALSE;
    }

    // ���ͻ������������������
    if ((end + 1) % POWER_UART_DRIVE_TX_QUEUE_SIZE == head)
    {
        return FALSE;
    }

    POWERUartCB.tx.cmdQueue[end].deviceID = id;
    for (i = 0; i < length; i++)
    {
        POWERUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
    }
    POWERUartCB.tx.cmdQueue[end].length = length;

    // ���ͻ��ζ��и���λ��
    POWERUartCB.tx.end++;
    POWERUartCB.tx.end %= POWER_UART_DRIVE_TX_QUEUE_SIZE;
    POWERUartCB.tx.cmdQueue[POWERUartCB.tx.end].length = 0;

    return TRUE;
}
