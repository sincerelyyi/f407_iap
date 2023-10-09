/** **************************************************************************************
 * @file
 * @brief           码表驱动程序
 * @note            通过扫描方式形成驱动脉冲
 * @author          zhangjiayi
 * @date            2020-10-09 09:04:47
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2021-04-07 13:32:43
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/counter.h
 *****************************************************************************************/
#ifndef _COUNTER_H
#define _COUNTER_H
#include "main.h"
#include "gpio.h"
/** **************************************************************************************
 *
 * @brief 码表结构体
 *****************************************************************************************/
typedef struct counter
{
    volatile uint32_t time;				///< 每个计数脉冲开始时间
    volatile uint16_t *notCount;		///< 将要计数的数字
    volatile uint16_t *tmpCount;		///< 测试用的计数的数字
    uint16_t outPin;					///< 驱动脚
    GPIO_TypeDef* outPort;				///< 驱动口
    volatile uint8_t outputPolarity;	///< 码表控制电平
    volatile uint8_t isInit;			///< 是否初始化了
} CounterTypeDef;

extern uint16_t Tmp_counter[4];  ///< 管理员系统下的未跳码表数
extern CounterTypeDef P1Counter, P2Counter, P3Counter, P4Counter;
extern void counter_scan(void);
#endif
