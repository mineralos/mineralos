#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>

#include "util.h"
#include "mcompat_config.h"
#include "opi_spi.h"
#include "drv_opi.h"
#include "opi_cmd.h"


bool init_opi_cmd(void)
{
    opi_spi_init();
    
    return true;
}

bool exit_opi_cmd(void)
{
    opi_spi_exit();    

    return true;
}


bool opi_cmd_reset(unsigned char chain_id, unsigned char chip_id, unsigned char *in, unsigned char *out)
{
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(!opi_send_command(chain_id, CMD_RESET, chip_id, in, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!opi_poll_result(chain_id, CMD_RESET, chip_id, rx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memcpy(out, rx_buf, 4);
    
    return true;
}


int opi_cmd_bist_start(unsigned char chain_id, unsigned char chip_id)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!opi_send_command(chain_id, CMD_BIST_START, chip_id, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return -1;
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!opi_poll_result(chain_id, CMD_BIST_START, chip_id, rx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return -1;
    }

    return rx_buf[3];
}


bool opi_cmd_bist_collect(unsigned char chain_id, unsigned char chip_id)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!opi_send_command(chain_id, CMD_BIST_COLLECT, chip_id, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!opi_poll_result(chain_id, CMD_BIST_COLLECT, chip_id, rx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    return true;
}


bool opi_cmd_bist_fix(unsigned char chain_id, unsigned char chip_id)
{
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!opi_send_command(chain_id, CMD_BIST_FIX, chip_id, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!opi_poll_result(chain_id, CMD_BIST_FIX, chip_id, rx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    return true;
}


bool opi_cmd_write_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len)
{
    int i;
    unsigned short crc;
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char tmp_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(reg == NULL)
    {
        mcompat_log_err("%s,%d: %s para error !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memset(tx_buf, 0, sizeof(tx_buf));
    tx_buf[0] = OPI_HI_BYTE(AX_CMD_SYNC_HEAD);
	tx_buf[1] = OPI_LO_BYTE(AX_CMD_SYNC_HEAD);
    tx_buf[2] = CMD_WRITE_REG;
    tx_buf[3] = chip_id;
    memcpy(tx_buf + 4, reg, len);
    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = tx_buf[i + 1 + 2];
        tmp_buf[i + 1] = tx_buf[i + 0 + 2];
    }
    crc = CRC16_2(tmp_buf, len + 2);
    tx_buf[4 + len + 0] = (unsigned char)((crc >> 8) & 0xff);
    tx_buf[4 + len + 1] = (unsigned char)((crc >> 0) & 0xff);

    opi_spi_read_write(chain_id, tx_buf, rx_buf, len + 6);

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!opi_poll_result(chain_id, CMD_WRITE_REG, chip_id, rx_buf, len))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    return true;
}

bool opi_cmd_read_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len)
{
    int i;
    int max_len;
    unsigned short crc1, crc2;
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char tmp_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(reg == NULL)
    {
        mcompat_log_err("%s,%d: %s para error !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!opi_send_command(chain_id, CMD_READ_REG, chip_id, tx_buf, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    max_len = g_chip_num * 4;
    memset(rx_buf, 0, sizeof(rx_buf));

    for(i = 0; i < max_len; i = i + 2)
    {
        opi_spi_read_write(chain_id, NULL, rx_buf, 2);
        if((rx_buf[0] & 0x0f) == CMD_READ_REG)
        {
            break;
        }
    }

    if(i >= max_len)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    opi_spi_read_write(chain_id, NULL, rx_buf + 2, len + 2);

    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = rx_buf[i + 1];
        tmp_buf[i + 1] = rx_buf[i + 0];
    }
    crc1 = CRC16_2(tmp_buf, len + 2);
    crc2 = (rx_buf[2 + len + 0] << 8) + (rx_buf[2 + len + 1] << 0);

    if(crc1 != crc2)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s crc fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memcpy(reg, rx_buf + 2, len);

    return true;
}


bool opi_cmd_read_write_reg0d(unsigned char chain_id, unsigned char chip_id, unsigned char *in, int len, unsigned char *out)
{
    int i;
    int max_len;
    unsigned short crc;
    unsigned short crc1, crc2;
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char tmp_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if((in == NULL) || (out == NULL))
    {
        mcompat_log_err("%s,%d: %s para error !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memset(tx_buf, 0, sizeof(tx_buf));
    tx_buf[0] = OPI_HI_BYTE(AX_CMD_SYNC_HEAD);
	tx_buf[1] = OPI_LO_BYTE(AX_CMD_SYNC_HEAD);
    tx_buf[2] = CMD_WRITE_REG0d;
    tx_buf[3] = chip_id;
    memcpy(tx_buf + 4, in, len);
    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = tx_buf[i + 1 + 2];
        tmp_buf[i + 1] = tx_buf[i + 0 + 2];
    }
    crc = CRC16_2(tmp_buf, len + 2);
    tx_buf[4 + len + 0] = (unsigned char)((crc >> 8) & 0xff);
    tx_buf[4 + len + 1] = (unsigned char)((crc >> 0) & 0xff);

    opi_spi_read_write(chain_id, tx_buf, rx_buf, len + 6);

    max_len = g_chip_num * 4;
    memset(rx_buf, 0, sizeof(rx_buf));

    for(i = 0; i < max_len; i = i + 2)
    {
        opi_spi_read_write(chain_id, NULL, rx_buf, 2);
        if((rx_buf[0] & 0x0f) == CMD_WRITE_REG0d)
        {
            break;
        }
    }

    if(i >= max_len)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    opi_spi_read_write(chain_id, NULL, rx_buf + 2, len + 2);

    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = rx_buf[i + 1];
        tmp_buf[i + 1] = rx_buf[i + 0];
    }
    crc1 = CRC16_2(tmp_buf, len + 2);
    crc2 = (rx_buf[2 + len + 0] << 8) + (rx_buf[2 + len + 1] << 0);

    if(crc1 != crc2)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s crc fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memcpy(out, rx_buf + 2, len);

    return true;
}


bool opi_cmd_read_result(unsigned char chain_id, unsigned char chip_id, unsigned char *res, int len)
{
    int i;
    int max_len;
    unsigned short crc1, crc2;
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char tmp_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(res == NULL)
    {
        mcompat_log_err("%s,%d: %s para error !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!opi_send_command(chain_id, CMD_READ_RESULT, chip_id, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s send fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    max_len = g_chip_num * 4;
    memset(rx_buf, 0, sizeof(rx_buf));

    for(i = 0; i < max_len; i = i + 2)
    {
        opi_spi_read_write(chain_id, NULL, rx_buf, 2);
        if(((rx_buf[0] & 0x0f) == CMD_READ_RESULT) && (rx_buf[1] != 0))
        {
            break;
        }
    }

    if(i >= max_len)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s poll fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    opi_spi_read_write(chain_id, NULL, rx_buf + 2, len + 2);

    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = rx_buf[i + 1];
        tmp_buf[i + 1] = rx_buf[i + 0];
    }
    crc1 = CRC16_2(tmp_buf, len + 2);
    crc2 = (rx_buf[2 + len + 0] << 8) + (rx_buf[2 + len + 1] << 0);

    if(crc1 != crc2)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s,%d: %s crc fail !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    memcpy(res, rx_buf, len + 2);

    return true;
}


bool opi_cmd_write_job(unsigned char chain_id, unsigned char chip_id, unsigned char *job, int len)
{
    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s(%d, %d, %p, %d)\n", __FILE__, __LINE__, __FUNCTION__, chain_id, chip_id, job, len);

    if(job == NULL)
    {
        mcompat_log_err("%s,%d: %s para error !\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    
    return opi_spi_read_write(chain_id, NULL, job, len);
}




