/******************************************************************************
 *
 * 文件名  ： dm_chip_type.c
 * 负责人  ： pengp
 * 创建日期： 20170928
 * 版本号  ： v1.0
 * 文件描述： 矿机类型判断
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 矿机多条链全部为同一个类型的算力板
 *            使用mcompat中的库函数
 * 修改日志： 无
 *
 *******************************************************************************/


/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include "mcompat_config.h"
#include "mcompat_lib.h"
#include "dm_chip_type.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/
/*******************************************************************************
 *
 * 函数名  : power_up
 * 描述    : 链上电
 * 输入参数: chain_id 链编号
 * 输出参数: 无
 * 返回值:   true  上电成功
 *           false 上电失败(或未插入)
 *
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
static bool power_on(unsigned char chain_id);

/*******************************************************************************
 *
 * 函数名  : usage
 * 描述    : 输出算力芯片检测的使用说明
 * 输入参数: prog_name 程序名
 * 输出参数: 无
 * 返回值:   无
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
static void usage(const char *prog_name);

/********************************** 变量实现区 *********************************/


/********************************** 变量实现区 *********************************/

/********************************** 函数实现区 *********************************/
/*******************************************************************************
 *
 * 函数名  : main
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 主函数
 *
 * 输入参数: argc - 参数个数
 *           argv - 命令行参数数组
 *
 * 输出参数: 无
 *
 * 返回值:   0   : 正常退出
 *           其它: 异常退出
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
int main(int argc, char *argv[])
{
    int platform = 0;
    int miner_type = 0; /* 检测前未知 */
    int chain_num = 0;
    int chip_num = 0;

    bool rst = false;
    bool type_is_ok = false;
    bool chip_num_is_ok = false;
    FILE *fp = NULL;
    char *type_path = NULL;
    char *chip_num_path = NULL;
    char *hwver_path = NULL;
    char *dev_type_path = NULL;
    int real_chip_num = 0;

    char hwver_str[128] = {0};
    char dev_type_str[128] = {0};

    /* 复位参数及缓存 */
    int i = 0;
    unsigned char spi_tx[2] = {0};
    unsigned char spi_rx[4] = {0};
    unsigned char type_bits = 0;
    char *type_str = DM_CHIP_TYPE_ERR;

    if(argc < 9)
    {
        usage(argv[0]);
        exit(0);
    }

    platform = atoi(argv[1]);
    miner_type = atoi(argv[2]);
    chain_num = atoi(argv[3]);
    chip_num = atoi(argv[4]);

    type_path = argv[5];
    chip_num_path = argv[6];

    hwver_path = argv[7];
    dev_type_path = argv[8];

    mcompat_log_nt(MCOMPAT_LOG_DEBUG, "%d,%d,%d,%d,%s,%s,%s,%s.\n", platform, miner_type, chain_num, chip_num, type_path, chip_num_path, hwver_path, dev_type_path);

    /* 初始化 */
    rst = sys_platform_init(platform, miner_type, chain_num, chip_num);
    if(!rst) /* 失败 */
    {
        mcompat_log_err("sys_platform_init: platform[%d] miner_type[%d] chain_num[%d] chip_num[%d].\n", platform, miner_type, chain_num, chip_num);
    }

    /* 遍历链,获取需要的信息 */
    for(i = 0; i < chain_num; i++)
    {
        /* step1: 上电流程 */
        rst = power_on(i);
        if(!rst)/* 未插入或上电失败 */
        {
            continue;
        }

        /* step2: 设置spi速度 */
        rst = mcompat_set_spi_speed(i, SPI_SPEED_390K);
        if(!rst)
        {
            mcompat_log_nt(MCOMPAT_LOG_DEBUG, "spi %d speed set to SPI_SPEED_390K failled.\n", i);
            continue;
        }

        /* step2: 获取类型 */
        if(!type_is_ok)
        {
            rst = mcompat_cmd_reset(i, 0x00, spi_tx, spi_rx); /* 广播复位 */
            if(rst) /* 类型获取成功 */
            {
                mcompat_log_nt(MCOMPAT_LOG_DEBUG, "recv data:%02x,%02x\n", spi_rx[0], spi_rx[1]);
                type_is_ok = true;
            }
        }

        /* step3: 获取芯片数目 */
        if(!chip_num_is_ok)
        {
            real_chip_num = mcompat_cmd_bist_start(i, 0x00);
            if(0 != real_chip_num)
            {
                chip_num_is_ok = true;
            }

        }

        if(type_is_ok && chip_num_is_ok)
        {
            break;
        }
    }

    /* 依据返回值最高4bit判断类型 */
    type_bits = (0x0f & (spi_rx[0] >> 4));
    switch(type_bits)
    {
        case 0x0:
            type_str = DM_CHIP_TYPE_A5;
            break;

        case 0x6:
            type_str = DM_CHIP_TYPE_A6;
            break;

        case 0xb:
            type_str = DM_CHIP_TYPE_A7;
            break;

        case 0x3:
            type_str = DM_CHIP_TYPE_A8;
            break;

        case 0x7:
            type_str = DM_CHIP_TYPE_A11;
            break;

        case 0xc:
            type_str = DM_CHIP_TYPE_A12;
            break;

        default:
            type_str = DM_CHIP_TYPE_ERR;
            break;
    }

    /* 写入到文件 */
    memset(hwver_str, 0, sizeof(hwver_str));
    misc_system("/sbin/fw_printenv -n hwver\n", hwver_str, sizeof(hwver_str));
    fp = fopen(hwver_path, "w");
    if(NULL == fp)
    {
        mcompat_log_err("fopen %s failled.\n", hwver_path);
    }
    fprintf(stderr, "%s,%d:%s,%d\n", __FILE__, __LINE__, hwver_str, strlen(hwver_str));
    fprintf(fp, "%s", hwver_str);
    fclose(fp);
    fp = NULL;

    memset(dev_type_str, 0, sizeof(dev_type_str));
    misc_system("/sbin/fw_printenv -n dev_type\n", dev_type_str, sizeof(dev_type_str));
    fp = fopen(dev_type_path, "w");
    if(NULL == fp)
    {
        mcompat_log_err("fopen %s failled.\n", dev_type_path);
    }
    fprintf(fp, "%s", dev_type_str);
    fclose(fp);
    fp = NULL;

    fp = fopen(type_path, "w");
    if(NULL == fp)
    {
        mcompat_log_err("fopen %s failled.\n", type_path);
    }
    fprintf(fp, "%s", type_str);
    fclose(fp);
    fp = NULL;

    fp = fopen(chip_num_path, "w");
    if(NULL == fp)
    {
        mcompat_log_err("fopen %s failled.\n", chip_num_path);
    }
    fprintf(fp, "%d", real_chip_num);
    fclose(fp);

    rst = sys_platform_exit();
    if(!rst) /* 失败 */
    {
        mcompat_log_err("sys_platform_init: platform[%d] miner_type[%d] chain_num[%d] chip_num[%d].\n", platform, miner_type, chain_num, chip_num);
    }

    return 0;
}

static bool power_on(unsigned char chain_id)
{
    mcompat_set_power_en(chain_id, 0);

    if(1 == mcompat_get_plug(chain_id))
    {
        mcompat_log(MCOMPAT_LOG_DEBUG, "spi %d not inserted.\n", chain_id);
        return false;
    }
    sleep(1);

    mcompat_set_start_en(chain_id, 0);
    mcompat_set_reset(chain_id, 0);
    sleep(5);

    mcompat_set_power_en(chain_id, 1);
    sleep(5);

    mcompat_set_reset(chain_id, 1);
    sleep(1);

    mcompat_set_start_en(chain_id, 1);
    sleep(1);

    return true;
}

static void usage(const char *prog_name)
{
    mcompat_log_nt(MCOMPAT_LOG_ERR, "USAGE:\n");
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      %s platform miner_type chain_num chip_num type_path chip_num_path hwver_path dev_type_path\n", prog_name);
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      platform:\n");
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      - ZYNQ_SPI_G9: %d\n", PLATFORM_ZYNQ_SPI_G9);
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      - ZYNQ_SPI_G19:%d\n", PLATFORM_ZYNQ_SPI_G19);
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      - ZYNQ_HUB_G9: %d\n", PLATFORM_ZYNQ_HUB_G9);
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      - ZYNQ_HUB_G19:%d\n", PLATFORM_ZYNQ_HUB_G19);
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      - SOC:         %d\n", PLATFORM_SOC);
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      miner_type is 0\n");
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      chain_num  is 8(max chain nums)\n");
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      chip_num   is 80(max chip nums)\n");
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      type_path  is /tmp/type(type file path)\n");
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      chip_num   is /tmp/chip_nums(chip nums file path)\n");
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      hwver_path is /tmp/hwver(hwver file path)\n");
    mcompat_log_nt(MCOMPAT_LOG_ERR, "      dev_type_path is /tmp/dev_type_path(dev_type nums file path)\n");
}

