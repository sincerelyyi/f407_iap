/** ****************************************************************************
* @file             myware/version.c
* @brief            版本信息
* @note             通过串口获取另外一块板的信息
* @author           zhangjiayi
* @date             2023-08-24 12:42:39
* @version          V3
 v1.0:
    从tk iap 修改串口脚位而来的	
* @copyright        Copyright (c) 2020-2050 zhangjiayi
* @par              LastEdit
* @LastEditTime     2026-08-28 13:21:56
* @LastEditors      jiayi
*******************************************************************************/
#include "version.h"
#include "stdio.h"
#include "string.h"


/** **************************************************************************************
 * @brief 生成固件版本信息
 * @note
 * @param void
 * @return char *
 * @retval  返回版本信息字符串地址
 *****************************************************************************************/
char * software_version(void)
{
    return "TK_IAP_V1.0";
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
