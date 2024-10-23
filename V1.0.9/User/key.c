#include "common.h"
#include "key.h"
#include "keymsgprocess.h"
#include "state.h"
#include "timer.h"
#include "param.h"

KEY_CB keyCB;

void KEY_Init(void)
{
    rcu_periph_clock_enable(KEY_CHECK_RCU_PORT);
    gpio_init(KEY_CHECK_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY_CHECK_PIN);

    // һ��Ҫ��KEYMSG_RegisterKeyScanCallback()����֮ǰ����
    KEYMSG_Init();

    // ע�ᰴ��ɨ��ӿ�
    KEYMSG_RegisterKeyScanCallback(KEY_CALLBACK_Scan);
		
		memset(keyCB.buff,      0xff, KEY_NAME_MAX);
		memset(keyCB.lastValue, 0xff, KEY_NAME_MAX);
		
}

//// ��ʱ���ص������ͷ�+��-����������
//void KEY_CALLBACK_ClearAddDecKeyUpMaskRequest(uint32 param)
//{
//    // ���+����-�����ͷ���������
//    KEYMSG_ClearMaskRequest(KEY_NAME_ADD, KEY_MSG_UP);
//    KEYMSG_ClearMaskRequest(KEY_NAME_DEC, KEY_MSG_UP);

//    // +�����¿���
//    keyMsgCB.addKeyInvalidRequest = FALSE;
//}

//// ��ʱ���ص������ͷ�+��i����������
//void KEY_CALLBACK_ClearAddInfoKeyUpMaskRequest(uint32 param)
//{
//    // ���+����i�����ͷ���������
//    KEYMSG_ClearMaskRequest(KEY_NAME_ADD, KEY_MSG_UP);
//    KEYMSG_ClearMaskRequest(KEY_NAME_INFO, KEY_MSG_UP);

//    // +�����¿���
//    keyMsgCB.addKeyInvalidRequest = FALSE;
//}

uint8 keyRead = 0;
void KEY_CALLBACK_Scan(uint8 *p8bitKeyValueArray)
{
    uint8 keyMask = 0x07;
//    uint8 keyAddDecMask = 0x03;     // �Ӽ�ͬʱ��ʱ
//    uint8 keyAddInfo = 0x05;        // +,iͬʱ����
    uint8 mask = (0xFF >> (8 - KEY_FILTER_LEVEL_N)); //�����˲�������
    uint8 i;
    uint8 keyValue = 0;

    keyRead = gpio_input_bit_get(KEY_CHECK_PORT, KEY_CHECK_PIN); // ������
//  keyRead <<= 1;
//  keyRead |= GPIO_ReadInputDataBit(GPIOA, GPIO_PIN_15);//-
//  keyRead <<= 1;
//  keyRead |= GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_3);//+

    // �����������Ӱ����˲���ʩ����
    // ����һ��������ȡ���İ������뻺����  eg:keyRead=111  �任����
    for (i = 0; i < KEY_NAME_MAX; i++) //5-3=2  ��ϰ�������Ҫ�������˲����������ѭ������-1
    {
        // ��ȡ����ÿ����������Ϣ��ӵ���Ӧ�Ļ�������
        keyCB.buff[i] <<= 1;
        keyCB.buff[i] |= (keyRead >> i) & 0x01;
    }

    // ���ڶ�������������������N�ζ���ͬһ��״̬��ȷ�ϣ�N���趨
    for (i = 0; i < KEY_NAME_MAX; i++)
    {
        // ��ǰ�����������ӵ�λ��ʼ����N����Ϊ1���ж�Ϊ�ͷ�
        if ((keyCB.buff[i] & mask) == mask)
        {
            keyCB.lastValue[i] = 0x01 << i;
        }
        // �ӵ�λ��ʼ����N����Ϊ0�����ж�Ϊ����
        else if (0 == (keyCB.buff[i] & mask))
        {
            keyCB.lastValue[i] = 0;
        }
        // ���ೡ����Ϊ���ȶ��ڣ�������
        else
        {}

        keyValue |= keyCB.lastValue[i];
    }

//  PARAM_SetKeyValue(keyRead);

    // ��λ�����Ϊ̧��״̬����û�а���
    keyValue |= 0x18;

    // ��������ȫ���ͷţ����������Ϣȫ����������
    if (keyMask == (keyValue & keyMask))
    {
        // ���ȫ����������
        KEYMSG_ClearGlobalMaskRequest();

//        // ��ϼ����������+-�������ܣ����Ե������ͷź�ָ����ʱ���¿���UP����
//        if (KEYMSG_GetAddDecKeyUpRecoveryRequest())
//        {
//            KEYMSG_SetAddDecKeyUpRecoveryRequest(FALSE);

//            // ����һ��+-�������ʱ����ָ��ʱ��ָ�������
//            TIMER_AddTask(TIMER_ID_RECOVERY_KEY_UP_ADD_DEC,
//                          KEY_SCAN_TIME * 2,
//                          KEY_CALLBACK_ClearAddDecKeyUpMaskRequest,
//                          TRUE,
//                          1,
//                          ACTION_MODE_ADD_TO_QUEUE);
//        }

//        // ��ϼ����������+ i�������ܣ����Ե������ͷź�ָ����ʱ���¿���UP����
//        if (KEYMSG_GetAddInfoKeyUpRecoveryRequest())
//        {
//            KEYMSG_SetAddInfoKeyUpRecoveryRequest(FALSE);

//            // ����һ��+i�������ʱ����ָ��ʱ��ָ�������
//            TIMER_AddTask(TIMER_ID_RECOVERY_KEY_UP_ADD_INFO,
//                          KEY_SCAN_TIME * 2,
//                          KEY_CALLBACK_ClearAddInfoKeyUpMaskRequest,
//                          TRUE,
//                          1,
//                          ACTION_MODE_ADD_TO_QUEUE);
//        }
    }

//    // ��⵽�Ӽ�����ͬʱ���£�����ԭ��ͼ�޸�(bit0, +; bit1, -; bit2, i)
//    if (0 == (keyValue & keyAddDecMask))
//    {
//        // ֻҪ����ϼ�������������ȫ�������ͷź����»ָ�+-��������
//        KEYMSG_SetAddDecKeyUpRecoveryRequest(TRUE);

//        // �Ƴ�����ʹ�ܺ�+����������Ϣ������
////      if (PARAM_IsPushAssistOn())
////      {
////          return;
////      }

//        // ������İ�����λ
//        keyValue &= ~KEY_MASK_ADD_DEC;

//        // ���������İ���
//        keyValue |= keyAddDecMask;

//        // ���μӼ��ͼ������ͷ���Ϣ
//        KEYMSG_SetMaskRequest(KEY_NAME_ADD, KEY_MSG_UP);
//        KEYMSG_SetMaskRequest(KEY_NAME_DEC, KEY_MSG_UP);
//    }

//    // ��⵽+,i����ͬʱ���£�����ԭ��ͼ�޸�(bit0, +; bit1, -; bit2, i)
//    if (0 == (keyValue & keyAddInfo))
//    {
//        // ֻҪ����ϼ�������������ȫ�������ͷź����»ָ�+-��������
//        KEYMSG_SetAddInfoKeyUpRecoveryRequest(TRUE);

//        // ������İ�����λ
//        keyValue &= ~KEY_MASK_ADD_INFO;

//        // ���������İ���
//        keyValue |= keyAddInfo;

//        // ���μӼ���i�����ͷ���Ϣ
//        KEYMSG_SetMaskRequest(KEY_NAME_ADD, KEY_MSG_UP);
//        KEYMSG_SetMaskRequest(KEY_NAME_INFO, KEY_MSG_UP);
//    }
    *p8bitKeyValueArray = keyValue;

    // ֻҪ�а������������������Զ��ػ�ʱ��
//  if (keyValue != KEY_MASK_ALL)
//  {
//      // �����Զ��ػ�����
//      STATE_ResetAutoPowerOffControl();
//  }
}
