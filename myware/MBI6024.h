/** **************************************************************************************
 * @file
 * @brief           PWM灯光控制
 * @note			增加了ABC版本的区别代码
                    使用前，先定义
                    1，type_mbi6024 Mbi6024_in
                    2，uint8_t Mbi6024_send_buff[6+MBI6024IN_NUM*24];        //最终要发送到MIB6024的数据包。外部程序不用理会
                    3，uint16_t Mbi_gray_scale_data[MBI6024IN_NUM * 12];     //用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
                    4，uint8_t Mbi_dot_correct_data[MBI6024IN_NUM * 12];     //用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。
                    5, 给各个mbi6024实例赋值
 * @author          zhangjiayi
 * @date            2023-04-04 12:03:23
 * @version         v1.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-04-04 12:53:17
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/MBI6024.h
 *****************************************************************************************/
#ifndef _MBI6024_H
#define _MBI6024_H
#include "main.h"
#include <stdbool.h>
#include "gpio.h"
#define HARDWARE_C   //硬件版本号,有A、B、C
#define MBI6024IN_NUM 2
#define MBI6024OUT_NUM 28
#define _USE_SPI_DMA_  ///< 使用spi dma,注释掉使用GPIO,需要自己初始化SPI DMA 或GPIO
typedef struct mbi6024_t
{
    SPI_HandleTypeDef *hspi;
    uint16_t ics;                  //有多少个IC
    uint8_t* send_buff;            //最终要发送到MIB6024的数据包。外部程序不用理会
    uint16_t* gray_scale_data;     //用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
    uint8_t* dot_correct_data;     //用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。
    GPIO_TypeDef * clk_port;       //所连接的clk port
    uint16_t clk_pin;              //所连接的clk pin
    GPIO_TypeDef * dat_port;       //所连接的clk port
    uint16_t dat_pin;              //所连接的clk pin
} type_mbi6024;



#ifdef _USE_SPI_DMA_
#include "spi.h"       //使用spi dma
#else                  //用GPIO模拟spi时，指定CLK和DAT对应的GPIO
#define CLK_SET ( HAL_GPIO_WritePin(mbi6024->clk_port,mbi6024->clk_pin,GPIO_PIN_SET))
#define CLK_CLR ( HAL_GPIO_WritePin(mbi6024->clk_port,mbi6024->clk_pin,GPIO_PIN_RESET))
#define DAT_SET ( HAL_GPIO_WritePin(mbi6024->dat_port,mbi6024->dat_pin,GPIO_PIN_SET))
#define DAT_CLR ( HAL_GPIO_WritePin(mbi6024->dat_port,mbi6024->dat_pin,GPIO_PIN_RESET))
#endif


//config register1 data
#define GCLK_frequency ((uint16_t)2)
///< 0: GCLK=frequecy of internal oscillator divided by 8,i.e. 3MHz(typ.).
///< 1: GCLK=frequecy of internal oscillator divided by 4,i.e. 6MHz(typ.).
///< 2: GCLK=frequecy of internal oscillator divided by 2,i.e. 12MHz(typ.).default
///< 3: GCLK=frequecy of internal oscillator divided by 1,i.e. 24MHz(typ.).
#define GCLK_source ((uint16_t)3)  //B,C版无效
///< 0: Reserved.
///< 1: Reserved.
///< 3: E-GCK pin.
///< 4: Internal oscillator. default.
#define DOT_correction_mode ((uint16_t)1)  //A版无效
///< 0: Disable default
///< 1: Enable
#define PWN_counter_reset ((uint16_t)1)
///< 0: DO not reset PWM after programming configuration register.
///< 1: Reset PWM after programming configuration register. default.
#define PWM_date_synchronization ((uint16_t)1) ///<	0:Manual synchronization
///< 1: Automatic synchronization
#define Phase_inversed ((uint16_t)3)
///< 不能修改
#define parity_check ((uint16_t)1)
///< 0: Disable default
///< 1: Enable


//config register2 data
#define CKI_time_out_period ((uint16_t)3) //A版有效，B、C版必须是3
///< 0: 11~22cycles
///< 1: 23~44cycles
///< 2:47~86cycles
///< 3:95~172cycles



extern uint32_t mbi6024_16bit_configuration(type_mbi6024* mbi6024);
extern uint32_t mbi6024_10bit_configuration(type_mbi6024* mbi6024);
extern uint32_t mbi6024_16bit_gray_scale(type_mbi6024* mbi6024);
extern uint32_t mbi6024_10bit_gray_scale(type_mbi6024* mbi6024);
extern uint32_t mbi6024_8bit_dot_correct(type_mbi6024* mbi6024);
extern uint32_t mbi6024_6bit_dot_correct(type_mbi6024* mbi6024);
extern void mbi6024_init(type_mbi6024* mbi6024);


//用户定义区


extern type_mbi6024 Mbi6024_in;
extern type_mbi6024 Mbi6024_out;
extern uint8_t MbiIn_sendBuff[6+MBI6024IN_NUM*24];				//最终要发送到MIB6024的数据包。外部程序不用理会
extern uint16_t MbiIn_UserGrayBuff[MBI6024IN_NUM * 12];		//用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
extern uint8_t MbiIn_UserCorrectBuff[MBI6024IN_NUM * 12];		//用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。

extern uint8_t MbiOut_sendBuff[6+MBI6024OUT_NUM*24];				//最终要发送到MIB6024的数据包。外部程序不用理会
extern uint16_t MbiOut_UserGrayBuff[MBI6024OUT_NUM * 12];		//用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
extern uint8_t MbiOut_UserCorrectBuff[MBI6024OUT_NUM * 12];		//用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。

#endif
