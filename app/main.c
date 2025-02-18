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
// Globals! (yes they deserve their own lil space)
// ----------------------------------------------------------------------------
// For general key tracking
char curr_key = 0;
char prev_key = 0;

// For numeric-only tracking
char curr_num = 0;
char prev_num = 0;

// A boolean for locked/unlocked
bool locked = true; // Default true ofc

// Our base transition period variable. Technically this is an int representation of how many 1/16s our actual BTP is but whatever
int base_transition_period = 4;

// If a numeric key is pressed, set num_update = true
bool num_update = false;

// If the new numeric key == previous numeric key, set reset_pattern = true
bool reset_pattern = false;

// Variables for our passcode
bool unlocking = false;             // True while we're collecting 4 digits
static const char correct_pass[] = "1234";  // Hard-coded correct passcode
char pass_entered[5];              // Room for 4 digits + null terminator
unsigned pass_index = 0;           // How many digits we've collected so far

// Timeout tracking: We'll use the heartbeat (1Hz) to decrement
volatile int pass_timer = 0;       // 5-second countdown if unlocking

// ----------------------------------------------------------------------------
// Function Prototypes
// ----------------------------------------------------------------------------
void init_heartbeat(void);     // Heartbeat LED on P1.0 (1 Hz)
void init_keypad(void);       
void init_responseLED(void);   // LED on P6.6
char poll_keypad(void);        // Active-high keypad scan
void init_keyscan_timer(void); // Timer_B1 => ~50 ms

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
    init_keyscan_timer(); // Timer_B1 => ~50 ms interrupt

    // Enable global interrupts for the heartbeat timer
    __bis_SR_register(GIE);

    while(1)
    {
        // ----------------------------------------------------------------------------
        // 1) If we are locked but NOT unlocking...
        //    - Wait for the first numeric key press (num_update)
        // ----------------------------------------------------------------------------
        if (locked && !unlocking)
        {
            // If no numeric key has been pressed yet, do nothing
            if (!num_update)
            {
                // Just idle here
                __no_operation();
            }
            else
            {
                // We got a numeric press => start unlocking process
                unlocking = true;
                num_update = false;  // We consumed this press

                // Start collecting passcode digits
                pass_index = 0;

                // The digit that triggered num_update is in 'curr_num'
                pass_entered[pass_index++] = curr_num;

                // Start 5-second timer
                pass_timer = 5;
            }
        }

        // ----------------------------------------------------------------------------
        // 2) If we ARE locked and in the middle of unlocking...
        //    - Gather a total of 4 numeric keys
        // ----------------------------------------------------------------------------
        else if (locked && unlocking)
        {
            // Check if we've run out of time
            if (pass_timer == 0)
            {
                // Time’s up => reset D:
                unlocking = false;
                pass_index = 0;
            }
            else
            {
                // We still have time => collect digits!
                if (pass_index < 4)
                {
                    if (num_update)
                    {
                        pass_entered[pass_index++] = curr_num;
                        num_update = false;
                    }
                }
                else
                {
                    // 4 digits => compare
                    pass_entered[4] = '\0';
                    if (strcmp(pass_entered, correct_pass) == 0)
                    {
                        // Correct => unlock
                        locked = false;
                    }
                    // Otherwise => stay locked, reset
                    unlocking = false;
                }
            }
        }

        // ----------------------------------------------------------------------------
        // 3) If we are NOT locked => do nothing special
        // ----------------------------------------------------------------------------
        else
        {
            // locked == false => do nothing
            __no_operation();
        }
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

    // Decrement pass_timer if we're in unlocking mode
    if (unlocking && pass_timer > 0)
    {
        pass_timer--;
    }
}

// ----------------------------------------------------------------------------
// init_keyscan_timer: Timer_B1 => fires ~every 50 ms
// ----------------------------------------------------------------------------
void init_keyscan_timer(void)
{
    // For 50 ms @ ~1 MHz SMCLK, /64 => 1 MHz/64 = 15625 Hz
    // 50 ms => 0.05 * 15625 = 781 counts
    TB1CTL   = TBSSEL__SMCLK | ID__8 | MC__UP | TBCLR; 
    TB1EX0   = TBIDEX__8;        // total /64
    TB1CCR0  = 781;             // ~50 ms 
    TB1CCTL0 = CCIE;            // Enable CCR0 interrupt
}

// ----------------------------------------------------------------------------
// Timer_B1 ISR => polls keypad every ~50ms and runs keyboard_interrupt logic
// ----------------------------------------------------------------------------
#pragma vector=TIMER1_B0_VECTOR
__interrupt void TIMER1_B0_ISR(void)
{
    char key = poll_keypad();
    if (key != 0)  // A key was detected
    {
        // Toggle LED on any key press
        P6OUT ^= BIT6;

        // Shift curr_key -> prev_key, store the new key
        prev_key = curr_key;
        curr_key = key;

        // ---------------------------
        // Additional Logic
        // ---------------------------
        
        // 1) If 'D' => set locked to true
        if (key == 'D')
        {
            locked = true;
        }
        // 2) If 'A' => base_transition_period -= 4, min=4
        else if (key == 'A')
        {
            base_transition_period -= 4;
            if (base_transition_period < 4)
            {
                base_transition_period = 4;
            }
        }
        // 3) If 'B' => base_transition_period += 4
        else if (key == 'B')
        {
            base_transition_period += 4;
        }
        // 4) If key is numeric => update prev_num/curr_num, set flags
        else if (key >= '0' && key <= '9')
        {
            // SHIFT
            prev_num = curr_num;
            curr_num = key;

            // SET num_update = true
            num_update = true;

            // If prev_num == curr_num => reset_pattern = true
            if (prev_num == curr_num)
            {
                reset_pattern = true;
            }
        }
        // else: ignore other keys (*, #, C)

        // Debounce ~50ms
        __delay_cycles(50000);

        // Wait until key is released
        while (poll_keypad() != 0)
        {
            // do nothing until no key is pressed
        }
    }
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
