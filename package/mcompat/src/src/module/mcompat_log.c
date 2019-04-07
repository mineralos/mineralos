/******************************************************************************
 *
 * 文件名  ： mcompat_log.c
 * 负责人  ： pengp
 * 创建日期： 20171002
 * 版本号  ： v1.0
 * 文件描述： 实现日志打印
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>

#include "mcompat_config.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/
/*******************************************************************************
 *
 * 函数名  : log_out
 * 负责人  : 彭鹏
 * 创建日期：20171214
 * 函数功能: 输出字符串
 * 输入参数: prio - 打印级别(MCOMPAT_LOG_X)
 *           str  - 待输出的字符串
 * 输出参数: 无
 * 返回值:   无
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
static void log_out(int prio, const char *str);

/********************************** 变量实现区 *********************************/
/* 调试级别 */
int g_log_level = 0;

/********************************** 函数实现区 *********************************/
void mcompat_log_init(int default_level, const char *indent)
{
    g_log_level = default_level;
    openlog(indent, LOG_PID, LOG_USER);
}

void _mcompat_log(int prio, char *tmp, bool has_prefix)
{
    char prefix[16] = {'\0'};
    char datetime[64] = {'\0'};
    char line_buf[MCOMPAT_CONFIG_LOG_LINE_BUF_SIZE] = {'\0'};

    /* 不够级别,不输出 */
    if(prio < g_log_level)
    {
        return;
    }

    if(has_prefix)
    {
        struct timeval tv = {0, 0};
        struct tm *tm = NULL;

        gettimeofday(&tv, NULL);

        const time_t tmp_time = tv.tv_sec;
        int ms = (int)(tv.tv_usec / 1000);
        tm = localtime(&tmp_time);

        switch(prio)
        {
            case MCOMPAT_LOG_DEBUG:
                {
                    snprintf(prefix, sizeof(prefix), "[DEBG:");
                    break;
                }
            case MCOMPAT_LOG_INFO:
                {
                    snprintf(prefix, sizeof(prefix), "[INFO:");
                    break;
                }
            case MCOMPAT_LOG_ERR:
                {
                    snprintf(prefix, sizeof(prefix), "[ERRO:");
                    break;
                }
            default:
                {
                    snprintf(prefix, sizeof(prefix), "[NIMP:");
                    break;
                }
        }

        snprintf(datetime, sizeof(datetime), "%d-%02d-%02d %02d:%02d:%02d.%03d] ",
                tm->tm_year + 1900,
                tm->tm_mon + 1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec, ms);

    }

    /* FIXME: 需要更复杂的逻辑确保 line_buf不会溢出 */

    memset(line_buf, '\0', sizeof(line_buf));
    strncat(line_buf, prefix, strlen(prefix));
    strncat(line_buf, datetime, strlen(datetime));
    strncat(line_buf, tmp, strlen(tmp));

    log_out(prio, line_buf);
    return;
}

static void log_out(int prio, const char *str)
{
    FILE *fd = NULL;
    int level = 0;

    level = LOG_LOCAL0;
    switch(prio)
    {
        case MCOMPAT_LOG_DEBUG:
        {
            level |= LOG_DEBUG;
            break;
        }
        case MCOMPAT_LOG_INFO:
        {
            level |= LOG_INFO;
            break;
        }
        case MCOMPAT_LOG_NOTICE:
        {
            level |= LOG_NOTICE;
            break;
        }
        case MCOMPAT_LOG_WARNING:
        {
            level |= LOG_WARNING;
            break;
        }
        case MCOMPAT_LOG_ERR:
        {
            level |= LOG_ERR;
            break;
        }
        case MCOMPAT_LOG_CRIT:
        {
            level |= LOG_CRIT;
            break;
        }
        case MCOMPAT_LOG_ALERT:
        {
            level |= LOG_ALERT;
            break;
        }
        case MCOMPAT_LOG_EMERG:
        {
            level |= LOG_EMERG;
            break;
        }
        default: /* 不符合预期,使用最高级别输出 */
        {
            level |= LOG_EMERG;
            break;
        }
    }
    syslog(level, str);

    /* 控制台 */
    if(prio >= MCOMPAT_LOG_ERR)
    {
        fd = stderr;
    }
    else
    {
        fd = stdout;
    }
    fprintf(fd, "%s", str);
    fflush(fd);
}


