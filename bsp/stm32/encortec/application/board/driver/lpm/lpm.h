/*
 * @FilePath: lpm.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-31 13:46:26
 * @copyright : Copyright (c) 2024
 */
#ifndef __LPM_H__
#define __LPM_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"

void nbiot_power_pin_init(void);
rt_err_t nbiot_pwron_pin_enable(rt_uint8_t mode);
rt_err_t nbiot_power_on(void);
rt_err_t nbiot_power_off(void);

void esp32_power_pin_init(void);
rt_err_t esp32_pwron_pin_enable(rt_uint8_t mode);
rt_err_t esp32_en_pin_enable(rt_uint8_t mode);
rt_err_t esp32_download_pin_enable(rt_uint8_t mode);
rt_err_t esp32_power_on(void);
rt_err_t esp32_power_off(void);
rt_err_t esp32_start_download(void);

void cat1_power_pin_init(void);
rt_err_t cat1_pwron_pin_enable(rt_uint8_t mode);
rt_err_t cat1_pwrkey_stm_pin_enable(rt_uint8_t mode);
rt_err_t cat1_power_on(void);
rt_err_t cat1_power_off(void);

void shut_down(void);

rt_uint8_t get_reset_source(void);
rt_uint8_t get_wakeup_source(void);
rt_uint8_t is_pin_wakeup(const rt_uint8_t *source);
rt_uint8_t is_rtc_wakeup(const rt_uint8_t *source);

rt_err_t rtc_init(void);
rt_err_t rtc_set_datetime(int year, int month, int day, int hour, int minute, int second);
rt_err_t rtc_set_wakeup(time_t sleep_time);

#endif  // __LPM_H__