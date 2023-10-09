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
#include "coin.h"
#include "stdio.h"
#include "string.h"
#include "main.h"
#include "Fm24c64.h"
#include "74HC165.h"
#include "packet_handle.h"
#include "command.h"
#include "mainFunction.h"
#define MAX_WIDTH  500
#define MIN_WIDTH  10

cointype Coin[4] = {0};
uint16_t Tmp_coin[4] = {0};   ///< 管理员下的币数

/** **************************************************************************************
 * @brief 投币器初始化
 * @note
 * @param void
 * @return char *
 * @retval  返回版本信息字符串地址
 *****************************************************************************************/

void coin_init(cointype *coin)
{
    if(coin == &Coin[0])
    {
        coin->byte = 1;
        coin->pin  = 5;
        coin->oldtime = 0;
        coin->polarity = 0;
        coin->oldstate = coin->polarity;
        coin->num = &FM.coin_num[0];
        coin->tmp_num = &Tmp_coin[0];
        coin->init = 1;
    }

    else if(coin== &Coin[1])
    {

        coin->byte = 1;
        coin->pin  = 6;
        coin->oldtime = 0;
        coin->polarity = 0;
        coin->oldstate = coin->polarity;
        coin->num = &FM.coin_num[1];
        coin->tmp_num = &Tmp_coin[1];
        coin->init = 1;
    }
    else if(coin == &Coin[2])
    {
        coin->byte = 1;
        coin->pin  = 7;
        coin->oldtime = 0;
        coin->polarity = 0;
        coin->oldstate = coin->polarity;
        coin->num = &FM.coin_num[2];
        coin->tmp_num = &Tmp_coin[2];
        coin->init = 1;
    }
    else if(coin == &Coin[3])
    {
        coin->byte = 0;
        coin->pin  = 0;
        coin->oldtime = 0;
        coin->polarity = 0;
        coin->oldstate = coin->polarity;
        coin->num = &FM.coin_num[3];
        coin->tmp_num = &Tmp_coin[3];
        coin->init = 1;
    }
}
/** **************************************************************************************
 * @brief 投币器输入脚扫描
 * @note
 * @param void
 * @return char *
 * @retval  返回版本信息字符串地址
 *****************************************************************************************/
void coin_scan(void)
{
    uint32_t now;
    uint8_t state;

    now = HAL_GetTick();
    for(uint8_t i = 0; i< 4; i++)
    {
        if(Coin[i].init ==0)  //如果没有初始化，就初始化
        {
            coin_init(&Coin[i]);
        }

        (in165_buff[Coin[i].byte] & 0x01<<Coin[i].pin)? (state=1):(state = 0);

        if(state !=Coin[i].oldstate )  //有跳动
        {
            if(state != Coin[i].polarity)  //第一边沿
            {
                ;
            }
            else                                      //第二边沿
            {
                if(now - Coin[i].oldtime <= MAX_WIDTH && now - Coin[i].oldtime>=MIN_WIDTH)  //有效脉宽
                {
                    if(Game_state == 0)						///< 正常游戏状态
                    {
                        *Coin[i].num = *Coin[i].num +1;
                        (FM.counter[i]) ++;                  //码表加一
                        active_bin(slave_out_coins,(uint8_t *)(FM.coin_num),sizeof(FM.coin_num),Comunucate_channel);
                    }
                    else if(Game_state == 1)				///< 管理员模式
                    {
                        *Coin[i].tmp_num = *Coin[i].tmp_num +1;
                        active_bin(slave_out_coins,(uint8_t *)(Tmp_coin),sizeof(Tmp_coin),Comunucate_channel);
                    }
                }
            }
            Coin[i].oldstate = state;
            Coin[i].oldtime = now;
        }
        else
        {
            if(now - Coin[i].oldtime > 2000)       //电平2秒没有跳动，判定为常态电平
            {
                Coin[i].polarity = state;
            }
        }
    }
}

/** **************************************************************************************
 * @brief 生成固件版本信息
 * @note
 * @param void
 * @return char *
 * @retval  返回版本信息字符串地址
 *****************************************************************************************/


