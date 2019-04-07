/******************************************************************************
 *
 * 文件名  ： hub_hardware.c
 * 负责人  ： pengp
 * 创建日期： 20180307
 * 版本号  ： v1.0
 * 文件描述： hub的硬件屏蔽层实现
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <sys/mman.h>

#include "hub_hardware.h"
#include "mcompat_config.h"
#include "mcompat_log.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/

/********************************** 变量实现区 *********************************/
typedef struct HUB_DEV_TAG {
    volatile uint8_t *vir_base;
    uint32_t          phy_addr;
    const char       *name;
} HUB_DEV_T;

static HUB_DEV_T s_dev_list[] = {
    {NULL, 0x43C30000, "spi"},
    {NULL, 0x43C10000, "peripheral"},
    {NULL, 0x43C00000, "fans"},
    {NULL, 0x41200000, "gpio"},
    {NULL, 0x43C32000, "sha256"},
};

/********************************** 函数实现区 *********************************/
void hub_hardware_init(void)
{
    int fd = 0;
    int i = 0;
    int iMax = sizeof(s_dev_list) / sizeof(s_dev_list[0]);

    mcompat_log(MCOMPAT_LOG_INFO, "max range: 0x%x.\n", _MAX_MEM_RANGE);

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(-1 == fd)
    {
        mcompat_log_err("open /dev/mem:");
        return;
    } 
    
    mcompat_log(MCOMPAT_LOG_INFO, "total: %d dev will mmap.\n", iMax);
    for(i = 0; i < iMax; i++)
    {
        s_dev_list[i].vir_base = mmap(NULL, _MAX_MEM_RANGE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, s_dev_list[i].phy_addr);
        if(MAP_FAILED == s_dev_list[i].vir_base)
        {
            close(fd);
            mcompat_log_err("mmap %s:phy:0x%08x => vir:%p fail.\n", s_dev_list[i].name, s_dev_list[i].phy_addr, s_dev_list[i].vir_base);
            return;
        }

        mcompat_log(MCOMPAT_LOG_INFO, "mmap %s:phy:0x%08x => vir:%p ok.\n", s_dev_list[i].name, s_dev_list[i].phy_addr, s_dev_list[i].vir_base);
    }

    mcompat_log(MCOMPAT_LOG_INFO, "total: %d dev mmap done.\n", iMax);

    close(fd);
}

void hub_hardware_deinit(void)
{
    int i = 0;
    int iMax = sizeof(s_dev_list) / sizeof(s_dev_list[0]);

    for(i = 0; i < iMax; i++)
    {
        munmap((void *)s_dev_list[i].vir_base, _MAX_MEM_RANGE);
        mcompat_log(MCOMPAT_LOG_INFO, "unmap %s:vir:%p.\n", s_dev_list[i].name, s_dev_list[i].vir_base);
    }
}

