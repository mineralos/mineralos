#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>

#include "opi_h3.h"


unsigned int SUNXI_IO_BASE = 0;

/*******************************************************************************************
GPIO
*******************************************************************************************/
int gpio_init(void)
{
    int fd;
    unsigned int addr_start, addr_offset, PageSize, PageMask;
    void *pc;

    fd = open("/dev/mem", O_RDWR);
    if (fd < 0)
        return -1;

    PageSize = sysconf(_SC_PAGESIZE);
    PageMask = (~(PageSize - 1));

    addr_start = SW_PORTC_IO_BASE & PageMask;
    addr_offset = SW_PORTC_IO_BASE & ~PageMask;

    pc = (void *)mmap(0, PageSize * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr_start);

    if (pc == MAP_FAILED)
        return -1;

    SUNXI_IO_BASE = (unsigned int) pc;
    SUNXI_IO_BASE += addr_offset;

    close(fd);
    return 0;
}

int gpio_setcfg(unsigned int pin, unsigned int p1)
{
    unsigned int cfg;
    unsigned int bank = GPIO_BANK(pin);
    unsigned int index = GPIO_CFG_INDEX(pin);
    unsigned int offset = GPIO_CFG_OFFSET(pin);

    if(SUNXI_IO_BASE == 0)
        return -1;

    struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *) SUNXI_IO_BASE)->gpio_bank[bank];

    cfg = *(&pio->cfg[0] + index);
    cfg &= ~(0xf << offset);
    cfg |= p1 << offset;

    *(&pio->cfg[0] + index) = cfg;

    return 0;
}

int gpio_getcfg(unsigned int pin)
{
    unsigned int cfg;
    unsigned int bank = GPIO_BANK(pin);
    unsigned int index = GPIO_CFG_INDEX(pin);
    unsigned int offset = GPIO_CFG_OFFSET(pin);

    if (SUNXI_IO_BASE == 0)
        return -0;

    struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *) SUNXI_IO_BASE)->gpio_bank[bank];
    cfg = *(&pio->cfg[0] + index);
    cfg >>= offset;
    
    return (cfg & 0xf);
}

int gpio_output(unsigned int pin, unsigned int p1)
{
    unsigned int bank = GPIO_BANK(pin);
    unsigned int num = GPIO_NUM(pin);

    if (SUNXI_IO_BASE == 0)
        return -1;

    struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *) SUNXI_IO_BASE)->gpio_bank[bank];

    if (p1)
        *(&pio->dat) |= 1 << num;
    else
        *(&pio->dat) &= ~(1 << num);

    return 0;
}

int gpio_pullup(unsigned int pin, unsigned int p1)
{
    unsigned int cfg;
    unsigned int bank = GPIO_BANK(pin);
    unsigned int index = GPIO_PUL_INDEX(pin);
    unsigned int offset = GPIO_PUL_OFFSET(pin);

    if (SUNXI_IO_BASE == 0)
        return -1;

    struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *) SUNXI_IO_BASE)->gpio_bank[bank];

    cfg = *(&pio->pull[0] + index);
    cfg &= ~(0x3 << offset);
    cfg |= p1 << offset;

    *(&pio->pull[0] + index) = cfg;

    return 0;
}

int gpio_input(unsigned int pin)
{
    unsigned int dat;
    unsigned int bank = GPIO_BANK(pin);
    unsigned int num = GPIO_NUM(pin);

    if (SUNXI_IO_BASE == 0)
        return -1;

    struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *) SUNXI_IO_BASE)->gpio_bank[bank];

    dat = *(&pio->dat);
    dat >>= num;

    return (dat & 0x1);
}

/*******************************************************************************************
I2C
*******************************************************************************************/
int i2c_open(char *dev, uint8_t address)
{
    int fd;
    int ret;

    fd = open(dev, O_RDWR);
    if (fd < 0)
        return fd;

    ret = ioctl(fd, I2C_SLAVE_FORCE, address);
    if (ret < 0)
        return ret;

    return fd;
}

int i2c_close(int fd)
{
    return (close(fd));
}

int i2c_send(int fd, uint8_t *buf, uint8_t num_bytes)
{
    return (write(fd, buf, num_bytes));
}

int i2c_read(int fd, uint8_t *buf, uint8_t num_bytes)
{
    return (write(fd, buf, num_bytes));
}

/*******************************************************************************************
spi
*******************************************************************************************/
int spi_open(char *dev, spi_config_t config)
{
    int fd;

    fd = open(dev, O_RDWR);
    if (fd < 0)
        return fd;

    /* Set SPI_POL and SPI_PHA */
    if (ioctl(fd, SPI_IOC_WR_MODE, &config.mode) < 0)
        return -1;

    if (ioctl(fd, SPI_IOC_RD_MODE, &config.mode) < 0)
        return -1;

    /* Set bits per word*/
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &config.bits) < 0)
        return -1;

    if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &config.bits) < 0)
        return -1;

    /* Set SPI speed*/
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &config.speed) < 0)
        return -1;

    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &config.speed) < 0)
        return -1;

    return fd;
}

int spi_close(int fd)
{
    return (close(fd));
}

int spi_xfer(int fd, uint8_t *tx_buf, uint8_t tx_len, uint8_t *rx_buf, uint8_t rx_len)
{
    struct spi_ioc_transfer spi_message[2];

    memset(spi_message, 0, sizeof(spi_message));

    spi_message[0].rx_buf = (unsigned long)tx_buf;
    spi_message[0].len = tx_len;

    spi_message[1].tx_buf = (unsigned long)rx_buf;
    spi_message[1].len = rx_len;

    return ioctl(fd, SPI_IOC_MESSAGE(2), spi_message);
}

int spi_read(int fd, uint8_t *rx_buf, uint8_t rx_len)
{
    struct spi_ioc_transfer spi_message[1];
    memset(spi_message, 0, sizeof(spi_message));

    spi_message[0].rx_buf = (unsigned long)rx_buf;
    spi_message[0].len = rx_len;

    return ioctl(fd, SPI_IOC_MESSAGE(1), spi_message);
}

int spi_write(int fd, uint8_t *tx_buffer, uint8_t tx_len)
{
    struct spi_ioc_transfer spi_message[1];
    memset(spi_message, 0, sizeof(spi_message));

    spi_message[0].tx_buf = (unsigned long)tx_buffer;
    spi_message[0].len = tx_len;

    return ioctl(fd, SPI_IOC_MESSAGE(1), spi_message);
}

/*******************************************************************************************
time
*******************************************************************************************/
void delay(unsigned int howLong)
{
    struct timespec sleeper, dummy ;

    sleeper.tv_sec  = (time_t)(howLong / 1000) ;
    sleeper.tv_nsec = (long)(howLong % 1000) * 1000000 ;

    nanosleep (&sleeper, &dummy) ;
}
