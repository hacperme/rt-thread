/*
 * @FilePath: hdc3021.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-31 18:54:08
 * @copyright : Copyright (c) 2024
 */

#ifndef __HDC3021_H__
#define __HDC3021_H__

#include "rtthread.h"
#include "rtdevice.h"

#define HDC3021_AVERAGE_TIMES 10
#define HDC3021_SAMPLE_PERIOD 300

typedef struct filter_data
{
    float buf[HDC3021_AVERAGE_TIMES];
    float average;

    rt_off_t index;
    rt_bool_t is_full;

} filter_data_t;

struct hdc3021_device
{
    struct rt_i2c_bus_device *i2c;

    filter_data_t temp_filter;
    filter_data_t humi_filter;

    rt_thread_t thread;
    rt_uint32_t period; //sample period

    rt_mutex_t lock;
};
typedef struct hdc3021_device *hdc3021_device_t;

typedef struct hdc3021_temp_rh
{
    float temperature;
    float relative_humidity;
} hdc3021_temp_rh_t;


hdc3021_device_t hdc3021_init(const char *i2c_bus_name);

void hdc3021_deinit(hdc3021_device_t dev);

float hdc3021_read_temperature(hdc3021_device_t dev);

float hdc3021_read_humidity(hdc3021_device_t dev);

#endif /* __HDC3021_H__ */
