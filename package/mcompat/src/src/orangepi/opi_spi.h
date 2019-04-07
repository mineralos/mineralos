#ifndef _OPI_SPI_H_
#define _OPI_SPI_H_

#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdbool.h>
#include <stdint.h>


#define PIN_SPI_A0		_11
#define PIN_SPI_A1		_12
#define PIN_SPI_A2		_13

#define PIN_SPI_E1		_22


#define SPI_DEVICE_TEMPLATE		"/dev/spidev%d.%d"
#define DEFAULT_SPI_BUS			1
#define DEFAULT_SPI_CS_LINE		0
#define DEFAULT_SPI_MODE		SPI_MODE_1
#define DEFAULT_SPI_BITS_PER_WORD	16
#define DEFAULT_SPI_SPEED		1500000
#define DEFAULT_SPI_DELAY_USECS		0


struct spi_config {
	int bus;
	int cs_line;
	uint8_t mode;
	uint32_t speed;
	uint8_t bits;
	uint16_t delay;
};

struct spi_ctx {
	int fd;
	int power_en;
	int start_en;
	int reset;
	int led;
	int plug;
	int id;
	struct spi_config config;
};


void opi_spi_init(void);

void opi_spi_exit(void);

//void opi_set_spi_speed(uint32_t speed);

bool opi_spi_transfer(uint8_t id, uint16_t *txbuf, uint16_t *rxbuf, int len);


#endif /* _OPI_SPI_H_ */
