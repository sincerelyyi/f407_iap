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
#include "MBI6024.h"
#include "Fm24c64.h"
#include "tim.h"
#include <stdio.h>
#include <stm32f407xx.h>
#include "spi.h"
#include "uartPrintf.h"
#include "lkt4101.h"
#include "74HC165.h"
#include "usbd_def.h"
#include "version.h"
#include "packet_handle.h"
#include "coin.h"
#include "counter.h"
#include "light.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

uint8_t Game_state = 0;                 ///< 表明游戏所处状态，0，正常游戏模式，1，管理员模式
uint8_t Game_state_old = 0;             ///< 表明游戏所处状态，0，正常游戏模式，1，管理员模式
void mbi6024_in_init(void)
{
    //不知为什么正向时只能用10bit，反向时只能用16bit
    HAL_GPIO_WritePin(POL_6024_GPIO_Port,POL_6024_Pin, GPIO_PIN_RESET);//reset为正向，set为反向
    Mbi6024_in.hspi = &hspi1;
    Mbi6024_in.ics  = MBI6024IN_NUM;                       //有多少个IC
    Mbi6024_in.send_buff = MbiIn_sendBuff;                //最终要发送到MIB6024的数据包。外部程序不用理会
    Mbi6024_in.gray_scale_data = MbiIn_UserGrayBuff;    //用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
    Mbi6024_in.dot_correct_data =MbiIn_UserCorrectBuff;   //用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。
    Mbi6024_in.clk_port = SCK_6024_GPIO_Port;             //所连接的clk port
    Mbi6024_in.clk_pin = SCK_6024_Pin;                    //所连接的clk pin
    Mbi6024_in.dat_port = DAT_6024_GPIO_Port;             //所连接的clk port
    Mbi6024_in.dat_pin = DAT_6024_Pin;                    //所连接的clk pin
    mbi6024_10bit_configuration(&Mbi6024_in);
}
//这个怎样都行。可能时3.3v和5v的区别
void mbi6024_out_init(void)
{
    Mbi6024_out.hspi = &hspi2;
    Mbi6024_out.ics  = MBI6024OUT_NUM;                                          //有多少个IC
    Mbi6024_out.send_buff = MbiOut_sendBuff;                            //最终要发送到MIB6024的数据包。外部程序不用理会
    Mbi6024_out.gray_scale_data = MbiOut_UserGrayBuff;          //用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
    Mbi6024_out.dot_correct_data =MbiOut_UserCorrectBuff;   //用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。
    Mbi6024_out.clk_port = SPI_CLK_GPIO_Port;                           //所连接的clk port
    Mbi6024_out.clk_pin = SPI_CLK_Pin;                                          //所连接的clk pin
    Mbi6024_out.dat_port = SPI_DAT_GPIO_Port;                           //所连接的clk port
    Mbi6024_out.dat_pin = SPI_DAT_Pin;                                          //所连接的clk pin
    mbi6024_10bit_configuration(&Mbi6024_out);
}
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
        read_165();
        while(fm24c64_load()!=HAL_OK)
        {
            HAL_Delay(10);
        }
        HAL_Delay(10);
        if(FM.have_data != 0x12345678)
        {
            memset(&FM, 0, sizeof(FM));
            FM.have_data = 0x12345678;
        }
        fm24c64_save();
        HAL_Delay(10);
        fm24c64_load();
        memset(MbiIn_UserGrayBuff,0xff,sizeof(MbiIn_UserGrayBuff));
        memset(MbiOut_UserGrayBuff,0xff,sizeof(MbiOut_UserGrayBuff));
        mbi6024_in_init();
        mbi6024_out_init();
        while(HAL_UART_Receive_DMA(&huart1,Rs232_receive_buff,RS232_RECEIVE_SIZE) == HAL_OK);
        main_init = 1;
    }
    HAL_UART_Receive_DMA(&huart1,Rs232_receive_buff,RS232_RECEIVE_SIZE);
    new_tick = HAL_GetTick();
    if(old_tick != new_tick)
    {
        enter_count++;
        old_tick = new_tick;
        if(Game_state != Game_state_old)
        {
            if(Game_state == 0)
            {
                memset(Tmp_counter,0, sizeof(Tmp_counter));
                memset(Tmp_coin, 0, sizeof(Tmp_coin));
            }
            Game_state_old = Game_state;
        }
        lkt_identify(1);
        lkt_led(100);;
        switch(enter_count%10)
        {
        case 0:
            light_pattern();
            mbi6024_10bit_gray_scale(&Mbi6024_in);
            break;
        case 1:
            coin_scan();
            counter_scan();
            read_165();
            break;
        case 2:
            if(memcmp(&FM,&FM_old,sizeof(FM)) !=0)
            {
                fm24c64_save();
            }
            break;
        case 3:
            mbi6024_10bit_gray_scale(&Mbi6024_out);
            usb_pickup_packet();
            rs232_pickup_packet();
            break;
        default:
            break;
        }

    }
}
