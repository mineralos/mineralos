/******************************************************************************
 *
 * 文件名  ： zynq_vid.c
 * 负责人  ： pengp
 * 创建日期： 20171212
 * 版本号  ： v1.0
 * 文件描述： zynq平台vid驱动实现
 * 版权说明： Copyright (c) 2000-2020 im
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "mcompat_config.h"
#include "zynq_pwm.h"

/*----------------------------------- 声明区 ----------------------------------*/


/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/


/********************************** 变量实现区 *********************************/
pthread_mutex_t s_pwm_lock;


/********************************** 函数实现区 *********************************/
void zynq_set_pwm(unsigned char fan_id, int frequency, int duty)
{
    int fd = 0;
    int duty_driver = 0;

    duty_driver = frequency / 100 * (100 - duty);

    //pthread_mutex_lock(&s_pwm_lock);

    /* 开启风扇结点 */
    fd = open(SYSFS_PWM_DEV, O_RDWR);
    if(fd < 0)
    {
        mcompat_log(MCOMPAT_LOG_ERR, "open %s fail\n", SYSFS_PWM_DEV);
        //pthread_mutex_unlock(&s_pwm_lock);
        return;
    }

    if(ioctl(fd, IOCTL_SET_PWM_FREQ(fan_id), frequency) < 0)
    {
        mcompat_log(MCOMPAT_LOG_ERR,"set fan%d frequency fail \n", fan_id);
        close(fd);
        //pthread_mutex_unlock(&s_pwm_lock);
        return ;
    }

    if(ioctl(fd, IOCTL_SET_PWM_DUTY(fan_id), duty_driver) < 0)
    {
        mcompat_log(MCOMPAT_LOG_ERR,"set fan%d duty fail \n", fan_id);
        close(fd);
        //pthread_mutex_unlock(&s_pwm_lock);
        return ;
    }

    close(fd);
    //pthread_mutex_unlock(&s_pwm_lock);

    return;
}



