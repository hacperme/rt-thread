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

#define ADXL372_SPI_NAME                "spi1"
#define ADXL372_DEV_NAME                "adxl372"
#define ADXL372_ORD                     1600
#define ADXL372_ACQUISITION_TIME        20
#define ADXL372_FIFO_XYZ_BUFF_SIZE      (ADXL372_ORD * ADXL372_ACQUISITION_TIME)

struct rt_spi_device *adxl372_dev;
rt_sem_t adxl372_inact_sem = RT_NULL;
rt_thread_t adxl372_recv_inact_thd = RT_NULL;
rt_uint8_t adxl372_recv_inact_exit = 0;
rt_uint8_t XYZ_REGS[3][2] = {
    {ADI_ADXL372_X_DATA_H, ADI_ADXL372_X_DATA_L},
    {ADI_ADXL372_Y_DATA_H, ADI_ADXL372_Y_DATA_L},
    {ADI_ADXL372_Z_DATA_H, ADI_ADXL372_Z_DATA_L}
};

static rt_int16_t fifo_x_buf[ADXL372_FIFO_XYZ_BUFF_SIZE] = {0};
static rt_int16_t fifo_y_buf[ADXL372_FIFO_XYZ_BUFF_SIZE] = {0};
static rt_int16_t fifo_z_buf[ADXL372_FIFO_XYZ_BUFF_SIZE] = {0};
static rt_uint16_t fifo_xyz_size = 0;

static void adxl372_inact_event_handler(void)
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
        res = adxl372_read(ADI_ADXL372_STATUS_2, &recv_buf, 1);
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
    // log_debug("==== adxl372_inactive_irq_callback ====");
    rt_sem_release(adxl372_inact_sem);
}

rt_err_t adxl372_int1_pin_irq_enable(void)
{
    rt_err_t res;
    res = rt_pin_attach_irq(ADXL372_INT1_PIN, PIN_IRQ_MODE_RISING, adxl372_inactive_irq_callback, RT_NULL);
    if (res != RT_EOK)
    {
        log_error("ADXL372_INT1_PIN rt_pin_attach_irq falied. res=%d", res);
        return res;
    }
    res = rt_pin_irq_enable(ADXL372_INT1_PIN, PIN_IRQ_ENABLE);
    if (res != RT_EOK)
    {
        log_error("ADXL372_INT1_PIN rt_pin_irq_enable falied. res=%d", res);
    }
    return res;
}

rt_err_t adxl372_int1_pin_irq_disable(void)
{
    rt_err_t res;
    res = rt_pin_irq_enable(ADXL372_INT1_PIN, PIN_IRQ_DISABLE);
    if (res != RT_EOK)
    {
        log_error("ADXL372_INT1_PIN rt_pin_irq_enable falied. res=%d", res);
    }
    return res;
}

rt_err_t adxl372_enable_inactive_irq(rt_uint16_t *milliseconds, rt_uint16_t *threshold)
{
    rt_err_t res = RT_ERROR;

    res = adxl372_set_standby();
    log_debug("adxl372_set_standby %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_recv_inact_event_thd_start();
    log_debug("adxl372_recv_inact_event_thd_start %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_int1_pin_irq_enable();
    log_debug("adxl372_int1_pin_irq_enable %s", res_msg(res == RT_EOK));
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
    rt_uint8_t int1map_fun = 0b00010000;
    res = adxl372_set_int1_map(&int1map_fun);
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_full_bandwidth_measurement_mode();
    log_debug("adxl372_full_bandwidth_measurement_mode %s", res_msg(res == RT_EOK));

    return res;
}

rt_err_t adxl372_disable_inactive_irq(void)
{
    rt_err_t res = RT_ERROR;

    res = adxl372_set_standby();
    log_debug("adxl372_set_standby %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_recv_inact_event_thd_stop();
    log_debug("adxl372_recv_inact_event_thd_stop %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_int1_pin_irq_disable();
    log_debug("adxl372_int1_pin_irq_disable %s", res_msg(res == RT_EOK));
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
    log_debug("adxl372_full_bandwidth_measurement_mode %s", res_msg(res == RT_EOK));

    return res;
}

rt_err_t adxl372_set_measure_config(rt_uint8_t *measure_val, rt_uint8_t *odr_val, rt_uint8_t *hpf_val, rt_uint8_t *fifo_format, rt_uint8_t *fifo_mode, rt_uint16_t *fifo_samples)
{
    rt_err_t res = RT_ERROR;

    res = adxl372_set_standby();
    log_debug("adxl372_set_standby %s", res_msg(res == RT_EOK));
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
    log_debug("adxl372_set_measure %s", res_msg(res == RT_EOK));
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
    log_debug("adxl372_set_odr %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    // *hpf_val = 0x03;
    res = adxl372_set_hpf(hpf_val);
    log_debug("adxl372_set_hpf %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_set_fifo(fifo_format, fifo_mode, fifo_samples);
    log_debug("adxl372_set_fifo %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_full_bandwidth_measurement_mode();
    log_debug("adxl372_full_bandwidth_measurement_mode %s", res_msg(res == RT_EOK));

    return res;
}

rt_err_t adxl372_init(void)
{
    rt_err_t res = RT_ERROR;

    /* G-Sensor irq pin init. */
    rt_pin_mode(ADXL372_INT1_PIN, PIN_MODE_INPUT_PULLDOWN);

    res = adxl372_dev != RT_NULL ? RT_EOK : RT_ERROR;
    if (res == RT_EOK)
    {
        return res;
    }

    adxl372_dev = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    res = adxl372_dev != RT_NULL ? RT_EOK : RT_ERROR;
    log_debug("rt_malloc adxl372_dev %s.", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    res = rt_spi_bus_attach_device_cspin(adxl372_dev, ADXL372_DEV_NAME, ADXL372_SPI_NAME, ADXL372_CS_PIN, RT_NULL);
    log_debug("rt_spi_bus_attach_device_cspin bus name %s device name %s %s.", ADXL372_SPI_NAME, ADXL372_DEV_NAME, res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    struct rt_spi_configuration adxl372_spi_cfg = {0};
    adxl372_spi_cfg.data_width = 8;
    adxl372_spi_cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    adxl372_spi_cfg.max_hz = 5 * 1000 * 1000;
    res = rt_spi_configure(adxl372_dev, &adxl372_spi_cfg);
    log_debug("rt_spi_configure res %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    return res;
}

rt_err_t adxl372_read(rt_uint8_t reg, rt_uint8_t *data, rt_uint16_t size)
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
    //     log_debug("adxl372_read send_buf[%d] 0x%02X recv_buf[%d] 0x%02X", i, send_buf[i], i, recv_buf[i]);
    // }
    rt_memcpy(data, (recv_buf + 1), size);
    return res == buf_size ? RT_EOK : RT_ERROR;
}

rt_err_t adxl372_write(rt_uint8_t reg, rt_uint8_t *data, rt_uint16_t size)
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
    //     log_debug("adxl372_read send_buf[%d] 0x%02X recv_buf[%d] 0x%02X", i, send_buf[i], i, recv_buf[i]);
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

        // rt_thread_mdelay(100);
        recv_buf = 0xFF;
        res = adxl372_read(regs[i], &recv_buf, 1);
        log_debug(
            "adxl372_query_dev_info %s, reg=0x%02X, recv_buf=0x%02X",
            res_msg(res == RT_EOK), regs[i], recv_buf
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
        res = adxl372_read(ADI_ADXL372_STATUS_1, &recv_buf, 1);
        // log_debug("adxl372_read reg 0x%02X recv_buf 0x%02X res %d", ADI_ADXL372_STATUS_1, recv_buf, res);
        eticks = rt_tick_get_millisecond();
        if (rt_tick_diff(sticks, eticks) % 100 == 0)
        {
            rt_thread_mdelay(10);
        }
    } while ((recv_buf & 0x01) == 0 && rt_tick_diff(sticks, eticks) < 1000);

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
            res = adxl372_read(XYZ_REGS[i][j], &recv_buf, 1);
            if (res == RT_EOK)
            {
                XYZ_REGS_VAL[i][j] = recv_buf;
            }
            else
            {
                log_error("adxl372_read reg 0x%02X failed.", XYZ_REGS[i][j]);
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
    log_debug(
        "\r\nX H DATA 0x%02X, X L DATA 0x%02X, \r\nY L DATA 0x%02X, Y L DATA 0x%02X, \r\nZ L DATA 0x%02X, Z L DATA 0x%02X",
        XYZ_REGS_VAL[0][0], XYZ_REGS_VAL[0][1],
        XYZ_REGS_VAL[1][0], XYZ_REGS_VAL[1][1],
        XYZ_REGS_VAL[2][0], XYZ_REGS_VAL[2][1]
    );
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
    res = adxl372_write(ADI_ADXL372_MEASURE, val, 1);
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl372_read(ADI_ADXL372_MEASURE, &recv_buf, 1);
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
    res = adxl372_read(ADI_ADXL372_POWER_CTL, &recv_buf, 1);
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
    res = adxl372_write(ADI_ADXL372_POWER_CTL, val, 1);
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl372_read(ADI_ADXL372_POWER_CTL, &recv_buf, 1);
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
    res = adxl372_write(ADI_ADXL372_TIMING, val, 1);
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl372_read(ADI_ADXL372_TIMING, &recv_buf, 1);
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
    res = adxl372_read(ADI_ADXL372_TIMING, &recv_buf, 1);
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
        res = adxl372_write(time_inact_send_buf[i], &time_inact_send_buf[i + 1], 1);
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
        res = adxl372_read(time_inact_send_buf[i], &recv_buf, 1);
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
        res = adxl372_write(thresh_inact_send_buff[i], &thresh_inact_send_buff[i + 1], 1);
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
        res = adxl372_read(thresh_inact_send_buff[i], &recv_buf, 1);
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
    res = adxl372_write(ADI_ADXL372_INT1_MAP, val, 1);
    if (res != RT_EOK)
    {
        log_error("write ADI_ADXL372_INT1_MAP val=0x%02X failed.", *val);
        return res;
    }
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl372_read(ADI_ADXL372_INT1_MAP, &recv_buf, 1);
    log_debug("adxl372_set_int1_map reg=0x%02X val=0x%02X reread=0x%02X res=%d", ADI_ADXL372_INT1_MAP, *val, recv_buf, res);
    res = (recv_buf == *val ? RT_EOK : RT_ERROR);
    return res;
}

rt_err_t adxl372_set_hpf(rt_uint8_t *val)
{
    rt_err_t res = RT_ERROR;
    res = adxl372_write(ADI_ADXL372_HPF, val, 1);
    if (res != RT_EOK)
    {
        log_error("write ADI_ADXL372_HPF val=0x%02X failed.", *val);
        return res;
    }
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl372_read(ADI_ADXL372_HPF, &recv_buf, 1);
    log_debug("adxl372_set_hpf reg=0x%02X val=0x%02X reread=0x%02X res=%d", ADI_ADXL372_HPF, *val, recv_buf, res);
    res = (recv_buf == *val ? RT_EOK : RT_ERROR);
    return res;
}

rt_err_t adxl372_reset(void)
{
    rt_err_t res;
    rt_uint8_t val = 0x52;
    res = adxl372_write(ADI_ADXL372_TIMING, &val, 1);
    rt_thread_mdelay(1000);
    rt_uint8_t recv_buf = 0xFF;
    res = adxl372_read(ADI_ADXL372_TIMING, &recv_buf, 1);
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
    rt_tick_t stime, etime, rtime;
    rt_tick_t wstime, wetime, wrtime;

    stime = rt_tick_get_millisecond();
    for (i = 0; i < size; i++)
    {
        wstime = rt_tick_get_millisecond();
        res = adxl372_check_xyz_ready();
        wetime = rt_tick_get_millisecond();
        wrtime = rt_tick_diff(wstime, wetime);
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
        etime = rt_tick_get_millisecond();
        rtime = rt_tick_diff(stime, etime);
        if ((wrtime < 150) && rtime % 100 == 0)
        {
            rt_thread_mdelay(10);
        }
    }
    etime = rt_tick_get_millisecond();
    rtime = rt_tick_diff(stime, etime);
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

rt_err_t adxl372_set_fifo(rt_uint8_t *fifo_format, rt_uint8_t *fifo_mode, rt_uint16_t *fifo_samples)
{
    rt_err_t res = RT_ERROR;
    rt_uint8_t val, ctrl_val, smp_val;

    ctrl_val = *fifo_samples & 0xFF;
    res = adxl372_write(ADI_ADXL372_FIFO_SAMPLES, &ctrl_val, 1);
    val = 0xFF;
    res = adxl372_read(ADI_ADXL372_FIFO_SAMPLES, &val, 1);
    res == ctrl_val == val ? RT_EOK : RT_ERROR;
    log_debug(
        "Write ADI_ADXL372_FIFO_SAMPLES %s, ctrl_val=0x%02X, reread=0x%02X",
        res_msg(res == RT_EOK), ctrl_val, val
    );
    if (res != RT_EOK)
    {
        return res;
    }

    smp_val = ((*fifo_format & 7) << FIFO_CRL_FORMAT_POS) | ((*fifo_mode & 3) << FIFO_CRL_MODE_POS) | ((*fifo_samples >> 8) & 1);
    res = adxl372_write(ADI_ADXL372_FIFO_CTL, &smp_val, 1);
    val = 0xFF;
    res = adxl372_read(ADI_ADXL372_FIFO_CTL, &val, 1);
    log_debug(
        "Write ADI_ADXL372_FIFO_CTL %s, smp_val=0x%02X, reread=0x%02X",
        res_msg(res == RT_EOK), smp_val, val
    );
    res == smp_val == val ? RT_EOK : RT_ERROR;
    if (res != RT_EOK)
    {
        return res;
    }

    return res;
}

rt_err_t adxl372_read_fifo_num(rt_uint16_t *fifo_num)
{
    rt_err_t res;

    rt_uint8_t fifo_num_m = 0xFF;
    res = adxl372_read(ADI_ADXL372_FIFO_ENTRIES_2, &fifo_num_m, 1);
    // log_debug("Read ADI_ADXL372_FIFO_ENTRIES_2 %s, fifo_num_m=0x%02x", res_msg(res == RT_EOK), fifo_num_m);
    if (res != RT_EOK)
    {
        return res;
    }

    rt_uint8_t fifo_num_l = 0xFF;
    res = adxl372_read(ADI_ADXL372_FIFO_ENTRIES_1, &fifo_num_l, 1);
    // log_debug("Read ADI_ADXL372_FIFO_ENTRIES_1 %s, fifo_num_l=0x%02x", res_msg(res == RT_EOK), fifo_num_l);
    if (res != RT_EOK)
    {
        return res;
    }

    *fifo_num = ((fifo_num_m & 3) << 8) | fifo_num_l;

    return res;
}

rt_err_t adxl372_read_fifo_data(rt_int16_t *fifo_data)
{
    rt_err_t res;

    rt_uint8_t recv_buf[2] = {0xFF};

    res = adxl372_read(ADI_ADXL372_FIFO_DATA, recv_buf, 2);
    if (res != RT_EOK)
    {
        return res;
    }

    *fifo_data = (rt_int16_t)(recv_buf[0] << 8) | (recv_buf[1] & 0xF0);
    *fifo_data >>= 4;

    return res;
}

rt_err_t adxl372_read_fifo_xyz(rt_int16_t **x_buff, rt_int16_t **y_buff, rt_int16_t **z_buff, rt_uint16_t *xyz_size)
{
    rt_err_t res;
    rt_uint16_t fifo_num = 0;
    rt_uint16_t i;
    rt_uint8_t j;
    rt_int16_t fifo_data;
    rt_tick_t sticks, eticks;
    rt_uint8_t recv_buf;
    rt_err_t group_read_res;

    do {
        rt_thread_mdelay(10);
        recv_buf = 0xFF;
        res = adxl372_read(ADI_ADXL372_STATUS_1, &recv_buf, 1);
    } while (((recv_buf & 2) >> 1) == 0);

    sticks = rt_tick_get_millisecond();
    while (fifo_xyz_size < ADXL372_FIFO_XYZ_BUFF_SIZE && adxl372_recv_inact_exit == 0)
    {
        res = adxl372_read_fifo_num(&fifo_num);
        // log_debug("fifo_num=%d", fifo_num);
        if (fifo_num > 12)
        {
            fifo_num = (fifo_num / 6 - 1);
            for(i = 0; i < fifo_num; i++)
            {
                group_read_res = RT_EOK;
                for(j = 0; j < 3; j++)
                {
                    fifo_data = 0xFF;
                    res = adxl372_read_fifo_data(&fifo_data);
                    if (group_read_res != RT_EOK)
                    {
                        continue;
                    }
                    group_read_res = res;
                    if (res == RT_EOK)
                    {
                        switch (j)
                        {
                        case 0:
                            fifo_x_buf[fifo_xyz_size] = fifo_data;
                            break;
                        case 1:
                            fifo_y_buf[fifo_xyz_size] = fifo_data;
                            break;
                        case 2:
                            fifo_z_buf[fifo_xyz_size] = fifo_data;
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        fifo_x_buf[fifo_xyz_size] = 0;
                        fifo_y_buf[fifo_xyz_size] = 0;
                        fifo_z_buf[fifo_xyz_size] = 0;
                    }
                }
                if (group_read_res == RT_EOK)
                {
                    fifo_xyz_size++;
                }
                if (fifo_xyz_size >= ADXL372_FIFO_XYZ_BUFF_SIZE)
                {
                    break;
                }
            }
        }
        if (fifo_xyz_size < ADXL372_FIFO_XYZ_BUFF_SIZE)
        {
            rt_thread_mdelay(10);
        }
    }
    eticks = rt_tick_get_millisecond();
    log_debug("runtime=%d, fifo_xyz_size=%d", rt_tick_diff(sticks, eticks), fifo_xyz_size);
    if (adxl372_recv_inact_exit == 0)
    {
        adxl372_inact_event_handler();
    }

    *x_buff = (rt_int16_t *)fifo_x_buf;
    *y_buff = (rt_int16_t *)fifo_y_buf;
    *z_buff = (rt_int16_t *)fifo_z_buf;
    *xyz_size = fifo_xyz_size;

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
    log_debug("adxl372_measure_acc %s", res_msg(res == RT_EOK));
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

void test_adxl372(void)
{
    rt_err_t res;

    /* Config args init. */
    rt_uint16_t milliscond = 520;
    rt_uint16_t threshold = 10;  // 0.1 g
    rt_uint8_t measure_val = 0x00;
    rt_uint8_t odr_val = 0x40;
    rt_uint8_t hpf_val = 0x03;
    rt_uint8_t run_always = 0;
    rt_uint8_t inact_enable = 1;
    rt_uint8_t set_config = 1;
    rt_uint8_t set_reset = 0;
    rt_uint8_t fifo_format = 0;
    rt_uint8_t fifo_mode = 1;
    rt_uint16_t fifo_samples = 170;

    rt_pin_mode(SENSOR_PWRON_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SENSOR_PWRON_PIN, 1);

    res = adxl372_init();
    log_debug("adxl372_init %s", res_msg(res == RT_EOK));
    res = adxl372_query_dev_info();
    if (set_reset == 1)
    {
        res = adxl372_reset();
        log_debug("adxl372_reset %s", res_msg(res == RT_EOK));
    }

    if (set_config == 1)
    {
        res = adxl372_set_measure_config(&measure_val, &odr_val, &hpf_val, &fifo_format, &fifo_mode, &fifo_samples);
        log_debug(
            "adxl372_set_measure_config(measure_val=0x%02X, odr_val=0x%02X, hpf_val=0x%02X) %s",
            measure_val, odr_val, hpf_val, res_msg(res == RT_EOK)
        );
    }

    if (inact_enable == 1)
    {
        res = adxl372_enable_inactive_irq(&milliscond, &threshold);
        log_debug(
            "adxl372_enable_inactive_irq(milliscond=%d, threshold=%d) %s",
            milliscond, threshold, res_msg(res == RT_EOK)
        );
    }

    rt_int16_t *x_buf, *y_buf, *z_buf;
    rt_uint16_t xyz_size = 0;
    res = adxl372_read_fifo_xyz(&x_buf, &y_buf, &z_buf, &xyz_size);
    log_debug("adxl372_read_fifo_xyz res=%d, xyz_size=%d", res, xyz_size);
    for (rt_uint16_t i = 0; i < xyz_size; i++)
    {
        log_debug("X=%d, Y=%d, Z=%d", x_buf[i], y_buf[i], z_buf[i]);
    }

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
                log_debug("%s", msg);
            }
            else
            {
                log_debug("adxl372_query_xyz failed.");
            }

            /* Inactive irq check. */
            // recv_buf = 0xFF;
            // res = adxl372_read(ADI_ADXL372_STATUS_2, &recv_buf, 1);
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