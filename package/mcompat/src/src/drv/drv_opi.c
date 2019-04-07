#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcompat_config.h"
#include "mcompat_drv.h"
#include "opi_spi.h"
#include "drv_opi.h"


void flush_spi(uint8_t chain_id)
{
	uint16_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
	uint16_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

	memset(spi_tx, 0, sizeof(spi_tx));
	memset(spi_rx, 0, sizeof(spi_rx));

	opi_spi_transfer(chain_id, spi_tx, spi_rx, MCOMPAT_CONFIG_MAX_CMD_LENGTH);
}

bool opi_spi_read_write(uint8_t chain_id, uint8_t *txbuf, uint8_t *rxbuf, int len)
{
	int i;
	bool ret;
	int len16 = len / 2;
	uint16_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
	uint16_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    if(rxbuf == NULL)
    {
        mcompat_log_err("%s,%s() %d: para erro! \n", __FILE__, __func__, __LINE__);
    }
    
	memset(spi_tx, 0, sizeof(spi_tx));
	memset(spi_rx, 0, sizeof(spi_rx));

	if(txbuf == NULL)
	{
		ret = opi_spi_transfer(chain_id, NULL, spi_rx, len16);
	}
	else
	{
	   for(i = 0; i < len16; i++)
	   {
		   spi_tx[i] = OPI_MAKE_WORD(txbuf[2*i], txbuf[(2*i)+1]);
	   }
	
	   ret = opi_spi_transfer(chain_id, spi_tx, spi_rx, len16);
	}

	if(!ret)
	{
		return false;
	}
	
	for(i = 0; i < len16; i++)
	{
		rxbuf[2*i]		= OPI_HI_BYTE(spi_rx[i]);
		rxbuf[(2*i)+1]	= OPI_LO_BYTE(spi_rx[i]);
	}
	
	return true;
}


bool opi_send_command(uint8_t chain_id, uint8_t cmd, uint8_t chip_id, uint8_t *buff, int len)
{
	int tx_len;
	uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
	uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    if(buff == NULL)
    {
        mcompat_log_err("%s,%s() %d: para erro! \n", __FILE__, __func__, __LINE__);
    }
    
	memset(spi_tx, 0, sizeof(spi_tx));
	memset(spi_rx, 0, sizeof(spi_rx));

	spi_tx[0] = OPI_HI_BYTE(AX_CMD_SYNC_HEAD);
	spi_tx[1] = OPI_LO_BYTE(AX_CMD_SYNC_HEAD);
	
	spi_tx[2] = cmd;
	spi_tx[3] = chip_id;
	
	if(len > 0)
	{
		memcpy(spi_tx + 4, buff, len);
	}
	
	tx_len = (4 + len + 1) & ~1;

	if(opi_spi_read_write(chain_id, spi_tx, spi_rx, tx_len))
	{
		return true;
	}
	else
	{
		mcompat_log(MCOMPAT_LOG_ERR, "send command fail !");
		return false;
	}
}


bool opi_poll_result(uint8_t chain_id, uint8_t cmd, uint8_t chip_id, uint8_t *buff, int len)
{
	int i;
	int tx_len;
	uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
	uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    if(buff == NULL)
    {
        mcompat_log_err("%s,%s() %d: para erro! \n", __FILE__, __func__, __LINE__);
    }
    
	memset(spi_tx, 0, sizeof(spi_tx));
	memset(spi_rx, 0, sizeof(spi_rx));

	tx_len = g_chip_num * 4;

	for(i = 0; i < tx_len; i++)
	{
		usleep(1);
		
		if(opi_spi_read_write(chain_id, NULL, spi_rx, 2))
		{
			break;
		}
	}

	if(i >= tx_len)
	{
	    mcompat_log(MCOMPAT_LOG_ERR, "%s,%d: poll fail !\n", __FILE__, __LINE__);
		return false;
	}

	if((spi_rx[0] != OPI_HI_BYTE(AX_CMD_SYNC_HEAD)) || (spi_rx[1] != OPI_LO_BYTE(AX_CMD_SYNC_HEAD)))
    {
		return false;
	}

	opi_spi_read_write(chain_id, NULL, spi_rx, 2);
	if(spi_rx[1] != OPI_STATUS_SUC)
	{
		return false;
	}

    opi_spi_read_write(chain_id, NULL, spi_rx, 2);
	if((spi_rx[0] & 0x0f) != cmd)
	{
		return false;
	}

	if(len > 0)
	{
		opi_spi_read_write(chain_id, NULL, spi_rx+2, len);
	}

    memcpy(buff, spi_rx, len+2);
    
	return true;
}



bool opi_send_cmd(uint8_t chain_id, uint8_t cmd, uint8_t *buff, int len)
{
	int tx_len;
	uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
	uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    if((buff == NULL) && (len != 0))
    {
        mcompat_log_err("%s,%s() %d: para erro! \n", __FILE__, __func__, __LINE__);
    }
    
	memset(spi_tx, 0, sizeof(spi_tx));
	memset(spi_rx, 0, sizeof(spi_rx));

	spi_tx[0] = OPI_HI_BYTE(CUSTOM_SYNC_HEAD);
	spi_tx[1] = OPI_LO_BYTE(CUSTOM_SYNC_HEAD);
	
	spi_tx[2] = cmd;
	spi_tx[3] = 0;
	
	if(len > 0)
	{
		memcpy(spi_tx + 4, buff, len);
	}
	
	tx_len = (4 + len + 1) & ~1;

	if(opi_spi_read_write(chain_id, spi_tx, spi_rx, tx_len))
	{
		return true;
	}
	else
	{
		mcompat_log(MCOMPAT_LOG_ERR, "send command fail !");
		return false;
	}
}


bool opi_poll_rslt(uint8_t chain_id, uint8_t cmd, uint8_t *buff, int len)
{
	int i;
	int tx_len;
	uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
	uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    if((buff == NULL) && (len != 0))
    {
        mcompat_log_err("%s,%s() %d: para erro! \n", __FILE__, __func__, __LINE__);
    }
    
	memset(spi_tx, 0, sizeof(spi_tx));
	memset(spi_rx, 0, sizeof(spi_rx));

	tx_len = g_chip_num * 4;

	for(i = 0; i < tx_len; i++)
	{
		usleep(1);
		
		if(opi_spi_read_write(chain_id, NULL, spi_rx, 2))
		{
			break;
		}
	}

	if(i >= tx_len)
	{
	    mcompat_log(MCOMPAT_LOG_ERR, "%s,%d: poll fail !\n", __FILE__, __LINE__);
		return false;
	}

	if((spi_rx[0] != OPI_HI_BYTE(CUSTOM_SYNC_HEAD)) || (spi_rx[1] != OPI_LO_BYTE(CUSTOM_SYNC_HEAD)))
    {
		return false;
	}

	opi_spi_read_write(chain_id, NULL, spi_rx, 2);
	if(spi_rx[1] != OPI_STATUS_SUC)
	{
		return false;
	}

	if(len > 0)
	{
		opi_spi_read_write(chain_id, NULL, spi_rx+2, len);
        memcpy(buff, spi_rx+2, len);
	}
    
	return true;
}


void opi_set_power_en(unsigned char chain_id, int val)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    tx_buf[0] = (val >> 0) & 0xff;
    tx_buf[1] = (val >> 8) & 0xff;

    if(!opi_send_cmd(chain_id, OPI_SET_POWER_EN, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    if(!opi_poll_rslt(chain_id, OPI_SET_POWER_EN, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }  
}


void opi_set_start_en(unsigned char chain_id, int val)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    tx_buf[0] = (val >> 0) & 0xff;
    tx_buf[1] = (val >> 8) & 0xff;

    if(!opi_send_cmd(chain_id, OPI_SET_STARR_EN, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    if(!opi_poll_rslt(chain_id, OPI_SET_STARR_EN, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }  
}


void opi_set_reset(unsigned char chain_id, int val)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    tx_buf[0] = (val >> 0) & 0xff;
    tx_buf[1] = (val >> 8) & 0xff;

    if(!opi_send_cmd(chain_id, OPI_SET_RESET, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    if(!opi_poll_rslt(chain_id, OPI_SET_RESET, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }  
}


void opi_set_led(unsigned char chain_id, int val)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    tx_buf[0] = (val >> 0) & 0xff;
    tx_buf[1] = (val >> 8) & 0xff;

    if(!opi_send_cmd(chain_id, OPI_SET_LED, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    if(!opi_poll_rslt(chain_id, OPI_SET_LED, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }  
}


int opi_get_plug(unsigned char chain_id)
{    
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    
    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(!opi_send_cmd(chain_id, OPI_SET_LED, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return -1;
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!opi_poll_rslt(chain_id, OPI_SET_LED, rx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return -1;
    }

    return (int)(rx_buf[0]);
}


bool opi_set_vid(unsigned char chain_id, int vid)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    tx_buf[0] = (vid >> 0) & 0xff;
    tx_buf[1] = (vid >> 8) & 0xff;

    if(!opi_send_cmd(chain_id, OPI_SET_VID, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    if(!opi_poll_rslt(chain_id, OPI_SET_VID, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    return true;
}


void opi_set_pwm(unsigned char fan_id, int frequency, int duty)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    tx_buf[0] = fan_id;
    tx_buf[1] = 0x00;

    tx_buf[2] = (frequency >> 0) & 0xff;
    tx_buf[3] = (frequency >> 8) & 0xff;
    tx_buf[4] = (frequency >> 16) & 0xff;
    tx_buf[5] = (frequency >> 24) & 0xff;

    tx_buf[6] = (duty >> 0) & 0xff;
    tx_buf[7] = (duty >> 8) & 0xff;

    if(!opi_send_cmd(0, OPI_SET_PWM, tx_buf, 8))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    if(!opi_poll_rslt(0, OPI_SET_PWM, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }   
}

bool opi_chain_power_on(unsigned char chain_id)
{
    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(!opi_send_cmd(chain_id, OPI_POWER_ON, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    if(!opi_poll_rslt(chain_id, OPI_POWER_ON, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    
    return true;
}


bool opi_chain_power_down(unsigned char chain_id)
{
    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(!opi_send_cmd(chain_id, OPI_POWER_DOWN, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    if(!opi_poll_rslt(chain_id, OPI_POWER_DOWN, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    
    return true;
}

bool opi_chain_hw_reset(unsigned char chain_id)
{
    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(!opi_send_cmd(chain_id, OPI_POWER_RESET, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    if(!opi_poll_rslt(chain_id, OPI_POWER_RESET, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    
    return true;
}


bool opi_chain_power_on_all(void)
{
    int i;

    for(i = 0; i < g_chain_num; i++)
    {
        opi_chain_power_on(i);
    }

    return true;
}

bool opi_chain_power_down_all(void)
{
    int i;

    for(i = 0; i < g_chain_num; i++)
    {
        opi_chain_power_down(i);
    }

    return true;
}


void opi_set_spi_speed(unsigned char chain_id, int index)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    tx_buf[0] = (index >> 0) & 0xff;
    tx_buf[1] = (index >> 8) & 0xff;

    if(!opi_send_cmd(chain_id, OPI_SET_SPI_SPEED, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    if(!opi_poll_rslt(chain_id, OPI_SET_SPI_SPEED, NULL, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }   
}

