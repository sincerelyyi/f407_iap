/** **************************************************************************************
 * @file
 * @brief           usb传输事务处理
 * @note
 * @author          zhangjiayi
 * @date            2023-04-02 12:23:23
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-04-02 12:23:23
 * @LastEditors     zhangjiayi
 * @FilePath        /test/myware/time_handle.c
 *****************************************************************************************/
#include "main.h"
#include "packet_handle.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "command.h"
#include "stdio.h"
#include "version.h"
#include "string.h"
#include "gpio.h"
#include "usart.h"
#include "mainFunction.h"
#include <stm32f4xx_hal_flash.h>

extern USBD_HandleTypeDef hUsbDeviceFS;

uint16_t Usb_send_p = 0;
uint8_t Comunucate_channel = 0;     //选择主动发送通道，0，usb，1，rs232

uint8_t Rs232_receive_buff[RS232_RECEIVE_SIZE] = {0};
#define RS232_SEND_SIZE 1024
uint8_t Rs232_send_buff[2][RS232_SEND_SIZE] = {0};
uint16_t Rs232_send_p[2] = {0};
uint8_t Rs232_pipo = 0;
uint16_t Rs232_handle_p = 0;      //正在处理的指针

uint16_t program_sum = 0;
/** **************************************************************************************
* @brief checksum
* @note
* @param buff:发送的数据指针，len：数据长度
* @return uint8_t checksum值
* @retval
*****************************************************************************************/
uint8_t checksum(uint8_t *buff,uint16_t len)
{
    uint8_t sum = 0;
    for(uint16_t i = 0; i< len; i++)
    {
        sum += buff[i];
    }
    return  sum;
}

/** **************************************************************************************
* @brief 打包发送usb数据
* @note
* @param buff:发送的数据指针，len：数据长度
* @return void
* @retval
*****************************************************************************************/
static void rs232_send(uint8_t *buff,uint16_t len)
{
    if(len > RS232_SEND_SIZE - Rs232_send_p[Rs232_pipo])   //如果缓存不够,发送旧缓存，切换新缓存
    {
        if(HAL_UART_Transmit_DMA(&huart1,Rs232_send_buff[Rs232_pipo],Rs232_send_p[Rs232_pipo%2]) == HAL_OK)
        {
            Rs232_pipo = (Rs232_pipo+1)%2;
            Rs232_send_p[0] = 0;
            Rs232_send_p[1] = 0;

            for(uint16_t i= 0; i<len; i++)
            {
                Rs232_send_buff[Rs232_pipo][(Rs232_send_p[Rs232_pipo])++] = buff[i];
            }
        }

    }
    else                                       //缓存足够，放入缓存
    {
        for(uint16_t i= 0; i<len; i++)
        {
            Rs232_send_buff[Rs232_pipo][(Rs232_send_p[Rs232_pipo])++] = buff[i];
        }
    }
}

/** **************************************************************************************
* @brief 打包发送usb数据
* @note
* @param buff:发送的数据指针，len：数据长度
* @return void
* @retval
*****************************************************************************************/
static void usb_send(uint8_t *buff,uint16_t len)
{
    if(hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED)
        return;

    if(len > APP_TX_DATA_SIZE - Usb_send_p)   //如果缓存不够
    {

        Usb_send_len[Usb_send_in] = Usb_send_p;                    ///< 数据长度等于当前指针
        Usb_send_in = (Usb_send_in +1)%USBSENDSIZE;                ///< 切换发送缓存
        Usb_send_p = 0;                                            ///< 当前指针归0
        for(uint16_t i= 0; i<len; i++)
        {
            UserTxBufferFS[Usb_send_in][Usb_send_p++] = buff[i];   ///< 写入新缓存
        }
        Usb_send_len[Usb_send_in] = Usb_send_p;                    ///< 数据长度等于当前指针
    }
    else                                       //缓存足够，放入缓存
    {
        for(uint16_t i= 0; i<len; i++)
        {
            UserTxBufferFS[Usb_send_in][Usb_send_p++] = buff[i];
        }
        Usb_send_len[Usb_send_in] = Usb_send_p;                    ///< 数据长度等于当前指针
        return;
    }
}

/** **************************************************************************************
* @brief string打包
* @note
* @param command,*string,isanswert:1,为回复，0为主动发送
* @return void
* @retval
*****************************************************************************************/
static inline void _string(uint8_t command,char * string,uint8_t isanswer,uint8_t channel)
{
    uint16_t len;
    len = 5+strlen(string)+1;

    uint8_t answer[len];
    answer[0] = 0x55;
    if(isanswer)
        answer[1] = command | 0x80;
    else
        answer[1] = command;
    answer[2] = strlen(string)+1;
    sprintf((char *)(answer+3),"%s",string);                          //字符串部分
    answer[sizeof(answer)-2] =0;
    answer[sizeof(answer)-1] = 0xaa;
    answer[sizeof(answer)-2] = 0-checksum(answer,sizeof(answer));     //赋值checksum位
    if(channel == USB_CHANNEL)
        usb_send(answer,sizeof(answer));
    if(channel == RS232_CHANNEL)
        rs232_send(answer,sizeof(answer));
}

/** **************************************************************************************
* @brief bin打包
* @note
* @param command,*string,isanswert:1,为回复，0为主动发送
* @return void
* @retval
*****************************************************************************************/
static inline void _bin(uint8_t command,uint8_t * buff,uint16_t len,uint8_t isanswer,uint8_t channel)
{
    uint16_t length;
    length = 5+len;
    uint8_t answer[length];
    answer[0] = 0x55;
    if(isanswer)
        answer[1] = command | 0x80;
    else
        answer[1] = command;
    answer[2] = len;
    memcpy(answer+3,buff,len);
    answer[sizeof(answer)-2] =0;
    answer[sizeof(answer)-1] = 0xaa;
    answer[sizeof(answer)-2] = 0-checksum(answer,sizeof(answer));     //赋值checksum位
    if(channel == USB_CHANNEL)
        usb_send(answer,sizeof(answer));
    if(channel == RS232_CHANNEL)
        rs232_send(answer,sizeof(answer));
}

/** **************************************************************************************
* @brief 回复string打包
* @note
* @param command,*string
* @return void
* @retval
*****************************************************************************************/
void answer_string(uint8_t command,char * string,uint8_t channel)
{
    _string(command,string,1,channel);

}

/** **************************************************************************************
* @brief 回复二进制数据打包
* @note buff，len是指数据部分
* @param
* @return void
* @retval
*****************************************************************************************/
void answer_bin(uint8_t command,uint8_t * buff,uint16_t len,uint8_t channel)
{
    _bin(command,buff,len,1,channel);
}

/** **************************************************************************************
* @brief 主动发送string打包
* @note
* @param command,*string
* @return void
* @retval
*****************************************************************************************/
void active_string(uint8_t command,char * string,uint8_t channel)
{
    _string(command,string,0,channel);
}

/** **************************************************************************************
* @brief 主动发送二进制数据打包
* @note buff，len是指数据部分
* @param
* @return void
* @retval
*****************************************************************************************/
void active_bin(uint8_t command,uint8_t * buff,uint16_t len,uint8_t channel)
{
    _bin(command,buff,len,0,channel);
}

/** **************************************************************************************
* @brief get hardware version
* @note
* @param void
* @return void
* @retval
*****************************************************************************************/
void get_hardware(uint8_t channel)
{
    answer_string(master_get_hardware,hardware_version(),channel);
}

/** **************************************************************************************
* @brief get hardware version
* @note
* @param void
* @return void
* @retval
*****************************************************************************************/
void get_software(uint8_t channel)
{
    answer_string(master_get_software,software_version(),channel);
}


/** **************************************************************************************
  * @brief 判断是否在iap段
  * @note
  * @param buff:要处理的数据指针， len：要处理的数据长度
  * @return void
  * @retval
  *****************************************************************************************/
void  iniap(uint8_t *buff,uint16_t len,uint8_t channel)
{
    answer_bin(master_iniap, NULL, 0, channel);
}

/** **************************************************************************************
  * @brief 跳转到app程序段
  * @note
  * @param buff:要处理的数据指针， len：要处理的数据长度
  * @return void
  * @retval
  *****************************************************************************************/
void  jumptoapp(uint8_t *buff,uint16_t len,uint8_t channel)
{
#define addr 0x08010000
    void (*jumpapp)(void);                                  // 声明一个函数指针
    __disable_irq();                                        // 关闭所有中断
    HAL_SuspendTick();                                      // 关闭tick定时器
    HAL_RCC_DeInit();                                       // 设置默认时钟HSI
    for(uint8_t i=0; i<8; i++)                              // 清除所有中断为
    {
        NVIC->ICER[i] = 0xffffffff;
        NVIC->ICPR[i] = 0xffffffff;
    }
    HAL_DeInit();
    SCB->VTOR = addr;                                       // 重定向中断向量表
    HAL_ResumeTick();                                       // 开启tick
    __enable_irq();                                         // 开启全局中断
    jumpapp = (void (*)(void)) (*((uint32_t *)(addr + 4))); // 中断复位地址
    __set_MSP(*(uint32_t *)addr);                           // 设置主堆栈指针
    // __set_CONTROL(0);                                       // 如果使用RTOS，需要这条指令
    jumpapp();                                              // 跳转
#undef  addr
}

/** **************************************************************************************
  * @brief senddata
  * @note
  * @param buff:要处理的数据指针， len：要处理的数据长度
  * @return void
  * @retval
  *****************************************************************************************/
void  senddata(uint8_t *buff,uint16_t len,uint8_t channel)
{
    //回复一个字节的状态
    //0,正常写完flash
    //1，固件超界
    //2, 写flash错误
    //3,擦除flash_sector错误
    uint32_t Address;
    uint32_t sectorerror = 0;
    uint8_t state = 0;

    FLASH_EraseInitTypeDef my_flash;
    // __disable_irq();                                        // 关闭所有中断
    HAL_FLASH_Unlock();
    __HAL_FLASH_DATA_CACHE_DISABLE();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    Address =  *(uint32_t *)(buff+3);
    //debug_printf("0x%.8x\n",Address);
    if(Address ==  0x8010000)
    {
        my_flash.TypeErase = FLASH_TYPEERASE_SECTORS;
        my_flash.Sector = FLASH_SECTOR_4;
        my_flash.NbSectors = 1;
        my_flash.VoltageRange = VOLTAGE_RANGE_3;
        if(HAL_FLASHEx_Erase(&my_flash, &sectorerror) != HAL_OK)
        {
            __enable_irq();
            __HAL_FLASH_DATA_CACHE_ENABLE();
            HAL_FLASH_Lock();
            state = 3;
            answer_bin(master_senddata, &state,1, channel);
            return;
        }
    }
    if((Address - 0x8020000)%0x20000 == 0)
    {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
        my_flash.TypeErase = FLASH_TYPEERASE_SECTORS;
        my_flash.Sector = FLASH_SECTOR_5 + (Address - 0x8020000)/0x20000;
        if(my_flash.Sector >= FLASH_SECTOR_7) // FLASH_SECTOR_7是iap是所在
        {
            __enable_irq();
            __HAL_FLASH_DATA_CACHE_ENABLE();
            HAL_FLASH_Lock();
            state = 1;
            answer_bin(master_senddata, &state,1, channel);
            return;
        }
        my_flash.NbSectors = 1;
        my_flash.VoltageRange = VOLTAGE_RANGE_3;
        if(HAL_FLASHEx_Erase(&my_flash, &sectorerror) != HAL_OK)
        {
            __enable_irq();
            __HAL_FLASH_DATA_CACHE_ENABLE();
            HAL_FLASH_Lock();
            state = 3;
            answer_bin(master_senddata, &state,1, channel);
            return;
        }
    }
    for(uint8_t i = 0; i< len-9; i+=2)
    {
        if( HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Address, *(uint16_t *)(buff+7+i)) != HAL_OK)
        {
            __enable_irq();
            __HAL_FLASH_DATA_CACHE_ENABLE();
            HAL_FLASH_Lock();
            state = 2;
            answer_bin(master_senddata, &state,1, channel);
            return;
        }
        Address += 2;
    }
    __enable_irq();
    __HAL_FLASH_DATA_CACHE_ENABLE();
    HAL_FLASH_Lock();
    state = 0;
    answer_bin(master_senddata, &state,1, channel);
}

/** **************************************************************************************
  * @brief enddata
  * @note
  * @param buff:要处理的数据指针， len：要处理的数据长度
  * @return void
  * @retval
  *****************************************************************************************/
void  enddata(uint8_t *buff,uint16_t len,uint8_t channel)
{
}

/** **************************************************************************************
* @brief 处理接收到的数据
* @note
* @param buff:要处理的数据指针， len：要处理的数据长度
* @return void
* @retval
*****************************************************************************************/
void packet_receive_handle(uint8_t *buff,uint16_t len,uint8_t channel)
{
    switch (buff[1])
    {
    case master_get_hardware:
        get_hardware(channel);
        break;
    case master_get_software:
        get_software(channel);
        break;
    case master_iniap:
        iniap(buff, len, channel);
        break;
    case master_jumptoapp:
        jumptoapp(buff,len,channel);
        break;
    case master_sum:
        break;
    case master_senddata:
        senddata(buff, len, channel);
        break;
    case master_enddata:
        enddata(buff, len, channel);
        break;
    default:
        ;
    }
}

/** **************************************************************************************
* @brief 解开usb接收到的数据包
* @note
* @param void
* @return void
* @retval
*****************************************************************************************/
#define  size 2048
uint16_t usb_buff_p = 0;
uint8_t usb_buff[size] = {0};
void usb_pickup_packet(void)
{
    uint16_t number = 0;
    uint16_t tmp_handl_p = 0xffff; ///< 为防止假包暂存处理计数i
    for(uint8_t packet=0; packet < USBRECEIVESIZE*APP_RX_DATA_SIZE/size; packet++)
    {
        if(Usb_receive_in == Usb_receive_out) ///<无数据，返回
        {
            break;
        }
        //将usb接收缓存复制到数组buff
        for(; Usb_receive_in != Usb_receive_out;)
        {
            if(Usb_receive_len[Usb_receive_out] > size -usb_buff_p )
            {
                break;
            }
            else
            {
                for(uint8_t j = 0; j<Usb_receive_len[Usb_receive_out]; j++)
                {
                    usb_buff[usb_buff_p++] =  UserRxBufferFS[Usb_receive_out][j];
                }
                Usb_receive_out = (1+Usb_receive_out)%USBRECEIVESIZE;
            }
        }
        //处理复制过来的数据
        uint16_t i;
        for(i=0; i < usb_buff_p; i++)
        {
            if(usb_buff[i] == 0x55)
            {
                if((i+2)< usb_buff_p)                                           //数据段n就在此buff中
                {
                    number = usb_buff[i+2];
                    if(i+number +4 < usb_buff_p)                                //整个数据包都在此buff中
                    {
                        //判断数据包是否有效
                        if(usb_buff[i+number+4] == 0xaa)      //包尾有效
                        {

                            if(checksum(usb_buff+i,number +5) ==0)             //checksum验证成功，数据包最终有效
                            {
                                packet_receive_handle(usb_buff+i,number+5,USB_CHANNEL);        //交给数据处理
                                i = i+ number + 4;
                                tmp_handl_p = 0xffff; ///< 怀疑假包后面有真包，坐实怀疑。0xffff是为了for后面恢复假包地址
                            }
                        }
                    }
                    else                                                    //整个数据包不全在此buff中，等待数据补充
                    {
                        //要防止假包
                        if(tmp_handl_p == 0xffff)                           //之前没有进入怀疑假包状态
                        {
                            tmp_handl_p = i;
                        }
                        else                                                //之前已经进入了怀疑假包状态
                        {
                            i = tmp_handl_p;                                //恢复怀疑假包的地址
                            tmp_handl_p = 0xffff;
                            break;                                          //退出循环体，补充数据
                        }
                    }
                }
                else                                                        //数据段n不在此buff中，等待数据补充
                {
                    break;
                }
            }
        }
        if(tmp_handl_p != 0xffff) ///< 怀疑的假包后面没有真包
        {
            i = tmp_handl_p; ///< 恢复怀疑假包的地址
            tmp_handl_p = 0xffff;
        }
        //全部处理，buff_P.有没有处理玩的放在前面
        uint16_t k;
        for(k = 0; k< usb_buff_p-i;)
        {

            usb_buff[k] = usb_buff[i+k];                                        //等待数据补充,将之前没有处理的数据放在buff前面。
            k++;

        }
        usb_buff_p = k;
    }
    //发送usb旧缓存
    for(; Usb_send_in != Usb_send_out;)
    {
        if(Usb_send_len[Usb_send_out])   ///< 如果有数据，就发送
        {
            if(CDC_Transmit_FS(UserTxBufferFS[Usb_send_out], Usb_send_len[Usb_send_out]) == USBD_OK )
            {
                Usb_send_len[Usb_send_out] = 0;  ///< 发送成功后，数据长度归0
                Usb_send_out = (Usb_send_out+1)% USBSENDSIZE;///< 发送成功，下一个
            }
            return;
        }
        else
        {
            Usb_send_out = (Usb_send_out+1)% USBSENDSIZE; ///< 如果无数据，下一个
        }
    }
    //发送最新缓存
    if(Usb_send_len[Usb_send_out])   ///< 如果有数据，就发送
    {
        if(CDC_Transmit_FS(UserTxBufferFS[Usb_send_out], Usb_send_len[Usb_send_out]) == USBD_OK )
        {
            memset(Usb_send_len,0,sizeof(Usb_send_len));                        ///< 发送成功后，清除所有旧数据
            Usb_send_in = (Usb_send_in +1)%USBSENDSIZE;                ///< 切换发送缓存
            Usb_send_p = 0;                                            ///< 当前指针归0
            Usb_send_out = (Usb_send_out+1)% USBSENDSIZE;
            return;
        }
    }
}
#undef size

/** **************************************************************************************
* @brief 解开rs232接收到的数据包
* @note
* @param void
* @return void
* @retval
*****************************************************************************************/
#define rs232_top_p    ((RS232_RECEIVE_SIZE  - (huart1.hdmarx->Instance->NDTR))% RS232_RECEIVE_SIZE)
void rs232_pickup_packet(void)
{
    uint8_t handled_buff[300];
    uint16_t number = 0;
    uint8_t * temp_p;
    uint8_t sum =0;
    uint16_t tmp_handl_p = 0xffff; ///< 为防止假包暂存Rs232_handle_p
    //处理rs232 接收循环缓存
    for(; rs232_top_p != Rs232_handle_p;)
    {
        if(Rs232_receive_buff[Rs232_handle_p] == 0x55)
        {
            if((Rs232_handle_p + 1)%RS232_RECEIVE_SIZE != rs232_top_p)      //数据段n不在处理指针的前面,在，说明没有准备好数据                                        //数据段n就在此buff中
            {
                number = Rs232_receive_buff[(Rs232_handle_p+2)%RS232_RECEIVE_SIZE];
                if((rs232_top_p - Rs232_handle_p)% RS232_RECEIVE_SIZE > number +4)                                //整个数据包都在此buff中
                {
                    //判断数据包是否有效
                    if(Rs232_receive_buff[(Rs232_handle_p+number+4)%RS232_RECEIVE_SIZE] == 0xaa)      //包尾有效
                    {

                        for(uint16_t i = 0; i< number +5; i++)                      //checksum 计算
                        {
                            temp_p = Rs232_receive_buff +Rs232_handle_p +i;
                            if(temp_p>=Rs232_receive_buff+RS232_RECEIVE_SIZE)
                            {
                                temp_p = temp_p - RS232_RECEIVE_SIZE;
                            }
                            sum += *temp_p;
                        }
                        if(sum ==0)             //checksum验证成功，数据包最终有效
                        {
                            for(uint16_t i= 0; i< number+5 ; i++ )
                            {
                                handled_buff[i] = Rs232_receive_buff[(Rs232_handle_p +i)% RS232_RECEIVE_SIZE ];
                            }
                            packet_receive_handle(handled_buff,number+5,RS232_CHANNEL);                //交给数据处理
                            Rs232_handle_p = (Rs232_handle_p + number + 5)%RS232_RECEIVE_SIZE;
                            tmp_handl_p = 0xffff; ///< 怀疑假包后面有真包，坐实怀疑。0xffff是为了for后面恢复假包地址
                        }
                        else //checksum无效的话，处理指针+1
                        {
                            Rs232_handle_p = (1+Rs232_handle_p)%RS232_RECEIVE_SIZE;
                        }
                    }
                    else //包尾无效的话，处理指针+1
                    {
                        Rs232_handle_p = (1+Rs232_handle_p)%RS232_RECEIVE_SIZE;
                    }
                }
                else                                                    //整个数据包不全在此buff中，等待数据补充
                {
                    //要防止假包
                    if(tmp_handl_p == 0xffff)                           //之前没有进入怀疑假包状态
                    {
                        tmp_handl_p = Rs232_handle_p;
                        Rs232_handle_p = (1+Rs232_handle_p)%RS232_RECEIVE_SIZE;
                    }
                    else                                                //之前已经进入了怀疑假包状态
                    {
                        Rs232_handle_p = tmp_handl_p;                   //恢复怀疑假包的地址
                        tmp_handl_p = 0xffff;
                        break;                                          //退出循环体，补充数据
                    }
                }
            }
            else                                                        //数据段n不在此buff中，等待数据补充
            {
                break;
            }
        }
        else
        {
            Rs232_handle_p = (1+Rs232_handle_p)%RS232_RECEIVE_SIZE;
        }
    }
    if(tmp_handl_p != 0xffff) ///< 怀疑的假包后面没有真包
    {
        Rs232_handle_p = tmp_handl_p; ///< 恢复怀疑假包的地址
        tmp_handl_p = 0xffff;
    }

    //发送rs232缓存
    if(Rs232_send_p[Rs232_pipo])
    {
        if(HAL_UART_Transmit_DMA(&huart1,Rs232_send_buff[Rs232_pipo],Rs232_send_p[Rs232_pipo%2]) == HAL_OK)
        {
            Rs232_pipo = (Rs232_pipo+1)%2;
            Rs232_send_p[0] = 0;
            Rs232_send_p[1] = 0;
        }
    }


}
