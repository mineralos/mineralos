#ifndef _MCOMPAT_GPIO_
#define _MCOMPAT_GPIO_


typedef struct MCOMPAT_GPIO_TAG{
    //
    void (*set_power_en)(unsigned char, int);
    //
    void (*set_start_en)(unsigned char, int);
    //
    void (*set_reset)(unsigned char, int);
    //
    void (*set_led)(unsigned char, int);
    //
    int (*get_plug)(unsigned char);
    //
    bool (*set_vid)(unsigned char, int);
    //
    void (*set_green_led)(int mode);
    //
    void (*set_red_led)(int mode);
    //
    int (*get_button)(void);
}MCOMPAT_GPIO_T;



void init_mcompat_gpio(void);
void exit_mcompat_gpio(void);

void register_mcompat_gpio(MCOMPAT_GPIO_T * ops);



#endif
