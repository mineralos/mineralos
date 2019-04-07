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
#include "zynq_vid.h"

/*----------------------------------- 声明区 ----------------------------------*/


/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/


/********************************** 变量实现区 *********************************/


/********************************** 函数实现区 *********************************/
int zynq_gpio_g9_vid_set(int level)
{
    int fd = 0;

    fd = open(SYSFS_VID_DEV, O_RDWR);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }

    if(ioctl(fd, IOCTL_SET_VAL_0, 0x0100 | level) < 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    close(fd);

    return 0;
}

int zynq_gpio_g19_vid_set(int chain_id, int level)
{
    int fd = 0;

    fd = open(SYSFS_VID_DEV, O_RDWR);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    if(ioctl(fd, IOCTL_SET_CHAIN_0, chain_id) < 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    if(ioctl(fd, IOCTL_SET_VALUE_0, 0x100 | level) < 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    close(fd);

    return 0;
}

