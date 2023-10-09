/** **************************************************************************************
 * @file
 * @brief			灯光闪动的实现
 * @note            通过扫描方式
 * @author          zhangjiayi
 * @date            2023-04-11 09:04:47
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-04-11 13:32:10
 * @LastEditors     zhangjiayi
 * @FilePath        /myware/light.c
 *****************************************************************************************/
#ifndef _LIGHT_
#define _LIGHT_
#include "main.h"
#include "MBI6024.h"
/** **************************************************************************************
 * @brief PWM组灯光
 *****************************************************************************************/
typedef struct _pwm
{
    uint8_t isinit;
    uint8_t command;				///< 控制命令
    uint16_t* pwm_data;				///< pwm数据
    uint16_t count_100ms[MBI6024IN_NUM * 12];           ///< 暂存100ms倒计时
    uint8_t channel_state[MBI6024IN_NUM * 12];			///< 每个通道样式值
    uint8_t all_state;				///< 整体样式值
    uint8_t old_all_state;          ///< 上次的整体样式值
    uint8_t all_state_num;          ///< 整体样式值进度序号
    uint32_t old_tick;              ///< 暂存上次tick，tick/TICK
    uint8_t channel_num;		    ///< 通道数
} PwmTypeDef;
/** **************************************************************************************
 * @brief Circle组灯光
 *****************************************************************************************/
typedef struct _circle
{
    uint8_t isinit;
    uint8_t command;				///< 控制命令
    uint16_t* pwm_data;				///< pwm数据
    uint16_t pwm_tmp[78];           ///< 暂存pwm数据
    uint8_t channel_state[MBI6024OUT_NUM * 12];			///< 每个通道样式值
    uint8_t all_state;				///< 整体样式值
    uint8_t old_all_state;          ///< 上次的整体样式值
    uint8_t all_state_num;          ///< 整体样式值进度序号
    uint32_t old_tick;              ///< 暂存上次tick，tick/TICK
    uint8_t channel_num;		    ///< 通道数
} CircleTypeDef;


extern CircleTypeDef P1circle,P2circle,P3circle,P4circle;
extern PwmTypeDef Pwm;
extern CircleTypeDef *Circle[4];
extern void light_pattern(void);
#endif
