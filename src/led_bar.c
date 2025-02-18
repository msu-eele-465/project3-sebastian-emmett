#include <msp430.h>
#include <stdbool.h>
#include "led_bar.h"

// We'll reference these globals from main:
extern int base_transition_period;
extern int BTP_multiplier;
extern int curr_num;
extern bool locked;
extern bool num_update;
extern bool reset_pattern;

// ----------------------------------------------------------------------------
// led_bar_update_pattern:
//   Single switch case over curr_num, cases for 0-7
//   Set BTP_multiplier = 2 in the respective value
//   React to reset_pattern
//   Then update the pattern accordingly #TODO
// ----------------------------------------------------------------------------
void led_bar_update_pattern(void)
{
    switch (curr_num)
    {
        case '0':
            BTP_multiplier = 1;
            // Set LED bar to stationary 10101010 #TODO
            reset_pattern = false;
            break;

        case '1':
            BTP_multiplier = 4;
            if (reset_pattern)
            {
                // True outcome space
            }
            else
            {
                // False outcome space
            }
            break;

        case '2':
            BTP_multiplier = 2;
            if (reset_pattern)
            {
                // True outcome space
            }
            else
            {
                // False outcome space
            }
            break;

        case '3':
            BTP_multiplier = 2;
            if (reset_pattern)
            {
                // True outcome space
            }
            else
            {
                // False outcome space
            }
            break;

        case '4':
            BTP_multiplier = 1;
            if (reset_pattern)
            {
                // True outcome space
            }
            else
            {
                // False outcome space
            }
            break;

        case '5':
            BTP_multiplier = 6;
            if (reset_pattern)
            {
                // True outcome space
            }
            else
            {
                // False outcome space
            }
            break;

        case '6':
            BTP_multiplier = 2;
            if (reset_pattern)
            {
                // True outcome space
            }
            else
            {
                // False outcome space
            }
            break;

        case '7':
            BTP_multiplier = 4;
            if (reset_pattern)
            {
                // True outcome space
            }
            else
            {
                // False outcome space
            }
            break;

        default:
            // No changes? Not 100% sure
            break;
    }

    return;
}

// ----------------------------------------------------------------------------
// led_bar_delay: 
// Run a delay loop for the appropriate amount of time, kicking out if system gets locked or gets a new pattern
// ----------------------------------------------------------------------------
void led_bar_delay(void)
{
    // local variable to keep track of how long the delay is
    int loop_count = base_transition_period * BTP_multiplier;

    // while loop_count > 0
    while (loop_count > 0)
    {
        // If locked == true, return out of the function
        if (locked == true)
        {
            return;
        }

        // If num_update == true, return out of the function
        if (num_update == true)
        {
            return;
        }

        // else, delay for 1/16th of a second
        // (~1 MHz clock => 62500 cycles ~1/16s)
        __delay_cycles(62500);

        // decrement loop_count
        loop_count--;
    }
}