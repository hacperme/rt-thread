/*
 * @FilePath: hdc3021.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-31 18:53:53
 * @copyright : Copyright (c) 2024
 */

#include "hdc3021.h"

#define DBG_TAG "HDC3021"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define HDC3021_ADDR 0x44
static rt_uint8_t CMD_SEQ_TRIGGER_ON_DEMAND[] = {0x24, 0x00};
static rt_uint8_t CMD_SEQ_SOFT_RESET[] = {0x30, 0xA2};

static void hdc3021_filter_entry(void *device);
static rt_err_t read_hw_temperature_humidity(hdc3021_device_t dev, hdc3021_temp_rh_t *temp_rh);
static void average_measurement(hdc3021_device_t dev, filter_data_t *filter);
static void hdc3021_start(int argc, char **argv);

hdc3021_device_t hdc3021_init(const char *i2c_bus_name)
{
    hdc3021_device_t dev;

    RT_ASSERT(i2c_bus_name);

    dev = rt_calloc(1, sizeof(struct hdc3021_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for hdc3021 device on '%s' ", i2c_bus_name);
        return RT_NULL;
    }

    dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
    if (dev->i2c == RT_NULL)
    {
        LOG_E("Can't find hdc3021 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->lock = rt_mutex_create("TRHLK1", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for hdc3021 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }
    dev->period = HDC3021_SAMPLE_PERIOD;

    dev->thread = rt_thread_create("hdc3021", hdc3021_filter_entry, (void *)dev, 1024, 15, 10);
    if (dev->thread != RT_NULL)
    {
        rt_thread_startup(dev->thread);
    }
    else
    {
        LOG_E("Can't start filtering function for hdc3021 device on '%s' ", i2c_bus_name);
        rt_mutex_delete(dev->lock);
        rt_free(dev);
    }
    return dev;
}

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void hdc3021_deinit(hdc3021_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);

    rt_thread_delete(dev->thread);

    rt_free(dev);
}

static void hdc3021_filter_entry(void *device)
{
    RT_ASSERT(device);

    hdc3021_device_t dev = (hdc3021_device_t)device;
    hdc3021_temp_rh_t temp_rh = {0};

    while (1)
    {
        if (dev->temp_filter.index >= HDC3021_AVERAGE_TIMES)
        {
            if (dev->temp_filter.is_full != RT_TRUE)
            {
                dev->temp_filter.is_full = RT_TRUE;
            }

            dev->temp_filter.index = 0;
        }
        if (dev->humi_filter.index >= HDC3021_AVERAGE_TIMES)
        {
            if (dev->humi_filter.is_full != RT_TRUE)
            {
                dev->humi_filter.is_full = RT_TRUE;
            }

            dev->humi_filter.index = 0;
        }

        if (read_hw_temperature_humidity(dev, &temp_rh) == RT_EOK)
        {
            dev->temp_filter.buf[dev->temp_filter.index] = temp_rh.temperature;
            dev->humi_filter.buf[dev->humi_filter.index] = temp_rh.relative_humidity;
        }

        rt_thread_delay(rt_tick_from_millisecond(dev->period));

        dev->temp_filter.index++;
        dev->humi_filter.index++;
    }
}

static rt_err_t read_hw_temperature_humidity(hdc3021_device_t dev, hdc3021_temp_rh_t *temp_rh)
{
    rt_uint8_t data[6] = {0};
    if (rt_i2c_master_send(dev->i2c, HDC3021_ADDR, RT_I2C_WR, CMD_SEQ_TRIGGER_ON_DEMAND, 2) == 2)
    {
        if (rt_i2c_master_recv(dev->i2c, HDC3021_ADDR, RT_I2C_RD, data, 6) == 6)
        {
            // LOG_D("DATA: %02X, %02X, %02X, %02X, %02X, %02X", data[0], data[1], data[2], data[3], data[4], data[5]);
            temp_rh->temperature = (float)((data[0] << 8) | data[1]) / 65536.0 * 175.0 - 45.0;
            temp_rh->relative_humidity = (float)((data[3] << 8) | data[4]) / 65536.0 * 100.0;
            return RT_EOK;
        }
        else
        {
            LOG_E("rt_i2c_master_recv failed.");
        }
    }
    else{
        LOG_E("rt_i2c_master_send failed.");
    }
    return RT_ERROR;
}

float hdc3021_read_temperature(hdc3021_device_t dev)
{
    average_measurement(dev, &dev->temp_filter);
    return dev->temp_filter.average;
}

float hdc3021_read_humidity(hdc3021_device_t dev)
{
    average_measurement(dev, &dev->humi_filter);
    return dev->humi_filter.average;
}


static void average_measurement(hdc3021_device_t dev, filter_data_t *filter)
{
    rt_uint32_t i;
    float sum = 0;
    rt_uint32_t temp;
    rt_err_t result;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        if (filter->is_full)
        {
            temp = HDC3021_AVERAGE_TIMES;
        }
        else
        {
            temp = filter->index + 1;
        }

        for (i = 0; i < temp; i++)
        {
            sum += filter->buf[i];
        }
        filter->average = sum / temp;
    }
    else
    {
        LOG_E("The software failed to average at this time. Please try again");
    }
    rt_mutex_release(dev->lock);
}

static void hdc3021_start(int argc, char **argv)
{
    hdc3021_device_t dev = hdc3021_init("i2c1");
    rt_uint8_t cnt = 0;
    while (cnt < 10)
    {
        float temp, humi;
        temp = hdc3021_read_temperature(dev);
        humi = hdc3021_read_humidity(dev);
        LOG_D("temp %lf, humi %lf\r\n", temp, humi);
        rt_thread_delay(rt_tick_from_millisecond(1000));
        cnt++;
    }
    hdc3021_deinit(dev);
}

MSH_CMD_EXPORT(hdc3021_start, hdc3021 start);
