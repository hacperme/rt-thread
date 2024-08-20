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
#include <string.h>

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

static nmea_item NMEA_ITEMS = {0};
static const char NMEA_VALID_CHAR[4] = ",A,";
static const char NMEA_START_CHAR[3] = "$G";
static const char CRLF[3] = "\r\n";
static const char NMEA_HEADRES[6][8] = {"$GNRMC", "$GNGGA", "$GNGLL", "$GNVTG", "$GNGSA", "$GPGSV"};

void gnss_parse_nmea_item(char *item)
{
    // LOG_D("gnss_parse_nmea_item %s", item);
    char *result;
    char *first_comma;
    result = rt_strstr(&item[2], NMEA_START_CHAR);
    if (result)
    {
        gnss_parse_nmea_item(&item[result - item]);
        return;
    }
    for (rt_uint8_t i = 0; i < 6; i++)
    {
        result = rt_strstr(item, NMEA_HEADRES[i]);
        if (result)
        {
            if (i == 0)
            {
                rt_strncpy(NMEA_ITEMS.GNRMC, item, rt_strlen(item));
                result = rt_strstr(&NMEA_ITEMS.GNRMC[7], NMEA_VALID_CHAR);
                if (result)
                {
                    first_comma = strchr(&NMEA_ITEMS.GNRMC[7], ',');
                    NMEA_ITEMS.is_valid = (first_comma != NULL && result == first_comma) ? 1: 0;
                }
                else
                {
                    NMEA_ITEMS.is_valid = 0;
                }
            }
            else if (i == 1)
            {
                rt_strncpy(NMEA_ITEMS.GNGGA, item, rt_strlen(item));
            }
            else if (i == 2)
            {
                rt_strncpy(NMEA_ITEMS.GNGLL, item, rt_strlen(item));
            }
            else if (i == 3)
            {
                rt_strncpy(NMEA_ITEMS.GNVTG, item, rt_strlen(item));
            }
            else if (i == 4)
            {
                if (NMEA_ITEMS.GNGSA_SIZE < 5)
                {
                    rt_strncpy(NMEA_ITEMS.GNGSA[NMEA_ITEMS.GNGSA_SIZE], item, rt_strlen(item));
                    NMEA_ITEMS.GNGSA_SIZE++;
                }
            }
            else if (i == 5)
            {
                if (NMEA_ITEMS.GPGSV_SIZE < 5)
                {
                    rt_strncpy(NMEA_ITEMS.GPGSV[NMEA_ITEMS.GPGSV_SIZE], item, rt_strlen(item));
                    NMEA_ITEMS.GPGSV_SIZE++;
                }
            }
            LOG_D("NMEA: %s", item);
            return;
        }
    }
}

#if 0
void gnss_parse_nmea_bak(char *nmea)
{
    char *result;
    char nmea_item[100];
    rt_uint32_t start_index, end_index;
    start_index = end_index = 0;

    while (1)
    {
        result = rt_strstr(&nmea[end_index], NMEA_START_CHAR);
        if (result)
        {
            start_index = result - nmea;
            // LOG_D("start_index=%d", start_index);
            result = rt_strstr(&nmea[start_index], CRLF);
            if (result)
            {
                end_index = result - nmea;
                rt_memset(nmea_item, '\0', 100);
                rt_strncpy(nmea_item, &nmea[start_index], (end_index - start_index));
                // LOG_D("end_index=%d", end_index);
                gnss_parse_nmea_item(nmea_item);
                end_index += 2;
                if (end_index >= rt_strlen(nmea))
                {
                    break;
                }
            }
            else
            {
                // LOG_I("Can not find CRLF in nmea");
                break;
            }
        }
        else
        {
            // LOG_I("Can not find %s in nmea", NMEA_START_CHAR);
            break;
        }
    }
}
#endif

void gnss_parse_nmea(char *nmea)
{
    char *nmea_item;

    rt_memset(&NMEA_ITEMS, 0, sizeof(NMEA_ITEMS));

    // This function will replace CRLF to \0\0, so if need to show nmea, should pay attention to that.
    nmea_item = strtok(nmea, CRLF);
    while (nmea_item != NULL)
    {
        gnss_parse_nmea_item(nmea_item);
        nmea_item = strtok(NULL, CRLF);
    }

}

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
                // LOG_D("===================================");
                // LOG_D("\r\n%s", nmea);
                // LOG_D("===================================");
                // LOG_D("NMEA Size %d", rt_strlen(nmea));
                lwgps_process(&hgps, nmea, rt_strlen(nmea));
                gnss_parse_nmea(nmea);
            }
        }
        else
        {
            LOG_E("Take GNSS_LOCK to read nmea failed.");
        }
        rt_mutex_release(GNSS_LOCK);
        rt_thread_mdelay(1000);
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

/*
If not necessary, not to use this function to read source nmea.
Because CRLF in nmea is all replaced to \0\0. 
*/
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

rt_err_t gnss_read_nmea_item(nmea_item_t nmea_item)
{
    rt_err_t res = RT_ERROR;
    rt_err_t result;
    result = rt_mutex_take(GNSS_LOCK, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        rt_memset(nmea_item, 0, sizeof(*nmea_item));
        rt_memcpy(nmea_item, &NMEA_ITEMS, sizeof(NMEA_ITEMS));
        res = RT_EOK;
    }
    else
    {
        LOG_E("Get GNSS_LOCK to read gnss data failed.");
    }
    rt_mutex_release(GNSS_LOCK);
    return res;
}

static rt_err_t test_show_nmea_item(nmea_item_t test_nmea_item)
{
    rt_err_t res = RT_ERROR;
    rt_err_t result;
    rt_uint8_t i;
    gnss_read_nmea_item(test_nmea_item);
    LOG_D("test_nmea_item->GNRMC: %s", test_nmea_item->GNRMC);
    LOG_D("test_nmea_item->is_valid: %d", test_nmea_item->is_valid);
    LOG_D("test_nmea_item->GNGGA: %s", test_nmea_item->GNGGA);
    LOG_D("test_nmea_item->GNGLL: %s", test_nmea_item->GNGLL);
    LOG_D("test_nmea_item->GNVTG: %s", test_nmea_item->GNVTG);
    LOG_D("test_nmea_item->GNGSA_SIZE: %d", test_nmea_item->GNGSA_SIZE);
    for (i = 0; i < test_nmea_item->GNGSA_SIZE; i++)
    {
        LOG_D("test_nmea_item->GNGSA[%d]: %s", i, test_nmea_item->GNGSA[i]);
    }
    LOG_D("test_nmea_item->GPGSV_SIZE: %d", test_nmea_item->GPGSV_SIZE);
    for (i = 0; i < test_nmea_item->GPGSV_SIZE; i++)
    {
        LOG_D("test_nmea_item->GPGSV[%d]: %s", i, test_nmea_item->GPGSV[i]);
    }
    return res;
}

static void gnss_data_show(int argc, char **argv)
{
    gnss_open();
    rt_thread_mdelay(100); //at least 300 ms
    char gnss_data[2048] = {0};
    char msg[256];
    rt_uint8_t cnt = 0;
    nmea_item test_nmea_item = {0};

    while (cnt < 60)
    {
        /* Test to show GNSS data */
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

        /* Test to show NMEA item data */
        test_show_nmea_item(&test_nmea_item);
        // rt_memset(gnss_data, 0, 2048);
        // gnss_read_nmea(gnss_data, 2048);
        // LOG_D("=============================");
        // LOG_D("\r\n%s", gnss_data);
        // LOG_D("=============================");
        cnt++;
        rt_thread_mdelay(1000); //at least 300 ms
    }
    gnss_close();
}

MSH_CMD_EXPORT(gnss_data_show, gnss data show);
