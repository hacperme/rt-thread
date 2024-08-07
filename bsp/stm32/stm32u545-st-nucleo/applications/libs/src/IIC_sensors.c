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

#define HDC3021_ADDR 0x44
static rt_uint8_t HDC3021_TRIGGER_ON_DEMAND[2] = {0x24, 0x00};
static rt_uint8_t HDC3021_SOFT_RESET[2] = {0x30, 0xA2};

#define TMP116_1_ADDR 0x48
#define TMP116_2_ADDR 0x49
static rt_uint8_t TMP116_CFG_OS_DATA[3] = {0x01, 0x0E, 0x20};
static rt_uint8_t TMP116_TEMP_REG_ADDR = 0x00;

#define FDC1004_ADDR 0x50
static rt_uint8_t FDC1004_FDC_READ_DATA = 0;
static rt_uint16_t FDC1004_CLEVEL_0 = 0;
static rt_uint16_t FDC1004_CIN_LVLS[4] = {0};
static rt_uint8_t FDC1004_FDC_CFG_DATA[4][3] = {
    {0x0C, 0x0D, 0x80},  // CIN1
    {0x0C, 0x0D, 0x40},  // CIN2
    {0x0C, 0x0D, 0x20},  // CIN3
    {0x0C, 0x0D, 0x10},  // CIN4
};
static rt_uint8_t FDC1004_MEAS_CFG[4][3] = {
	{0x08, 0x1C, 0x00},  // CIN1
	{0x09, 0x3C, 0x00},  // CIN2
	{0x0A, 0x5C, 0x00},  // CIN3
	{0x0B, 0x7C, 0x00},  // CIN4
};
static rt_uint8_t FDC1004_MEAS_MSB_LSB_ADDR[4][2] = {
    {0x00, 0x01},  // CIN1 MSB, LSB
    {0x02, 0x03},  // CIN2 MSB, LSB
    {0x04, 0x05},  // CIN3 MSB, LSB
    {0x06, 0x07},  // CIN4 MSB, LSB
};

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
            if (read_hw_tmp116_temperature(dev, (rt_uint8_t)TMP116_1_ADDR, &tmp116_1_temp) == RT_EOK)
            {
                dev->tmp116_1_temp_filter.buf[dev->tmp116_1_temp_filter.index] = tmp116_1_temp;
            }
            else
            {
                dev->tmp116_1_temp_filter.buf[dev->tmp116_1_temp_filter.index] = ERROR_SENSOR_VALUE;
            }
            dev->tmp116_1_temp_filter.index++;
            filter_check_full(&dev->tmp116_2_temp_filter);
            if (read_hw_tmp116_temperature(dev, (rt_uint8_t)TMP116_2_ADDR, &tmp116_2_temp) == RT_EOK)
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
    rt_uint8_t data[6] = {0};
    if (rt_i2c_master_send(dev->i2c, HDC3021_ADDR, RT_I2C_WR, HDC3021_TRIGGER_ON_DEMAND, 2) == 2)
    {
        rt_thread_delay(rt_tick_from_millisecond(20));
        if (rt_i2c_master_recv(dev->i2c, HDC3021_ADDR, RT_I2C_RD, data, 6) == 6)
        {
            // LOG_D("DATA: %02X, %02X, %02X, %02X, %02X, %02X", data[0], data[1], data[2], data[3], data[4], data[5]);
            temp_rh->temperature = (float)((data[0] << 8) | data[1]) / 65536.0 * 175.0 - 45.0;
            temp_rh->humidity = (float)((data[3] << 8) | data[4]) / 65536.0 * 100.0;
            return RT_EOK;
        }
        else
        {
            LOG_E("hdc3021 rt_i2c_master_recv failed.");
        }
    }
    else{
        LOG_E("hdc3021 rt_i2c_master_send failed.");
    }
    return RT_ERROR;
}

static rt_err_t read_hw_tmp116_temperature(iic_sensor_t dev, const rt_uint8_t addr, float *temp)
{
    rt_err_t res = RT_ERROR;
    rt_uint8_t data[2] = {0};
    rt_ssize_t i2c_res;
    i2c_res = rt_i2c_master_send(dev->i2c, addr, RT_I2C_WR, TMP116_CFG_OS_DATA, 3);
    LOG_D("read_hw_tmp116_temperature ADDR %#02X, rt_i2c_master_send %d", addr, res);
    if (i2c_res == 3)
    {
        rt_thread_delay(rt_tick_from_millisecond(50));
        if(rt_i2c_master_send(dev->i2c, addr, RT_I2C_WR, &TMP116_TEMP_REG_ADDR, 1) == 1)
        {
            rt_thread_delay(rt_tick_from_millisecond(20));
            if (rt_i2c_master_recv(dev->i2c, addr, RT_I2C_RD, data, 2) == 2)
            {
                LOG_D("TMP116 DATA 0x%02X, 0x%02X", data[0], data[1]);
                *temp = (float)((data[0] << 8) | data[1]) * 0.0078125;
                res = RT_EOK;
            }
            else
            {
                LOG_E("rt_i2c_master_recv failed");
            }
        }
        else
        {
            LOG_E("rt_i2c_master_send TMP116_TEMP_REG_ADDR failed.");
        }
    }
    else
    {
        LOG_E("rt_i2c_master_send TMP116_CFG_OS_DATA failed.");
    }
    return res;
}

static rt_err_t read_hw_fdc1004_water_level(iic_sensor_t dev, const rt_uint8_t addr, float *level)
{
    rt_err_t res = RT_EOK;
    rt_uint8_t CIN_DATAS[4][2] = {0};
    rt_uint8_t cin_no = 0, lbm = 0;
    rt_ssize_t i2c_res;
    rt_uint8_t cnt = 0;

    rt_uint8_t FDC1004_ID_ADDR = 0xFF;
    rt_uint8_t FDC1004_DEV_ID[2] = {0};
    i2c_res = rt_i2c_master_send(dev->i2c, addr, RT_I2C_WR, &FDC1004_ID_ADDR, 1);
    if (i2c_res == 1)
    {
        rt_i2c_master_recv(dev->i2c, addr, RT_I2C_RD, FDC1004_DEV_ID, 2);
        LOG_D("FDC1004 Device ID 0x%02x 0x%02x", FDC1004_DEV_ID[0], FDC1004_DEV_ID[1]);
    }
    else
    {
        LOG_E("FDC1004 send FDC1004_ID_ADDR failed. i2c_res %d", i2c_res);
    }
    for (cin_no = 0; cin_no < 4; cin_no++)
    {
        i2c_res = rt_i2c_master_send(dev->i2c, addr, RT_I2C_WR, FDC1004_FDC_CFG_DATA[cin_no], 3);
        if (i2c_res == 3)
        {
            rt_thread_delay(rt_tick_from_millisecond(20));
            i2c_res = rt_i2c_master_send(dev->i2c, addr, RT_I2C_WR, FDC1004_MEAS_CFG[cin_no], 3);
            if (i2c_res == 3)
            {
                cnt = 0;
                FDC1004_FDC_READ_DATA = 0;
                do {
                    rt_i2c_master_recv(dev->i2c, addr, RT_I2C_RD, &FDC1004_FDC_READ_DATA, 1);
                    cnt++;
                    rt_thread_delay(rt_tick_from_millisecond(5));
                } while (FDC1004_FDC_READ_DATA & 0x08 == 0 && cnt < 20);
            }
            else
            {
                res = RT_ERROR;
                LOG_E("CIN%d Send FDC1004_MEAS_CFG failed. i2c_res %s", cin_no, i2c_res);
                break;
            }
            if (FDC1004_FDC_READ_DATA & 0x08 == 1)
            {
                for (lbm = 0; lbm < 2; lbm++)
                {
                    i2c_res = rt_i2c_master_send(dev->i2c, addr, RT_I2C_WR, &FDC1004_MEAS_MSB_LSB_ADDR[cin_no][lbm], 1);
                    if (i2c_res == 1)
                    {
                        rt_i2c_master_recv(dev->i2c, addr, RT_I2C_RD, &CIN_DATAS[cin_no][lbm], 1);
                    }
                    else
                    {
                        res = RT_ERROR;
                        LOG_E("CIN%d %s read failed. i2c_res %d", cin_no, ((lbm == 0) ? "MSB" : "LSB"), i2c_res);
                        break;
                    }
                }
                FDC1004_CIN_LVLS[cin_no] = (CIN_DATAS[cin_no][0] << 8) | CIN_DATAS[cin_no][1];
            }
            else
            {
                res = RT_ERROR;
                LOG_E("CIN%d measurement not ready.", cin_no);
                break;
            }
        }
        else
        {
            res = RT_ERROR;
            LOG_E("CIN%d Send FDC1004_FDC_CFG_DATA failed. i2c_res %d", cin_no, i2c_res);
            break;
        }
        rt_thread_delay(rt_tick_from_millisecond(50));
    }
    if (res == RT_EOK)
    {
        *level = 1.0 * (float)(((FDC1004_CIN_LVLS[0] - FDC1004_CIN_LVLS[3]) - FDC1004_CLEVEL_0) / (FDC1004_CIN_LVLS[1] - FDC1004_CIN_LVLS[2]));
    }
    return res;
}

rt_err_t check_fdc1004_clevel0(iic_sensor_t dev)
{
    float level;
    rt_err_t result, res;
    rt_uint8_t cnt = 0;
    res = RT_ERROR;
    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        while (cnt < 5)
        {
            if (read_hw_fdc1004_water_level(dev, (rt_uint8_t)FDC1004_ADDR, &level) == RT_EOK)
            {
                FDC1004_CLEVEL_0 = FDC1004_CIN_LVLS[0] - FDC1004_CIN_LVLS[3];
                res = RT_EOK;
                break;
            }
            else
            {
                LOG_E("read_hw_fdc1004_water_level failed.");
            }
            cnt++;
            rt_thread_delay(rt_tick_from_millisecond(100));
        }
    }
    else
    {
        LOG_E("check_fdc1004_clevel0 take lock failed.");
    }
    rt_mutex_release(dev->lock);
    return FDC1004_CLEVEL_0;
}

rt_uint16_t read_fdc1004_clevel0(void)
{
    return FDC1004_CLEVEL_0;
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

float tmp116_1_read_humidity(iic_sensor_t dev)
{
    average_measurement(dev, &dev->tmp116_1_temp_filter);
    return dev->tmp116_1_temp_filter.average;
}

float tmp116_2_read_humidity(iic_sensor_t dev)
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
    sensor_pwron_pin_enable(1);
    iic_sensor_t dev = iic_sensors_init("i2c1");
    rt_uint8_t cnt = 0;
    while (cnt < 10)
    {
        float temp, humi, level;

        temp = hdc3021_read_temperature(dev);
        humi = hdc3021_read_humidity(dev);
        LOG_D("HDC3021 temp %lf, humi %lf\r\n", temp, humi);

        temp = tmp116_1_read_humidity(dev);
        LOG_D("TMP116_1 temp %lf\r\n", temp);

        temp = tmp116_2_read_humidity(dev);
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
