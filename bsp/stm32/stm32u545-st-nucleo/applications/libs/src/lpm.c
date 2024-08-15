/*
 * @FilePath: lpm.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-31 13:45:56
 * @copyright : Copyright (c) 2024
 */
#include "lpm.h"

#define DBG_SECTION_NAME "LPM"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

rt_device_t rtc_dev;

void rtc_wakeup_irq_enable(void)
{
    /* RTC wakeup pin irq enable. */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_3);
}

rt_err_t nbiot_power_on(void)
{
    return nbiot_pwron_pin_enable(1);
}

rt_err_t nbiot_power_off(void)
{
    return nbiot_pwron_pin_enable(0);
}

rt_err_t esp32_power_on(void)
{
    return esp32_pwron_pin_enable(1);
}

rt_err_t esp32_power_off(void)
{
    return esp32_pwron_pin_enable(0);
}

rt_err_t esp32_en_on(void)
{
    return esp32_en_pin_enable(1);
}

rt_err_t esp32_en_off(void)
{
    return esp32_en_pin_enable(0);
}

rt_err_t cat1_power_on(void)
{
    return cat1_pwron_pin_enable(1);
}

rt_err_t cat1_power_off(void)
{
    return cat1_pwron_pin_enable(0);
}

void shut_down(void)
{
    rt_err_t res;
    /* Wakup irq enable. */
    res = pwrctrl_pwr_wkup3_irq_enable();
    LOG_D("pwrctrl_pwr_wkup3_irq_enable %s.", res != RT_EOK ? "failed" : "success");
    rtc_wakeup_irq_enable();
    // test_button_wakeup_irq_enable();

    LOG_D("Start Shut Down.");

    /* Clear all related wakeup flags*/
    __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG2);

    /* Enter the Shutdown mode */
    HAL_PWREx_EnterSHUTDOWNMode();
}

rt_uint8_t get_wakeup_source(void)
{
    rt_uint8_t source = 0;
    rt_int32_t status;
    rt_uint32_t PWR_WAKEUP_FLAGS[7] = {
        PWR_WAKEUP_FLAG1,
        PWR_WAKEUP_FLAG2,
        PWR_WAKEUP_FLAG3,
        PWR_WAKEUP_FLAG4,
        PWR_WAKEUP_FLAG5,
        PWR_WAKEUP_FLAG6,
        PWR_WAKEUP_FLAG7,
    };
    for (rt_uint8_t i = 0; i < 7; i++)
    {
        status = __HAL_PWR_GET_FLAG(PWR_WAKEUP_FLAGS[i]);
        LOG_D("__HAL_PWR_GET_FLAG(PWR_WAKEUP_FLAG%d)=%d", i + 1, status);
        if (status == 1)
        {
            status = __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAGS[i]);
            LOG_D("__HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG%d)=%d", i + 1, status);
            source |= (1 << i);
        }
    }
    /*
    GPIO Wakeup: (source & (1 << 2)) >> 2 == 1
     RTC Wakeup: (source & (1 << 6)) >> 6 == 1
    */
    return source;
}

rt_uint8_t is_pin_wakeup(const rt_uint8_t *source)
{
    return (*source & (1 << 2)) >> 2;
}

rt_uint8_t is_rtc_wakeup(const rt_uint8_t *source)
{
    return (*source & (1 << 6)) >> 6;
}

static void alarm_callback(rt_alarm_t alarm, time_t timestamp)
{
    LOG_D("user alarm callback function.");
}

rt_err_t rtc_init(void)
{
    rt_err_t res = RT_ERROR;
    rtc_dev = rt_device_find("rtc");
    LOG_D("find rtc device %s.", !rtc_dev ? "failed" : "success");
    if (!rtc_dev)
    {
        return res;
    }
    res = rt_device_open(rtc_dev, 0);
    LOG_D("open rtc device %s.", res != RT_EOK ? "failed" : "success");
    return res;
}

rt_err_t rtc_set_datetime(int year, int month, int day, int hour, int minute, int second)
{
    rt_err_t err = RT_ERROR;
    /* set time and date */
    struct tm tm_new = {0};
    time_t old = (time_t)0;
    time_t now = (time_t)0;

    tm_new.tm_year = year - 1900;
    tm_new.tm_mon = month - 1; /* .tm_min's range is [0-11] */
    tm_new.tm_mday = day;
    tm_new.tm_hour = hour;
    tm_new.tm_min = minute;
    tm_new.tm_sec = second;
    if (tm_new.tm_year <= 0)
    {
        LOG_D("year is out of range [1900-]\n");
        return err;
    }
    if (tm_new.tm_mon > 11) /* .tm_min's range is [0-11] */
    {
        LOG_D("month is out of range [1-12]\n");
        return err;
    }
    if (tm_new.tm_mday == 0 || tm_new.tm_mday > 31)
    {
        LOG_D("day is out of range [1-31]\n");
        return err;
    }
    if (tm_new.tm_hour > 23)
    {
        LOG_D("hour is out of range [0-23]\n");
        return err;
    }
    if (tm_new.tm_min > 59)
    {
        LOG_D("minute is out of range [0-59]\n");
        return err;
    }
    if (tm_new.tm_sec > 60)
    {
        LOG_D("second is out of range [0-60]\n");
        return err;
    }
    /* save old timestamp */
    err = get_timestamp(&old);
    if (err != RT_EOK)
    {
        LOG_D("Get current timestamp failed. %d\n", err);
        return err;
    }
    /* converts the local time into the calendar time. */
    now = mktime(&tm_new);
    err = set_timestamp(now);
    if (err != RT_EOK)
    {
        LOG_D("set date failed. %d\n", err);
        return err;
    }
    get_timestamp(&now); /* get new timestamp */
    LOG_D("old: %.*s", 25, ctime(&old));
    LOG_D("now: %.*s", 25, ctime(&now));
    return err;
}

rt_err_t rtc_get_datatime(void)
{
    time_t cur_time;
    struct tm *time_now;
    char buf[64];

    time(&cur_time);
    time_now = localtime(&cur_time);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", time_now);
    LOG_D("Now time %s", buf);
    return RT_EOK;
}

rt_err_t rtc_set_wakeup(time_t sleep_time)
{
    rt_err_t res = RT_ERROR;
    struct rt_alarm_setup setup;
    struct rt_alarm *alarm = RT_NULL;
    static time_t now;
    struct tm p_tm;

    now = time(NULL);
    LOG_D("now %s", ctime(&now));
    now += sleep_time;
    LOG_D("wakeup %s", ctime(&now));
    gmtime_r(&now, &p_tm);

    setup.flag = RT_ALARM_ONESHOT;            
    setup.wktime.tm_year = p_tm.tm_year;
    setup.wktime.tm_mon = p_tm.tm_mon;
    setup.wktime.tm_mday = p_tm.tm_mday;
    setup.wktime.tm_wday = p_tm.tm_wday;
    setup.wktime.tm_hour = p_tm.tm_hour;
    setup.wktime.tm_min = p_tm.tm_min;
    setup.wktime.tm_sec = p_tm.tm_sec;   

    alarm = rt_alarm_create(alarm_callback, &setup);
    res = alarm != RT_NULL ? RT_EOK : RT_ERROR;
    LOG_D("rt_alarm_create %s.", res != RT_EOK ? "failed" : "success");
    if(res == RT_EOK)
    {
        res = rt_alarm_start(alarm);
        LOG_D("rt_alarm_start %s.", res != RT_EOK ? "failed" : "success");
    }
    return res;
}

static void test_rtc(void)
{
    rt_err_t res;

    res = rtc_init();
    LOG_D("rtc_init %s", res == RT_EOK ? "success" : "failed");

    res = rtc_set_datetime(2024, 8, 10, 0, 0, 0);
    LOG_D("rtc_set_datetime %s", res == RT_EOK ? "success" : "failed");

    res = rtc_get_datatime();

    res = rtc_set_wakeup(30);

    if (res == RT_EOK)
    {
        shut_down();
    }
}

MSH_CMD_EXPORT(test_rtc, test rtc);

static void test_show_wkup_status(void)
{
    rt_uint8_t res;
    rt_uint8_t wkup_source = get_wakeup_source();
    LOG_D("get_wakeup_source=%d", wkup_source);
    res = is_pin_wakeup(&wkup_source);
    LOG_D("is_pin_wakeup res=%d", res);
    res = is_rtc_wakeup(&wkup_source);
    LOG_D("is_rtc_wakeup res=%d", res);
}

MSH_CMD_EXPORT(test_show_wkup_status, test show reset status);

static void test_rtc_wakeup(int argc, char **argv)
{
    rt_err_t res;

    res = nbiot_power_off();
    LOG_D("nbiot_power_off %s", res == RT_EOK ? "success" : "failed");

    res = cat1_power_off();
    LOG_D("cat1_power_off %s", res == RT_EOK ? "success" : "failed");

    res = gnss_close();
    LOG_D("gnss_close %s", res == RT_EOK ? "success" : "failed");

    res = sensor_pwron_pin_enable(0);
    LOG_D("sensor_pwron_pin_enable(0) %s", res == RT_EOK ? "success" : "failed");

    res = esp32_power_off();
    LOG_D("esp32_power_off %s", res == RT_EOK ? "success" : "failed");

    res = esp32_en_off();
    LOG_D("esp32_en_off %s", res == RT_EOK ? "success" : "failed");
}

MSH_CMD_EXPORT(test_rtc_wakeup, test rtc wakeup);

static void test_all_pin_enable(int argc, char **argv)
{
    rt_err_t res;

    res = nbiot_power_on();
    LOG_D("nbiot_power_on %s", res == RT_EOK ? "success" : "failed");

    res = cat1_power_on();
    LOG_D("cat1_power_on %s", res == RT_EOK ? "success" : "failed");

    res = gnss_open();
    LOG_D("gnss_open %s", res == RT_EOK ? "success" : "failed");

    res = sensor_pwron_pin_enable(1);
    LOG_D("sensor_pwron_pin_enable(1) %s", res == RT_EOK ? "success" : "failed");

    res = esp32_power_on();
    LOG_D("esp32_power_on %s", res == RT_EOK ? "success" : "failed");

    res = esp32_en_on();
    LOG_D("esp32_en_on %s", res == RT_EOK ? "success" : "failed");

}
MSH_CMD_EXPORT(test_all_pin_enable, test all pin enable);
