/** **************************************************************************************
 * @file
 * @brief           LKT4101
 * @note            通过串口控制
 * @author          zhangjiayi
 * @date            2022-11-09 12:03:23
 * @version         v0.2
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2022-10-28 12:53:17
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/MBI6024.c
 *****************************************************************************************/
#ifndef _LKT4101_H
#define _LKT4101_H
#include "usart.h"
#define LKT_SEND_SIZE 128
#define LKT_RECEIVE_SIZE 128
#define LKT_FIFO_SIZE 128

// #define LKT_OUT_FRE  3579545U
#define LKT_OUT_FRE 4000000U
// #define LKT_PPS 372 //默认分频
#define LKT_PPS 16
// #define LKT_PPS 32
// #define LKT_PPS 64
#define _HalfDuplex_ // 如果是全双工，请注释此行

/** **************************************************************************************
 * @brief           Lkt.status的枚举
 * @note
 *****************************************************************************************/
typedef enum _lkt_enum
{
    lkt_idle = 0,                                   ///< lkt芯片通信处于空闲状态
    lkt_reset,                                      ///< lkt芯片的复位脚处于啦低状态
    lkt_send,                                       ///< 通信处于arm -> lkt
    lkt_receive,                                    ///< 通信处于lkt -> arm
    lkt_newdate,                                    ///< arm接收到数据, 并正在接收
    lkt_timeout,                                    ///< arm接收数据超时
} LktEnumDef;

/** **************************************************************************************
 * @brief           lkt实例结构体
 * @note
 *****************************************************************************************/
typedef struct _lkt_struck
{
    uint8_t send_buff[LKT_SEND_SIZE];               ///< 发送动lkt芯片数据的缓存
    uint8_t receive_buff[LKT_RECEIVE_SIZE];         ///< 从lkt芯片接收有效数据的缓存
    uint8_t receive_circle[LKT_RECEIVE_SIZE];       ///< 从lkt芯片接收数据的循环缓存
    uint8_t uid[17];                                ///< lkt芯片的uid
    uint8_t circle_p;                               ///< 最后接收到的字节在circle_buff的位置
    uint8_t handle_p;                               ///< 最后处理的的字节在circle_buff的位置
    uint32_t send_cplt_tick;                        ///< 记录发送完成时的tick值
    uint32_t new_data_tick;                         ///< 记录收到新数据的tick值
    uint32_t new_len;                               ///< 新收到数据包长度
    LktEnumDef status;                              ///< 当前状态
    void (*reset)(void);                            ///< 复位函数
    void (*unreset)(void);                          ///< 反复位函数
    void (*receive_monitor)();                      ///< 监控接收的数据
    void (*block_receive)(uint8_t *,uint8_t);       ///< 阻塞接收函数
    HAL_StatusTypeDef (*send)(uint8_t *buff, uint8_t len);         ///< 发送函数
} LktTypeDef;

#define LKT4101_RX_LEN 70
#define RXBUFFERSIZE   1
#define USART_REC_LEN 70

/** **************************************************************************************
 * @brief         identify动作枚举
 * @note
 *****************************************************************************************/
typedef enum _move
{
    RST,                                            ///< identify处于复位动作
    CMD_H,                                          ///< identify处于发送动作
    DOG_Wait                                        ///< identify处于等待数据接动作
} MoveEnumDef;

/** **************************************************************************************
 * @brief         identify实例结构体
 * @note
 *****************************************************************************************/
typedef struct
{
    MoveEnumDef move;                               ///< identify过程中的动作状态
    uint8_t Check_err_flag;                         ///< 校验错误计数，0合格，其他为不合格次数
    uint8_t Fact_RX_LEN;                            ///< 实际接收数据长度
    uint8_t SendBuff[70];                           ///< 串口3发送出去数组
    uint8_t RXBuff[70];                             ///< 串口3接收到的数组,LKT4101最大发过来的数据有68个，A3+长度+数据+90+00
    uint8_t Check_Val_Ago;                          ///< 校验前的值
    uint8_t Check_Val_After;                        ///< 校验后的值
    uint8_t LED_flag;                               ///<加密狗LED,=1慢闪,=0快闪
} _DOG;

/** **************************************************************************************
 * @brief         identify 随机数结构体
 * @note
 *****************************************************************************************/
typedef struct
{
    uint8_t LEN;                                    ///< 随机数的长度(多少个字节)
    uint8_t Buffer[64];                             ///< 随机数缓存
} _Random;

extern LktTypeDef Lkt;
extern int lkt4101_burn(void);
extern int lkt4101_upbaud(void);
extern void Lkt_init(void);
extern uint16_t GetSoftDogFlag(void);
extern void lkt_identify(uint32_t ms);
extern void lkt_led(uint32_t ms);
#endif
