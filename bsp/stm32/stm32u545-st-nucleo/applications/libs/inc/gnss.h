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
int gnss_open(void);
int gnss_close(void);
int gnss_reset(void);
void gnss_data_show(void);

#endif  // __GNSS_H__