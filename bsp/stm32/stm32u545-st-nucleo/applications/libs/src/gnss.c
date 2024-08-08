/*
 * @FilePath: gnss.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-30 09:24:58
 * @copyright : Copyright (c) 2024
 */
#include "gnss.h"

#define DBG_SECTION_NAME "GNSS"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

static rt_device_t gnss_serial;
static rt_uint8_t GNSS_THD_EXIT=0;
rt_mutex_t GNSS_LOCK=NULL;
static lwgps_t hgps;
static char nmea[GNSS_BUFF_SIZE];

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
    rt_thread_delay(rt_tick_from_millisecond(300)); //at least 300 ms
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
            if (rt_device_read(gnss_serial, -1, &nmea, GNSS_BUFF_SIZE) > 0)
            {
                // LOG_D(nmea);
                lwgps_process(&hgps, nmea, rt_strlen(nmea));
            }
            // else
            // {
            //     LOG_I("No gnss nmea data output.");
            // }
        }
        else
        {
            LOG_E("Take GNSS_LOCK to read nmea failed.");
        }
        rt_mutex_release(GNSS_LOCK);
        rt_thread_delay(rt_tick_from_millisecond(250)); //at least 250 ms
    }
}

rt_err_t gnss_open(void)
{
    rt_err_t ret = RT_EOK;

    GNSS_LOCK = rt_mutex_create("GNSSLK", RT_IPC_FLAG_FIFO);
    gnss_power_on();
    gnss_reset_init();
    swith_gnss_source(0);

    /* 查找系统中的串口设备 */
    gnss_serial = rt_device_find(GNSS_UART_NAME);
    if (!gnss_serial)
    {
        LOG_D("find %s failed!\n", GNSS_UART_NAME);
        return RT_ERROR;
    }

    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(gnss_serial, RT_DEVICE_FLAG_INT_RX);

    /* 创建 GNSS 线程 */
    rt_thread_t thread = rt_thread_create("GNSS", gnss_thread_entry, RT_NULL, 0x1000, 25, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        GNSS_THD_EXIT = 0;
        lwgps_init(&hgps);
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }

    return ret;
}

rt_err_t gnss_close(void)
{
    GNSS_THD_EXIT = 1;
    rt_thread_delay(rt_tick_from_millisecond(300)); //at least 250 ms
    rt_mutex_delete(GNSS_LOCK);
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
    rt_thread_delay(rt_tick_from_millisecond(100)); //at least 300 ms
    rt_uint8_t cnt = 0;
    while (cnt < 120)
    {
        LOG_D("GNSS Date: %d-%d-%d %d:%d:%d\r\n", hgps.year, hgps.month, hgps.date, 
            hgps.hours, hgps.minutes, hgps.seconds);
        LOG_D("Valid status: %d\r\n", hgps.is_valid);
        LOG_D("Latitude: %f degrees\r\n", hgps.latitude);
        LOG_D("Longitude: %f degrees\r\n", hgps.longitude);
        LOG_D("Altitude: %f meters\r\n", hgps.altitude);
        cnt++;
        rt_thread_delay(rt_tick_from_millisecond(1000)); //at least 300 ms
    }
    gnss_close();
}

MSH_CMD_EXPORT(gnss_data_show, gnss data show);
