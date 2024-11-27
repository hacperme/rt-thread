#include <rtthread.h>
#include "board.h"


#define FLASH_INTERVAL_MICROSECONDS 500


static rt_thread_t led_flash_th = RT_NULL;
static rt_mutex_t led_flash_mutex = RT_NULL;


rt_err_t debug_led1_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(DEBUG_LED1_PIN, mode);
    return rt_pin_read(DEBUG_LED1_PIN) == mode ? RT_EOK : RT_ERROR;
}


rt_err_t debug_led1_on(void)
{
    return debug_led1_pin_enable(PIN_HIGH);
}


rt_err_t debug_led1_off(void)
{
    return debug_led1_pin_enable(PIN_LOW);
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



void debug_led1_start_flash(void)
{
    
    rt_mutex_take(led_flash_mutex, RT_WAITING_FOREVER);
    
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
    rt_thread_delete(led_flash_th);
    led_flash_th = RT_NULL;
    rt_mutex_release(led_flash_mutex);
}


void debug_led1_flash(void)
{
    debug_led1_pin_init();

    debug_led1_on();
    rt_kprintf("led1 on...\n");

    rt_thread_mdelay(10000);
    debug_led1_off();
    rt_kprintf("led1 off after 10 seconds.\n");

    rt_thread_mdelay(10000);
    debug_led1_start_flash();
    rt_kprintf("led start flash after 10 seconds.\n");

    rt_thread_mdelay(10000);
    debug_led1_stop_flash();
    rt_kprintf("led stop flash after 10 seconds.\n");

    debug_led1_on();
    rt_kprintf("led on at last.\n");
}
