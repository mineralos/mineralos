#ifndef _OPI_CMD_
#define _OPI_CMD_

bool init_opi_cmd(void);

bool exit_opi_cmd(void);

bool opi_cmd_reset(unsigned char chain_id, unsigned char chip_id, unsigned char *in, unsigned char *out);

int opi_cmd_bist_start(unsigned char chain_id, unsigned char chip_id);

bool opi_cmd_bist_collect(unsigned char chain_id, unsigned char chip_id);

bool opi_cmd_bist_fix(unsigned char chain_id, unsigned char chip_id);

bool opi_cmd_write_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len);

bool opi_cmd_read_register(unsigned char chain_id, unsigned char chip_id, unsigned char *reg, int len);

bool opi_cmd_read_write_reg0d(unsigned char chain_id, unsigned char chip_id, unsigned char *in, int len, unsigned char *out);

bool opi_cmd_read_result(unsigned char chain_id, unsigned char chip_id, unsigned char *res, int len);

bool opi_cmd_write_job(unsigned char chain_id, unsigned char chip_id, unsigned char *job, int len);





#endif
