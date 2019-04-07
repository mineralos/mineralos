#include "mcompat_config.h"
#include "mcompat_drv.h"
#include "mcompat_cmd.h"
#include "mcompat_pwm.h"
#include "mcompat_gpio.h"
#include "mcompat_chain.h"
#include "spi_cmd.h"
#include "hub_cmd.h"


int g_platform;
int g_miner_type;
int g_chain_num;
int g_chip_num;
int g_vid_type;

void sys_platform_debug_init(int debug_level)
{
    mcompat_log_init(debug_level, "mcompat_lib");
}

bool sys_platform_init(int platform, int miner_type, int chain_num, int chip_num)
{
    mcompat_log(MCOMPAT_LOG_NOTICE, "sys : platform[%d] miner_type[%d] chain_num[%d] chip_num[%d] \n", platform, miner_type, chain_num, chip_num);

    switch(platform)
    {
        case PLATFORM_ZYNQ_SPI_G9:
        case PLATFORM_ZYNQ_SPI_G19:
        case PLATFORM_ZYNQ_HUB_G9:
        case PLATFORM_ZYNQ_HUB_G19:
        case PLATFORM_SOC:
            break;
        default:
            mcompat_log_err("the platform is undefined !!! \n");
            break;
    }

    if(chain_num > MCOMPAT_CONFIG_MAX_CHAIN_NUM)
    {
        mcompat_log_err("the chain_num is error !!! \n");
        return false;
    }

    if(chip_num > MCOMPAT_CONFIG_MAX_CHIP_NUM)
    {
        mcompat_log_err("the chip_num is error !!! \n");
        return false;
    }

    g_platform = platform;
    g_miner_type = miner_type;
    g_chain_num = chain_num;
    g_chip_num = chip_num;

    init_mcompat_cmd();

    init_mcompat_gpio();

    init_mcompat_pwm();

    init_mcompat_chain();

    g_vid_type = misc_get_vid_type();

    return true;
}


bool sys_platform_exit(void)
{
    exit_mcompat_cmd();

    exit_mcompat_gpio();

    exit_mcompat_pwm();

    exit_mcompat_chain();
    
    return true;
}

