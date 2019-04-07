/******************************************************************************
 *
 * 文件名  ： mcompat_misc.c
 * 负责人  ： pengp
 * 创建日期： 20180329
 * 版本号  ： v2.0
 * 文件描述： 各种杂项库函数 实现
 * 版权说明： Copyright (c) 2000-2020 im
 * 其    他： 无
 * 修改人  ：  sunjw
 * 修改日志： 增加读取电压的函数
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mcompat_config.h"
#include "mcompat_lib.h"
#include "zynq_gpio.h"
#include "mcompat_config.h"
#include "mcompat_lib.h"
#include "mcompat_drv.h"

#define SOCK_SIZE           (65535)
#define SOCK_ERR_MSG        strerror(errno)

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/


/********************************** 变量实现区 *********************************/


/********************************** 函数实现区 *********************************/
int mcompat_get_shell_cmd_rst(char *cmd, char *result, int size)
{
    char buffer[1024] = {0};
    int offset = 0;
    int len;
    FILE *fp = NULL;

    fp = popen(cmd, "r");
    if(NULL == fp)
    {
        mcompat_log(MCOMPAT_LOG_ERR, "failed to open pipe for command %s", cmd);
        return 0;
    }

    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        len = strlen(buffer);
        if(offset + len < size)
        {
            strcpy(result + offset, buffer);
            offset += len;
        }
        else
        {
            strncpy(result + offset, buffer, size - offset);
            offset = size;
            break;
        }
    }

    mcompat_log(MCOMPAT_LOG_DEBUG, "command result(%d): %s", offset, result);

    return offset;
}

int misc_call_api(char *command, char *host, short int port)
{
    struct sockaddr_in serv;
    int sock = 0;
    int ret = 0;
    int n = 0;
    char *buf = NULL;
    size_t len = SOCK_SIZE;
    size_t p = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        printf("Socket initialisation failed: %s\n", SOCK_ERR_MSG);
        return -1;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(host);
    serv.sin_port = htons(port);
    if (connect(sock, (struct sockaddr *)&serv, sizeof(struct sockaddr)) < 0)
    {
        printf("Socket connect failed: %s\n", SOCK_ERR_MSG);
        return -1;
    }

    n = send(sock, command, strlen(command), 0);
    if (n < 0)
    {
        printf("Send failed: %s\n", SOCK_ERR_MSG);
        ret = -1;
    }
    else
    {
        buf = malloc(len+1);
        if (!buf)
        {
            printf("Err: OOM (%d)\n", (int)(len+1));
            return -1;
        }

        while(1)
        {
            if ((len - p) < 1)
            {
                len += SOCK_SIZE;
                buf = realloc(buf, len+1);
                if (!buf)
                {
                    printf("Err: OOM (%d)\n", (int)(len+1));
                    return -1;
                }
            }

            n = recv(sock, &buf[p], len - p , 0);
            if(n < 0)
            {
                printf("Recv failed: %s\n", SOCK_ERR_MSG);
                ret = -1;
                break;
            }

            if(0 == n)
            {
                break;
            }

            p += n;
        }
        buf[p] = '\0';
        printf("%s\n", buf);

        free(buf);
        buf = NULL;
    }

    close(sock);

    return ret;
}

bool misc_tcp_is_ok(char *host, short int port)
{
    struct sockaddr_in serv;
    int sock = 0;
    int mode = 0;
    bool ret = false;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        printf("Socket initialisation failed: %s\n", SOCK_ERR_MSG);
        return -1;
    }

    /* 非阻塞模式 */
    ioctl(sock, FIONBIO, &mode);

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(host);
    serv.sin_port = htons(port);
    ret = false;
    if(0 == connect(sock, (struct sockaddr *)&serv, sizeof(struct sockaddr)))
    {
        ret = true;
    }

    close(sock);

    return ret;
}

char *misc_trim(char *str)
{
    char *ptr;

    while (isspace(*str))
    {
        str++;
    }

    ptr = strchr(str, '\0');
    while (ptr-- > str)
    {
        if (isspace(*ptr))
        {
            *ptr = '\0';
        }
    }

    return str;
}

/* 控制板类型 */
int misc_get_board_version(void)
{
    FILE* fd = NULL;
    char buffer[64] = {'\0'};
    int version = MCOMPAT_LIB_HARDWARE_VERSION_ERR;

    fd = fopen(MCOMPAT_LIB_HARDWARE_VERSION_FILE, "r");
    if(fd == NULL)
    {
        mcompat_log_err("open hwver file:%s failed! \n", MCOMPAT_LIB_HARDWARE_VERSION_FILE);
    }

    memset(buffer, 0, sizeof(buffer));
    fread(buffer, 8, 1, fd);
    fclose(fd);

    if(strstr(buffer, "G9") != NULL)
    {
        version = MCOMPAT_LIB_HARDWARE_VERSION_G9;
        mcompat_log(MCOMPAT_LOG_INFO, "hardware version is G9 \n");
    }
    else if(strstr(buffer, "G19") != 0)
    {
        version = MCOMPAT_LIB_HARDWARE_VERSION_G19;
        mcompat_log(MCOMPAT_LOG_INFO, "hardware version is G19 \n");
    }
    else
    {
        mcompat_log_err("unknown hardware version:%s! \n", buffer);
    }

    return version;
}

/* Deprecated */
int misc_get_miner_type(void)
{
#if 0
    FILE *fd = NULL;
    char buffer[64] = {'\0'};
    int miner_type = MCOMPAT_LIB_MINER_TYPE_ERR;

    fd = fopen(MCOMPAT_LIB_MINER_TYPE_FILE, "r");
    if(fd == NULL)
    {
        mcompat_log_err("open miner type file:%s failed!", MCOMPAT_LIB_MINER_TYPE_FILE);
    }

    memset(buffer, 0, sizeof(buffer));
    fread(buffer, 8, 1, fd);
    fclose(fd);

    if(strstr(buffer, "T1") != NULL)
    {
        miner_type = MCOMPAT_LIB_MINER_TYPE_T1;
        mcompat_log(MCOMPAT_LOG_INFO, "miner type is T1 \n");
    }
    else if(strstr(buffer, "T2") != NULL)
    {
        miner_type = MCOMPAT_LIB_MINER_TYPE_T2;
        mcompat_log(MCOMPAT_LOG_INFO, "miner type is T2 \n");
    }
    else if(strstr(buffer, "T3") != NULL)
    {
        miner_type = MCOMPAT_LIB_MINER_TYPE_T3;
        mcompat_log(MCOMPAT_LOG_INFO, "miner type is T3 \n");
    }
    else if(strstr(buffer, "T4") != NULL)
    {
        miner_type = MCOMPAT_LIB_MINER_TYPE_T4;
        mcompat_log(MCOMPAT_LOG_INFO, "miner type is T4 \n");
    }
    else
    {
        mcompat_log_err("unknown miner type:%s! \n", buffer);
    }

    return miner_type;
#endif
    return MCOMPAT_LIB_MINER_TYPE_ERR;
}

int misc_get_vid_type(void)
{
    int val_b9 = 0;
	int val_a10 = 0;
	int val = 0;
	int type = 0;

	zynq_gpio_init(MCOMPAT_CONFIG_B9_GPIO, 1);
	zynq_gpio_init(MCOMPAT_CONFIG_A10_GPIO, 1);

	val_b9 = zynq_gpio_read(MCOMPAT_CONFIG_B9_GPIO);
	val_a10 = zynq_gpio_read(MCOMPAT_CONFIG_A10_GPIO);

	type = val_a10 << 1 | val_b9;

	return type;
}

void misc_system(const char *cmd, char *rst_buf, int buf_size)
{
    int  i = 0;
    FILE *fp = NULL;
    char *go_ptr = NULL;
    char c = 0;

    if(NULL == cmd || NULL == rst_buf || buf_size < 0)
    {
        mcompat_log_err("param error:%s,%s,%d.\n", cmd, rst_buf, buf_size);
    }

    fp = popen(cmd, "r");
    if(NULL == fp)
    {
        mcompat_log_err("popen error:%s,%s,%d.\n", cmd, rst_buf, buf_size);
    }
    else
    {
        go_ptr = rst_buf;

        memset(go_ptr, 0, buf_size);
        for(i = 0; i < buf_size; i++)
        {
            c = fgetc(fp);
            if(isprint(c))
            {
                *go_ptr++ = c;
                fprintf(stderr, "%s,%d: %c\n", __FILE__, __LINE__, c);
            }
            else
            {
                break;
            }
        }

        /* 截断,避免溢出 */
        rst_buf[buf_size-1] = '\0';

        pclose(fp);
    }
}

static double mcompat_get_average_volt(int *volt, int size)
{
	int i;
	int count = 0;
	int total = 0, max = 0, min = 1000;

	for (i = 0; i < size; i++) {
		if (volt[i] > 0) {
			total += volt[i];
			max = MAX(max, volt[i]);
			min = MIN(min, volt[i]);
			count++;
		}
	}

	if (count > 2)
		return (double) (total - max - min) / (count - 2);
	else
		return 0;
}

#define VOLT_STEP       (-3.0f)

int mcompat_find_chain_vid(int chain_id, int chip_num, int vid_start, double volt_target)
{
	int chip_volt[MCOMPAT_CONFIG_MAX_CHIP_NUM] = {0};
	int vid = vid_start;
	double volt_diff_prev;
	double volt_diff;
	double volt_avg;

    // config to V-sensor
	mcompat_configure_tvsensor(chain_id, CMD_ADDR_BROADCAST, 0);
	usleep(1000);

	mcompat_log(MCOMPAT_LOG_NOTICE, "chain%d find_chain_vid: start_vid = %d, target_volt = %.1f", 
		chain_id, vid_start, volt_target);

	// set start vid
	mcompat_set_vid(chain_id, vid);
	sleep(3);
	mcompat_get_chip_volt(chain_id, chip_volt);
	volt_avg = mcompat_get_average_volt(chip_volt, chip_num);
	volt_diff_prev = volt_avg - volt_target;
    if (volt_avg < 1) {
		vid = -1;
		goto out;
	}

	mcompat_log(MCOMPAT_LOG_NOTICE, "Chain %d VID %d voltage %.1f", chain_id, vid, volt_avg);

	// set vid again with estimated value when volt_diff > 10mV
	if (abs(volt_diff_prev) > 10)
	{
		// estimate new vid
		vid = vid_start - volt_diff_prev / VOLT_STEP;
		if (vid > VID_MAX)
			vid = VID_MAX;
		else if (vid < VID_MIN)
			vid = VID_MIN;

		mcompat_set_vid_by_step(chain_id, vid_start, vid);
		sleep(3);
		mcompat_get_chip_volt(chain_id, chip_volt);
		volt_avg = mcompat_get_average_volt(chip_volt, chip_num);
		volt_diff_prev = volt_avg - volt_target;
        if (volt_avg < 1) {
    		vid = -1;
    		goto out;
    	}

		mcompat_log(MCOMPAT_LOG_NOTICE, "Chain %d VID %d voltage %.1f", chain_id, vid, volt_avg);
	}

	while (1)
	{
		(volt_diff_prev < 0) ? (vid--) : (vid++);
        if (vid > VID_MAX)
			vid = VID_MAX;
		else if (vid < VID_MIN)
			vid = VID_MIN;
		mcompat_set_vid(chain_id, vid);
		sleep(3);
		mcompat_get_chip_volt(chain_id, chip_volt);
		volt_avg = mcompat_get_average_volt(chip_volt, chip_num);
		volt_diff = volt_avg - volt_target;
        if (volt_avg < 1) {
    		vid = -1;
    		goto out;
    	}

		mcompat_log(MCOMPAT_LOG_NOTICE, "Chain %d VID %d voltage %.1f", chain_id, vid, volt_avg);

		if (volt_diff_prev < 0)
		{ // voltage increasing, vid decreasing
			// finish searching if the sign of volt_diff is changed 
			// or reaches the minimal vid
			if (volt_diff >= 0 || vid == VID_MIN)
			{ 
				if (-volt_diff_prev < volt_diff)
				{ // the last vid is closer to the target
				    mcompat_set_vid(chain_id, ++vid);
				    usleep(500000);
				}
				break;
			}
		}
		else
		{ // voltage decreasing, vid increasing
			// finish searching if the sign of volt_diff is changed 
			// or reaches the maximal vid
			if (volt_diff <= 0 || vid == VID_MAX)
			{
				if (volt_diff_prev < -volt_diff)
				{ // the last vid is closer to the target
				    mcompat_set_vid(chain_id, --vid);
				    usleep(500000);
				}
				break;
			}
		}

	    volt_diff_prev = volt_diff;
	}

out:
	mcompat_configure_tvsensor(chain_id, CMD_ADDR_BROADCAST, 1);

	return vid;
}

int mcompat_get_chip_tech()
{
    switch(g_miner_type)
	{
    case MCOMPAT_LIB_MINER_TYPE_T1:
    case MCOMPAT_LIB_MINER_TYPE_A9:
        return CHIP_TECH_10NM;
    case MCOMPAT_LIB_MINER_TYPE_A5:
    case MCOMPAT_LIB_MINER_TYPE_A6:
    case MCOMPAT_LIB_MINER_TYPE_A8:
    case MCOMPAT_LIB_MINER_TYPE_D9:
    case MCOMPAT_LIB_MINER_TYPE_S11:
        return CHIP_TECH_14NM;
    default:
        return CHIP_TECH_UNKNOWN;
	}
}

