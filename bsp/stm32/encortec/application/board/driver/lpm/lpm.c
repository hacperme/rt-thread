/*
 * @FilePath: lpm.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-31 13:45:56
 * @copyright : Copyright (c) 2024
 */
#include "lpm.h"
#include "board_pin.h"
#include "tools.h"
#include "logging.h"

rt_device_t rtc_dev;

static void rtc_wakeup_irq_enable(void)
{
    /* RTC wakeup pin irq enable. */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_3);
}

static void pwrctrl_pwr_wkup3_irq(void *args)
{
    LOG_D("pwrctrl_pwr_wkup3_irq");
}

static rt_err_t pwrctrl_pwr_wkup3_irq_enable(void)
{
    rt_err_t res;
    rt_pin_mode(PWRCTRL_PWR_WKUP3, PIN_MODE_INPUT_PULLDOWN);
    res = rt_pin_attach_irq(PWRCTRL_PWR_WKUP3, PIN_IRQ_MODE_RISING_FALLING, pwrctrl_pwr_wkup3_irq, RT_NULL);
    LOG_D("rt_pin_attach_irq PWRCTRL_PWR_WKUP3 PIN_IRQ_MODE_RISING_FALLING res=%d", res);
    if (res != RT_EOK)
    {
        return res;
    }
    res = rt_pin_irq_enable(PWRCTRL_PWR_WKUP3, PIN_IRQ_ENABLE);
    LOG_D("rt_pin_irq_enable PWRCTRL_PWR_WKUP3 PIN_IRQ_ENABLE res=%d", res);
    /* Power harvster/tracker monitor wakeup pin irq enable. */
    if (res != RT_EOK)
    {
        return res;
    }
#ifdef SOC_STM32U545RE
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4_HIGH_2);
#else
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN3_HIGH_0);
#endif
    return res;
}

void nbiot_power_pin_init(void)
{
    rt_pin_mode(NBIOT_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t nbiot_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(NBIOT_PWRON_PIN, mode);
    return rt_pin_read(NBIOT_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

rt_err_t nbiot_power_on(void)
{
    return nbiot_pwron_pin_enable(1);
}

rt_err_t nbiot_power_off(void)
{
    return nbiot_pwron_pin_enable(0);
}

void esp32_power_pin_init(void)
{
    rt_pin_mode(ESP32_PWRON_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(ESP32_EN_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(ESP32_DOWNLOAD_PIN, PIN_MODE_OUTPUT);
}

rt_err_t esp32_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(ESP32_PWRON_PIN, mode);
    return rt_pin_read(ESP32_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

rt_err_t esp32_en_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(ESP32_EN_PIN, mode);
    return rt_pin_read(ESP32_EN_PIN) == mode ? RT_EOK : RT_ERROR;
}

rt_err_t esp32_download_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(ESP32_DOWNLOAD_PIN, mode);
    return rt_pin_read(ESP32_DOWNLOAD_PIN) == mode ? RT_EOK : RT_ERROR;
}

rt_err_t esp32_power_on(void)
{
    rt_err_t res;

    res = esp32_download_pin_enable(1);
    log_debug("esp32_download_pin_enable(1) %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }
    rt_thread_mdelay(100);

    res = esp32_pwron_pin_enable(1);
    if (res != RT_EOK)
    {
        return res;
    }
    res = esp32_en_pin_enable(1);
    return res;
}

rt_err_t esp32_power_off(void)
{
    rt_err_t res;
    res = esp32_en_pin_enable(0);
    res = esp32_pwron_pin_enable(0);
    return res;
}

rt_err_t esp32_start_download(void)
{
    rt_err_t res;
    res = esp32_power_off();
    log_debug("esp32_power_off() %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }
    rt_thread_mdelay(100);

    res = esp32_download_pin_enable(0);
    log_debug("esp32_download_pin_enable(0) %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }
    rt_thread_mdelay(100);

    res = esp32_pwron_pin_enable(1);
    if (res != RT_EOK)
    {
        return res;
    }
    res = esp32_en_pin_enable(1);
    return res;
}

void cat1_power_pin_init(void)
{
    rt_pin_mode(CAT1_PWRON_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CAT1_PWRKEY_STM_PIN, PIN_MODE_OUTPUT);
}

rt_err_t cat1_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(CAT1_PWRON_PIN, mode);
    return rt_pin_read(CAT1_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

rt_err_t cat1_pwrkey_stm_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(CAT1_PWRKEY_STM_PIN, mode);
    return rt_pin_read(CAT1_PWRKEY_STM_PIN) == mode ? RT_EOK : RT_ERROR;
}

rt_err_t cat1_power_on(void)
{
    rt_err_t res;
    res = cat1_pwron_pin_enable(1);
    // TODO: Enable this check.
    // if (res != RT_EOK)
    // {
    //     return res;
    // }
    res = cat1_pwrkey_stm_pin_enable(1);
    rt_thread_mdelay(1000);
    res = cat1_pwrkey_stm_pin_enable(0);
    return res;
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
    log_debug("pwrctrl_pwr_wkup3_irq_enable %s.", res != RT_EOK ? "failed" : "success");
    rtc_wakeup_irq_enable();
    // test_button_wakeup_irq_enable();

    log_debug("Start Shut Down.");

    /* Clear all related wakeup flags*/
    __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG2);

    /* Enter the Shutdown mode */
    HAL_PWREx_EnterSHUTDOWNMode();
}

static rt_uint8_t reset_source = 0;
static rt_uint8_t reset_source_clear = 0;
rt_uint8_t get_reset_source(void)
{
    if (reset_source_clear == 1)
    {
        return reset_source;
    }
    rt_int32_t status;
    rt_uint32_t RCC_RST_FLAGS[7] = {
        RCC_FLAG_BORRST,
        RCC_FLAG_OBLRST,
        RCC_FLAG_PINRST,
        RCC_FLAG_SFTRST,
        RCC_FLAG_IWDGRST,
        RCC_FLAG_WWDGRST,
        RCC_FLAG_LPWRRST,
    };
    for (rt_uint8_t i = 0; i < 7; i++)
    {
        status = __HAL_RCC_GET_FLAG(RCC_RST_FLAGS[i]);
        log_debug("__HAL_RCC_GET_FLAG(PWR_WAKEUP_FLAG%d)=%d", i + 1, status);
        if (status == 1)
        {
            set_bit(reset_source, i);
        }
    }
    __HAL_RCC_CLEAR_RESET_FLAGS();
    wakeup_source_clear = 1;
    // Is BORRST  -- (wakeup_source & (1 << 0)) >> 0 == 1
    // Is OBLRST  -- (wakeup_source & (1 << 1)) >> 1 == 1
    // Is PINRST  -- (wakeup_source & (1 << 2)) >> 2 == 1
    // Is SFTRST  -- (wakeup_source & (1 << 3)) >> 3 == 1
    // Is IWDGRST -- (wakeup_source & (1 << 4)) >> 4 == 1
    // Is WWDGRST -- (wakeup_source & (1 << 5)) >> 5 == 1
    // Is LPWRRST -- (wakeup_source & (1 << 6)) >> 6 == 1
    return wakeup_source;
}

static rt_uint8_t wakeup_source = 0;
static rt_uint8_t wakeup_source_clear = 0;
rt_uint8_t get_wakeup_source(void)
{
    if (wakeup_source_clear == 1)
    {
        return wakeup_source;
    }
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
        log_debug("__HAL_PWR_GET_FLAG(PWR_WAKEUP_FLAG%d)=%d", i + 1, status);
        if (status == 1)
        {
            status = __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAGS[i]);
            log_debug("__HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG%d)=%d", i + 1, status);
            set_bit(wakeup_source, i);
        }
    }
    wakeup_source_clear = 1;

    // GPIO Wakeup: (wakeup_source & (1 << 2)) >> 2 == 1
    //  RTC Wakeup: (wakeup_source & (1 << 6)) >> 6 == 1
    return wakeup_source;
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
    log_debug("user alarm callback function.");
}

rt_err_t rtc_init(void)
{
    rt_err_t res = RT_ERROR;
    rtc_dev = rt_device_find("rtc");
    log_debug("find rtc device %s.", !rtc_dev ? "failed" : "success");
    if (!rtc_dev)
    {
        return res;
    }
    res = rt_device_open(rtc_dev, 0);
    log_debug("open rtc device %s.", res != RT_EOK ? "failed" : "success");
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
        log_debug("year is out of range [1900-]\n");
        return err;
    }
    if (tm_new.tm_mon > 11) /* .tm_min's range is [0-11] */
    {
        log_debug("month is out of range [1-12]\n");
        return err;
    }
    if (tm_new.tm_mday == 0 || tm_new.tm_mday > 31)
    {
        log_debug("day is out of range [1-31]\n");
        return err;
    }
    if (tm_new.tm_hour > 23)
    {
        log_debug("hour is out of range [0-23]\n");
        return err;
    }
    if (tm_new.tm_min > 59)
    {
        log_debug("minute is out of range [0-59]\n");
        return err;
    }
    if (tm_new.tm_sec > 60)
    {
        log_debug("second is out of range [0-60]\n");
        return err;
    }
    /* save old timestamp */
    err = get_timestamp(&old);
    if (err != RT_EOK)
    {
        log_debug("Get current timestamp failed. %d\n", err);
        return err;
    }
    /* converts the local time into the calendar time. */
    now = mktime(&tm_new);
    err = set_timestamp(now);
    if (err != RT_EOK)
    {
        log_debug("set date failed. %d\n", err);
        return err;
    }
    get_timestamp(&now); /* get new timestamp */
    log_debug("old: %.*s", 25, ctime(&old));
    log_debug("now: %.*s", 25, ctime(&now));
    return err;
}

rt_err_t rtc_set_wakeup(time_t sleep_time)
{
    rt_err_t res = RT_ERROR;
    struct rt_alarm_setup setup;
    struct rt_alarm *alarm = RT_NULL;
    static time_t now;
    struct tm p_tm;

    now = time(NULL);
    log_debug("now %s, stamp=%d", ctime(&now), now);
    now += sleep_time;
    log_debug("wakeup %s", ctime(&now));
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
    log_debug("rt_alarm_create %s.", res != RT_EOK ? "failed" : "success");
    if(res == RT_EOK)
    {
        res = rt_alarm_start(alarm);
        log_debug("rt_alarm_start %s.", res != RT_EOK ? "failed" : "success");
    }
    return res;
}

#ifdef RT_USING_MSH
static rt_err_t rtc_get_datatime(void)
{
    time_t cur_time;
    struct tm *time_now;
    char buf[64];

    time_t now;
    now = time(NULL);
    log_debug("now %s, stamp=%d", ctime(&now), now);
    time(&cur_time);
    log_debug("cur_time=%d", cur_time);
    time_now = localtime(&cur_time);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", time_now);
    log_debug("Now time %s", buf);
    return RT_EOK;
}

static void test_rtc(void)
{
    rt_err_t res;

    res = rtc_init();
    log_debug("rtc_init %s", res == RT_EOK ? "success" : "failed");

    res = rtc_set_datetime(2024, 8, 10, 0, 0, 0);
    log_debug("rtc_set_datetime %s", res == RT_EOK ? "success" : "failed");
  
    res = rtc_get_datatime();

    res = rtc_set_wakeup(30);

    if (res == RT_EOK)
    {
        shut_down();
    }
}

// MSH_CMD_EXPORT(test_rtc, test rtc);

static void test_show_wkup_status(void)
{
    rt_uint8_t res;
    rt_uint8_t wkup_source = get_wakeup_source();
    log_debug("get_wakeup_source=%d", wkup_source);
    res = is_pin_wakeup(&wkup_source);
    log_debug("is_pin_wakeup res=%d", res);
    res = is_rtc_wakeup(&wkup_source);
    log_debug("is_rtc_wakeup res=%d", res);
}

// MSH_CMD_EXPORT(test_show_wkup_status, test show reset status);

static void test_esp32_download(int argc, char **argv)
{
    rt_err_t res;
    rt_uint8_t mode = 0;
    if (argc >= 2)
    {
        mode = atoi(argv[1]);
    }

    res = esp32_download_pin_enable(mode);
    log_debug("esp32_download_pin_enable(%d) %s", mode, res == RT_EOK ? "success" : "failed");

    rt_thread_mdelay(500);

    res = esp32_power_on();
    log_debug("esp32_power_on %s", res == RT_EOK ? "success" : "failed");
}

// MSH_CMD_EXPORT(test_esp32_download, test esp32 donwload);
#endif