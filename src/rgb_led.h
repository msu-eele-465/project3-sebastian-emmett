#ifndef RGB_LED_H
#define RGB_LED_H

// update the pwm values in the range 0 - 1020 (0x00 - 0xFF)
// red, green, and blue values will be used to update the appropriate CCRn register
// NOTE: do not call this without first calling pwm_init
void rgb_set(char red_intensity, char green_intensity, char blue_intensity);

#endif
