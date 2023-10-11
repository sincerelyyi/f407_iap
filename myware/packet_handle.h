/** **************************************************************************************
 * @file
 * @brief           传输事务处理
 * @note
 * @author          zhangjiayi
 * @date            2022-10-9 22:23:23
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2022-10-9 22:23:23
 * @LastEditors     zhangjiayi
 * @FilePath        tk_software/myware/packet_handle.h
 *****************************************************************************************/
#ifndef _TIME_HANDLE__
#define _TIME_HANDLE__
#include "main.h"
#define USB_CHANNEL 1
#define RS232_CHANNEL 2
#define ALL_CHANNEL 255

extern void usb_pickup_packet(void);
extern void rs232_pickup_packet(void);
extern void answer_string(uint8_t command,char * string,uint8_t channel);
extern void answer_bin(uint8_t command,uint8_t * buff,uint16_t len,uint8_t channel);
extern void active_string(uint8_t command,char * string,uint8_t channel);
extern void active_bin(uint8_t command,uint8_t * buff,uint16_t len,uint8_t channel);
extern uint8_t Comunucate_channel;    //选择主动发送通道，USB_CHANNEL,RS232_CHANNEL,ALL_CHANNEL
extern uint8_t Rs232_receive_buff[];
#define  RS232_RECEIVE_SIZE   1024
#endif
