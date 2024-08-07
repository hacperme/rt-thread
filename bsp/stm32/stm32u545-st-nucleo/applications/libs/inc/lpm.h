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
#include "board_pin.h"
#include "gnss.h"

#define DBG_SECTION_NAME "LPM"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

void rtc_wakeup_irq_enable(void);
void shut_down(void);
rt_err_t nbiot_power_on(void);
rt_err_t nbiot_power_off(void);
rt_err_t esp32_power_on(void);
rt_err_t esp32_power_off(void);
rt_err_t esp32_en_on(void);
rt_err_t esp32_en_off(void);
rt_err_t cat1_power_on(void);
rt_err_t cat1_power_off(void);
static void alarm_callback(rt_alarm_t alarm, time_t timestamp);
rt_err_t set_rtc_wakeup(time_t sleep_time);
static void test_rtc_wakeup(int argc, char **argv);
static void test_all_pin_enable(int argc, char **argv);

#endif  // __LPM_H__