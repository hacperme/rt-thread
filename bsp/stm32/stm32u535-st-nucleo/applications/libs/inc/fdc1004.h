/*
 * @FilePath: fdc1004.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-10 11:14:45
 * @copyright : Copyright (c) 2024
 */

#ifndef __FDC1004_H__
#define __FDC1004_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"

#define FDC1004_ADDR 0x50

static const rt_uint8_t FDC1004_MEAS1_MSB_REG = 0x00;
static const rt_uint8_t FDC1004_MEAS1_LSB_REG = 0x01;
static const rt_uint8_t FDC1004_MEAS2_MSB_REG = 0x02;
static const rt_uint8_t FDC1004_MEAS2_LSB_REG = 0x03;
static const rt_uint8_t FDC1004_MEAS3_MSB_REG = 0x04;
static const rt_uint8_t FDC1004_MEAS3_LSB_REG = 0x05;
static const rt_uint8_t FDC1004_MEAS4_MSB_REG = 0x06;
static const rt_uint8_t FDC1004_MEAS4_LSB_REG = 0x07;
static const rt_uint8_t FDC1004_CONF_MEAS1_REG = 0x08;
static const rt_uint8_t FDC1004_CONF_MEAS2_REG = 0x09;
static const rt_uint8_t FDC1004_CONF_MEAS3_REG = 0x0A;
static const rt_uint8_t FDC1004_CONF_MEAS4_REG = 0x0B;
static const rt_uint8_t FDC1004_FDC_CONF_REG = 0x0C;
static const rt_uint8_t FDC1004_OFFSET_CAL_CIN1_REG = 0x0D;
static const rt_uint8_t FDC1004_OFFSET_CAL_CIN2_REG = 0x0E;
static const rt_uint8_t FDC1004_OFFSET_CAL_CIN3_REG = 0x0F;
static const rt_uint8_t FDC1004_OFFSET_CAL_CIN4_REG = 0x10;
static const rt_uint8_t FDC1004_GAIN_CAL_CIN1_REG = 0x11;
static const rt_uint8_t FDC1004_GAIN_CAL_CIN2_REG = 0x12;
static const rt_uint8_t FDC1004_GAIN_CAL_CIN3_REG = 0x13;
static const rt_uint8_t FDC1004_GAIN_CAL_CIN4_REG = 0x14;
static const rt_uint8_t FDC1004_Manufacturer_REG = 0xFE;
static const rt_uint8_t FDC1004_Device_REG = 0xFF;

static rt_uint8_t FDC1004_MEAS_CFG[4][3] = {
	{FDC1004_CONF_MEAS1_REG, 0x1C, 0x00},
	{FDC1004_CONF_MEAS2_REG, 0x3C, 0x00},
	{FDC1004_CONF_MEAS3_REG, 0x5C, 0x00},
	{FDC1004_CONF_MEAS4_REG, 0x7C, 0x00},
};
static rt_uint8_t FDC1004_MEAS_TRG[4][2] = {
    {0x0D, 0x80},  // CIN1
    {0x0D, 0x40},  // CIN2
    {0x0D, 0x20},  // CIN3
    {0x0D, 0x10},  // CIN4
};
static rt_uint8_t FDC1004_MEAS_READ_REGS[4][2] = {
    {FDC1004_MEAS1_MSB_REG, FDC1004_MEAS1_LSB_REG},
    {FDC1004_MEAS2_MSB_REG, FDC1004_MEAS2_LSB_REG},
    {FDC1004_MEAS3_MSB_REG, FDC1004_MEAS3_LSB_REG},
    {FDC1004_MEAS4_MSB_REG, FDC1004_MEAS4_LSB_REG},
};

/* TODO: Read clevel0 from flash. */
static rt_int32_t FDC1004_CLEVEL0 = 0xFC7E2700;

rt_err_t fdc1004_read_manufacturer_id(struct rt_i2c_bus_device *iic_dev, rt_uint16_t *manufacturer_id);
rt_err_t dfc1004_read_device_id(struct rt_i2c_bus_device *iic_dev, rt_uint16_t *dev_id);
rt_err_t fdc1004_meas_config(struct rt_i2c_bus_device *iic_dev, rt_uint8_t reg, rt_uint8_t val_h, rt_uint8_t val_l);
rt_err_t fdc1004_meas_trigger(struct rt_i2c_bus_device *iic_dev, rt_uint8_t val_h, rt_uint8_t val_l);
rt_err_t fdc1004_meas_done(struct rt_i2c_bus_device *iic_dev, rt_uint8_t meas_id, rt_uint32_t timeout, rt_uint8_t *done);
rt_err_t fdc1004_meas_read(struct rt_i2c_bus_device *iic_dev, rt_uint8_t reg, rt_uint16_t *value);
rt_err_t fdc1004_meas_data(struct rt_i2c_bus_device *iic_dev, float *value);
rt_err_t fdc1004_check_clevel0(struct rt_i2c_bus_device *iic_dev);

#endif /* __FDC1004_H__ */
