/** **************************************************************************************
 * @file
 * @brief           PWM灯光控制
 * @note			增加了ABC版本的区别代码
 * @author          zhangjiayi
 * @date            2023-04-04 12:03:23
 * @version         v1.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-04-04 12:53:17
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/MBI6024.c
 *****************************************************************************************/
#include "MBI6024.h"

//用户定义区
type_mbi6024 Mbi6024_in;
type_mbi6024 Mbi6024_out;
uint8_t MbiIn_sendBuff[6+MBI6024IN_NUM*24];				        //最终要发送到MIB6024的数据包。外部程序不用理会
uint16_t MbiIn_UserGrayBuff[MBI6024IN_NUM * 12];		        //用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
uint8_t MbiIn_UserCorrectBuff[MBI6024IN_NUM * 12];		        //用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。

uint8_t MbiOut_sendBuff[6+MBI6024OUT_NUM*24];				    //最终要发送到MIB6024的数据包。外部程序不用理会
uint16_t MbiOut_UserGrayBuff[MBI6024OUT_NUM * 12];	            //用户要显示的gray scale数据。数组[0],靠近MCU第一的MBI6024的第一通道。
uint8_t MbiOut_UserCorrectBuff[MBI6024OUT_NUM * 12];	        //用户要显示的dot correct数据。数组[0],靠近MCU第一的MBI6024的第一通道。


//以下不需更改
#define BRIGHT_DIVISOR  2  //总亮除数
//six command
#define GRAY_SCALE_16     0x3f
#define GRAY_SCALE_10     0x2b
#define DOT_CORRECTION_8  0x33
#define DOT_CORRECTION_6  0x27
#define CONFIGURATION_16  0X23
#define CONFIGURATION_10  0X37


#ifdef HARDWARE_A
#define CONFIG_register1 ( parity_check                     << 0 |\
											     Phase_inversed                   << 1 |\
											     PWM_date_synchronization         << 3 |\
											     PWN_counter_reset                << 4 |\
											     GCLK_source                      << 5 |\
											     GCLK_frequency                   << 8 )
#define CONFIG_register2 (0x0004 | CKI_time_out_period)
#else
#define CONFIG_register1 ( parity_check                     << 0 |\
											     Phase_inversed                   << 1 |\
											     PWM_date_synchronization         << 3 |\
											     PWN_counter_reset                << 4 |\
                                                 3                                << 5 |\
											     DOT_correction_mode              << 7 |\
											     GCLK_frequency                   << 8 )
#define CONFIG_register2 (0x0007)
#endif


/** **************************************************************************************
 * @brief 判断一个16位无符号变量的1的个数为偶数还是奇数
 * @note 有符号16位变量，不知道是否能得出正确答案，请慎用
 * @param temp：要判断的16位无符号变量
 * @return uint8_t
 * @retval 0:有偶数个1；1:有奇数个1
 *****************************************************************************************/
static uint8_t parity_even_bit(uint16_t temp)
{
    uint8_t number = 0;
    uint8_t ret = 0;
    while (temp) //计算出‘1’的个数
    {
        temp &= temp - 1;
        number++;
    }
    if (number & 0x01) //偶数个1，ret=0，否则=1，注意，number必须<=7
        ret = 1;
    else
        ret = 0;
    return ret;
}

/** **************************************************************************************
 * @brief 将一个实际有效位为size的uint16放在一个8位的数组队列
 * @note size不能大于15。队列的queque[0]的最高位为队列0位，将最先被处理
 * @param input:输入的数据：如xxx...x000b(高对齐),000x...xxxb(低对齐)
 * @param is_align_h: 1,xxx...x000b(高对齐)  0000x...xxxb(低对齐)
 * @param size: 实际有效位数，如3，xxx0000...00b或000...00xxxb
 * @param start_bit: 从队列的第几位开始拼装
 * @param *queque: 要拼装的队列
 * @return uint32_t 队列下次拼接的起始位
 * @retval
 *****************************************************************************************/
uint32_t assemble_nbitsToQueque(uint16_t input,uint8_t is_align_h,uint8_t size,uint32_t start_bit,uint8_t *queque)
{
    uint32_t temp = 0;;
    uint32_t set_zero = 0x00000000;
    if(is_align_h ==0)
    {
        input  = input << (16-size) ;  ///< 将低对齐转换为高对齐
    }
    temp =  input;
    temp = temp << 16;   						///< 将16位高对齐转换为32位高对齐

    for(uint8_t i = 0; i<size; i++) ///< 将有效位设为1
    {
        set_zero |= (0x80000000>>i);
    }

    temp     = temp     >> (start_bit%8); ///< 移到要拼接的位置
    set_zero = set_zero >> (start_bit%8); ///< 移到要拼接的位置
    set_zero = ~set_zero;                 ///< set_zero取反

    queque[start_bit/8 + 0] &= (uint8_t)(set_zero>>24);
    queque[start_bit/8 + 0] |= (uint8_t)(temp>>24);
    queque[start_bit/8 + 1] &= (uint8_t)(set_zero>>16);
    queque[start_bit/8 + 1] |= (uint8_t)(temp>>16);
    queque[start_bit/8 + 2] &= (uint8_t)(set_zero>>8);
    queque[start_bit/8 + 2] |= (uint8_t)(temp>>8);

    return ( start_bit + size);
}

/** **************************************************************************************
 * @brief 配置一个48位的包头
 * @note 用8位数组来表示。
 * @param command：包命令，见six command
 * @param mbi6024：mbi6024实例
 * @return uint32_t 队列下次拼接的起始位
 *****************************************************************************************/
uint32_t set_head_48bits(uint8_t command,type_mbi6024 * mbi6024)
{
    uint32_t start_bit = 0;
    uint8_t  H = command;
    uint8_t  P = 0;
    uint8_t  X = 0;
    uint16_t A = 0;
    uint16_t L = ((mbi6024->ics>1023) ? 1023:(mbi6024->ics-1));

    //set parity
    if (parity_even_bit(L))
        P |= 0x01;
    if (parity_even_bit(A))
        P |= 0x02;
    if (parity_even_bit(H))
        P |= 0x04;
    if (parity_even_bit(P))
        P |= 0x08;

    //48 bits header
    //H[5:0],A[9:0],H[5:0],L[9:0],P[3,0],X1[1:0],,L[9:0]
    start_bit =  assemble_nbitsToQueque(H,0,6,start_bit,mbi6024->send_buff);   ///< H[5:0]
    start_bit =  assemble_nbitsToQueque(A,0,10,start_bit,mbi6024->send_buff);  ///< A[9:0]
    start_bit =  assemble_nbitsToQueque(H,0,6,start_bit,mbi6024->send_buff);   ///< H[5:0]
    start_bit =  assemble_nbitsToQueque(L,0,10,start_bit,mbi6024->send_buff);  ///< L[9:0]
    start_bit =  assemble_nbitsToQueque(P,0,4,start_bit,mbi6024->send_buff);   ///< P[3:0]
    start_bit =  assemble_nbitsToQueque(X,0,2,start_bit,mbi6024->send_buff);   ///< X1[1:0]
    start_bit =  assemble_nbitsToQueque(L,0,10,start_bit,mbi6024->send_buff);  ///< L[9:0]

    return start_bit;
}

/** **************************************************************************************
 * @brief 配置一个30位的包头
 * @note 用8位数组来表示。
 * @param command：包命令，见six command
 * @param mbi6024：mbi6024实例
 * @return uint32_t 队列下次拼接的起始位
 *****************************************************************************************/
uint32_t set_head_30bits(uint8_t command,type_mbi6024 * mbi6024)
{
    uint32_t start_bit = 0;
    uint8_t  H = command;
    uint8_t  P = 0;
    uint16_t A = 0;
    uint16_t L = ((mbi6024->ics>1023) ? 1023:(mbi6024->ics-1));
    if (parity_even_bit(L))
        P |= 0x10;
    if (parity_even_bit(A))
        P |= 0x40;
    if (parity_even_bit(H))
        P |= 0x40;
    if (parity_even_bit(P))
        P |= 0x80;

    //30 bits header
    //H[5:0],P[3,0],A[9:0],L[9:0]
    start_bit =  assemble_nbitsToQueque(H,0,6,start_bit,mbi6024->send_buff);   ///< H[5:0]
    start_bit =  assemble_nbitsToQueque(P,0,4,start_bit,mbi6024->send_buff);   ///< P[3:0]
    start_bit =  assemble_nbitsToQueque(A,0,10,start_bit,mbi6024->send_buff);  ///< A[9:0]
    start_bit =  assemble_nbitsToQueque(L,0,10,start_bit,mbi6024->send_buff);  ///< L[9:0]

    return start_bit;
}


/** **************************************************************************************
 * @brief spi 发送
 * @note 此函数可以在工程的任意位置重写。新函数将自动覆盖此函数
 * @param mbi6024：mbi6024实例
 * @param send_bits: 要发送多少位（注意：不是字节）
 * @return uint32_t 队列下次拼接的起始位
 *****************************************************************************************/
void __weak mbi6024_send(type_mbi6024* mbi6024,uint16_t send_bits )
{
#ifdef _USE_SPI_DMA_
    HAL_SPI_Transmit_DMA(mbi6024->hspi, mbi6024->send_buff, (send_bits%8 ? send_bits/8+1:send_bits/8));
#else
    uint8_t temp;
    uint32_t i;
    for (i = 0; i < send_bits / 8; i++)
    {
        temp = (mbi6024->send_buff)[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            CLK_SET;
            if (temp & 0x80)
                DAT_SET;
            else
                DAT_CLR;
            CLK_CLR;
            temp <<= 1;
        }
    }
    temp = (mbi6024->send_buff)[i];
    for (uint8_t j = 0; j < send_bits % 8; j++)
    {
        CLK_SET;
        if (temp & 0x80)
            DAT_SET;
        else
            DAT_CLR;
        CLK_CLR;
        temp <<= 1;
    }
    CLK_CLR;
    DAT_CLR;
#endif


}

/** **************************************************************************************
 * @brief 设置16-bit configuration包
 * @note
 * @param mbi6024：mbi6024实例
 * @return uint32_t 队列下次拼接的起始位
 * @retval
 *****************************************************************************************/
uint32_t mbi6024_16bit_configuration(type_mbi6024* mbi6024)
{
    uint32_t start_bit = 0;
    //head
    start_bit = set_head_48bits(CONFIGURATION_16,mbi6024);

    //configure data
    //X2[5:0]~CF1n[9:0]~X2[5:0]~CF1n[9:0]~X3[12:0]~CF2n[2:0]
    for(uint16_t n = 0; n < mbi6024->ics; n++)
    {
        start_bit = assemble_nbitsToQueque(0x0000,0,6,start_bit,mbi6024->send_buff);             ///< X2[5:0
        start_bit = assemble_nbitsToQueque(CONFIG_register1,0,10,start_bit,mbi6024->send_buff);  ///< CF1n[9:0]
        start_bit = assemble_nbitsToQueque(0x0000,0,6,start_bit,mbi6024->send_buff);             ///< X2[6:0]
        start_bit = assemble_nbitsToQueque(CONFIG_register1,0,10,start_bit,mbi6024->send_buff);  ///< CF1n[9:0]
        start_bit = assemble_nbitsToQueque(0x0000,0,13,start_bit,mbi6024->send_buff);            ///< X3[6:0]
        start_bit = assemble_nbitsToQueque(CONFIG_register2,0,3,start_bit,mbi6024->send_buff);   ///< CF2n[9:0]
    }
    mbi6024_send(mbi6024,start_bit);
    return start_bit;
}

/** **************************************************************************************
 * @brief 设置10-bit configuration包
 * @note
 * @param mbi6024：mbi6024实例
 * @return uint32_t 队列下次拼接的起始位
 * @retval
 *****************************************************************************************/
uint32_t mbi6024_10bit_configuration(type_mbi6024* mbi6024)
{
    uint32_t start_bit = 0;
    //head
    set_head_30bits(CONFIGURATION_10,mbi6024);

    //configure data
    //CF1n[9:0]~CF1n[9:0]~X4[6:0]~CF2n[2:0]
    for(uint16_t n = 0; n < mbi6024->ics; n++)
    {
        start_bit = assemble_nbitsToQueque(CONFIG_register1,0,10,start_bit,mbi6024->send_buff);  ///< CF1n[9:0]
        start_bit = assemble_nbitsToQueque(CONFIG_register1,0,10,start_bit,mbi6024->send_buff);  ///< CF1n[9:0]
        start_bit = assemble_nbitsToQueque(0x0000,0,7,start_bit,mbi6024->send_buff);             ///< X4[6:0]
        start_bit = assemble_nbitsToQueque(CONFIG_register2,0,3,start_bit,mbi6024->send_buff);   ///< CF2n[9:0]
    }
    mbi6024_send(mbi6024,start_bit);
    return start_bit;
}

/** **************************************************************************************
 * @brief 设置并发送16big gray scale 包
 * @note gray scale 数组的最后一个数将最先发送。以实现最后一个数对应芯片最后一个通道
 * @param mbi6024：mbi6024实例
 * @return uint32_t 队列下次拼接的起始位
 * @retval
 *****************************************************************************************/
uint32_t mbi6024_16bit_gray_scale(type_mbi6024* mbi6024)
{
    uint32_t start_bit = 0;
    //head
    start_bit = set_head_48bits(GRAY_SCALE_16,mbi6024);
    //16bit_gray_scale
    //C3n[16:0]~B3n[16:0]~A3n[16:0]
    for(uint16_t n = 0; n < mbi6024->ics *12; n++)
    {
        start_bit = assemble_nbitsToQueque((mbi6024->gray_scale_data)[ mbi6024->ics *12-1-n]/BRIGHT_DIVISOR,0,16,start_bit,mbi6024->send_buff);
    }
    mbi6024_send(mbi6024,start_bit);
    return start_bit;
}

/** **************************************************************************************
 * @brief 设置并发送10big gray scale 包
 * @note gray scale 数组的最后一个数将最先发送。以实现最后一个数对应芯片最后一个通道.数据高对齐
 * @param mbi6024：mbi6024实例
 * @return uint32_t 队列下次拼接的起始位
 * @retval
 *****************************************************************************************/
uint32_t mbi6024_10bit_gray_scale(type_mbi6024* mbi6024)
{
    uint32_t start_bit = 0;
    //head
    start_bit = set_head_30bits(GRAY_SCALE_10,mbi6024);
    //10bit_gray_scale
    //C3n[10:0]~B3n[10:0]~A3n[10:0]
    for(uint16_t n = 0; n < mbi6024->ics *12; n++)
    {
        start_bit = assemble_nbitsToQueque((mbi6024->gray_scale_data)[ mbi6024->ics*12-1-n]/BRIGHT_DIVISOR,1,10,start_bit,mbi6024->send_buff);
    }
    mbi6024_send(mbi6024,start_bit);
    return start_bit;
}

/** **************************************************************************************
 * @brief 设置并发送8bit_dot_correct 包
 * @note 数组的最后一个数将最先发送。以实现最后一个数对应芯片最后一个通道
 * @param mbi6024：mbi6024实例
 * @return uint32_t 队列下次拼接的起始位
 * @retval
 *****************************************************************************************/
uint32_t mbi6024_8bit_dot_correct(type_mbi6024* mbi6024)
{
    uint32_t start_bit = 0;
    //head
    start_bit = set_head_48bits(DOT_CORRECTION_8,mbi6024);

    //8 bit dot correct data
    //C3n[7:0]~C3n[7:0]~B3n[5:0]~B3n[7:0]~A3n[7:0]~A3n[7:0]
    for(uint16_t n = 0; n < mbi6024->ics *12; n++)
    {
        start_bit = assemble_nbitsToQueque((uint16_t)(mbi6024->dot_correct_data)[ mbi6024->ics *12-1-n],0,8,start_bit,mbi6024->send_buff);    ///< 要两个相同的字节
        start_bit = assemble_nbitsToQueque((uint16_t)(mbi6024->dot_correct_data)[ mbi6024->ics *12-1-n],0,8,start_bit,mbi6024->send_buff);    ///< 要两个相同的字节
    }
    mbi6024_send(mbi6024,start_bit);
    return start_bit;
}

/** **************************************************************************************
 * @brief 设置并发送6bit_dot_correct 包
 * @note 数组的最后一个数将最先发送。以实现最后一个数对应芯片最后一个通道
 * @param mbi6024：mbi6024实例
 * @return uint32_t 队列下次拼接的起始位
 * @retval
 *****************************************************************************************/
uint32_t mbi6024_6bit_dot_correct(type_mbi6024* mbi6024)
{
    uint32_t start_bit = 0;
    //head
    start_bit = set_head_30bits(DOT_CORRECTION_6,mbi6024);

    //6 bit dot correct data
    //Z3[3:0]~C3n[5:0]~Z3[3:0]~B3n[5:0]~Z3[3:0]~A3n[5:0]
    for(uint16_t n = 0; n < mbi6024->ics *12; n++)
    {
        start_bit = assemble_nbitsToQueque(0x0000,0,4,start_bit,mbi6024->send_buff);                                 ///< Z[3:0]
        start_bit = assemble_nbitsToQueque((uint16_t)(mbi6024->dot_correct_data)[ mbi6024->ics *12-1-n],0,6,start_bit,mbi6024->send_buff);    ///< data
    }
    mbi6024_send(mbi6024,start_bit);
    return start_bit;
}

/** **************************************************************************************
 * @brief 初始化MBI6024的相应功能
 * @note 配置Mbi6024,并开启定时器
 * @param mbi6024：mbi6024实例
 * @return void
 * @retval
 *****************************************************************************************/
void mbi6024_init(type_mbi6024* mbi6024)
{
    mbi6024_10bit_configuration(mbi6024);
}
