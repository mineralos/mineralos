#ifndef _MCOMPAT_CMD_
#define _MCOMPAT_CMD_

#include "mcompat_fan.h"

typedef struct MCOMPAT_CMD_TAG{
    //
    void (*set_speed)(unsigned char, int);
    //
    bool (*cmd_reset)(unsigned char, unsigned char, unsigned char *, unsigned char *);
    //
    int (*cmd_bist_start)(unsigned char, unsigned char);
    //
    bool (*cmd_bist_collect)(unsigned char, unsigned char);
    //
    bool (*cmd_bist_fix)(unsigned char, unsigned char);
    //
    bool (*cmd_write_register)(unsigned char, unsigned char, unsigned char *, int);
    //
    bool (*cmd_read_register)(unsigned char, unsigned char, unsigned char *, int);
    //
    bool (*cmd_read_write_reg0d)(unsigned char, unsigned char, unsigned char *, int, unsigned char *);
    //
    bool (*cmd_write_job)(unsigned char, unsigned char, unsigned char *, int);
    //
    bool (*cmd_read_result)(unsigned char, unsigned char, unsigned char *, int);
    //
    bool (*cmd_auto_nonce)(unsigned char, int, int);
    //
    bool (*cmd_read_nonce)(unsigned char, unsigned char *, int);

    bool (*cmd_get_temp)(mcompat_fan_temp_s *temp_ctrl);
}MCOMPAT_CMD_T;



void init_mcompat_cmd(void);
void exit_mcompat_cmd(void);

void register_mcompat_cmd(MCOMPAT_CMD_T * cmd_ops_p);



#endif
