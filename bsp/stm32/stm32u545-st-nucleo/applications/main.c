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

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* defined the LED2 pin: PA5 */
#define LED2_PIN    GET_PIN(A, 5)

void led_toggle(void) {
    rt_pin_write(LED2_PIN, !rt_pin_read(LED2_PIN));
}

extern char __bootloader_rom_start[];
extern char __bootloader_rom_end[];
extern char __bootloader_rom_occupied_end[];
extern char __app_rom_start[];
extern char __app_rom_end[];

extern char __bootloader_ram_start[];
extern char __bootloader_ram_end[];
extern char __bootloader_ram_occupied_end[];
extern char __app_ram_start[];
extern char __app_ram_end[];

extern char __heap_start[];
extern char __heap_end[];

int main(void)
{
    int count = 1;
    /* set LED0 pin mode to output */
    rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);

    LOG_E("__bootloader_rom_start: %p\r\n", __bootloader_rom_start);
    LOG_E("__bootloader_rom_end: %p\r\n", __bootloader_rom_end);
    LOG_E("__bootloader_rom_occupied_end: %p\r\n", __bootloader_rom_occupied_end);
    LOG_E("__app_rom_start: %p\r\n", __app_rom_start);
    LOG_E("__app_rom_end: %p\r\n", __app_rom_end);

    LOG_E("__bootloader_ram_start: %p\r\n", __bootloader_ram_start);
    LOG_E("__bootloader_ram_end: %p\r\n", __bootloader_ram_end);
    LOG_E("__bootloader_ram_occupied_end: %p\r\n", __bootloader_ram_occupied_end);
    LOG_E("__app_ram_start: %p\r\n", __app_ram_start);
    LOG_E("__app_ram_end: %p\r\n", __app_ram_end);

    LOG_E("__heap_start: %p\r\n", __heap_start);
    LOG_E("__heap_end: %p\r\n", __heap_end);

    while (count++)
    {
        led_toggle();
        LOG_D("hello, world!\r\n");
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}
