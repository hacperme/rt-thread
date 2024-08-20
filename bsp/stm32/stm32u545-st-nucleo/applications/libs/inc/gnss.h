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
#include "board.h"
#include "lwgps/lwgps.h"
#include "board_pin.h"

#define GNSS_UART_NAME "uart4"
#define GNSS_BUFF_SIZE 0x800

typedef struct
{
    char GNRMC[100];
    rt_uint8_t is_valid;
    char GNGGA[100];
    char GNGLL[100];
    char GNVTG[100];
    rt_uint8_t GNGSA_SIZE;
    char GNGSA[5][100];
    rt_uint8_t GPGSV_SIZE;
    char GPGSV[5][100];
} nmea_item;
typedef nmea_item *nmea_item_t;

void gnss_parse_nmea_item(char *item);
void gnss_parse_nmea(char *nmea);
static void gnss_thread_entry(void *parameter);
static rt_err_t gnss_power_on(void);
static rt_err_t gnss_power_off(void);
static rt_err_t swith_gnss_source(rt_uint8_t mode);
static rt_err_t gnss_reset_init(void);
rt_err_t gnss_reset(void);
rt_err_t gnss_open(void);
rt_err_t gnss_close(void);
rt_err_t gnss_read_nmea(char *data, rt_uint32_t size);
rt_err_t gnss_read_data(lwgps_t *gnss_data);
rt_err_t gnss_read_nmea_item(nmea_item_t nmea_item);

static rt_err_t test_show_nmea_item(nmea_item_t test_nmea_item);
static void gnss_data_show(int argc, char **argv);

#endif  // __GNSS_H__