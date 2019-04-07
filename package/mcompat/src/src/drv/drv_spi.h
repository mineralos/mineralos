#ifndef _DRV_SPI_
#define _DRV_SPI_

#include "zynq_spi.h"


void spi_send_data(ZYNQ_SPI_T *spi, unsigned char *buf, int len);
void spi_recv_data(ZYNQ_SPI_T *spi, unsigned char *buf, int len);

void spi_send_data_in_word(ZYNQ_SPI_T *spi, unsigned char *buf, int len);
void spi_recv_data_in_word(ZYNQ_SPI_T *spi, unsigned char *buf, int len);

bool spi_send_command(ZYNQ_SPI_T *spi, unsigned char cmd, unsigned char chip_id, unsigned char *buff, int len);
bool spi_poll_result(ZYNQ_SPI_T *spi, unsigned char cmd, unsigned char chip_id, unsigned char *buff, int len);


void init_spi_gpio(int chain_num);
void exit_spi_gpio(int chain_num);


void spi_set_power_en(unsigned char chain_id, int val);
void spi_set_start_en(unsigned char chain_id, int val);
void spi_set_reset(unsigned char chain_id, int val);
void spi_set_led(unsigned char chain_id, int val);
bool spi_set_vid(unsigned char chain_id, int vid);
int spi_get_plug(unsigned char chain_id);

void spi_set_spi_speed(unsigned char chain_id, int index);


#endif
