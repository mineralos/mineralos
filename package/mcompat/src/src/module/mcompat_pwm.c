#include "mcompat_config.h"
#include "zynq_pwm.h"
#include "drv_hub.h"
#include "drv_opi.h"
#include "mcompat_pwm.h"
#include "mcompat_drv.h"



MCOMPAT_PWM_T s_pwm_ops;
MCOMPAT_PWM_T* s_pwm_ops_p = &s_pwm_ops;



void init_mcompat_pwm(void)
{
    memset(&s_pwm_ops, 0, sizeof(s_pwm_ops));

    switch(g_platform)
    {
    case PLATFORM_ZYNQ_SPI_G9:
    case PLATFORM_ZYNQ_SPI_G19:
        s_pwm_ops_p->set_pwm    = zynq_set_pwm;
        break;
    case PLATFORM_ZYNQ_HUB_G9:
    case PLATFORM_ZYNQ_HUB_G19:
        s_pwm_ops_p->set_pwm    = hub_set_pwm;
        break;    
    case PLATFORM_ORANGE_PI:
        s_pwm_ops_p->set_pwm    = opi_set_pwm;
    default:
        mcompat_log_err("the platform is undefined !!!");
        break;
    }
}

void exit_mcompat_pwm(void)
{
    switch(g_platform)
    {
    case PLATFORM_ZYNQ_SPI_G9:
    case PLATFORM_ZYNQ_SPI_G19:
        break;
    case PLATFORM_ZYNQ_HUB_G9:
    case PLATFORM_ZYNQ_HUB_G19:
        break;
    default:
        mcompat_log_err("the platform is undefined !!!");
        break;
    }
}


void register_mcompat_pwm(MCOMPAT_PWM_T * ops)
{
    if(ops->set_pwm != NULL)
    {
        s_pwm_ops_p->set_pwm = ops->set_pwm;
    }
}


void mcompat_set_pwm(unsigned char fan_id, int frequency, int duty)
{
    if(s_pwm_ops_p->set_pwm == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return;
    }

    mcompat_log(MCOMPAT_LOG_DEBUG, "set fan[%d] pwm freq[%d] duty[%d] \n", fan_id, frequency, duty);
    
    s_pwm_ops_p->set_pwm(fan_id, frequency, duty);
}


