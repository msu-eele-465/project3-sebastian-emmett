#include <msp430.h>
#include <stdbool.h>
#include <string.h>

// 4x4 Keypad Layout
static const char keypadMap[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// ----------------------------------------------------------------------------
// init_keypad: 
//   Rows (P5.0..3) as outputs
//   Columns now on P4.4..7 - active-high requires pull-downs
// ----------------------------------------------------------------------------
void init_keypad(void)
{
    // Rows = outputs, start low
    P5DIR |= (BIT0 | BIT1 | BIT2 | BIT3);
    P5OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    // Columns = inputs on P4.4..7
    P4DIR &= ~(BIT4 | BIT5 | BIT6 | BIT7);

    // Enable internal pull-downs for columns on P4.4..7
    P4REN |=  (BIT4 | BIT5 | BIT6 | BIT7);
    P4OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
}

// ----------------------------------------------------------------------------
// poll_keypad:
//   1) For each row, set that row high, all others low
//   2) Read columns (P4.4..7). If any column bit is 1 => pressed key
//   3) Return the char from keypadMap[row][col], or 0 if none
// ----------------------------------------------------------------------------
char poll_keypad(void)
{
    #define ROW_MASK (BIT0 | BIT1 | BIT2 | BIT3)

    unsigned int row;
    for (row = 0; row < 4; row++)
    {
        // Clear all rows
        P5OUT &= ~ROW_MASK;
        // Drive *only* this row high
        P5OUT |= (BIT0 << row);

        // Small settle delay - DO NOT REMOVE THIS LMAO
        __delay_cycles(50);

        // Read columns from P4.4..7 => shift right by 4, mask 0x0F
        unsigned char colState = (P4IN >> 4) & 0x0F;

        unsigned int col;
        for (col = 0; col < 4; col++)
        {
            // If column bit is high => pressed key
            if (colState & (1 << col))
            {
                // Reset rows
                P5OUT &= ~ROW_MASK;
                // Return the key from keypadMap[row][col]
                return keypadMap[row][col];
            }
        }
    }

    // No key found
    P5OUT &= ~ROW_MASK;
    return 0;
}
