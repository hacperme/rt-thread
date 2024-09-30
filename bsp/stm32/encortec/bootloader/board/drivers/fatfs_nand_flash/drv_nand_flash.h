/*
 * @FilePath: drv_nand_flash.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-25 14:35:35
 * @copyright : Copyright (c) 2024
 */
#ifndef __DRV_NAND_FLASH_H__
#define __DRV_NAND_FLASH_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
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

rt_err_t nand_direction_switch(nand_direction_e direction);
rt_err_t nand_power_switch(nand_poweron_e poweron);
void nand_to_stm32(void);
void nand_to_esp32(void);
int rt_hw_nand_flash_init(void);

#endif