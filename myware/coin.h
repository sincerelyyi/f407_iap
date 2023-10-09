/** **************************************************************************************
 * @file
 * @brief           投币器相关
 * @note
 * @author          zhangjiayi
 * @date            2023-01-03 06:35:39
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-01-03 21:36:01
 * @LastEditors     zhangjiayi
 * @FilePath
 *****************************************************************************************/
#ifndef _COIN_
#define _COIN_
#include "main.h"
typedef struct _coin
{
    uint8_t init;                   //投币器
    uint8_t byte;                   //投币器输出脚所在的byte
    uint8_t pin;                    //投币器输出脚所在的pin
    uint8_t polarity;               //投币器常态是高电平为0，低电平为1（key的采集是反向的）
    uint32_t oldtime;               //采集上次跳动的时间
    uint8_t oldstate;               //上次跳动状态，只能0，1
    uint16_t *num;                  //现有币数
    uint16_t *tmp_num;				//测试用的币数
} cointype;

extern void coin_scan(void);
extern cointype Coin[4];
extern uint16_t Tmp_coin[4];   ///< 管理员下的币数
#endif
