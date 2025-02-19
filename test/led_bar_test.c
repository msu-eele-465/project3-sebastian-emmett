#include <msp430.h>
#include <stdbool.h>
#include <string.h>
#include "../src/keyboard.h"  // Include this to use init_keypad() and poll_keypad()
#include "../src/heartbeat.h" // For init_heartbeat()
#include "../src/led_bar.h" // For led_bar_update_pattern() and led_bar_delay

// ----------------------------------------------------------------------------
// Globals! (yes they deserve their own lil space)
// ----------------------------------------------------------------------------
// For general key tracking
bool key_down = false;
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

// The global int BTP_multiplier:
int BTP_multiplier = 0;

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
void init_heartbeat(void);  // Heartbeat LED on P1.0 (1 Hz)

// ----------------------------------------------------------------------------
// MAIN
// ----------------------------------------------------------------------------
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog

    init_heartbeat();    // Set up Timer_B0 for blinking P1.0
    init_keypad();       // Init keyboard with P6s as input and P5s as output
    init_responseLED();  // LED on P6.6 to show when a key gets pressed
    init_keyscan_timer(); // Timer_B1 => ~50 ms interrupt
    led_bar_init();

    PM5CTL0 &= ~LOCKLPM5;      // Unlock I/Os

    // Enable global interrupts for the heartbeat timer
    __bis_SR_register(GIE);

    while(1)
    {
        led_bar_update_pattern();
        locked = false;
        num_update = false;
        led_bar_delay();
    }
}