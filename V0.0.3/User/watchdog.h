#ifndef __WATCH_H_
#define __WATCH_H_

// ���Ź�ʹ�ܿ���
#define WDT_ENABLE              0

#define WDT_RESET_TIME 			3000U		// ���Ź���λʱ�䣬��λ:ms

// ��������
void WDT_Init(void);
void WDT_Clear(void);

#endif
