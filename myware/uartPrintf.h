/** **************************************************************************************
 * @file
 * @brief           uart printf
 * @note            how it work
 * @author          zhangjiayi
 * @date            2020-12-24 18:01:15
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2020-12-24 18:31:26
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrainext/myware/uartPrintf.c
 *****************************************************************************************/
#ifndef _UARTPRINTF_
#define _UARTPRINTF_
#define _USE_DEBUG_PRINTF_ 1 ///< 1:use debug_printf. 0: no use
extern void debug_printf(char *fmt, ...);
#endif

