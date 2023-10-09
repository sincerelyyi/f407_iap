/** **************************************************************************************
 * @file
 * @brief           TK 主流程
 * @note
 * @author          zhangjiayi
 * @date            2023-04-01 12:23:23
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-04-01 12:23:23
 * @LastEditors     zhangjiayi
 * @FilePath        /test/myware/mainFunction.c
 *****************************************************************************************/
#ifndef _BOARDTEST_H
#define _BOARDTEST_H
#include "main.h"



extern uint8_t Game_state;   ///< 表明游戏所处状态，0，正常游戏模式，1，管理员模式

extern void main_free(void);

#endif
