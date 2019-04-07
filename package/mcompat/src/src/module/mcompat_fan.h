/******************************************************************************
 *
 * �ļ���  ��mcompat_fan.h
 * ������  �� yex
 * �������ڣ� 20171122
 * �汾��  �� V1.0
 * �ļ������� �������¶ȿ��ƽӿ�
 * ��Ȩ˵���� Copyright (c) 2000-2020   GNU
 * ��    ���� ��
 * �޸���־�� ��
 *
 *******************************************************************************/

/*---------------------------------- Ԥ������ ---------------------------------*/
#ifndef _MCOMPAT_FAN_H_
#define _MCOMPAT_FAN_H_

/************************************ ͷ�ļ� ***********************************/

#include "mcompat_config.h"

/************************************ �궨�� ***********************************/

/*********************************** ���Ͷ��� **********************************/

/*--------------------------------- �ӿ������� --------------------------------*/

/*********************************** ȫ�ֱ��� **********************************/
extern int g_temp_hi_thr;
extern int g_temp_lo_thr;
extern int g_temp_start_thr;
extern int g_dangerous_temp;
extern int g_work_temp;

/*--------------------------------- data struct  --------------------------------*/

/*********************************** realize **********************************/
typedef struct {
    int final_temp_avg;
    int final_temp_hi;
    int final_temp_lo;
    int temp_highest[3];
    int temp_lowest[3];
}mcompat_temp_s;


typedef struct {
    int temp_hi_thr;
    int temp_lo_thr;
    int temp_start_thr;
    int dangerous_stat_temp;
    int work_temp;
    int default_fan_speed;
}mcompat_temp_config_s;


typedef struct {
    int fd;
    int last_valid_temp;

    int speed;
    int last_fan_speed;
    int last_fan_temp;
    mcompat_temp_s * mcompat_temp;
    int temp_average;
    int temp_highest;
    int temp_lowest;
}mcompat_fan_temp_s;


/*********************************** function prototype **********************************/
//extern int mcompat_deal_temp(unsigned char spi_id, mcompat_fan_temp_s *temp_ctrl);
extern void mcompat_fan_temp_init(unsigned char fan_id,mcompat_temp_config_s temp_config);
extern void mcompat_fan_speed_set(unsigned char fan_id, int speed);
extern void mcompat_fan_speed_update_hub(mcompat_fan_temp_s *fan_temp);

#endif // #ifndef _MCOMPAT_FAN_TEMP_H_
