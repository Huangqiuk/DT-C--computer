#include "common.h"
#include "key.h"
#include "keymsgprocess.h"
#include "timer.h"
#include "param.h"

void KEY_Init(void)
{	 
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_0);
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_1);
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_2);
	
	// һ��Ҫ��KEYMSG_RegisterKeyScanCallback()����֮ǰ����
	KEYMSG_Init();
	
	// ע�ᰴ��ɨ��ӿ�
	KEYMSG_RegisterKeyScanCallback(KEY_CALLBACK_Scan);
}

void KEY_CALLBACK_Scan(uint8* p8bitKeyValueArray)
{
	uint8 keyMask = 0x1F;			// �൱����ʵ�尴������
	uint8 keyAddDecMask = 0x05;		// �Ӽ�ͬʱ��ʱ
	uint8 keyValue = 0;

	keyValue = (gpio_input_port_get(GPIOC) & keyMask) | KEY_MASK_ADD_DEC;

	*p8bitKeyValueArray = keyValue;

	// ֻҪ�а������������������Զ��ػ�ʱ��
	if (keyValue != KEY_MASK_ALL)
	{
	}
}

