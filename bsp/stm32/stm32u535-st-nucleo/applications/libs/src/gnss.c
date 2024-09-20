/*
 * @FilePath: gnss.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-30 09:24:58
 * @copyright : Copyright (c) 2024
 */
#include <stdio.h>
#include <string.h>
#include "gnss.h"

#define DBG_SECTION_NAME "GNSS"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

static rt_device_t GNSS_SERIAL = RT_NULL;

static rt_uint8_t GNSS_INIT_OVER = 0;
static rt_uint8_t GNSS_THD_EXIT = 0;
static struct rt_mutex GNSS_LOCK;

static struct rt_semaphore GNSS_THD_SUSPEND_SEM;
static struct rt_thread GNSS_READ_THD;
static char GNSS_READ_THD_STACK[0x1000];

#ifdef PKG_USING_LWGPS
static lwgps_t HGPS;
#endif
static char nmea[GNSS_BUFF_SIZE];
static nmea_item NMEA_ITEMS = {0};
static const char NMEA_VALID_CHAR[4] = ",A,";
static const char NMEA_START_CHAR[3] = "$G";
static const char CRLF[3] = "\r\n";
static const char NMEA_HEADRES[6][8] = {"$GNRMC", "$GNGGA", "$GNGLL", "$GNVTG", "$GNGSA", "$GPGSV"};

void gnss_pwron_pin_init(void)
{
    rt_pin_mode(GNSS_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t gnss_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(GNSS_PWRON_PIN, mode);
    return rt_pin_read(GNSS_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

void gnss_rst_pin_init(void)
{
    rt_pin_mode(GNSS_RST_PIN, PIN_MODE_OUTPUT);
}

rt_err_t gnss_rst_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(GNSS_RST_PIN, mode);
    return rt_pin_read(GNSS_RST_PIN) == mode ? RT_EOK : RT_ERROR;
}

void eg915_gnssen_pin_init(void)
{
    rt_pin_mode(EG915_GNSSEN_PIN, PIN_MODE_OUTPUT);
}

rt_err_t eg915_gnssen_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(EG915_GNSSEN_PIN, mode);
    return rt_pin_read(EG915_GNSSEN_PIN) == mode ? RT_EOK : RT_ERROR;
}

static void gnss_parse_nmea_item(char *item)
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
            // LOG_D("NMEA: %s", item);
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

static void gnss_parse_nmea(char *nmea)
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

static void gnss_thread_entry(void *parameter)
{
    // LOG_D("gnss_thread_entry start.");
    rt_err_t res;
    do {
        res = rt_sem_take(&GNSS_THD_SUSPEND_SEM, RT_WAITING_NO);
    } while (res == RT_EOK);

    while (GNSS_THD_EXIT == 0)
    {
        res = rt_mutex_take(&GNSS_LOCK, 1000);
        // LOG_D("rt_mutex_take GNSS_LOCK %s", res == RT_EOK ? "success" : "failed");
        if (res == RT_EOK)
        {
            rt_memset(nmea, 0, GNSS_BUFF_SIZE);
            if (rt_device_read(GNSS_SERIAL, -1, &nmea, GNSS_BUFF_SIZE) > 0)
            {
                // LOG_D("===================================");
                // LOG_D("\r\n%s", nmea);
                // LOG_D("===================================");
                // LOG_D("NMEA Size %d", rt_strlen(nmea));
#ifdef PKG_USING_LWGPS
                rt_memset(&HGPS, 0, sizeof(HGPS));
                lwgps_process(&HGPS, nmea, rt_strlen(nmea));
#endif
                rt_memset(&NMEA_ITEMS, 0, sizeof(NMEA_ITEMS));
                gnss_parse_nmea(nmea);
            }
        }
        else
        {
            LOG_E("Take GNSS_LOCK to read nmea failed.");
        }
        res = rt_mutex_release(&GNSS_LOCK);
        // LOG_D("rt_mutex_release GNSS_LOCK %s", res == RT_EOK ? "success" : "failed");
        rt_thread_mdelay(1000);
    }
    // LOG_D("Clear nmea, HGPS, NMEA_ITEMS");
    rt_memset(nmea, 0, GNSS_BUFF_SIZE);
#ifdef PKG_USING_LWGPS
    rt_memset(&HGPS, 0, sizeof(HGPS));
#endif
    rt_memset(&NMEA_ITEMS, 0, sizeof(NMEA_ITEMS));
    rt_enter_critical();
    res = rt_sem_release(&GNSS_THD_SUSPEND_SEM);
    // LOG_D("rt_sem_release GNSS_THD_SUSPEND_SEM %s", res == RT_EOK ? "success" : "failed");
    res = rt_thread_suspend(rt_thread_self());
    // LOG_D("rt_thread_suspend rt_thread_self %s", res == RT_EOK ? "success" : "failed");
    rt_schedule();
    rt_exit_critical();
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

static rt_err_t gnss_init(void)
{
    rt_err_t res;
    res = GNSS_INIT_OVER == 1 ? RT_EOK : RT_ERROR;
    if (GNSS_INIT_OVER == 1)
    {
        return res;
    }

    gnss_pwron_pin_init();
    gnss_rst_pin_init();
    eg915_gnssen_pin_init();

    res = rt_sem_init(&GNSS_THD_SUSPEND_SEM, "gnsssem", 0, RT_IPC_FLAG_PRIO);
    if (res != RT_EOK)
    {
        LOG_E("rt_sem_init GNSS_THD_SUSPEND_SEM failed.");
        return res;
    }

    res = rt_mutex_init(&GNSS_LOCK, "GNSSLK", RT_IPC_FLAG_FIFO);
    if (res != RT_EOK)
    {
        LOG_E("rt_mutex_init GNSS_LOCK failed.");
        rt_sem_detach(&GNSS_THD_SUSPEND_SEM);
        return res;
    }

    /* 查找系统中的串口设备 */
    if (GNSS_SERIAL == RT_NULL)
    {
        GNSS_SERIAL = rt_device_find(GNSS_UART_NAME);
        res = GNSS_SERIAL == RT_NULL ? RT_ERROR : RT_EOK;
        if (res != RT_EOK)
        {
            LOG_D("find %s failed!\n", GNSS_UART_NAME);
            rt_sem_detach(&GNSS_THD_SUSPEND_SEM);
            rt_mutex_detach(&GNSS_LOCK);
            return res;
        }
    }

    GNSS_INIT_OVER = ((res == RT_EOK) ? 1 : 0);
    return res;
}

static rt_err_t gnss_deinit(void)
{
    rt_err_t res;
    res = GNSS_INIT_OVER == 0 ? RT_EOK : RT_ERROR;
    if (GNSS_INIT_OVER == 0)
    {
        return res;
    }

    res = rt_sem_detach(&GNSS_THD_SUSPEND_SEM);
    LOG_D("rt_sem_detach(&GNSS_THD_SUSPEND_SEM) %s", res == RT_EOK ? "success" : "failed");
    res = rt_mutex_detach(&GNSS_LOCK);
    LOG_D("rt_mutex_detach(&GNSS_LOCK) %s", res == RT_EOK ? "success" : "failed");
    GNSS_SERIAL = RT_NULL;
    GNSS_INIT_OVER = 0;

    return res;
}

rt_err_t gnss_open(void)
{
    rt_err_t res = RT_EOK;

    res = gnss_init();
    LOG_D("gnss_init %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

#ifdef SOC_STM32U535VE
    res = gnss_power_on();
    LOG_D("gnss_power_on %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        gnss_power_off();
        return res;
    }
    res = gnss_reset_init();
    LOG_D("gnss_reset_init %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        gnss_power_off();
        return res;
    }
    res = swith_gnss_source(0);
    LOG_D("swith_gnss_source(0) %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        gnss_power_off();
        return res;
    }
#endif

    /* 以中断接收及轮询发送模式打开串口设备 */
    res = rt_device_open(GNSS_SERIAL, RT_DEVICE_FLAG_INT_RX);
    LOG_D("rt_device_open GNSS_SERIAL %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        gnss_power_off();
        return res;
    }

    /* 创建 GNSS 线程 */
    res = rt_thread_init(&GNSS_READ_THD, "GNSSTHD", gnss_thread_entry, RT_NULL, GNSS_READ_THD_STACK, 0x1000, 25, 10);
    LOG_D("rt_thread_init GNSS_READ_THD %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        gnss_power_off();
        rt_device_close(GNSS_SERIAL);
        return res;
    }

    GNSS_THD_EXIT = 0;
    res = rt_thread_startup(&GNSS_READ_THD);
    LOG_D("gnss_thread_entry start %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        gnss_power_off();
        rt_device_close(GNSS_SERIAL);
    }

    return res;
}

rt_err_t gnss_close(void)
{
    rt_err_t res = RT_EOK;
    if (GNSS_INIT_OVER == 0)
    {
        return res;
    }

    GNSS_THD_EXIT = 1;
    if (GNSS_SERIAL != RT_NULL)
    {
        res = rt_device_close(GNSS_SERIAL);
        LOG_D("rt_device_close %s", res == RT_EOK ? "success" : "failed");
    }

    res = rt_sem_take(&GNSS_THD_SUSPEND_SEM, 2100);
    LOG_D("rt_sem_take GNSS_THD_SUSPEND_SEM %s", res == RT_EOK ? "success" : "failed");
    if (res == RT_EOK)
    {
        res = rt_thread_detach(&GNSS_READ_THD);
        LOG_D("rt_thread_detach GNSS_READ_THD %s", res == RT_EOK ? "success" : "failed");
    }

    do {
        res = rt_mutex_take(&GNSS_LOCK, RT_WAITING_NO);
        rt_mutex_release(&GNSS_LOCK);
    } while (res != RT_EOK);

    res = gnss_deinit();
    LOG_D("gnss_deinit %s", res == RT_EOK ? "success" : "failed");

#ifdef SOC_STM32U535VE
    res = gnss_power_off();
#endif

    return res;
}

rt_err_t gnss_reset(void)
{
    if (GNSS_INIT_OVER == 0)
    {
        return RT_ERROR;
    }
    gnss_rst_pin_enable(0);
    rt_thread_mdelay(1000);
    return gnss_rst_pin_enable(1);
}

/*
If not necessary, not to use this function to read source nmea.
Because CRLF in nmea is all replaced to \0\0. 
*/
rt_err_t gnss_read_nmea(char *data, rt_uint32_t size, rt_uint16_t timeout)
{
    rt_err_t res = RT_ERROR;
    if (GNSS_INIT_OVER == 0)
    {
        return res;
    }
    res = rt_mutex_take(&GNSS_LOCK, timeout);
    if (res == RT_EOK)
    {
        if (size > GNSS_BUFF_SIZE)
        {
            size = GNSS_BUFF_SIZE;
        }
        rt_memcpy(data, nmea, size);
    }
    else
    {
        LOG_E("Get GNSS_LOCK to read nmea failed.");
    }
    rt_mutex_release(&GNSS_LOCK);
    return res;
}

#ifdef PKG_USING_LWGPS
rt_err_t gnss_read_data(lwgps_t *gnss_data, rt_uint16_t timeout)
{
    rt_err_t res = RT_ERROR;
    if (GNSS_INIT_OVER == 0)
    {
        return res;
    }
    res = rt_mutex_take(&GNSS_LOCK, timeout);
    if (res == RT_EOK)
    {
        rt_memset(gnss_data, 0, sizeof(*gnss_data));
        rt_memcpy(gnss_data, &HGPS, sizeof(lwgps_t));
    }
    else
    {
        LOG_E("Get GNSS_LOCK to read gnss data failed.");
    }
    rt_mutex_release(&GNSS_LOCK);
    return res;
}
#endif

rt_err_t gnss_read_nmea_item(nmea_item_t nmea_item, rt_uint16_t timeout)
{
    rt_err_t res = RT_ERROR;
    if (GNSS_INIT_OVER == 0)
    {
        return res;
    }
    res = rt_mutex_take(&GNSS_LOCK, timeout);
    if (res == RT_EOK)
    {
        rt_memset(nmea_item, 0, sizeof(*nmea_item));
        rt_memcpy(nmea_item, &NMEA_ITEMS, sizeof(NMEA_ITEMS));
    }
    else
    {
        LOG_E("Get GNSS_LOCK to read gnss data failed.");
    }
    rt_mutex_release(&GNSS_LOCK);
    return res;
}

#ifdef RT_USING_MSH
#ifdef PKG_USING_LWGPS
static rt_err_t test_show_nmea_data(lwgps_t *gnss_data)
{
    char msg[64];
    rt_err_t res = gnss_read_data(gnss_data, 1000);
    LOG_D("gnss_read_data %s", res == RT_EOK ? "success" : "failed");
    if (res == RT_EOK)
    {
        sprintf(msg, "GNSS Date: %d-%d-%d %d:%d:%d", gnss_data->year, gnss_data->month, gnss_data->date, 
                gnss_data->hours, gnss_data->minutes, gnss_data->seconds);
        LOG_D(msg);
        sprintf(msg, "Valid status: %d", gnss_data->is_valid);
        LOG_D(msg);
        sprintf(msg, "Latitude: %f degrees", (float)gnss_data->latitude);
        LOG_D(msg);
        sprintf(msg, "Longitude: %f degrees", (float)gnss_data->longitude);
        LOG_D(msg);
        sprintf(msg, "Altitude: %f meters", (float)gnss_data->altitude);
        LOG_D(msg);
    }
    return res;
}
#endif

static rt_err_t test_show_nmea_item(nmea_item_t test_nmea_item)
{
    rt_err_t res = RT_ERROR;
    rt_err_t result;
    rt_uint8_t i;
    res = gnss_read_nmea_item(test_nmea_item, 1000);
    LOG_D("gnss_read_nmea_item %s", res == RT_EOK ? "success" : "failed");
    if (res == RT_EOK)
    {
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
    }
    return res;
}

static void test_gnss(int argc, char **argv)
{
    rt_err_t res;
    res = gnss_open();
    LOG_D("gnss_open %s", res == RT_EOK ? "success" : "failed");
    rt_thread_mdelay(100); //at least 300 ms

    rt_uint8_t cnt = 5;
#ifdef PKG_USING_LWGPS
    lwgps_t gnss_data = {0};
#endif
    nmea_item test_nmea_item = {0};

    while (cnt > 0)
    {
#ifdef PKG_USING_LWGPS
        /* Test to show GNSS data */
        test_show_nmea_data(&gnss_data);
#endif

        /* Test to show NMEA item data */
        test_show_nmea_item(&test_nmea_item);

        cnt--;
        rt_thread_mdelay(1000); //at least 300 ms
    }
    res = gnss_close();
    LOG_D("gnss_close %s", res == RT_EOK ? "success" : "failed");
}

// MSH_CMD_EXPORT(test_gnss, gnss data show);
#endif