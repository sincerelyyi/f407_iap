/** **************************************************************************************
 * @file
 * @brief           74HC165串行输入
 * @note            通过spi控制
 * @author          zhangjiayi
 * @date            2023-03-24 12:03:23
 * @version         v0.2 通过__USE_SPI__ 选择spi方式还是gpio方式
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-09-18 12:53:17
 * @LastEditors     zhangjiayi
 * @FilePath        /colortrain/myware/MBI6024.c
 *****************************************************************************************/
#include "74HC165.h"
#include "gpio.h"
#include "uartPrintf.h"
#include "packet_handle.h"
#include "string.h"
#include "command.h"
/** **************************************************************************************
* @brief 读取74HC165输入数据
* @note  in165_buff[0]是最高位
* @param
* @return uint8_t
* @retval 1成功，0失败
*****************************************************************************************/
uint8_t in165_buff[NUM_165];
uint8_t old165_buff[NUM_165];
uint8_t read_165(void)
{
    static uint8_t init = 0;

    if(init == 0)
    {
#if !__USE_SPI__

        HAL_SPI_MspDeInit(&SPI_165);

        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        HAL_GPIO_WritePin(SCK165_GPIO_Port,SCK165_Pin, GPIO_PIN_SET);
        GPIO_InitStruct.Pin = SCK165_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(SCK165_GPIO_Port, &GPIO_InitStruct);

        HAL_GPIO_WritePin(LD165_GPIO_Port,LD165_Pin, GPIO_PIN_SET);
        GPIO_InitStruct.Pin = LD165_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(LD165_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = DAT165_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(DAT165_GPIO_Port, &GPIO_InitStruct);
#endif
        init = 1;
    }
#if __USE_SPI__
    HAL_GPIO_WritePin(LD165_GPIO_Port,LD165_Pin,GPIO_PIN_RESET);
    __NOP();
    __NOP();
    HAL_GPIO_WritePin(LD165_GPIO_Port,LD165_Pin,GPIO_PIN_SET);  //切换为串行，为read_165()准备
    __NOP();
    __NOP();
    if(memcmp(in165_buff,old165_buff,NUM_165))  //按键值有改变
    {
        memcpy(old165_buff,in165_buff,NUM_165);
        active_bin(slave_out_keys,old165_buff,NUM_165,Comunucate_channel);
    }
    if(HAL_SPI_GetState(&SPI_165) == HAL_SPI_STATE_READY)
    {
        HAL_SPI_Receive_DMA(&SPI_165, in165_buff, NUM_165);
        return 1;
    }
    return 0;
#else
    uint8_t temp[2][NUM_165]= {0};
    uint8_t diff[NUM_165] = {0};   ///< 两次检测每一位的异或值
    for(uint8_t i= 0; i< 2; i++)
    {
        HAL_GPIO_WritePin(LD165_GPIO_Port,LD165_Pin,GPIO_PIN_RESET);  //切换为并行
        for(uint8_t nop = 0; nop<1; nop++) __NOP();
        HAL_GPIO_WritePin(LD165_GPIO_Port,LD165_Pin,GPIO_PIN_SET);  //切换为串行，为read_165()准备
        __NOP();
        for(uint8_t j= 0; j< NUM_165; j++)
        {
            temp[i][j] = 0;
            for(uint8_t k= 0; k< 8; k++)
            {
                HAL_GPIO_WritePin(SCK165_GPIO_Port, SCK165_Pin, GPIO_PIN_RESET);
                if(HAL_GPIO_ReadPin(DAT165_GPIO_Port, DAT165_Pin))
                {
                    temp[i][j] |=  (0x80>>k);
                }
                HAL_GPIO_WritePin(SCK165_GPIO_Port, SCK165_Pin, GPIO_PIN_SET);
                __NOP();
                __NOP();
            }
        }
    }
    //判断每一位是否相同，相同的话，更新，不相同的化用旧的数据
    for(uint8_t i= 0; i<NUM_165; i++)
    {
        diff[i] = temp[0][i]^temp[1][i];
        in165_buff[i] = 0;
        for(uint8_t j = 0; j < 8; j++)
        {
            (diff[i] & 0x01<<j)?(in165_buff[i] |= old165_buff[i] & 0x01<<j):(in165_buff[i] |= temp[0][i] & 0x01<<j);
        }
    }
    if(memcmp(in165_buff,old165_buff,NUM_165))  //按键值有改变
    {
        memcpy(old165_buff,in165_buff,NUM_165);
        active_bin(slave_out_keys,old165_buff,NUM_165,Comunucate_channel);
    }
    return 1;
#endif
}

//void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//    if(hspi == &SPI_165)
//        HAL_GPIO_WritePin(LD165_GPIO_Port,LD165_Pin,GPIO_PIN_RESET);
//}
