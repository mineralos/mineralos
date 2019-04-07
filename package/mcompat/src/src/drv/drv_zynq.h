#ifndef _DRV_ZYNQ_
#define _DRV_ZYNQ_


bool zynq_chain_power_on(unsigned char chain_id, unsigned char mode);

bool zynq_chain_power_down(unsigned char chain_id);

bool zynq_chain_hw_reset(unsigned char chain_id);

bool zynq_chain_power_on_all(void);

bool zynq_chain_power_down_all(void);



#endif
