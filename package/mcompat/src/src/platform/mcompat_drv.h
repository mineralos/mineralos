#ifndef _MCOMPAT_DRV_H_
#define _MCOMPAT_DRV_H_

#include "mcompat_fan.h"
#include "mcompat_tvsensor.h"

#define PLATFORM_ZYNQ_SPI_G9    (0x01)
#define PLATFORM_ZYNQ_SPI_G19   (0x02)
#define PLATFORM_ZYNQ_HUB_G9    (0x03)
#define PLATFORM_ZYNQ_HUB_G19   (0x04)
#define PLATFORM_SOC            (0x10)
#define PLATFORM_ORANGE_PI      (0x20)

#define SPI_SPEED_390K          (0)
#define SPI_SPEED_781K          (1)
#define SPI_SPEED_1562K         (2)
#define SPI_SPEED_3125K         (3)
#define SPI_SPEED_6250K         (4)
#define SPI_SPEED_9960K         (5)

#define MCOMPAT_LOG_DEBUG            (1)
#define MCOMPAT_LOG_INFO             (2)
#define MCOMPAT_LOG_NOTICE           (3)
#define MCOMPAT_LOG_WARNING          (4)
#define MCOMPAT_LOG_ERR              (5)
#define MCOMPAT_LOG_CRIT             (6)
#define MCOMPAT_LOG_ALERT            (7)
#define MCOMPAT_LOG_EMERG            (8)


extern void sys_platform_debug_init(int debug_level);

extern bool sys_platform_init(int platform, int miner_type, int chain_num, int chip_num);

extern bool sys_platform_exit();


extern bool mcompat_set_spi_speed(unsigned char chain_id, int index);

extern bool mcompat_cmd_reset(unsigned char chain_id, unsigned char chip_id, unsigned char *in, unsigned char *out);

extern int mcompat_cmd_bist_start(unsigned char chain_id, unsigned char chip_id);

extern bool mcompat_cmd_bist_collect(unsigned char chain_id, unsigned char chip_id);

extern bool mcompat_cmd_bist_fix(unsigned char chain_id, unsigned char chip_id);

extern bool mcompat_cmd_write_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len);

extern bool mcompat_cmd_read_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len);

extern bool mcompat_cmd_read_write_reg0d(unsigned char chain_id, unsigned char chip_id, unsigned char *in, int len, unsigned char *out);

extern bool mcompat_cmd_read_result(unsigned char chain_id, unsigned char chip_id, unsigned char *res, int len);

extern bool mcompat_cmd_write_job(unsigned char chain_id, unsigned char chip_id, unsigned char *job, int len);

extern bool mcompat_cmd_auto_nonce(unsigned char chain_id, int mode, int len);

extern bool mcompat_cmd_read_nonce(unsigned char chain_id, unsigned char *res, int len);

extern bool mcompat_cmd_get_temp(mcompat_fan_temp_s * fan_temp);

extern void mcompat_set_power_en(unsigned char chain_id, int val);

extern void mcompat_set_start_en(unsigned char chain_id, int val);

extern void mcompat_set_reset(unsigned char chain_id, int val);

extern void mcompat_set_led(unsigned char chain_id, int val);

extern bool mcompat_set_vid(unsigned char chain_id, int val);

extern bool mcompat_set_vid_by_step(unsigned char chain_id, int start_vid, int target_vid);

extern void mcompat_set_pwm(unsigned char fan_id, int frequency, int duty);

extern int mcompat_get_plug(unsigned char chain_id);

extern int mcompat_get_button(void);

extern void mcompat_set_green_led(int mode);

extern void mcompat_set_red_led(int mode);

extern bool mcompat_chain_power_on(unsigned char chain_id, unsigned char mode);

extern bool mcompat_chain_power_down(unsigned char chain_id);

extern bool mcompat_chain_hw_reset(unsigned char chain_id);

extern bool mcompat_chain_power_on_all(void);

extern bool mcompat_chain_power_down_all(void);

extern bool mcompat_i2c_feed_watchdog();

extern bool mcompat_i2c_set_timeout(int time);

#endif
