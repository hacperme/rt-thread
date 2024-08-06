/*
 * @FilePath: adxl372.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-05 19:11:03
 * @copyright : Copyright (c) 2024
 */
#ifndef __ADXL372_H__
#define __ADXL372_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"

#define DBG_SECTION_NAME "ADXL372"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

// void g_sensor_callback(void);
void g_sensor_wakeup_irq_enable(void);
rt_err_t rt_hw_spi_adxl372_init(void);
rt_err_t adxl372_init(void);
static void test_adxl372(int argc, char **argv);

#endif  // __ADXL372_H__
