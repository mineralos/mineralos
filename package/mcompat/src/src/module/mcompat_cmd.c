#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcompat_config.h"
#include "mcompat_cmd.h"
#include "mcompat_drv.h"
#include "drv_spi.h"
#include "drv_hub.h"
#include "drv_opi.h"
#include "spi_cmd.h"
#include "hub_cmd.h"
#include "opi_cmd.h"


MCOMPAT_CMD_T s_cmd_ops;
MCOMPAT_CMD_T* s_cmd_ops_p = &s_cmd_ops;


void init_mcompat_cmd(void)
{
    memset(&s_cmd_ops, 0, sizeof(s_cmd_ops));

    switch(g_platform)
    {
    case PLATFORM_ZYNQ_SPI_G9:
    case PLATFORM_ZYNQ_SPI_G19:
        init_spi_cmd(g_chain_num);
        s_cmd_ops_p->set_speed            = spi_set_spi_speed;
        s_cmd_ops_p->cmd_reset            = spi_cmd_reset;
        s_cmd_ops_p->cmd_bist_start       = spi_cmd_bist_start;
        s_cmd_ops_p->cmd_bist_fix         = spi_cmd_bist_fix;
        s_cmd_ops_p->cmd_bist_collect     = spi_cmd_bist_collect;
        s_cmd_ops_p->cmd_read_register    = spi_cmd_read_register;
        s_cmd_ops_p->cmd_write_register   = spi_cmd_write_register;
        s_cmd_ops_p->cmd_read_write_reg0d = spi_cmd_read_write_reg0d;
        s_cmd_ops_p->cmd_read_result      = spi_cmd_read_result;
        s_cmd_ops_p->cmd_write_job        = spi_cmd_write_job;
        break;
    case PLATFORM_ZYNQ_HUB_G9:
    case PLATFORM_ZYNQ_HUB_G19:
        hub_init();
        init_hub_cmd(g_chain_num, g_chip_num);
        s_cmd_ops_p->set_speed            = hub_set_spi_speed;
        s_cmd_ops_p->cmd_reset            = hub_cmd_reset;
        s_cmd_ops_p->cmd_bist_start       = hub_cmd_bist_start;
        s_cmd_ops_p->cmd_bist_fix         = hub_cmd_bist_fix;
        s_cmd_ops_p->cmd_bist_collect     = hub_cmd_bist_collect;
        s_cmd_ops_p->cmd_read_register    = hub_cmd_read_register;
        s_cmd_ops_p->cmd_write_register   = hub_cmd_write_register;
        s_cmd_ops_p->cmd_read_write_reg0d = hub_cmd_read_write_reg0d;
        s_cmd_ops_p->cmd_read_result      = hub_cmd_read_result;
        s_cmd_ops_p->cmd_write_job        = hub_cmd_write_job;
        s_cmd_ops_p->cmd_auto_nonce       = hub_cmd_auto_nonce;
        s_cmd_ops_p->cmd_read_nonce       = hub_cmd_read_nonce;
        break;
    case PLATFORM_ORANGE_PI:
        init_opi_cmd();
        s_cmd_ops_p->set_speed            = opi_set_spi_speed;
        s_cmd_ops_p->cmd_reset            = opi_cmd_reset;
        s_cmd_ops_p->cmd_bist_start       = opi_cmd_bist_start;
        s_cmd_ops_p->cmd_bist_fix         = opi_cmd_bist_fix;
        s_cmd_ops_p->cmd_bist_collect     = opi_cmd_bist_collect;
        s_cmd_ops_p->cmd_read_register    = opi_cmd_read_register;
        s_cmd_ops_p->cmd_write_register   = opi_cmd_write_register;
        s_cmd_ops_p->cmd_read_write_reg0d = opi_cmd_read_write_reg0d;
        s_cmd_ops_p->cmd_read_result      = opi_cmd_read_result;
        s_cmd_ops_p->cmd_write_job        = opi_cmd_write_job;
        break;
    default:
        mcompat_log_err("the platform is undefined !!!");
        break;
    }
}


void exit_mcompat_cmd(void)
{
    switch(g_platform)
    {
    case PLATFORM_ZYNQ_SPI_G9:
    case PLATFORM_ZYNQ_SPI_G19:
        exit_spi_cmd(g_chain_num);
        break;
    case PLATFORM_ZYNQ_HUB_G9:
    case PLATFORM_ZYNQ_HUB_G19:
        hub_deinit();
        exit_hub_cmd(g_chain_num);
        break;
    default:
        mcompat_log_err("the platform is undefined !!!");
        break;
    }
}

void register_mcompat_cmd(MCOMPAT_CMD_T * ops)
{
    if(ops->set_speed != NULL)
    {
        s_cmd_ops_p->set_speed = ops->set_speed;
    }
    if(ops->cmd_reset != NULL)
    {
        s_cmd_ops_p->cmd_reset = ops->cmd_reset;
    }
    if(ops->cmd_bist_start != NULL)
    {
        s_cmd_ops_p->cmd_bist_start = ops->cmd_bist_start;
    }
    if(ops->cmd_bist_fix != NULL)
    {
        s_cmd_ops_p->cmd_bist_fix = ops->cmd_bist_fix;
    }
    if(ops->cmd_bist_collect != NULL)
    {
        s_cmd_ops_p->cmd_bist_collect = ops->cmd_bist_collect;
    }
    if(ops->cmd_read_register != NULL)
    {
        s_cmd_ops_p->cmd_read_register = ops->cmd_read_register;
    }
    if(ops->cmd_write_register != NULL)
    {
        s_cmd_ops_p->cmd_write_register = ops->cmd_write_register;
    }
    if(ops->cmd_read_result != NULL)
    {
        s_cmd_ops_p->cmd_read_result = ops->cmd_read_result;
    }
    if(ops->cmd_write_job != NULL)
    {
        s_cmd_ops_p->cmd_write_job = ops->cmd_write_job;
    }
}

bool mcompat_set_spi_speed(unsigned char chain_id, int index)
{
    if(s_cmd_ops_p->set_speed == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    s_cmd_ops_p->set_speed(chain_id, index);
    return true;
}

bool mcompat_cmd_reset(unsigned char chain_id, unsigned char chip_id, unsigned char *in, unsigned char *out)
{
    if(s_cmd_ops_p->cmd_reset == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_reset(chain_id, chip_id, in, out);
}

int mcompat_cmd_bist_start(unsigned char chain_id, unsigned char chip_id)
{
    if(s_cmd_ops_p->cmd_bist_start == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_bist_start(chain_id, chip_id);
}

bool mcompat_cmd_bist_collect(unsigned char chain_id, unsigned char chip_id)
{
    if(s_cmd_ops_p->cmd_bist_collect == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_bist_collect(chain_id, chip_id);
}

bool mcompat_cmd_bist_fix(unsigned char chain_id, unsigned char chip_id)
{
    if(s_cmd_ops_p->cmd_bist_fix == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_bist_fix(chain_id, chip_id);
}

bool mcompat_cmd_write_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len)
{
    if(s_cmd_ops_p->cmd_write_register == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_write_register(chain_id, chip_id, reg, len);
}


bool mcompat_cmd_read_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len)
{
    if(s_cmd_ops_p->cmd_read_register == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_read_register(chain_id, chip_id, reg, len);
}


bool mcompat_cmd_read_write_reg0d(unsigned char chain_id, unsigned char chip_id, unsigned char *in, int len, unsigned char *out)
{
    if(s_cmd_ops_p->cmd_read_write_reg0d == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_read_write_reg0d(chain_id, chip_id, in, len, out);
}


bool mcompat_cmd_write_job(unsigned char chain_id, unsigned char chip_id, unsigned char *job, int len)
{
    if(s_cmd_ops_p->cmd_write_job == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_write_job(chain_id, chip_id, job, len);
}


bool mcompat_cmd_read_result(unsigned char chain_id, unsigned char chip_id, unsigned char *res, int len)
{
    if(s_cmd_ops_p->cmd_read_result == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_read_result(chain_id, chip_id, res, len);
}

bool mcompat_cmd_auto_nonce(unsigned char chain_id, int mode, int len)
{
    if(s_cmd_ops_p->cmd_auto_nonce == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_auto_nonce(chain_id, mode, len);
}

bool mcompat_cmd_read_nonce(unsigned char chain_id, unsigned char *res, int len)
{
    if(s_cmd_ops_p->cmd_read_nonce == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    return s_cmd_ops_p->cmd_read_nonce(chain_id, res, len);
}

 bool mcompat_cmd_get_temp( mcompat_fan_temp_s * fan_temp)
 {
     if(s_cmd_ops_p->cmd_get_temp== NULL)
     {
         mcompat_log_err("%s not register !\n", __FUNCTION__);
         return false;
     }

     return s_cmd_ops_p->cmd_get_temp(fan_temp);
 }

