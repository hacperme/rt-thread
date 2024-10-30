/*
 * @FilePath: stm32u575_ota.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-15 15:09:34
 * @copyright : Copyright (c) 2024
 */
#ifndef __STM32U575_OTA_H__
#define __STM32U575_OTA_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "ota_app.h"

rt_err_t set_stm32u575_ota_option(ota_tag_e ota_tag, char *ota_file_name);
rt_err_t clear_stm32u575_ota_option(void);

#endif