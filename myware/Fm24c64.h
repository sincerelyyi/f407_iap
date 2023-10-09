/** **************************************************************************************
 * @file
 * @brief         铁电体相关部分
 * @note          用I2C读写铁电体
 * @author        zhangjiayi
 * @date          2020-09-25 12:03:23
 * @version       v0.1
 * @copyright     Copyright (c) 2020-2050  zhangjiayi
 * @par           LastEdit
 * @LastEditTime    2021-10-20 10:29:16
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/Fm24c64.h
 *****************************************************************************************/
#ifndef _FM24C64_H
#define _FM24C64_H
#include "main.h"
#include "i2c.h"
#include "version.h"
#define I2C_SPEECH_100K 100000	///< I2C 100K Hz通信频率
#define I2C_SPEECH_400K 400000	///<  I2C 400K Hz通信频率
#define FM24C64_ADDR 0xa0		///< 铁电体的设备地址
#define FM24C64_SIZE 0x2000		///< 铁电体的容量



/** **************************************************************************************
 * @brief 铁电体存储的数据结构
 * @note
 * -# 所有要保存的数据都在这个结构体中
 * -# 一般只用定义一个此结构体的变量在`
 *****************************************************************************************/
typedef struct fmDate
{
    uint16_t coin_num[4];   ///< coin的币数
    uint16_t counter[4];    ///< 未跳码表数
    uint32_t have_data;		///< 标志是否有数据
    uint32_t Data;          ///< 测试用数据
} fmDataTypeDef;

extern fmDataTypeDef FM; //铁电存储得变量
extern fmDataTypeDef FM_old; //旧的铁电存储得变量
extern HAL_StatusTypeDef fm24c64_save(void);
extern HAL_StatusTypeDef fm24c64_load(void);
extern void data_init(void);
#endif
