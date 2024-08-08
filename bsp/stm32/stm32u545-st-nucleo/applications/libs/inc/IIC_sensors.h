/*
 * @FilePath: IIC_sensors.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-01 11:14:49
 * @copyright : Copyright (c) 2024
 */

#ifndef __IIC_SENSORS_H__
#define __IIC_SENSORS_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "board_pin.h"

#define IIC_SAMPLE_SIZE 10
#define IIC_SAMPLE_PERIOD 1000

typedef struct filter_data
{
    float buf[IIC_SAMPLE_SIZE];
    float average;

    rt_off_t index;
    rt_bool_t is_full;

} filter_data_t;

struct iic_sensor
{
    struct rt_i2c_bus_device *i2c;

    filter_data_t hdc3021_temp_filter;
    filter_data_t hdc3021_humi_filter;
    filter_data_t tmp116_1_temp_filter;
    filter_data_t tmp116_2_temp_filter;
    filter_data_t fdc1004_level_filter;

    rt_thread_t thread;
    rt_uint32_t period; //sample period

    rt_mutex_t lock;
    rt_mutex_t filter_lock;
};
typedef struct iic_sensor *iic_sensor_t;

typedef struct hdc3021_iic
{
    float temperature;
    float humidity;
} hdc3021_iic_t;

iic_sensor_t iic_sensors_init(const char *i2c_bus_name);
void iic_sensors_deinit(iic_sensor_t dev);
static void iic_sensors_filter_entry(void *device);
static rt_err_t read_hw_hdc3021_temperature_humidity(iic_sensor_t dev, hdc3021_iic_t *temp_rh);
static rt_err_t read_hw_tmp116_temperature(iic_sensor_t dev, const rt_uint8_t addr, float *temp);
static rt_err_t read_hw_fdc1004_water_level(iic_sensor_t dev, const rt_uint8_t addr, float *level);
float hdc3021_read_temperature(iic_sensor_t dev);
float hdc3021_read_humidity(iic_sensor_t dev);
float tmp116_1_read_temperature(iic_sensor_t dev);
float tmp116_2_read_temperature(iic_sensor_t dev);
float fdc1004_read_water_level(iic_sensor_t dev);
rt_err_t check_fdc1004_clevel0(iic_sensor_t dev);
rt_uint16_t read_fdc1004_clevel0(void);
static void average_measurement(iic_sensor_t dev, filter_data_t *filter);
static void filter_check_full(filter_data_t *filter);
static void test_iic_sensors(int argc, char **argv);

#endif /* __IIC_SENSORS_H__ */
