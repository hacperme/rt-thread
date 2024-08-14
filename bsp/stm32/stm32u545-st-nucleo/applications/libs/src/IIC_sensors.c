/*
 * @FilePath: IIC_sensors.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-01 11:15:14
 * @copyright : Copyright (c) 2024
 */
#include "IIC_sensors.h"

#define DBG_TAG "IIC_SENSORS"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_uint8_t TPH_THD_EXIT = 0;
static float ERROR_SENSOR_VALUE = -999.0;

iic_sensor_t iic_sensors_init(const char *i2c_bus_name)
{
    iic_sensor_t dev;

    RT_ASSERT(i2c_bus_name);

    dev = rt_calloc(1, sizeof(struct iic_sensor));
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

    dev->lock = rt_mutex_create("THSLK", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for temp humi sensor device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }
    dev->filter_lock = rt_mutex_create("THSFLK", RT_IPC_FLAG_FIFO);
    if (dev->filter_lock == RT_NULL)
    {
        LOG_E("Can't create mutex for temp humi sensor device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }
    dev->period = IIC_SAMPLE_PERIOD;

    dev->thread = rt_thread_create("tmp_humi_sensor", iic_sensors_filter_entry, (void *)dev, 1024, 15, 10);
    if (dev->thread != RT_NULL)
    {
        TPH_THD_EXIT = 0;
        rt_err_t result = rt_mutex_take(dev->filter_lock, RT_WAITING_FOREVER);
        LOG_D("iic_sensors_filter_entry filer lock take %s", result == RT_EOK ? "success" : "failed");
        rt_thread_startup(dev->thread);
    }
    else
    {
        LOG_E("Can't start filtering function for tmp_humi_sensor device on '%s' ", i2c_bus_name);
        rt_mutex_delete(dev->lock);
        rt_mutex_delete(dev->filter_lock);
        rt_free(dev);
    }
    return dev;
}

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void iic_sensors_deinit(iic_sensor_t dev)
{
    RT_ASSERT(dev);

    TPH_THD_EXIT = 1;
    rt_thread_delay(rt_tick_from_millisecond(dev->period + 100));

    rt_mutex_delete(dev->lock);

    rt_free(dev);
}

static void iic_sensors_filter_entry(void *device)
{
    RT_ASSERT(device);

    iic_sensor_t dev = (iic_sensor_t)device;
    hdc3021_iic_t hdc3021_th = {0};
    float tmp116_1_temp = 0;
    float tmp116_2_temp = 0;
    float water_level = 0;
    rt_err_t result;
    rt_uint8_t first_cycle = 0;

    while (TPH_THD_EXIT == 0)
    {
        result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {

            filter_check_full(&dev->hdc3021_temp_filter);
            filter_check_full(&dev->hdc3021_humi_filter);
            if (read_hw_hdc3021_temperature_humidity(dev, &hdc3021_th) == RT_EOK)
            {
                dev->hdc3021_temp_filter.buf[dev->hdc3021_temp_filter.index] = hdc3021_th.temperature;
                dev->hdc3021_humi_filter.buf[dev->hdc3021_humi_filter.index] = hdc3021_th.humidity;
            }
            else
            {
                dev->hdc3021_temp_filter.buf[dev->hdc3021_temp_filter.index] = ERROR_SENSOR_VALUE;
                dev->hdc3021_humi_filter.buf[dev->hdc3021_humi_filter.index] = ERROR_SENSOR_VALUE;
            }
            dev->hdc3021_temp_filter.index++;
            dev->hdc3021_humi_filter.index++;

            filter_check_full(&dev->tmp116_1_temp_filter);
            if (read_hw_tmp116_temperature(dev, TMP116_1_ADDR, &tmp116_1_temp) == RT_EOK)
            {
                dev->tmp116_1_temp_filter.buf[dev->tmp116_1_temp_filter.index] = tmp116_1_temp;
            }
            else
            {
                dev->tmp116_1_temp_filter.buf[dev->tmp116_1_temp_filter.index] = ERROR_SENSOR_VALUE;
            }
            dev->tmp116_1_temp_filter.index++;

            filter_check_full(&dev->tmp116_2_temp_filter);
            if (read_hw_tmp116_temperature(dev, TMP116_2_ADDR, &tmp116_2_temp) == RT_EOK)
            {
                dev->tmp116_2_temp_filter.buf[dev->tmp116_2_temp_filter.index] = tmp116_2_temp;
            }
            else
            {
                dev->tmp116_2_temp_filter.buf[dev->tmp116_2_temp_filter.index] = ERROR_SENSOR_VALUE;
            }
            dev->tmp116_2_temp_filter.index++;

            filter_check_full(&dev->fdc1004_level_filter);
            if (read_hw_fdc1004_water_level(dev, (rt_uint8_t)FDC1004_ADDR, &water_level) == RT_EOK)
            {
                dev->fdc1004_level_filter.buf[dev->fdc1004_level_filter.index] = water_level;
            }
            else
            {
                dev->fdc1004_level_filter.buf[dev->fdc1004_level_filter.index] = ERROR_SENSOR_VALUE;
            }
            dev->fdc1004_level_filter.index++;
        }
        else
        {
            LOG_E("The software failed to take temperature and humidity at this time. Please try again");
        }
        rt_mutex_release(dev->lock);

        if (first_cycle == 0)
        {
            first_cycle = 1;
            rt_mutex_release(dev->filter_lock);
        }

        rt_thread_delay(rt_tick_from_millisecond(dev->period));
    }
}

static rt_err_t read_hw_hdc3021_temperature_humidity(iic_sensor_t dev, hdc3021_iic_t *temp_rh)
{
    rt_err_t res;
    res = hdc3021_read_by_trigger_on_demand_mode(dev->i2c, &temp_rh->temperature, &temp_rh->humidity);
    return res;
}

static rt_err_t read_hw_tmp116_temperature(iic_sensor_t dev, const rt_uint8_t addr, float *temp)
{
    rt_err_t res;
    res = temp116_read_temp_by_one_shot(dev->i2c, addr, temp);
    return res;
}

static rt_err_t read_hw_fdc1004_water_level(iic_sensor_t dev, const rt_uint8_t addr, float *level)
{
    rt_err_t res;
    res = fdc1004_meas_data(dev->i2c, level);
    return res;
}

float hdc3021_read_temperature(iic_sensor_t dev)
{
    average_measurement(dev, &dev->hdc3021_temp_filter);
    return dev->hdc3021_temp_filter.average;
}

float hdc3021_read_humidity(iic_sensor_t dev)
{
    average_measurement(dev, &dev->hdc3021_humi_filter);
    return dev->hdc3021_humi_filter.average;
}

float tmp116_1_read_temperature(iic_sensor_t dev)
{
    average_measurement(dev, &dev->tmp116_1_temp_filter);
    return dev->tmp116_1_temp_filter.average;
}

float tmp116_2_read_temperature(iic_sensor_t dev)
{
    average_measurement(dev, &dev->tmp116_2_temp_filter);
    return dev->tmp116_2_temp_filter.average;
}

float fdc1004_read_water_level(iic_sensor_t dev)
{
    average_measurement(dev, &dev->fdc1004_level_filter);
    return dev->fdc1004_level_filter.average;
}

static void average_measurement(iic_sensor_t dev, filter_data_t *filter)
{
    rt_uint32_t i;
    float sum = 0;
    rt_uint32_t temp;
    rt_uint32_t cnt = 0;
    rt_err_t result;

    RT_ASSERT(dev);

    if (filter->is_full != RT_TRUE && filter->index == 0)
    {
        rt_mutex_take(dev->filter_lock, 1000);
        rt_mutex_release(dev->filter_lock);
    }

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);

    if (result == RT_EOK)
    {
        if (filter->is_full)
        {
            temp = IIC_SAMPLE_SIZE;
        }
        else
        {
            temp = filter->index;
        }

        for (i = 0; i < temp; i++)
        {
            if (filter->buf[i] != ERROR_SENSOR_VALUE)
            {
                sum += filter->buf[i];
                cnt += 1;
            }
        }
        if (cnt > 0)
        {
            filter->average = sum / cnt;
        }
        else
        {
            filter->average = ERROR_SENSOR_VALUE;
        }
    }
    else
    {
        LOG_E("The software failed to average at this time. Please try again");
    }
    rt_mutex_release(dev->lock);
}

static void filter_check_full(filter_data_t *filter)
{
    if (filter->index >= IIC_SAMPLE_SIZE)
    {
        if (filter->is_full != RT_TRUE)
        {
            filter->is_full = RT_TRUE;
        }
        filter->index = 0;
    }
}

static void test_iic_sensors(int argc, char **argv)
{
    rt_err_t res;

    sensor_pwron_pin_enable(1);
    iic_sensor_t dev = iic_sensors_init("i2c1");

    rt_uint8_t cnt = 0;
    while (cnt < 10)
    {
        float temp, humi, level;
        temp = hdc3021_read_temperature(dev);
        humi = hdc3021_read_humidity(dev);
        LOG_D("HDC3021 temp %lf, humi %lf\r\n", temp, humi);

        temp = tmp116_1_read_temperature(dev);
        LOG_D("TMP116_1 temp %lf\r\n", temp);

        temp = tmp116_2_read_temperature(dev);
        LOG_D("TMP116_2 temp %lf\r\n", temp);

        level = fdc1004_read_water_level(dev);
        LOG_D("FDC1004 water level %lf\r\n", level);

        rt_thread_delay(rt_tick_from_millisecond(1000));
        cnt++;
    }

    iic_sensors_deinit(dev);
    sensor_pwron_pin_enable(0);
}

MSH_CMD_EXPORT(test_iic_sensors, iic start);
