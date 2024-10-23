#include "key.h"
#include "adc1.h"
#include "timer.h"
#include "meterControl.h"

// 按键对应的电压值
const float KeyConstVolBuf[KEY_NUM_MAX] = {2.2,1.65,1.10,0.55};
const AVOMETER_GEARS_CMD KeyConstStitchGear[KEY_NUM_MAX] = {AVOMETER_GEARS_DCV,AVOMETER_GEARS_DCA,AVOMETER_GEARS_DCMA,AVOMETER_GEARS_OHM};
float keyVal;
uint8_t KeyChangeGear = 0;
// 按键处理函数
void KeyScanTimeCallBack(uint32 prame)
{
    uint8_t i = 0;
    static uint8_t lastKey,KeyOffest,pressDownFlag;
    // 获取ADC 引脚电压值
    keyVal = Get_ADC1_Value(ADC_CH_5) * 3.3 / 4096;
    for(i = 0; i < KEY_NUM_MAX; i++)
    {
        if(keyVal >= KeyConstVolBuf[i] * KEY_VOL_THRESHOLD_MIN && keyVal <= KeyConstVolBuf[i] * KEY_VOL_THRESHOLD_MAX)
        {
            break;
        }
    }
		// 遍历出按键是无效值 可以认为按键已经释放
    if(KEY_NUM_MAX == i)
    {
        pressDownFlag = bFALSE;
        return;
    }
		// 长按状态下不响应 只有释放过一次才会相应
    if(bTRUE == pressDownFlag)
    {
        return;
    }
    // 上一次按键一样
    if(lastKey == i)
    {
        // 对于所有的非欧姆档按键可以交直流切换
        KeyOffest = (KEY_NUM_MAX - 1 == i)? 0 :(KeyOffest + 1) % 2;
    }
    // 切换档位
    Avoment_StitchGear((AVOMETER_GEARS_CMD)(KeyConstStitchGear[i] + KeyOffest));
    lastKey = i;
    pressDownFlag = bTRUE;
}


// 按键初始化
void KEY_Init(void)
{
    // ADC初始化
    ADC1_Init();
    // 添加万用表接收消息定时器
    TIMER_AddTask(TIMER_ID_KEY_PROCESS,
        50,
        KeyScanTimeCallBack,
        0,
        TIMER_LOOP_FOREVER,
        ACTION_MODE_ADD_TO_QUEUE);

}


