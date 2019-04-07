
#include "mcompat_drv.h"
#include "mcompat_lib.h"
#include "drv_hub.h"

#include "mcompat_tvsensor.h"

/******************************************************************************
 * Temperature
 ******************************************************************************/

/******************************************************************************
 * Function:    mcompat_temp_to_centigrade
 * Description: temperature value to centigrade
 * Arguments:   temp        temperature value from AD
 * Return:      centigrade value
 ******************************************************************************/
int mcompat_temp_to_centigrade(int temp)
{
    if (CHIP_TECH_10NM == mcompat_get_chip_tech())
	    return ((float)TEMP_COEF_10NM - temp) * 2 / 3 + 0.5f;
    else
        return ((float)TEMP_COEF_14NM - temp) * 2 / 3 + 0.5f;
}

bool mcompat_get_chain_temp(unsigned char chain_id, c_temp *chain_tmp)
{
	uint32_t reg_val;
	uint32_t tmp_val0;
	uint32_t tmp_val1;
	uint32_t tmp_val2;
	int timeout = 1000;

	// enable auto 0a
	enable_auto_cmd0a(chain_id, 100, 33, 24, 0, 0);
	do {
		reg_val = Xil_SPI_In32(SPI_BASEADDR_GAP * chain_id + AUTO_CMD0A_REG4_ADDR);
		timeout--;
		usleep(100);
	} while(timeout && !((reg_val >> 24) & 0x1));
	if (!timeout) {
		disable_auto_cmd0a(chain_id, 100, 33, 24, 0, 0);
		return false;
	}

	// temp_lo:
	reg_val = Xil_SPI_In32(SPI_BASEADDR_GAP * chain_id + AUTO_CMD0A_REG2_ADDR);
	tmp_val0 = mcompat_temp_to_centigrade(reg_val & 0x3ff);
	tmp_val1 = mcompat_temp_to_centigrade((reg_val >> 10) & 0x3ff);
	tmp_val2 = mcompat_temp_to_centigrade((reg_val >> 20) & 0x3ff);
	chain_tmp->tmp_lo = (tmp_val0 + tmp_val1 + tmp_val2) / 3;

	// temp_hi:
	reg_val = Xil_SPI_In32(SPI_BASEADDR_GAP * chain_id + AUTO_CMD0A_REG3_ADDR);
	tmp_val0 = mcompat_temp_to_centigrade(reg_val & 0x3ff);
	tmp_val1 = mcompat_temp_to_centigrade((reg_val >> 10) & 0x3ff);
	tmp_val2 = mcompat_temp_to_centigrade((reg_val >> 20) & 0x3ff);
	chain_tmp->tmp_hi = (tmp_val0 + tmp_val1 + tmp_val2) / 3;

	// temp_avg:
	reg_val = Xil_SPI_In32(SPI_BASEADDR_GAP * chain_id + AUTO_CMD0A_REG4_ADDR);
	tmp_val1 = mcompat_temp_to_centigrade(2 * (reg_val & 0xffff) / g_chip_num);
	chain_tmp->tmp_avg = tmp_val1;

	// disable auto 0a
	disable_auto_cmd0a(chain_id, 100, 33, 24, 0, 0);

	return true;
}

void mcompat_get_chip_temp(unsigned char chain_id, int *chip_temp)
{
	int chip_id;
	unsigned char reg[REG_LENGTH] = {0};

	for (chip_id = 1; chip_id <= g_chip_num; chip_id++) {
		if (!mcompat_cmd_read_register(chain_id, chip_id, reg, REG_LENGTH)) {
			mcompat_log(MCOMPAT_LOG_ERR, "failed to read temperature for chain%d chip%d", 
				chain_id, chip_id);
			chip_temp[chip_id - 1] = mcompat_temp_to_centigrade(0);
			break;
		} else
			chip_temp[chip_id - 1] = mcompat_temp_to_centigrade(0x000003ff & ((reg[7] << 8) | reg[8]));
	}
}

/******************************************************************************
 * Voltage
 ******************************************************************************/

/******************************************************************************
 * Function:    mcompat_volt_to_mV
 * Description: voltage value to mV
 * Arguments:   volt        voltage value from AD
 * Return:      voltage value in mV
 ******************************************************************************/
int mcompat_volt_to_mV(int volt)
{
    int coef = (CHIP_TECH_10NM == mcompat_get_chip_tech()) ? VOLT_COEF_10NM : VOLT_COEF_14NM;

    return (volt * coef) >> 10;
}

void mcompat_get_chip_volt(unsigned char chain_id, int *chip_volt)
{
    int chip_id;
    unsigned char reg[REG_LENGTH] = {0};
    unsigned int volt = 0;
    
    for (chip_id = 1; chip_id <= g_chip_num; chip_id++)
    {
        if(!mcompat_cmd_read_register(chain_id, chip_id, reg, REG_LENGTH))
        {
            mcompat_log(MCOMPAT_LOG_ERR, "failed to read voltage for chain%d chip%d", chain_id, chip_id);
            chip_volt[chip_id - 1] = 0;
            break;
        }
        else
        {
            usleep(2000);
            volt = 0x000003ff & ((reg[7] << 8) | reg[8]);
            chip_volt[chip_id - 1] = mcompat_volt_to_mV(volt);
        }
    }
}

void mcompat_configure_tvsensor(int chain_id, int chip_id, bool is_tsensor)
{
    unsigned char tmp_reg[REG_LENGTH] = {0};
    unsigned char src_reg[REG_LENGTH] = {0};
    unsigned char reg[REG_LENGTH] = {0};

    mcompat_cmd_read_register(chain_id, 0x01, reg,REG_LENGTH);
    memcpy(src_reg,reg,REG_LENGTH);
    mcompat_cmd_write_register(chain_id,chip_id,src_reg,REG_LENGTH);
    usleep(200);

    if(is_tsensor)//configure for tsensor
    {
        reg[7] = (src_reg[7]&0x7f);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
        reg[7] = (src_reg[7]|0x80);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
  
        reg[6] = (src_reg[6]|0x04);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);

        //Step6: high tsadc_en
        reg[7] = (src_reg[7]|0x20);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);

        //Step7: tsadc_ana_reg_9 = 0;tsadc_ana_reg_8  = 0
        reg[5] = (src_reg[5]&0xfc);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
    
        //Step8: tsadc_ana_reg_7 = 1;tsadc_ana_reg_1 = 0
        reg[6] = (src_reg[6]&0x7d);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
    }
    else
    {
        //configure for vsensor
        reg[7] = (src_reg[7]&0x7f);
        memcpy(tmp_reg,reg,REG_LENGTH);
        
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
        reg[7] = (src_reg[7]|0x80);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
        
        reg[6] = (src_reg[6]|0x04);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
    
        //Step6: high tsadc_en
        reg[7] = (src_reg[7]|0x20);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
        
        //Step7: tsadc_ana_reg_9 = 0;tsadc_ana_reg_8  = 0
        reg[5] = ((src_reg[5]|0x01)&0xfd);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
        
        //Step8: tsadc_ana_reg_7 = 1;tsadc_ana_reg_1 = 0
        reg[6] = ((src_reg[6]|0x02)&0x7f);
        memcpy(tmp_reg,reg,REG_LENGTH);
        mcompat_cmd_write_register(chain_id,chip_id,tmp_reg,REG_LENGTH);
        usleep(200);
    }
}

void  mcompat_cfg_tsadc_divider(int chain_id,unsigned int pll_clk)
{
    unsigned int tsadc_divider_tmp;
    unsigned char  tsadc_divider;
    unsigned char  buffer[64] = {0x02,0x50,0xa0,0x06,0x28,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned char  readbuf[32] = {0};
    
    tsadc_divider_tmp = (pll_clk/2)*1000/16/650;
    tsadc_divider = (unsigned char)(tsadc_divider_tmp & 0xff);
    buffer[5] = 0x00 | tsadc_divider;

    if(!mcompat_cmd_read_write_reg0d(chain_id, 0x00, buffer, REG_LENGTH, readbuf))
    {
        mcompat_log(MCOMPAT_LOG_DEBUG,"Write t/v sensor Value Failed!\n");
    }
    else
    {
        mcompat_log(MCOMPAT_LOG_DEBUG,"Write t/v sensor Value Success!\n");
    }
}

