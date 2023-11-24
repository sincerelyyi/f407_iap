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
    // 上位机主动发送的命令
    master_channel_test         = 0x00, ///< 通道测试，发送什么数据，就回复什么数据
    master_get_channel          = 0x01, ///< 获取当前通道
    master_switch_channel       = 0x02, ///< 切换通道
    master_get_hardware         = 0x03, ///< 获取硬件版本号
    master_get_software         = 0x04, ///< 获取软件版本号
    master_get_UID              = 0x05, ///< 获取IO板的UID
    master_get_keys             = 0x06, ///< 获取当前键值
    master_get_coins            = 0x07, ///< 获取当前币数
    master_decrease_coins       = 0x08, ///< 扣除币数
    master_cipher_test          = 0x09, ///< 加密测试
    master_reset_mcu            = 0x0a, ///< 复位单片机
    master_reset_usb            = 0x0b, ///< 重新连接usb
    master_pwm_ll               = 0x0c, ///< pwm灯光底层控制
    master_circle_ll            = 0x0d, ///< 环形灯底层控制
    master_pwm_channel          = 0x0e, ///< pwm通道样式控制
    master_circle_channel       = 0x0f, ///< 环形灯通道样式控制
    master_pwm_all              = 0x10, ///< pwm整体样式控制
    master_circle_all           = 0x11, ///< 环形灯整体样式控制
    master_get_pwm_state        = 0x12, ///< 获取pwm灯光当前状态
    master_get_circle_state     = 0x13, ///< 获取环形灯当前状态
    master_set_game_state       = 0x14, ///< 设置游戏状态（游戏或管理员模式)
    master_get_game_state       = 0x15, ///< 获取游戏状态
    master_set_counter          = 0x16, ///< 设置未跳码表数
    master_get_counter          = 0x17, ///< 获取未跳码表数
    master_sign_uid             = 0x18, ///< 写uid签名到铁电存储器
    master_sign_force           = 0x19, ///< 强行写uid签名到铁电存储器
    master_get_cryptstring      = 0x1a, ///< 获取加密后的"wahlap"字符串
    master_set_crypt_state      = 0x1b, ///< 设置是否要通信加密
    master_get_crypt_state      = 0x1c, ///< 获取是否要通信加密
    //系统指令
    master_isiap                = 0x30, ///< 如果回复0,在app段，没有检测到iap;1,在app段，检测到有iap;在iap段
    master_jumptoiap            = 0x31, ///< 用于app段跳转到iap段
    master_jumptoapp            = 0x32, ///< 用于iap段跳转到app段
    master_sum                  = 0x33, ///< 暂时没有使用
    master_senddata             = 0x34, ///< 返送烧录数据
    master_enddata              = 0x35, ///< 暂时没有使用
    master_rdp                  = 0x36, ///< 设置读保护
    master_unrdp                = 0x37, ///< 解锁读保护
    master_get_date             = 0x38, ///< 读日期
    master_get_otp              = 0x39, ///< 获取OTP数据
    master_get_code_sign        = 0x3a, ///< 获取code hash and code sign
    // 下位机主动发送的命令
    slave_channel_test          = 0x40, ///< 下位机主动发送的通道测试命令
    slave_out_keys              = 0x41, ///< 下位机主动发送的键值
    slave_out_coins             = 0x42, ///< 下位机主动发送的币数
    slave_out_err               = 0x43, ///< 下位机主动发送的报错代码
} commandType;
#endif
