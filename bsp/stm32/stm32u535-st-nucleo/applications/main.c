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

#define DBG_SECTION_NAME "main"
#define DBG_LEVEL DBG_LOG
// #define DBG_LEVEL DBG_INFO
// #define DBG_LEVEL DBG_WARNING
// #define DBG_LEVEL DBG_ERROR
#include <rtdbg.h>

#if defined(BSP_USING_ON_CHIP_FLASH) && defined(RT_USING_DFS) && defined(PKG_USING_LITTLEFS)
#include "dfs.h"
#include "drv_dfs.h"
#endif

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

    int mem_total, mem_used, mem_used_max;
    rt_memory_info(&mem_total, &mem_used, &mem_used_max);
    LOG_I(
        "mem_total=%dKB, mem_used=%dKB, mem_used_max=%dKB, mem_free=%dKB",
        mem_total / 1024, mem_used / 1024, mem_used_max / 1024,
        (mem_total - mem_used) / 1024
    );
    int res;

#if defined(BSP_USING_ON_CHIP_FLASH) && defined(RT_USING_DFS) && defined(PKG_USING_LITTLEFS)
    res = rt_hw_fs_mount();
    LOG_I("rt_hw_fs_mount %s", res == RT_EOK ? "success" : "failed");
#endif

    // extern void main_business_entry(void);
    // main_business_entry();

    return RT_EOK;
}
