#include <msp430.h>
#include "../src/pwm.h"

void rgb_set(char red_intensity, char green_intensity, char blue_intensity)
{
    // since the max value for a CCRn register will be capped at 1020 each intensity
    // will be multiplied by 4 and the CCRn register updated with that value
    pwm_set_duty_ccr1(red_intensity * 4);
    pwm_set_duty_ccr2(green_intensity * 4);
    pwm_set_duty_ccr3(blue_intensity * 4);
}
