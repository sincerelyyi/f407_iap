/** **************************************************************************************
 * @file
 * @brief           fifo
 * @note
 * @author          zhangjiayi
 * @date            2022-11-23 12:03:23
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2022-11-23 10:29:16
 * @LastEditors     zhangjiayi
 * @FilePath
 *****************************************************************************************/

#ifndef _FIFO_
#define _FIFO_
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"

typedef enum
{
    size_8 = 1,
    size_16 = 2,
    size_32 = 4,
    size_64 = 8
} data_size;

typedef struct
{

    uint16_t length;                ///< fifo max length
    uint16_t p_in;                 ///< point to first data
    uint16_t p_out;                ///< point to the last date
    uint16_t num;                  ///< now fifo data numbers. if full size = length
    void*  buff;                   ///< point to fifo buff;
    data_size size;                ///< data size.1,int8_t,2,int16,4,int32
} fifo_t;

extern fifo_t* _fifo_creat(uint16_t length,data_size size);
#define fifo_creat(length,type) (fifo_t *)_fifo_creat(length, (data_size)sizeof(type))
extern int fifo_delete(fifo_t *fifo);
extern int fifo_in(fifo_t *fifo,void *buff,uint16_t len);
extern int fifo_out(fifo_t *fifo,void *buff,uint16_t len);

#endif
