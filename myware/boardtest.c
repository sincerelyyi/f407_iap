/** **************************************************************************************
 * @file
 * @brief           io002板测试的主体
 * @note                        测试板上的所有功能。以方便出货
 * @author          zhangjiayi
 * @date            2023-03-24 12:23:23
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-03-24 12:23:23
 * @LastEditors     zhangjiayi
 * @FilePath        /test/myware/boardtest.c
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
uint32_t Main_count = 0;
extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
extern type_mbi6024 Mbi6024_in;
extern type_mbi6024 Mbi6024_out;

extern uint8_t MbiIn_sendBuff[6+MBI6024IN_NUM*24];                              //最终要发送到MIB6024的数据包。外部程序不用理会
extern uint16_t MbiIn_UserGrayBuff[MBI6024IN_NUM * 12];         //用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
extern uint8_t MbiIn_UserCorrectBuff[MBI6024IN_NUM * 12];               //用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。

extern uint8_t MbiOut_sendBuff[6+MBI6024OUT_NUM*24];                            //最终要发送到MIB6024的数据包。外部程序不用理会
extern uint16_t MbiOut_UserGrayBuff[MBI6024OUT_NUM * 12];               //用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
extern uint8_t MbiOut_UserCorrectBuff[MBI6024OUT_NUM * 12];             //用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。

/** **************************************************************************************
* @brief 其他out的测试
* @note  输出全部是OC输出
* @param ms: 每过ms毫秒翻转一次
* @return void
* @retval
*****************************************************************************************/
static void out_test(uint32_t ms)
{
    static uint32_t out_test_count = 0;
    static uint8_t flag_out_init = 0;

    if(flag_out_init == 0)
    {
        HAL_GPIO_WritePin(OC1_GPIO_Port,OC1_Pin,GPIO_PIN_SET);
        HAL_GPIO_WritePin(OC2_GPIO_Port,OC2_Pin,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OC3_GPIO_Port,OC3_Pin,GPIO_PIN_SET);
        HAL_GPIO_WritePin(OC4_GPIO_Port,OC4_Pin,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OC5_GPIO_Port,OC5_Pin,GPIO_PIN_SET);
        HAL_GPIO_WritePin(OC6_GPIO_Port,OC6_Pin,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OC7_GPIO_Port,OC7_Pin,GPIO_PIN_SET);
        HAL_GPIO_WritePin(OUT1_GPIO_Port,OUT1_Pin,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT2_GPIO_Port,OUT2_Pin,GPIO_PIN_SET);
        HAL_GPIO_WritePin(OUT3_GPIO_Port,OUT3_Pin,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT4_GPIO_Port,OUT4_Pin,GPIO_PIN_SET);
        HAL_GPIO_WritePin(OUT5_GPIO_Port,OUT5_Pin,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT6_GPIO_Port,OUT6_Pin,GPIO_PIN_SET);
        HAL_GPIO_WritePin(OUT7_GPIO_Port,OUT7_Pin,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT8_GPIO_Port,OUT8_Pin,GPIO_PIN_SET);
        flag_out_init = 1;
    }

    if(HAL_GetTick() - out_test_count > ms)
    {
        HAL_GPIO_TogglePin(OC1_GPIO_Port,OC1_Pin);
        HAL_GPIO_TogglePin(OC2_GPIO_Port,OC2_Pin);
        HAL_GPIO_TogglePin(OC3_GPIO_Port,OC3_Pin);
        HAL_GPIO_TogglePin(OC4_GPIO_Port,OC4_Pin);
        HAL_GPIO_TogglePin(OC5_GPIO_Port,OC5_Pin);
        HAL_GPIO_TogglePin(OC6_GPIO_Port,OC6_Pin);
        HAL_GPIO_TogglePin(OC7_GPIO_Port,OC7_Pin);
        HAL_GPIO_TogglePin(OUT1_GPIO_Port,OUT1_Pin);
        HAL_GPIO_TogglePin(OUT2_GPIO_Port,OUT2_Pin);
        HAL_GPIO_TogglePin(OUT3_GPIO_Port,OUT3_Pin);
        HAL_GPIO_TogglePin(OUT4_GPIO_Port,OUT4_Pin);
        HAL_GPIO_TogglePin(OUT5_GPIO_Port,OUT5_Pin);
        HAL_GPIO_TogglePin(OUT6_GPIO_Port,OUT6_Pin);
        HAL_GPIO_TogglePin(OUT7_GPIO_Port,OUT7_Pin);
        HAL_GPIO_TogglePin(OUT8_GPIO_Port,OUT8_Pin);

        out_test_count = HAL_GetTick();
    }
}


/** **************************************************************************************
* @brief mbi6024的测试
* @note  输出全部是OC输出
* @param ms: 每过ms毫秒翻转一次
* @return void
* @retval
*****************************************************************************************/
static void mbi6024_in_test(type_mbi6024 * mbi6024,uint32_t ms)
{
    static uint32_t mbi6024_test_count = 0;
    if(HAL_GetTick() - mbi6024_test_count > ms)
    {
        for(uint16_t i = 0; i < mbi6024->ics* 6; i++)
        {
            (mbi6024->gray_scale_data)[i*2]   += 0x100;
            (mbi6024->gray_scale_data)[i*2+1] -= 0x100;
            //Mbi_gray_scale_data[i*2]   = 0x8000;
            //Mbi_gray_scale_data[i*2+1]   = 0x8000;
        }

//              for(uint16_t i = 0; i < MBI6024_NUM* 6; i++)
//              {
//                      Mbi_dot_correct_data[i*2]   += 0x01;
//                      Mbi_dot_correct_data[i*2+1] -= 0x01;
//              }

//       mbi6024_10bit_gray_scale(mbi6024);
//       HAL_Delay(1);
//              mbi6024_8bit_dot_correct();
//              HAL_Delay(1);

        mbi6024_test_count = HAL_GetTick();
    }
}
/** **************************************************************************************
* @brief mbi6024的测试
* @note
* @param ms: 每过ms毫秒翻转一次
* @return void
* @retval
*****************************************************************************************/
static void mbi6024_out_test(type_mbi6024 * mbi6024,uint32_t ms)
{
    static uint32_t mbi6024_test_count = 0;

    if(HAL_GetTick() - mbi6024_test_count > ms)
    {
        for(uint16_t i = 0; i < mbi6024->ics* 6; i++)
        {
            (mbi6024->gray_scale_data)[i*2]   += 0x100;
            (mbi6024->gray_scale_data)[i*2+1] -= 0x100;
            //Mbi_gray_scale_data[i*2]   = 0x8000;
            //Mbi_gray_scale_data[i*2+1]   = 0x8000;
        }

//              for(uint16_t i = 0; i < MBI6024_NUM* 6; i++)
//              {
//                      Mbi_dot_correct_data[i*2]   += 0x01;
//                      Mbi_dot_correct_data[i*2+1] -= 0x01;
//              }

//     mbi6024_10bit_gray_scale(mbi6024);
        HAL_Delay(1);
//              mbi6024_8bit_dot_correct();
//              HAL_Delay(1);

        mbi6024_test_count = HAL_GetTick();
    }
}

#define IN_NUM 8
GPIO_TypeDef * port[IN_NUM] =
{
    IN1_GPIO_Port,
    IN2_GPIO_Port,
    IN3_GPIO_Port,
    IN4_GPIO_Port,
    IN5_GPIO_Port,
    IN6_GPIO_Port,
    IN7_GPIO_Port,
    IN8_GPIO_Port,
};

uint16_t  pin[IN_NUM] =
{
    IN1_Pin,
    IN2_Pin,
    IN3_Pin,
    IN4_Pin,
    IN5_Pin,
    IN6_Pin,
    IN7_Pin,
    IN8_Pin,
};


/** **************************************************************************************
* @brief INPUT的测试
* @note
* @param ms: 每过ms 检测一个通道
* @return void
* @retval
*****************************************************************************************/
static void input_test(uint32_t ms)
{
    static uint32_t test_count = 0;
    static GPIO_PinState in0[IN_NUM] = {GPIO_PIN_SET};
    GPIO_PinState temp;


    if(HAL_GetTick() - test_count > ms)
    {
        for(uint8_t i = 0; i<IN_NUM; i++)
        {
            temp = HAL_GPIO_ReadPin(port[i],pin[i]);
            if(temp == GPIO_PIN_RESET && in0[i] != GPIO_PIN_RESET)
            {
                debug_printf("IN%d is putdown\n",i+1);
            }
            in0[i] = temp;
        }
        test_count = HAL_GetTick();
    }
}


/** **************************************************************************************
* @brief keys的测试
* @note
* @param ms: 每过ms 检测一个通道
* @return void
* @retval
*****************************************************************************************/
static void key_test(uint32_t ms)
{
    static uint32_t test_count = 0;
    static uint8_t olddata[NUM_165] = {0xff};

    if(HAL_GetTick() - test_count > ms)
    {
        for(uint8_t j = IN_NUM; j >0 ; j--)
        {
            for(uint8_t i = 0; i<8; i++)
            {
                if(((in165_buff[j-1] & (0x01 << i)) != 0) &&\
                        ((olddata[j-1] & (0x01 << i)) == 0))
                {
                    debug_printf("key%d is putdown\n",(6-j)*8+i+1);
                }
            }
            olddata[j-1] = in165_buff[j-1];
        }
        test_count = HAL_GetTick();
    }
}
/** **************************************************************************************
* @brief 铁电的测试
* @note
* @param ms: 每过ms毫秒翻转一次
* @return void
* @retval
*****************************************************************************************/
void fm_test(void)
{
    FM.Data = 0x55aa55aa;
    fm24c64_save();
    HAL_Delay(100);
    FM.Data = 0;
    HAL_Delay(10);
    fm24c64_load();
    HAL_Delay(100);
    if(FM.Data == 0x55aa55aa)
    {
        debug_printf("fm test ok!\n");
    }
    else
    {
        debug_printf("fm test failed!\n");
        while(1);
    }

}



/** **************************************************************************************
* @brief 获取单片机UID
* @note
* @param
* @return int16_t: the number of songs
* @retval
 *****************************************************************************************/
void get_uid(void)
{
    debug_printf("STM32F407 UID: %.8x-%.8x-%.8x\n",HAL_GetUIDw2(),HAL_GetUIDw1(),HAL_GetUIDw0());
}

/** **************************************************************************************
 * @brief 板测试的主函数
 * @note  这是单线程的
 * @param 无
 * @return void
 * @retval
 *****************************************************************************************/
void main_function(void)
{
    static uint8_t main_init = 0;
    //uint8_t test[] = "usb connected\n";
    Main_count++;

    if(main_init == 0)
    {
        /*for(uint16_t i = 0; i <100; i++)
        {
            //(hUsbDeviceFS.dev_state == USBD_STATE_SUSPENDED);
            if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
            {
                break;
            }
            HAL_Delay(100);
        }

        debug_printf("\nthis is IO002 test:\n");

        if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
        {
            for(uint16_t i = 0; i <100; i++)
            {
                if(CDC_Transmit_FS((uint8_t *)test, sizeof(test)-1)==0)
                {
                    break;
                }
                HAL_Delay(100);
            }
        }*/
        debug_printf("pcb version:%s\n",hardware_version());
        debug_printf("soft version:%s\n",software_version());
//        HAL_Delay(3000);
//        char t[]="0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
//        uint32_t   n,o;
//        o = HAL_GetTick();
//        for(uint32_t i =0;i<500;i++)
//        {
//            while(CDC_Transmit_FS((uint8_t *)t, 10));
//        }
//        n = HAL_GetTick();
//        debug_printf("o=%ld\nn=%ld\ntime=%ld\n",o,n,n-o);
        get_uid();
        fm_test();
        if(lkt4101_burn() ==0)
            ;
        // while(1);
        main_init = 1;
        HAL_Delay(100);
    }
    out_test(1000);
    mbi6024_in_test(&Mbi6024_in,10);
    mbi6024_out_test(&Mbi6024_out,10);
    input_test(5);
    key_test(20);
    lkt_identify(1);
    lkt_led(100);;
}
