/*
 * @FilePath: drv_nand_flash.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-14 11:02:58
 * @copyright : Copyright (c) 2024
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define BSP_USING_NAND_FLASH
#ifdef BSP_USING_NAND_FLASH

#define DRV_DEBUG
#define LOG_TAG     "DRV.NAND_FLASH"
#include <drv_log.h>
#include "hal_nand_device_info.h"
#include "hal_spi_nand_driver.h"
#include "drv_nand_flash.h"

struct rt_mutex rt_hw_nand_lock = {0};

static const struct rt_mtd_nand_driver_ops ops =
    {
        _read_id,
        _read_page,
        _write_page,
        _page_copy,
        _erase_block,
        _check_block,
        _mark_bad,
};
static struct rt_mtd_nand_device nand_dev;

int rt_hw_nand_init(void)
{
    rt_err_t result = RT_EOK;

    bool res = HAL_SPI_NAND_Init(hal_nand_device);
    if (!res)
    {
        LOG_E("HAL_SPI_NAND_Init failed.");
        return -RT_ERROR;
    }

    rt_mutex_init(&rt_hw_nand_lock, "nand", RT_IPC_FLAG_PRIO);

    nand_dev.page_size       = 4096;
    nand_dev.pages_per_block = 64;
    nand_dev.plane_num       = 1;
    nand_dev.oob_size        = 256;
    nand_dev.oob_free        = 256;
    nand_dev.block_start     = 0;
    nand_dev.block_end       = 4095;

    nand_dev.block_total     = 4096;
    nand_dev.ops = &ops;

    result = rt_mtd_nand_register_device("nand", &nand_dev);
    if (result != RT_EOK)
    {
        rt_device_unregister(&nand_dev.parent);
        rt_mutex_detach(&rt_hw_nand_lock);
        return -RT_ERROR;
    }

    LOG_I("nand flash init success, id: 0x%02x\n", hal_nand_device.nand_flash_info->device_id);

    return RT_EOK;
}

// INIT_DEVICE_EXPORT(rt_hw_nand_init);

#endif