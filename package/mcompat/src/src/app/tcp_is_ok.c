/******************************************************************************
 *
 * 文件名  ： tcp_is_ok.c
 * 负责人  ： pengp
 * 创建日期： 20180129
 * 版本号  ： v1.0
 * 文件描述： 判断TCP链接是否OK
 * 版权说明： Copyright (c) 2000-2020 im
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mcompat_lib.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/


/********************************** 变量实现区 *********************************/


/********************************** 函数实现区 *********************************/
/*******************************************************************************
 *
 * 函数名  : main
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 主函数
 * 输入参数: argc - 参数个数
 *           argv - 命令行参数数组
 * 输出参数: 无
 * 返回值:   0   : 正常退出
 *           其它: 异常退出
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
int main(int argc, char *argv[])
{
    /* char *host = "127.0.0.1"; */
    /* char *host = "114.114.114.114"; */
    char *host = "8.8.8.8";
    short int port = 53;
    char *ptr = NULL;
    int ret = 0;
    int i = 1;

    if(argc > 1)
    {
        if (strcmp(argv[1], "-?") == 0
        ||  strcmp(argv[1], "-h") == 0
        ||  strcmp(argv[1], "--help") == 0)
        {
            fprintf(stderr, "usage: %s [ip/host [port]]\n", argv[0]);
            return 1;
        }
    }

    if(argc > i)
    {
        ptr = misc_trim(argv[i++]);
        if (strlen(ptr) > 0)
        {
            host = ptr;
        }
    }

    if(argc > i)
    {
        ptr = misc_trim(argv[i]);
        if (strlen(ptr) > 0)
        {
            port = atoi(ptr);
        }
    }

    ret = misc_tcp_is_ok(host, port);
    if(ret)
    {
        fprintf(stdout, "%s,%d is ok\n", host, port);
    }
    else
    {
        fprintf(stdout, "%s,%d is not ok\n", host, port);
    }

    return 0;
}

