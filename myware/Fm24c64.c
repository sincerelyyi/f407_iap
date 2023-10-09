/** **************************************************************************************
 * @file
 * @brief         铁电体相关部分
 * @note          用I2C读写铁电体
 * @author        zhangjiayi
 * @date          2020-09-25 12:03:23
 * @version       v0.1
 * @copyright     Copyright (c) 2020-2050  zhangjiayi
 * @par           LastEdit
 * @LastEditTime    2021-05-18 18:10:17
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/Fm24c64.c
 *****************************************************************************************/
#include "Fm24c64.h"

fmDataTypeDef FM; ///< 所有存储的数据都在此变量中
fmDataTypeDef FM_old; ///< 旧存储的数据都在此变量中


/*************************************************************
 * @brief write all data to FE
 * @note
 * 		-# 和RTC芯片共用IC总线，注意总线的争夺
 * 		-# 避免FE读写的竞争
 * @param {type}
 * @return {type}
 * @retval
 **************************************************************/
HAL_StatusTypeDef fm24c64_save()
{
    uint16_t size;
    size = sizeof(FM);
    return HAL_I2C_Mem_Write_DMA(&hi2c1, FM24C64_ADDR, 0, I2C_MEMADD_SIZE_16BIT, (uint8_t *)&FM, size);
}
/*************************************************************
 * @brief read all data from FE to ram
 * @note
 * 		-# 和RTC芯片共用IC总线，注意总线的争夺
 * 		-# 避免FE读写的竞争
 * @param void
 * @return HAL_StatusTypeDef    HAL的状态
 * 		 - HAL_OK
 * 		 - HAL_ERROR
 * 		 - HAL_BUSY
 * 		 - HAL_TIMEOUT
 **************************************************************/
HAL_StatusTypeDef fm24c64_load(void)
{
    uint16_t size;
    size = sizeof(FM);
    if (size > FM24C64_SIZE) //检查是否超过铁电容量
    {
        return HAL_ERROR;
    }
    else
    {
        return HAL_I2C_Mem_Read_DMA(&hi2c1, FM24C64_ADDR, 0, I2C_MEMADD_SIZE_16BIT, (uint8_t *)&FM, size);
    }
}
