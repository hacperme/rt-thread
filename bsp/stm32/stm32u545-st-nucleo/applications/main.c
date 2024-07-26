/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <rtdbg.h>

/* defined the LED2 pin: PA5 */
#define LED2_PIN    GET_PIN(A, 5)

void led_toggle(void) {
    rt_pin_write(LED2_PIN, !rt_pin_read(LED2_PIN));
}

int main(void)
{
    int count = 1;
    /* set LED0 pin mode to output */
    rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);

    while (count++)
    {
        led_toggle();
        LOG_E("hello, world!\r\n");
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}
