/*
 * @FilePath: gnss.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-30 09:24:43
 * @copyright : Copyright (c) 2024
 */
#ifndef __GNSS_H__
#define __GNSS_H__

#include "rtthread.h"
#include "rtdevice.h"
#include <board.h>
#include "lwgps/lwgps.h"

#define GNSS_UART_NAME "uart4"
#define GNSS_BUFF_SIZE 0x400

static void gnss_thread_entry(void *parameter);
static void gnss_power_on(void);
static rt_err_t gnss_power_off(void);
static void gnss_reset_init(void);
rt_err_t gnss_open(void);
void gnss_close(void);
rt_err_t gnss_reset(void);
static void gnss_data_show(int argc, char **argv);

#endif  // __GNSS_H__