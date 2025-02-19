#include <msp430.h>
#include "../src/pwm.h"

void rgb_set(unsigned char red_intensity, unsigned char green_intensity, unsigned char blue_intensity)
{
    // since the max value for a CCRn register will be capped at 1020 each intensity
    // will be multiplied by 4 and the CCRn register updated with that value
    pwm_set_duty_ccr1((unsigned short)red_intensity * 6);
    pwm_set_duty_ccr2((unsigned short)green_intensity * 3);
    pwm_set_duty_ccr3((unsigned short)blue_intensity * 4);
}
