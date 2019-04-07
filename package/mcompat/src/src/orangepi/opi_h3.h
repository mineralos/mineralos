#ifndef _OPI_H3_H_
#define _OPI_H3_H_

#include <stdint.h>

#define SW_PORTC_IO_BASE 0x01c20800
#define GPIO_BANK(pin)	((pin) >> 5)
#define GPIO_CFG_INDEX(pin)	(((pin) & 0x1F) >> 3)
#define GPIO_CFG_OFFSET(pin)	((((pin) & 0x1F) & 0x7) << 2)
#define GPIO_NUM(pin)	((pin) & 0x1F)
#define GPIO_PUL_INDEX(pin)	(((pin) & 0x1F )>> 4)
#define GPIO_PUL_OFFSET(pin)	(((pin) & 0x0F) << 1)

struct sunxi_gpio {
    unsigned int cfg[4];
    unsigned int dat;
    unsigned int drv[2];
    unsigned int pull[2];
};

struct sunxi_gpio_int {
    unsigned int cfg[3];
    unsigned int ctl;
    unsigned int sta;
    unsigned int deb;
};

struct sunxi_gpio_reg {
    struct sunxi_gpio gpio_bank[9];
    unsigned char res[0xbc];
    struct sunxi_gpio_int gpio_int;
};

typedef struct
{
	uint8_t mode;
	uint8_t bits;
	uint32_t speed;
	uint16_t delay;
} spi_config_t;

//左侧排针GPIO口共 15 个
/*
用法:
gpio_setcfg(PA12, OUTPUT);
gpio_output(PA12, HIGH);
*/
#define PA12 12
#define PA11 11
#define PA6  6
#define PA0  0
#define PA1  1
#define PA3  3
#define PC0  64
#define PC1  65
#define PC2  66
#define PA19 19
#define PA7  7
#define PA8  8
#define PA9  9
#define PA10 10
#define PA20 20

//右侧排针GPIO口共 13 个
#define PA13 13
#define PA14 14
#define PD14 110
#define PC4  68
#define PC7  71
#define PA2  2
#define PC3  67
#define PA21 21
#define PA18 18
#define PG8  200
#define PG9  201
#define PG6  198
#define PG7  199


//板载LED指示灯共 2 个
#define PA15 15
#define PL10 362
#define PL3 355

//按排针号定义共 28 个
/*
用法:
gpio_setcfg(_3, OUTPUT);
gpio_output(_3, HIGH);
*/
#define _3   12
#define _5   11
#define _7   6
#define _8   13
#define _10  14
#define _11  0
#define _12  110
#define _13  1
#define _15  3
#define _16  68
#define _18  71
#define _19  64
#define _21  65
#define _22  2
#define _23  66
#define _24  67
#define _26  21
#define _27  19
#define _28  18
#define _29  7
#define _31  8
#define _32  200
#define _33  9
#define _35  10
#define _36  201
#define _37  20
#define _38  198
#define _40  199

//板载LED指示灯共 2 个
#define STATUS_LED 15
#define POWER_LED  362
#define POWER_KEY 355
//高电平.
#define HIGH 1
//低电平.
#define LOW 0

//IO 方向
#define INPUT 0
#define OUTPUT 1

//拉操作
#define PUTDOWM 2
#define PUTUP 1


int gpio_init(void);
int gpio_setcfg(unsigned int pin, unsigned int p1);
int gpio_getcfg(unsigned int pin);
int gpio_output(unsigned int pin, unsigned int p1);
int gpio_pullup(unsigned int pin, unsigned int p1);
int gpio_input(unsigned int pin);
int i2c_open(char *dev, uint8_t address);
int i2c_close(int fd);
int i2c_send(int fd, uint8_t *buf, uint8_t num_bytes);
int i2c_read(int fd, uint8_t *buf, uint8_t num_bytes);
int spi_open(char *dev, spi_config_t config);
int spi_close(int fd);
int spi_xfer(int fd, uint8_t *tx_buf, uint8_t tx_len, uint8_t *rx_buf, uint8_t rx_len);
int spi_read(int fd, uint8_t *rx_buf, uint8_t rx_len);
int spi_write(int fd, uint8_t *tx_buffer, uint8_t tx_len);
void delay(unsigned int howLong);


#endif
