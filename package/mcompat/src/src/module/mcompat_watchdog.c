/******************************************************************************
 *
 * 文件名  ： mcompat_watchdog.c
 * 负责人  ： pengp
 * 创建日期： 20180225
 * 版本号  ： v1.0
 * 文件描述： 看门狗api实现
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 矿机多条链全部为同一个类型的算力板
 *            使用mcompat_miner中的库函数
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>

#include "mcompat_config.h"
#include "mcompat_log.h"
#include "mcompat_watchdog.h"

#define MCOMPAT_WATCHDOG_DEV               ("/dev/watchdog0")
/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数声明区 *********************************/

/********************************** 变量实现区 *********************************/
/* 看门狗设备文件描述符 */
static int s_watchdog_fd = 0;

/********************************** 函数实现区 *********************************/
void mcompat_watchdog_keep_alive(void)
{
    int dummy = 0;
    ioctl(s_watchdog_fd, WDIOC_KEEPALIVE, &dummy);
}

void mcompat_watchdog_open(void)
{
    s_watchdog_fd = open(MCOMPAT_WATCHDOG_DEV, O_WRONLY);
    if(-1 == s_watchdog_fd)
    {
        mcompat_log_err("%s watchdog device can't be enabled.", MCOMPAT_WATCHDOG_DEV);
    }
}

void mcompat_watchdog_set_timeout(int timeout)
{
    ioctl(s_watchdog_fd, WDIOC_SETTIMEOUT, &timeout);
}

void mcompat_watchdog_close(void)
{
    close(s_watchdog_fd);
}

