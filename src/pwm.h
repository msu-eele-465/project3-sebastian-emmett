#ifndef PWM_H
#define PWM_H

// CCRn based pwm on TimerB3
// there is no ISR for this since it outputs directly to P6.0 - P6.2
void pwm_init(void);

// update CCRn for TimerB3 to the new on pulse width
// in other words, update the duty cycle
#define pwm_set_duty(ccrn, new_pulse_width) (ccrn = new_pulse_width)

// update CCR0 for TimerB3 to the new period
#define pwm_set_period(new_period) (TB3CCR0 = new_period)

#endif
