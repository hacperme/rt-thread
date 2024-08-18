/*
 * @FilePath: voltage.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-30 16:27:10
 * @copyright : Copyright (c) 2024
 */
#ifndef __VOLTAGE_H__
#define __VOLTAGE_H__

#include "rtthread.h"
#include "rtdevice.h"

static rt_err_t adc_vol_read(rt_int8_t channel, rt_uint16_t *value);
rt_err_t cur_vol_read(rt_uint16_t *value);
rt_err_t vcat_vol_read(rt_uint16_t *value);
rt_err_t vbat_vol_read(rt_uint16_t *value);
rt_err_t vrefint_vol_read(rt_uint16_t *value);
static void test_read_voltage(int argc, char *argv[]);

#endif  // __VOLTAGE_H__