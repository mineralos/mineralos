/******************************************************************************
 *
 * 文件名  ： mcompat_gpio_i2c.c
 * 负责人  ： pengp
 * 创建日期： 20180306
 * 版本号  ： v1.0
 * 文件描述： gpio模拟i2c的实现
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <stdio.h>
#include "mcompat_log.h"
#include "mcompat_config.h"
#include "mcompat_gpio.h"
#include "mcompat_gpio_i2c.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/
static void delay(int time);
static bool start(void);
static void stop(void);
static void ack(void);
static void no_ack(void);
static bool wait_ack(void);
static bool write_addr(uint16_t reg_addr, uint8_t dev_addr);

/********************************** 变量实现区 *********************************/

/********************************** 函数实现区 *********************************/
void mcompat_gpio_i2c_init(void)
{
    return;
}

void mcompat_gpio_i2c_deinit(void)
{
    return;
}

/* 数据从高位到低位 */
void mcompat_gpio_i2c_send_byte(uint8_t data)
{
    uint8_t i = 8;

    while(i--)
    {
        _SCL_L();
        delay(6);
        if(data & 0x80)
        {
            _SDA_H();
        }
        else
        {
            _SDA_L();
        }

        data = data << 1;

        delay(18);
        _SCL_H();
        delay(24);
    }
    _SCL_L();
}

/* 数据从高位到低位 */
uint8_t mcompat_gpio_i2c_recv_byte(void)
{
    uint8_t data = 0;
    uint8_t i = 8;

    while(i--)
    {
        data = data << 1;
        _SCL_L();
        delay(24);

        _SCL_H();
        delay(24);
        if(_SDA_READ())
        {
            data |= 0x01;
        }
    }
    _SCL_L();

    return data;
}

bool mcompat_gpio_i2c_send_buf(uint8_t *buf, uint8_t buf_len, uint8_t dev_addr, uint16_t reg_addr)
{
    /* step1: 写地址 */
    if(!write_addr(dev_addr, reg_addr))
    {
        return false;
    }

    /* step2: 写数据 */
    while(buf_len--)
    {
        mcompat_gpio_i2c_send_byte(*buf);
        wait_ack();
        buf++;
    }
    stop();

    return true;
}

bool mcompat_gpio_i2c_recv_buf(uint8_t *buf, uint8_t buf_len, uint8_t dev_addr, uint16_t reg_addr)
{
    /* step1: 写地址 */
    if(!write_addr(dev_addr, reg_addr))
    {
        return false;
    }

    /* step2: 读数据 */
    while(buf_len)
    {
        *buf = mcompat_gpio_i2c_recv_byte();
        if(1 == buf_len)
        {
            no_ack();
        }
        else
        {
            ack();
        }
        buf++;
        buf_len--;
    }
    stop();

    return true;
}

static void delay(int time)
{
    usleep(time);
}

/* I2C协议 */
static bool start(void)
{
    _SDA_H();
    _SCL_H();
    delay(25);

    /* 总线忙 */
    if(!_SDA_READ())
    {
        return false;
    }

    _SDA_L();
    delay(25);

    /* 总线出错 */
    if(!_SDA_READ())
    {
        return false;
    }

    _SDA_L();
    delay(25);

    return true;
}

static void stop(void)
{
    _SCL_L();
    delay(25);
    _SDA_L();
    delay(50);
    _SCL_H();
    delay(50);
    _SDA_H();
    delay(25);
}

static void ack(void)
{
    _SCL_L();
    delay(4);
    _SDA_L();
    delay(20);
    _SCL_H();
    delay(24);
    _SCL_L();
    _SDA_H();
}

static void no_ack(void)
{
    _SCL_L();
    delay(6);
    _SDA_H();
    delay(18);
    _SCL_H();
    delay(24);
    _SCL_L();
}

static bool wait_ack(void)
{
    int timeout = 0;

    _SCL_L();
    delay(6);

    _SDA_H();
    delay(18);

    _SCL_H();
    delay(25);

    while(_SDA_READ())
    {
        timeout++;
        if(timeout > 40)
        {
            _SCL_L();
            delay(25);
            return false;
        }
    }

    _SCL_L();

    return true;
}

static bool write_addr(uint16_t reg_addr, uint8_t dev_addr)
{
    if(!start())
    {
        return false;
    }

    /* step1: 设置dev_addr */
    mcompat_gpio_i2c_send_byte(dev_addr & 0xFE);
    if(!wait_ack())
    {
        stop();
        return false;
    }

    /* step2: 设置reg_addr */
    mcompat_gpio_i2c_send_byte((uint8_t)(reg_addr & 0x00FF));
    if(!wait_ack())
    {
        stop();
        return false;
    }

    return true;
}

