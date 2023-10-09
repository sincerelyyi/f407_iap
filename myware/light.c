/** **************************************************************************************
 * @file
 * @brief			灯光闪动的实现
 * @note            通过扫描方式
 * @author          zhangjiayi
 * @date            2023-04-11 09:04:47
 * @version         v0.1
 * @copyright       Copyright (c) 2020-2050  zhangjiayi
 * @par             LastEdit
 * @LastEditTime    2023-04-11 13:32:10
 * @LastEditors     zhangjiayi
 * @FilePath        /myware/light.c
 *****************************************************************************************/

#include "light.h"
#include "MBI6024.h"
#include "command.h"
#include "string.h"

CircleTypeDef P1circle,P2circle,P3circle,P4circle;
PwmTypeDef Pwm;
CircleTypeDef *Circle[4] = {&P1circle,&P2circle,&P3circle,&P4circle};

#define DIM_LIGHT 0x2000
/** **************************************************************************************
* @brief pwm 组初始化
* @note
* @param
* @return
* @retval
*****************************************************************************************/
static void pwm_init(void)
{
    Pwm.command = master_pwm_ll ;
    Pwm.pwm_data = MbiIn_UserGrayBuff;
    Pwm.all_state = 0;
    Pwm.old_all_state = Pwm.all_state;
    Pwm.all_state_num = 0;
    Pwm.channel_num = MBI6024IN_NUM * 12;
}

/** **************************************************************************************
* @brief P1circle组初始化
* @note
* @param
* @return
* @retval
*****************************************************************************************/
static void p1circle_init(void)
{
    P1circle.command = master_circle_ll;
    P1circle.pwm_data = MbiOut_UserGrayBuff;
    P1circle.all_state = 0;
    P1circle.old_all_state = P1circle.all_state;
    P1circle.all_state_num = 0;
    P1circle.channel_num = 26 * 3;
}

/** **************************************************************************************
* @brief P2circle组初始化
* @note
* @param
* @return
* @retval
*****************************************************************************************/
static void p2circle_init(void)
{
    P2circle.command = master_circle_ll ;
    P2circle.pwm_data = MbiOut_UserGrayBuff + 3*28;
    P2circle.all_state = 0;
    P2circle.old_all_state = P2circle.all_state;
    P2circle.all_state_num = 0;
    P2circle.channel_num = 26 * 3;
}

/** **************************************************************************************
* @brief P3circle组初始化
* @note
* @param
* @return
* @retval
*****************************************************************************************/
static void p3circle_init(void)
{
    P3circle.command = master_circle_ll ;
    P3circle.pwm_data = MbiOut_UserGrayBuff + 2*3*28;
    P3circle.all_state = 0;
    P3circle.old_all_state = P3circle.all_state;
    P3circle.all_state_num = 0;
    P3circle.channel_num = 26 * 3;
}

/** **************************************************************************************
* @brief P4circle组初始化
* @note
* @param
* @return
* @retval
*****************************************************************************************/
static void p4circle_init(void)
{
    P4circle.command = master_circle_ll ;
    P4circle.pwm_data = MbiOut_UserGrayBuff + 3*3*28;
    P4circle.all_state = 0;
    P4circle.old_all_state = P4circle.all_state;
    P4circle.all_state_num = 0;
    P4circle.channel_num= 26 * 3;
}

#define TICK 10 ///< 最小时间间隔为10ms
/** **************************************************************************************
* @brief  PWM灯组每条通道的样式实现
* @note 周期无限长的样式不在此函数实现
* @param *light: 灯组指针
* @param ch: 灯组通道号
* @param period: 灯的闪动周期
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @retval
*****************************************************************************************/
static void pwm_channel_pattern(PwmTypeDef *light,uint8_t ch,uint16_t period,uint32_t now)
{
    uint8_t ch_state = light->channel_state[ch];
    uint16_t *pwm_data = &light->pwm_data[ch];
    uint16_t ticks = period / TICK;
    uint16_t bright = (uint16_t)(ch_state & 0x0ff)<<12;
    uint32_t pwm_temp;

    if(ch_state&0x80)									///< on/off闪动
    {
        if(now%ticks < ticks/2)
        {
            *pwm_data = 0;

        }
        else
        {
            *pwm_data = bright;
        }
    }
    else												///< 呼吸闪动
    {
        if(now%ticks <= ticks/2)
        {
            pwm_temp = (uint32_t)bright*(now%ticks)*2/ticks;
            *pwm_data = (uint16_t)pwm_temp;
        }
        else
        {
            pwm_temp = (uint32_t)bright*(ticks-now%ticks)*2/ticks;
            *pwm_data = (uint16_t)pwm_temp;
        }
    }
}

/** **************************************************************************************
* @brief PWM逐个通道以1秒间隔流水
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void pwm_all_pattern_flow(PwmTypeDef *light,uint32_t now)
{
    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;
    }
    memset(light->pwm_data,0,light->channel_num*2);
    light->pwm_data[now/100%light->channel_num] = 0xffff;

}
/** **************************************************************************************
* @brief Pwm灯光闪动的实现
* @note
* @param
* @retval
*****************************************************************************************/
static void pwm_pattern(void)
{
    static uint32_t old_time;
    uint32_t now;
    now = HAL_GetTick()/TICK;
    if(now != old_time)
    {
        if(Pwm.isinit == 0)
        {
            pwm_init();
            Pwm.isinit = 1;
        }
        if(Pwm.command == master_pwm_ll)							///< ll底层控制
        {
            ;
        }
        else if(Pwm.command == master_pwm_channel)			///< channle 样式
        {
            for(uint8_t ch = 0; ch < Pwm.channel_num; ch++)
            {
                switch(Pwm.channel_state[ch]& 0x70)					///< 确定时间
                {
                case 0x00:														///< 无限长
                    Pwm.pwm_data[ch] = (uint16_t)(Pwm.channel_state[ch] & 0x0ff)<<12;
                    break;
                case 0x10:														///< 100ms
                    if(Pwm.count_100ms[ch] == 0 )
                    {
                        Pwm.count_100ms[ch] = 101;
                    }
                    else if ( Pwm.count_100ms[ch]==1)
                    {
                        Pwm.count_100ms[ch]=0;
                        Pwm.channel_state[ch] = 0x0f;//常亮
                    }
                    else
                    {
                        Pwm.count_100ms[ch]--;
                    }
                    pwm_channel_pattern(&Pwm, ch, 101,now);
                    break;
                case 0x20:														///< 200ms
                    pwm_channel_pattern(&Pwm, ch, 199,now);
                    break;
                case 0x30:														///< 500ms
                    pwm_channel_pattern(&Pwm, ch, 503,now);
                    break;
                case 0x40:														///< 1s
                    pwm_channel_pattern(&Pwm, ch, 997,now);
                    break;
                case 0x50:														///< 2s
                    pwm_channel_pattern(&Pwm, ch, 1999,now);
                    break;
                case 0x60:														///< 3s
                    pwm_channel_pattern(&Pwm, ch, 2999,now);
                    break;
                case 0x70:														///< 4s
                    pwm_channel_pattern(&Pwm, ch, 4001,now);
                    break;
                }
            }
        }
        else if(Pwm.command == master_pwm_all)					        ///< PWM整体样式
        {
            switch(Pwm.all_state)
            {
            case 0x00: ///< 逐个通道以1秒间隔流水
                pwm_all_pattern_flow(&Pwm, now);
                break;
            default:
                break;
            }
        }
        old_time = now;
    }
}
/** **************************************************************************************
* @brief  Circle灯组每条通道的样式实现
* @note 周期无限长的样式不在此函数实现
* @param *light: 灯组指针
* @param ch: 灯组通道号
* @param period: 灯的闪动周期
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @retval
*****************************************************************************************/
static void circle_channel_pattern(CircleTypeDef *light,uint8_t ch,uint16_t period,uint32_t now)
{
    uint8_t ch_state = light->channel_state[ch];
    uint16_t *pwm_data = &light->pwm_data[ch];
    uint16_t ticks = period / TICK;
    uint16_t bright = (uint16_t)(ch_state & 0x0ff)<<12;
    uint32_t pwm_temp;

    if(ch_state&0x80)									///< on/off闪动
    {
        if(now%ticks < ticks/2)
        {
            *pwm_data = 0;

        }
        else
        {
            *pwm_data = bright;
        }
    }
    else												///< 呼吸闪动
    {
        if(now%ticks <= ticks/2)
        {
            pwm_temp = (uint32_t)bright*(now%ticks)*2/ticks;
            *pwm_data = (uint16_t)pwm_temp;
        }
        else
        {
            pwm_temp = (uint32_t)bright*(ticks-now%ticks)*2/ticks;
            *pwm_data = (uint16_t)pwm_temp;
        }
    }
}


/** **************************************************************************************
* @brief circle逐个通道以1秒间隔流水
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_flow(CircleTypeDef *light,uint32_t now)
{
    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;
    }
    memset(light->pwm_data,0,light->channel_num*2);
    light->pwm_data[now/100%light->channel_num] = 0xffff;

}
/** **************************************************************************************
* @brief circle待机无操作
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_idle(CircleTypeDef *light,uint32_t now)
{
    const uint8_t speed = 3; //越大越慢
    const uint8_t speed_fast = 1; //越大越慢
    uint16_t red,green,blue;
    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;
        memset(light->pwm_data,0xff,light->channel_num*2);
    }
    switch (light->all_state_num)
    {
    case 0: ///< 白色常量3s
        if(now - light->old_tick >= 3000 / TICK)
        {
            light->old_tick = now;
            light->all_state_num++;
        }
        break;
    case 1: ///< 绿色一格一格顺序播放
        if(now - light->old_tick >= light->channel_num /3 * speed)
        {
            light->old_tick = now;
            light->all_state_num++;
        }
        else
        {
            red = 0x0000;
            green = 0xffff;
            blue = 0x0000;
            light->pwm_data[((now-light->old_tick)/speed*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed*3+2)%light->channel_num] = red;
        }
        break;
    case 2: ///< 蓝一格一格顺序播放
        if(now - light->old_tick >= light->channel_num /3 * speed)
        {
            light->old_tick = now;
            light->all_state_num++;
        }
        else
        {
            red = 0x0000;
            green = 0x0000;
            blue = 0xffff;
            light->pwm_data[((now-light->old_tick)/speed*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed*3+2)%light->channel_num] = red;

        }
        break;
    case 3: ///< 紫色一格一格顺序播放
        if(now - light->old_tick >= light->channel_num /3 * speed)
        {
            light->old_tick = now;
            light->all_state_num++;
        }
        else
        {
            red = 0xffff;
            green = 0x0000;
            blue = 0xffff;
            light->pwm_data[((now-light->old_tick)/speed*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed*3+2)%light->channel_num] = red;
        }
        break;
    case 4: ///< 橙色一格一格顺序播放
        if(now - light->old_tick >= light->channel_num /3 * speed)
        {
            light->old_tick = now;
            light->all_state_num++;

            red = 0xffff-0xffff/9*((now-light->old_tick)/speed_fast);
            green = 0xffff/9*((now-light->old_tick)/speed_fast);
            blue = 0x0000;
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else
        {
            red = 0xffff;
            green = 0x7fff;
            blue = 0x0000;
            light->pwm_data[((now-light->old_tick)/speed*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed*3+2)%light->channel_num] = red;
        }
        break;
    case 5: ///< 七彩快速铺满一遍
        if(now - light->old_tick >= light->channel_num /3 * speed_fast)
        {
            light->old_tick = now;
            light->all_state_num++;

            red = 0xffff;
            green = 0xffff;
            blue = 0xffff;
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else if(now - light->old_tick < 9 * speed_fast)
        {
            red = 0xffff-0xffff/9*((now-light->old_tick)/speed_fast);
            green = 0xffff/9*((now-light->old_tick)/speed_fast);
            blue = 0x0000;
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else if(now - light->old_tick < 18 * speed_fast)
        {
            red = 0x0000;
            green = 0xffff-0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
            blue = 0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else
        {
            red = 0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
            green = 0x0000;
            blue = 0xffff-0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        break;
    case 6: ///< 白色快速铺满一遍
        if(now - light->old_tick >= light->channel_num /3 * speed_fast)
        {
            light->old_tick = now;
            light->all_state_num++;
        }
        else
        {
            red = 0xffff;
            green = 0xffff;
            blue = 0xffff;
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        break;
    default:
        light->all_state_num = 0;
        light->old_all_state = light->all_state;
        light->old_tick = now;
        memset(light->pwm_data,0xff,light->channel_num*2);

    }

}

/** **************************************************************************************
* @brief Circle待机时拍任意键
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_idle_button(CircleTypeDef *light,uint32_t now)
{
    const uint8_t speed = 3; //越大越慢
    const uint8_t speed_fast = 1; //越大越慢
    uint16_t red,green,blue;
    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;
    }
    switch (light->all_state_num)
    {
    case 0: ///< 七彩快速铺满一遍
        if(now - light->old_tick >= light->channel_num /3 * speed_fast)
        {
            light->old_tick = now;
            light->all_state_num++;
            memcpy(light->pwm_tmp,light->pwm_data,light->channel_num*2);
        }
        else if(now - light->old_tick < 9 * speed_fast)
        {
            red = 0xffff-0xffff/9*((now-light->old_tick)/speed_fast);
            green = 0xffff/9*((now-light->old_tick)/speed_fast);
            blue = 0x0000;
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else if(now - light->old_tick < 18 * speed_fast)
        {
            red = 0x0000;
            green = 0xffff-0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
            blue = 0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else
        {
            red = 0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
            green = 0x0000;
            blue = 0xffff-0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        break;
    case 1: ///< 闪动3次
        if(now - light->old_tick >= 30*speed)
        {
            light->old_tick = now;
            light->all_state_num++;

        }
        else if(now - light->old_tick < 5*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 10*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
        else if(now - light->old_tick < 15*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 20*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
        else if(now - light->old_tick < 25*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else
        {
            memset(light->pwm_data,0xff,light->channel_num*2);
        }
        break;
    case 2: ///<白色20秒
        if(now - light->old_tick >= 20000/TICK)
        {
            light->old_tick = now;
            light->all_state_num++;
            light->all_state = 0x01;///<待机无操作

        }
        break;
    }
}

/** **************************************************************************************
* @brief circle读卡
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_read_card(CircleTypeDef *light,uint32_t now)
{
    const uint8_t speed = 3; //越大越慢
    const uint8_t speed_fast = 1; //越大越慢
    uint16_t red,green,blue;

    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;
    }
    switch (light->all_state_num)
    {
    case 0: ///< 特定颜色铺满一遍
        if(light->all_state != 0x07)
        {
            switch(light->all_state)
            {
            case 0x03: ///<读普通卡
                red = 0x0000;
                green = 0xffff;
                blue = 0x0000;
                break;
            case 0x04: ///<读稀有卡
                red = 0x0000;
                green = 0x0000;
                blue = 0xffff;
                break;
            case 0x05: ///<读史诗卡
                red = 0xffff;
                green = 0x0000;
                blue = 0xffff;
                break;
            case 0x06: ///<读传说卡
                red = 0xffff;
                green = 0x7fff;
                blue = 0x0000;
                break;
            default:
                red = 0xffff;
                green = 0xffff;
                blue = 0xffff;
            }
            if(now - light->old_tick >= light->channel_num /3 * speed_fast)
            {
                light->old_tick = now;
                light->all_state_num++;
                memcpy(light->pwm_tmp,light->pwm_data,light->channel_num*2);
            }
            else
            {
                light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
                light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
                light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
            }

        }
        else///<读满星卡
        {
            if(now - light->old_tick >= light->channel_num /3 * speed_fast)
            {
                light->old_tick = now;
                light->all_state_num++;
                memcpy(light->pwm_tmp,light->pwm_data,light->channel_num*2);
            }
            else if(now - light->old_tick < 9 * speed_fast)
            {
                red = 0xffff-0xffff/9*((now-light->old_tick)/speed_fast);
                green = 0xffff/9*((now-light->old_tick)/speed_fast);
                blue = 0x0000;
                light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
                light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
                light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
            }
            else if(now - light->old_tick < 18 * speed_fast)
            {
                red = 0x0000;
                green = 0xffff-0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
                blue = 0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
                light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
                light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
                light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
            }
            else
            {
                red = 0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
                green = 0x0000;
                blue = 0xffff-0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
                light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
                light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
                light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
            }
        }
        break;
    case 1: ///< 闪动3次
        if(now - light->old_tick >= 30*speed)
        {
            light->old_tick = now;
            light->all_state_num++;

        }
        else if(now - light->old_tick < 5*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 10*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
        else if(now - light->old_tick < 15*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 20*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
//        else if(now - light->old_tick < 25*speed)
//        {
//            memset(light->pwm_data,0x00,light->channel_num*2);
//        }
        else
        {
            memset(light->pwm_data,0xff,light->channel_num*2);
            light->all_state = 0x0a;///<游戏中有币
        }
        break;
    }

}

/** **************************************************************************************
* @brief circle投币后有币
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_coin(CircleTypeDef *light,uint32_t now)
{
    uint16_t red,green,blue;

    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;

        if(light == &P1circle)
        {
            red = 0x0000;
            green = 0x0000;
            blue = DIM_LIGHT;
        }
        else if(light == &P2circle)
        {
            red = DIM_LIGHT;
            green = 0x0000;
            blue = 0x0000;
        }
        else if(light == &P3circle)
        {
            red = 0x0000;
            green = DIM_LIGHT;
            blue = 0x0000;
        }
        else if(light == &P4circle)
        {
            red = DIM_LIGHT;
            green = DIM_LIGHT;
            blue = 0x0000;
        }
        else
        {
            red = 0xffff;
            green = 0xffff;
            blue = 0xffff;
        }
        for(uint8_t i= 0; i<26; i++)
        {
            light->pwm_data[i*3+0] = blue;
            light->pwm_data[i*3+1] = green;
            light->pwm_data[i*3+2] = red;
        }
    }
}

/** **************************************************************************************
* @brief circle投币后无币
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_nocoin(CircleTypeDef *light,uint32_t now)
{
    uint16_t red,green,blue;

    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;


        red = DIM_LIGHT;
        green = DIM_LIGHT;
        blue = DIM_LIGHT;

        for(uint8_t i= 0; i<26; i++)
        {
            light->pwm_data[i*3+0] = blue;
            light->pwm_data[i*3+1] = green;
            light->pwm_data[i*3+2] = red;
        }
    }
}

/** **************************************************************************************
* @brief circle使用兵法技能_按键
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_skill_button(CircleTypeDef *light,uint32_t now)
{
    uint16_t red,green,blue;
    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;

        if(light == &P1circle)
        {
            red = 0x0000;
            green = 0x0000;
            blue = 0xffff;
        }
        else if(light == &P2circle)
        {
            red = 0xffff;
            green = 0x0000;
            blue = 0x0000;
        }
        else if(light == &P3circle)
        {
            red = 0x0000;
            green = 0xffff;
            blue = 0x0000;
        }
        else if(light == &P4circle)
        {
            red = 0xffff;
            green = 0xffff;
            blue = 0x0000;
        }
        else
        {
            red = 0xffff;
            green = 0xffff;
            blue = 0xffff;
        }
        for(uint8_t i= 0; i<26; i++)
        {
            light->pwm_data[i*3+0] = blue;
            light->pwm_data[i*3+1] = green;
            light->pwm_data[i*3+2] = red;
        }
    }
    switch (light->all_state_num)
    {
    case 0: ///< 等待30ms
        if(now - light->old_tick >= 30 / TICK)
        {
            light->old_tick = now;
            light->all_state_num++;
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        break;
    case 1: ///< 灭灯
        if(now - light->old_tick >= 30 / TICK)
        {
            light->old_tick = now;
            light->all_state_num++;
        }
        break;
    case 2:
        light->all_state = 0x0a;///< 游戏中有币模式
        break;
    }
}

/** **************************************************************************************
* @brief circle使用兵法技能_刷出
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_skill_brushout(CircleTypeDef *light,uint32_t now)
{
    const uint8_t speed = 3; //越大越慢
    uint16_t red,green,blue;
    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;

        for(uint8_t i = 0; i<26; i++)
        {
            if(i < 9)
            {
                red = 0xffff-0xffff/9*i;
                green = 0xffff/9*i;
                blue = 0x0000;
            }
            else if(i < 18)
            {
                red = 0x0000;
                green = 0xffff-0xffff/9*(i-9);
                blue = 0xffff/9*(i-9);
            }
            else
            {
                red = 0xffff/9*(i-18);
                green = 0x0000;
                blue = 0xffff-0xffff/9*(i-18);
            }
            light->pwm_data[i*3+0]   = blue;
            light->pwm_data[i*3+1] = green;
            light->pwm_data[i*3+2] = red;
        }
        memcpy(light->pwm_tmp,light->pwm_data,light->channel_num*2);
    }
    switch (light->all_state_num)
    {
    case 0: ///< 闪动3次
        if(now - light->old_tick < 5*speed)
        {
            ;
        }
        else if(now - light->old_tick < 10*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 15*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
        else if(now - light->old_tick < 20*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 25*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
        else
        {
            memset(light->pwm_data,0xff,light->channel_num*2);
            light->all_state = 0x0a;///< 游戏中有币模式
        }
        break;
    }
}


/** **************************************************************************************
* @brief circle战斗胜利
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_victory(CircleTypeDef *light,uint32_t now)
{
    const uint8_t speed = 3; //越大越慢
    const uint8_t speed_fast = 1; //越大越慢
    uint16_t red,green,blue;

    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;
    }
    switch (light->all_state_num)
    {
    case 0: ///< 七彩颜色铺满一遍
        if(now - light->old_tick >= light->channel_num /3 * speed_fast)
        {
            light->old_tick = now;
            light->all_state_num++;
            memcpy(light->pwm_tmp,light->pwm_data,light->channel_num*2);
        }
        else if(now - light->old_tick < 9 * speed_fast)
        {
            red = 0xffff-0xffff/9*((now-light->old_tick)/speed_fast);
            green = 0xffff/9*((now-light->old_tick)/speed_fast);
            blue = 0x0000;
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else if(now - light->old_tick < 18 * speed_fast)
        {
            red = 0x0000;
            green = 0xffff-0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
            blue = 0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else
        {
            red = 0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
            green = 0x0000;
            blue = 0xffff-0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        break;
    case 1: ///< 闪动3次
        if(now - light->old_tick >= 30*speed)
        {
            light->old_tick = now;
            light->all_state_num++;

        }
        else if(now - light->old_tick < 5*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 10*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
        else if(now - light->old_tick < 15*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 20*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
//        else if(now - light->old_tick < 25*speed)
//        {
//            memset(light->pwm_data,0x00,light->channel_num*2);
//        }
        else
        {
            memset(light->pwm_data,0xff,light->channel_num*2);
            light->all_state = 0x0a;///< 游戏中有币模式
        }
        break;
    }
}

/** **************************************************************************************
* @brief circle战斗失败
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_fail(CircleTypeDef *light,uint32_t now)
{
    const uint8_t speed_fast = 1; //越大越慢
    uint16_t red,green,blue;

    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;
    }
    switch (light->all_state_num)
    {
    case 0: ///< 白色快速铺满一遍
        if(now - light->old_tick >= light->channel_num /3 * speed_fast)
        {
            light->old_tick = now;
            light->all_state_num++;
            light->command = master_circle_channel;
            light->all_state = 0xff;
            light->old_all_state = 0xff;
            for(uint8_t i = 0; i<light->channel_num; i++)
            {
                light->channel_state[i] = 0x7f;
            }
        }
        else
        {
            red = 0xffff;
            green = 0xffff;
            blue = 0xffff;
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        break;
    case 1: ///< 缓慢呼吸
        break;
    }

}

/** **************************************************************************************
* @brief circle抽卡环节
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_draw_card(CircleTypeDef *light,uint32_t now)
{
    const uint8_t speed = 3; //越大越慢
    const uint8_t speed_fast = 1; //越大越慢
    uint16_t red,green,blue;

    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;
    }
    switch (light->all_state_num)
    {
    case 0: ///< 特定颜色铺满一遍

        if(now - light->old_tick >= light->channel_num /3 * speed_fast)
        {
            light->old_tick = now;
            light->all_state_num++;
            memcpy(light->pwm_tmp,light->pwm_data,light->channel_num*2);
        }
        else
        {
            if(light == &P1circle)
            {
                red = 0x0000;
                green = 0x0000;
                blue = 0xffff;
            }
            else if(light == &P2circle)
            {
                red = 0xffff;
                green = 0x0000;
                blue = 0x0000;
            }
            else if(light == &P3circle)
            {
                red = 0x0000;
                green = 0xffff;
                blue = 0x0000;
            }
            else if(light == &P4circle)
            {
                red = 0xffff;
                green = 0xffff;
                blue = 0x0000;
            }
            else
            {
                red = 0xffff;
                green = 0xffff;
                blue = 0xffff;
            }
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        break;
    case 1: ///< 闪动3次
        if(now - light->old_tick >= 30*speed)
        {
            light->old_tick = now;
            light->all_state_num++;

        }
        else if(now - light->old_tick < 5*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 10*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
        else if(now - light->old_tick < 15*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 20*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
//        else if(now - light->old_tick < 25*speed)
//        {
//            memset(light->pwm_data,0x00,light->channel_num*2);
//        }
        else
        {
            memset(light->pwm_data,0xff,light->channel_num*2);
            light->all_state = 0x10;///< 结算中有币模式
        }
        break;
    }

}

/** **************************************************************************************
* @brief rare card
* @param *light: 灯光组
* @param now: 当前时间计数 = HAL_GetTick() / TICK
* @return
* @retval
*****************************************************************************************/
static void circle_all_pattern_RareCard(CircleTypeDef *light,uint32_t now)
{
    const uint8_t speed = 3; //越大越慢
    const uint8_t speed_fast = 1; //越大越慢
    uint16_t red,green,blue;

    if(light->old_all_state != light->all_state)
    {
        light->all_state_num = 0;
        light->old_tick = now;
        light->old_all_state = light->all_state;
    }
    switch (light->all_state_num)
    {
    case 0: ///< 七彩颜色铺满一遍
        if(now - light->old_tick >= light->channel_num /3 * speed_fast)
        {
            light->old_tick = now;
            light->all_state_num++;
            memcpy(light->pwm_tmp,light->pwm_data,light->channel_num*2);
        }
        else if(now - light->old_tick < 9 * speed_fast)
        {
            red = 0xffff-0xffff/9*((now-light->old_tick)/speed_fast);
            green = 0xffff/9*((now-light->old_tick)/speed_fast);
            blue = 0x0000;
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else if(now - light->old_tick < 18 * speed_fast)
        {
            red = 0x0000;
            green = 0xffff-0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
            blue = 0xffff/9*((now-light->old_tick-9*speed_fast)/speed_fast);
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        else
        {
            red = 0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
            green = 0x0000;
            blue = 0xffff-0xffff/9*((now-light->old_tick-18*speed_fast)/speed_fast);
            light->pwm_data[((now-light->old_tick)/speed_fast*3)%light->channel_num]   = blue;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+1)%light->channel_num] = green;
            light->pwm_data[((now-light->old_tick)/speed_fast*3+2)%light->channel_num] = red;
        }
        break;
    case 1: ///< 闪动3次
        if(now - light->old_tick >= 30*speed)
        {
            light->old_tick = now;
            light->all_state_num++;

        }
        else if(now - light->old_tick < 5*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 10*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
        else if(now - light->old_tick < 15*speed)
        {
            memset(light->pwm_data,0x00,light->channel_num*2);
        }
        else if(now - light->old_tick < 20*speed)
        {
            memcpy(light->pwm_data,light->pwm_tmp,light->channel_num*2);
        }
//        else if(now - light->old_tick < 25*speed)
//        {
//            memset(light->pwm_data,0x00,light->channel_num*2);
//        }
        else
        {
            memset(light->pwm_data,0xff,light->channel_num*2);
            light->all_state = 0x10;///< 结算中有币模式
        }
        break;
    }
}

/** **************************************************************************************
* @brief circle灯光闪动的实现
* @note
* @param
* @retval
*****************************************************************************************/
static void circle_pattern(void)
{
    static uint32_t old_time;
    uint32_t now;
    void  (*_init[4])(void) = {p1circle_init,p2circle_init,p3circle_init,p4circle_init};
    now = HAL_GetTick()/TICK;
    if(now != old_time)
    {
        for(uint8_t i=0; i<4; i++)
        {
            if(Circle[i]->isinit == 0)
            {
                _init[i]();
                Circle[i]->isinit = 1;
            }
            if(Circle[i]->command == master_circle_ll)							///< ll底层控制
            {
                ;
            }
            else if(Circle[i]->command == master_circle_channel)			///< channle 样式
            {
                for(uint8_t ch = 0; ch < Circle[i]->channel_num; ch++)
                {
                    switch(Circle[i]->channel_state[ch]& 0x70)					///< 确定时间
                    {
                    case 0x00:														///< 无限长
                        Circle[i]->pwm_data[ch] = (uint16_t)(Circle[i]->channel_state[ch] & 0x0ff)<<12;
                        break;
                    case 0x10:														///< 100ms
                        circle_channel_pattern(Circle[i], ch, 101,now);
                        break;
                    case 0x20:														///< 200ms
                        circle_channel_pattern(Circle[i], ch, 199,now);
                        break;
                    case 0x30:														///< 500ms
                        circle_channel_pattern(Circle[i], ch, 503,now);
                        break;
                    case 0x40:														///< 1s
                        circle_channel_pattern(Circle[i], ch, 997,now);
                        break;
                    case 0x50:														///< 2s
                        circle_channel_pattern(Circle[i], ch, 1999,now);
                        break;
                    case 0x60:														///< 3s
                        circle_channel_pattern(Circle[i], ch, 2999,now);
                        break;
                    case 0x70:														///< 4s
                        circle_channel_pattern(Circle[i], ch, 4001,now);
                        break;
                    }
                }
            }
            else if(Circle[i]->command == master_circle_all)					        ///< 环形灯整体样式
            {
                switch(Circle[i]->all_state)
                {
                case 0x00: ///< 逐个通道以1秒间隔流水
                    circle_all_pattern_flow(Circle[i], now);
                    break;
                case 0x01: ///< 待机无操作
                    circle_all_pattern_idle(Circle[i], now);
                    break;
                case 0x02: ///< 待机时拍任意键
                    circle_all_pattern_idle_button(Circle[i], now);
                    break;
                case 0x03: ///< 读普通卡
                    circle_all_pattern_read_card(Circle[i], now);
                    break;
                case 0x04: ///< 读稀有卡
                    circle_all_pattern_read_card(Circle[i], now);
                    break;
                case 0x05: ///< 读史诗卡
                    circle_all_pattern_read_card(Circle[i], now);
                    break;
                case 0x06: ///< 读传说卡
                    circle_all_pattern_read_card(Circle[i], now);
                    break;
                case 0x07: ///< 读满星卡
                    circle_all_pattern_read_card(Circle[i], now);
                    break;
                case 0x08: ///< 投币后
                    circle_all_pattern_coin(Circle[i], now);
                    break;
                case 0x09: ///< 投币后没有币
                    circle_all_pattern_nocoin(Circle[i], now);
                    break;
                case 0x0a: ///< 游戏有币
                    circle_all_pattern_coin(Circle[i], now);
                    break;
                case 0x0b: ///< 游戏无币
                    circle_all_pattern_nocoin(Circle[i], now);
                    break;
                case 0x0c: ///< 使用兵法技能_按键
                    circle_all_pattern_skill_button(Circle[i], now);
                    break;
                case 0x0d: ///< 使用兵法技能_刷出
                    circle_all_pattern_skill_brushout(Circle[i], now);
                    break;
                case 0x0e: ///< 胜利
                    circle_all_pattern_victory(Circle[i], now);
                    break;
                case 0x0f: ///< 失败
                    circle_all_pattern_fail(Circle[i], now);
                    break;
                case 0x10: ///< 结算中_有币
                    circle_all_pattern_coin(Circle[i], now);
                    break;
                case 0x11: ///< 结算中_无币
                    circle_all_pattern_nocoin(Circle[i], now);
                    break;
                case 0x12: ///< 抽卡环节
                    circle_all_pattern_draw_card(Circle[i], now);
                    break;
                case 0x13: ///< 抽到珍惜卡
                    circle_all_pattern_RareCard(Circle[i], now);
                    break;
                default:
                    break;
                }
            }
        }
        old_time = now;

    }
}

/** **************************************************************************************
* @brief 灯光闪动的实现
* @note
* @param
* @retval
*****************************************************************************************/
void light_pattern(void)
{
    pwm_pattern();
    circle_pattern();
}
#undef TICK
