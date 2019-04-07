#include "mcompat_config.h"
#include "zynq_spi.h"
#include "drv_spi.h"
#include "drv_hub.h"
#include "drv_opi.h"
#include "mcompat_gpio.h"
#include "mcompat_drv.h"



MCOMPAT_GPIO_T s_gpio_ops;
MCOMPAT_GPIO_T* s_gpio_ops_p = &s_gpio_ops;



void init_mcompat_gpio(void)
{
    memset(&s_gpio_ops, 0, sizeof(s_gpio_ops));

    switch(g_platform)
    {
    case PLATFORM_ZYNQ_SPI_G9:
    case PLATFORM_ZYNQ_SPI_G19:
        init_spi_gpio(g_chain_num);
        s_gpio_ops_p->set_power_en  = spi_set_power_en;
        s_gpio_ops_p->set_start_en  = spi_set_start_en;
        s_gpio_ops_p->set_reset     = spi_set_reset;
        s_gpio_ops_p->set_led       = spi_set_led;
        s_gpio_ops_p->get_plug      = spi_get_plug;
        s_gpio_ops_p->set_vid       = spi_set_vid;
        break;
    case PLATFORM_ZYNQ_HUB_G9:
    case PLATFORM_ZYNQ_HUB_G19:
        init_hub_gpio();
        s_gpio_ops_p->set_power_en  = hub_set_power_en;
        s_gpio_ops_p->set_start_en  = hub_set_start_en;
        s_gpio_ops_p->set_reset     = hub_set_reset;
        s_gpio_ops_p->set_led       = hub_set_led;
        s_gpio_ops_p->get_plug      = hub_get_plug;
        s_gpio_ops_p->set_vid       = hub_set_vid;
        s_gpio_ops_p->get_button    = hub_get_button;
        s_gpio_ops_p->set_green_led = hub_set_green_led;
        s_gpio_ops_p->set_red_led   = hub_set_red_led;
        break;
    case PLATFORM_ORANGE_PI:
        s_gpio_ops_p->set_power_en  = opi_set_power_en;
        s_gpio_ops_p->set_start_en  = opi_set_start_en;
        s_gpio_ops_p->set_reset     = opi_set_reset;
        s_gpio_ops_p->set_led       = opi_set_led;
        s_gpio_ops_p->get_plug      = opi_get_plug;
        s_gpio_ops_p->set_vid       = opi_set_vid;
        break;
    default:
        mcompat_log_err("the platform is undefined !!!");
        break;
    }
}

void exit_mcompat_gpio(void)
{
    switch(g_platform)
    {
    case PLATFORM_ZYNQ_SPI_G9:
    case PLATFORM_ZYNQ_SPI_G19:
        exit_spi_gpio(g_chain_num);
        break;
    case PLATFORM_ZYNQ_HUB_G9:
    case PLATFORM_ZYNQ_HUB_G19:
        break;
    default:
        mcompat_log_err("the platform is undefined !!!");
        break;
    }
}


void register_mcompat_gpio(MCOMPAT_GPIO_T * ops)
{
    if(ops->set_power_en != NULL)
    {
        s_gpio_ops_p->set_power_en = ops->set_power_en;
    }
    if(ops->set_start_en != NULL)
    {
        s_gpio_ops_p->set_start_en = ops->set_start_en;
    }
    if(ops->set_reset != NULL)
    {
        s_gpio_ops_p->set_reset = ops->set_reset;
    }
    if(ops->set_led != NULL)
    {
        s_gpio_ops_p->set_led = ops->set_led;
    }
    if(ops->get_plug != NULL)
    {
        s_gpio_ops_p->get_plug = ops->get_plug;
    }
    if(ops->get_button != NULL)
    {
        s_gpio_ops_p->get_button = ops->get_button;
    }
    if(ops->set_green_led != NULL)
    {
        s_gpio_ops_p->set_green_led = ops->set_green_led;
    }
    if(ops->set_red_led != NULL)
    {
        s_gpio_ops_p->set_red_led = ops->set_red_led;
    }
}


void mcompat_set_power_en(unsigned char chain_id, int val)
{
    if(s_gpio_ops_p->set_power_en == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return;
    }

    s_gpio_ops_p->set_power_en(chain_id, val);
}


void mcompat_set_start_en(unsigned char chain_id, int val)
{
    if(s_gpio_ops_p->set_start_en == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return;
    }

    s_gpio_ops_p->set_start_en(chain_id, val);
}


void mcompat_set_reset(unsigned char chain_id, int val)
{
    if(s_gpio_ops_p->set_reset == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return;
    }

    s_gpio_ops_p->set_reset(chain_id, val);
}

void mcompat_set_led(unsigned char chain_id, int val)
{
    if(s_gpio_ops_p->set_led == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return;
    }

    s_gpio_ops_p->set_led(chain_id, val);
}


int mcompat_get_plug(unsigned char chain_id)
{
    if(s_gpio_ops_p->get_plug == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return -1;
    }

    return s_gpio_ops_p->get_plug(chain_id);
}


bool mcompat_set_vid(unsigned char chain_id, int val)
{
    if(s_gpio_ops_p->set_vid == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return false;
    }

    mcompat_log(MCOMPAT_LOG_DEBUG, "set chain %d vid value %d\n", chain_id, val);

    return s_gpio_ops_p->set_vid(chain_id, val);
}

bool mcompat_set_vid_by_step(
	unsigned char chain_id, int start_vid, int target_vid)
{
	int i;

	if(target_vid > 31)
		target_vid = 31;
	else if(target_vid < 0)
		target_vid =0;

	if (target_vid > start_vid)
	{
		// increase vid step by step
		for (i = start_vid + 1; i <= target_vid; ++i)
		{
			mcompat_set_vid(chain_id, i);
			mcompat_log(MCOMPAT_LOG_DEBUG, "set_vid_value_G19: %d", i);
			usleep(500000);
		}
	}
	else if (target_vid < start_vid)
	{
		// decrease vid step by step
		for (i = start_vid - 1; i >= target_vid; --i)
		{
			mcompat_set_vid(chain_id, i);
			mcompat_log(MCOMPAT_LOG_DEBUG, "set_vid_value_G19: %d", i);
			usleep(500000);
		}
	}

	return true;
}

int mcompat_get_button(void)
{
    if(s_gpio_ops_p->get_button == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return -1;
    }

    return s_gpio_ops_p->get_button();
}


void mcompat_set_green_led(int mode)
{
    if(s_gpio_ops_p->set_green_led == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return;
    }

    s_gpio_ops_p->set_green_led(mode);
}

void mcompat_set_red_led(int mode)
{
    if(s_gpio_ops_p->set_red_led == NULL)
    {
        mcompat_log_err("%s not register !\n", __FUNCTION__);
        return;
    }

    s_gpio_ops_p->set_red_led(mode);
}

