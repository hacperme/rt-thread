/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "common.h"

/* Don't use api mapping method to let App call bootloader apis */
// #include "api_map.h"

#define DBG_SECTION_NAME "main"
#define DBG_LEVEL DBG_LOG
// #define DBG_LEVEL DBG_INFO
// #define DBG_LEVEL DBG_WARNING
// #define DBG_LEVEL DBG_ERROR
#include <rtdbg.h>

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
    mbr_t *mbr = (mbr_t *)__app_rom_start;

    LOG_I("__bootloader_rom_start: %p", __bootloader_rom_start);
    LOG_I("__bootloader_rom_end: %p", __bootloader_rom_end);
    LOG_I("__bootloader_rom_occupied_end: %p", __bootloader_rom_occupied_end);
    LOG_I("__app_rom_start: %p", __app_rom_start);
    LOG_I("__app_rom_end: %p", __app_rom_end);

    LOG_I("__bootloader_ram_start: %p", __bootloader_ram_start);
    LOG_I("__bootloader_ram_end: %p", __bootloader_ram_end);
    LOG_I("__bootloader_ram_occupied_end: %p", __bootloader_ram_occupied_end);
    LOG_I("__app_ram_start: %p", __app_ram_start);
    LOG_I("__app_ram_end: %p", __app_ram_end);

    LOG_I("__heap_start: %p", __heap_start);
    LOG_I("__heap_end: %p", __heap_end);

    return RT_EOK;
}
