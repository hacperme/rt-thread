/*
 * @FilePath: gnss.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-30 09:24:58
 * @copyright : Copyright (c) 2024
 */
#include "gnss.h"
#include <stdio.h>

#define DBG_SECTION_NAME "GNSS"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

static rt_device_t gnss_serial = RT_NULL;
static rt_uint8_t GNSS_THD_EXIT=0;
rt_mutex_t GNSS_LOCK = RT_NULL;
static lwgps_t hgps;
static char nmea[GNSS_BUFF_SIZE];
static struct rt_thread gnss_read_thd;
static char gnss_read_thd_stack[0x1000];

static rt_err_t gnss_power_on(void)
{
    return gnss_pwron_pin_enable(1);
}

static rt_err_t gnss_power_off(void)
{
    return gnss_pwron_pin_enable(0);
}

static rt_err_t swith_gnss_source(rt_uint8_t mode)
{
    // mode -- 0 GNSS LC76G Enable
    // mode -- 1 GNSS EG915 Enable
    return eg915_gnssen_pin_enable(mode);
}

static rt_err_t gnss_reset_init(void)
{
    return gnss_rst_pin_enable(1);
}

rt_err_t gnss_reset(void)
{
    gnss_rst_pin_enable(0);
    rt_thread_mdelay(300); //at least 300 ms
    return gnss_rst_pin_enable(1);
}

static void gnss_thread_entry(void *parameter)
{
    rt_err_t result;
    while (GNSS_THD_EXIT == 0)
    {
        result = rt_mutex_take(GNSS_LOCK, RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            rt_memset(nmea, 0, GNSS_BUFF_SIZE);
            if (rt_device_read(gnss_serial, -1, &nmea, GNSS_BUFF_SIZE) > 0)
            {
                // LOG_D(nmea);
                lwgps_process(&hgps, nmea, rt_strlen(nmea));
            }
        }
        else
        {
            LOG_E("Take GNSS_LOCK to read nmea failed.");
        }
        rt_mutex_release(GNSS_LOCK);
        rt_thread_mdelay(500);
    }
}

rt_err_t gnss_open(void)
{
    rt_err_t res = RT_EOK;

    if (GNSS_LOCK == RT_NULL)
    {
        GNSS_LOCK = rt_mutex_create("GNSSLK", RT_IPC_FLAG_FIFO);
    }

    res = lwgps_init(&hgps) == 1 ? RT_EOK : RT_ERROR;
    LOG_D("lwgps_init %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    gnss_power_on();
    gnss_reset_init();
    swith_gnss_source(0);

    /* 查找系统中的串口设备 */
    if (gnss_serial == RT_NULL)
    {
        gnss_serial = rt_device_find(GNSS_UART_NAME);
        if (gnss_serial == RT_NULL)
        {
            LOG_D("find %s failed!\n", GNSS_UART_NAME);
            return RT_ERROR;
        }
    }

    /* 以中断接收及轮询发送模式打开串口设备 */
    res = rt_device_open(gnss_serial, RT_DEVICE_FLAG_INT_RX);
    LOG_D("rt_device_open gnss_serial %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    /* 创建 GNSS 线程 */
    res = rt_thread_init(&gnss_read_thd, "GNSSTHD", gnss_thread_entry, RT_NULL, gnss_read_thd_stack, 0x1000, 25, 10);
    LOG_D("rt_thread_init gnss_read_thd %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        rt_device_close(gnss_serial);
        return res;
    }

    GNSS_THD_EXIT = 0;
    res = rt_thread_startup(&gnss_read_thd);
    LOG_D("gnss_thread_entry start %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        rt_device_close(gnss_serial);
        GNSS_THD_EXIT = 1;
    }

    return res;
}

rt_err_t gnss_close(void)
{
    GNSS_THD_EXIT = 1;
    rt_device_close(gnss_serial);
    return gnss_power_off();
}

rt_err_t gnss_read_nmea(char *data, rt_uint32_t size) {
    rt_err_t res = RT_ERROR;
    rt_err_t result;
    result = rt_mutex_take(GNSS_LOCK, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        if (size > GNSS_BUFF_SIZE)
        {
            size = GNSS_BUFF_SIZE;
        }
        rt_memcpy(data, nmea, size);
        res = RT_EOK;
    }
    else
    {
        LOG_E("Get GNSS_LOCK to read nmea failed.");
    }
    rt_mutex_release(GNSS_LOCK);
    return res;
}

rt_err_t gnss_read_data(lwgps_t *gnss_data)
{
    rt_err_t res = RT_ERROR;
    rt_err_t result;
    result = rt_mutex_take(GNSS_LOCK, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        rt_memcpy(gnss_data, &hgps, sizeof(lwgps_t));
        res = RT_EOK;
    }
    else
    {
        LOG_E("Get GNSS_LOCK to read gnss data failed.");
    }
    rt_mutex_release(GNSS_LOCK);
    return res;
}

static void gnss_data_show(int argc, char **argv)
{
    gnss_open();
    rt_thread_mdelay(100); //at least 300 ms
    char msg[256];
    rt_uint8_t cnt = 0;
    while (cnt < 60)
    {
        sprintf(msg, "GNSS Date: %d-%d-%d %d:%d:%d", hgps.year, hgps.month, hgps.date, 
                hgps.hours, hgps.minutes, hgps.seconds);
        LOG_D(msg);
        sprintf(msg, "Valid status: %d", hgps.is_valid);
        LOG_D(msg);
        sprintf(msg, "Latitude: %f degrees", (float)hgps.latitude);
        LOG_D(msg);
        sprintf(msg, "Longitude: %f degrees", (float)hgps.longitude);
        LOG_D(msg);
        sprintf(msg, "Altitude: %f meters", (float)hgps.altitude);
        LOG_D(msg);
        cnt++;
        rt_thread_mdelay(1000); //at least 300 ms
    }
    gnss_close();
}

MSH_CMD_EXPORT(gnss_data_show, gnss data show);
