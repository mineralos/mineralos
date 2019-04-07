
#ifndef _MCOMPAT_TVSENSOR_H_
#define _MCOMPAT_TVSENSOR_H_

typedef struct _c_temp
{
    short tmp_lo;       // lowest temperature
    short tmp_hi;       // highest temperature
    short tmp_avg;      // average temperature
} c_temp;

extern void mcompat_configure_tvsensor(int chain_id, int chip_id, bool is_tsensor);
extern void mcompat_cfg_tsadc_divider(int chain_id,unsigned int pll_clk);

extern int  mcompat_temp_to_centigrade(int temp);
extern bool mcompat_get_chain_temp(unsigned char chain_id, c_temp *chain_tmp);
extern void mcompat_get_chip_temp(unsigned char chain_id, int *chip_temp);
extern int  mcompat_volt_to_mV(int volt);
extern void mcompat_get_chip_volt(unsigned char chain_id, int *chip_volt);

#endif // #ifndef _MCOMPAT_TVSENSOR_H_


