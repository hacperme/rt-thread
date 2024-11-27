/*
 * @FilePath: fdc1004.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-10 11:14:56
 * @copyright : Copyright (c) 2024
 */

#include <stdio.h>
#include "fdc1004.h"
#include "logging.h"
#include "tools.h"

rt_err_t fdc1004_read_manufacturer_id(struct rt_i2c_bus_device *iic_dev, rt_uint16_t *manufacturer_id)
{
    rt_err_t res;
    rt_ssize_t i2c_res;
    rt_uint8_t MANUFACTURER_ID[2] = {0};

    i2c_res = rt_i2c_master_send(iic_dev, FDC1004_ADDR, RT_I2C_WR, &FDC1004_Manufacturer_REG, 1);
    res = i2c_res == 1 ? RT_EOK : RT_ERROR;
    log_debug("Send reg=0x%02x res=%d", FDC1004_Manufacturer_REG, i2c_res);
    if (res != RT_EOK)
    {
        return res;
    }

    i2c_res = rt_i2c_master_recv(iic_dev, FDC1004_ADDR, RT_I2C_RD, MANUFACTURER_ID, 2);
    res = i2c_res == 2 ? RT_EOK : RT_ERROR;
    log_debug("Recv res=%d MANUFACTURER_ID 0x%02x 0x%02x", i2c_res, MANUFACTURER_ID[0], MANUFACTURER_ID[1]);

    if (res == RT_EOK)
    {
        *manufacturer_id = (MANUFACTURER_ID[0] << 8) | MANUFACTURER_ID[1];
    }
    return res;
}

rt_err_t dfc1004_read_device_id(struct rt_i2c_bus_device *iic_dev, rt_uint16_t *dev_id)
{
    rt_err_t res;
    rt_ssize_t i2c_res;
    rt_uint8_t DEVICE_ID[2] = {0};

    i2c_res = rt_i2c_master_send(iic_dev, FDC1004_ADDR, RT_I2C_WR, &FDC1004_Device_REG, 1);
    res = i2c_res == 1 ? RT_EOK : RT_ERROR;
    log_debug("Send reg=0x%02x res=%d", FDC1004_Device_REG, i2c_res);
    if (res != RT_EOK)
    {
        return res;
    }

    i2c_res = rt_i2c_master_recv(iic_dev, FDC1004_ADDR, RT_I2C_RD, DEVICE_ID, 2);
    res = i2c_res == 2 ? RT_EOK : RT_ERROR;
    log_debug("Recv res=%d DEVICE_ID 0x%02x 0x%02x", i2c_res, DEVICE_ID[0], DEVICE_ID[1]);
    if (res == RT_EOK)
    {
        *dev_id = (DEVICE_ID[0] << 8) | DEVICE_ID[1];
    }
    return res;
}

rt_err_t fdc1004_meas_config(struct rt_i2c_bus_device *iic_dev, rt_uint8_t reg, rt_uint8_t val_h, rt_uint8_t val_l)
{
    rt_err_t res;
    rt_ssize_t i2c_res;
    rt_uint8_t send_buf[3] = {reg, val_h, val_l};

    i2c_res = rt_i2c_master_send(iic_dev, FDC1004_ADDR, RT_I2C_WR, send_buf, 3);
    res = i2c_res == 3 ? RT_EOK : RT_ERROR;
    log_debug("fdc1004_meas_config reg=0x%02x res=%d", reg, i2c_res);
    return res;
}

rt_err_t fdc1004_meas_trigger(struct rt_i2c_bus_device *iic_dev, rt_uint8_t val_h, rt_uint8_t val_l)
{
    rt_err_t res;
    rt_ssize_t i2c_res;
    rt_uint8_t send_buf[3] = {FDC1004_FDC_CONF_REG, val_h, val_l};

    i2c_res = rt_i2c_master_send(iic_dev, FDC1004_ADDR, RT_I2C_WR, send_buf, 3);
    res = i2c_res == 3 ? RT_EOK : RT_ERROR;
    log_debug("fdc1004_meas_trigger reg=0x%02x res=%d", FDC1004_FDC_CONF_REG, i2c_res);
    return res;
}

rt_err_t fdc1004_meas_done(struct rt_i2c_bus_device *iic_dev, rt_uint8_t meas_id, rt_uint32_t timeout, rt_uint8_t *done)
{
    rt_err_t res = RT_ERROR;
    rt_ssize_t i2c_res;
    rt_uint8_t meas_status[2] = {0};
    rt_uint32_t cnt = timeout / 50;
    rt_int8_t offset = 4 - meas_id;
    if (offset < 0)
    {
        return res;
    }

    do {
        i2c_res = rt_i2c_master_send(iic_dev, FDC1004_ADDR, RT_I2C_WR, &FDC1004_FDC_CONF_REG, 1);
        res = i2c_res == 1 ? RT_EOK : RT_ERROR;
        log_debug("Send reg=0x%02x res=%d", FDC1004_FDC_CONF_REG, i2c_res);
        if (res != RT_EOK)
        {
            break;
        }

        i2c_res = rt_i2c_master_recv(iic_dev, FDC1004_ADDR, RT_I2C_RD, meas_status, 2);
        res = i2c_res == 2 ? RT_EOK : RT_ERROR;
        log_debug("Recv res=%d meas_status 0x%02x 0x%02x", i2c_res, meas_status[0], meas_status[1]);
        if (res != RT_EOK)
        {
            break;
        }
        *done = (meas_status[1] & (0x01 << offset)) >> offset;
        cnt--;
        rt_thread_mdelay(50);
    } while (*done != 1 && cnt > 0);

    return res;
}

rt_err_t fdc1004_meas_read(struct rt_i2c_bus_device *iic_dev, rt_uint8_t reg, rt_uint16_t *value)
{
    rt_err_t res;
    rt_ssize_t i2c_res;
    rt_uint8_t values[2] = {0};

    i2c_res = rt_i2c_master_send(iic_dev, FDC1004_ADDR, RT_I2C_WR, &reg, 1);
    res = i2c_res == 1 ? RT_EOK : RT_ERROR;
    log_debug("Send reg=0x%02x res=%d", reg, i2c_res);
    if (res != RT_EOK)
    {
        return res;
    }

    i2c_res = rt_i2c_master_recv(iic_dev, FDC1004_ADDR, RT_I2C_RD, values, 2);
    res = i2c_res == 2 ? RT_EOK : RT_ERROR;
    log_debug("Recv res=%d values 0x%02x 0x%02x", i2c_res, values[0], values[1]);
    if (res == RT_EOK)
    {
        *value = (values[0] << 8) | values[1];
    }
    return res;
}

rt_err_t fdc1004_meas_all_config(struct rt_i2c_bus_device *iic_dev)
{
    rt_err_t res = RT_ERROR;
    rt_uint8_t i;

    for (i = 0; i < 4; i++)
    {
        res = fdc1004_meas_config(
            iic_dev, FDC1004_MEAS_CFG[i][0], FDC1004_MEAS_CFG[i][1], FDC1004_MEAS_CFG[i][2]
        );
        if (res != RT_EOK)
        {
            return res;
        }
    }
    return res;
}

rt_err_t fdc1004_meas_all_trigger(struct rt_i2c_bus_device *iic_dev)
{
    rt_err_t res = RT_ERROR;
    rt_uint8_t i;
    /* Measurement trigger */
    for (i = 0; i < 4; i++)
    {
        res = fdc1004_meas_trigger(iic_dev, FDC1004_MEAS_TRG[i][0], FDC1004_MEAS_TRG[i][1]);
        if (res != RT_EOK)
        {
            return res;
        }
    }
    return res;
}

rt_err_t fdc1004_meas_all_done(struct rt_i2c_bus_device *iic_dev, rt_uint8_t *meas_done, rt_uint8_t size)
{
    rt_err_t res = RT_ERROR;
    rt_uint8_t i;
    rt_uint32_t timeout = 3000;
    for (i = 0; i < size; i++)
    {
        res = fdc1004_meas_done(iic_dev, i + 1, timeout, &meas_done[i]);
        log_debug("MEAS%d done %d", i, meas_done[i]);
        if (res != RT_EOK)
        {
            return res;
        }
    }
    return res;
}

rt_err_t fdc1004_meas_all_read(struct rt_i2c_bus_device *iic_dev, rt_uint8_t *meas_done, rt_int32_t *measuerment, rt_int8_t size)
{
    rt_err_t res = RT_ERROR;
    rt_uint8_t i, j;
    rt_uint16_t meas_data[4][2] = {0};
    for (i = 0; i < size; i++)
    {
        if (meas_done[i] == 1)
        {
            for (j = 0; j < 2; j++)
            {
                res = fdc1004_meas_read(iic_dev, FDC1004_MEAS_READ_REGS[i][j], &meas_data[i][j]);
                if (res != RT_EOK)
                {
                    return res;
                }
            }
        }
    }
    for (i = 0; i < size; i++)
    {
        measuerment[i] = (meas_data[i][0] << 16) | meas_data[i][1];
    }
    log_debug(
        "MEAS1=0x%02X, MEAS2=0x%02X, MEAS3=0x%02X, MEAS4=0x%02X",
        measuerment[0], measuerment[1], measuerment[2], measuerment[3]
    );
    return res;
}

rt_err_t fdc1004_meas_data(struct rt_i2c_bus_device *iic_dev, float *value)
{
    rt_err_t res = RT_ERROR;

    /* Measurement config */
    res = fdc1004_meas_all_config(iic_dev);
    if (res != RT_EOK)
    {
        return res;
    }

    /* Measurement trigger */
    res = fdc1004_meas_all_trigger(iic_dev);
    if (res != RT_EOK)
    {
        return res;
    }

    /* Measurement wait done */
    rt_uint8_t meas_done[4] = {0};
    res = fdc1004_meas_all_done(iic_dev, meas_done, 4);
    if (res != RT_EOK)
    {
        return res;
    }

    /* Measurement read data */
    rt_int32_t measuerment[4] = {0};
    res = fdc1004_meas_all_read(iic_dev, meas_done, measuerment, 4);
    if (res == RT_EOK)
    {
        res = measuerment[1] != measuerment[2] ? RT_EOK : RT_ERROR;
        if (res == RT_EOK)
        {
            *value = ((float)(measuerment[0] - measuerment[3]) - FDC1004_CLEVEL0) / (float)(measuerment[1] - measuerment[2]);
        }
    }

    return res;
}

rt_err_t fdc1004_check_clevel0(struct rt_i2c_bus_device *iic_dev)
{
    rt_err_t res = RT_ERROR;

    /* Measurement config */
    res = fdc1004_meas_all_config(iic_dev);
    log_debug("fdc1004_meas_all_config res=%d", res);
    if (res != RT_EOK)
    {
        return res;
    }

    /* Measurement trigger */
    res = fdc1004_meas_all_trigger(iic_dev);
    log_debug("fdc1004_meas_all_trigger res=%d", res);
    if (res != RT_EOK)
    {
        return res;
    }

    /* Measurement wait done */
    rt_int8_t meas_done[4] = {0};
    res = fdc1004_meas_all_done(iic_dev, meas_done, 4);
    log_debug("fdc1004_meas_all_done res=%d", res);
    if (res != RT_EOK)
    {
        return res;
    }

    /* Measurement read data */
    rt_int32_t measuerment[4] = {0};
    res = fdc1004_meas_all_read(iic_dev, meas_done, measuerment, 4);
    log_debug("fdc1004_meas_all_read res=%d", res);
    if (res == RT_EOK)
    {
        FDC1004_CLEVEL0 = measuerment[0] - measuerment[3];
    }

    return res;
}

#ifdef RT_USING_MSH
// #include "board_pin.h"
rt_err_t test_fdc1004(int argc, char **argv)
{
    rt_err_t res = RT_ERROR;
    char i2c_bus_name[] = "i2c1";
    static struct rt_i2c_bus_device *iic_dev;

    // res = sensor_pwron_pin_enable(1);
    // log_debug("sensor_pwron_pin_enable(1) %s", res_msg(res == RT_EOK));

    rt_pin_mode(SENSOR_PWRON_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SENSOR_PWRON_PIN, 1);

    iic_dev = rt_i2c_bus_device_find(i2c_bus_name);
    res = !iic_dev ? RT_ERROR : RT_EOK;
    log_debug("rt_i2c_bus_device_find %s %s", i2c_bus_name, res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    rt_uint16_t manufacturer_id = 0x00;
    res = fdc1004_read_manufacturer_id(iic_dev, &manufacturer_id);
    log_debug("fdc1004_read_manufacturer_id %s 0x%02X", res_msg(res == RT_EOK), manufacturer_id);

    rt_uint16_t dev_id = 0x00;
    res = dfc1004_read_device_id(iic_dev, &dev_id);
    log_debug("dfc1004_read_device_id %s 0x%02X", res_msg(res == RT_EOK), dev_id);

    /* FDC1004_CLEVEL0 need to be saved to naflash. */
    if (FDC1004_CLEVEL0 == 0)
    {
        res = fdc1004_check_clevel0(iic_dev);
        log_debug("fdc1004_check_clevel0 %s 0x%02X", res_msg(res == RT_EOK), FDC1004_CLEVEL0);
    }

    float value = 0.0;
    res = fdc1004_meas_data(iic_dev, &value);
    char msg[128];
    // sprintf(msg, "fdc1004_meas_data %s, value=%f", res_msg(res == RT_EOK), value);
    log_debug("fdc1004_meas_data %s, value=%f", res_msg(res == RT_EOK), value);
}

// MSH_CMD_EXPORT(test_fdc1004, test fdc1004);
#endif
