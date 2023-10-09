/** **************************************************************************************
 * @file
 * @brief           74HC165串行输入
 * @note            通过spi控制
 * @author          zhangjiayi
 * @date            2023-03-24 12:03:23
 * @version         v0.2 通过__USE_SPI__ 选择spi方式还是gpio方式
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-09-18 12:53:17
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/MBI6024.c
 *****************************************************************************************/
#ifndef _74HC65_H
#define _74HC65_H
#include "main.h"
#include "spi.h"
#define NUM_165 6
#define __USE_SPI__ 0
#define SPI_165 hspi3
extern uint8_t in165_buff[NUM_165];
extern uint8_t read_165(void);
#endif
