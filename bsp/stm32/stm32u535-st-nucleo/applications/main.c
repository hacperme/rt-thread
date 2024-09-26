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

#if defined(BSP_USING_ON_CHIP_FLASH) && defined(RT_USING_DFS) && defined(PKG_USING_LITTLEFS)
#include "dfs.h"
#include "drv_dfs.h"
#endif

#if defined(PKG_USING_DFS_YAFFS)
#include "dfs_fs.h"
#include "yaffsfs.h"
#endif

#if defined(BSP_USING_DHARA) && 1
#include "dfs_fs.h"
#include "dhara_blk_device.h"
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

#if defined(PKG_USING_DFS_YAFFS)
    static struct rt_mtd_nand_device *nand_dev;
    nand_dev = (struct rt_mtd_nand_device *)rt_device_find("nand");
    
    yaffs_set_trace(0xFFFF);
    res = yaffs_start_up(nand_dev, "/");
    LOG_I("yaffs_start_up %s", res == RT_EOK ? "success" : "failed");

    // res = dfs_mkfs("yaffs", "nand");
    // LOG_D("dfs_mkfs yaffs nand %s res=%d", res == 0 ? "success" : "failed", res);
    res = dfs_mount("nand", "/", "yaffs", 0, 0);
    if (res != 0)
    {
        rt_err_t err_no = rt_get_errno();
        LOG_E("dfs_mount faield error_code=%d", err_no);
        // res = dfs_mkfs("yaffs", "nand");
        // if (res == 0)
        // {
        //     res = dfs_mount("nand", "/", "yaffs", 0, 0);
        // }
        // else
        // {
        //     LOG_E("dfs_mkfs yaffs nand failed. res=%d", res);
        // }
    }
    LOG_D("dfs_mount nand yaffs %s res=%d", res == 0 ? "success" : "failed", res);

    mem_total = mem_used = mem_used_max = 0;
    rt_memory_info(&mem_total, &mem_used, &mem_used_max);
    LOG_I(
        "mem_total=%dKB, mem_used=%dKB, mem_used_max=%dKB, mem_free=%dKB",
        mem_total / 1024, mem_used / 1024, mem_used_max / 1024,
        (mem_total - mem_used) / 1024
    );
#endif

#if defined(BSP_USING_DHARA) && 1
    // extern rt_err_t dhara_blk_device_init(void);
    rt_tick_t st, et;
    st = rt_tick_get_millisecond();
    res = dhara_blk_device_init();
    LOG_D("dhara_blk_device_init %s", res == 0 ? "success" : "failed");
    if (res == 0)
    {
        res = dfs_mount("dharadev", "/", "elm", 0, 0);
        if (res != 0)
        {
            res = dfs_mkfs("elm", "dharadev");
            LOG_D("dfs_mkfs elm dharadev %s", res == 0 ? "success" : "failed");
            if (res == 0)
            {
                res = dfs_mount("dharadev", "/", "elm", 0, 0);
            }
            // else
            // {
            //     LOG_E("dfs_mkfs elm dharadev failed.");
            // }
        }
        LOG_D("dfs_mount dharadev elm %s", res == 0 ? "success" : "failed");
    }
    et = rt_tick_get_millisecond();
    LOG_I("FATFS init time=%dms", et - st);
#endif

    mem_total = mem_used = mem_used_max = 0;
    rt_memory_info(&mem_total, &mem_used, &mem_used_max);
    LOG_I(
        "mem_total=%dKB, mem_used=%dKB, mem_used_max=%dKB, mem_free=%dKB",
        mem_total / 1024, mem_used / 1024, mem_used_max / 1024,
        (mem_total - mem_used) / 1024
    );

    // extern void main_business_entry(void);
    // main_business_entry();

    return RT_EOK;
}
