#include <rtthread.h>
#include "board.h"


static rt_int32_t FLASH_INTERVAL_MICROSECONDS = 500;


static rt_thread_t led_flash_th = RT_NULL;
static rt_mutex_t led_flash_mutex = RT_NULL;


void debug_led1_on(void)
{
    rt_pin_write(DEBUG_LED1_PIN, PIN_HIGH);
}


void debug_led1_off(void)
{
    rt_pin_write(DEBUG_LED1_PIN, PIN_LOW);
}


void debug_led1_flash_entry(void *param)
{
    while (1) {
        debug_led1_on();
        rt_thread_mdelay(FLASH_INTERVAL_MICROSECONDS);
        debug_led1_off();
        rt_thread_mdelay(FLASH_INTERVAL_MICROSECONDS);
    }
}


void debug_led1_pin_init(void)
{
    rt_pin_mode(DEBUG_LED1_PIN, PIN_MODE_OUTPUT);
    led_flash_mutex = rt_mutex_create("flash_mutex", RT_IPC_FLAG_PRIO);
}



void debug_led1_start_flash(rt_int32_t flash_interval_microseconds)
{
    
    rt_mutex_take(led_flash_mutex, RT_WAITING_FOREVER);

    FLASH_INTERVAL_MICROSECONDS = flash_interval_microseconds > 0 ? flash_interval_microseconds : 500;
    
    if (led_flash_th == RT_NULL) {
        led_flash_th = rt_thread_create("debug_led1", &debug_led1_flash_entry, NULL, 1024, RT_THREAD_PRIORITY_MAX / 2, 10);
    }
    if (led_flash_th != RT_NULL) {
        rt_thread_startup(led_flash_th);
    }

    rt_mutex_release(led_flash_mutex);
}


void debug_led1_stop_flash(void)
{
    rt_mutex_take(led_flash_mutex, RT_WAITING_FOREVER);
    if (led_flash_th != RT_NULL) {
        rt_thread_delete(led_flash_th);
    }
    led_flash_th = RT_NULL;
    debug_led1_off();
    rt_mutex_release(led_flash_mutex);
}


void test_debug_led1_flash(void)
{
    debug_led1_pin_init();
    
    while (1) {
        rt_kprintf("led start flash 500...\n");
        debug_led1_start_flash(500);

        rt_thread_mdelay(5000);
        debug_led1_stop_flash();
        rt_kprintf("led stop flash...\n");

        rt_thread_mdelay(5000);

        rt_kprintf("led start flash 250...\n");
        debug_led1_start_flash(250);

        rt_thread_mdelay(5000);
        debug_led1_stop_flash();
        rt_kprintf("led stop flash...\n");

        rt_thread_mdelay(5000);
    }
}
