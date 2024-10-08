/*
 * @FilePath: test_nand_flash.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-18 10:35:47
 * @copyright : Copyright (c) 2024
 */
#include "rtthread.h"
#include "rtdevice.h"

#define DRV_DEBUG
#define LOG_TAG     "TEST_NAND_FLASH"
#include <drv_log.h>

static struct rt_mtd_nand_device *nand_dev;

void test_nand_flash(void)
{
    rt_err_t res;
    if (!nand_dev)
    {
        nand_dev = (struct rt_mtd_nand_device *)rt_device_find("nand");
    }

    res = rt_mtd_nand_read_id(nand_dev);
    LOG_D("rt_mtd_nand_read_id %s", res == RT_EOK ? "success" : "failed");

    rt_uint32_t block = 0x100;
    res = rt_mtd_nand_erase_block(nand_dev, block);
    LOG_D("rt_mtd_nand_erase_block block=%d %s", block, res == RT_EOK ? "success" : "failed");

    rt_uint32_t page = block * 64;
    rt_uint8_t wbuf[32] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    res = rt_mtd_nand_write(nand_dev, page, wbuf, 26, RT_NULL, 0);
    LOG_D("rt_mtd_nand_write %s", res == RT_EOK ? "success" : "failed");

    rt_uint8_t rbuf[32] = {0};
    res = rt_mtd_nand_read(nand_dev, page, rbuf, 32, RT_NULL, 0);
    LOG_D("rt_mtd_nand_read %s, page=%d, rbuf=%s", res == RT_EOK ? "success" : "failed", page, (char *)rbuf);

    res = rt_mtd_nand_move_page(nand_dev, page, page + 1);
    LOG_D("rt_mtd_nand_move_page %s", res == RT_EOK ? "success" : "failed");

    rt_memset(rbuf, 0xFF, 32);
    res = rt_mtd_nand_read(nand_dev, page + 1, rbuf, 32, RT_NULL, 0);
    LOG_D("rt_mtd_nand_read %s, page=%d, rbuf=%s", res == RT_EOK ? "success" : "failed", page + 1, (char *)rbuf);
}

MSH_CMD_EXPORT(test_nand_flash, test nand flash);
