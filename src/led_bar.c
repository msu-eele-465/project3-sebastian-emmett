#include <msp430.h>
#include <stdbool.h>
#include "led_bar.h"

// We'll reference these globals from main:
extern int base_transition_period;
extern int BTP_multiplier;
extern char curr_num;
extern bool locked;
extern bool num_update;
extern bool reset_pattern;

// arrays used for pattern logic
const char COUNT_3_ARRAY[] = {0x18, 0x24, 0x42, 0x81, 0x42, 0x24};
#define COUNT_3_ARRAY_MAX_INDEX 5

char pointer_5_array[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#define pointer_5_array_END 0x80

char pointer_6_array[] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};
#define pointer_6_array_END 0xFE

char pointer_7_array[] = {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};
#define pointer_7_array_END 0xFF

// pointers and counts used for the pattern logic
unsigned char count_1 = 0xAA;
unsigned char count_2 = 0x0;
unsigned char count_3 = 0;
unsigned char count_4 = 255;
char * pointer_5 = pointer_5_array;
char * pointer_6 = pointer_6_array;
char * pointer_7 = pointer_7_array;

void led_bar_init(void)
{
    // configure digital I/O
    P3SEL0 &= ~(BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
    P3SEL1 &= ~(BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);

    P3DIR |= BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0;         // set as output
    P3OUT &= ~(BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);      // clear output
}

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

            led_bar_update(0xAA);   // display 10101010

            reset_pattern = false;
            break;

        case '1':
            BTP_multiplier = 4;
            if (reset_pattern)
            {
                count_1 = 0xAA;                
                reset_pattern = false;
            }
            else
            {
                count_1 = ~count_1;
            }

            led_bar_update(count_1);    // display either 10101010 or 01010101

            break;

        case '2':
            BTP_multiplier = 2;
            if (reset_pattern)
            {
                count_2 = 0;
                reset_pattern = false;
            }
            else
            {
                count_2++;              // unsigned expressions cannot overflow, this will automatically
                                        // roll to 0 at 255
            }

            led_bar_update(count_2);    // display binary count from 0 to 255

            break;

        case '3':
            BTP_multiplier = 2;
            if (reset_pattern)
            {
                count_3 = 0;
                reset_pattern = false;
            }
            else
            {
                (count_3 <= COUNT_3_ARRAY_MAX_INDEX) ? (count_3++) : (count_3 = 0);

            }

            led_bar_update(count_3);    // display two led's boucing against each other

            break;

        case '4':
            BTP_multiplier = 1;
            if (reset_pattern)
            {
                count_4 = 0;
                reset_pattern = false;
            }
            else
            {
                count_4--;              // unsigned expressions cannot overflow, this will automatically
                                        // roll to 255 at 0
            }

            led_bar_update(count_4);    // display binary count from 255 to 0

            break;

        case '5':
            BTP_multiplier = 6;
            if (reset_pattern)
            {
                pointer_5 = pointer_5_array;
                reset_pattern = false;
            }
            else
            {
                (*pointer_5 == pointer_5_array_END) ? (pointer_5 = pointer_5_array) : (pointer_5++);
            }

            led_bar_update(*pointer_5); // display a single bit shifting across

            break;

        case '6':
            BTP_multiplier = 2;
            if (reset_pattern)
            {
                pointer_6 = pointer_6_array;
                reset_pattern = false;
            }
            else
            {
                (*pointer_6 == pointer_6_array_END) ? (pointer_6 = pointer_6_array) : (pointer_6++);
            }

            led_bar_update(*pointer_6); // display an unflipped bit shifting across

            break;

        case '7':
            BTP_multiplier = 4;
            if (reset_pattern)
            {
                pointer_7 = pointer_7_array;
                reset_pattern = false;
            }
            else
            {
                (*pointer_7 == pointer_7_array_END) ? (pointer_7 = pointer_7_array) : (pointer_7++);
            }

            led_bar_update(*pointer_7); // display an every growing number of set bits

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