#ifndef _HUB_CMD_
#define _HUB_CMD_

bool init_hub_cmd(int chain_num, int chip_num);

bool exit_hub_cmd(int chain_num);

bool hub_cmd_reset(unsigned char chain_id, unsigned char chip_id, unsigned char *in, unsigned char *out);

int hub_cmd_bist_start(unsigned char chain_id, unsigned char chip_id);

bool hub_cmd_bist_collect(unsigned char chain_id, unsigned char chip_id);

bool hub_cmd_bist_fix(unsigned char chain_id, unsigned char chip_id);

bool hub_cmd_write_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len);

bool hub_cmd_read_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len);

bool hub_cmd_read_write_reg0d(unsigned char chain_id, unsigned char chip_id, unsigned char *in, int len, unsigned char *out);

bool hub_cmd_read_result(unsigned char chain_id, unsigned char chip_id, unsigned char *res, int len);

bool hub_cmd_write_job(unsigned char chain_id, unsigned char chip_id, unsigned char *job, int len);

bool hub_cmd_auto_nonce(unsigned char chain_id, int mode, int len);

bool hub_cmd_read_nonce(unsigned char chain_id, unsigned char *res, int len);

//bool hub_cmd_get_temp(mcompat_fan_temp_s *fan_temp_ctrl);

bool hub_get_chain_temp(unsigned char chain_id, short tmp_hi, short tmp_lo, short tmp_avg);

#endif
