#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcompat_config.h"
#include "mcompat_drv.h"


bool zynq_chain_power_on(unsigned char chain_id, unsigned char mode)
{
    int i = chain_id;

	if(mcompat_get_plug(i) != 0)
	{
		mcompat_log(MCOMPAT_LOG_NOTICE, "chain %d >>> the board not inserted !!! \n", i);
		return false;
	}	 

	/* Try different power-on modes */
	if (mode == 0) {
		mcompat_log(MCOMPAT_LOG_NOTICE, "chain%d: encore first mode", i);
		mcompat_set_reset(i, 1);
		sleep(1);
		mcompat_set_power_en(i, 1);
		sleep(1);
		mcompat_set_reset(i, 0);
		sleep(1);
		mcompat_set_start_en(i, 1);
		sleep(1);
		mcompat_set_reset(i, 1);
		sleep(1);
	} else if (mode == 1) {
		mcompat_log(MCOMPAT_LOG_NOTICE, "chain%d: encore second mode", i);
		mcompat_set_start_en(i, 1);
		mcompat_set_reset(i, 1);
		sleep(1);
		mcompat_set_power_en(i, 1);
		sleep(1);
		mcompat_set_start_en(i, 0);
		mcompat_set_reset(i, 0);
		sleep(1);
		mcompat_set_start_en(i, 1);
		mcompat_set_reset(i, 1);
		sleep(1);
	} else if (mode == 2) {
		mcompat_log(MCOMPAT_LOG_NOTICE, "chain%d: encore third mode", i);
		mcompat_set_power_en(i, 1);
		sleep(1);
		mcompat_set_start_en(i, 1);
		sleep(1);
		mcompat_set_reset(i, 1);
		sleep(1);
		mcompat_set_reset(i, 0);
		sleep(1);
		mcompat_set_reset(i, 1);
		sleep(1);
	} else {
		mcompat_log(MCOMPAT_LOG_NOTICE, "chain%d: encore fourth mode", i);
		mcompat_set_power_en(i, 1);
		sleep(1);
		mcompat_set_reset(i, 1);
//				sleep(1);
		mcompat_set_start_en(i, 1);
		sleep(1);
	}
    return true;
}


bool zynq_chain_power_down(unsigned char chain_id)
{
    mcompat_set_power_en(chain_id, 0);
    sleep(1);
    mcompat_set_reset(chain_id, 0);
    mcompat_set_start_en(chain_id, 0);
    mcompat_set_led(chain_id, 1);

    return true;
}


bool zynq_chain_hw_reset(unsigned char chain_id)
{
    mcompat_set_reset(chain_id, 0);
    sleep(1);
    mcompat_set_reset(chain_id, 1);
    sleep(1);

    return true;
}


bool zynq_chain_power_on_all(void)
{
    int i;

    for(i = 0; i < g_chain_num; i++)
    {
        if(mcompat_get_plug(i) != 0)
        {
            mcompat_log(MCOMPAT_LOG_NOTICE, "chain %d >>> the board not inserted !!! \n", i);
        }    
    }
    
    for(i = 0; i < g_chain_num; i++) {
        mcompat_set_power_en(i, 1);
        usleep(5000);
    }

    sleep(5);
    
    for(i = 0; i < g_chain_num; i++) {
        mcompat_set_reset(i, 1);
        usleep(5000);
    }

    sleep(1);
    
    for(i = 0; i < g_chain_num; i++) {
        mcompat_set_start_en(i, 1);
        usleep(5000);
    }
    
    return true;
}


bool zynq_chain_power_down_all(void)
{
    int i;
    
    for(i = 0; i < g_chain_num; i++) {
        mcompat_set_power_en(i, 0);
    }

    sleep(1);
    
    for(i = 0; i < g_chain_num; i++) {
        mcompat_set_reset(i, 0);
        mcompat_set_start_en(i, 0);
        mcompat_set_led(i, 1);
    }

    return true;
}


