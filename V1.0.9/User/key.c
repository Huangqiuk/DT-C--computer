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

    // 一定要在KEYMSG_RegisterKeyScanCallback()函数之前调用
    KEYMSG_Init();

    // 注册按键扫描接口
    KEYMSG_RegisterKeyScanCallback(KEY_CALLBACK_Scan);
		
		memset(keyCB.buff,      0xff, KEY_NAME_MAX);
		memset(keyCB.lastValue, 0xff, KEY_NAME_MAX);
		
}

//// 定时器回调重新释放+、-键弹起请求
//void KEY_CALLBACK_ClearAddDecKeyUpMaskRequest(uint32 param)
//{
//    // 清除+键和-键的释放屏蔽请求
//    KEYMSG_ClearMaskRequest(KEY_NAME_ADD, KEY_MSG_UP);
//    KEYMSG_ClearMaskRequest(KEY_NAME_DEC, KEY_MSG_UP);

//    // +键重新开放
//    keyMsgCB.addKeyInvalidRequest = FALSE;
//}

//// 定时器回调重新释放+、i键弹起请求
//void KEY_CALLBACK_ClearAddInfoKeyUpMaskRequest(uint32 param)
//{
//    // 清除+键和i键的释放屏蔽请求
//    KEYMSG_ClearMaskRequest(KEY_NAME_ADD, KEY_MSG_UP);
//    KEYMSG_ClearMaskRequest(KEY_NAME_INFO, KEY_MSG_UP);

//    // +键重新开放
//    keyMsgCB.addKeyInvalidRequest = FALSE;
//}

uint8 keyRead = 0;
void KEY_CALLBACK_Scan(uint8 *p8bitKeyValueArray)
{
    uint8 keyMask = 0x07;
//    uint8 keyAddDecMask = 0x03;     // 加减同时按时
//    uint8 keyAddInfo = 0x05;        // +,i同时按下
    uint8 mask = (0xFF >> (8 - KEY_FILTER_LEVEL_N)); //按键滤波的掩码
    uint8 i;
    uint8 keyValue = 0;

    keyRead = gpio_input_bit_get(KEY_CHECK_PORT, KEY_CHECK_PIN); // 启动键
//  keyRead <<= 1;
//  keyRead |= GPIO_ReadInputDataBit(GPIOA, GPIO_PIN_15);//-
//  keyRead <<= 1;
//  keyRead |= GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_3);//+

    // ■■这里增加按键滤波措施■■
    // 【第一步】将读取到的按键放入缓冲区  eg:keyRead=111  变换过程
    for (i = 0; i < KEY_NAME_MAX; i++) //5-3=2  组合按键不需要在这里滤波，因此这里循环上限-1
    {
        // 读取到的每个按键的信息添加到对应的缓冲区里
        keyCB.buff[i] <<= 1;
        keyCB.buff[i] |= (keyRead >> i) & 0x01;
    }

    // 【第二步】缓冲区处理，连续N次读到同一个状态才确认，N可设定
    for (i = 0; i < KEY_NAME_MAX; i++)
    {
        // 当前按键缓冲区从低位开始连续N个均为1，判定为释放
        if ((keyCB.buff[i] & mask) == mask)
        {
            keyCB.lastValue[i] = 0x01 << i;
        }
        // 从低位开始连续N个均为0，则判定为按下
        else if (0 == (keyCB.buff[i] & mask))
        {
            keyCB.lastValue[i] = 0;
        }
        // 其余场景，为不稳定期，不处理
        else
        {}

        keyValue |= keyCB.lastValue[i];
    }

//  PARAM_SetKeyValue(keyRead);

    // 复位虚拟键为抬起状态，即没有按下
    keyValue |= 0x18;

    // 三个按键全部释放，清除按键消息全局屏蔽请求
    if (keyMask == (keyValue & keyMask))
    {
        // 清除全局屏蔽请求
        KEYMSG_ClearGlobalMaskRequest();

//        // 组合键引起的屏蔽+-键弹起功能，所以当按键释放后指定延时重新开启UP功能
//        if (KEYMSG_GetAddDecKeyUpRecoveryRequest())
//        {
//            KEYMSG_SetAddDecKeyUpRecoveryRequest(FALSE);

//            // 开启一个+-键弹起后定时器，指定时间恢复弹起功能
//            TIMER_AddTask(TIMER_ID_RECOVERY_KEY_UP_ADD_DEC,
//                          KEY_SCAN_TIME * 2,
//                          KEY_CALLBACK_ClearAddDecKeyUpMaskRequest,
//                          TRUE,
//                          1,
//                          ACTION_MODE_ADD_TO_QUEUE);
//        }

//        // 组合键引起的屏蔽+ i键弹起功能，所以当按键释放后指定延时重新开启UP功能
//        if (KEYMSG_GetAddInfoKeyUpRecoveryRequest())
//        {
//            KEYMSG_SetAddInfoKeyUpRecoveryRequest(FALSE);

//            // 开启一个+i键弹起后定时器，指定时间恢复弹起功能
//            TIMER_AddTask(TIMER_ID_RECOVERY_KEY_UP_ADD_INFO,
//                          KEY_SCAN_TIME * 2,
//                          KEY_CALLBACK_ClearAddInfoKeyUpMaskRequest,
//                          TRUE,
//                          1,
//                          ACTION_MODE_ADD_TO_QUEUE);
//        }
    }

//    // 检测到加减按键同时按下，根据原理图修改(bit0, +; bit1, -; bit2, i)
//    if (0 == (keyValue & keyAddDecMask))
//    {
//        // 只要是组合键按下立刻允许当全部按键释放后重新恢复+-键弹起功能
//        KEYMSG_SetAddDecKeyUpRecoveryRequest(TRUE);

//        // 推车助力使能后+键的任意消息都拦截
////      if (PARAM_IsPushAssistOn())
////      {
////          return;
////      }

//        // 将虚拟的按键置位
//        keyValue &= ~KEY_MASK_ADD_DEC;

//        // 消除单独的按键
//        keyValue |= keyAddDecMask;

//        // 屏蔽加键和减键的释放消息
//        KEYMSG_SetMaskRequest(KEY_NAME_ADD, KEY_MSG_UP);
//        KEYMSG_SetMaskRequest(KEY_NAME_DEC, KEY_MSG_UP);
//    }

//    // 检测到+,i按键同时按下，根据原理图修改(bit0, +; bit1, -; bit2, i)
//    if (0 == (keyValue & keyAddInfo))
//    {
//        // 只要是组合键按下立刻允许当全部按键释放后重新恢复+-键弹起功能
//        KEYMSG_SetAddInfoKeyUpRecoveryRequest(TRUE);

//        // 将虚拟的按键置位
//        keyValue &= ~KEY_MASK_ADD_INFO;

//        // 消除单独的按键
//        keyValue |= keyAddInfo;

//        // 屏蔽加键和i键的释放消息
//        KEYMSG_SetMaskRequest(KEY_NAME_ADD, KEY_MSG_UP);
//        KEYMSG_SetMaskRequest(KEY_NAME_INFO, KEY_MSG_UP);
//    }
    *p8bitKeyValueArray = keyValue;

    // 只要有按键触发就立刻重设自动关机时间
//  if (keyValue != KEY_MASK_ALL)
//  {
//      // 重置自动关机任务
//      STATE_ResetAutoPowerOffControl();
//  }
}
