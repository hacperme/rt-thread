/*
 * @FilePath: on_chip_flash.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-12 15:16:14
 * @copyright : Copyright (c) 2024
 */
#ifndef __ON_CHIP_FLASH_H__
#define __ON_CHIP_FLASH_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "fal.h"

#define DBG_SECTION_NAME "ON_CHIP_FLASH"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

rt_err_t find_app_partition(void);
rt_err_t read_app_partition(rt_uint32_t addr, rt_uint8_t *buf, rt_int32_t size);
rt_err_t write_boolload_partition(rt_uint32_t addr, rt_uint8_t *buf, rt_int32_t size);
rt_err_t test_on_chip_flash(void);

#endif  // __ON_CHIP_FLASH_H__
