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

#define DBG_SECTION_NAME "LPM"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define PHTM_PWR_WKUP3    GET_PIN(E, 6)
#define NBIOT_PWRON_PIN   GET_PIN(E, 2)
#define ESP32_EN_PIN      GET_PIN(E, 5)
#define ESP32_PWRON_PIN   GET_PIN(H, 1)
#define CAT1_PWRON_PIN    GET_PIN(A, 8)

/* This wakeup pin is in STM32U545 Board, just for test.*/
// #define TEST_BTN_WKUP2  GET_PIN(C, 13)

void all_pin_init(void);
void power_wakeup_irq_enable(void);
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