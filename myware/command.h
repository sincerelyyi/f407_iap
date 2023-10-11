/** **************************************************************************************
 * @file
 * @brief           命令列表
 * @note
 * @author          zhangjiayi
 * @date            2022-10-9 22:23:23
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2022-10-9 22:23:23
 * @LastEditors     zhangjiayi
 * @FilePath        tk_software/myware/command.h
 *****************************************************************************************/
#ifndef _COMMAND_
#define _COMMAND_

typedef enum _command
{
    master_channel_test			= 0x00,
    master_get_channel			= 0x01,
    master_switch_channel		= 0x02,
    master_get_hardware			= 0x03,
    master_get_software			= 0x04,
    master_get_UID				= 0x05,
    master_get_keys				= 0x06,
    master_get_coins			= 0x07,
    master_decrease_coins		= 0x08,
    master_out_cards			= 0x09,
    master_reset_mcu			= 0x0a,
    master_reset_usb			= 0x0b,
    master_pwm_ll				= 0x0c,
    master_circle_ll			= 0x0d,
    master_pwm_channel			= 0x0e,
    master_circle_channel		= 0x0f,
    master_pwm_all				= 0x10,
    master_circle_all			= 0x11,
    master_get_pwm_state		= 0x12,
    master_get_circle_state		= 0x13,
    master_set_game_state		= 0x14,
    master_get_game_state		= 0x15,
    master_set_counter			= 0x16,
    master_get_counter			= 0x17,
    //系统指令
    master_isiap                = 0x30, ///< 如果回复0,在app段，没有检测到iap;1,在app段，检测到有iap;在iap段
    master_jumptoiap            = 0x31, ///< 用于app段跳转到iap段
    master_jumptoapp            = 0x32, ///< 用于iap段跳转到app段
    master_sum                  = 0x33,
    master_senddata             = 0x34,
    master_enddata              = 0x35,


    slave_channel_test			= 0x40,
    slave_out_keys				= 0x41,
    slave_out_coins				= 0x42,
    slave_out_err				= 0x43
} commandType;
#endif
