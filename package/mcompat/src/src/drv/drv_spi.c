#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcompat_config.h"
#include "zynq_spi.h"
#include "zynq_vid.h"
#include "zynq_gpio.h"
#include "mcompat_drv.h"


const int pin_power_en[] =
{
    MCOMPAT_CONFIG_CHAIN0_POWER_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN1_POWER_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN2_POWER_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN3_POWER_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN4_POWER_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN5_POWER_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN6_POWER_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN7_POWER_EN_GPIO
};

const int pin_start_en[] =
{
    MCOMPAT_CONFIG_CHAIN0_START_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN1_START_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN2_START_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN3_START_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN4_START_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN5_START_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN6_START_EN_GPIO,
    MCOMPAT_CONFIG_CHAIN7_START_EN_GPIO
};

const int pin_reset[] =
{
    MCOMPAT_CONFIG_CHAIN0_RESET_GPIO,
    MCOMPAT_CONFIG_CHAIN1_RESET_GPIO,
    MCOMPAT_CONFIG_CHAIN2_RESET_GPIO,
    MCOMPAT_CONFIG_CHAIN3_RESET_GPIO,
    MCOMPAT_CONFIG_CHAIN4_RESET_GPIO,
    MCOMPAT_CONFIG_CHAIN5_RESET_GPIO,
    MCOMPAT_CONFIG_CHAIN6_RESET_GPIO,
    MCOMPAT_CONFIG_CHAIN7_RESET_GPIO
};

const int pin_plug[] =
{
    MCOMPAT_CONFIG_CHAIN0_PLUG_GPIO,
    MCOMPAT_CONFIG_CHAIN1_PLUG_GPIO,
    MCOMPAT_CONFIG_CHAIN2_PLUG_GPIO,
    MCOMPAT_CONFIG_CHAIN3_PLUG_GPIO,
    MCOMPAT_CONFIG_CHAIN4_PLUG_GPIO,
    MCOMPAT_CONFIG_CHAIN5_PLUG_GPIO,
    MCOMPAT_CONFIG_CHAIN6_PLUG_GPIO,
    MCOMPAT_CONFIG_CHAIN7_PLUG_GPIO
};

const int pin_led[] =
{
    MCOMPAT_CONFIG_CHAIN0_LED_GPIO,
    MCOMPAT_CONFIG_CHAIN1_LED_GPIO,
    MCOMPAT_CONFIG_CHAIN2_LED_GPIO,
    MCOMPAT_CONFIG_CHAIN3_LED_GPIO,
    MCOMPAT_CONFIG_CHAIN4_LED_GPIO,
    MCOMPAT_CONFIG_CHAIN5_LED_GPIO,
    MCOMPAT_CONFIG_CHAIN6_LED_GPIO,
    MCOMPAT_CONFIG_CHAIN7_LED_GPIO
};


void spi_send_data_in_word(ZYNQ_SPI_T *spi, unsigned char *buf, int len)
{
    int i;

    for(i = 0; i < len; i = i + 2)
    {
        zynq_spi_write(spi, buf + i, 2);
    }
}

void spi_recv_data_in_word(ZYNQ_SPI_T *spi, unsigned char *buf, int len)
{
    int i;

    for(i = 0; i < len; i = i + 2)
    {
        zynq_spi_read(spi, buf + i, 2);
    }
}

void spi_send_data(ZYNQ_SPI_T *spi, unsigned char *buf, int len)
{
    zynq_spi_write(spi, buf, len);
}

void spi_recv_data(ZYNQ_SPI_T *spi, unsigned char *buf, int len)
{
    zynq_spi_read(spi, buf, len);
}


bool spi_send_command(ZYNQ_SPI_T *spi, unsigned char cmd, unsigned char chip_id, unsigned char *buff, int len)
{
    int tx_len;
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    if((len > 0) && (buff == NULL))
    {
        mcompat_log_err("%s,%d: para error !\n", __FILE__, __LINE__);
        return false;
    }

    memset(tx_buf, 0, sizeof(tx_buf));

    tx_buf[0] = cmd;
    tx_buf[1] = chip_id;

    if(len > 0)
    {
        memcpy(tx_buf + 2, buff, len);
    }

    tx_len = (2 + len + 1) & ~1;
    //spi_send_data_in_word(spi, tx_buf, tx_len);
    spi_send_data(spi, tx_buf, tx_len);

    return true;
}


bool spi_poll_result(ZYNQ_SPI_T *spi, unsigned char cmd, unsigned char chip_id, unsigned char *buff, int len)
{
    int i;
    int max_len;
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    max_len = g_chip_num * 4;
    memset(rx_buf, 0, sizeof(rx_buf));

    for(i = 0; i < max_len; i = i + 2)
    {
        spi_recv_data(spi, rx_buf, 2);
        if((rx_buf[0] & 0x0f) == cmd)
        {
            break;
        }
    }

    if(i >= max_len)
    {
        mcompat_log(MCOMPAT_LOG_ERR, "%s,%d: poll fail !\n", __FILE__, __LINE__);
        return false;
    }

    spi_recv_data_in_word(spi, rx_buf+2, len);
    memcpy(buff, rx_buf, len+2);

    return true;
}



void init_spi_gpio(int chain_num)
{
    int i;

    for(i = 0; i < chain_num; i++)
    {
        zynq_gpio_init(pin_power_en[i], 0);
        zynq_gpio_init(pin_start_en[i], 0);
        zynq_gpio_init(pin_reset[i], 0);
        zynq_gpio_init(pin_led[i], 0);
        zynq_gpio_init(pin_plug[i], 1);
    }
}

void exit_spi_gpio(int chain_num)
{
    int i;

    for(i = 0; i < chain_num; i++)
    {
        zynq_gpio_exit(pin_power_en[i]);
        zynq_gpio_exit(pin_start_en[i]);
        zynq_gpio_exit(pin_reset[i]);
        zynq_gpio_exit(pin_led[i]);
        zynq_gpio_exit(pin_plug[i]);
    }
}


void spi_set_power_en(unsigned char chain_id, int val)
{
    zynq_gpio_write(pin_power_en[chain_id], val);
}

void spi_set_start_en(unsigned char chain_id, int val)
{
    zynq_gpio_write(pin_start_en[chain_id], val);
}

void spi_set_reset(unsigned char chain_id, int val)
{
    zynq_gpio_write(pin_reset[chain_id], val);
}

void spi_set_led(unsigned char chain_id, int val)
{
    zynq_gpio_write(pin_led[chain_id], val);
}

int spi_get_plug(unsigned char chain_id)
{
    return zynq_gpio_read(pin_plug[chain_id]);
}

static int s_vid = 0;
bool spi_set_vid(unsigned char chain_id, int vid)
{
    if(g_platform == PLATFORM_ZYNQ_SPI_G19)
    {
        zynq_gpio_g19_vid_set(chain_id, vid);
    }
    else if(g_platform == PLATFORM_ZYNQ_SPI_G9)
    {
        if(s_vid != vid)
        {
            zynq_gpio_g9_vid_set(vid);
        }
    }
    else
    {
        mcompat_log(MCOMPAT_LOG_ERR, "platform[%d] error in set vid \n", g_platform);
        return false;
    }

    return true;
}

void spi_set_spi_speed(unsigned char chain_id, int index)
{
    uint32_t cfg[] = {390625, 781250, 1562500, 3125000, 6250000, 9960000};

    zynq_set_spi_speed(cfg[index]);
}


