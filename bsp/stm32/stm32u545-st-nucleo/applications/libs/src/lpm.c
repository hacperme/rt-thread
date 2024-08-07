/*
 * @FilePath: lpm.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-31 13:45:56
 * @copyright : Copyright (c) 2024
 */
#include "lpm.h"

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
    pwrctrl_pwr_wkup3_irq_enable();
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

rt_err_t set_rtc_wakeup(time_t sleep_time)
{
    rt_device_t dev = rt_device_find("rtc");
    if (!dev)
    {
        LOG_E("find device rtc failed.");
        return RT_ERROR;
    }
    LOG_D("find device rtc success.");
    if (rt_device_open(dev, 0) != RT_EOK)
    {
        LOG_E("open device rtc failed.");
        return RT_ERROR;
    }
    LOG_D("open device rtc success.");
    if (set_date(2024, 8, 5) != RT_EOK)
    {
        LOG_E("set_date(2024, 8, 5) failed.");
        return RT_ERROR;
    }
    LOG_D("set_date(2024, 8, 5) success.");
    if (set_time(0, 0, 0) != RT_EOK)
    {
        LOG_E("set_time(0, 0, 0) failed.");
        return RT_ERROR;
    }
    LOG_D("set_time(0, 0, 0) success.");
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
    if(alarm != RT_NULL)
    {
        LOG_D("rt_alarm_create success.");
        if (rt_alarm_start(alarm) != RT_EOK)
        {
            LOG_E("rt_alarm_start failed.");
        }
        LOG_D("rt_alarm_start success.");
    }
}

static void test_rtc_wakeup(int argc, char **argv)
{
    rt_err_t res;
    // Device power off.
    // res = nbiot_power_off();
    // LOG_D("nbiot_power_off %s", res == RT_EOK ? "success" : "failed");
    // res = cat1_power_off();
    // LOG_D("cat1_power_off %s", res == RT_EOK ? "success" : "failed");
    // res = gnss_close();
    // LOG_D("gnss_close %s", res == RT_EOK ? "success" : "failed");
    // res = all_sensors_off();
    // LOG_D("all_sensors_off %s", res == RT_EOK ? "success" : "failed");

    res = esp32_power_off();
    LOG_D("esp32_power_off %s", res == RT_EOK ? "success" : "failed");
    res = esp32_en_off();
    LOG_D("esp32_en_off %s", res == RT_EOK ? "success" : "failed");

    // set_rtc_wakeup(60);
    shut_down();
}
MSH_CMD_EXPORT(test_rtc_wakeup, test rtc wakeup);

static void test_all_pin_enable(int argc, char **argv)
{
    rt_err_t res;

    res = nbiot_power_on();
    LOG_D("nbiot_power_on %s", res == RT_EOK ? "success" : "failed");
    res = nbiot_power_off();
    LOG_D("nbiot_power_off %s", res == RT_EOK ? "success" : "failed");
    res = cat1_power_on();
    LOG_D("cat1_power_on %s", res == RT_EOK ? "success" : "failed");
    res = cat1_power_off();
    LOG_D("cat1_power_off %s", res == RT_EOK ? "success" : "failed");
    res = gnss_open();
    LOG_D("gnss_open %s", res == RT_EOK ? "success" : "failed");
    res = gnss_close();
    LOG_D("gnss_close %s", res == RT_EOK ? "success" : "failed");
    res = sensor_pwron_pin_enable(1);
    LOG_D("sensor_pwron_pin_enable(1) %s", res == RT_EOK ? "success" : "failed");
    res = sensor_pwron_pin_enable(0);
    LOG_D("sensor_pwron_pin_enable(0) %s", res == RT_EOK ? "success" : "failed");

    res = esp32_power_on();
    LOG_D("esp32_power_on %s", res == RT_EOK ? "success" : "failed");
    res = esp32_en_on();
    LOG_D("esp32_en_on %s", res == RT_EOK ? "success" : "failed");

}
MSH_CMD_EXPORT(test_all_pin_enable, test all pin enable);
