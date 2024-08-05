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

void g_sensor_wakeup_irq_enable(void);
void power_wakeup_irq_enable(void);
void rtc_wakeup_irq_enable(void);
void shut_down(void);
static void alarm_callback(rt_alarm_t alarm, time_t timestamp);
rt_err_t set_rtc_wakeup(time_t sleep_time);
static void test_rtc_wakeup(int argc, char **argv);

#endif  // __LPM_H__