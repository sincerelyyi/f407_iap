/** **************************************************************************************
 * @file
 * @brief           码表驱动程序
 * @note            通过扫描方式形成驱动脉冲
 * @author          zhangjiayi
 * @date            2020-10-09 09:04:47
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2021-04-07 13:32:10
 * @LastEditors     zhangjiayi
 * @FilePath        /myware/counter.c
 *****************************************************************************************/
#include "counter.h"
#include "Fm24c64.h"
#include "gpio.h"
#include "mainFunction.h"
#define _SET_TIME 50    ///< 码表通电时间（ms）
#define _RESET_TIME 150 ///< 码表至少不通电时间，少于一定时间，码表不会跳动。
CounterTypeDef P1Counter, P2Counter, P3Counter, P4Counter;
uint16_t Tmp_counter[4]= {0};  ///< 管理员系统下的未跳码表数
/** **************************************************************************************
 * @brief 码表初始化
 * @note  使用counter_scan()前，必须先初始化
 * @param CounterTypeDef *counter:P1Counter, P2Counter, P3Counter, P4Counter
 *****************************************************************************************/
void counter_init(volatile CounterTypeDef *counter)
{
    //P1ConterL init
    if (counter == &P1Counter)
    {
        counter->outputPolarity = 0;
        counter->outPin = OC1_Pin;
        counter->outPort = OC1_GPIO_Port;
        counter->notCount = &FM.counter[0];
        counter->tmpCount = &Tmp_counter[0];
        counter->isInit = 1;
    }
    //P2Counter init
    if (counter == &P2Counter)
    {
        counter->outputPolarity = 0;
        counter->outPin = OC2_Pin;
        counter->outPort = OC2_GPIO_Port;
        counter->notCount = &FM.counter[1];
        counter->tmpCount = &Tmp_counter[1];
        counter->isInit = 1;
    }
    //P3Counter init
    if (counter == &P3Counter)
    {
        counter->outputPolarity = 0;
        counter->outPin = OC3_Pin;
        counter->outPort = OC3_GPIO_Port;
        counter->notCount = &FM.counter[2];
        counter->tmpCount = &Tmp_counter[2];
        counter->isInit = 1;
    }
    //P3Counter init
    if (counter == &P4Counter)
    {
        counter->outputPolarity = 0;
        counter->outPin = OC4_Pin;
        counter->outPort = OC4_GPIO_Port;
        counter->notCount = &FM.counter[3];
        counter->tmpCount = &Tmp_counter[3];
        counter->isInit = 1;
    }
    if (counter->outputPolarity) //码表复位
    {
        HAL_GPIO_WritePin(counter->outPort,counter->outPin,GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(counter->outPort,counter->outPin,GPIO_PIN_RESET);
    }
}
/** **************************************************************************************
 * @brief 码表扫描程序
 * @note  使用此函数前须先初始化，即使用counter_init(void)
 * @param CounterTypeDef *counter:P1Counter, P2Counter, P3Counter, P4Counter
 *****************************************************************************************/
static void _counter_scan(volatile CounterTypeDef *counter)
{
    volatile uint16_t *tmp_count;
    if(Game_state == 0)
    {
        tmp_count = counter->notCount;
    }
    else
    {
        tmp_count = counter->tmpCount;
    }
    if (counter->isInit == 0)
    {
        counter_init(counter);
    }
    if (HAL_GetTick() - counter->time > _SET_TIME + _RESET_TIME)
    {
        if (*tmp_count)
        {
            counter->time = HAL_GetTick();
            (*tmp_count)--;
            if (counter->outputPolarity) //码表置位
            {
                HAL_GPIO_WritePin(counter->outPort,counter->outPin,GPIO_PIN_RESET);
            }
            else
            {
                HAL_GPIO_WritePin(counter->outPort,counter->outPin,GPIO_PIN_SET);
            }
        }
    }
    if (HAL_GetTick() - counter->time > _SET_TIME)
    {
        if (counter->outputPolarity) //码表复位
        {
            HAL_GPIO_WritePin(counter->outPort,counter->outPin,GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(counter->outPort,counter->outPin,GPIO_PIN_RESET);
        }
    }
}
void counter_scan(void)
{
    _counter_scan(&P1Counter);
    _counter_scan(&P2Counter);
    _counter_scan(&P3Counter);
    _counter_scan(&P4Counter);
}
