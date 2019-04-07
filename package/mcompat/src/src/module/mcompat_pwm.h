#ifndef _MCOMPAT_PWM_
#define _MCOMPAT_PWM_


typedef struct MCOMPAT_PWM_TAG{
    //
    void (*set_pwm)(unsigned char, int, int);
}MCOMPAT_PWM_T;



void init_mcompat_pwm(void);
void exit_mcompat_pwm(void);

void register_mcompat_pwm(MCOMPAT_PWM_T * ops);



#endif
