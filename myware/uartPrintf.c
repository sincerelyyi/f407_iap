/** **************************************************************************************
 * @file
 * @brief           用串口输出printf
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
#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "uartPrintf.h"
#include "usbd_def.h"
extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
extern USBD_HandleTypeDef hUsbDeviceFS;
void debug_printf(char * fmt, ...)
{
#if _USE_DEBUG_PRINTF_
    char str[256];
    uint8_t len;
    va_list args;

    memset((char *)str, 0, sizeof(char) * 256);
    va_start(args, fmt);
    vsprintf((char *)str, fmt, args);
    va_end(args);

    len = strlen((char *)str);
    HAL_UART_Transmit(&huart1, (uint8_t *)str,len,1000);
    //HAL_UART_Transmit_DMA(&huart1, (uint8_t *)str,len);
    //HAL_UART_Transmit(&huart3, (uint8_t *)str,len,1000);
    //HAL_UART_Transmit_DMA(&huart3, (uint8_t *)str,len);
//    if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
//    {
//        CDC_Transmit_FS((uint8_t *)str, len);
//    }
#endif
}
