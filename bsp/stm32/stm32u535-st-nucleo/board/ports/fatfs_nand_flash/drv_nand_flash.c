/*
 * @FilePath: drv_nand_flash.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-14 11:02:58
 * @copyright : Copyright (c) 2024
 */

#include "drv_nand_flash.h"

#define DBG_SECTION_NAME "DRV.NAND_FLASH"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

extern HAL_NAND_Device_t hal_nand_device;

rt_err_t nand_direction_switch(nand_direction_e direction)
{
    rt_pin_mode(QSPI_CPUN_ESP_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(QSPI_CPUN_ESP_PIN, direction);
    rt_err_t res = rt_pin_read(QSPI_CPUN_ESP_PIN) == direction ? RT_EOK : RT_ERROR;
    return res;
}

rt_err_t nand_power_switch(nand_poweron_e poweron)
{
    rt_pin_mode(FLASH_PWRON_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(FLASH_PWRON_PIN, poweron);
    rt_err_t res = rt_pin_read(FLASH_PWRON_PIN) == poweron ? RT_EOK : RT_ERROR;
    return res;
}

void nand_to_stm32(void)
{
    rt_err_t res;
    res = nand_power_switch(NAND_POWERON);
    LOG_D("nand_power_switch NAND_POWERON %s", res == RT_EOK ? "success" : "failed");
    res = nand_direction_switch(NAND_STM32_DIRECTION);
    LOG_D("nand_direction_switch NAND_STM32_DIRECTION %s", res == RT_EOK ? "success" : "failed");
}

void nand_to_esp32(void)
{
    rt_err_t res;
    res = nand_power_switch(NAND_POWERON);
    LOG_D("nand_power_switch NAND_POWERON %s", res == RT_EOK ? "success" : "failed");
    res = nand_direction_switch(NAND_ESP32_DIRECTION);
    LOG_D("nand_direction_switch NAND_ESP32_DIRECTION %s", res == RT_EOK ? "success" : "failed");
}

static void erase_nand_flash(void)
{
    int res;
    uint8_t status;
    for(uint32_t i = 0; i < hal_nand_device.nand_flash_info->memory_info->block_num_per_chip; i++)
    {
        HAL_SPI_NAND_Write_Enable(hal_nand_device);
        HAL_SPI_NAND_Read_Status(hal_nand_device, &status);

        LOG_I("status before erase_nand_flash block %02X", status);
        res = HAL_SPI_NAND_Erase_Block(hal_nand_device, i);
        LOG_D("erase_nand_flash block=%d %s", i, res == 0 ? "success": "failed");

        HAL_SPI_NAND_Wait(hal_nand_device, &status);
        LOG_I("after before erase_nand_flash block %02X", status);
    }
}

int rt_hw_nand_flash_init(void)
{
    int hal_res;
    uint8_t wlock = 0;

    nand_to_stm32();

    HAL_NAND_BSP_Init();

    hal_res = HAL_SPI_NAND_Init(hal_nand_device);
    LOG_D("HAL_SPI_NAND_Init %s.", hal_res == 0 ? "success" : "failed");
    if (hal_res != 0)
    {
        // LOG_E("HAL_SPI_NAND_Init failed.");
        return -RT_ERROR;
    }
    hal_res = HAL_SPI_NAND_Get_Lock_Block(hal_nand_device, &wlock);
    LOG_D("HAL_SPI_NAND_Get_Lock_Block %s.", hal_res == 0 ? "success" : "failed");
    if (hal_res != 0)
    {
        // LOG_E("HAL_SPI_NAND_Get_Lock_Block failed.");
        return -RT_ERROR;
    }
    if (wlock != 0)
    {
        wlock = 0;
        hal_res = HAL_SPI_NAND_Lock_Block(hal_nand_device, &wlock);
        LOG_D("HAL_SPI_NAND_Lock_Block %s.", hal_res == 0 ? "success" : "failed");
        if (hal_res != 0)
        {
            // LOG_E("HAL_SPI_NAND_Lock_Block failed");
            return -RT_ERROR;
        }
        wlock = 0;
        hal_res = HAL_SPI_NAND_Get_Lock_Block(hal_nand_device, &wlock);
        LOG_D("HAL_SPI_NAND_Get_Lock_Block %s. wlock=%d", hal_res == 0 ? "success" : "failed", wlock);
        if (hal_res != 0)
        {
            // LOG_E("HAL_SPI_NAND_Get_Lock_Block failed.");
            return -RT_ERROR;
        }
        if (wlock != 0)
        {
            LOG_E("HAL_SPI_NAND_Lock_Block set failed.");
            return -RT_ERROR;
        }
    }

    // erase_nand_flash();
    return RT_EOK;
}
