/*
 * @FilePath: test_st_at.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-11-06 14:40:47
 * @copyright : Copyright (c) 2024
 */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include "dfs_fs.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "tools.h"
#include "logging.h"
#include "at.h"

#define ST_AT_CMD_TRANSFILE "+TRANSFILE:"
#define ST_AT_CMD_TRSFDATA "+TRSFDATA:"
#define ST_AT_CMD_TRSFEND "+TRSFEND"

static FILE *trans_file = NULL;
static char trans_file_name[64] = {0};
static char trans_file_size_str[16] = {0};
static int trans_file_size = 0;
static int recv_file_size = 0;

#define lpuart_recv_buff_size 512
static rt_device_t lpuart_serial;
static struct rt_semaphore lpuart_rx_sem;
static rt_uint8_t lpuart_recv_buff[lpuart_recv_buff_size] = {0};
static rt_uint8_t recv_exit = 1;
static rt_thread_t lpuart_recv_thd;

static rt_err_t lpuart_rx_input(rt_device_t dev, rt_size_t size)
{
    return rt_sem_release(&lpuart_rx_sem);
}

void lpuart_parse_msg(char *data, rt_size_t size)
{
    // log_debug("lpuart_parse_msg data %s", data);
    if (rt_strncmp(data, ST_AT_CMD_TRANSFILE, rt_strlen(ST_AT_CMD_TRANSFILE)) == 0)
    {
        char *file_name_start = data;
        file_name_start += rt_strlen(ST_AT_CMD_TRANSFILE);
        char *file_name_end = file_name_start;
        for (int i = 0; i < size - 2 - rt_strlen(ST_AT_CMD_TRANSFILE); i++)
        {
            if (file_name_end[0] == ',')
            {
                break;
            }
            file_name_end++;
        }
        rt_memset(trans_file_name, 0, sizeof(trans_file_name));
        rt_memcpy(trans_file_name, file_name_start, (file_name_end - file_name_start));
        // log_debug("trans_file_name=%s", trans_file_name);

        rt_memset(trans_file_size_str, 0, sizeof(trans_file_size_str));
        trans_file_size = 0;
        recv_file_size = 0;
        file_name_end += 1;
        rt_memcpy(trans_file_size_str, file_name_end, (size - 2 - (file_name_end - data)));
        trans_file_size = atoi(trans_file_size_str);
        // log_debug("trans_file_size_str=%s, trans_file_size=%d", trans_file_size_str, trans_file_size);

        trans_file = fopen(trans_file_name, "wb");

        rt_device_write(lpuart_serial, 0, data, size);
    }
    else if (rt_strncmp(data, ST_AT_CMD_TRSFDATA, rt_strlen(ST_AT_CMD_TRSFDATA)) == 0)
    {
        if (trans_file != NULL)
        {
            int wret = fwrite(
                data + rt_strlen(ST_AT_CMD_TRSFDATA),
                1,
                size - 2 - rt_strlen(ST_AT_CMD_TRSFDATA),
                trans_file
            );
            recv_file_size += wret;
        }
        char resp[32] = {0};
        rt_memset(resp, 0, 32);
        rt_snprintf(resp, 32, "RECV:%d,%d\r\n", recv_file_size, trans_file_size);
        rt_device_write(lpuart_serial, 0, resp, rt_strlen(resp));
    }
    else if (rt_strncmp(data, ST_AT_CMD_TRSFEND, rt_strlen(ST_AT_CMD_TRSFEND)) == 0)
    {
        fclose(trans_file);
        trans_file = NULL;
        char resp[] = "TRANS OVER\r\n";
        rt_device_write(lpuart_serial, 0, resp, rt_strlen(resp));
    }
}

void lpuart_recv_entry(void *args)
{
    rt_err_t res;
    rt_size_t rsize = 0;
    rt_size_t ttlrsize = 0;
    int timeout = 0;
    res = rt_sem_trytake(&lpuart_rx_sem);
    while (!recv_exit)
    {
        timeout = ttlrsize == 0 ? RT_WAITING_FOREVER : 10;
        res = rt_sem_take(&lpuart_rx_sem, timeout);
        if (res == RT_EOK)
        {
            rsize = rt_device_read(lpuart_serial, 0, lpuart_recv_buff + ttlrsize, 1);
            ttlrsize += rsize;
        }
        else
        {
            if (ttlrsize != 0)
            {
                lpuart_parse_msg((char *)lpuart_recv_buff, ttlrsize);
                rt_memset(lpuart_recv_buff, 0, lpuart_recv_buff_size);
                ttlrsize = rsize = 0;
            }
        }
    }
}

rt_err_t st_lpuart_init(void)
{
    rt_err_t res;
    lpuart_serial = rt_device_find(LOG_UART);
    res = lpuart_serial != RT_NULL ? RT_EOK : RT_ERROR;
    log_debug("rt_device_find %s %s", LOG_UART, res_msg(res == RT_EOK));
    if (res != RT_EOK) return res;

    res = rt_sem_init(&lpuart_rx_sem, "lprxsem", 0, RT_IPC_FLAG_PRIO);
    log_debug("rt_sem_init lprxsem %s", res_msg(res == RT_EOK));
    if (res != RT_EOK) return res;

    res = rt_device_set_rx_indicate(lpuart_serial, lpuart_rx_input);
    log_debug("rt_device_set_rx_indicate %s", res_msg(res == RT_EOK));
    if (res != RT_EOK) return res;

    lpuart_recv_thd = rt_thread_create("plurecv", lpuart_recv_entry, RT_NULL, 0x1000, 5, 10);
    res = lpuart_recv_thd != RT_NULL ? RT_EOK : RT_ERROR;
    log_debug("rt_thread_create plurecv %s", res_msg(res == RT_EOK));
    if (res != RT_EOK) return res;

    recv_exit = 0;
    res = rt_thread_startup(lpuart_recv_thd);
    log_debug("rt_thread_startup plurecv %s", res_msg(res == RT_EOK));
    if (res != RT_EOK) recv_exit = 1;

    return res;
}

void test_st_at(void)
{
    rt_err_t res;
    res = st_lpuart_init();
    log_debug("st_lpuart_init %s", res_msg(res == RT_EOK));
}