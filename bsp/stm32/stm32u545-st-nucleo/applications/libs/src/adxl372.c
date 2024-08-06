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
#define ADXL372_DEV_NAME "spi10"

#define GSEN_PWR_WKUP7            GET_PIN(E, 8)
// #define ADXL372_CS_PIN         GET_PIN(E, 9)
#define ADXL372_CS_PIN            GET_PIN(C, 9)
// #define ADXL372_INT1_Pin       GET_PIN(E, 8)
#define ADXL372_INT1_Pin          GET_PIN(B, 7)

extern rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cs_pin);

// void g_sensor_callback(void)
// {
//     LOG_D("g_sensor_callback");
// }

void g_sensor_wakeup_irq_enable(void)
{
    /* G-Sensor wakeup pin irq enable. */
    rt_pin_mode(GSEN_PWR_WKUP7, PIN_MODE_INPUT);
    rt_pin_irq_enable(GSEN_PWR_WKUP7, PIN_IRQ_ENABLE);
    // rt_pin_attach_irq(GSEN_PWR_WKUP7, PIN_IRQ_MODE_RISING_FALLING, g_sensor_callback, RT_NULL);
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
    struct rt_spi_device *dev;
    dev = (struct rt_spi_device *)rt_device_find(ADXL372_DEV_NAME);
    if (dev == RT_NULL)
    {
        LOG_D("find device %s failed.", ADXL372_DEV_NAME);
        return res;
    }

    struct rt_spi_configuration adxl372_spi_cfg = {0};
    adxl372_spi_cfg.data_width = 8;
    adxl372_spi_cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    adxl372_spi_cfg.max_hz = 3200;
    res = rt_spi_configure(dev, &adxl372_spi_cfg);
    LOG_D("rt_spi_configure res %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    rt_uint8_t send_buf[] = {0x3D, 0x40};
    rt_uint8_t recv_buf = 0xFF;

    res = rt_spi_send(dev, send_buf, 2);
    LOG_D("rt_spi_send %d", res);
    rt_thread_mdelay(100);
    res = rt_spi_recv(dev, &recv_buf, 1);
    LOG_D("rt_spi_recv %d, recv_buf 0x%02X", res, recv_buf);

    return res;
}

static void test_adxl372(int argc, char **argv)
{
    rt_hw_spi_adxl372_init();
    adxl372_init();
}

MSH_CMD_EXPORT(test_adxl372, test adxl372);
