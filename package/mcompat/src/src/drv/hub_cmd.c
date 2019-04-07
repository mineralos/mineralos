#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcompat_config.h"
#include "mcompat_lib.h"
#include "zynq_spi.h"
#include "drv_hub.h"

#include "hub_cmd.h"

bool init_hub_cmd(int chain_num, int chip_num)
{
    int i;

    for(i = 0; i < chain_num; i++)
    {
        hub_spi_init(i, chip_num);
    }

    return true;
}

bool exit_hub_cmd(int chain_num)
{
    return true;
}


bool hub_cmd_reset(unsigned char chain_id, unsigned char chip_id, unsigned char *in, unsigned char *out)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    spi_tx[0] = CMD_RESET;
    spi_tx[1] = chip_id;
    spi_tx[2] = in[0];
    spi_tx[3] = in[1];

    //cfg_len = 0x03000200;
    cfg_len += (0x03) << 24;
    cfg_len += (0x00) << 16;
    cfg_len += (0x02) << 8;
    cfg_len += (0x00) << 0;

    if(do_spi_cmd(chain_id, spi_tx, spi_rx, cfg_len) == XST_FAILURE)
    {
        return false;
    }

    //print_data_hex("tx:", spi_tx, 8);
    //print_data_hex("rx:", spi_rx, 8);
    memcpy(out, spi_rx, 4);

    return true;
}


int hub_cmd_bist_start(unsigned char chain_id, unsigned char chip_id)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    spi_tx[0] = CMD_BIST_START;
    spi_tx[1] = chip_id;

    //cfg_len = 0x02000200;
    cfg_len += (0x02) << 24;
    cfg_len += (0x00) << 16;
    cfg_len += (0x02) << 8;
    cfg_len += (0x00) << 0;

    if(do_spi_cmd(chain_id, spi_tx, spi_rx, cfg_len) == XST_FAILURE)
    {
        return -1;
    }

    //print_data_hex("tx:", spi_tx, 8);
    //print_data_hex("rx:", spi_rx, 8);

    return spi_rx[3];;
}


bool hub_cmd_bist_collect(unsigned char chain_id, unsigned char chip_id)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    spi_tx[0] = CMD_BIST_COLLECT;
    spi_tx[1] = chip_id;

    //cfg_len = 0x02000200;
    cfg_len += (0x02) << 24;
    cfg_len += (0x00) << 16;
    cfg_len += (0x02) << 8;
    cfg_len += (0x00) << 0;

    if(do_spi_cmd(chain_id, spi_tx, spi_rx, cfg_len) == XST_FAILURE)
    {
        return false;
    }

    return true;
}


bool hub_cmd_bist_fix(unsigned char chain_id, unsigned char chip_id)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    spi_tx[0] = CMD_BIST_FIX;
    spi_tx[1] = chip_id;

    //cfg_len = 0x02000200;
    cfg_len += (0x02) << 24;
    cfg_len += (0x00) << 16;
    cfg_len += (0x02) << 8;
    cfg_len += (0x00) << 0;

    if(do_spi_cmd(chain_id, spi_tx, spi_rx, cfg_len) == XST_FAILURE)
    {
        return false;
    }

    return true;
}


bool hub_cmd_write_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    spi_tx[0] = CMD_WRITE_REG;
    spi_tx[1] = chip_id;
    memcpy(spi_tx + 2, reg, len);

    //cfg_len = 0x09070807;
    cfg_len += (((len / 2) + 2) & 0xff) << 24;
    cfg_len += (((len / 2) + 1) & 0xff) << 16;
    cfg_len += (((len / 2) + 2) & 0xff) << 8;
    cfg_len += (((len / 2) + 1) & 0xff) << 0;

    if(do_spi_cmd(chain_id, spi_tx, spi_rx, cfg_len) == XST_FAILURE)
    {
        return false;
    }

    return true;
}


bool hub_cmd_read_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    spi_tx[0] = CMD_READ_REG;
    spi_tx[1] = chip_id;

    //cfg_len = 0x02000807;
    cfg_len += (0x02) << 24;
    cfg_len += (0x00) << 16;
    cfg_len += (((len / 2) + 2) & 0xff) << 8;
    cfg_len += (((len / 2) + 1) & 0xff) << 0;

    if(do_spi_cmd(chain_id, spi_tx, spi_rx, cfg_len) == XST_FAILURE)
    {
        return false;
    }

    memcpy(reg, spi_rx + 2, len);

    return true;
}


bool hub_cmd_read_write_reg0d(unsigned char chain_id, unsigned char chip_id, unsigned char *in, int len, unsigned char *out)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: %s \n", __FILE__, __LINE__, __FUNCTION__);

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    spi_tx[0] = CMD_WRITE_REG0d;
    spi_tx[1] = chip_id;
    memcpy(spi_tx + 2, in, len);

    //cfg_len = 0x09070807;
    cfg_len += (((len / 2) + 2) & 0xff) << 24;
    cfg_len += (((len / 2) + 1) & 0xff) << 16;
    cfg_len += (((len / 2) + 2) & 0xff) << 8;
    cfg_len += (((len / 2) + 1) & 0xff) << 0;

    if(do_spi_cmd(chain_id, spi_tx, spi_rx, cfg_len) == XST_FAILURE)
    {
        return false;
    }

    //print_data_hex("tx:", spi_tx, 32);
    //print_data_hex("rx:", spi_rx, 32);

    memcpy(out, spi_rx + 2, len);

    return true;
}

bool hub_cmd_write_job(unsigned char chain_id, unsigned char chip_id, unsigned char *job, int len)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    memcpy(spi_tx, job, len);
    //cfg_len = 0x504f0000;
    cfg_len += (((len / 2) + 1) & 0xff) << 24;
    cfg_len += (((len / 2) - 1) & 0xff) << 16;
    cfg_len += (0x00) << 8;
    cfg_len += (0x00) << 0;

    if(send_job_queue(chain_id, spi_tx, spi_rx, cfg_len, (chip_id == 1)) == XST_FAILURE)
    {
        return false;
    }

    return true;
}


bool hub_cmd_read_result(unsigned char chain_id, unsigned char chip_id, unsigned char *res, int len)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    spi_tx[0] = CMD_READ_RESULT;
    spi_tx[1] = chip_id;

    //cfg_len = 0x02000604;
    cfg_len += (0x02) << 24;
    cfg_len += (0x00) << 16;
    cfg_len += (((len / 2) + 2) & 0xff) << 8;
    cfg_len += (((len / 2) + 1) & 0xff) << 0;

    if(do_spi_cmd(chain_id, spi_tx, spi_rx, cfg_len) == XST_FAILURE)
    {
        return false;
    }

    if(spi_rx[1] == 0)
    {
        return false;
    }

    //print_data_hex("read_result rx:", spi_rx, len + 2);
    memcpy(res, spi_rx, len + 2);

    return true;
}


bool hub_cmd_auto_nonce(unsigned char chain_id, int mode, int len)
{
    uint16_t cmd08 = 0x0800;
    uint32_t cfg_len = 0;

    cfg_len += (0x02) << 24;
    cfg_len += (0x00) << 16;
    cfg_len += (((len / 2) + 2) & 0xff) << 8;
    cfg_len += (((len / 2) + 1) & 0xff) << 0;

    if(mode == 0)
    {
        disable_auto_nonce(chain_id);
    }
    else
    {
        enable_auto_nonce(chain_id, cmd08, cfg_len);
    }

    return true;
}

bool hub_cmd_read_nonce(unsigned char chain_id, unsigned char *res, int len)
{
    uint32_t cfg_len = 0;
    uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];
    uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH];

    memset(spi_tx, 0, sizeof(spi_tx));
    memset(spi_rx, 0, sizeof(spi_rx));

    //cfg_len = 0x02000604;
    cfg_len += (0x02) << 24;
    cfg_len += (0x00) << 16;
    cfg_len += (((len / 2) + 2) & 0xff) << 8;
    cfg_len += (((len / 2) + 1) & 0xff) << 0;

    if(!rece_queue_has_nonce(chain_id, 1000))
    {
        return false;
    }

    read_nonce_buffer(chain_id, spi_rx, cfg_len);

    if(spi_rx[1] == 0)
    {
        return false;
    }

    //print_data_hex("read_nonce rx:", spi_rx, len + 2);
    memcpy(res, spi_rx, len + 2);

    return true;
}


bool hub_cmd_get_temp(mcompat_fan_temp_s *fan_temp_ctrl,unsigned char chain_id)
{
  uint32_t val;

      if(hub_get_plug(chain_id))
        return false;

      enable_auto_cmd0a(chain_id,g_dangerous_temp,33,24,0,0);
      mcompat_temp_s *temp_ctrl = &fan_temp_ctrl->mcompat_temp[chain_id];

      do{
        val = Xil_SPI_In32(SPI_AXIBASE + SPI_BASEADDR_GAP*chain_id+AUTO_CMD0A_REG4_ADDR);
       }while(!((val >> 24) & 0x1));

      hub_get_hitemp_stat(chain_id,temp_ctrl);
      hub_get_lotemp_stat(chain_id,temp_ctrl);
      hub_get_avgtemp_stat(chain_id,temp_ctrl);
      disable_auto_cmd0a(chain_id,g_dangerous_temp,33,24,0,0);


 return true;
}
