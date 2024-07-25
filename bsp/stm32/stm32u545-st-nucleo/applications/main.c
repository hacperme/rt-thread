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

/* defined the LED0 pin: PC7 */
#define LED0_PIN    GET_PIN(A, 5)

int main(void)
{
    int count = 1;
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

    while (count++)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        LOG_D("hello, world!\r\n");
        rt_thread_mdelay(1500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        LOG_D("hello, world!\r\n");
        rt_thread_mdelay(1500);
    }

    return RT_EOK;
}
