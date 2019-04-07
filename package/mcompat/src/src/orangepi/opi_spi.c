/*
 * generic SPI functions
 *
 * Copyright 2013, 2014 Zefir Kurtisi <zefir.kurtisi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.  See COPYING for more details.
 */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "mcompat_config.h"
#include "opi_h3.h"
#include "opi_spi.h"

static int opi_spi_fd = 0;
static pthread_mutex_t opi_spi_lock;
static struct spi_config opi_spi_config = {
	.bus		= DEFAULT_SPI_BUS,
	.cs_line	= DEFAULT_SPI_CS_LINE,
	.mode		= DEFAULT_SPI_MODE,
	.speed		= DEFAULT_SPI_SPEED,
	.bits		= DEFAULT_SPI_BITS_PER_WORD,
	.delay		= DEFAULT_SPI_DELAY_USECS,
};


void opi_spi_gpio_init(void)
{
	if (gpio_init() == -1) {
        printf("gpio initial fail");
    }
	
	gpio_setcfg(PIN_SPI_E1, OUTPUT);
    gpio_output(PIN_SPI_E1, HIGH);
	
	gpio_setcfg(PIN_SPI_A0, OUTPUT);
    gpio_output(PIN_SPI_A0, HIGH);
	gpio_setcfg(PIN_SPI_A1, OUTPUT);
    gpio_output(PIN_SPI_A1, HIGH);
	gpio_setcfg(PIN_SPI_A2, OUTPUT);
    gpio_output(PIN_SPI_A2, HIGH);
}

void opi_spi_cs_enable(int id)
{
	if(id & 01)
	{
		gpio_output(PIN_SPI_A0, HIGH);
	}
	else
	{
		gpio_output(PIN_SPI_A0, LOW);
	}
	
	if(id & 02)
	{
		gpio_output(PIN_SPI_A1, HIGH);
	}
	else
	{
		gpio_output(PIN_SPI_A1, LOW);
	}
	
	if(id & 04)
	{
		gpio_output(PIN_SPI_A2, HIGH);
	}
	else
	{
		gpio_output(PIN_SPI_A2, LOW);
	}
	
	gpio_output(PIN_SPI_E1, LOW);
}

void opi_spi_cs_disable(void)
{
	gpio_output(PIN_SPI_E1, HIGH);
}

void opi_spi_init(void)
{
	char dev_fname[PATH_MAX];
    struct spi_config *config = &opi_spi_config;

    pthread_mutex_init(&opi_spi_lock, NULL);	
	opi_spi_gpio_init();

	sprintf(dev_fname, SPI_DEVICE_TEMPLATE, config->bus, config->cs_line);

	int fd = open(dev_fname, O_RDWR);
	if (fd < 0) {
		mcompat_log_err("SPI: Can not open SPI device %s \n", dev_fname);
	}

	if ((ioctl(fd, SPI_IOC_WR_MODE, &config->mode) < 0) ||
	    (ioctl(fd, SPI_IOC_RD_MODE, &config->mode) < 0) ||
	    (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &config->bits) < 0) ||
	    (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &config->bits) < 0) ||
	    (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &config->speed) < 0) ||
	    (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &config->speed) < 0)) {
	    close(fd);
		mcompat_log_err("SPI: ioctl error on SPI device %s", dev_fname);
	}

	opi_spi_fd = fd;

	mcompat_log(MCOMPAT_LOG_INFO, "SPI '%s': mode=%hhu, bits=%hhu, speed=%u \n", dev_fname, config->mode, config->bits, config->speed);	
}

void opi_spi_exit(void)
{	
	close(opi_spi_fd);
}

/*
void opi_set_spi_speed(uint32_t speed)
{
    pthread_mutex_lock(&opi_spi_lock);
    
	if ((ioctl(opi_spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, speed) < 0) ||
	    (ioctl(opi_spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, speed) < 0)) {
		mcompat_log(MCOMPAT_LOG_ERR, "SPI: ioctl error on SPI device");
	}

    opi_spi_config.speed = speed;
    
    pthread_mutex_unlock(&opi_spi_lock);
}
*/
bool opi_spi_transfer(uint8_t id, uint16_t *txbuf, uint16_t *rxbuf, int len)
{
	struct spi_ioc_transfer xfr;
	int ret;

    if (rxbuf != NULL) {
		memset(rxbuf, 0xff, len);
    }
    
	pthread_mutex_lock(&opi_spi_lock);
    
	opi_spi_cs_enable(id);
	usleep(1);
	
	ret = len;

	xfr.tx_buf = (unsigned long)txbuf;
	xfr.rx_buf = (unsigned long)rxbuf;
	xfr.len = len;
	xfr.speed_hz = opi_spi_config.speed;
	xfr.delay_usecs = opi_spi_config.delay;
	xfr.bits_per_word = opi_spi_config.bits;
	xfr.cs_change = 0;
	xfr.pad = 0;

	ret = ioctl(opi_spi_fd, SPI_IOC_MESSAGE(1), &xfr);
	if (ret < 1) {
		mcompat_log(MCOMPAT_LOG_ERR, "SPI: ioctl error on SPI device: %d", ret);
	}

	usleep(1);
	opi_spi_cs_disable();
    
	pthread_mutex_unlock(&opi_spi_lock);

	return ret > 0;
}


