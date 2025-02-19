#include <msp430.h>
#include "../src/pwm.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog

    pwm_init();

    PM5CTL0 &= ~LOCKLPM5;      // Unlock I/Os

    //while(1);

    pwm_set_duty_ccr1(1000);
    pwm_set_duty_ccr2(1000);
    pwm_set_duty_ccr3(1000);

    while(1);
}
