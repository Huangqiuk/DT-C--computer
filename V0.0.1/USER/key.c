#include "key.h"
#include "adc1.h"
#include "timer.h"
#include "meterControl.h"

// ������Ӧ�ĵ�ѹֵ
const float KeyConstVolBuf[KEY_NUM_MAX] = {2.2,1.65,1.10,0.55};
const AVOMETER_GEARS_CMD KeyConstStitchGear[KEY_NUM_MAX] = {AVOMETER_GEARS_DCV,AVOMETER_GEARS_DCA,AVOMETER_GEARS_DCMA,AVOMETER_GEARS_OHM};
float keyVal;
uint8_t KeyChangeGear = 0;
// ����������
void KeyScanTimeCallBack(uint32 prame)
{
    uint8_t i = 0;
    static uint8_t lastKey,KeyOffest,pressDownFlag;
    // ��ȡADC ���ŵ�ѹֵ
    keyVal = Get_ADC1_Value(ADC_CH_5) * 3.3 / 4096;
    for(i = 0; i < KEY_NUM_MAX; i++)
    {
        if(keyVal >= KeyConstVolBuf[i] * KEY_VOL_THRESHOLD_MIN && keyVal <= KeyConstVolBuf[i] * KEY_VOL_THRESHOLD_MAX)
        {
            break;
        }
    }
		// ��������������Чֵ ������Ϊ�����Ѿ��ͷ�
    if(KEY_NUM_MAX == i)
    {
        pressDownFlag = bFALSE;
        return;
    }
		// ����״̬�²���Ӧ ֻ���ͷŹ�һ�βŻ���Ӧ
    if(bTRUE == pressDownFlag)
    {
        return;
    }
    // ��һ�ΰ���һ��
    if(lastKey == i)
    {
        // �������еķ�ŷķ���������Խ�ֱ���л�
        KeyOffest = (KEY_NUM_MAX - 1 == i)? 0 :(KeyOffest + 1) % 2;
    }
    // �л���λ
    Avoment_StitchGear((AVOMETER_GEARS_CMD)(KeyConstStitchGear[i] + KeyOffest));
    lastKey = i;
    pressDownFlag = bTRUE;
}


// ������ʼ��
void KEY_Init(void)
{
    // ADC��ʼ��
    ADC1_Init();
    // ������ñ������Ϣ��ʱ��
    TIMER_AddTask(TIMER_ID_KEY_PROCESS,
        50,
        KeyScanTimeCallBack,
        0,
        TIMER_LOOP_FOREVER,
        ACTION_MODE_ADD_TO_QUEUE);

}


