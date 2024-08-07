/*
 * @FilePath: board_pin.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-07 09:21:43
 * @copyright : Copyright (c) 2024
 */
#ifndef __BOARD_PIN_H__
#define __BOARD_PIN_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"

void gnss_pwron_pin_init(void);
void gnss_rst_pin_init(void);
void eg915_gnssen_pin_init(void);
void sensor_pwron_pin_init(void);
void pwrctrl_pwr_wkup3_init(void);
void nbiot_pwron_pin_init(void);
void esp32_pwron_pin_init(void);
void esp32_en_pin_init(void);
void cat1_pwron_pin_init(void);

rt_err_t gnss_pwron_pin_enable(rt_uint8_t mode);
rt_err_t gnss_rst_pin_enable(rt_uint8_t mode);
rt_err_t eg915_gnssen_pin_enable(rt_uint8_t mode);
rt_err_t sensor_pwron_pin_enable(rt_uint8_t mode);
rt_err_t pwrctrl_pwr_wkup3_irq_enable(void);
rt_err_t nbiot_pwron_pin_enable(rt_uint8_t mode);
rt_err_t esp32_pwron_pin_enable(rt_uint8_t mode);
rt_err_t esp32_en_pin_enable(rt_uint8_t mode);
rt_err_t cat1_pwron_pin_enable(rt_uint8_t mode);

int board_pins_init(void);

#endif  // __BOARD_PIN_H__
