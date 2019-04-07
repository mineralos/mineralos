/******************************************************************************
 *
 * 文件名  ： hub_vid.h
 * 负责人  ： pengp
 * 创建日期： 20170307
 * 版本号  ： v1.0
 * 文件描述： hub的vid的接口
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 目前hub的vid实现包括三种方案:
 *            1. vid脉冲
 *            2. gpio模拟 I2C
 *            3. uart命令
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _HUB_VID_H_
#define _HUB_VID_H_

/************************************ 头文件 ***********************************/
#include "mcompat_config.h"

/************************************ 宏定义 ***********************************/
#define I2C_DEVICE_NAME     "/dev/i2c-0"
#define I2C_SLAVE_ADDR      0x01



/*********************************** 类型定义 **********************************/

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
bool hub_set_vid(uint8_t chan_id, int vol);

#endif // #ifndef _HUB_VID_H_

