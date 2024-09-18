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

/* read nand flash id */
static rt_err_t _read_id(struct rt_mtd_nand_device *device)
{
    RT_ASSERT(device != RT_NULL);
    rt_uint8_t dev_id[2] = {0};
    rt_err_t res = RT_ERROR;

    rt_mutex_take(&rt_hw_nand_lock, RT_WAITING_FOREVER);
    if (HAL_SPI_NAND_Read_ID(&hal_nand_device, dev_id) == 0)
    {
        LOG_D("Read MFR ID: 0x%02X  DEV ID: 0x%02X", dev_id[0], dev_id[1]);
        if(hal_nand_device.nand_flash_info->manufacturer_id != dev_id[0] ||\
        hal_nand_device.nand_flash_info->device_id != dev_id[1])
        {
            LOG_E("nand id not match.");
        }
        else
        {
            res = RT_EOK;
        }
    }
    else
    {
        LOG_E("HAL_SPI_NAND_Read_ID failed.");
    }
    rt_mutex_release(&rt_hw_nand_lock);
    return res;
}

static rt_err_t _read_page(struct rt_mtd_nand_device *device,
                           rt_off_t page,
                           rt_uint8_t *data,
                           rt_uint32_t data_len,
                           rt_uint8_t *spare,
                           rt_uint32_t spare_len)
{
    RT_ASSERT(device != RT_NULL);
    if (page < 0 || page >= hal_nand_device.nand_flash_info->memory_info->page_per_chip)
    {
        LOG_E("page must be larger than 0 and smaller than %d", hal_nand_device.nand_flash_info->memory_info->page_per_chip);
        return RT_EINVAL;
    }
    if (data == RT_NULL)
    {
        LOG_E("data buff is NULL");
        return RT_EINVAL;
    }
    if (data_len > hal_nand_device.nand_flash_info->memory_info->page_size)
    {
        LOG_E("data_len is larger than %d", hal_nand_device.nand_flash_info->memory_info->page_size);
        return RT_EINVAL;
    }
    // if (spare == RT_NULL)
    // {
    //     LOG_E("spare buff is NULL");
    //     return RT_EINVAL;
    // }
    // if (spare_len > hal_nand_device.nand_flash_info->memory_info->spare_size)
    // {
    //     LOG_E("spare_len is larger than %d", hal_nand_device.nand_flash_info->memory_info->spare_size);
    // }

    rt_err_t res = RT_ERROR;
    rt_uint8_t status = 0;
    int hal_res;
    rt_uint32_t nand_block = page / (hal_nand_device.nand_flash_info->memory_info->page_per_block);
    rt_uint16_t nand_page = page % (hal_nand_device.nand_flash_info->memory_info->page_per_block);
    rt_uint32_t nand_page_addr = (nand_page << 12) | nand_block;

    rt_mutex_take(&rt_hw_nand_lock, RT_WAITING_FOREVER);
    hal_res = HAL_SPI_NAND_Read_Page_To_Cache(&hal_nand_device, nand_page_addr);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Read_Page_To_Cache failed res=%d nand_page_addr=0x%08X", hal_res, nand_page_addr);
        goto _read_page_exit;
    }
    hal_res = HAL_SPI_NAND_Wait(&hal_nand_device, &status);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Wait not ready res=%d status=%d", hal_res, status);
        goto _read_page_exit;
    }
    hal_res = HAL_SPI_NAND_Read_From_Cache(
        &hal_nand_device, nand_page_addr, 0, data_len, data
    );
    if (hal_res != 0)
    {
        LOG_E(
            "HAL_SPI_NAND_Read_From_Cache failed, res=%d, nand_page_addr=0x%08X, data_len=%d",
            hal_res, nand_page_addr, data_len
        );
        goto _read_page_exit;
    }
    if (spare != RT_NULL && spare_len > 0)
    {
        hal_res = HAL_SPI_NAND_Read_From_Cache(
            &hal_nand_device, nand_page_addr,
            hal_nand_device.nand_flash_info->memory_info->page_size,
            spare_len, spare
        );
        if (hal_res != 0)
        {
            LOG_E(
                "HAL_SPI_NAND_Read_From_Cache failed, res=%d, nand_page_addr=0x%08X, spare_len=%d",
                hal_res, nand_page_addr, spare_len
            );
            goto _read_page_exit;
        }
    }
    res = RT_EOK;

_read_page_exit:
    rt_mutex_release(&rt_hw_nand_lock);
    return res;
}

static rt_err_t _write_page(struct rt_mtd_nand_device *device,
                            rt_off_t page,
                            const rt_uint8_t *data,
                            rt_uint32_t data_len,
                            const rt_uint8_t *spare,
                            rt_uint32_t spare_len)
{
    RT_ASSERT(device != RT_NULL);
    if (page < 0 || page >= hal_nand_device.nand_flash_info->memory_info->page_per_chip)
    {
        LOG_E("page must be larger than 0 and smaller than %d", hal_nand_device.nand_flash_info->memory_info->page_per_chip);
        return RT_EINVAL;
    }
    if (data == RT_NULL)
    {
        LOG_E("data buff is NULL");
        return RT_EINVAL;
    }
    if (data_len > hal_nand_device.nand_flash_info->memory_info->page_size)
    {
        LOG_E("data_len is larger than %d", hal_nand_device.nand_flash_info->memory_info->page_size);
        return RT_EINVAL;
    }
    // if (spare == RT_NULL)
    // {
    //     LOG_E("spare buff is NULL");
    //     return RT_EINVAL;
    // }
    // if (spare_len > hal_nand_device.nand_flash_info->memory_info->spare_size)
    // {
    //     LOG_E("spare_len is larger than %d", hal_nand_device.nand_flash_info->memory_info->spare_size);
    // }

    rt_err_t res = RT_ERROR;
    rt_uint8_t status = 0;
    int hal_res;
    rt_uint32_t nand_block = page / (hal_nand_device.nand_flash_info->memory_info->page_per_block);
    rt_uint16_t nand_page = page % (hal_nand_device.nand_flash_info->memory_info->page_per_block);
    rt_uint32_t nand_page_addr = (nand_page << 12) | nand_block;

    // TODO: Write spare data.
    rt_mutex_take(&rt_hw_nand_lock, RT_WAITING_FOREVER);
    hal_res = HAL_SPI_NAND_Read_Page_To_Cache(&hal_nand_device, nand_page_addr);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Read_Page_To_Cache failed. res=%d, nand_page_addr=0x%08X", hal_res, nand_page_addr);
        goto _write_page_exit;
    }
    hal_res = HAL_SPI_NAND_Wait(&hal_nand_device, &status);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Wait failed. res=%d, status=%d", hal_res, status);
        goto _write_page_exit;
    }
    hal_res = HAL_SPI_NAND_Write_Enable(&hal_nand_device);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Write_Enable failed. res=%d", hal_res);
        goto _write_page_exit;
    }
    status = 0;
    hal_res = HAL_SPI_NAND_Read_Status(&hal_nand_device, &status);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Read_Status status failed. res=%d, status=%d", hal_res, status);
        goto _write_page_exit;
    }
    if (status & STATUS_WEL == 0)
    {
        LOG_E("STATUS_WEL is not ready. status=%d", status);
        goto _write_page_exit;
    }
    hal_res = HAL_SPI_NAND_Program_Data_To_Cache(
        &hal_nand_device, nand_page_addr, 0,
        data_len, (rt_uint8_t *)data, 0
    );
    if (hal_res != 0)
    {
        LOG_E(
            "HAL_SPI_NAND_Program_Data_To_Cache failed. res=%d, nand_page_addr=0x%08X, data_len=%d",
            hal_res, nand_page_addr, data_len
        );
        goto _write_page_exit;
    }
    if (spare != RT_NULL && spare_len > 0)
    {
        hal_res = HAL_SPI_NAND_Program_Data_To_Cache(
            &hal_nand_device, nand_page_addr,
            hal_nand_device.nand_flash_info->memory_info->page_size,
            spare_len, (rt_uint8_t *)spare, 0
        );
        if (hal_res != 0)
        {
            LOG_E(
                "HAL_SPI_NAND_Program_Data_To_Cache failed. res=%d, nand_page_addr=0x%08X, spare_len=%d",
                hal_res, nand_page_addr, spare_len
            );
            goto _write_page_exit;
        }
    }
    status = 0;
    hal_res = HAL_SPI_NAND_Wait(&hal_nand_device, &status);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Wait failed. res=%d, status=%d", hal_res, status);
        goto _write_page_exit;
    }
    hal_res = HAL_SPI_NAND_Program_Execute(&hal_nand_device, nand_page_addr);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Program_Execute failed. res=%d, nand_page_addr=0x%08X", hal_res, nand_page_addr);
        goto _write_page_exit;
    }
    status = 0;
    hal_res = HAL_SPI_NAND_Wait(&hal_nand_device, &status);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Wait failed. res=%d, status=%d", hal_res, status);
        goto _write_page_exit;
    }
    res = RT_EOK;

_write_page_exit:
    rt_mutex_release(&rt_hw_nand_lock);
    return res;
}

static rt_err_t _page_copy(struct rt_mtd_nand_device *device,
                           rt_off_t src_page,
                           rt_off_t dst_page)
{
    RT_ASSERT(device != RT_NULL);
    if (src_page < 0 || src_page >= hal_nand_device.nand_flash_info->memory_info->page_per_chip)
    {
        LOG_E("src_page must be larger than 0 and smaller than %d", hal_nand_device.nand_flash_info->memory_info->page_per_chip);
        return RT_EINVAL;
    }
    if (dst_page < 0 || dst_page >= hal_nand_device.nand_flash_info->memory_info->page_per_chip)
    {
        LOG_E("dst_page must be larger than 0 and smaller than %d", hal_nand_device.nand_flash_info->memory_info->page_per_chip);
        return RT_EINVAL;
    }

    rt_err_t res = RT_ERROR;
    int hal_res;
    rt_uint32_t nand_src_block = src_page / (hal_nand_device.nand_flash_info->memory_info->page_per_block);
    rt_uint16_t nand_src_page = src_page % (hal_nand_device.nand_flash_info->memory_info->page_per_block);
    rt_uint32_t nand_src_page_addr = (nand_src_page << 12) | nand_src_block;
    rt_uint32_t nand_dst_block = dst_page / (hal_nand_device.nand_flash_info->memory_info->page_per_block);
    rt_uint16_t nand_dst_page = dst_page % (hal_nand_device.nand_flash_info->memory_info->page_per_block);
    rt_uint32_t nand_dst_page_addr = (nand_dst_page << 12) | nand_dst_block;

    rt_mutex_take(&rt_hw_nand_lock, RT_WAITING_FOREVER);
    hal_res = HAL_SPI_NAND_Internal_Data_Move(&hal_nand_device, nand_src_page_addr, nand_dst_page_addr, 0, RT_NULL, 0);
    if (hal_res == 0)
    {
        res = RT_EOK;
    }
    else
    {
        LOG_E(
            "HAL_SPI_NAND_Internal_Data_Move failed. res=%d, src_page_addr=0x%08X dst_page_addr=0x%08X",
            hal_res, nand_src_page_addr, nand_dst_page_addr
        );
    }
    rt_mutex_release(&rt_hw_nand_lock);
    return RT_EOK;
}

/* erase one block */
static rt_err_t _erase_block(struct rt_mtd_nand_device *device, rt_uint32_t block)
{
    RT_ASSERT(device != RT_NULL);
    if (block >= hal_nand_device.nand_flash_info->memory_info->block_num_per_chip)
    {
        LOG_E("block number must be smaller than %d", hal_nand_device.nand_flash_info->memory_info->block_num_per_chip);
        return RT_EINVAL;
    }

    rt_err_t res = RT_ERROR;
    rt_uint8_t status = 0;
    int hal_res;

    rt_mutex_take(&rt_hw_nand_lock, RT_WAITING_FOREVER);
    hal_res = HAL_SPI_NAND_Write_Enable(&hal_nand_device);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Write_Enable failed. res=%d", hal_res);
        goto _erase_block_exit;
    }
    hal_res = HAL_SPI_NAND_Read_Status(&hal_nand_device, &status);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Read_Status failed. res=%d, status=%d", hal_res, status);
        goto _erase_block_exit;
    }
    hal_res = HAL_SPI_NAND_Erase_Block(&hal_nand_device, block);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Erase_Block failed. res=%d, block=%d", hal_res, block);
        goto _erase_block_exit;
    }
    status = 0;
    hal_res = HAL_SPI_NAND_Wait(&hal_nand_device, &status);
    if (hal_res != 0)
    {
        LOG_E("HAL_SPI_NAND_Wait failed. hal_res=%d, status=%d", hal_res, status);
        goto _erase_block_exit;
    }
    if (status & STATUS_E_FAIL == 0)
    {
        res = RT_EOK;
    }
    else
    {
        LOG_E("Erase failed. status=%d", status);
    }

_erase_block_exit:
    rt_mutex_release(&rt_hw_nand_lock);
    return res;
}

static rt_err_t _check_block(struct rt_mtd_nand_device *device, rt_uint32_t block)
{
    RT_ASSERT(device != RT_NULL);
    return (RT_MTD_EOK);
}

static rt_err_t _mark_bad(struct rt_mtd_nand_device *device, rt_uint32_t block)
{
    RT_ASSERT(device != RT_NULL);
    return (RT_MTD_EOK);
}

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

    bool res = HAL_SPI_NAND_Init(&hal_nand_device);
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

INIT_DEVICE_EXPORT(rt_hw_nand_init);

#endif