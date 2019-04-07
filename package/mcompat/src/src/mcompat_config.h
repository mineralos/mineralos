/******************************************************************************
 *
 * 文件名  ： config.h
 * 负责人  ： pengp
 * 创建日期： 20171212
 * 版本号  ： v1.0
 * 文件描述： im库的配置
 * 版权说明： Copyright (c) 2000-2020 im
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _CONFIG_H_
#define _CONFIG_H_

/************************************ 头文件 ***********************************/
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcompat_log.h"

/************************************ 宏定义 ***********************************/
/* log打印的最长行 */
#define MCOMPAT_CONFIG_LOG_LINE_BUF_SIZE           (512)

#define MCOMPAT_CONFIG_MAX_CHAIN_NUM               (8)
#define MCOMPAT_CONFIG_MAX_CHIP_NUM                (80)
#define MCOMPAT_CONFIG_MAX_JOB_LEN                 (92)
#define MCOMPAT_CONFIG_MAX_CMD_LENGTH              (256)

#define MAGIC_NUM                               (100)

/* Ax芯片命令 */
#define CMD_BIST_START                          (0x01)
#define CMD_BIST_COLLECT                        (0x0b)
#define CMD_BIST_FIX                            (0x03)
#define CMD_RESET                               (0x04)
#define CMD_RESETBC                             (0x05)
#define CMD_WRITE_JOB                           (0x07)
#define CMD_WRITE_JOB_T1                        (0x0c)
#define CMD_READ_RESULT                         (0x08)
#define CMD_WRITE_REG                           (0x09)
#define CMD_READ_REG                            (0x0a)
#define CMD_WRITE_REG0d                         (0x0d)
#define CMD_POWER_ON                            (0x02)
#define CMD_POWER_OFF                           (0x06)
#define CMD_POWER_RESET                         (0x0c)

#define RESP_READ_REG                           (0x1a)

#define CMD_ADDR_BROADCAST                      (0x00)
#define CMD_HL                                  (2)
#define CMD_RESET_DL                            (4)
#define CMD_RESET_TL                            (CMD_HL + CMD_RESET_DL)

#define LED_GREEN                               (0x1)
#define LED_RED                                 (0x2)
#define LED_ON                                  (0)
#define LED_OFF                                 (1)
#define LED_BLINK                               (2)

#define VID_MAX			                        (31)
#define VID_MIN			                        (0)

#define CHIP_TECH_UNKNOWN                       (-1)
#define CHIP_TECH_10NM                          (0)
#define CHIP_TECH_14NM                          (1)
#define TEMP_COEF_10NM                          (588)
#define TEMP_COEF_14NM                          (594)
#define VOLT_COEF_10NM                          (1248)
#define VOLT_COEF_14NM                          (1230)

#define ASIC_MCOMPAT_FAN_PWM_STEP            (5)
#define ASIC_MCOMPAT_FAN_PWM_DUTY_MAX        (100)
#define ASIC_MCOMPAT_FAN_PWM_FREQ_TARGET     (20000)
#define ASIC_MCOMPAT_FAN_PWM_FREQ            (20000)
#define FAN_CNT                           ( 2 )
#define ASIC_MCOMPAT_FAN_TEMP_MAX_THRESHOLD  (100)
#define ASIC_MCOMPAT_FAN_TEMP_UP_THRESHOLD   (55)
#define ASIC_MCOMPAT_FAN_TEMP_DOWN_THRESHOLD (35)

/* uart方案vid串口设备 */
/* FIXME: 实际使用切换为/dev/ttyPS1 */
#define MCOMPAT_VID_UART_PATH                        ("/dev/ttyPS1")


/* 第一个2由芯片数据手册得到,第二个2是由于芯片的数据单位为16bits(2Bytes) */
#define MCOMPAT_CONFIG_CMD_MAX_LEN                 (MCOMPAT_CONFIG_MAX_JOB_LEN + MCOMPAT_CONFIG_MAX_CHAIN_NUM * 2 * 2)
#define MCOMPAT_CONFIG_CMD_RST_MAX_LEN             (MCOMPAT_CONFIG_CMD_MAX_LEN)

/* 矿机配置 */
/* SPI */
#define MCOMPAT_CONFIG_SPI_DEFAULT_CS_LINE         (0)
#define MCOMPAT_CONFIG_SPI_DEFAULT_MODE            (SPI_MODE_1)
#define MCOMPAT_CONFIG_SPI_DEFAULT_SPEED           (1500000)
#define MCOMPAT_CONFIG_SPI_DEFAULT_BITS_PER_WORD   (8)
/* GPIO */
#define MCOMPAT_CONFIG_CHAIN0_POWER_EN_GPIO        (872)
#define MCOMPAT_CONFIG_CHAIN1_POWER_EN_GPIO        (873)
#define MCOMPAT_CONFIG_CHAIN2_POWER_EN_GPIO        (874)
#define MCOMPAT_CONFIG_CHAIN3_POWER_EN_GPIO        (875)
#define MCOMPAT_CONFIG_CHAIN4_POWER_EN_GPIO        (876)
#define MCOMPAT_CONFIG_CHAIN5_POWER_EN_GPIO        (877)
#define MCOMPAT_CONFIG_CHAIN6_POWER_EN_GPIO        (878)
#define MCOMPAT_CONFIG_CHAIN7_POWER_EN_GPIO        (879)
#define MCOMPAT_CONFIG_CHAIN0_START_EN_GPIO        (854)
#define MCOMPAT_CONFIG_CHAIN1_START_EN_GPIO        (856)
#define MCOMPAT_CONFIG_CHAIN2_START_EN_GPIO        (858)
#define MCOMPAT_CONFIG_CHAIN3_START_EN_GPIO        (860)
#define MCOMPAT_CONFIG_CHAIN4_START_EN_GPIO        (862)
#define MCOMPAT_CONFIG_CHAIN5_START_EN_GPIO        (864)
#define MCOMPAT_CONFIG_CHAIN6_START_EN_GPIO        (866)
#define MCOMPAT_CONFIG_CHAIN7_START_EN_GPIO        (868)
#define MCOMPAT_CONFIG_CHAIN0_RESET_GPIO           (855)
#define MCOMPAT_CONFIG_CHAIN1_RESET_GPIO           (857)
#define MCOMPAT_CONFIG_CHAIN2_RESET_GPIO           (859)
#define MCOMPAT_CONFIG_CHAIN3_RESET_GPIO           (861)
#define MCOMPAT_CONFIG_CHAIN4_RESET_GPIO           (863)
#define MCOMPAT_CONFIG_CHAIN5_RESET_GPIO           (865)
#define MCOMPAT_CONFIG_CHAIN6_RESET_GPIO           (867)
#define MCOMPAT_CONFIG_CHAIN7_RESET_GPIO           (869)
#define MCOMPAT_CONFIG_CHAIN0_LED_GPIO             (881)
#define MCOMPAT_CONFIG_CHAIN1_LED_GPIO             (882)
#define MCOMPAT_CONFIG_CHAIN2_LED_GPIO             (883)
#define MCOMPAT_CONFIG_CHAIN3_LED_GPIO             (884)
#define MCOMPAT_CONFIG_CHAIN4_LED_GPIO             (885)
#define MCOMPAT_CONFIG_CHAIN5_LED_GPIO             (886)
#define MCOMPAT_CONFIG_CHAIN6_LED_GPIO             (887)
#define MCOMPAT_CONFIG_CHAIN7_LED_GPIO             (888)
#define MCOMPAT_CONFIG_CHAIN0_PLUG_GPIO            (896)
#define MCOMPAT_CONFIG_CHAIN1_PLUG_GPIO            (897)
#define MCOMPAT_CONFIG_CHAIN2_PLUG_GPIO            (898)
#define MCOMPAT_CONFIG_CHAIN3_PLUG_GPIO            (899)
#define MCOMPAT_CONFIG_CHAIN4_PLUG_GPIO            (900)
#define MCOMPAT_CONFIG_CHAIN5_PLUG_GPIO            (901)
#define MCOMPAT_CONFIG_CHAIN6_PLUG_GPIO            (902)
#define MCOMPAT_CONFIG_CHAIN7_PLUG_GPIO            (903)

#define MCOMPAT_CONFIG_B9_GPIO                     (906 + 51)
#define MCOMPAT_CONFIG_A10_GPIO                    (906 + 37)

/*********************************** 类型定义 **********************************/


/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/
extern int g_platform;
extern int g_miner_type;
extern int g_chain_num;
extern int g_chip_num;
extern int g_vid_type;


/*********************************** 接口函数 **********************************/

#endif // #ifndef _CONFIG_H_

