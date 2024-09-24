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

#define DRV_DEBUG
#define LOG_TAG     "DRV.NAND_FLASH"
#include <drv_log.h>

#include "hal_nand_device_info.h"
#include "hal_spi_nand_driver.h"

typedef enum {
    NAND_STM32_DIRECTION = 0,
    NAND_ESP32_DIRECTION = 1,
} nand_direction_e;

typedef enum {
    NAND_POWERON = 1,
    NAND_POWEROFF = 0,
} nand_poweron_e;

static inline void nand_direction_switch(nand_direction_e direction)
{
    rt_pin_mode(QSPI_CPUN_ESP_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(QSPI_CPUN_ESP_PIN, direction);
}

static inline void nand_power_switch(nand_poweron_e poweron)
{
    rt_pin_mode(FLASH_PWRON_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(FLASH_PWRON_PIN, poweron);
}

void nand_to_stm32(void)
{
    nand_power_switch(NAND_POWERON);
    nand_direction_switch(NAND_STM32_DIRECTION);
}

void nand_to_esp32(void)
{
    nand_power_switch(NAND_POWERON);
    nand_direction_switch(NAND_ESP32_DIRECTION);
}

int rt_hw_nand_flash_init(void)
{
    int hal_res;
    uint8_t wlock = 0;
    extern HAL_NAND_Device_t hal_nand_device;

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
    return RT_EOK;
}
