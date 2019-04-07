#ifndef _MCOMPAT_CHAIN_
#define _MCOMPAT_CHAIN_


typedef struct MCOMPAT_CHAIN_TAG{
    //
    bool (*power_on)(unsigned char, unsigned char);
    //
    bool (*power_down)(unsigned char);
    //
    bool (*hw_reset)(unsigned char);
    //
    bool (*power_on_all)(void);
    //
    bool (*power_down_all)(void);
}MCOMPAT_CHAIN_T;



void init_mcompat_chain(void);
void exit_mcompat_chain(void);

void register_mcompat_chain(MCOMPAT_CHAIN_T * ops);


bool mcompat_chain_power_on(unsigned char chain_id, unsigned char mode);

bool mcompat_chain_power_down(unsigned char chain_id);

bool mcompat_chain_hw_reset(unsigned char chain_id);


#endif
