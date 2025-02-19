#include <msp430.h>
#include "../src/pwm.h"
#include "../src/rgb_led.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog

    pwm_init();
    unsigned short i = 0;
    unsigned short j = 0;

    PM5CTL0 &= ~LOCKLPM5;      // Unlock I/Os

    while(1)
    {
        // purple
        rgb_set(0xb7, 0x0, 0xff);

        for(j = 0; j < 5; j++)
        {
            for(i = 0; i < 65000; i++);
        }

        // orange
        rgb_set(0xff, 0x91, 0x00);

        for(j = 0; j < 5; j++)
        {
            for(i = 0; i < 65000; i++);
        }

        // cyan
        rgb_set(0x00, 0xff, 0xee);

        for(j = 0; j < 5; j++)
        {
            for(i = 0; i < 65000; i++);
        }

        // yellow
        rgb_set(0xFB, 0xFF, 0x00);

        for(j = 0; j < 5; j++)
        {
            for(i = 0; i < 65000; i++);
        }
    }
}
