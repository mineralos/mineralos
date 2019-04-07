/******************************************************************************
 *
 * 文件名  ： hub_hardware.h
 * 负责人  ： pengp
 * 创建日期： 20170307
 * 版本号  ： v1.0
 * 文件描述： hub的硬件屏蔽层接口,用于屏蔽与数字组代码的差异
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _HUB_HARDWARE_H_
#define _HUB_HARDWARE_H_

/************************************ 头文件 ***********************************/

/************************************ 宏定义 ***********************************/
#define _MAX_MEM_RANGE              (0x10000)

/*********************************** 类型定义 **********************************/

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void hub_hardware_init(void);
void hub_hardware_deinit(void);

#endif // #ifndef _HUB_HARDWARE_H_

