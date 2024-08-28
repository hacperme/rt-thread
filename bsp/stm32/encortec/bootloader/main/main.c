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
#include "common.h"
#include <string.h>
#include "nandflash_as5f38.h"

/* Don't use api mapping method to let App call bootloader apis */
// #include "api_map.h"

#define DBG_SECTION_NAME "main"
#define DBG_LEVEL DBG_LOG
// #define DBG_LEVEL DBG_INFO
// #define DBG_LEVEL DBG_WARNING
// #define DBG_LEVEL DBG_ERROR
#include <rtdbg.h>

/* defined the LED2 pin: PA5 */
#define LED2_PIN            GET_PIN(A, 5)
#define FLASH_PWR_CON_PIN   GET_PIN(D, 14)
#define FLASH_SW_PIN        GET_PIN(D, 5)

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

void ReadNANDExample(void) {
    uint8_t readData[PAGE_SIZE] = {0}; // 用于存储读取的数据
    uint32_t address = 0x000000; // NAND Flash 中的起始地址

    // 从 NAND Flash 中读取一页数据
    NAND_ReadPage(address, readData, PAGE_SIZE);

    // 打印读取的数据（或者在调试器中检查 readData 的内容）
    for (int i = 0; i < PAGE_SIZE; i++) {
        rt_kprintf("0x%02X ", readData[i]);
        if ((i + 1) % 16 == 0) {
            rt_kprintf("\r\n");
        }
    }

    // 如果有需要，可以在这里检查 ECC 结果或处理坏块
}

void WriteNANDExample(void) {
    uint8_t writeData[PAGE_SIZE]; // 准备写入的数据
    uint32_t address = 0x000000; // NAND Flash 中的起始地址

    // 填充数据用于写入
    for (int i = 0; i < PAGE_SIZE; i++) {
        writeData[i] = i & 0xFF; // 写入一些递增的值
    }

    // 向 NAND Flash 写入一页数据
    NAND_WritePage(address, writeData, PAGE_SIZE, false);

    // 写入后，可以检查 ECC 结果或处理坏块
}

void BadBlockManagementExample(void) {
    uint32_t blockAddr = 0x000000; // 假设我们检查第一个块

    // 检查块是否为坏块
    if (CheckIfBadBlock(blockAddr)) {
        LOG_I("Block at address 0x%08X is a bad block.\n", blockAddr);
    } else {
        LOG_I("Block at address 0x%08X is a good block.\n", blockAddr);

        // 如果需要，可以标记为坏块
        MarkBlockAsBad(blockAddr);
        LOG_I("Block at address 0x%08X has been marked as bad.\n", blockAddr);
    }
}

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

    /* set LED2 pin mode to output */
    // rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(FLASH_PWR_CON_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(FLASH_PWR_CON_PIN, 1);

    rt_pin_mode(FLASH_SW_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(FLASH_SW_PIN, 0);

    // 初始化 QSPI 外设和 GPIO
    MX_OSPI_Init();

    // 启用 NAND Flash 的 ECC 功能
    NAND_EnableECC();

    // 读取芯片ID
    uint8_t nand_id[2];
    NAND_ReadID(nand_id, 2);
    LOG_I("NAND ID: 0x%02X 0x%02X", nand_id[0], nand_id[1]);

    // 运行示例操作
    ReadNANDExample();     // 读取数据示例
    WriteNANDExample();    // 写入数据示例
    BadBlockManagementExample(); // 坏块管理示例

    /* Don't use api mapping method to let App call bootloader apis */
    // rt_api_map_init();

    if(memcmp(mbr->app_magic_number, MBR_APP_MAGIC_NUMBER, sizeof(mbr->app_magic_number)) == 0) {
        LOG_I("app_magic_number: %s", mbr->app_magic_number);
        LOG_I("app_startup_entry: %p", mbr->app_startup_entry);
        LOG_I("app_main_entry: %p", mbr->app_main_entry);

        /* Don't use api mapping method to let App call bootloader apis */
        app_startup_params_t params = {NULL, NULL, NULL};
        mbr->app_startup_entry(&params);
        mbr->app_main_entry(0, NULL);
    } else {
        LOG_W("No application program.");
    }

    while (1)
    {
        led_toggle();
        LOG_D("hello, world!");
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}
