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

// ----------------------------------------------------------------------------
// MAIN
// ----------------------------------------------------------------------------
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog
    PM5CTL0 &= ~LOCKLPM5;      // Unlock I/Os

    init_heartbeat();    // Set up Timer_B0 for blinking P1.0

    // Enable global interrupts for the heartbeat timer
    __bis_SR_register(GIE);

    while(1)
    {
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