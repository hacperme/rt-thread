/*
 * @FilePath: adxl372.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-05 19:11:21
 * @copyright : Copyright (c) 2024
 */
#include <stdlib.h>
#include <stdio.h>
#include "adxl372.h"
#include "tools.h"
#include "logging.h"

#define ADXL372_SPI_NAME "spi1"
#define ADXL372_DEV_NAME "adxl372"

struct rt_spi_device *adxl372_dev;
rt_sem_t adxl372_inact_sem = RT_NULL;
rt_thread_t adxl372_recv_inact_thd = RT_NULL;
rt_uint8_t adxl372_recv_inact_exit = 0;
rt_uint8_t XYZ_REGS[3][2] = {
    {ADI_ADXL372_X_DATA_H, ADI_ADXL372_X_DATA_L},
    {ADI_ADXL372_Y_DATA_H, ADI_ADXL372_Y_DATA_L},
    {ADI_ADXL372_Z_DATA_H, ADI_ADXL372_Z_DATA_L}
};

__weak void adxl372_inact_event_handler(void)
{
    log_debug("adxl372_inact_event_handler");
    adxl372_int1_pin_irq_disable();
    adxl372_set_standby();
    adxl372_recv_inact_event_thd_stop();
}

void adxl372_recv_inact_event(void *parameter)
{
    rt_err_t res;
    rt_uint8_t recv_buf;

    while (adxl372_recv_inact_exit == 0)
    {
        rt_sem_take(adxl372_inact_sem, RT_WAITING_FOREVER);
        log_debug("**** adxl372_recv_inact_event ****");
        recv_buf = 0xFF;
        res = adxl732_read(ADI_ADXL372_STATUS_2, &recv_buf, 1);
        log_debug(
            "ADI_ADXL372_STATUS_2 reg=0x%02X recv_buf=0x%02X",
            ADI_ADXL372_STATUS_2, recv_buf
        );
        if ((recv_buf & (1 << 4)) >> 4 == 1)
        {
            adxl372_inact_event_handler();
        }
    }
}

rt_err_t adxl372_recv_inact_event_thd_start(void)
{
    rt_err_t res = RT_ERROR;

    if (!adxl372_inact_sem)
    {
        adxl372_inact_sem = rt_sem_create("ginact", 0, RT_IPC_FLAG_PRIO);
    }
    do {
        res = rt_sem_take(adxl372_inact_sem, RT_WAITING_NO);
    }
    while (res == RT_EOK);

    adxl372_recv_inact_thd = rt_thread_create(
        "grecvin", adxl372_recv_inact_event, RT_NULL, 0x400, 10, 5
    );
    if (adxl372_recv_inact_thd != RT_NULL)
    {
        adxl372_recv_inact_exit = 0;
        res = rt_thread_startup(adxl372_recv_inact_thd);
    }
    return res;
}

rt_err_t adxl372_recv_inact_event_thd_stop(void)
{
    adxl372_recv_inact_exit = 1;
    rt_sem_release(adxl372_inact_sem);
    return RT_EOK;
}

void adxl372_inactive_irq_callback(void *args)
{
    log_debug("==== adxl372_inactive_irq_callback ====");
    rt_sem_release(adxl372_inact_sem);
}

rt_err_t adxl372_int1_pin_irq_enable(void)
{
    rt_err_t res;
    res = rt_pin_attach_irq(ADXL372_INT1_Pin, PIN_IRQ_MODE_RISING, adxl372_inactive_irq_callback, RT_NULL);
    if (res != RT_EOK)
    {
        log_error("ADXL372_INT1_Pin rt_pin_attach_irq falied. res=%d", res);
        return res;
    }
    res = rt_pin_irq_enable(ADXL372_INT1_Pin, PIN_IRQ_ENABLE);
    if (res != RT_EOK)
    {
        log_error("ADXL372_INT1_Pin rt_pin_irq_enable falied. res=%d", res);
    }
    return res;
}

rt_err_t adxl372_int1_pin_irq_disable(void)
{
    rt_err_t res;
    res = rt_pin_irq_enable(ADXL372_INT1_Pin, PIN_IRQ_DISABLE);
    if (res != RT_EOK)
    {
        log_error("ADXL372_INT1_Pin rt_pin_irq_enable falied. res=%d", res);
    }
    return res;
}

rt_err_t adxl372_enable_inactive_irq(rt_uint16_t *milliseconds, rt_uint16_t *threshold)
{
    rt_err_t res = RT_ERROR;

    res = adxl372_set_standby();
    log_debug("adxl372_set_standby %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_recv_inact_event_thd_start();
    log_debug("adxl372_recv_inact_event_thd_start %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_int1_pin_irq_enable();
    log_debug("adxl372_int1_pin_irq_enable %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_set_time_inact(milliseconds);
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_set_thresh_inact(threshold);
    if (res != RT_EOK)
    {
        return res;
    }

    /* Set inactive interupt to INT1. */
    rt_uint8_t int1map_fun = 0x10;
    res = adxl372_set_int1_map(&int1map_fun);
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_full_bandwidth_measurement_mode();
    log_debug("adxl372_full_bandwidth_measurement_mode %s", res == RT_EOK ? "success" : "failed");

    return res;
}

rt_err_t adxl372_disable_inactive_irq(void)
{
    rt_err_t res = RT_ERROR;

    res = adxl372_set_standby();
    log_debug("adxl372_set_standby %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_recv_inact_event_thd_stop();
    log_debug("adxl372_recv_inact_event_thd_stop %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_int1_pin_irq_disable();
    log_debug("adxl372_int1_pin_irq_disable %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    /* Remove inactive interupt from INT1. */
    rt_uint8_t int1map_fun = 0x00;
    res = adxl372_set_int1_map(&int1map_fun);
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_full_bandwidth_measurement_mode();
    log_debug("adxl372_full_bandwidth_measurement_mode %s", res == RT_EOK ? "success" : "failed");

    return res;
}

rt_err_t adxl372_set_measure_config(rt_uint8_t *measure_val, rt_uint8_t *odr_val, rt_uint8_t *hpf_val)
{
    rt_err_t res = RT_ERROR;

    res = adxl372_set_standby();
    log_debug("adxl372_set_standby %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    /* Set Bandwidth */
    // *measure_val = 0x04;  // 3200 Hz
    // *measure_val = 0x03;  // 1600 Hz
    // *measure_val = 0x02;  // 800 Hz
    // *measure_val = 0x00;  // 200 Hz (Default)
    res = adxl372_set_measure(measure_val);
    log_debug("adxl372_set_measure %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    /* Set ORD */
    // *odr_val = 0x80;  // 6400 Hz
    // *odr_val = 0x60;  // 3200 Hz
    // *odr_val = 0x40;  // 1600 Hz
    // *odr_val = 0x00;  // 400 Hz (Default)
    res = adxl372_set_odr(odr_val);
    log_debug("adxl372_set_odr %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    // *hpf_val = 0x03;
    res = adxl372_set_hpf(hpf_val);
    log_debug("adxl372_set_hpf %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_full_bandwidth_measurement_mode();
    log_debug("adxl372_full_bandwidth_measurement_mode %s", res == RT_EOK ? "success" : "failed");

    return res;
}

rt_err_t adxl372_init(void)
{
    rt_err_t res = RT_ERROR;

    /* G-Sensor irq pin init. */
    rt_pin_mode(ADXL372_INT1_Pin, PIN_MODE_INPUT_PULLDOWN);

    res = adxl372_dev != RT_NULL ? RT_EOK : RT_ERROR;
    if (res == RT_EOK)
    {
        return res;
    }

    // rt_device_t spi_dev = rt_device_find(ADXL372_SPI_NAME);
    // log_debug("find device %s %s.", ADXL372_SPI_NAME, spi_dev == RT_NULL ? "failed" : "success");
    // if (spi_dev == RT_NULL)
    // {
    //     return res;
    // }

    res = rt_hw_spi_device_attach(ADXL372_SPI_NAME, ADXL372_DEV_NAME, ADXL372_CS_PIN);
    log_debug("rt_hw_spi_device_attach bus name %s device name %s %s.", ADXL372_SPI_NAME, ADXL372_DEV_NAME, res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    adxl372_dev = (struct rt_spi_device *)rt_device_find(ADXL372_DEV_NAME);
    log_debug("find device %s %s.", ADXL372_DEV_NAME, (adxl372_dev == RT_NULL ? "failed" : "success"));
    if (adxl372_dev == RT_NULL)
    {
        return res;
    }

    struct rt_spi_configuration adxl372_spi_cfg = {0};
    adxl372_spi_cfg.data_width = 8;
    adxl372_spi_cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    adxl372_spi_cfg.max_hz = 5 * 1000 * 1000;
    res = rt_spi_configure(adxl372_dev, &adxl372_spi_cfg);
    log_debug("rt_spi_configure res %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    return res;
}

rt_err_t adxl732_read(rt_uint8_t reg, rt_uint8_t *data, rt_uint16_t size)
{
    rt_ssize_t res;
    rt_uint16_t buf_size = size + 1;
    rt_uint8_t send_buf[buf_size];
    rt_memset(send_buf, 0, buf_size);
    rt_uint8_t recv_buf[buf_size];
    rt_memset(recv_buf, 0xFF, buf_size);
    send_buf[0] = (reg << 1) | 0x01;
    rt_memcpy((recv_buf + 1), data, size);
    res = rt_spi_transfer(adxl372_dev, send_buf, recv_buf, buf_size);
    // for (rt_uint16_t i = 0; i < buf_size; i++)
    // {
    //     log_debug("adxl732_read send_buf[%d] 0x%02X recv_buf[%d] 0x%02X", i, send_buf[i], i, recv_buf[i]);
    // }
    rt_memcpy(data, (recv_buf + 1), size);
    return res == buf_size ? RT_EOK : RT_ERROR;
}

rt_err_t adxl732_write(rt_uint8_t reg, rt_uint8_t *data, rt_uint16_t size)
{
    rt_ssize_t res;
    rt_uint16_t buf_size = size + 1;
    rt_uint8_t send_buf[buf_size];
    rt_memset(send_buf, 0, buf_size);
    rt_uint8_t recv_buf[buf_size];
    rt_memset(recv_buf, 0xFF, buf_size);
    send_buf[0] = (reg << 1);
    rt_memcpy((send_buf + 1), data, size);
    res = rt_spi_transfer(adxl372_dev, send_buf, recv_buf, buf_size);
    // for (rt_uint16_t i = 0; i < buf_size; i++)
    // {
    //     log_debug("adxl732_read send_buf[%d] 0x%02X recv_buf[%d] 0x%02X", i, send_buf[i], i, recv_buf[i]);
    // }
    return res == buf_size ? RT_EOK : RT_ERROR;
}

rt_err_t adxl372_query_dev_info(void)
{

    rt_err_t res;
    rt_uint8_t recv_buf;
    rt_uint8_t regs[5] = {
        ADI_ADXL372_ADI_DEVID,
        ADI_ADXL372_MST_DEVID,
        ADI_ADXL372_DEVID,
        ADI_ADXL372_REVID,
        ADI_ADXL372_STATUS_1
    };

    for (rt_uint8_t i = 0; i < 5; i++)
    {

        rt_thread_mdelay(100);
        recv_buf = 0xFF;
        res = adxl732_read(regs[i], &recv_buf, 1);
        log_debug(
            "adxl372_query_dev_info %s, reg=0x%02X, recv_buf=0x%02X",
            res == RT_EOK ? "success" : "failed", regs[i], recv_buf
        );
    }

    return res;
}

rt_err_t adxl372_check_xyz_ready(void)
{
    rt_err_t res;
    rt_uint8_t recv_buf;
    rt_tick_t sticks, eticks;

    sticks = rt_tick_get_millisecond();
    do {
        recv_buf = 0x00;
        res = adxl732_read(ADI_ADXL372_STATUS_1, &recv_buf, 1);
        // log_debug("adxl732_read reg 0x%02X recv_buf 0x%02X res %d", ADI_ADXL372_STATUS_1, recv_buf, res);
        eticks = rt_tick_get_millisecond();
    } while ((recv_buf & 0x01) == 0 && (eticks - sticks) < 1000);

    res = (recv_buf & 0x01) == 0 ? RT_ERROR : RT_EOK;
}

rt_err_t adxl372_read_xyz_regs_val(rt_uint8_t XYZ_REGS_VAL[3][2])
{
    rt_err_t res;
    rt_uint8_t recv_buf;

    for (rt_uint8_t i = 0; i < 3; i++)
    {
        for (rt_uint8_t j = 0; j < 2; j++)
        {
            recv_buf = 0xFF;
            res = adxl732_read(XYZ_REGS[i][j], &recv_buf, 1);
            if (res == RT_EOK)
            {
                XYZ_REGS_VAL[i][j] = recv_buf;
            }
            else
            {
                log_error("adxl732_read reg 0x%02X failed.", XYZ_REGS[i][j]);
                break;
            }
        }
    }
    return res;
}

rt_err_t adxl372_convert_xyz_val(rt_uint8_t XYZ_REGS_VAL[3][2], rt_int16_t *XYZ_VAL)
{
    for (rt_uint8_t i = 0; i < 3; i++)
    {
        XYZ_VAL[i] = (XYZ_REGS_VAL[i][0] << 8) | XYZ_REGS_VAL[i][1];
        XYZ_VAL[i] = XYZ_VAL[i] >> 4;
    }
    return RT_EOK;
}

rt_err_t adxl372_read_xyz_val(rt_int16_t *XYZ_VAL)
{
    rt_err_t res;
    rt_uint8_t recv_buf;
    rt_uint8_t XYZ_REGS_VAL[3][2] = {0};

    res = adxl372_read_xyz_regs_val(XYZ_REGS_VAL);
    if (res == RT_EOK)
    {
        res = adxl372_convert_xyz_val(XYZ_REGS_VAL, XYZ_VAL);
    }
    // log_debug(
    //     "\r\nX H DATA 0x%02X, X L DATA 0x%02X, \r\nY L DATA 0x%02X, Y L DATA 0x%02X, \r\nZ L DATA 0x%02X, Z L DATA 0x%02X",
    //     XYZ_REGS_VAL[0][0], XYZ_REGS_VAL[0][1],
    //     XYZ_REGS_VAL[1][0], XYZ_REGS_VAL[1][1],
    //     XYZ_REGS_VAL[2][0], XYZ_REGS_VAL[2][1]
    // );
    return res;
}

rt_err_t adxl372_query_xyz(adxl372_xyz_t xyz)
{
    rt_err_t res;

    res = adxl372_check_xyz_ready();
    if (res != RT_EOK)
    {
        return res;
    }

    rt_int16_t XYZ_VAL[3] = {0};
    res = adxl372_read_xyz_val(XYZ_VAL);

    if (res == RT_EOK)
    {
        // log_debug("X VAL %d, Y VAL %d, Z VAL %d", XYZ_VAL[0], XYZ_VAL[1], XYZ_VAL[2]);
        xyz->x = XYZ_VAL[0] * ADXL372_SCALEG;
        xyz->y = XYZ_VAL[1] * ADXL372_SCALEG;
        xyz->z = XYZ_VAL[2] * ADXL372_SCALEG;
    }

    return res;
}

rt_err_t adxl372_set_measure(rt_uint8_t *val)
{
    rt_err_t res;
    res = adxl732_write(ADI_ADXL372_MEASURE, val, 1);
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_MEASURE, &recv_buf, 1);
    log_debug(
        "adxl372_set_measure reg=0x%02X val=0x%02X reread=0x%02X res=%d",
        ADI_ADXL372_MEASURE, *val, recv_buf, res
    );
    res = (recv_buf == *val ? RT_EOK : RT_ERROR);
    return res;
}

static rt_err_t adxl372_read_power_ctl(void)
{
    rt_err_t res;
    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_POWER_CTL, &recv_buf, 1);
    log_debug(
        "adxl372_set_power_ctl reg=0x%02X read=0x%02X res=%d",
        ADI_ADXL372_POWER_CTL, recv_buf, res
    );
    return res;
}

rt_err_t adxl372_set_power_ctl(rt_uint8_t *val)
{
    adxl372_read_power_ctl();
    rt_err_t res;
    res = adxl732_write(ADI_ADXL372_POWER_CTL, val, 1);
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_POWER_CTL, &recv_buf, 1);
    log_debug(
        "adxl372_set_power_ctl reg=0x%02X val=0x%02X reread=0x%02X res=%d",
        ADI_ADXL372_POWER_CTL, *val, recv_buf, res
    );
    res = (recv_buf == *val ? RT_EOK : RT_ERROR);
    return res;
}

rt_err_t adxl372_set_odr(rt_uint8_t *val)
{
    rt_err_t res;
    res = adxl732_write(ADI_ADXL372_TIMING, val, 1);
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_TIMING, &recv_buf, 1);
    log_debug(
        "adxl372_set_odr reg=0x%02X val=0x%02X reread=0x%02X res=%d",
        ADI_ADXL372_TIMING, *val, recv_buf, res
    );
    res = (recv_buf == *val ? RT_EOK : RT_ERROR);
    return res;
}

rt_err_t adxl372_set_time_inact(rt_uint16_t *milliseconds)
{
    rt_err_t res = RT_ERROR;
    rt_ssize_t send_res = 0;
    rt_uint16_t time_inact, per_code;
    rt_uint8_t time_inact_send_buf[4];
    rt_uint8_t time_inact_recv_buf[4] = {0xFF};
    time_inact_send_buf[0] = ADI_ADXL372_TIME_INACT_H;
    time_inact_send_buf[2] = ADI_ADXL372_TIME_INACT_L;
    rt_uint8_t send_buf_size = sizeof(time_inact_send_buf) / sizeof(rt_uint8_t);

    /* Get ORD. */
    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_TIMING, &recv_buf, 1);
    if (res != RT_EOK)
    {
        log_error("READ ADI_ADXL372_TIMING failed.");
        return res;
    }
    per_code = recv_buf & 0x80 ? 13 : 26;

    /* Set time inactive. */
    time_inact = *milliseconds / per_code;
    time_inact_send_buf[1] = (time_inact >> 8) & 0xFF;
    time_inact_send_buf[3] = time_inact & 0xFF;
    for (rt_uint8_t i = 0; i < send_buf_size; i += 2)
    {
        res = adxl732_write(time_inact_send_buf[i], &time_inact_send_buf[i + 1], 1);
        if (res != RT_EOK)
        {
            log_error(
                "adxl372_set_time_inact reg=0x%02X val=0x%02X failed.",
                time_inact_send_buf[i], time_inact_send_buf[i + 1]
            );
            return res;
        }
        rt_thread_mdelay(50);

        recv_buf = 0xFF;
        res = adxl732_read(time_inact_send_buf[i], &recv_buf, 1);
        log_debug(
            "adxl372_set_time_inact reg=0x%02X val=0x%02X reread=0x%02X res=%d",
            time_inact_send_buf[i], time_inact_send_buf[i + 1], recv_buf, res
        );
        res = (recv_buf == time_inact_send_buf[i + 1] ? RT_EOK : RT_ERROR);
        if (res != RT_EOK)
        {
            return res;
        }
    }

    return res;
}

rt_err_t adxl372_set_thresh_inact(rt_uint16_t *threshold)
{
    rt_err_t res = RT_ERROR;
    rt_ssize_t send_res = 0;
    rt_uint16_t thresh_inact;
    rt_uint8_t thresh_inact_h, thresh_inact_l;
    rt_uint8_t thresh_inact_recv_buff[12] = {0xFF};
    rt_uint8_t thresh_inact_send_buff[12] = {
        ADI_ADXL372_X_THRESH_INACT_H, 0x00,
        ADI_ADXL372_X_THRESH_INACT_L, 0x00,
        ADI_ADXL372_Y_THRESH_INACT_H, 0x00,
        ADI_ADXL372_Y_THRESH_INACT_L, 0x00,
        ADI_ADXL372_Z_THRESH_INACT_H, 0x00,
        ADI_ADXL372_Z_THRESH_INACT_L, 0x00,
    };
    rt_uint8_t send_buf_size = sizeof(thresh_inact_send_buff) / sizeof(rt_uint8_t);
    rt_uint8_t recv_buf = 0xFF;

    /* Set XYZ Inactive thredshold. */
    thresh_inact = ((*threshold & 0x7FF) << 5) | 0x03;
    thresh_inact_h = (thresh_inact >> 8) & 0xFF;
    thresh_inact_l = thresh_inact & 0xFF;
    for (rt_uint8_t i = 1; i < send_buf_size; i += 2)
    {
        if (i == 1 || i == 5 || i == 9)
        {
            thresh_inact_send_buff[i] = thresh_inact_h;
        }
        else
        {
            if (i == 3)
            {
                thresh_inact_send_buff[i] = thresh_inact_l;
            }
            else
            {
                thresh_inact_send_buff[i] = clr_bit(thresh_inact_l, 1);
            }
        }
    }

    for (rt_uint8_t i= 0; i < send_buf_size; i+=2)
    {
        res = adxl732_write(thresh_inact_send_buff[i], &thresh_inact_send_buff[i + 1], 1);
        if (res != RT_EOK)
        {
            log_error(
                "adxl372_set_thresh_inact reg=0x%02X val=0x%02X failed.",
                thresh_inact_send_buff[i], thresh_inact_send_buff[i + 1]
            );
            return res;
        }
        rt_thread_mdelay(50);

        recv_buf = 0xFF;
        res = adxl732_read(thresh_inact_send_buff[i], &recv_buf, 1);
        log_debug(
            "adxl372_set_thresh_inact reg=0x%02X val=0x%02X reread=0x%02X res=%d",
            thresh_inact_send_buff[i], thresh_inact_send_buff[i + 1], recv_buf, res
        );
        // res = (recv_buf == thresh_inact_send_buff[i + 1] ? RT_EOK : RT_ERROR);
        // if (res != RT_EOK)
        // {
        //     return res;
        // }
    }

    return res;
}

rt_err_t adxl372_set_int1_map(rt_uint8_t *val)
{
    rt_err_t res = RT_ERROR;
    res = adxl732_write(ADI_ADXL372_INT1_MAP, val, 1);
    if (res != RT_EOK)
    {
        log_error("write ADI_ADXL372_INT1_MAP val=0x%02X failed.", *val);
        return res;
    }
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_INT1_MAP, &recv_buf, 1);
    log_debug("adxl372_set_int1_map reg=0x%02X val=0x%02X reread=0x%02X res=%d", ADI_ADXL372_INT1_MAP, *val, recv_buf, res);
    res = (recv_buf == *val ? RT_EOK : RT_ERROR);
    return res;
}

rt_err_t adxl372_set_hpf(rt_uint8_t *val)
{
    rt_err_t res = RT_ERROR;
    res = adxl732_write(ADI_ADXL372_HPF, val, 1);
    if (res != RT_EOK)
    {
        log_error("write ADI_ADXL372_HPF val=0x%02X failed.", *val);
        return res;
    }
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_HPF, &recv_buf, 1);
    log_debug("adxl372_set_hpf reg=0x%02X val=0x%02X reread=0x%02X res=%d", ADI_ADXL372_HPF, *val, recv_buf, res);
    res = (recv_buf == *val ? RT_EOK : RT_ERROR);
    return res;
}

rt_err_t adxl372_reset(void)
{
    rt_err_t res;
    rt_uint8_t val = 0x52;
    res = adxl732_write(ADI_ADXL372_TIMING, &val, 1);
    rt_thread_mdelay(1000);
    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_TIMING, &recv_buf, 1);
    log_debug("adxl372_reset reg=0x%02X val=0x%02X reread=0x%02X, res=%d", ADI_ADXL372_TIMING, val, recv_buf, res);
    res = (recv_buf == val ? RT_EOK : RT_ERROR);
    return res;
}

rt_err_t adxl372_set_standby(void)
{
    rt_uint8_t val = 0x00;
    return adxl372_set_power_ctl(&val);
}

rt_err_t adxl372_full_bandwidth_measurement_mode(void)
{
    rt_uint8_t val = 0x03;
    return adxl372_set_power_ctl(&val);
}

rt_err_t adxl372_measure_acc(float acc_xyz_buff[][3], rt_uint16_t size)
{
    rt_err_t res;
    rt_int16_t *acc_xyz_int16 = (rt_int16_t *)acc_xyz_buff;
    rt_uint16_t i, j;

    rt_tick_t stime = rt_tick_get_millisecond();
    for (i = 0; i < size; i++)
    {
        res = adxl372_check_xyz_ready();
        if (res != RT_EOK)
        {
            log_error("adxl372_check_xyz_ready acc is not ready.");
            return res;
        }
        res = adxl372_read_xyz_val(&acc_xyz_int16[i * 6]);
        if (res != RT_EOK)
        {
            log_error("adxl372_read_xyz_val failed.");
            return res;
        }
        acc_xyz_int16[i * 6 + 4] = acc_xyz_int16[i * 6 + 2];
        acc_xyz_int16[i * 6 + 2] = acc_xyz_int16[i * 6 + 1];
    }
    rt_tick_t etime = rt_tick_get_millisecond();
    rt_tick_t rtime = rt_tick_diff(stime, etime);
    log_debug("Start %d, End %d, Run %d", stime, etime, rtime);

    // char msg[64];
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < 3; j++)
        {
            // log_debug("acc_xyz_int16[%d]=%d",(i * 3 + j) * 2, acc_xyz_int16[(i * 3 + j) * 2]);
            acc_xyz_buff[i][j] = (float)acc_xyz_int16[(i * 3 + j) * 2] * ADXL372_SCALEG;
            // sprintf(msg, "*acc_xyz_buff[%d][%d]=%f", i, j, acc_xyz_buff[i][j]);
            // log_debug(msg);
        }
    }

    return res;
}

#ifdef RT_USING_MSH
// #define TEST_ADXL372_MEASURE_FUN
#ifdef TEST_ADXL372_MEASURE_FUN
static float ACC_XYZ_BUFF[100][3] = {0};
static void test_adxl372_measure(void)
{
    rt_err_t res;
    char msg[64];
    rt_uint16_t size = 100;
    res = adxl372_measure_acc(ACC_XYZ_BUFF, size);
    log_debug("adxl372_measure_acc %s", res == RT_EOK ? "success" : "failed");
    if (res == RT_EOK)
    {
        for (rt_uint16_t i = 0; i < size; i++)
        {
            rt_memset(msg, 0, 64);
            sprintf(msg, "X=%f, Y=%f, Z=%f", ACC_XYZ_BUFF[i][0], ACC_XYZ_BUFF[i][1], ACC_XYZ_BUFF[i][2]);
            log_debug(msg);
        }
    }
}
#endif

#include "board_pin.h"
static void test_adxl372(int argc, char **argv)
{
    rt_err_t res;

    /* Config args init. */
    rt_uint16_t milliscond = 520;
    rt_uint16_t threshold = 10;  // 0.1 g
    rt_uint8_t measure_val = 0x00;
    rt_uint8_t odr_val = 0x60;
    rt_uint8_t hpf_val = 0x03;
    rt_uint8_t run_always = 1;
    rt_uint8_t inact_enable = 0;
    rt_uint8_t set_config = 1;
    rt_uint8_t set_reset = 0;
    if (argc >= 10)
    {
        milliscond = atoi(argv[1]);
        threshold = atoi(argv[2]);
        measure_val = atoi(argv[3]);
        odr_val = atoi(argv[4]);
        hpf_val = atoi(argv[5]);
        run_always = atoi(argv[6]);
        inact_enable = atoi(argv[7]);
        set_config = atoi(argv[8]);
        set_reset = atoi(argv[9]);
    }

    res = sensor_pwron_pin_enable(1);
    log_debug("sensor_pwron_pin_enable(1) %s", res != RT_EOK ? "failed" : "success");
    res = adxl372_init();
    log_debug("adxl372_init %s", res != RT_EOK ? "failed" : "success");
    res = adxl372_query_dev_info();
    if (set_reset == 1)
    {
        res = adxl372_reset();
        log_debug("adxl372_reset %s", res != RT_EOK ? "failed" : "success");
    }

    if (set_config == 1)
    {
        res = adxl372_set_measure_config(&measure_val, &odr_val, &hpf_val);
        log_debug(
            "adxl372_set_measure_config(measure_val=0x%02X, odr_val=0x%02X, hpf_val=0x%02X) %s",
            measure_val, odr_val, hpf_val, res != RT_EOK ? "failed" : "success"
        );
    }

    if (inact_enable == 1)
    {
        res = adxl372_enable_inactive_irq(&milliscond, &threshold);
        log_debug(
            "adxl372_enable_inactive_irq(milliscond=%d, threshold=%d) %s",
            milliscond, threshold, res != RT_EOK ? "failed" : "success"
        );
    }

    res = adxl372_query_dev_info();
    if (run_always == 1)
    {
        rt_uint8_t recv_buf;
        struct adxl372_xyz xyz = {0};
        char msg[128];

        // while (adxl372_recv_inact_exit == 0)
        while (1)
        {
            res = adxl372_query_xyz(&xyz);
            if (res == RT_EOK)
            {
                sprintf(msg, "zyx.x %f, zyx.y %f, zyx.z %f", xyz.x, xyz.y, xyz.z);
                log_debug(msg);
            }
            else
            {
                log_debug("adxl372_query_xyz failed.");
            }

            /* Inactive irq check. */
            // recv_buf = 0xFF;
            // res = adxl732_read(ADI_ADXL372_STATUS_2, &recv_buf, 1);
            // log_debug(
            //     "ADI_ADXL372_STATUS_2 reg=0x%02X recv_buf=0x%02X",
            //     ADI_ADXL372_STATUS_2, recv_buf
            // );

            rt_thread_mdelay(100);
        }
    }
    else
    {
    #ifdef TEST_ADXL372_MEASURE_FUN
        test_adxl372_measure();
    #endif
        return;
    }
}

// MSH_CMD_EXPORT(test_adxl372, test adxl372);
#endif