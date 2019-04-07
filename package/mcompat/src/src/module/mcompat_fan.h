/******************************************************************************
 *
 * 文件名  ：mcompat_fan.h
 * 负责人  ： yex
 * 创建日期： 20171122
 * 版本号  ： V1.0
 * 文件描述： 风扇与温度控制接口
 * 版权说明： Copyright (c) 2000-2020   GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _MCOMPAT_FAN_H_
#define _MCOMPAT_FAN_H_

/************************************ 头文件 ***********************************/

#include "mcompat_config.h"

/************************************ 宏定义 ***********************************/

/*********************************** 类型定义 **********************************/

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/
extern int g_temp_hi_thr;
extern int g_temp_lo_thr;
extern int g_temp_start_thr;
extern int g_dangerous_temp;
extern int g_work_temp;

/*--------------------------------- data struct  --------------------------------*/

/*********************************** realize **********************************/
typedef struct {
    int final_temp_avg;
    int final_temp_hi;
    int final_temp_lo;
    int temp_highest[3];
    int temp_lowest[3];
}mcompat_temp_s;


typedef struct {
    int temp_hi_thr;
    int temp_lo_thr;
    int temp_start_thr;
    int dangerous_stat_temp;
    int work_temp;
    int default_fan_speed;
}mcompat_temp_config_s;


typedef struct {
    int fd;
    int last_valid_temp;

    int speed;
    int last_fan_speed;
    int last_fan_temp;
    mcompat_temp_s * mcompat_temp;
    int temp_average;
    int temp_highest;
    int temp_lowest;
}mcompat_fan_temp_s;


/*********************************** function prototype **********************************/
//extern int mcompat_deal_temp(unsigned char spi_id, mcompat_fan_temp_s *temp_ctrl);
extern void mcompat_fan_temp_init(unsigned char fan_id,mcompat_temp_config_s temp_config);
extern void mcompat_fan_speed_set(unsigned char fan_id, int speed);
extern void mcompat_fan_speed_update_hub(mcompat_fan_temp_s *fan_temp);

#endif // #ifndef _MCOMPAT_FAN_TEMP_H_
