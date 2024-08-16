/*
 * @FilePath: adxl372.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-05 19:11:21
 * @copyright : Copyright (c) 2024
 */
#include "adxl372.h"

#define DBG_SECTION_NAME "ADXL372"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define ADXL372_SPI_NAME "spi1"
#define ADXL372_DEV_NAME "adxl372"

#ifdef SOC_STM32U545RE
#define ADXL372_CS_PIN            GET_PIN(C, 9)   // U545
#define ADXL372_INT1_Pin          GET_PIN(B, 7)   // U545
// #define ADXL372_INT1_Pin          GET_PIN(A, 8)   // U545
#else
#define ADXL372_CS_PIN            GET_PIN(E, 9)
#define ADXL372_INT1_Pin          GET_PIN(E, 8)
#endif

struct rt_spi_device *adxl372_dev;
rt_sem_t adxl372_inact_sem = RT_NULL;
rt_thread_t adxl372_recv_inact_thd = RT_NULL;
rt_uint8_t adxl372_recv_inact_exit = 0;

__weak void adxl372_inact_event_handler(void)
{
    LOG_D("adxl372_inact_event_handler");
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
        LOG_D("**** adxl372_recv_inact_event ****");
        recv_buf = 0xFF;
        res = adxl732_read(ADI_ADXL372_STATUS_2, &recv_buf, 1);
        LOG_D(
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
        adxl372_inact_sem = rt_sem_create("ginact", 1, RT_IPC_FLAG_PRIO);
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
    LOG_D("==== adxl372_inactive_irq_callback ====");
    rt_sem_release(adxl372_inact_sem);
}

rt_err_t adxl372_int1_pin_irq_enable(void)
{
    rt_err_t res;
    /* G-Sensor wakeup pin irq enable. */
    rt_pin_mode(ADXL372_INT1_Pin, PIN_MODE_INPUT_PULLDOWN);
    res = rt_pin_attach_irq(ADXL372_INT1_Pin, PIN_IRQ_MODE_RISING, adxl372_inactive_irq_callback, RT_NULL);
    if (res != RT_EOK)
    {
        LOG_E("ADXL372_INT1_Pin rt_pin_attach_irq falied. res=%d", res);
        return res;
    }
    res = rt_pin_irq_enable(ADXL372_INT1_Pin, PIN_IRQ_ENABLE);
    if (res != RT_EOK)
    {
        LOG_E("ADXL372_INT1_Pin rt_pin_irq_enable falied. res=%d", res);
    }
    return res;
}

rt_err_t adxl372_int1_pin_irq_disable(void)
{
    rt_err_t res;
    res = rt_pin_irq_enable(ADXL372_INT1_Pin, PIN_IRQ_DISABLE);
    if (res != RT_EOK)
    {
        LOG_E("ADXL372_INT1_Pin rt_pin_irq_enable falied. res=%d", res);
    }
    return res;
}

rt_err_t rt_hw_spi_adxl372_init(void)
{
    rt_err_t res = RT_ERROR;

    res = adxl372_dev != RT_NULL ? RT_EOK : RT_ERROR;
    if (res == RT_EOK)
    {
        return res;
    }

    rt_device_t spi_dev = rt_device_find(ADXL372_SPI_NAME);
    LOG_D("find device %s %s.", ADXL372_SPI_NAME, spi_dev == RT_NULL ? "failed" : "success");
    if (spi_dev == RT_NULL)
    {
        return res;
    }

    res = rt_hw_spi_device_attach(ADXL372_SPI_NAME, ADXL372_DEV_NAME, ADXL372_CS_PIN);
    LOG_E("rt_hw_spi_device_attach bus name %s device name %s %s.", ADXL372_SPI_NAME, ADXL372_DEV_NAME, res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    adxl372_dev = (struct rt_spi_device *)rt_device_find(ADXL372_DEV_NAME);
    LOG_D("find device %s %s.", ADXL372_DEV_NAME, (adxl372_dev == RT_NULL ? "failed" : "success"));
    if (adxl372_dev == RT_NULL)
    {
        return res;
    }

    struct rt_spi_configuration adxl372_spi_cfg = {0};
    adxl372_spi_cfg.data_width = 8;
    adxl372_spi_cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    adxl372_spi_cfg.max_hz = 10 * 1000 * 1000;
    res = rt_spi_configure(adxl372_dev, &adxl372_spi_cfg);
    LOG_D("rt_spi_configure res %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    return res;
}

rt_err_t adxl372_init(rt_uint16_t inact_ms, rt_uint16_t inact_threshold)
{
    rt_err_t res = RT_ERROR;
    rt_uint8_t measure_val, power_ctl_val, odr_val, hpf_val;

    res = adxl372_recv_inact_event_thd_start();
    LOG_D("adxl372_recv_inact_event_thd_start %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    power_ctl_val = 0x00;
    res = adxl372_set_power_ctl(&power_ctl_val);
    LOG_D("adxl372_set_power_ctl %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_int1_pin_irq_enable();
    LOG_D("adxl372_int1_pin_irq_enable %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    /* Set Bandwidth */
    // measure_val = 0x04;  // 3200 Hz
    // measure_val = 0x03;  // 1600 Hz
    // measure_val = 0x02;  // 800 Hz
    measure_val = 0x00;  // 200 Hz (Default)
    res = adxl372_set_measure(&measure_val);
    LOG_D("adxl372_set_measure %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    /* Set ORD */
    // odr_val = 0x80;  // 6400 Hz
    // odr_val = 0x60;  // 3200 Hz
    // odr_val = 0x40;  // 1600 Hz
    odr_val = 0x00;  // 400 Hz (Default)
    res = adxl372_set_odr(&odr_val);
    LOG_D("adxl372_set_odr %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = adxl372_enable_inactive_irq(inact_ms, inact_threshold);
    LOG_D("adxl372_enable_inactive_irq %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    hpf_val = 0x03;
    res = adxl372_set_hpf(&hpf_val);
    LOG_D("adxl372_set_hpf %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    power_ctl_val = 0x03;
    res = adxl372_set_power_ctl(&power_ctl_val);
    LOG_D("adxl372_set_power_ctl %s", res == RT_EOK ? "success" : "failed");

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
    //     LOG_D("adxl732_read send_buf[%d] 0x%02X recv_buf[%d] 0x%02X", i, send_buf[i], i, recv_buf[i]);
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
    //     LOG_D("adxl732_read send_buf[%d] 0x%02X recv_buf[%d] 0x%02X", i, send_buf[i], i, recv_buf[i]);
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
        LOG_D(
            "adxl732_read %s, reg=0x%02X, recv_buf=0x%02X",
            res == RT_EOK ? "success" : "failed", regs[i], recv_buf
        );
    }

    return res;
}

rt_err_t adxl372_query_xyz(adxl372_xyz_t xyz)
{
    rt_err_t res;
    rt_uint8_t recv_buf, cnt;
    rt_uint16_t raw_x, raw_y, raw_z;
    float x, y, z;

    cnt = 0;
    do {
        recv_buf = 0x00;
        res = adxl732_read(ADI_ADXL372_STATUS_1, &recv_buf, 1);
        LOG_I("adxl732_read reg 0x%02X recv_buf 0x%02X res %d", ADI_ADXL372_STATUS_1, recv_buf, res);
        cnt++;
    } while ((recv_buf & 0x01) == 0 && cnt < 20);

    if ((recv_buf & 0x01) == 0)
    {
        return RT_ERROR;
    }

    rt_uint8_t XYZ_REGS[3][2] = {
        {ADI_ADXL372_X_DATA_H, ADI_ADXL372_X_DATA_L},
        {ADI_ADXL372_Y_DATA_H, ADI_ADXL372_Y_DATA_L},
        {ADI_ADXL372_Z_DATA_H, ADI_ADXL372_Z_DATA_L}
    };
    rt_uint8_t XYZ_REGS_VAL[3][2];
    rt_int16_t XYZ_VAL[3];
    float XYZ_ACC[3];

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
                LOG_E("adxl732_read reg 0x%02X failed.", XYZ_REGS[i][j]);
                break;
            }
        }
        if (res == RT_EOK)
        {
            XYZ_VAL[i] = (XYZ_REGS_VAL[i][0] << 8) | XYZ_REGS_VAL[i][1];
            XYZ_VAL[i] = XYZ_VAL[i] >> 4;
            XYZ_ACC[i] = (float)XYZ_VAL[i] * ADXL372_SCALEG;
        }
        else
        {
            break;
        }
    }
    // LOG_D(
    //     "\r\nX H DATA 0x%02X, X L DATA 0x%02X, \r\nY L DATA 0x%02X, Y L DATA 0x%02X, \r\nZ L DATA 0x%02X, Z L DATA 0x%02X",
    //     XYZ_REGS_VAL[0][0], XYZ_REGS_VAL[0][1],
    //     XYZ_REGS_VAL[1][0], XYZ_REGS_VAL[1][1],
    //     XYZ_REGS_VAL[2][0], XYZ_REGS_VAL[2][1]
    // );
    if (res == RT_EOK)
    {
        LOG_I("X VAL %d, Y VAL %d, Z VAL %d", XYZ_VAL[0], XYZ_VAL[1], XYZ_VAL[2]);
        // LOG_D("X ACC %f, Y ACC %f, Z ACC %f", XYZ_ACC[0], XYZ_ACC[1], XYZ_ACC[2]);
        xyz->x = XYZ_ACC[0];
        xyz->y = XYZ_ACC[1];
        xyz->z = XYZ_ACC[2];
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
    LOG_D(
        "adxl372_set_measure reg=0x%02X val=0x%02X reread=0x%02X res=%d",
        ADI_ADXL372_MEASURE, *val, recv_buf, res
    );
    return res;
}

rt_err_t adxl372_set_power_ctl(rt_uint8_t *val)
{
    rt_err_t res;
    res = adxl732_write(ADI_ADXL372_POWER_CTL, val, 1);
    rt_thread_mdelay(100);

    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_POWER_CTL, &recv_buf, 1);
    LOG_D(
        "adxl372_set_power_ctl reg=0x%02X val=0x%02X reread=0x%02X res=%d",
        ADI_ADXL372_POWER_CTL, *val, recv_buf, res
    );
    return res;
}

rt_err_t adxl372_set_odr(rt_uint8_t *val)
{
    rt_err_t res;
    res = adxl732_write(ADI_ADXL372_TIMING, val, 1);
    rt_thread_mdelay(100);
    rt_uint8_t recv_buf = 0xFF;
    res = adxl732_read(ADI_ADXL372_TIMING, &recv_buf, 1);
    LOG_D(
        "adxl372_set_odr reg=0x%02X val=0x%02X reread=0x%02X res=%d",
        ADI_ADXL372_TIMING, *val, recv_buf, res
    );
    return res;
}

rt_err_t adxl372_set_time_inact(rt_uint16_t milliseconds)
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
        LOG_E("READ ADI_ADXL372_TIMING failed.");
        return res;
    }
    per_code = recv_buf & 0x80 ? 13 : 26;

    /* Set time inactive. */
    time_inact = milliseconds / per_code;
    time_inact_send_buf[1] = (time_inact >> 8) & 0xFF;
    time_inact_send_buf[3] = time_inact & 0xFF;
    for (rt_uint8_t i = 0; i < send_buf_size; i += 2)
    {
        res = adxl732_write(time_inact_send_buf[i], &time_inact_send_buf[i + 1], 1);
        if (res != RT_EOK)
        {
            LOG_E(
                "adxl372_set_time_inact reg=0x%02X val=0x%02X failed.",
                time_inact_send_buf[i], time_inact_send_buf[i + 1]
            );
            return res;
        }
    }

    for (rt_uint8_t i = 0; i < send_buf_size; i += 2)
    {
        recv_buf = 0xFF;
        res = adxl732_read(time_inact_send_buf[i], &recv_buf, 1);
        LOG_D(
            "adxl372_set_time_inact reg=0x%02X val=0x%02X reread=0x%02X",
            time_inact_send_buf[i], time_inact_send_buf[i + 1], recv_buf
        );
    }
    return res;
}

rt_err_t adxl372_set_thresh_inact(rt_uint16_t threshold)
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

    /* Set XYZ Inactive thredshold. */
    threshold = threshold & 0x7FF;
    thresh_inact = threshold << 5 | 0x03;
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
            thresh_inact_send_buff[i] = thresh_inact_l;
        }
    }

    for (rt_uint8_t i= 0; i < send_buf_size; i+=2)
    {
        res = adxl732_write(thresh_inact_send_buff[i], &thresh_inact_send_buff[i + 1], 1);
        if (res != RT_EOK)
        {
            LOG_E(
                "adxl372_set_thresh_inact reg=0x%02X val=0x%02X failed.",
                thresh_inact_send_buff[i], thresh_inact_send_buff[i + 1]
            );
        }
    }

    rt_uint8_t recv_buf = 0xFF;
    for (rt_uint8_t i= 0; i < send_buf_size; i+=2)
    {
        recv_buf = 0xFF;
        adxl732_read(thresh_inact_send_buff[i], &recv_buf, 1);
        LOG_D(
            "adxl372_set_thresh_inact reg=0x%02X val=0x%02X reread=0x%02X",
            thresh_inact_send_buff[i], thresh_inact_send_buff[i + 1], recv_buf
        );
    }
    return res;
}

rt_err_t adxl372_set_int1_map(rt_uint8_t *val)
{
    rt_err_t res = RT_ERROR;
    res = adxl732_write(ADI_ADXL372_INT1_MAP, val, 1);
    if (res != RT_EOK)
    {
        LOG_E("write ADI_ADXL372_INT1_MAP val=0x%02X failed.", *val);
        return res;
    }
    rt_uint8_t recv_buf = 0xFF;
    adxl732_read(ADI_ADXL372_INT1_MAP, &recv_buf, 1);
    LOG_D("adxl372_set_int1_map reg=0x%02X val=0x%02X reread=0x%02X", ADI_ADXL372_INT1_MAP, *val, recv_buf);
    return res;
}

rt_err_t adxl372_set_hpf(rt_uint8_t *val)
{
    rt_err_t res = RT_ERROR;
    res = adxl732_write(ADI_ADXL372_HPF, val, 1);
    if (res != RT_EOK)
    {
        LOG_E("write ADI_ADXL372_HPF val=0x%02X failed.", *val);
        return res;
    }
    rt_uint8_t recv_buf = 0xFF;
    adxl732_read(ADI_ADXL372_HPF, &recv_buf, 1);
    LOG_D("adxl372_set_int1_map reg=0x%02X val=0x%02X reread=0x%02X", ADI_ADXL372_HPF, *val, recv_buf);
    return res;
}

rt_err_t adxl372_enable_inactive_irq(rt_uint16_t milliseconds, rt_uint16_t threshold)
{
    rt_err_t res = RT_ERROR;

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
    return res;
}

rt_err_t adxl372_reset(void)
{
    rt_err_t res;
    rt_uint8_t val = 0x52;
    res = adxl732_write(ADI_ADXL372_TIMING, &val, 1);
    rt_thread_mdelay(1000);
    return res;
}

rt_err_t adxl372_set_standby(void)
{
    rt_uint8_t val = 0x00;
    return adxl372_set_power_ctl(&val);
}

static void test_adxl372(int argc, char **argv)
{
    rt_err_t res;
    struct adxl372_xyz xyz = {0};
    rt_uint8_t recv_buf;

    res = sensor_pwron_pin_enable(1);
    LOG_D("sensor_pwron_pin_enable(1) %s", res != RT_EOK ? "failed" : "success");
    rt_hw_spi_adxl372_init();

    rt_uint16_t milliscond = 520;
    rt_uint16_t threshold = 10;  // 0.1 g
    adxl372_init(milliscond, threshold);

    res = adxl372_query_dev_info();

    // rt_uint16_t cnt = 10 * 60;
    while (adxl372_recv_inact_exit == 0)
    {
        res = adxl372_query_xyz(&xyz);
        if (res == RT_EOK)
        {
            LOG_D("zyx.x %f, zyx.y %f, zyx.z %f", xyz.x, xyz.y, xyz.z);
        }
        // cnt--;
        rt_thread_mdelay(10);
    }
}

MSH_CMD_EXPORT(test_adxl372, test adxl372);
