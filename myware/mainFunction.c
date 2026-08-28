/** ****************************************************************************
* @file             myware/mainFunction.c
* @brief            TK 主流程
* @note             
* @author           zhangjiayi
* @date             2023-04-01 12:23:23
* @version          V1
* @copyright        Copyright (c) 2020-2050 zhangjiayi
* @par              LastEdit
* @LastEditTime     2026-08-28 12:37:07
* @LastEditors      jiayi
*******************************************************************************/
#include <stdio.h>
#include <stm32f407xx.h>
#include "usart.h"
#include "usbd_def.h"
#include "version.h"
#include "packet_handle.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

void main_free(void)
{
    static uint8_t main_init = 0;
    static uint32_t Main_count;
    static uint32_t new_tick = 0;
    static uint32_t old_tick = 0;
    static uint32_t enter_count = 0;
    Main_count++;
    if(main_init == 0)
    {
        while(HAL_UART_Receive_DMA(&huart1,Rs232_receive_buff,RS232_RECEIVE_SIZE) == HAL_OK);
        main_init = 1;
    }
    HAL_UART_Receive_DMA(&huart1,Rs232_receive_buff,RS232_RECEIVE_SIZE);
    new_tick = HAL_GetTick();
    if(old_tick != new_tick)
    {
        enter_count++;
        old_tick = new_tick;
        if(enter_count%100==0)
        {
            HAL_GPIO_TogglePin(LED_NORMAL_GPIO_Port, LED_NORMAL_Pin);
        }
        switch(enter_count%10)
        {
        case 0:
            usb_pickup_packet();
            rs232_pickup_packet();
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
        }

    }
}
