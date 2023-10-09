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
#include "fifo.h"
/** **************************************************************************************
 * @brief creat a new fifo
 * @note
 * @param length: the length of fifo(bytes)
 * @return fifo_t: a address point to a fifo;if 0 false
 *****************************************************************************************/
fifo_t* _fifo_creat(uint16_t length,data_size size)
{
    fifo_t* fifo =  malloc(sizeof(fifo_t));
    if(fifo == NULL)
        return 0;
    fifo->length = length;

    fifo->p_in = 0;
    fifo->p_out = 0;
    fifo->num =0;
    fifo->size = size;
    fifo->buff = malloc(length*size);
    if(fifo->buff == NULL)
        return 0;
    return fifo;
}

/** **************************************************************************************
 * @brief delete a fifo
 * @note
 * @param length: the length of fifo(bytes)
 * @return fifo_t: a address point to a fifo;if 0 false
 *****************************************************************************************/
int fifo_delete(fifo_t *fifo)
{

    if(fifo == NULL)
        return 0;
    if(fifo->buff == NULL)
        return 0;
    else
    {
        free(fifo->buff);
        fifo->buff = NULL;
    }
    free(fifo);
    return 1;
}

/** **************************************************************************************
 * @brief put data into fifo
 * @note
   @param fifo: fifo address
 * @param buff: the address of data will be putted int0 fifo
 * @param len: the length of data will be putted int0 fifo
 * @return 1,ok; 0,false.-1: fifo not exit;
 *****************************************************************************************/
int fifo_in(fifo_t *fifo,void *buff,uint16_t len)
{

    if(fifo == NULL)
        return -1;
    if(len > fifo->length - fifo->num)
        return 0;
    for(uint16_t i = 0; i<len; i++)
    {
        switch(fifo->size)
        {
        case size_8:
            ((int8_t *)fifo->buff)[fifo->p_in] = ((int8_t *)buff)[i];
            break;
        case size_16:
            ((int16_t *)fifo->buff)[fifo->p_in] = ((int16_t *)buff)[i];
            break;
        case size_32:
            ((int32_t *)fifo->buff)[fifo->p_in] = ((int32_t *)buff)[i];
            break;
        case size_64:
            ((int64_t *)fifo->buff)[fifo->p_in] = ((int64_t *)buff)[i];
            break;
        default:
            ((int8_t *)fifo->buff)[fifo->p_in] = ((int8_t *)buff)[i];
        }
        fifo->p_in = (fifo->p_in +1) %fifo->length;
    }
    fifo->num += len;
    return 1;
}

/** **************************************************************************************
 * @brief read data from fifo
 * @note
   @param fifo: fifo address
 * @param buff: the address will store the data read from fifo
 * @param len: the length of data will be read
 * @return 1,ok; 0,false,-1: fifo not exit;
 *****************************************************************************************/
int fifo_out(fifo_t *fifo,void *buff,uint16_t len)
{

    if(fifo == NULL)
        return -1;
    if(fifo->num < len)
        return 0;
    for(uint16_t i = 0; i < len; i++)
    {
        switch(fifo->size)
        {
        case size_8:
            ((int8_t *)buff)[i] = ((int8_t *)fifo->buff)[fifo->p_out];
            break;
        case size_16:
            ((int16_t *)buff)[i] = ((int16_t *)fifo->buff)[fifo->p_out];
            break;
        case size_32:
            ((int32_t *)buff)[i] = ((int32_t *)fifo->buff)[fifo->p_out];
            break;
        case size_64:
            ((int64_t *)buff)[i] = ((int64_t *)fifo->buff)[fifo->p_out];
            break;
        default:
            ((int8_t *)buff)[i] = ((int8_t *)fifo->buff)[fifo->p_out];
        }
        fifo->p_out =  (fifo->p_out + 1) % fifo->length;
    }
    fifo->num -=len;
    return 1;
}
