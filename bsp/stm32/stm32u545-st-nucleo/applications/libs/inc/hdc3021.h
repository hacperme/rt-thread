/*
 * @FilePath: hdc3021.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-14 10:02:44
 * @copyright : Copyright (c) 2024
 */
#ifndef __HDC3021_H__
#define __HDC3021_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "board_pin.h"

#define HDC3021_ADDR 0x44

static rt_uint8_t HDC3021_TRIGGER_ON_DEMAND[2] = {0x24, 0x00};
// static rt_uint8_t HDC3021_AUTO_MEAS_MDOE_1M_2S[2] = {0x20, 0x32};
// static rt_uint8_t HDC3021_AUTO_MEAS_MDOE_1M_1S[2] = {0x31, 0x30};
// static rt_uint8_t HDC3021_AUTO_MEAS_MDOE_2M_1S[2] = {0x22, 0x36};
// static rt_uint8_t HDC3021_AUTO_MEAS_MDOE_4M_1S[2] = {0x23, 0x34};
// static rt_uint8_t HDC3021_AUTO_MEAS_MDOE_10M_1S[2] = {0x27, 0x37};
// static rt_uint8_t HDC3021_AUTO_MEAS_EIXT[2] = {0x30, 0x93};
// static rt_uint8_t HDC3021_AUTO_MEAS_READOUT[2] = {0xE0, 0x00};
// static rt_uint8_t HDC3021_AUTO_MEAS_READ_HIST_MIN_TEMP[2] = {0xE0, 0x02};
// static rt_uint8_t HDC3021_AUTO_MEAS_READ_HIST_MAX_TEMP[2] = {0xE0, 0x03};
// static rt_uint8_t HDC3021_AUTO_MEAS_READ_HIST_MIX_HUMI[2] = {0xE0, 0x04};
// static rt_uint8_t HDC3021_AUTO_MEAS_READ_HIST_MAX_HUMI[2] = {0xE0, 0x05};
static rt_uint8_t HDC3021_SOFT_RESET[2] = {0x30, 0xA2};

rt_err_t hdc3021_crc_check(const rt_uint8_t *input, rt_size_t length, const rt_uint8_t *cmp_val);
rt_err_t hdc3021_soft_reset(struct rt_i2c_bus_device *iic_dev);
rt_err_t hdc3021_trigger_on_demand(struct rt_i2c_bus_device *iic_dev);
rt_err_t hdc3021_read_temp_humi_by_tod(struct rt_i2c_bus_device *iic_dev, float *temp, float *humi);
rt_err_t hdc3021_read_temp_humi(struct rt_i2c_bus_device *iic_dev, float *temp, float *humi);
rt_err_t test_hdc3021(void);

#endif  // __HDC3021_H__
