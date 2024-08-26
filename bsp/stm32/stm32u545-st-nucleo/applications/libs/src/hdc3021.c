/*
 * @FilePath: hdc3021.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-14 10:03:03
 * @copyright : Copyright (c) 2024
 */
#include <stdio.h>
#include "hdc3021.h"
#include "tools.h"

#define DBG_TAG "HDC3021"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

rt_err_t hdc3021_soft_reset(struct rt_i2c_bus_device *iic_dev)
{
    rt_err_t res;
    rt_ssize_t ret;
    ret = rt_i2c_master_send(iic_dev, HDC3021_ADDR, RT_I2C_WR, HDC3021_SOFT_RESET, 2);
    res = ret == 2 ? RT_EOK : RT_ERROR;
    LOG_D("hdc3021_soft_reset %s ret=%d", res == RT_EOK ? "success": "failed", ret);
    return res;
}

rt_err_t hdc3021_trigger_on_demand(struct rt_i2c_bus_device *iic_dev)
{
    rt_err_t res;
    rt_ssize_t ret;
    ret = rt_i2c_master_send(iic_dev, HDC3021_ADDR, RT_I2C_WR, HDC3021_TRIGGER_ON_DEMAND, 2);
    res = ret == 2 ? RT_EOK : RT_ERROR;
    LOG_D("hdc3021_trigger_on_demand %s ret=%d", res == RT_EOK ? "success": "failed", ret);
    return res;
}

rt_err_t hdc3021_read_temp_humi_by_tod(struct rt_i2c_bus_device *iic_dev, float *temp, float *humi)
{
    rt_err_t res;
    rt_ssize_t ret;
    rt_uint8_t recv_buf[6] = {0};
    rt_uint8_t data_buf[2] = {0};
    rt_uint8_t crc_num = 0;
    ret = rt_i2c_master_recv(iic_dev, HDC3021_ADDR, RT_I2C_RD, recv_buf, 6);
    res = ret == 6 ? RT_EOK : RT_ERROR;
    if (res == RT_EOK)
    {
        LOG_D(
            "hdc3021_read_temp_humi_by_tod recv_buf 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
            recv_buf[0], recv_buf[1], recv_buf[2], recv_buf[3], recv_buf[4], recv_buf[5]
        );
        for (rt_uint8_t i = 0; i < 6; i++)
        {
            data_buf[0] = recv_buf[i];
            data_buf[1] = recv_buf[++i];
            crc_num = recv_buf[++i];
            res = crc8_check(data_buf, 2, &crc_num);
            LOG_D(
                "crc8_check 0x%02X 0x%02X CRC=0x%02X %s",
                data_buf[0], data_buf[1], crc_num, res == RT_EOK ? "success" : "failed"
            );
            if (res != RT_EOK)
            {
                break;
            }
            if (i < 3)
            {
                *temp = (float)((recv_buf[0] << 8) | recv_buf[1]) / 65535.0 * 175.0 - 45.0;
            }
            else
            {
                *humi = (float)((recv_buf[3] << 8) | recv_buf[4]) / 65535.0 * 100.0;
            }
        }
    }
    LOG_D("hdc3021_read_temp_humi_by_tod %s ret=%d", res == RT_EOK ? "success": "failed", ret);
    return res;
}

rt_err_t hdc3021_read_temp_humi(struct rt_i2c_bus_device *iic_dev, float *temp, float *humi)
{
    rt_err_t res = RT_EOK;
    res = hdc3021_trigger_on_demand(iic_dev);
    LOG_D("hdc3021_trigger_on_demand %s", res != RT_EOK ? "failed" : "success");
    if (res != RT_EOK)
    {
        return res;
    }
    rt_thread_mdelay(10);
    res = hdc3021_read_temp_humi_by_tod(iic_dev, temp, humi);
    LOG_D(
        "hdc3021_read_temp_humi_by_tod %s. temp=%f, humi=%f",
        res != RT_EOK ? "failed" : "success", *temp, *humi
    );

    return res;
}

#ifdef RT_USING_MSH
#include "board_pin.h"
static rt_err_t test_crc_check(void)
{
    rt_err_t res = RT_EOK;

    rt_uint8_t data[2] = {0xAB, 0xCD};
    rt_uint8_t cmp_val = 0x6F;
    rt_uint32_t ret;
    res = crc8_check(data, 2, &cmp_val);
    LOG_D(
        "crc8_check 0x%02X 0x%02X CRC=0x%02X %s",
        data[0], data[1], cmp_val, res == RT_EOK ? "success" : "failed"
    );
    return res;
}

static rt_err_t test_hdc3021(void)
{
    rt_err_t res = RT_EOK;

    res = test_crc_check();

    char i2c_bus_name[] = "i2c1";
    static struct rt_i2c_bus_device *iic_dev;

    res = sensor_pwron_pin_enable(1);
    LOG_D("sensor_pwron_pin_enable(1) %s", res != RT_EOK ? "failed" : "success");

    iic_dev = rt_i2c_bus_device_find(i2c_bus_name);
    res = !iic_dev ? RT_ERROR : RT_EOK;
    LOG_D("rt_i2c_bus_device_find %s %s", i2c_bus_name, res != RT_EOK ? "failed" : "success");
    if (res != RT_EOK)
    {
        return res;
    }

    float temp = -999.0;
    float humi = -999.0;
    char msg[256];
    for (rt_uint8_t i = 0; i < 3; i++)
    {
        /* Not Use soft reset, humi will not be 0.0 after soft reset. */
        // res = hdc3021_soft_reset(iic_dev);
        // LOG_D("hdc3021_soft_reset %s.", res != RT_EOK ? "failed" : "success");
        res = hdc3021_read_temp_humi(iic_dev, &temp, &humi);
        sprintf(msg, "hdc3021_read_temp_humi %s. temp=%f, humi=%f", res != RT_EOK ? "failed" : "success", temp, humi);
        LOG_D(msg);
        rt_thread_mdelay(1000);
    }

    return res;
}

// MSH_CMD_EXPORT(test_hdc3021, test hdc3021);
#endif
