/*
 * @FilePath: dhara_blk_device.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-23 14:40:30
 * @copyright : Copyright (c) 2024
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "dhara_blk_device.h"
#include "dhara/map.h"
#include "hal_nand_device_info.h"
#include "hal_spi_nand_driver.h"
#include "drv_nand_flash.h"
#include <math.h>

#define DBG_SECTION_NAME "DHARA_BLK_DEVICE"
#define DBG_LEVEL DBG_INFO
#include <rtdbg.h>

struct dhara_blk_device
{
    struct rt_device parent;
    struct rt_device_blk_geometry geometry;
    struct dhara_map dhara_map;
    struct dhara_nand dhara_nand;
    struct rt_mutex dhara_lock;
    uint8_t *work_buffer;
    uint8_t gc_factor;
};
typedef struct dhara_blk_device *dhara_blk_device_t;

static struct dhara_blk_device dhara_blk_dev = {0};
extern HAL_NAND_Device_t hal_nand_device;
extern int dhara_bind_with_spi_nand_device(struct dhara_nand *n, HAL_NAND_Device_t* spi_nand_device);

static rt_err_t dhara_blk_dev_control(rt_device_t dev, int cmd, void *args)
{
    dhara_blk_device_t part = (dhara_blk_device_t)dev;
    RT_ASSERT(part != RT_NULL);
    rt_err_t res = RT_EOK;
    dhara_error_t err;

    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;

        geometry = (struct rt_device_blk_geometry *)args;
        if (geometry == RT_NULL)
        {
            return -RT_ERROR;
        }

        rt_memcpy(geometry, &part->geometry, sizeof(struct rt_device_blk_geometry));
    }
    else if (cmd == RT_DEVICE_CTRL_BLK_SYNC)
    {
        rt_mutex_take(&part->dhara_lock, RT_WAITING_FOREVER);
        res = dhara_map_sync(&part->dhara_map, &err) == 0 ? RT_EOK : -RT_ERROR;
        rt_mutex_release(&part->dhara_lock);
    }
    else if (cmd == RT_DEVICE_CTRL_BLK_ERASE)
    {
        rt_mutex_take(&part->dhara_lock, RT_WAITING_FOREVER);
        res = dhara_map_trim(&part->dhara_map, *(uint32_t *)args, &err);
        rt_mutex_release(&part->dhara_lock);
    }
    return res;
}

static rt_ssize_t dhara_blk_dev_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    dhara_blk_device_t part = (dhara_blk_device_t)dev;
    RT_ASSERT(part != RT_NULL);
    dhara_error_t err;
    rt_ssize_t i;

    for (i = 0; i < size; i++)
    {
        rt_mutex_take(&part->dhara_lock, RT_WAITING_FOREVER);
        if (dhara_map_read(&part->dhara_map, pos + i, buffer + i * part->geometry.bytes_per_sector, &err))
        {
            i = -1;
        }
        else if (err)
        {
            // This indicates a soft ECC error, we rewrite the sector to recover
            if (dhara_map_write(&part->dhara_map, pos + i, buffer + i * part->geometry.bytes_per_sector, &err))
            {
                i = -1;
            }
        }
        rt_mutex_release(&part->dhara_lock);
        if (i == -1)
        {
            break;
        }
    }
    LOG_D("dhara_blk_dev_read pos=%d, size=%d, ret_size=%d", pos, size, i);
    return i;
}

static rt_ssize_t dhara_blk_dev_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    dhara_blk_device_t part = (dhara_blk_device_t)dev;
    RT_ASSERT(part != RT_NULL);
    dhara_error_t err;
    rt_ssize_t i;

    for (i = 0; i < size; i++)
    {
        rt_mutex_take(&part->dhara_lock, RT_WAITING_FOREVER);
        if (dhara_map_write(&part->dhara_map, pos + i, buffer + i * part->geometry.bytes_per_sector, &err))
        {
            i = -1;
        }
        rt_mutex_release(&part->dhara_lock);
        if (i == -1)
        {
            break;
        }
    }
    LOG_D("dhara_blk_dev_write pos=%d, size=%d, ret_size=%d", pos, size, i);

    return i;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops dhara_blk_dev_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    dhara_blk_dev_read,
    dhara_blk_dev_write,
    dhara_blk_dev_control
};
#endif

rt_err_t dhara_blk_device_init(void)
{
    int res;

    res = rt_mutex_init(&dhara_blk_dev.dhara_lock, "dharalk", RT_IPC_FLAG_PRIO);
    LOG_D("rt_mutex_init dharalk %s.", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        // LOG_E("rt_mutex_init dharalk failed.");
        return res;
    }

    dhara_blk_dev.geometry.block_size = hal_nand_device.nand_flash_info->memory_info->block_size;
    dhara_blk_dev.geometry.bytes_per_sector = hal_nand_device.nand_flash_info->memory_info->page_size;
    dhara_blk_dev.geometry.sector_count = hal_nand_device.nand_flash_info->memory_info->block_num_per_chip * \
                                          hal_nand_device.nand_flash_info->memory_info->page_per_block;

    dhara_blk_dev.dhara_nand.num_blocks = hal_nand_device.nand_flash_info->memory_info->block_num_per_chip;
    dhara_blk_dev.dhara_nand.log2_ppb = (uint8_t)log2((double)hal_nand_device.nand_flash_info->memory_info->page_per_block);  // 64 pages per block is standard
    dhara_blk_dev.dhara_nand.log2_page_size = (uint8_t)log2((double)hal_nand_device.nand_flash_info->memory_info->page_size);  // 4096 bytes per page is fairly standard
    LOG_D(
        "dhara_nand log2_ppb=%d, log2_page_size=%d, num_blocks=%d",
        dhara_blk_dev.dhara_nand.log2_ppb,
        dhara_blk_dev.dhara_nand.log2_page_size,
        dhara_blk_dev.dhara_nand.num_blocks
    );

    dhara_blk_dev.work_buffer = rt_malloc(dhara_blk_dev.geometry.bytes_per_sector);
    dhara_blk_dev.gc_factor = 45;

    res = dhara_bind_with_spi_nand_device(&dhara_blk_dev.dhara_nand, &hal_nand_device);
    LOG_D("dhara_bind_with_spi_nand_device dhara_nand hal_nand_device %s.", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        // LOG_E("dhara_bind_with_spi_nand_device dhara_nand hal_nand_device failed");
        goto _fail_;
    }

    dhara_map_init(&dhara_blk_dev.dhara_map, &dhara_blk_dev.dhara_nand, dhara_blk_dev.work_buffer, dhara_blk_dev.gc_factor);
    LOG_D("dhara_map_init over.");
    LOG_D(
        "dhara_map.journal.nand->log2_ppb=%d, log2_page_size=%d, num_blocks=%d",
        dhara_blk_dev.dhara_map.journal.nand->log2_ppb,
        dhara_blk_dev.dhara_map.journal.nand->log2_page_size,
        dhara_blk_dev.dhara_map.journal.nand->num_blocks
    );

    dhara_error_t ignored;
    res = dhara_map_resume(&dhara_blk_dev.dhara_map, &ignored) == 0 ? RT_EOK : RT_ERROR;
    LOG_D("dhara_map_resume %s. res=%d, ignored=%d", res == RT_EOK ? "success" : "failed", res, ignored);
    LOG_D("dhara_map.count=%d", dhara_blk_dev.dhara_map.count);
    // if (res != RT_EOK)
    // {
    //     // LOG_E("dhara_map_resume failed");
    //     goto _fail_;
    // }

#ifdef RT_USING_DEVICE_OPS
    dhara_blk_dev.parent.ops  = &dhara_blk_dev_ops;
#else
    /* register device */
    dhara_blk_dev.parent.type = RT_Device_Class_Block;
    dhara_blk_dev.parent.init = NULL;
    dhara_blk_dev.parent.open = NULL;
    dhara_blk_dev.parent.close = NULL;
    dhara_blk_dev.parent.read = dhara_blk_dev_read;
    dhara_blk_dev.parent.write = dhara_blk_dev_write;
    dhara_blk_dev.parent.control = dhara_blk_dev_control;
#endif

    dhara_blk_dev.parent.user_data = RT_NULL;
    res = rt_device_register(&dhara_blk_dev.parent, DHARA_BLK_DEV_NAME, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
    LOG_D("rt_device_register dharadev %s.", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        // LOG_E("rt_device_register dharadev failed");
        goto _fail_;
    }

    return res;

_fail_:
    rt_mutex_detach(&dhara_blk_dev.dhara_lock);
    rt_free(dhara_blk_dev.work_buffer);
    return RT_ERROR;
}

void refresh_dhara_map(void)
{
    dhara_error_t ignored;
    int res = dhara_map_resume(&dhara_blk_dev.dhara_map, &ignored) == 0 ? RT_EOK : RT_ERROR;
    LOG_D("dhara_map_resume. res=%d, ignored=%d", res, ignored);
}