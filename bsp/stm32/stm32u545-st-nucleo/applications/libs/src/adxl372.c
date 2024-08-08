/*
 * @FilePath: adxl372.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-05 19:11:21
 * @copyright : Copyright (c) 2024
 */
#include "adxl372.h"

#define ADXL372_SPI_NAME "spi1"
#define ADXL372_DEV_NAME "adxl372"

#ifdef SOC_STM32U545RE
#define ADXL372_CS_PIN            GET_PIN(C, 9)   // U545
#define ADXL372_INT1_Pin          GET_PIN(B, 7)   // U545
#else
#define ADXL372_CS_PIN            GET_PIN(E, 9)
#define ADXL372_INT1_Pin          GET_PIN(E, 8)
#endif

struct rt_spi_device *adxl372_dev;

extern rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cs_pin);

void g_sensor_wakeup_irq_enable(void)
{
    /* G-Sensor wakeup pin irq enable. */
    rt_pin_mode(ADXL372_INT1_Pin, PIN_MODE_INPUT);
    rt_pin_irq_enable(ADXL372_INT1_Pin, PIN_IRQ_ENABLE);
}

rt_err_t rt_hw_spi_adxl372_init(void)
{
    rt_err_t res = RT_ERROR;

    LOG_D("Start to find device %s", ADXL372_SPI_NAME);
    rt_device_t spi_dev = rt_device_find(ADXL372_SPI_NAME);
    LOG_D("find device %s %s.", ADXL372_SPI_NAME, spi_dev == RT_NULL ? "failed" : "success");

    res = rt_hw_spi_device_attach(ADXL372_SPI_NAME, ADXL372_DEV_NAME, ADXL372_CS_PIN);
    LOG_D("rt_hw_spi_device_attach bus name %s device name %s %s.", ADXL372_SPI_NAME, ADXL372_DEV_NAME, res == RT_EOK ? "success" : "failed");

    return res;
}

rt_err_t adxl372_init(void)
{
    rt_err_t res = RT_ERROR;
    rt_uint8_t measure_val, power_ctl_val;

    adxl372_dev = (struct rt_spi_device *)rt_device_find(ADXL372_DEV_NAME);
    if (adxl372_dev == RT_NULL)
    {
        LOG_D("find device %s failed.", ADXL372_DEV_NAME);
        return res;
    }

    struct rt_spi_configuration adxl372_spi_cfg = {0};
    adxl372_spi_cfg.data_width = 8;
    adxl372_spi_cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    adxl372_spi_cfg.max_hz = 5 * 1000 * 1000;
    res = rt_spi_configure(adxl372_dev, &adxl372_spi_cfg);
    LOG_D("rt_spi_configure res %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    measure_val = 0x02;
    res = adxl372_set_measure(&measure_val);
    LOG_D("adxl372_set_measure %s", res == RT_EOK ? "success" : "failed");

    power_ctl_val = 0x03;
    adxl372_set_power_ctl(&power_ctl_val);
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
        rt_thread_mdelay(100);
    } while ((recv_buf & 0x01) == 0 && cnt < 60);

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
            XYZ_ACC[i] = (float)XYZ_VAL[i] * SCALE_FACTOR * MG_TO_G;
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
    // LOG_I("X VAL %f, Y VAL %f, Z VAL %f", XYZ_VAL[0], XYZ_VAL[1], XYZ_VAL[2]);
    if (res == RT_EOK)
    {
        LOG_D("X ACC %f, Y ACC %f, Z ACC %f", XYZ_ACC[0], XYZ_ACC[1], XYZ_ACC[2]);
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
    LOG_D("adxl732_write reg 0x%02X send_buf 0x%02X res %d", ADI_ADXL372_MEASURE, val, res);
    return res;
}

rt_err_t adxl372_set_power_ctl(rt_uint8_t *val)
{
    rt_err_t res;
    res = adxl732_write(ADI_ADXL372_POWER_CTL, val, 1);
    LOG_D("adxl732_write reg 0x%02X send_buf 0x%02X res %d", ADI_ADXL372_POWER_CTL, val, res);
    return res;
}

rt_err_t adxl372_set_odr(void)
{
    rt_err_t res;
    struct rt_spi_message message = {0};

    rt_spi_take(adxl372_dev);

    rt_uint8_t send_buf1[] = {ADI_ADXL372_TIMING << 1, 0x40};
    rt_uint8_t recv_buf1[] = {0xFF, 0xFF};

    message.send_buf = send_buf1;
    message.recv_buf = recv_buf1;
    message.length = 2;
    message.cs_take = message.cs_release = 0;
    rt_spi_transfer_message(adxl372_dev, &message);
    LOG_D(
        "rt_spi_transfer_message send 0x%02X, recv 0x%02X, send 0x%02X, recv 0x%02X",
        send_buf1[0], recv_buf1[0], send_buf1[1], recv_buf1[1]
    );

    // res = rt_spi_transfer(adxl372_dev, send_buf1, RT_NULL, 2);
    // LOG_D("rt_spi_transfer reg 0x%02X send value 0x%02X res %d", ADI_ADXL372_TIMING, send_buf1[1], res);
    // rt_spi_release(adxl372_dev);
    // rt_thread_mdelay(1000);
    // rt_spi_take(adxl372_dev);
    // rt_uint8_t send_buf2[] = {ADI_ADXL372_TIMING << 1 | 0x01, 0xFF};

    rt_uint8_t send_buf2[] = {ADI_ADXL372_TIMING << 1 | 0x01, 0xFF};
    rt_uint8_t recv_buf2[] = {0xFF, 0xFF};

    message.send_buf = send_buf2;
    message.recv_buf = recv_buf2;
    message.length = 2;
    message.cs_take = message.cs_release = 0;
    rt_spi_transfer_message(adxl372_dev, &message);
    LOG_D(
        "rt_spi_transfer_message send 0x%02X, recv 0x%02X, send 0x%02X, recv 0x%02X",
        send_buf2[0], recv_buf2[0], send_buf2[1], recv_buf2[1]
    );

    // res = rt_spi_transfer(adxl372_dev, send_buf2, recv_buf2, 2);
    // LOG_D("rt_spi_transfer reg 0x%02X recv value 0x%02X res %d", ADI_ADXL372_TIMING, recv_buf, res);
    rt_spi_release(adxl372_dev);
    return res;
}

static void test_adxl372(int argc, char **argv)
{
    rt_err_t res;
    struct adxl372_xyz xyz = {0};

    rt_hw_spi_adxl372_init();
    adxl372_init();

    // res = adxl372_query_dev_info();

    rt_uint16_t cnt = 10;
    while (cnt > 0)
    {
        res = adxl372_query_xyz(&xyz);
        if (res == RT_EOK)
        {
            LOG_D("zyx.x %f, zyx.y %f, zyx.z %f", xyz.x, xyz.y, xyz.z);
        }
        cnt--;
        rt_thread_mdelay(1000);
    }

    // set_odr();
}

MSH_CMD_EXPORT(test_adxl372, test adxl372);
