/******************************************************************************
 *
 * 文件名  ： zynq_gpio.c
 * 负责人  ： wangp
 * 创建日期： 20171208
 * 版本号  ： v1.0
 * 文件描述： zynq平台gpio驱动实现
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
#include "zynq_gpio.h"

#define IOCTL_SET_VAL_0                         _IOR(MAGIC_NUM, 0, char *)
#define IOCTL_SET_VALUE_0                       _IOR(MAGIC_NUM, 0, char *)
#define IOCTL_SET_CHAIN_0                       _IOR(MAGIC_NUM, 1, char *)

#define BUF_MAX                                 (256)

#define SYSFS_GPIO_EXPORT                       ("/sys/class/gpio/export")
#define SYSFS_GPIO_DIR_STR                      ("/sys/class/gpio/gpio%d/direction")
#define SYSFS_GPIO_VAL_STR                      ("/sys/class/gpio/gpio%d/value")
#define SYSFS_GPIO_DIR_OUT                      ("out")
#define SYSFS_GPIO_DIR_IN                       ("in")
#define SYSFS_GPIO_VAL_LOW                      ("0")
#define SYSFS_GPIO_VAL_HIGH                     ("1")

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/


/********************************** 变量实现区 *********************************/

/********************************** 函数实现区 *********************************/
void zynq_gpio_init(int pin, int dir)
{
    int fd = 0;
    ssize_t write_bytes = 0;
    char fvalue[BUF_MAX] = {'\0'};
    char fpath[BUF_MAX] = {'\0'};

    fd = open(SYSFS_GPIO_EXPORT, O_WRONLY);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    memset(fvalue, 0, sizeof(fvalue));
    sprintf(fvalue, "%d", pin);
    write_bytes = write(fd, fvalue, strlen(fvalue));
    if(-1 == write_bytes)
    {
        if(EBUSY == errno) /* 已经初始化 */
        {
            close(fd);
            return;
        }
        else
        {
            mcompat_log_err("%s,%d: %d,%s.\n", __FILE__, __LINE__, errno, strerror(errno));
        }
    }
    close(fd);

    memset(fpath, 0, sizeof(fpath));
    sprintf(fpath, SYSFS_GPIO_DIR_STR, pin);
    fd = open(fpath, O_WRONLY);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    if(0 == dir)
    {
        write_bytes = write(fd, SYSFS_GPIO_DIR_OUT, sizeof(SYSFS_GPIO_DIR_OUT));
        if(-1 == write_bytes)
        {
            mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
        }
    }
    else
    {
        write_bytes = write(fd, SYSFS_GPIO_DIR_IN, sizeof(SYSFS_GPIO_DIR_IN));
        if(-1 == write_bytes)
        {
            mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
        }
    }
    close(fd);

    return;
}

void zynq_gpio_write(int pin, int val)
{
    int  fd = 0;
    ssize_t  write_bytes = 0;
    char fpath[BUF_MAX] = {'\0'};

    memset(fpath, 0, sizeof(fpath));
    sprintf(fpath, SYSFS_GPIO_VAL_STR, pin);
    fd = open(fpath, O_WRONLY);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }

    if(0 == val)
    {
        write_bytes = write(fd, SYSFS_GPIO_VAL_LOW, sizeof(SYSFS_GPIO_VAL_LOW));
        if(-1 == write_bytes)
        {
            mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
        }
    }
    else
    {
        write_bytes = write(fd, SYSFS_GPIO_VAL_HIGH, sizeof(SYSFS_GPIO_VAL_HIGH));
        if(-1 == write_bytes)
        {
            mcompat_log_err("%s,%d: %s,%s.\n", __FILE__, __LINE__, fpath, strerror(errno));
        }
    }

    close(fd);
    return;
}

int zynq_gpio_read(int pin)
{
    int  fd = 0;
    int  val = 0;
    ssize_t read_bytes = 0;
    char fpath[BUF_MAX] = {'\0'};
    char fvalue[BUF_MAX] = {'\0'};

    memset(fpath, 0, sizeof(fpath));
    sprintf(fpath, SYSFS_GPIO_VAL_STR, pin);
    fd = open(fpath, O_RDONLY);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    memset(fvalue, 0, sizeof(fvalue));
    read_bytes = read(fd, fvalue, 1);
    if(-1 == read_bytes)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    close(fd);

    if('0' == fvalue[0])
    {
        val = 0;
    }
    else if('1' == fvalue[0])
    {
        val = 1;
    }
    else
    {
        val = -1;
    }

    return val;
}

void zynq_gpio_exit(int pin)
{
    return;
}

