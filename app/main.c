#include <msp430.h>
#include <stdbool.h>

// 4x4 Keypad Layout
static const char keypadMap[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// Variables for most recent and previous key
char curr_key = 0;
char prev_key = 0;

// ----------------------------------------------------------------------------
// Function Prototypes
// ----------------------------------------------------------------------------
void init_heartbeat(void);    // Heartbeat LED on P1.0 (1 Hz)
void init_keypad(void);       
void init_responseLED(void);  // LED on P6.6
char poll_keypad(void);

// ----------------------------------------------------------------------------
// MAIN
// ----------------------------------------------------------------------------
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog
    PM5CTL0 &= ~LOCKLPM5;      // Unlock I/Os

    init_heartbeat();    // Set up Timer_B0 for blinking P1.0
    init_keypad();       // Init keyboard with P6s as input and P5s as output
    init_responseLED();  // LED on P6.6 to show when a key gets pressed

    // Enable global interrupts for the heartbeat timer
    __bis_SR_register(GIE);

    while(1)
    {
        // Poll the keypad
        char key = poll_keypad();

        if (key != 0)  // A key was detected
        {
            // Toggle LED on *any* key press
            P6OUT ^= BIT6;

            // Shift curr_key -> prev_key, store the new key
            prev_key = curr_key;
            curr_key = key;

            // Debounce ~200ms
            __delay_cycles(200000);

            // Wait until key is released
            while (poll_keypad() != 0)
            {
                // do nothing until no key is pressed
            }
        }

        // ~20ms gap between polls
        __delay_cycles(20000);
    }
}

// ----------------------------------------------------------------------------
// init_heartbeat: Toggle P1.0 ~ once per second using Timer_B
// ----------------------------------------------------------------------------
void init_heartbeat(void)
{
    // P1.0 as output, off initially
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    // Configure Timer_B0 for ~1Hz if SMCLK ~1MHz
    TB0CTL  = TBSSEL__SMCLK | ID__8 | MC__UP | TBCLR; // SMCLK/8, up mode
    TB0EX0  = TBIDEX__8;                              // further /8 => total /64
    TB0CCR0 = 15625;                                  // 1 second at ~1 MHz/64
    TB0CCTL0= CCIE;                                   // enable CCR0 interrupt :D
}

// ----------------------------------------------------------------------------
// Timer_B0 ISR => toggles P1.0 (heartbeat)
// ----------------------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void)
{
    P1OUT ^= BIT0;  // Toggle heartbeat LED!
}

// ----------------------------------------------------------------------------
// init_keypad: 
//   Rows (P5.0..3) as outputs
//   Columns (P6.0..3) as inputs - active-high requires pull-downs
// ----------------------------------------------------------------------------
void init_keypad(void)
{
    // Rows = outputs, start low
    P5DIR |= (BIT0 | BIT1 | BIT2 | BIT3);
    P5OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    // Columns = inputs
    P6DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    // Enable internal pull-downs
    P6REN |=  (BIT0 | BIT1 | BIT2 | BIT3);
    P6OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
}

// ----------------------------------------------------------------------------
// init_responseLED: LED on P6.6 (off initially)
// ----------------------------------------------------------------------------
void init_responseLED(void)
{
    P6DIR |= BIT6;
    P6OUT &= ~BIT6;
}

// ----------------------------------------------------------------------------
// poll_keypad:
//   1) For each row, set that row high, all others low
//   2) Read columns. If any column bit is 1 => pressed key
//   3) Return the char from keypadMap[row][col], or 0 if none
// ----------------------------------------------------------------------------
char poll_keypad(void)
{
    #define ROW_MASK (BIT0 | BIT1 | BIT2 | BIT3)

    int row;
    for (row = 0; row < 4; row++)
    {
        // Clear all rows
        P5OUT &= ~ROW_MASK;
        // Drive *only* this row high
        P5OUT |= (BIT0 << row);

        // Small settle delay - DO NOT REMOVE THIS LMAO
        __delay_cycles(50);

        // Read columns (low nibble of P6 :D)
        unsigned char colState = P6IN & 0x0F;

        int col;
        for (col = 0; col < 4; col++)
        {
            // If column bit is high => pressed key
            if (colState & (1 << col))
            {
                // Reset rows
                P5OUT &= ~ROW_MASK;
                // Return the key from keypadMap
                return keypadMap[row][col];
            }
        }
    }

    // No key found
    P5OUT &= ~ROW_MASK;
    return 0;
}
