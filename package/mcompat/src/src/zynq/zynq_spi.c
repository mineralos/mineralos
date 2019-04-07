/******************************************************************************
 *
 * 文件名  ： zynq_spi.c
 * 负责人  ： wangp
 * 创建日期： 20171208
 * 版本号  ： v1.0
 * 文件描述： zynq平台spi驱动实现
 * 版权说明： Copyright (c) 2000-2020 im
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/types.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "mcompat_config.h"
#include "zynq_spi.h"

#define BUF_MAX                     (256)

#define DEV_TEMPLATE                ("/dev/spidev%d.%d")
#define SYSFS_EXPORT                ("/sys/devices/soc0/amba/f8007000.devcfg/fclk_export")
#define SYSFS_VAL_STR               ("/sys/devices/soc0/amba/f8007000.devcfg/fclk/fclk1/set_rate")

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数声明区 *********************************/
/*******************************************************************************
 *
 * 函数名  : zynq_spi_clock_init
 * 描述    : spi时钟初始化
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   无
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
static void zynq_spi_clock_init(void);

/*******************************************************************************
 *
 * 函数名  : zynq_set_spi_speed
 * 描述    : spi速度设置
 * 输入参数: speed - spi速度
 * 输出参数: 无
 * 返回值:   无
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/

#if 0
/*******************************************************************************
 *
 * 函数名  : zynq_get_spi_speed
 * 描述    : spi速度获取
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   spi速度
 *
 * 调用关系: 无
 * 其 它:    暂未使用,避免编译告警 关闭编译
 *
 ******************************************************************************/
static void zynq_get_spi_speed(void);
#endif


/********************************** 变量实现区 *********************************/

/********************************** 函数实现区 *********************************/
void zynq_spi_init(ZYNQ_SPI_T *spi, int bus)
{
    char dev_fname[BUF_MAX] = {'\0'};
    int fd = 0;
    uint8_t mode   = MCOMPAT_CONFIG_SPI_DEFAULT_MODE;
    uint32_t speed = MCOMPAT_CONFIG_SPI_DEFAULT_SPEED;
    uint8_t bits   = MCOMPAT_CONFIG_SPI_DEFAULT_BITS_PER_WORD;

    zynq_spi_clock_init();
    zynq_set_spi_speed(MCOMPAT_CONFIG_SPI_DEFAULT_SPEED);

    sprintf(dev_fname, DEV_TEMPLATE, bus, MCOMPAT_CONFIG_SPI_DEFAULT_CS_LINE);
    fd = open(dev_fname, O_RDWR);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }

    if(ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    if(ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }

    spi->fd = fd;
    pthread_mutex_init(&(spi->lock), NULL);

    mcompat_log(MCOMPAT_LOG_DEBUG, "SPI '%s': mode=%hhu, bits=%hhu, speed=%u \n",
            dev_fname, MCOMPAT_CONFIG_SPI_DEFAULT_MODE, MCOMPAT_CONFIG_SPI_DEFAULT_BITS_PER_WORD, MCOMPAT_CONFIG_SPI_DEFAULT_SPEED);
    return;
}

void zynq_spi_exit(ZYNQ_SPI_T *spi)
{
    if(NULL == spi)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }

    close(spi->fd);

    return;
}

void zynq_spi_write(ZYNQ_SPI_T *spi, uint8_t *txbuf, int len)
{
    pthread_mutex_lock(&(spi->lock));

    if((len <= 0) || (txbuf == NULL))
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }

    if(write(spi->fd, txbuf, len) <= 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }

    pthread_mutex_unlock(&spi->lock);
    return;
}

void zynq_spi_read(ZYNQ_SPI_T *spi, uint8_t *rxbuf, int len)
{
    pthread_mutex_lock(&(spi->lock));

    if((len <= 0) || (rxbuf == NULL))
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }

    if(read(spi->fd, rxbuf, len) <= 0)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }

    pthread_mutex_unlock(&spi->lock);
    return;
}

void zynq_spi_clock_init(void)
{
    int  fd = 0;
    ssize_t write_bytes = 0;
    char fvalue[BUF_MAX] = {'\0'};

    /* 已经导出,不可重复导出 */
    fd = access(SYSFS_VAL_STR, F_OK);
    if(0 == fd)
    {
        return;
    }

    fd = open(SYSFS_EXPORT, O_WRONLY);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    memset(fvalue, 0, sizeof(fvalue));
    sprintf(fvalue, "%s", "fclk1");
    write_bytes = write(fd, fvalue, strlen(fvalue));
    if(-1 == write_bytes)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    close(fd);

    return;
}

void zynq_set_spi_speed(int speed)
{
    int  fd = 0;
    ssize_t write_bytes = 0;
    char fvalue[BUF_MAX] = {'\0'};

    mcompat_log(MCOMPAT_LOG_DEBUG, "set spi speed %d \n", speed);
    fd = open(SYSFS_VAL_STR, O_WRONLY);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    memset(fvalue, 0, sizeof(fvalue));
    sprintf(fvalue, "%d", speed * 16);
    write_bytes = write(fd, fvalue, strlen(fvalue));
    if(-1 == write_bytes)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    close(fd);

    return;
}

#if 0
static int zynq_get_spi_speed(void)
{
    int  fd = 0;
    ssize_t read_bytes = 0;
    char fvalue[BUF_MAX] = {'\0'};
    int  speed = 0;

    fd = open(SYSFS_VAL_STR, O_RDONLY);
    if(-1 == fd)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    memset(fvalue, 0, sizeof(fvalue));
    read_bytes = read(fd, fvalue, 12);
    if(-1 == read_bytes)
    {
        mcompat_log_err("%s,%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    }
    speed = atoi(fvalue);
    close(fd);

    return speed;
}
#endif

