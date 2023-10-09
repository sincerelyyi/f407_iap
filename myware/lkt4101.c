/** **************************************************************************************
 * @file
 * @brief           LKT4101
 * @note            通过串口控制
 * @author          zhangjiayi
 * @date            2022-11-09 12:03:23
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2022-11-09 12:53:17
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/MBI6024.c
 *****************************************************************************************/
#include "main.h"
#include "string.h"
#include "usart.h"
#include "stm32f4xx_hal_uart.h"
#include "rng.h"
#include "stdio.h"
#include "uartPrintf.h"
#include "lkt4101.h"
#define DAT_LINE  5
UART_HandleTypeDef  *Lkt_huart = &huart6;
LktTypeDef Lkt;
uint8_t receive_buff[32];
_Random rd = {15, {0}};                             ///< 定义随机数组
_DOG dog = {0,  0, 0, {0}, {0}, 0, 0, 0};
#define top_p    ((LKT_FIFO_SIZE  - (huart6.hdmarx->Instance->NDTR))%LKT_FIFO_SIZE) ///< 接收数据的当前指针
/** **************************************************************************************
 * @brief 将复位脚拉低
 * @note
 *****************************************************************************************/
static void reset(void)
{
    HAL_GPIO_WritePin(DOG_RST_GPIO_Port,DOG_RST_Pin, GPIO_PIN_RESET);  ///< pull reset down
    HAL_UART_AbortTransmit(Lkt_huart);
    Lkt.status = lkt_reset;
}

/** **************************************************************************************
 * @brief 将复位脚拉高
 * @note
 *****************************************************************************************/
static void unreset(void)
{
    HAL_GPIO_WritePin(DOG_RST_GPIO_Port,DOG_RST_Pin, GPIO_PIN_SET);     ///< pull reset up
    Lkt.send_cplt_tick = HAL_GetTick();
    Lkt.status  = lkt_receive;                                          ///< 复位会有数据回传
    Lkt.handle_p = Lkt.circle_p = top_p;
}


/** **************************************************************************************
 * @brief 发送数据到加密芯片
 * @note
 * @param buff,要发送的数据缓存
 * @param len, 要发送的数据长度
 * @param need_receive_size, 期望接收的字节数
 * @return HAL_StatusTypeDef
 * @retval HAL_OK，正常，其他，错误
 *****************************************************************************************/
static HAL_StatusTypeDef send(uint8_t *buff,uint8_t len)
{
    if(Lkt.status == lkt_idle)
    {
        memcpy(Lkt.send_buff,buff,len);
        if(HAL_UART_Transmit_DMA(Lkt_huart,Lkt.send_buff, len) == HAL_OK)
        {
            Lkt.status = lkt_send;
            return HAL_OK;
        }
    }
    return HAL_BUSY;
}

/** **************************************************************************************
 * @brief 监视接收数据
 * @note
 *****************************************************************************************/
static void receive_monitor(void)
{
    HAL_UART_Receive_DMA(Lkt_huart, Lkt.receive_circle, LKT_FIFO_SIZE);
    if(Lkt.status == lkt_receive)
    {
        if(Lkt.circle_p == top_p)   ///< 没有新数据
        {
            if(HAL_GetTick() - Lkt.send_cplt_tick > (uint32_t) 2*12 *1000 /Lkt_huart->Init.BaudRate + 1000)
            {
                Lkt.status = lkt_timeout;
                return;
            }
        }
        else                        ///< 有新数据
        {
            Lkt.circle_p = top_p;
            Lkt.new_data_tick = HAL_GetTick();
            Lkt.status = lkt_newdate;
        }
    }
    if(Lkt.status == lkt_newdate)
    {
        if(HAL_GetTick() - Lkt.new_data_tick > (uint32_t) 2*12 *1000 /Lkt_huart->Init.BaudRate + 1)
        {
            if(Lkt.circle_p == top_p)       ///< 没有新数据，说明传输完成
            {
                Lkt.new_len = (uint8_t)(Lkt.circle_p - Lkt.handle_p)%LKT_FIFO_SIZE;
                for(uint8_t i = 0; i < Lkt.new_len  ; i ++)
                {
                    Lkt.receive_buff[i] = Lkt.receive_circle[(Lkt.handle_p)%LKT_FIFO_SIZE];
                    Lkt.handle_p = (Lkt.handle_p+1)%LKT_FIFO_SIZE;
                    Lkt.status = lkt_idle;
                }
            }
            else                          ///< 有新数据
            {
                Lkt.circle_p = top_p;
                Lkt.new_data_tick = HAL_GetTick();
            }
        }
    }
}

/** **************************************************************************************
 * @brief 非阻塞接收函数
 * @note 当status为lkt_idle是，复制Lkt.receive_buff到buff：
 *****************************************************************************************/
static LktEnumDef  receive(uint8_t *buff,uint8_t len)
{
    receive_monitor();
    if(Lkt.status == lkt_idle)
    {
        memcpy(buff,Lkt.receive_buff,len);
    }
    return Lkt.status;
}

/** **************************************************************************************
 * @brief 阻塞接收函数
 * @note
 *****************************************************************************************/
static void  block_receive(uint8_t *buff,uint8_t len)
{
    receive_monitor();
    for(receive_monitor(); (Lkt.status == lkt_receive) || (Lkt.status == lkt_send ||Lkt.status == lkt_newdate ); receive_monitor());
    if(Lkt.status == lkt_idle)
    {
        memcpy(buff,Lkt.receive_buff,len);
    }
}
/** **************************************************************************************
 * @brief 初始化加密芯片结构体
 * @note
 *****************************************************************************************/
void Lkt_init(void)
{
    memset(Lkt.send_buff,0,LKT_SEND_SIZE);              ///< 发送动lkt芯片数据的缓存
    memset(Lkt.receive_buff,0,LKT_RECEIVE_SIZE);        ///< 从lkt芯片接收有效数据的缓存
    memset(Lkt.receive_circle,0,LKT_RECEIVE_SIZE);      ///< 从lkt芯片接收数据的循环缓存
    Lkt.circle_p = 0;                                   ///< 最后接收到的字节在circle_buff的位置
    Lkt.handle_p = 0;                                   ///< 最后处理的的字节在circle_buff的位置
    Lkt.send_cplt_tick = 0;                             ///< 记录发送完成时的tick值
    Lkt.status = lkt_receive;                           ///< 当前状态
    Lkt.reset = reset;                                  ///< 复位函数
    Lkt.unreset = unreset;                              ///< 反复位函数
    Lkt.receive_monitor = receive_monitor;              ///< 监控接收的数据
    Lkt.send = send;                                    ///< 发送函数
    Lkt.block_receive = block_receive;                  ///< 阻塞接收函数
}

/** **************************************************************************************
 * @brief 初始化加密芯片结构体
 * @note
 *****************************************************************************************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == Lkt_huart)
    {
        Lkt.handle_p = Lkt.circle_p = top_p;
        Lkt.send_cplt_tick  = HAL_GetTick();
        Lkt.status = lkt_receive;
    }
}

/** **************************************************************************************
 * @brief 校验下载口令的指令
 * @note  后8个字节位下载口令
 *****************************************************************************************/
const uint8_t downPassword[5+8] = {
    0XF0,0XF6,0X00,0X00,0X08,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00
};

/** **************************************************************************************
 * @brief 发送烧录代码的指令
 * @note
 *****************************************************************************************/
const uint8_t dat[DAT_LINE][5+64] = {
    {
        0XF0,0XF4,0X40,0X00,0X40,
        0X8E,0X28,0X8F,0X29,0X8C,0X2A,0X8D,0X2B,
        0XE4,0XFE,0XF5,0X2C,0XF5,0X2D,0XE5,0X29,
        0X24,0X05,0XF5,0X82,0XE4,0X35,0X28,0XF5,
        0X83,0XE0,0X24,0X06,0XFD,0XE4,0X33,0XFC,
        0XE5,0X29,0X2D,0XF5,0X82,0XE5,0X28,0X3C,
        0XF5,0X83,0XE0,0X64,0X01,0X60,0X03,0X02,
        0X40,0XEF,0XE5,0X29,0X24,0X06,0XF5,0X82,
        0XE4,0X35,0X28,0XF5,0X83,0XE0,0X24,0X06
    },
    {
        0XF0,0XF4,0X40,0X40,0X40,
        0XFF,0XEE,0X33,0XFE,0XE5,0X29,0X2F,0XF5,
        0X82,0XE5,0X28,0X3E,0XF5,0X83,0XE0,0XF4,
        0X54,0X0F,0XF5,0X2C,0X7E,0X01,0X85,0X29,
        0X82,0X85,0X28,0X83,0XA3,0XE0,0X25,0X29,
        0XF5,0X82,0XE4,0X35,0X28,0XF5,0X83,0XE0,
        0XFF,0XEE,0XC3,0X9F,0X50,0X12,0XE5,0X29,
        0X2E,0XF5,0X82,0XE4,0X35,0X28,0XF5,0X83,
        0XE0,0X25,0X2D,0XF5,0X2D,0X0E,0X80,0XD6,
    },
    {
        0XF0,0XF4,0X40,0X80,0X40,
        0XE5,0X2D,0XD3,0X94,0X40,0X40,0X05,0X75,
        0X2D,0X40,0X80,0X0A,0XE5,0X2D,0XC3,0X94,
        0X0F,0X50,0X03,0X75,0X2D,0X0F,0XAD,0X2D,
        0XAF,0X2B,0XAE,0X2A,0X12,0X40,0XF2,0X7E,
        0X01,0XEE,0XC3,0X95,0X2D,0X50,0X23,0XE5,
        0X2B,0X2E,0XF5,0X82,0XE4,0X35,0X2A,0XF5,
        0X83,0XE0,0X75,0XF0,0X0A,0X84,0XE5,0XF0,
        0X54,0X0F,0XFF,0XE5,0X2B,0X2E,0XF5,0X82
    },
    {
        0XF0,0XF4,0X40,0XC0,0X40,
        0XE4,0X35,0X2A,0XF5,0X83,0XEF,0XF0,0X0E,
        0X80,0XD7,0XE5,0X2C,0X75,0XF0,0X0A,0X84,
        0XE5,0X2B,0X24,0X05,0XF5,0X82,0XE4,0X35,
        0X2A,0XF5,0X83,0XE0,0X24,0X06,0XFD,0XE4,
        0X33,0XFC,0XE5,0X2B,0X2D,0XF5,0X82,0XE5,
        0X2A,0X3C,0XF5,0X83,0XE5,0XF0,0XF0,0X7F,
        0X01,0X22,0X8E,0X2E,0X8F,0X2F,0X8D,0X30,
        0XEF,0X24,0X01,0XFF,0XE4,0X3E,0XFE,0X12
    },
    {
        0XF0,0XF4,0X41,0X00,0X20,
        0X10,0X20,0X85,0X2F,0X82,0X85,0X2E,0X83,
        0XE5,0X30,0XF0,0X22,0X7E,0X04,0X7F,0X80,
        0X7C,0X04,0X7D,0X81,0X12,0X40,0X00,0X80,
        0XFE,0X75,0X81,0X30,0X02,0X41,0X0C,0X00
    }
};
/** **************************************************************************************
 * @brief 获取算法压缩码的指令
 * @note  要先将烧录代码全部都发送到加密芯片，此指令才能正常
 *****************************************************************************************/
const  uint8_t compress_code[5] = {
    0XF0,0XF6,0X00,0X01,0X08
};

/** **************************************************************************************
 * @brief 验证算法签名
 * @note  后8个字节为：签名=DES(8字节全0算法签名密钥,算法压缩码)。
 *****************************************************************************************/
const uint8_t regist[5+8] = {
    0XF0,0XF6,0X00,0X02,0X08,
    0X82,0XB9,0X26,0X88,0XF6,0XB0,0XF6,0X1E
};

/** **************************************************************************************
 * @brief 临时速波特率
 * @note
 * @param
 * @return int
 * @retval 1，正常，0错误
 *****************************************************************************************/
int lkt4101_upbaud(void)
{
    uint32_t send_buff;
    switch (LKT_PPS)
    {
    case 16:
        send_buff = 0x799610FF;
        break;
    case 32:
        send_buff = 0x7A9510FF;
        break;
    case 64:
        send_buff = 0x7B9410FF;
        break;
    default:
        return 0;
    }

    Lkt.send((uint8_t *) (&send_buff), 4);
    block_receive((uint8_t *) (receive_buff), 4);
    if(receive_buff[0] == 0x83 || receive_buff[1] == 0x10 )
    {

        Lkt_huart->Init.BaudRate = LKT_OUT_FRE/LKT_PPS;
#ifdef _HalfDuplex_
        while(HAL_HalfDuplex_Init(Lkt_huart) != HAL_OK);
#else
        HAL_UART_Init(Lkt_huart);
#endif
        debug_printf("lkt4101 switch baud successed!\n");
        debug_printf("recieve:%2.2x%2.2x%2.2x%2.2x\n",receive_buff[0],receive_buff[1],receive_buff[2],receive_buff[3]);
        debug_printf("lkt4101'baud now is %d\n",Lkt_huart->Init.BaudRate);
        return 1;
    }
    else
    {
        debug_printf("lkt4101 switch baud fault!\n");
        debug_printf("err:%2.2x%2.2x%2.2x%2.2x\n",receive_buff[0],receive_buff[1],receive_buff[2],receive_buff[3]);
        return 0;
    }
}

/** **************************************************************************************
 * @brief 复位加密芯片，并试图临时提速波特率
 * @note  正确复位回传17个字节。其中后8个字节为加密芯片的唯一识别码
 * @param
 * @return int
 * @retval 1，正常，0错误
 *****************************************************************************************/
int lkt4101_reset(void)
{
    debug_printf("LKT4101 resetting...\n");
    //复位波特率为默认值
    Lkt_huart->Init.BaudRate = LKT_OUT_FRE/372;
#ifdef _HalfDuplex_
    while(HAL_HalfDuplex_Init(Lkt_huart) != HAL_OK);
#else
    HAL_UART_Init(Lkt_huart);
#endif
    //block_receive( (uint8_t *)receive_buff, 17);
    Lkt.reset();//pull reset down
    HAL_Delay(5);
    Lkt.unreset(); //pull reset up
    block_receive( (uint8_t *)receive_buff, 17);

    if(receive_buff[0] != 0x3b)
    {
        debug_printf("can not find LTK4101!\n");
        return 0;
    }
    else
    {
        debug_printf("LTK4101 reset message is:");
        for(uint8_t i = 0; i < 9; i++)
        {
            debug_printf("%.2X",receive_buff[i]);
        }
        debug_printf("-");
        for(uint8_t i = 9; i < 17; i++)
        {
            debug_printf("%.2X",receive_buff[i]);
        }
        debug_printf("\n");
        //改变波特率
        lkt4101_upbaud();

        uint8_t s_buff[] = {0x00,0x84,0x00,0x00,0x08};
        Lkt.send(s_buff, 5);
        block_receive((uint8_t *) (receive_buff), 11);

        if(receive_buff[0] == 0x84)
        {
            debug_printf("rng successed!\n");
        }
        else
        {
            debug_printf("rng fault!\n");
        }
        debug_printf("rng return:%2.2x%2.2x%2.2x%2.2x\n",receive_buff[0],receive_buff[1],receive_buff[2],receive_buff[3]);
        return 1;
    }
}

/** **************************************************************************************
 * @brief 校验下载口令
 * @note  后8个字节位下载口令
 * @param
 * @return int
 * @retval 1，正常，0错误
 *****************************************************************************************/
int download_password(void)
{
    Lkt.send( (uint8_t *)downPassword, 5);
    block_receive((uint8_t *)receive_buff, 1);
    if(receive_buff[0] != downPassword[1])
    {
        debug_printf("error password head!It shoud return %.2X,but return %.2X.\n",downPassword[1],receive_buff[0]);
        return 0;
    }
    Lkt.send( (uint8_t *)(downPassword+5), 8);
    block_receive( (uint8_t *)receive_buff, 2);
    if(receive_buff[0] == 0x60 && receive_buff[1] == 0x90 && receive_buff[2] == 0x00 )
    {
        return 1;
    }
    else if(receive_buff[0] == 0x90 && receive_buff[1] == 0x00)
    {
        return 1;
    }
    else
    {
        debug_printf("password error!It shoud return 9000,but return %.2X%.2X%.2X.\n",receive_buff[0],receive_buff[1],receive_buff[2]);
        return 0;
    }
}

/** **************************************************************************************
 * @brief 发送烧录代码
 * @note  一个DAT_LINE有64个烧录字节
 * @param
 * @return int
 * @retval 1，正常，0错误
 *****************************************************************************************/
int send_dat(void)
{
    for(uint16_t i=0; i<DAT_LINE; i++)
    {
        Lkt.send( (uint8_t *)dat[i], 5);
        block_receive(receive_buff, 1);
        if(receive_buff[0] != dat[i][1])
        {
            debug_printf("error send data head line %d!It shoud return %.2X,but return %.2X.\n",i,dat[i][1],receive_buff[0]);
            return 0;
        }
        Lkt.send((uint8_t *)dat[i] + 5, dat[i][4]);
        block_receive((uint8_t *)receive_buff, 2);
        if(receive_buff[0] != 0x90 || receive_buff[1] != 0x00 )
        {
            debug_printf("error send data content line %d!It shoud return 9000,but return %.2X%.2X.\n",i,receive_buff[0],receive_buff[1]);
            return 0;
        }
    }
    return 1;
}

/** **************************************************************************************
 * @brief 获取算法压缩码
 * @note  用于计算验证算法签名，签名=DES(8字节全0算法签名密钥,算法压缩码)。
 * @param
 * @return int
 * @retval 1，正常，0错误
 *****************************************************************************************/
int get_compressCode(void)
{
    for(uint16_t i = 0; i< 17; i++)
        receive_buff[i] = 0xff;
    Lkt.send((uint8_t *)compress_code, 5);
    block_receive((uint8_t *)receive_buff, 11);
    if(receive_buff[0] != compress_code[1] || receive_buff[9] != 0x90 || receive_buff[10] != 0x00)
    {
        debug_printf("error get compress code!\n");
        return 0;
    }
    else
    {
        debug_printf("compress code is:");
        for(uint8_t i = 0; i< 8; i++)
        {
            debug_printf("%.2X",receive_buff[i+1]);
        }
        debug_printf("\n");

    }
    return 1;
}

/** **************************************************************************************
 * @brief 验证算法签名
* @note  相当于注册。不注册的烧录代码无法正常运行
 * @param
 * @return int
 * @retval 1，正常，0错误
 *****************************************************************************************/
int regist_hardware(void)
{
    Lkt.send((uint8_t *)regist, 5);
    block_receive( (uint8_t *)receive_buff, 1);
    if(receive_buff[0] != regist[1])
    {
        debug_printf("error regist head!It shoud return %.2X,but return %.2X.\n",regist[1],receive_buff[0]);
        return 0;
    }
    Lkt.send( (uint8_t *)regist+5, 8);
    block_receive((uint8_t *)receive_buff, 2);
    if(receive_buff[0] != 0x90 || receive_buff[1] != 0x00 )
    {
        debug_printf("error regist data!It shoud return 9000,but return %.2X%.2X.\n",receive_buff[0],receive_buff[1]);
        return 0;
    }
    return 1;
}


/** **************************************************************************************
 * @brief 烧录代码到加密芯片
 * @note
 * @param
 * @return int
 * @retval 1，正常，0错误
 *****************************************************************************************/
int lkt4101_burn(void)
{

    debug_printf("prepare for burnning LKT4101...\n");
    if(lkt4101_reset())
    {
        debug_printf("begin to burn...\n");
        if(download_password())
        {
            if(send_dat())
            {
                get_compressCode();
                if(regist_hardware())
                {
                    debug_printf("LKT4101 had be burned successfully.\n");
                }
            }
        }
        return 1;
    }
    else
        return 0;
}


/** **************************************************************************************
* @brief 产生随机数
* @note
* @param 随机数组结构体
* @return int
* @retval 1，正常，0错误
*****************************************************************************************/
static void Random_number_begin(_Random  *p)
{
    for(uint8_t i=0; i<p->LEN; i++)                             ///< 产生Random_Structure->LEN个随机数，并存入数组
    {
        while(HAL_RNG_GetState(&hrng) == HAL_RNG_STATE_RESET);  ///< 等待随机数准备完毕
        p->Buffer[i]=HAL_RNG_GetRandomNumber(&hrng)%10;
    }
}

/** **************************************************************************************
* @brief LED根据校验结果闪动
* @note
*****************************************************************************************/
static void DOG_LED_display(void)
{
    static uint16_t ledTimer = 0;
    if (dog.LED_flag == 1)
    {
        if ((++ledTimer) >= 10)
        {
            ledTimer = 0;
            HAL_GPIO_TogglePin(DOG_LED_GPIO_Port, DOG_LED_Pin);
        }
    }
    else
    {
        HAL_GPIO_TogglePin(DOG_LED_GPIO_Port, DOG_LED_Pin);
    }
}

/** **************************************************************************************
* @brief 加密狗识别
* @note
*****************************************************************************************/
static void DOG_Identify(void)
{
    uint8_t i;
    static uint16_t DogTimer = 0;
    static uint16_t tick_10us=0;
    DogTimer++;
    tick_10us++;
    switch (dog.move)
    {
    case RST:
        if (tick_10us <= 10)
        {
            Lkt.reset();
            if(tick_10us == 1)
            {
                //复位波特率为默认值
                Lkt_huart->Init.BaudRate = LKT_OUT_FRE/372;
#ifdef _HalfDuplex_
                for(uint8_t i = 0; HAL_HalfDuplex_Init(Lkt_huart) != HAL_OK; i++)
                {
                    if(i>100)// error,初始化加密芯片串口错误
                    {
                        HAL_Delay(1);
                    }
                }
#else
                while(HAL_UART_Init(Lkt_huart));
                for(uint8_t i = 0; HAL_UART_Init(Lkt_huart)!= HAL_OK; i++)
                {
                    if(i>100)// error,初始化加密芯片串口错误
                    {
                        HAL_Delay(1);
                    }
                }
#endif
            }
        }
        else
        {
            if(Lkt.status == lkt_reset)
            {
                Lkt.unreset();
            }
            if(receive(Lkt.uid, 17) == lkt_idle)//复位成功
            {
                if(Lkt.uid[0] != 0x3b || Lkt.uid[1] != 0x6d)
                {
                    // errot, 加密芯片uid错误
                    tick_10us = 0;
                    dog.LED_flag = 0;               //快闪
                    dog.Check_err_flag++ ;          //检验不合格
                    dog.move = RST;
                    memset(dog.RXBuff,0,sizeof(dog.RXBuff));
                }
                else//成功获取uid,复位结束
                {
                    dog.move = CMD_H;
                }
            }
            else if(receive(Lkt.uid, 17) == lkt_timeout)//复位失败
            {
                // errot,复位失败
                tick_10us = 0;
                dog.LED_flag = 0;               //快闪
                dog.Check_err_flag++ ;          //检验不合格
                dog.move = RST;
                memset(dog.RXBuff,0,sizeof(dog.RXBuff));
            }
            else //等待接收复位数据
            {
                ;
            }
            //临时提高波特率
            //lkt4101_upbaud();
        }
        break;
    case CMD_H:
        dog.SendBuff[0] = 0xa3;                          //帧头
        dog.SendBuff[1] = rd.LEN;                        //发送出去的数据长度
        Random_number_begin(&rd);                        //产生随机数
        rd.Buffer[rd.Buffer[4] + 5] = 0x01;              //把随机数的04位置的值加上5，作为放入算法函数序号位置
        dog.Check_Val_Ago = rd.Buffer[rd.Buffer[5] + 5]; //把随机数的05位置的值加上5，该位置的值作为校验值
        for (i = 2; i < rd.LEN + 2; i++)                  //把随机数存入发送数组，0号位置=帧头，1号位置=总数据长度
        {
            dog.SendBuff[i] = rd.Buffer[i - 2];
        }
        Lkt.send(dog.SendBuff, rd.LEN + 2);
        dog.move = DOG_Wait;
        DogTimer = 0; //保证300ms后才重新复位
        break;
    case DOG_Wait:
        receive_monitor();

        receive(dog.RXBuff,dog.Fact_RX_LEN);
        if(Lkt.status == lkt_timeout)
        {
            dog.Check_err_flag++ ;  //检验不合格
            dog.LED_flag = 0;      //快闪
            dog.move = CMD_H;      //无论校验合不合格，重新发送
        }
        if(Lkt.status == lkt_idle)
        {
            dog.Fact_RX_LEN = Lkt.new_len;
            receive(dog.RXBuff,dog.Fact_RX_LEN);
            if ((dog.RXBuff[0] == 0xa3) && (dog.RXBuff[dog.Fact_RX_LEN - 2] == 0x90))  //头一个必须为0xA3，同时倒数第二个为0x90
            {
                dog.Check_Val_After = dog.RXBuff[dog.RXBuff[6] + 7]; //读出校验值
                rd.LEN = 0;
                for (i = 2; i < dog.RXBuff[dog.RXBuff[2]]; i++) //求下次要发送的随机数的个数
                    rd.LEN = rd.LEN + dog.RXBuff[i];
                if (rd.LEN > 64) // 随机数的个数在15~64之间
                    rd.LEN = 64;
                else if (rd.LEN < 15)
                    rd.LEN = 15;
                if (((dog.Check_Val_Ago ^ 0xff) % 10) == dog.Check_Val_After) //发送与接收下来的校验值进行比较
                {
                    dog.Check_err_flag = 0; //检验合格
                    dog.LED_flag = 1;      //指示灯慢闪开启
                }
                else
                {
                    dog.Check_err_flag++ ;  //检验不合格
                    dog.LED_flag = 0;      //快闪

                }
            }
            else
            {
                dog.Check_err_flag++ ;  //检验不合格
                dog.LED_flag = 0;       //快闪
            }
            dog.move = CMD_H;               //无论校验合不合格，重新发送
        }
        if (dog.Check_err_flag >= 1)       //错误次数超过一定数量时
        {
            tick_10us = 0;
            dog.LED_flag = 0;               //快闪
            dog.Check_err_flag++ ;          //检验不合格
            dog.move = RST;
            memset(dog.RXBuff,0,sizeof(dog.RXBuff));
        }
        break;
    default:
        break;
    }
}
/*********************************************************************************************************
**函数名称:GetSoftDogFlag
**函数功能：获取加密狗是否正确
**入口参数:无
**返回值: 1为正确,0为错误
**注释:
**********************************************************************************************************/
uint16_t GetSoftDogFlag(void)
{
    return dog.Check_err_flag;
}
/** **************************************************************************************
* @brief lkt_identify
* @note 放入循环体
* @param ms: 每过ms毫秒翻转一次
* @return void
* @retval
*****************************************************************************************/
void lkt_identify(uint32_t ms)
{
    static uint32_t test_count = 0;
    static uint8_t flag_init = 0;
    if(flag_init == 0)
    {
        flag_init =1;
    }

    if(HAL_GetTick() - test_count > ms)
    {
        DOG_Identify();
        test_count =  HAL_GetTick();
    }
}

/** **************************************************************************************
* @brief lkt_led
* @note 放入循环体
* @param ms: 每过ms毫秒翻转一次
* @return void
* @retval
*****************************************************************************************/
void lkt_led(uint32_t ms)
{
    static uint32_t test_count = 0;

    if(HAL_GetTick() - test_count > ms)
    {
        DOG_LED_display();
        test_count =  HAL_GetTick();
    }
}
