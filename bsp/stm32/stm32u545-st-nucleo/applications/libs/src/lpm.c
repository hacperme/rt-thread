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

#define SEN_PWR_WKUP7  GET_PIN(E, 8)
// #define BUTTON_PIN  GET_PIN(C, 13)
RTC_HandleTypeDef hrtc;
#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0x0F9
static void MX_RTC_Init(void);

void shut_down(void)
{
    /* Sensor wakeup pin irq enable. */
    rt_pin_mode(SEN_PWR_WKUP7, PIN_MODE_INPUT);
    rt_pin_irq_enable(SEN_PWR_WKUP7, PIN_IRQ_ENABLE);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_2);

    /* RTC wakeup pin irq enable. */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_3);

    // // /* Enable WakeUp Pin PWR_WAKEUP_PIN2 connected to PC.13 */
    // rt_pin_mode(BUTTON_PIN, PIN_MODE_INPUT);
    // rt_pin_irq_enable(BUTTON_PIN, PIN_IRQ_ENABLE);
    // HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_HIGH_1);

    LOG_D("Start Shut Down.");

    /* Clear all related wakeup flags*/
    __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG2);

    /* Enter the Shutdown mode */
    HAL_PWREx_EnterSHUTDOWNMode();
}

static void alarm_callback(rt_alarm_t alarm, time_t timestamp)
{
    LOG_D("user alarm callback function.\n");
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
    if (set_date(2024, 8, 3) != RT_EOK)
    {
        LOG_E("set_date(2024, 8, 3) failed.");
        return RT_ERROR;
    }
    LOG_D("set_date(2024, 8, 3) success.");
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
    set_rtc_wakeup(10);
    shut_down();
}
MSH_CMD_EXPORT(test_rtc_wakeup, test rtc wakeup);
