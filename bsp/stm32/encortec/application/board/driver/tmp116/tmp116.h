/*
 * @FilePath: tmp116.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-14 14:56:01
 * @copyright : Copyright (c) 2024
 */
#ifndef __TMP116_H__
#define __TMP116_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"

#define TMP116_1_ADDR 0x48
#define TMP116_2_ADDR 0x49

static rt_uint8_t TMP116_TEMP_REG = 0x00;
static rt_uint8_t TMP116_CFGR_REG = 0x01;
static rt_uint8_t TMP116_DEVICE_ID_REG = 0x0F;

rt_err_t tmp116_read_device_id(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, rt_uint16_t *dev_id);
rt_err_t tmp116_set_configuration(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, rt_uint8_t *cfg, rt_uint8_t size);
rt_err_t tmp116_read_configuration(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, rt_uint16_t *cfg);
rt_err_t tmp116_data_ready(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, rt_uint8_t *ready);
rt_err_t tmp116_measure_temperature(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, float *temp);
rt_err_t temp116_read_temperture(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, float *temp);

#endif  // __TMP116_H__
