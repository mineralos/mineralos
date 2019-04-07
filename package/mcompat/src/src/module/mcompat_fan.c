/******************************************************************************
 *
 * 文件名  ： mcompat_fan.c
 * 负责人  ： yex
 * 创建日期： 20171122
 * 版本号  ： v1.0
 * 文件描述： 风扇与温度控制相关实现
 * 版权说明： Copyright (c) 2000-2020   GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
#include "mcompat_fan.h"
#include "mcompat_drv.h"
#include "mcompat_lib.h"
#include "drv_hub.h"
#include <stdlib.h>

 /************************************ reslize ***********************************/
   int g_temp_hi_thr;
   int g_temp_lo_thr;
   int g_temp_start_thr;
   int g_dangerous_temp;
   int g_work_temp;
   int g_fan_speed;

 /********************************** function realize *********************************/
static int set_warn(int spi_id)
{
mcompat_set_power_en(spi_id, 0);
sleep(1);
mcompat_set_reset(spi_id, 0);
mcompat_set_start_en(spi_id, 0);
mcompat_set_led(spi_id, 1);

return 0;

}


static void mcompat_deal_temp(unsigned char spi_id, mcompat_fan_temp_s *fan_temp_ctrl)
{
    mcompat_temp_s *temp_ctrl = fan_temp_ctrl->mcompat_temp;

   //get max temperature value for all chain
   uint32_t temp_hi = 0;
   if(temp_ctrl[spi_id].temp_highest[0] && temp_ctrl[spi_id].temp_highest[1] && temp_ctrl[spi_id].temp_highest[2])
   {
    temp_hi = temp_ctrl[spi_id].temp_highest[0] > temp_ctrl[spi_id].temp_highest[1] ? temp_ctrl[spi_id].temp_highest[1]:temp_ctrl[spi_id].temp_highest[0];
    temp_hi = temp_hi > temp_ctrl[spi_id].temp_highest[2] ? temp_ctrl[spi_id].temp_highest[2]:temp_hi;

    }else{
    if(temp_ctrl[spi_id].temp_highest[0] && temp_ctrl[spi_id].temp_highest[1])
      temp_hi = temp_ctrl[spi_id].temp_highest[0] > temp_ctrl[spi_id].temp_highest[1] ? temp_ctrl[spi_id].temp_highest[1]:temp_ctrl[spi_id].temp_highest[0];
    else if(temp_ctrl[spi_id].temp_highest[0] && temp_ctrl[spi_id].temp_highest[2])
        temp_hi = temp_ctrl[spi_id].temp_highest[0] > temp_ctrl[spi_id].temp_highest[2] ? temp_ctrl[spi_id].temp_highest[2]:temp_ctrl[spi_id].temp_highest[0];
    else if(temp_ctrl[spi_id].temp_highest[1] && temp_ctrl[spi_id].temp_highest[2])
        temp_hi = temp_ctrl[spi_id].temp_highest[1] > temp_ctrl[spi_id].temp_highest[2] ? temp_ctrl[spi_id].temp_highest[2]:temp_ctrl[spi_id].temp_highest[1];
    }

    temp_ctrl[spi_id].final_temp_hi = temp_hi;

   //get min temperature value for all chain

   uint32_t temp_lo = 0;
   if(temp_ctrl[spi_id].temp_lowest[0] && temp_ctrl[spi_id].temp_lowest[1] && temp_ctrl[spi_id].temp_lowest[2])
   {
    temp_lo = temp_ctrl[spi_id].temp_lowest[0] < temp_ctrl[spi_id].temp_lowest[1] ? temp_ctrl[spi_id].temp_lowest[1]:temp_ctrl[spi_id].temp_lowest[0];
    temp_lo = temp_lo < temp_ctrl[spi_id].temp_lowest[2] ? temp_ctrl[spi_id].temp_lowest[2]:temp_lo;

    }else{
    if(temp_ctrl[spi_id].temp_lowest[0] && temp_ctrl[spi_id].temp_lowest[1])
      temp_lo = temp_ctrl[spi_id].temp_lowest[0] < temp_ctrl[spi_id].temp_lowest[1] ? temp_ctrl[spi_id].temp_lowest[1]:temp_ctrl[spi_id].temp_lowest[0];
    else if(temp_ctrl[spi_id].temp_lowest[0] && temp_ctrl[spi_id].temp_lowest[2])
        temp_lo = temp_ctrl[spi_id].temp_lowest[0] < temp_ctrl[spi_id].temp_lowest[2] ? temp_ctrl[spi_id].temp_lowest[2]:temp_ctrl[spi_id].temp_lowest[0];
    else if(temp_ctrl[spi_id].temp_lowest[1] && temp_ctrl[spi_id].temp_lowest[2])
        temp_lo = temp_ctrl[spi_id].temp_lowest[1] < temp_ctrl[spi_id].temp_lowest[2] ? temp_ctrl[spi_id].temp_lowest[2]:temp_ctrl[spi_id].temp_lowest[1];
    }

    temp_ctrl[spi_id].final_temp_lo = temp_lo;

   //get average temperature value for all chain

mcompat_log(MCOMPAT_LOG_INFO,"temp:%d,%d,%d\n",temp_ctrl[spi_id].final_temp_hi,temp_ctrl[spi_id].final_temp_avg,temp_ctrl[spi_id].final_temp_lo);
    return ;
}

/*
 * by duanhao
 * Miners using inverse pwm polarity are as below:
 *      T1 with g_vid_type == VID
 *      A5 with g_vid_type == VID
 *      A6 with g_vid_type == VID
 *      A8 with g_vid_type == VID
 *      D9 with g_vid_type == VID
 *      S11 with g_vid_type == VID
 */
void mcompat_fan_speed_set(unsigned char fan_id, int speed)
{
    bool fan_pol = (MCOMPAT_LIB_VID_VID_TYPE == g_vid_type)         // XHN
	               && (MCOMPAT_LIB_MINER_TYPE_T1 == g_miner_type
	                || MCOMPAT_LIB_MINER_TYPE_A5 == g_miner_type
	                || MCOMPAT_LIB_MINER_TYPE_A6 == g_miner_type
	                || MCOMPAT_LIB_MINER_TYPE_A8 == g_miner_type
	                || MCOMPAT_LIB_MINER_TYPE_D9 == g_miner_type
	                || MCOMPAT_LIB_MINER_TYPE_S11 == g_miner_type);
	int duty = fan_pol ? 100 - speed : speed;

	mcompat_set_pwm(fan_id, ASIC_MCOMPAT_FAN_PWM_FREQ_TARGET, duty);
	mcompat_set_pwm(fan_id + 1, ASIC_MCOMPAT_FAN_PWM_FREQ_TARGET, duty);
}

 void mcompat_fan_temp_init(unsigned char fan_id, mcompat_temp_config_s default_config)
 {
   g_temp_hi_thr = default_config.temp_hi_thr;
   g_temp_lo_thr = default_config.temp_lo_thr;
   g_temp_start_thr = default_config.temp_start_thr;
   g_dangerous_temp = default_config.dangerous_stat_temp;
   g_work_temp = default_config.work_temp;
   g_fan_speed = default_config.default_fan_speed;

   mcompat_log(MCOMPAT_LOG_INFO,"hi %d,lo %d,st %d,da %d, wk %d\n",g_temp_hi_thr,g_temp_lo_thr,g_temp_start_thr,g_dangerous_temp,g_work_temp);

     mcompat_fan_speed_set(fan_id,g_fan_speed);

     mcompat_log(MCOMPAT_LOG_INFO, "pwm  step:%d.\n", ASIC_MCOMPAT_FAN_PWM_STEP);
     mcompat_log(MCOMPAT_LOG_INFO, "duty max: %d.\n", ASIC_MCOMPAT_FAN_PWM_DUTY_MAX);
     mcompat_log(MCOMPAT_LOG_INFO, "targ freq:%d.\n", ASIC_MCOMPAT_FAN_PWM_FREQ_TARGET);
     mcompat_log(MCOMPAT_LOG_INFO, "freq rate:%d.\n", ASIC_MCOMPAT_FAN_PWM_FREQ);
     mcompat_log(MCOMPAT_LOG_INFO, "fan speed thrd:%d.\n", ASIC_MCOMPAT_FAN_TEMP_MAX_THRESHOLD);
     mcompat_log(MCOMPAT_LOG_INFO, "fan up thrd:%d.\n", ASIC_MCOMPAT_FAN_TEMP_UP_THRESHOLD);
     mcompat_log(MCOMPAT_LOG_INFO, "fan down thrd:%d.\n", ASIC_MCOMPAT_FAN_TEMP_DOWN_THRESHOLD);
 }


 void mcompat_fan_speed_update_hub(mcompat_fan_temp_s *fan_temp)
 {
   static int cnt = 0;
   int i = 0;

   int temp_hi = g_temp_lo_thr; //fan_temp->temp_highest[0];

   if(fan_temp->speed == 0)
    fan_temp->speed = g_fan_speed;


   for(i=0; i<g_chain_num; i++)
   {
    if(hub_get_plug(i))
        continue;

     mcompat_deal_temp(i,fan_temp);


     if((fan_temp->mcompat_temp[i].final_temp_hi > g_temp_lo_thr) || (fan_temp->mcompat_temp[i].final_temp_hi < g_temp_hi_thr) || \
             (fan_temp->mcompat_temp[i].final_temp_avg > g_temp_lo_thr) || (fan_temp->mcompat_temp[i].final_temp_avg < g_temp_hi_thr) || \
             (fan_temp->mcompat_temp[i].final_temp_lo > g_temp_lo_thr) || (fan_temp->mcompat_temp[i].final_temp_lo < g_temp_hi_thr) )
     {
         mcompat_log(MCOMPAT_LOG_ERR,"Notice!!! Error temperature for chain %d,h:%d,a:%d,l:%d\n", i, \
             fan_temp->mcompat_temp[i].final_temp_hi,fan_temp->mcompat_temp[i].final_temp_avg,fan_temp->mcompat_temp[i].final_temp_lo);
         continue ;
     }

     if(fan_temp->mcompat_temp[i].final_temp_hi < g_dangerous_temp)
         set_warn(i);

     if(temp_hi > fan_temp->mcompat_temp[i].final_temp_hi)
        temp_hi = fan_temp->mcompat_temp[i].final_temp_hi;
   }

     if((temp_hi == g_temp_lo_thr)||(temp_hi == g_temp_hi_thr))
     {
         mcompat_fan_speed_set(0,100);
         return ;
     }

     int delt_temp = abs(g_work_temp - temp_hi);
     int delt_speed = abs(temp_hi - fan_temp->last_fan_temp);
     mcompat_log(MCOMPAT_LOG_INFO,"Hi temp %d,delt_temp %d,delt_speed %d\n",temp_hi,delt_temp, delt_speed);

        if(delt_temp > 3)
        {
          if((delt_speed < 2) && (cnt < 3))
          {
              cnt ++;
              return;
           }

          cnt = 0;

           if(temp_hi > g_work_temp)
           {
             fan_temp->speed = (fan_temp->speed - 5)>10?(fan_temp->speed - 5):10;
             //applog(LOG_ERR, "%s +:arv:%5.2f, lest:%5.2f, hest:%5.2f, speed:%d%%", __func__, arvarge_f, lowest_f, highest_f, 100 - fan_ctrl->duty);
           }else if (temp_hi < g_work_temp)
           {
              fan_temp->speed = (fan_temp->speed + 5)<100?(fan_temp->speed + 5):100;
             //applog(LOG_ERR, "%s +:arv:%5.2f, lest:%5.2f, hest:%5.2f, speed:%d%%", __func__, arvarge_f, lowest_f, highest_f, 100 - fan_ctrl->duty);
           }
         }
         //applog(LOG_ERR,"temp_highest %d, fan speed %d,last fan id: %d\n",fan_temp->temp_highest[chain_id],fan_speed[fan_temp->last_fan_temp],fan_temp->last_fan_temp);


       if(fan_temp->speed != fan_temp->last_fan_speed)
       {
         fan_temp->last_fan_speed = fan_temp->speed;
         fan_temp->last_fan_temp = temp_hi;
         mcompat_fan_speed_set(0,fan_temp->speed);

        }
 }



