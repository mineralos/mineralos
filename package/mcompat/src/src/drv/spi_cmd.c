#include "mcompat_config.h"
#include "util.h"
#include "zynq_spi.h"
#include "drv_spi.h"
#include "spi_cmd.h"


ZYNQ_SPI_T s_spi[MCOMPAT_CONFIG_MAX_CHAIN_NUM];

bool init_spi_cmd(int chain_num)
{
    int i;

    for(i = 0; i < chain_num; i++)
    {
        memset((void*)&s_spi[i], 0, sizeof(ZYNQ_SPI_T));
        zynq_spi_init(&s_spi[i], i);
    }

    return true;
}

bool exit_spi_cmd(int chain_num)
{
    int i;

    for(i = 0; i < chain_num; i++)
    {
        zynq_spi_exit(&s_spi[i]);
    }

    return true;
}

bool spi_cmd_reset(unsigned char chain_id, unsigned char chip_id, unsigned char *in, unsigned char *out)
{
    ZYNQ_SPI_T *spi = &s_spi[chain_id];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(!spi_send_command(spi, CMD_RESET, chip_id, in, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s send fail !\n", __FUNCTION__);
        return false;
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!spi_poll_result(spi, CMD_RESET, chip_id, rx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s poll fail !\n", __FUNCTION__);
        return false;
    }

    memcpy(out, rx_buf, 4);
    
    return true;
}


int spi_cmd_bist_start(unsigned char chain_id, unsigned char chip_id)
{
    ZYNQ_SPI_T *spi = &s_spi[chain_id];
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!spi_send_command(spi, CMD_BIST_START, chip_id, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s send fail !\n", __FUNCTION__);
        return -1;
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!spi_poll_result(spi, CMD_BIST_START, chip_id, rx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s poll fail !\n", __FUNCTION__);
        return -1;
    }

    return rx_buf[3];
}

bool spi_cmd_bist_collect(unsigned char chain_id, unsigned char chip_id)
{
    ZYNQ_SPI_T *spi = &s_spi[chain_id];
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!spi_send_command(spi, CMD_BIST_COLLECT, chip_id, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s send fail !\n", __FUNCTION__);
        return false;
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!spi_poll_result(spi, CMD_BIST_COLLECT, chip_id, rx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s poll fail !\n", __FUNCTION__);
        return false;
    }

    return true;
}


bool spi_cmd_bist_fix(unsigned char chain_id, unsigned char chip_id)
{
    ZYNQ_SPI_T *spi = &s_spi[chain_id];
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!spi_send_command(spi, CMD_BIST_FIX, chip_id, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s send fail !\n", __FUNCTION__);
        return false;
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!spi_poll_result(spi, CMD_BIST_FIX, chip_id, rx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s poll fail !\n", __FUNCTION__);
        return false;
    }

    return true;
}


bool spi_cmd_write_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len)
{
    int i;
    unsigned short crc;
    ZYNQ_SPI_T *spi = &s_spi[chain_id];
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char tmp_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(reg == NULL)
    {
        mcompat_log_err("%s para error !\n", __FUNCTION__);
        return false;
    }

    memset(tx_buf, 0, sizeof(tx_buf));
    tx_buf[0] = CMD_WRITE_REG;
    tx_buf[1] = chip_id;
    memcpy(tx_buf + 2, reg, len);
    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = tx_buf[i + 1];
        tmp_buf[i + 1] = tx_buf[i + 0];
    }
    crc = CRC16_2(tmp_buf, len + 2);
    tx_buf[2 + len + 0] = (unsigned char)((crc >> 8) & 0xff);
    tx_buf[2 + len + 1] = (unsigned char)((crc >> 0) & 0xff);

    spi_send_data(spi, tx_buf, len + 4);

    memset(rx_buf, 0, sizeof(rx_buf));
    if(!spi_poll_result(spi, CMD_WRITE_REG, chip_id, rx_buf, len))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s poll fail !\n", __FUNCTION__);
        return false;
    }

    return true;
}


bool spi_cmd_read_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len)
{
    int i;
    int max_len;
    unsigned short crc1, crc2;
    ZYNQ_SPI_T *spi = &s_spi[chain_id];
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char tmp_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(reg == NULL)
    {
        mcompat_log_err("%s para error !\n", __FUNCTION__);
        return false;
    }

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!spi_send_command(spi, CMD_READ_REG, chip_id, tx_buf, 0))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s send fail !\n", __FUNCTION__);
        return false;
    }

    max_len = g_chip_num * 4;
    memset(rx_buf, 0, sizeof(rx_buf));

    for(i = 0; i < max_len; i = i + 2)
    {
        spi_recv_data(spi, rx_buf, 2);
        if(rx_buf[0] == RESP_READ_REG)
        {
            break;
        }
    }

    if(i >= max_len)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s poll fail !\n", __FUNCTION__);
        return false;
    }

    spi_recv_data_in_word(spi, rx_buf + 2, len + 2);

    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = rx_buf[i + 1];
        tmp_buf[i + 1] = rx_buf[i + 0];
    }
    crc1 = CRC16_2(tmp_buf, len + 2);
    crc2 = (rx_buf[2 + len + 0] << 8) + (rx_buf[2 + len + 1] << 0);

    if(crc1 != crc2)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s crc error !\n", __FUNCTION__);
        return false;
    }

    memcpy(reg, rx_buf + 2, len);

    return true;
}


bool spi_cmd_read_write_reg0d(unsigned char chain_id, unsigned char chip_id, unsigned char *in, int len, unsigned char *out)
{
    int i;
    int max_len;
    unsigned short crc;
    unsigned short crc1, crc2;
    ZYNQ_SPI_T *spi = &s_spi[chain_id];
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char tmp_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if((in == NULL) || (out == NULL))
    {
        mcompat_log_err("%s para error !\n", __FUNCTION__);
        return false;
    }

    memset(tx_buf, 0, sizeof(tx_buf));
    tx_buf[0] = CMD_WRITE_REG0d;
    tx_buf[1] = chip_id;
    memcpy(tx_buf + 2, in, len);
    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = tx_buf[i + 1];
        tmp_buf[i + 1] = tx_buf[i + 0];
    }
    crc = CRC16_2(tmp_buf, len + 2);
    tx_buf[2 + len + 0] = (unsigned char)((crc >> 8) & 0xff);
    tx_buf[2 + len + 1] = (unsigned char)((crc >> 0) & 0xff);

    spi_send_data(spi, tx_buf, len + 4);

    max_len = g_chip_num * 4;
    memset(rx_buf, 0, sizeof(rx_buf));

    for(i = 0; i < max_len; i = i + 2)
    {
        spi_recv_data(spi, rx_buf, 2);
        if((rx_buf[0] & 0x0f) == CMD_WRITE_REG0d)
        {
            break;
        }
    }

    if(i >= max_len)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s poll fail !\n", __FUNCTION__);
        return false;
    }

    spi_recv_data_in_word(spi, rx_buf + 2, len + 2);

    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = rx_buf[i + 1];
        tmp_buf[i + 1] = rx_buf[i + 0];
    }
    crc1 = CRC16_2(tmp_buf, len + 2);
    crc2 = (rx_buf[2 + len + 0] << 8) + (rx_buf[2 + len + 1] << 0);

    if(crc1 != crc2)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s crc error !\n", __FUNCTION__);
        return false;
    }

    memcpy(out, rx_buf + 2, len);

    return true;
}


bool spi_cmd_read_result(unsigned char chain_id, unsigned char chip_id, unsigned char *res, int len)
{
    int i;
    int max_len;
    unsigned short crc1, crc2;
    ZYNQ_SPI_T *spi = &s_spi[chain_id];
    unsigned char tx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char rx_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    unsigned char tmp_buf[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    if(res == NULL)
    {
        mcompat_log_err("%s para error !\n", __FUNCTION__);
        return false;
    }

    memset(tx_buf, 0, sizeof(tx_buf));
    if(!spi_send_command(spi, CMD_READ_RESULT, chip_id, tx_buf, 2))
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s send fail !\n", __FUNCTION__);
        return false;
    }

    max_len = g_chip_num * 4;
    memset(rx_buf, 0, sizeof(rx_buf));

    for(i = 0; i < max_len; i = i + 2)
    {
        spi_recv_data(spi, rx_buf, 2);
        if(((rx_buf[0] & 0x0f) == CMD_READ_RESULT) && (rx_buf[1] != 0))
        {
            break;
        }
    }

    if(i >= max_len)
    {
        return false;
    }

    spi_recv_data_in_word(spi, rx_buf + 2, len + 2);

    for(i = 0; i < len + 2; i = i + 2)
    {
        tmp_buf[i + 0] = rx_buf[i + 1];
        tmp_buf[i + 1] = rx_buf[i + 0];
    }
    crc1 = CRC16_2(tmp_buf, len + 2);
    crc2 = (rx_buf[2 + len + 0] << 8) + (rx_buf[2 + len + 1] << 0);

    if(crc1 != crc2)
    {
        mcompat_log(MCOMPAT_LOG_WARNING, "%s crc error !\n", __FUNCTION__);
        return false;
    }

    memcpy(res, rx_buf, len + 2);

    return true;
}


bool spi_cmd_write_job(unsigned char chain_id, unsigned char chip_id, unsigned char *job, int len)
{
    ZYNQ_SPI_T *spi = &s_spi[chain_id];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s(%d, %d, %p, %d)\n", __FILE__, __LINE__, __FUNCTION__, chain_id, chip_id, job, len);

    if(job == NULL)
    {
        mcompat_log_err("%s para error !\n", __FUNCTION__);
        return false;
    }

    spi_send_data(spi, job, len);

    return true;
}


