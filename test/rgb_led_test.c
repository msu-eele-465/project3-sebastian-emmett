#include <msp430.h>
#include "../src/pwm.h"
#include "../src/rgb_led.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog

    pwm_init();

    PM5CTL0 &= ~LOCKLPM5;      // Unlock I/Os

    //while(1);

    rgb_set(0xc8, 0x00, 0xff);

    while(1);
}
