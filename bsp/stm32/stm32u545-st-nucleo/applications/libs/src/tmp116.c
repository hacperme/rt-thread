/*
 * @FilePath: tmp116.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-14 14:56:20
 * @copyright : Copyright (c) 2024
 */
#include "tmp116.h"
#include <stdio.h>

#define DBG_TAG "TMP116"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

rt_err_t tmp116_read_device_id(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, rt_uint16_t *dev_id)
{
    rt_err_t res;
    rt_ssize_t ret;
    ret = rt_i2c_master_send(iic_dev, addr, RT_I2C_WR, &TMP116_DEVICE_ID_REG, 1);
    res = ret == 1 ? RT_EOK : RT_ERROR;
    LOG_D(
        "rt_i2c_master_send addr=0x%02X reg=0x%02X %s ret=%d",
        addr, TMP116_DEVICE_ID_REG, res == RT_EOK ? "success": "failed", ret
    );
    if (res != RT_EOK)
    {
        return res;
    }

    rt_uint8_t recv_buf[2] = {0};
    ret = rt_i2c_master_recv(iic_dev, addr, RT_I2C_RD, recv_buf, 2);
    res = ret == 2 ? RT_EOK : RT_ERROR;
    LOG_D(
        "rt_i2c_master_recv addr=0x%02X reg=0x%02X %s ret=%d, recv_buf 0x%02X 0x%02X",
        addr, TMP116_DEVICE_ID_REG, res == RT_EOK ? "success": "failed", ret, recv_buf[0], recv_buf[1]
    );
    if (res == RT_EOK)
    {
        *dev_id = recv_buf[0] << 8 | recv_buf[1];
    }

    return res;
}

rt_err_t tmp116_set_configuration(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, rt_uint8_t *cfg, rt_uint8_t size)
{
    rt_err_t res;
    rt_ssize_t ret;
    rt_uint8_t cfgs[size + 1];
    cfgs[0] = TMP116_DEVICE_ID_REG;
    rt_memcpy(&cfgs[1], cfg, size);
    ret = rt_i2c_master_send(iic_dev, addr, RT_I2C_WR, cfgs, size);
    res = ret == size ? RT_EOK : RT_ERROR;
    LOG_D(
        "rt_i2c_master_send addr=0x%02X reg=0x%02X val=0x%02X 0x%02X %s ret=%d",
        addr, cfgs[0], cfgs[1], cfgs[2], res == RT_EOK ? "success": "failed", ret
    );
    return res;
}

rt_err_t tmp116_read_configuration(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, rt_uint16_t *cfg)
{
    rt_err_t res;
    rt_ssize_t ret;
    ret = rt_i2c_master_send(iic_dev, addr, RT_I2C_WR, &TMP116_CFGR_REG, 1);
    res = ret == 1 ? RT_EOK : RT_ERROR;
    LOG_D(
        "rt_i2c_master_send addr=0x%02X reg=0x%02X %s ret=%d",
        addr, TMP116_CFGR_REG, res == RT_EOK ? "success": "failed", ret
    );
    if (res != RT_EOK)
    {
        return res;
    }

    rt_uint8_t recv_buf[2] = {0};
    ret = rt_i2c_master_recv(iic_dev, addr, RT_I2C_RD, recv_buf, 2);
    res = ret == 2 ? RT_EOK : RT_ERROR;
    LOG_D(
        "rt_i2c_master_recv addr=0x%02X reg=0x%02X %s ret=%d, recv_buf 0x%02X 0x%02X",
        addr, TMP116_CFGR_REG, res == RT_EOK ? "success": "failed", ret, recv_buf[0], recv_buf[1]
    );
    if (res == RT_EOK)
    {
        *cfg = recv_buf[0] << 8 | recv_buf[1];
    }

    return res;
}

rt_err_t tmp116_data_ready(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, rt_uint8_t *ready)
{
    rt_err_t res;
    rt_uint16_t cfg = 0;
    res = tmp116_read_configuration(iic_dev, addr, &cfg);
    if (res == RT_EOK)
    {
        *ready = (cfg & (0x01 << 13)) >> 13;
    }
    return res;
}

rt_err_t tmp116_measure_temperature(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, float *temp)
{
    rt_err_t res = RT_ERROR;
    rt_uint8_t ready = 0;
    rt_uint8_t cnt = 0;
    do {
        res = tmp116_data_ready(iic_dev, addr, &ready);
        if (res == RT_EOK && ready == 1){
            break;
        }
        cnt++;
        rt_thread_mdelay(5);
    } while (ready != 1 && cnt < 20);

    if (ready != 1)
    {
        LOG_E("TMP116 temperature is not ready.");
        res = RT_ERROR;
        return res;
    }

    rt_ssize_t ret;
    ret = rt_i2c_master_send(iic_dev, addr, RT_I2C_WR, &TMP116_TEMP_REG, 1);
    res = ret == 1 ? RT_EOK : RT_ERROR;
    LOG_D(
        "rt_i2c_master_send addr=0x%02X reg=0x%02X %s ret=%d",
        addr, TMP116_TEMP_REG, res == RT_EOK ? "success": "failed", ret
    );
    if (res != RT_EOK)
    {
        return res;
    }

    rt_uint8_t recv_buf[2] = {0};
    ret = rt_i2c_master_recv(iic_dev, addr, RT_I2C_RD, recv_buf, 2);
    res = ret == 2 ? RT_EOK : RT_ERROR;
    LOG_D(
        "rt_i2c_master_recv addr=0x%02X reg=0x%02X %s ret=%d, recv_buf 0x%02X 0x%02X",
        addr, TMP116_TEMP_REG, res == RT_EOK ? "success": "failed", ret, recv_buf[0], recv_buf[1]
    );
    if (res == RT_EOK)
    {
        *temp = (float)(recv_buf[0] << 8 | recv_buf[1]) * 0.0078125;
    }
    return res;
}

rt_err_t temp116_read_temperture(struct rt_i2c_bus_device *iic_dev, const rt_uint8_t addr, float *temp)
{
    rt_err_t res;

    rt_uint8_t cfg[2] = {0x0E, 0x20};
    res = tmp116_set_configuration(iic_dev, addr, cfg, 2);
    LOG_D("tmp116_set_configuration addr=0x%02X %s", addr, res != RT_EOK ? "failed" : "success");

    res = tmp116_measure_temperature(iic_dev, addr, temp);
    LOG_D("tmp116_measure_temperature addr=0x%02X %s temp=%f", addr, res != RT_EOK ? "failed" : "success", *temp);

    return res;
}

#ifdef RT_USING_MSH
#include "board_pin.h"
static rt_err_t test_temp116(void)
{
    rt_err_t res = RT_EOK;
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

    rt_uint8_t addrs[2] = {TMP116_1_ADDR, TMP116_2_ADDR};
    rt_uint16_t dev_id;
    float temp;
    char msg[256];

    for (rt_uint8_t i = 0; i < 2; i ++)
    {
        dev_id = 0;
        res = tmp116_read_device_id(iic_dev, addrs[i], &dev_id);
        LOG_D("tmp116_read_device_id addr=0x%02X %s dev_id=0x%02X", addrs[i], res != RT_EOK ? "failed" : "success", dev_id);

        temp = -999.0;
        res = temp116_read_temperture(iic_dev, addrs[i], &temp);
        sprintf(msg, "temp116_read_temperture addr=0x%02X %s temp=%f", addrs[i], res != RT_EOK ? "failed" : "success", temp);
        LOG_D(msg);
    }

    return res;
}

MSH_CMD_EXPORT(test_temp116, test temp116);
#endif
