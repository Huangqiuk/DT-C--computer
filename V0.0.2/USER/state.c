#include "common.h"
#include "state.h"
#include "timer.h"
#include "iap.h"
#include "powerCtl.h"
#include "param.h"
#include "pwmled.h"
#include "uartProtocol.h"

void  STATE_UpradeProgress(uint32 Progress);

// 状态机结构体定义
STATE_CB stateCB;

// 状态机初始化
void STATE_Init(void)
{
    // 默认状态为空
    stateCB.state = STATE_NULL;
    stateCB.preState = STATE_NULL;

    // 延时进入入口状态
    TIMER_AddTask(TIMER_ID_STATE_CONTROL,
                  10,
                  STATE_EnterState,
                  STATE_NULL,
                  1,
                  ACTION_MODE_ADD_TO_QUEUE);
}

uint32 upAppDataSize = 0;

// 每个状态的入口处理
void STATE_EnterState(uint32 state)
{
    uint8 updateFlag = 0;
    uint32 updateBuff[1] = {0};
    uint32 buff[1] = {0};
    
    // 让当前的状态成为历史
    stateCB.preState = stateCB.state;

    // 设置新的状态
    stateCB.state = (STATE_E)state;

    // 各状态的入口设定
    switch (state)
    {
        // ■■ 空状态 ■■

        case STATE_NULL:

            // 查询升级标志
            // 清除蓝牙升级标志和屏蔽蓝牙模块初始化标志
            updateFlag = IAP_FlashReadWord(PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS + 3);

            if (0xAA == updateFlag)
            {
                STATE_EnterState(STATE_ENTRY);
            }
            else
            {
                STATE_EnterState(STATE_JUMP);
            }
            break;


        // ■■ 入口状态的入口处理 ■■
        case STATE_ENTRY:
    //          POWER_ON();

            // 升级数据包大小
            upAppDataSize = IAP_FlashReadWord(PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS + 4);

            // 擦除APP1所有页
            IAP_EraseAPP1Area();
            break;

        // 跳转APP
        case STATE_JUMP:

            // 开启定时跳入APP
            TIMER_AddTask(TIMER_ID_JUMP,
                          200,
                          IAP_JumpToAppFun,
                          0,
                          1,
                          ACTION_MODE_ADD_TO_QUEUE);

            break;

        default:
            break;
    }
}

// 每个状态下的过程处理
void STATE_Process(void)
{
    uint32 buff[2] = {0};
    uint8  AppBuff[IAP_FLASH_APP_READ_UPDATA_SIZE] = {0};
    static uint16 i = 0;

    switch (stateCB.state)
    {
        // ■■ 空状态 ■■
        case STATE_NULL:
            break;

        // ■■ 入口状态过程处理 ■■
        case STATE_ENTRY:

            if (i > ((upAppDataSize * 128 / IAP_FLASH_APP_READ_UPDATA_SIZE) + 1))
            {

                buff[0] = 0xFFFFFFFF;
                
                // 清除APP升级标志
                IAP_FlashWriteWordArrayWithErase(PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS, buff, 1);
                
                // 写入升级成功标志位
                buff[0] = 0xCC000000;

                // APP升级成功标志
                IAP_FlashWriteWordArrayWithErase(PARAM_UPDATA_SUCCESS_FLAG_ADDEESS, buff, 1);     
                
                STATE_EnterState(STATE_JUMP);
                            
            }

            // 读取APP2中的数据
            IAP_FlashReadByteArray(IAP_FLASH_APP2_ADDR + i * IAP_FLASH_APP_READ_UPDATA_SIZE, AppBuff,  IAP_FLASH_APP_READ_UPDATA_SIZE);

            // 将APP2中的数据写入APP1
            IAP_WriteAppBin(IAP_FLASH_APP1_ADDR + i * IAP_FLASH_APP_READ_UPDATA_SIZE, AppBuff, IAP_FLASH_APP_READ_UPDATA_SIZE);

            i++;
            break;

        // 跳转APP
        case STATE_JUMP:
            break;

        default:
            break;
    }
}








///*********************************************************************
//* 函 数 名: LED_RidingStateShow
//* 函数入参: refreshAll
//* 函数出参: 无
//* 返 回 值: void
//* 功能描述:   蓝牙升级状态
//***********
//* 修改历史:
//*   1.修改作者: ZJ
//*     修改日期: 2017年04月20日
//*     修改描述: 新函数
//**********************************************************************/
//void  STATE_UpradeProgress(uint32 Progress)
//{

//  stateCB.ledPrePro =  stateCB.ledNewPro;
//  stateCB.ledNewPro =  Progress;

//  switch(Progress)
//  {
//      case UPDATE_PROGRESS_0:
//
//          if (stateCB.ledNewPro != stateCB.ledPrePro)
//          {
//              LED_Set(LED_NAME_ORANGE_1, 500, 50, LED_PWMBUFF, 0, 0, 0, -1);
//          }
//          break;

//      case UPDATE_PROGRESS_20:
//          if (stateCB.ledNewPro != stateCB.ledPrePro)
//          {
//              LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF, 0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_2, 500,  50, LED_PWMBUFF,  0, 0, 0, -1);
//          }
//          break;
//
//      case UPDATE_PROGRESS_40:
//
//          if (stateCB.ledNewPro != stateCB.ledPrePro)
//          {
//              LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_2, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_3, 500,  50,  LED_PWMBUFF,  0, 0, 0, -1);
//          }
//          break;
//
//      case UPDATE_PROGRESS_60:
//
//          if (stateCB.ledNewPro != stateCB.ledPrePro)
//          {
//              LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_2, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_3, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_4, 500, 50,  LED_PWMBUFF,  0, 0, 0, -1);

//          }
//          break;

//      case UPDATE_PROGRESS_80:
//
//          if (stateCB.ledNewPro != stateCB.ledPrePro)
//          {
//              LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_2, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_3, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_4, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_5, 500, 50, LED_PWMBUFF,  0, 0, 0, -1);
//          }
//          break;
//
//      case UPDATE_PROGRESS_100:
//
//          if (stateCB.ledNewPro != stateCB.ledPrePro)
//          {
//              LED_Set(LED_NAME_ORANGE_1, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_2, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_3, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_4, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//              LED_Set(LED_NAME_ORANGE_5, 1000, 100, LED_PWMBUFF,  0, 0, 0, -1);
//          }
//          break;
//
//      default:
//
//          break;

//  }

//}
///*********************************************************************
//* 函 数 名: LED_RidingStateShow
//* 函数入参: refreshAll
//* 函数出参: 无
//* 返 回 值: void
//* 功能描述:   蓝牙升级状态
//***********
//* 修改历史:
//*   1.修改作者: ZJ
//*     修改日期: 2017年04月20日
//*     修改描述: 新函数
//**********************************************************************/
//void  STATE_UpradeStateShow(uint32 param)
//{

//  if (param < 10)
//  {
//      STATE_UpradeProgress(UPDATE_PROGRESS_0);
//  }
//  else if (param == 20)
//  {
//      STATE_UpradeProgress(UPDATE_PROGRESS_20);

//  }
//  else if (param == 30)
//  {
//      STATE_UpradeProgress(UPDATE_PROGRESS_40);

//  }
//  else if (param == 40)
//  {
//      STATE_UpradeProgress(UPDATE_PROGRESS_60);

//  }
//  else if (param == 50)
//  {
//      STATE_UpradeProgress(UPDATE_PROGRESS_80);
//  }
//  else if (param== 55)
//  {
//      STATE_UpradeProgress(UPDATE_PROGRESS_100);

//  }

//}

