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
// #include "on_chip_flash.h"
// #include "board_pin.h"
// #include "lpm.h"

#define DBG_SECTION_NAME "main"
#define DBG_LEVEL DBG_LOG
// #define DBG_LEVEL DBG_INFO
// #define DBG_LEVEL DBG_WARNING
// #define DBG_LEVEL DBG_ERROR
#include <rtdbg.h>
#include "dfs.h"
#include "drv_dfs.h"

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
    LOG_I("__bootloader_rom_start: %p\r\n", __bootloader_rom_start);
    LOG_I("__bootloader_rom_end: %p\r\n", __bootloader_rom_end);
    LOG_I("__bootloader_rom_occupied_end: %p\r\n", __bootloader_rom_occupied_end);
    LOG_I("__app_rom_start: %p\r\n", __app_rom_start);
    LOG_I("__app_rom_end: %p\r\n", __app_rom_end);

    LOG_I("__bootloader_ram_start: %p\r\n", __bootloader_ram_start);
    LOG_I("__bootloader_ram_end: %p\r\n", __bootloader_ram_end);
    LOG_I("__bootloader_ram_occupied_end: %p\r\n", __bootloader_ram_occupied_end);
    LOG_I("__app_ram_start: %p\r\n", __app_ram_start);
    LOG_I("__app_ram_end: %p\r\n", __app_ram_end);

    LOG_I("__heap_start: %p\r\n", __heap_start);
    LOG_I("__heap_end: %p\r\n", __heap_end);

    int res = rt_hw_fs_mount();
    LOG_I("rt_hw_fs_mount %s", res == RT_EOK ? "success" : "failed");

    // extern void main_business_entry(void);
    // main_business_entry();

    return RT_EOK;
}
