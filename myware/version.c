/** **************************************************************************************
 * @file
 * @brief           版本信息
 * @note            通过串口获取另外一块板的信息
 * @author          zhangjiayi
 * @date            2023-08-24 12:42:39
 * @version         v0.8
 v0.1:
	第一此上机测试
v0.2:
	1,修复，上位位机连接了USB，但并没有打开虚拟串口时，整个IO板暂停运行的问题。
	2,修复，环形灯通道样式命令无法控制P2环形灯的问题
	3,板p1-p4环形灯指令各合并位一条
	4,增加设置游戏状态，获取游戏状态，控制码表，获取码表四条指令
	5,io板能保存码表数和投币数
	6,IO板上电默认打开所有灯光
	7,修复其它一下bug
v0.3:
	1,首次使用铁电存储器，所有值初始化为0.
	2,修正误将硬件版本"WL_IO002_V0.1"显示为"WL_IO002_V0.2"
v0.4
	1,修复获取币数和减币指令不区分正常游戏状态和管理员状态的问题。
v0.5
	1,环形灯通信频率降低一倍，为1.3MHz。
v0.6
    1,按要求实现多种环形灯整体样式。
    2，PWM通道100ms周期播放1s后常亮
v0.7
    1，0x11 环形灯整体样式控制0x03-0x07样式，改为结束后返回游戏有币状态。
    2，0x11 环形灯整体样式控制0x12-0x13样式，改为结束后返回结算中有币状态。
    3，0x11 环形灯整体样式控制所有要降低亮度的灯光，亮度降低更暗。
v0.8
	1，扩容硬件版本号
    2，铁电改为有数据变化就存储，并将i2c gpio速度降为低
	3，将片内外设GPIO频率降尽可能低
v0.9
	1,各p位有各p位的投币码表.
v1.0
	1,修正按键有时会有误动作的问题
v1.1
    1,实现非阻塞加密芯片认证
    2,投币器最大脉宽由100ms改为500ms
v1.2
    1,将按键整体处理更改为各个位单独处理
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2021-04-18 21:36:01
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/version.c
 *****************************************************************************************/
#include "version.h"
#include "stdio.h"
#include "string.h"
#include "main.h"

/** **************************************************************************************
 * @brief 生成硬件版本信息
 * @note
 * @param void
 * @return char *
 * @retval  返回版本信息字符串地址
 *****************************************************************************************/
char * hardware_version(void)
{
    uint8_t pins = 0;
    if(HAL_GPIO_ReadPin(VER0_GPIO_Port,VER0_Pin) == GPIO_PIN_RESET)
        pins = pins | 0x01;
    if(HAL_GPIO_ReadPin(VER1_GPIO_Port,VER1_Pin) == GPIO_PIN_RESET)
        pins = pins | 0x02;
    if(HAL_GPIO_ReadPin(VER2_GPIO_Port,VER2_Pin) == GPIO_PIN_RESET)
        pins = pins | 0x04;
    switch(pins)
    {
    case 0:
        return "WL_IO002_V0.1";
    case 1:
        return "WL_IO002_V0.2";
    case 2:
        return "WL_IO002_V0.3";
    case 3:
        return "WL_IO002_V0.4";
    case 4:
        return "WL_IO002_V0.5";
    case 5:
        return "WL_IO002_V0.6";
    case 6:
        return "WL_IO002_V0.7";
    case 7:
        return "WL_IO002_V0.8";
    default:
        return "Not identifiable";
    }
}

/** **************************************************************************************
 * @brief 生成固件版本信息
 * @note
 * @param void
 * @return char *
 * @retval  返回版本信息字符串地址
 *****************************************************************************************/
char * software_version(void)
{
    return "TK_SOFTWARE_V1.2";
}


/** **************************************************************************************
 * @brief 生成当前板的版本信息
 * @note  将处理好的版本信息传给verstr字符串
 * @param char *verstr:字符串容量不能小于26bytes
 * @return void
 * @retval
 *****************************************************************************************/
void local_version(char *verstr)
{
    char year[5];  ///< __DATE__字符串中的年字符串
    char month[4]; ///< __DATE__字符串中的月字符串
    char day[3];   ///< __DATE__字符串中的日字符串
    int mda;       ///< 将mda字符串转换为数值存在此变量,因为mday位数不定

    sscanf(__DATE__, "%s %s %s", month, day, year);
    sscanf(day, "%d", &mda);

    if (strcmp(month, "Jan") == 0)
        strcpy(month, "01");
    if (strcmp(month, "Feb") == 0)
        strcpy(month, "02");
    if (strcmp(month, "Mar") == 0)
        strcpy(month, "03");
    if (strcmp(month, "Apr") == 0)
        strcpy(month, "04");
    if (strcmp(month, "May") == 0)
        strcpy(month, "05");
    if (strcmp(month, "Jun") == 0)
        strcpy(month, "06");
    if (strcmp(month, "Jul") == 0)
        strcpy(month, "07");
    if (strcmp(month, "Aug") == 0)
        strcpy(month, "08");
    if (strcmp(month, "Sep") == 0)
        strcpy(month, "09");
    if (strcmp(month, "Oct") == 0)
        strcpy(month, "10");
    if (strcmp(month, "Nov") == 0)
        strcpy(month, "11");
    if (strcmp(month, "Dec") == 0)
        strcpy(month, "12");
    sprintf(verstr, "%s %s%s%.2d %s", SOFT_VERSION, year, month, mda, __TIME__);
}
