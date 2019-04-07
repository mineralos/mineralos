/******************************************************************************
 *
 * 文件名  ： dm_test
 * 负责人  ： 彭鹏
 * 创建日期： 20171002 
 * 版本号  ： v1.0
 * 文件描述： 测试平台库的代码
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <stdio.h>
#include <unistd.h>

#include "zynq_gpio.h"

#include "mcompat_lib.h"
#include "mcompat_drv.h"
#include "mcompat_log.h"



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
    int i = 0;
    int iMax = 0;
    int val = 0;
    int type = 0;

    int rst = 0;

    int platform = 4;
    int miner_type = 0;
    int chain_num = 8;
    int chip_num = 80;

    mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:sys_platform_init %d,%d,%d,%d.\n", __FILE__, __LINE__, platform, miner_type, chain_num, chip_num);

    /* 初始化 */
    rst = sys_platform_init(platform, miner_type, chain_num, chip_num);
    if(!rst) /* 失败 */
    {
        mcompat_log_err("sys_platform_init: platform[%d] miner_type[%d] chain_num[%d] chip_num[%d].\n", platform, miner_type, chain_num, chip_num);
    }

#if 1
    i = 0;
    iMax = 10;
    for(i = 0; i < iMax; i++)
    {
        type = misc_get_vid_type();
        mcompat_log(MCOMPAT_LOG_INFO, "%d times vid type:%d\n", i, type);
        sleep(2);
    }
#endif

#if 1
    int list[] = { 1, 15, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 46, 50, 51};
    i = 0;
    iMax = sizeof(list) / sizeof(list[0]);
    for(i = 0; i < iMax; i++)
    {
        mcompat_log(MCOMPAT_LOG_INFO, "mio%d ", list[i]);
        zynq_gpio_init(906 + list[i], 1);
        sync();
        usleep(50000);
        val = zynq_gpio_read(906 + list[i]); 
        mcompat_log(MCOMPAT_LOG_INFO, "is %d\n", val);
        sync();
        usleep(50000);
    }
#endif

#if 1
    i = 0;
    iMax = 32;
    int j = 0;
    for(i = 0; i < iMax; i++)
    {
        for(j = 0; j < 8; j++)
        {
            mcompat_set_vid(j, i);
            mcompat_log(MCOMPAT_LOG_INFO, "chain %d set vid %d.\n", j, i);
        }
        mcompat_log_nt(MCOMPAT_LOG_INFO, "\n\n");
        sleep(1);
    }
#endif

#if 1
    i = 0;
    iMax = 100;

    for(i = 0; i < iMax; i += 10)
    {
        mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:duty: %d.\n", __FILE__, __LINE__, i);
        mcompat_set_pwm(0, ASIC_MCOMPAT_FAN_PWM_FREQ_TARGET, i);
        mcompat_set_pwm(1, ASIC_MCOMPAT_FAN_PWM_FREQ_TARGET, i);
        mcompat_set_pwm(2, ASIC_MCOMPAT_FAN_PWM_FREQ_TARGET, i);
        sleep(1);
    }
#endif

    rst = sys_platform_exit();
    if(!rst) /* 失败 */
    {
        mcompat_log_err("sys_platform_init: platform[%d] miner_type[%d] chain_num[%d] chip_num[%d].\n", platform, miner_type, chain_num, chip_num);
    }

    mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:sys_platform_exit.\n", __FILE__, __LINE__);

    return 0;
}

