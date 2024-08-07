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

#define GSEN_PWR_WKUP7            GET_PIN(E, 8)
// #define ADXL372_CS_PIN         GET_PIN(E, 9)
#define ADXL372_CS_PIN            GET_PIN(C, 9)   // U545
// #define ADXL372_INT1_Pin       GET_PIN(E, 8)
#define ADXL372_INT1_Pin          GET_PIN(B, 7)   // U545

static const rt_uint8_t ANALOG_DEVICES_ID_REG = 0x00;
static const rt_uint8_t ANALOG_DEVICES_MEMS_ID_REG = 0x01;
static const rt_uint8_t DEVICE_ID_REG = 0x02;
static const rt_uint8_t PRODUCT_REVISION_ID_REG = 0x03;
static const rt_uint8_t EXTERNAL_TIMING_CONTROL_REG = 0x3D;

struct rt_spi_device *adxl372_dev;

extern rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cs_pin);

rt_err_t set_odr(void);

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
    adxl372_dev = (struct rt_spi_device *)rt_device_find(ADXL372_DEV_NAME);
    if (adxl372_dev == RT_NULL)
    {
        LOG_D("find device %s failed.", ADXL372_DEV_NAME);
        return res;
    }

    struct rt_spi_configuration adxl372_spi_cfg = {0};
    adxl372_spi_cfg.data_width = 8;
    adxl372_spi_cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_3 | RT_SPI_MSB;
    adxl372_spi_cfg.max_hz = 10 * 1000 * 1000;
    res = rt_spi_configure(adxl372_dev, &adxl372_spi_cfg);
    LOG_D("rt_spi_configure res %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    set_odr();
    return res;

    rt_uint8_t send_buf = 0;
    rt_uint8_t recv_buf = 0xFF;
    rt_uint8_t regs[4] = {
        ANALOG_DEVICES_ID_REG,
        ANALOG_DEVICES_MEMS_ID_REG,
        DEVICE_ID_REG,
        PRODUCT_REVISION_ID_REG,
    };

    for (rt_uint8_t i = 0; i < 4; i++)
    {
        send_buf = regs[i] << 1 | 0x01;
        recv_buf = 0xFF;

        // res = rt_spi_send(adxl372_dev, &send_buf, 1);
        // LOG_D("rt_spi_send reg 0x%02X, send_buf 0x%02X, %d", regs[i], send_buf, res);
        // // rt_thread_delay(rt_tick_from_millisecond(100));
        // res = rt_spi_recv(adxl372_dev, &recv_buf, 1);
        // LOG_D("rt_spi_recv %d, recv_buf 0x%02X", res, recv_buf);

        res = rt_spi_transfer(adxl372_dev, &send_buf, &recv_buf, 1);
        LOG_D("rt_spi_transfer reg 0x%02X, send_buf 0x%02X, recv_buf 0x%02X, %d", regs[i], send_buf, recv_buf, res);
    }

    return res;
}

rt_err_t set_odr(void)
{
    rt_err_t res;
    rt_uint8_t send_buf[] = {EXTERNAL_TIMING_CONTROL_REG << 1, 0x40};
    res = rt_spi_transfer(adxl372_dev, send_buf, RT_NULL, 2);
    LOG_D("rt_spi_transfer reg 0x%02X send value 0x%02X res %d", EXTERNAL_TIMING_CONTROL_REG, send_buf[1], res);
    rt_uint8_t recv_addr = EXTERNAL_TIMING_CONTROL_REG << 1 | 0x01;
    rt_uint8_t recv_buf = 0xFF;
    res = rt_spi_transfer(adxl372_dev, &recv_addr, &recv_buf, 1);
    LOG_D("rt_spi_transfer reg 0x%02X recv value 0x%02X res %d", EXTERNAL_TIMING_CONTROL_REG, recv_buf, res);
    return res;
}

static void test_adxl372(int argc, char **argv)
{
    rt_hw_spi_adxl372_init();
    adxl372_init();
}

MSH_CMD_EXPORT(test_adxl372, test adxl372);
