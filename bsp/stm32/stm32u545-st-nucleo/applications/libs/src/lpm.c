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

rt_err_t rtc_set_datetime(rt_uint32_t year, rt_uint32_t month, rt_uint32_t day,
                          rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second)
{
    rt_err_t res = RT_ERROR;

    res = set_date(year, month, day);
    LOG_D("set_date(%d, %d, %d) %s.", year, month, day, res != RT_EOK ? "failed" : "success");
    if (res != RT_EOK)
    {
        return res;
    }

    res = set_time(hour, minute, second);
    if (res != RT_EOK)
    LOG_D("set_time(%d, %d, %d) %s.", hour, minute, second, res != RT_EOK ? "failed" : "success");
    return res;
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

    res = rtc_init();
    LOG_D("rtc_init %s", res == RT_EOK ? "success" : "failed");

    res = rtc_set_datetime(2024, 8, 10, 0, 0, 0);
    LOG_D("rtc_set_datetime %s", res == RT_EOK ? "success" : "failed");

    res = rtc_set_wakeup(60);
    LOG_D("rtc_set_wakeup(60) %s", res == RT_EOK ? "success" : "failed");

    shut_down();
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
