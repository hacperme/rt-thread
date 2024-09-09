/*
 * @FilePath: test_wdg.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-29 14:23:05
 * @copyright : Copyright (c) 2024
 */

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "lpm.h"

#define DBG_SECTION_NAME "WDT"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define RT_RES_MSG(res) (res == RT_EOK ? "success" : "failed")

#define WDT_DEVICE_NAME "wdt"
static rt_device_t wdg_dev = RT_NULL;

#define WDG_THD_STACK_SIZE 0x400
static struct rt_thread WDG_THD;
static char WDG_THD_STACK[WDG_THD_STACK_SIZE];

#define wdgq_pool_size 128
#define wdgq_msg_size  8
static struct rt_messagequeue wdgq;
static char wdgq_pool[wdgq_pool_size] = {0};

static void feed_rtwdg(void)
{
    rt_err_t res = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
    LOG_D("RT_DEVICE_CTRL_WDT_KEEPALIVE %s", RT_RES_MSG(res));
}

static void wdg_feeded(void *args)
{
    while (1)
    {
        feed_rtwdg();
        rt_thread_mdelay(100);
    }
    
}

static rt_err_t test_wdg(void)
{
    rt_err_t res;

    wdg_dev = rt_device_find(WDT_DEVICE_NAME);
    res = wdg_dev != RT_NULL ? RT_EOK : RT_ERROR;
    LOG_D("Find %s device %s.", WDT_DEVICE_NAME, RT_RES_MSG(res));
    if (res == RT_ERROR)
    {
        return res;
    }

    // res = rt_device_init(wdg_dev);
    // LOG_D("rt_device_init wdg %s", RT_RES_MSG(res));
    // if (res == RT_ERROR)
    // {
    //     return res;
    // }

    // rt_uint32_t timeout = 0;
    // res = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_GET_TIMEOUT, &timeout);
    // LOG_D("RT_DEVICE_CTRL_WDT_GET_TIMEOUT %d %s.", timeout, RT_RES_MSG(res));
    // if (res != RT_EOK)
    // {
    //     return RT_ERROR;
    // }

    // timeout = 200;
    // res = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    // LOG_D("RT_DEVICE_CTRL_WDT_SET_TIMEOUT %d %s.", timeout, RT_RES_MSG(res));
    // if (res != RT_EOK)
    // {
    //     return RT_ERROR;
    // }

    /* 创建线程 */
    res = rt_thread_init(&WDG_THD, "GNSSTHD", wdg_feeded, RT_NULL, WDG_THD_STACK, WDG_THD_STACK_SIZE, 5, 20);
    LOG_D("rt_thread_init WDG_THD %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        rt_err_t _res = rt_device_close(wdg_dev);
        LOG_D("WDG closed %s.", RT_RES_MSG(_res));
        return res;
    }

    res = rt_thread_startup(&WDG_THD);
    LOG_D("wdg_feeded start %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        rt_err_t _res = rt_device_close(wdg_dev);
        LOG_D("WDG closed %s.", RT_RES_MSG(_res));
    }

    /* 启动看门狗 */
    res = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
    LOG_D("RT_DEVICE_CTRL_WDT_START %s.", RT_RES_MSG(res));

    res = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
    LOG_D("RT_DEVICE_CTRL_WDT_KEEPALIVE %s", RT_RES_MSG(res));

    return res;
}

// MSH_CMD_EXPORT(test_wdg, test watch dog);

static void test_show_rrc_flag(void)
{
    LOG_D("__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST)=%d", __HAL_RCC_GET_FLAG(RCC_FLAG_BORRST));
    LOG_D("__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST)=%d", __HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST));
    LOG_D("__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)=%d", __HAL_RCC_GET_FLAG(RCC_FLAG_PINRST));
    LOG_D("__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)=%d", __HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST));
    LOG_D("__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)=%d", __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST));
    LOG_D("__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)=%d", __HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST));
    LOG_D("__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)=%d", __HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST));
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

MSH_CMD_EXPORT(test_show_rrc_flag, test show rrc flag);

#include "watch_dog.h"
static void test_feed_wdg_soft(void *args)
{
    wdg_error_code_e res;
    rt_uint8_t wdg_id;
    rt_uint8_t cnt = 10;
    rt_uint8_t *params = (rt_uint8_t *)args;
    if (*params == 0)
    {
        res = wdg_create_soft(&wdg_id, 2000, BLOCK_TYPE_NO_BLOCK, RT_NULL);
        LOG_D("wdg_create_soft %s wdg_id=%d.", RT_RES_MSG(res), wdg_id);
        if (res != WDG_NO_ERROR)
        {
            return;
        }
        while (cnt > 0)
        {
            res = wdg_feed_soft(wdg_id);
            LOG_D("wdg_feed_soft(%d) %s.", wdg_id, RT_RES_MSG(res));
            rt_thread_mdelay(1500);
            cnt--;
        }
    }
    else if (*params == 1)
    {
        res = wdg_create_soft(&wdg_id, 2000, BLOCK_TYPE_MSGQ, &wdgq);
        // res = wdg_create_soft(&wdg_id, 2000, BLOCK_TYPE_MSGQ, RT_NULL);
        LOG_D("wdg_create_soft %s wdg_id=%d.", RT_RES_MSG(res), wdg_id);
        if (res != WDG_NO_ERROR)
        {
            return;
        }
        rt_ssize_t recv_size = 0;
        char recv_msg[wdgq_msg_size] = {0};
        while (cnt > 0)
        {
            rt_memset(recv_msg, 0, wdgq_msg_size);
            recv_size = rt_mq_recv(&wdgq, recv_msg, wdgq_msg_size, 2500);
            if (recv_size > 0)
            {
                LOG_D("rt_mq_recv msg_size=%d, msg=%s", recv_size, recv_msg);
            }
            res = wdg_feed_soft(wdg_id);
            LOG_D("wdg_feed_soft(%d) %s.", wdg_id, RT_RES_MSG(res));
            cnt--;
        }
        rt_mq_detach(&wdgq);
    }
    res = wdg_destroy_soft(wdg_id);
    shut_down();
    LOG_D("wdg_destroy_soft(wdg_id) %s.", RT_RES_MSG(res));
}

static rt_err_t test_watch_dog(void)
{
    rt_err_t res;

    res = rt_mq_init(&wdgq, "wdgq", wdgq_pool, wdgq_msg_size, wdgq_pool_size, RT_IPC_FLAG_PRIO);
    LOG_D("rt_mq_init wdgq %s", RT_RES_MSG(res));
    if (res != RT_EOK)
    {
        return res;
    }

    wdg_dev = rt_device_find(WDT_DEVICE_NAME);
    res = wdg_dev != RT_NULL ? RT_EOK : RT_ERROR;
    LOG_D("Find %s device %s.", WDT_DEVICE_NAME, RT_RES_MSG(res));
    if (res == RT_ERROR)
    {
        return res;
    }

    wdg_error_code_e err = wdg_init(100, feed_rtwdg);
    LOG_D("wdg_init %s.", RT_RES_MSG(err));

    /* 启动看门狗 */
    res = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
    LOG_D("RT_DEVICE_CTRL_WDT_START %s.", RT_RES_MSG(res));

    rt_uint8_t args = 1;
    rt_thread_t test_thd = rt_thread_create("testwdg", test_feed_wdg_soft, &args, 0x800, 10, 20);
    res = test_thd == RT_NULL ? RT_ERROR : RT_EOK;
    LOG_D("rt_thread_create testwdg %s.", RT_RES_MSG(res));
    if (res == RT_ERROR)
    {
        return res;
    }

    res = rt_thread_startup(test_thd);
    LOG_D("rt_thread_startup testwdg %s.", RT_RES_MSG(res));

    return res;
}

MSH_CMD_EXPORT(test_watch_dog, test watch dog);
