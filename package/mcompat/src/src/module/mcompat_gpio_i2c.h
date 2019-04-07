/******************************************************************************
 *
 * 文件名  ： mcompat_gpio_i2c.h
 * 负责人  ： pengp
 * 创建日期： 20170306
 * 版本号  ： v1.0
 * 文件描述： gpio模拟i2c接口
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _MCOMPAT_GPIO_I2C_H_
#define _MCOMPAT_GPIO_I2C_H_

/************************************ 头文件 ***********************************/
#include <stdio.h>
#include "mcompat_config.h"

#include "mcompat_drv.h"


/************************************ 宏定义 ***********************************/
/* 连接底层gpio */
#define _SCL_PIN                (0)
#define _SDA_PIN                (1)
#define _SCL_L()                mcompat_write_gpio(_SCL_PIN, 0)
#define _SCL_H()                mcompat_write_gpio(_SCL_PIN, 1)
#define _SCL_READ()             mcompat_read_gpio(_SCL_PIN)
#define _SDA_L()                mcompat_write_gpio(_SDA_PIN, 0)
#define _SDA_H()                mcompat_write_gpio(_SDA_PIN, 1)
#define _SDA_READ()             mcompat_read_gpio(_SDA_PIN)

/*********************************** 类型定义 **********************************/

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void mcompat_gpio_i2c_init(void);
void mcompat_gpio_i2c_deinit(void);

void mcompat_gpio_i2c_send_byte(uint8_t data);
uint8_t mcompat_gpio_i2c_recv_byte(void);

bool mcompat_gpio_i2c_send_buf(uint8_t *buf, uint8_t buf_len, uint8_t dev_addr, uint16_t reg_addr);
bool mcompat_gpio_i2c_recv_buf(uint8_t *buf, uint8_t buf_len, uint8_t dev_addr, uint16_t reg_addr);

#endif // #ifndef _MCOMPAT_GPIO_I2C_H_

