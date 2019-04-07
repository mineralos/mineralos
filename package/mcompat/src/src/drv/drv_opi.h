#ifndef _DRV_OPI_
#define _DRV_OPI_

#define AX_CMD_SYNC_HEAD	0xA55A
#define CUSTOM_SYNC_HEAD	0x6996

#define OPI_SPI_TIMEOUT     100

#define OPI_STATUS_SUC      0
#define OPI_STATUS_EOR      1

#define OPI_SET_POWER_EN    (0x11)
#define OPI_SET_STARR_EN    (0x12)
#define OPI_SET_RESET       (0x13)
#define OPI_SET_LED         (0x14)
#define OPI_GET_PLUG        (0x15)
#define OPI_SET_VID         (0x16)
#define OPI_SET_PWM         (0x17)
#define OPI_SET_SPI_SPEED   (0x18)
#define OPI_POWER_ON        (0x21)
#define OPI_POWER_DOWN      (0x22)
#define OPI_POWER_RESET     (0x23)



#define OPI_HI_BYTE(a)      ((uint8_t)(((a) >> 8) & 0xff))
#define OPI_LO_BYTE(a)      ((uint8_t)(((a) >> 0) & 0xff))

#define OPI_MAKE_WORD(a, b)	(uint16_t)((((a) & 0xff) << 8) | (( (a) & 0xff) << 0))


bool opi_spi_read_write(uint8_t chain_id, uint8_t *txbuf, uint8_t *rxbuf, int len);

bool opi_send_command(uint8_t chain_id, uint8_t cmd, uint8_t chip_id, uint8_t *buff, int len);

bool opi_poll_result(uint8_t chain_id, uint8_t cmd, uint8_t chip_id, uint8_t *buff, int len);


void opi_set_power_en(unsigned char chain_id, int val);

void opi_set_start_en(unsigned char chain_id, int val);

void opi_set_reset(unsigned char chain_id, int val);

void opi_set_led(unsigned char chain_id, int val);

int opi_get_plug(unsigned char chain_id);

bool opi_set_vid(unsigned char chain_id, int vid);

void opi_set_pwm(unsigned char fan_id, int frequency, int duty);


bool opi_chain_power_on(unsigned char chain_id);

bool opi_chain_power_down(unsigned char chain_id);

bool opi_chain_hw_reset(unsigned char chain_id);

bool opi_chain_power_on_all(void);

bool opi_chain_power_down_all(void);


void opi_set_spi_speed(unsigned char chain_id, int index);



#endif
