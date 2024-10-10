/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <rtconfig.h>
#include <board.h>


/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev stm32u5_onchip_flash;
// extern struct rt_mtd_nand_device nand_dev

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                                   \
{                                                                             \
    &stm32u5_onchip_flash,                                                    \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                        \
{                                                                             \
    {0x12345678, "bl",    "onchip_flash", 0,         512*1024, 0},            \
    {0x12345678, "data",  "onchip_flash", 512*1024,  16*1024,  0},            \
    {0x12345678, "app_a", "onchip_flash", 528*1024,  760*1024, 0},            \
    {0x12345678, "app_b", "onchip_flash", 1288*1024, 760*1024, 0},            \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
