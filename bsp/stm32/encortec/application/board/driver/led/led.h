#ifndef __LED_H__
#define __LED_H__

#include <rtthread.h>

void debug_led1_pin_init(void);
void debug_led1_on(void);
void debug_led1_off(void);
void debug_led1_start_flash(rt_int32_t flash_interval_microseconds);
void debug_led1_stop_flash(void);


#endif