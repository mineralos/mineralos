#include "mcompat_config.h"
#include "mcompat_chain.h"
#include "mcompat_drv.h"
#include "drv_zynq.h"
#include "drv_opi.h"


MCOMPAT_CHAIN_T s_chain_ops;
MCOMPAT_CHAIN_T* s_chain_ops_p = &s_chain_ops;


void init_mcompat_chain(void)
{
    memset(&s_chain_ops, 0, sizeof(s_chain_ops));

    switch(g_platform)
    {
    case PLATFORM_ZYNQ_SPI_G9:
    case PLATFORM_ZYNQ_SPI_G19:
    case PLATFORM_ZYNQ_HUB_G9:
    case PLATFORM_ZYNQ_HUB_G19:
        s_chain_ops_p->power_on         = zynq_chain_power_on;
        s_chain_ops_p->power_down       = zynq_chain_power_down;
        s_chain_ops_p->hw_reset         = zynq_chain_hw_reset;
        s_chain_ops_p->power_on_all     = zynq_chain_power_on_all;
        s_chain_ops_p->power_down_all   = zynq_chain_power_down_all;
        break;
    case PLATFORM_ORANGE_PI:
        s_chain_ops_p->power_on         = opi_chain_power_on;
        s_chain_ops_p->power_down       = opi_chain_power_down;
        s_chain_ops_p->hw_reset         = opi_chain_hw_reset;
        s_chain_ops_p->power_on_all     = opi_chain_power_on_all;
        s_chain_ops_p->power_down_all   = opi_chain_power_down_all;
        break;
    default:
        mcompat_log_err("the platform is undefined !!!");
        break;
    }
}


void exit_mcompat_chain(void)
{
    switch(g_platform)
    {
    case PLATFORM_ZYNQ_SPI_G9:
    case PLATFORM_ZYNQ_SPI_G19:
    case PLATFORM_ZYNQ_HUB_G9:
    case PLATFORM_ZYNQ_HUB_G19:
        break;
    case PLATFORM_ORANGE_PI:
        break;
    default:
        mcompat_log_err("the platform is undefined !!!");
        break;
    }
}


void register_mcompat_chain(MCOMPAT_CHAIN_T * ops)
{
    if(ops->power_on != NULL)
    {
        s_chain_ops_p->power_on = ops->power_on;
    }
    if(ops->power_down != NULL)
    {
        s_chain_ops_p->power_down = ops->power_down;
    }
    if(ops->hw_reset != NULL)
    {
        s_chain_ops_p->hw_reset = ops->hw_reset;
    }
    if(ops->power_on_all != NULL)
    {
        s_chain_ops_p->power_on_all = ops->power_on_all;
    }
    if(ops->power_down_all != NULL)
    {
        s_chain_ops_p->power_down_all = ops->power_down_all;
    }
}


bool mcompat_chain_power_on(unsigned char chain_id, unsigned char mode)
{
    if(s_chain_ops_p->power_on == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }
    
    return s_chain_ops_p->power_on(chain_id, mode);
}


bool mcompat_chain_power_down(unsigned char chain_id)
{
    if(s_chain_ops_p->power_down == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }
    
    return s_chain_ops_p->power_down(chain_id);
}


bool mcompat_chain_hw_reset(unsigned char chain_id)
{
    if(s_chain_ops_p->hw_reset == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }
    
    return s_chain_ops_p->hw_reset(chain_id);
}

bool mcompat_chain_power_on_all(void)
{
    if(s_chain_ops_p->power_on_all == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }
    
    return s_chain_ops_p->power_on_all();
}


bool mcompat_chain_power_down_all(void)
{
    if(s_chain_ops_p->power_down_all == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }
    
    return s_chain_ops_p->power_down_all();
}


