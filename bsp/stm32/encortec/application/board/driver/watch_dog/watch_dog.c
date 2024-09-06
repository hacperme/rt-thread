/*
 * @FilePath: watch_dog.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-29 18:57:39
 * @copyright : Copyright (c) 2024
 */
#include "watch_dog.h"
#include "tools.h"
#include "logging.h"

static wdg_t wdg_reg_list[WDG_INDEX_MAX] = {0};
static rt_uint32_t wdg_feed_cycle = 100;
static rt_bool_t wdg_feed_exit = RT_TRUE;
static rt_bool_t wdf_init_over = RT_FALSE;

static struct rt_mutex wdg_mutex = {0};
static struct rt_semaphore wdg_thd_suspend_sem = {0};

#define wdg_thread_stack_size 0x1000
static struct rt_thread wdg_thread = {0};
static char wdg_thread_stack[wdg_thread_stack_size] = {0};

wdg_feed_impl_t wdg_feed_impl = RT_NULL;

wdg_msg_t wdg_msg;
static char default_wdg_msg[] = "wdg";

static rt_bool_t _wdg_should_feed(void)
{
    for(rt_uint8_t i = 0; i < WDG_INDEX_MAX; i++)
    {
        if(wdg_reg_list[i].thread_id == RT_NULL)
        {
            continue;
        }

        if(!wdg_reg_list[i].is_fed && wdg_reg_list[i].timeout > 0)
        {
            rt_uint32_t feed_time_interval = rt_tick_diff(wdg_reg_list[i].last_feed_timestamp, rt_tick_get_millisecond());

            log_debug("Feed time interval:%ld", feed_time_interval);

            if(feed_time_interval > wdg_reg_list[i].timeout)
            {
                return RT_FALSE;
            }
        }
    }

    log_debug("Should feed the hardware watch dog.\r\n");

    return RT_TRUE;
}

static void _wdg_feed(void)
{
    if(wdg_feed_impl != NULL)
    {
        wdg_feed_impl();
    }

    for(rt_uint8_t i = 0; i < WDG_INDEX_MAX; i++)
    {
        if(wdg_reg_list[i].thread_id == RT_NULL)
        {
            continue;
        }
        else
        {
            if(wdg_reg_list[i].is_fed)
            {
                wdg_reg_list[i].last_feed_timestamp = rt_tick_get_millisecond();
                wdg_reg_list[i].is_fed = RT_FALSE;
            }
        }
    }
}

static wdg_error_code_e _block_check(void)
{
    for(rt_uint8_t i = 0; i < WDG_INDEX_MAX; i++)
    {
        if(wdg_reg_list[i].thread_id == RT_NULL || wdg_reg_list[i].block_handle == RT_NULL)
        {
            continue;
        }
        else
        {
            switch (wdg_reg_list[i].block_type)
            {
#ifdef RT_USING_SEMAPHORE
                case BLOCK_TYPE_SEMAPHORE:
                    rt_sem_release((rt_sem_t)wdg_reg_list[i].block_handle);
                    break;
#endif
#ifdef RT_USING_MUTEX
                case BLOCK_TYPE_MUTEX:
                    rt_mutex_release((rt_mutex_t)wdg_reg_list[i].block_handle);
                    break;
#endif
#ifdef RT_USING_EVENT
                case BLOCK_TYPE_EVENT:
                    rt_event_send((rt_event_t)wdg_reg_list[i].block_handle, (rt_uint32_t)wdg_msg.wdg_msg);
                    break;
#endif
#ifdef RT_USING_MESSAGEQUEUE
                case BLOCK_TYPE_MSGQ:
                    rt_mq_send((rt_mq_t)wdg_reg_list[i].block_handle, wdg_msg.wdg_msg, wdg_msg.msg_len);
                    break;
#endif
#ifdef RT_USING_MAILBOX
                case BLOCK_TYPE_MAILBOX:
                    rt_mb_send((rt_mailbox_t)wdg_reg_list[i].block_handle, (rt_uint32_t)wdg_msg.wdg_msg);
                    break;
#endif
#ifdef RT_USING_SIGNALS
                case BLOCK_TYPE_SIGNAL:
                    rt_thread_kill((rt_thread_t)wdg_reg_list[i].block_handle, (int)wdg_msg.wdg_msg);
                    break;
#endif
                default:
                    log_debug("NO OPERATION");
                    break;
            }
        }
    }

    return WDG_NO_ERROR;
}

static void _wdg_feed_task(void *args)
{
    //UNUSED(args);
    rt_bool_t should_feed = RT_TRUE;

    while(wdg_feed_exit == RT_FALSE)
    {
        rt_mutex_take(&wdg_mutex, RT_WAITING_FOREVER);
        should_feed = _wdg_should_feed();
        rt_mutex_release(&wdg_mutex);

        if(should_feed)
        {
            rt_mutex_take(&wdg_mutex, RT_WAITING_FOREVER);
            _wdg_feed();
            rt_mutex_release(&wdg_mutex);
        }

        rt_thread_mdelay(wdg_feed_cycle);

        rt_mutex_take(&wdg_mutex, RT_WAITING_FOREVER);
        _block_check();
        rt_mutex_release(&wdg_mutex);
    }

    rt_enter_critical();
    rt_sem_release(&wdg_thd_suspend_sem);
    rt_thread_suspend(rt_thread_self());
    rt_schedule();
    rt_exit_critical();
}

wdg_error_code_e wdg_init(rt_uint32_t feed_cycle, wdg_feed_impl_t feed_impl)
{
    rt_err_t res;
    wdg_msg.wdg_msg = (void *)default_wdg_msg;
    wdg_msg.msg_len = rt_strlen(default_wdg_msg);

    if (wdf_init_over == RT_TRUE)
    {
        return WDG_NO_ERROR;
    }

    res = rt_mutex_init(&wdg_mutex, "wdgmtx", RT_IPC_FLAG_PRIO);
    if(res != RT_EOK)
    {
        log_error("wdg MUTEX init error\r\n");
        return WDG_NOT_INIT;
    }

    res = rt_sem_init(&wdg_thd_suspend_sem, "wdgsem", 0, RT_IPC_FLAG_PRIO);
    if(res != RT_EOK)
    {
        log_error("wdg suspend semaphore create error\r\n");
        rt_mutex_detach(&wdg_mutex);
        rt_memset(&wdg_mutex, 0, sizeof(wdg_mutex));
        return WDG_NOT_INIT;
    }

    res = rt_thread_init(&wdg_thread, "wdgthd", _wdg_feed_task, RT_NULL, wdg_thread_stack, wdg_thread_stack_size, 1, 50);
    if(res != RT_EOK)
    {
        log_error("wdg Thread init error\r\n");
        rt_mutex_detach(&wdg_mutex);
        rt_memset(&wdg_mutex, 0, sizeof(wdg_mutex));
        rt_sem_detach(&wdg_thd_suspend_sem);
        rt_memset(&wdg_thd_suspend_sem, 0, sizeof(wdg_thd_suspend_sem));
        return WDG_NOT_INIT;
    }

    wdg_feed_exit = RT_FALSE;
    res = rt_thread_startup(&wdg_thread);
    if (res != RT_EOK)
    {
        log_error("wdg Thread start error\r\n");
        wdg_feed_exit = RT_TRUE;
        rt_mutex_detach(&wdg_mutex);
        rt_memset(&wdg_mutex, 0, sizeof(wdg_mutex));
        rt_sem_detach(&wdg_thd_suspend_sem);
        rt_memset(&wdg_thd_suspend_sem, 0, sizeof(wdg_thd_suspend_sem));
        rt_thread_detach(&wdg_thread);
        rt_memset(&wdg_thread, 0, sizeof(wdg_thread));
        return WDG_NOT_INIT;
    }

    wdg_feed_cycle = feed_cycle;

    wdg_feed_impl = feed_impl;

    wdf_init_over = RT_TRUE;

    log_info("wdg init OK\r\n");

    return WDG_NO_ERROR;
}

wdg_error_code_e wdg_deinit(void)
{
    rt_err_t res;
    if (wdf_init_over == RT_FALSE)
    {
        return WDG_NO_ERROR;
    }

    wdg_feed_exit = RT_TRUE;
    res = rt_sem_take(&wdg_thd_suspend_sem, 500);
    log_debug("rt_sem_take wdg_thd_suspend_sem %s", res == RT_EOK ? "success" : "failed");
    if (res == RT_EOK)
    {
        do {
            res = rt_mutex_take(&wdg_mutex, RT_WAITING_NO);
            rt_mutex_release(&wdg_mutex);
        } while (res != RT_EOK);
        rt_mutex_detach(&wdg_mutex);
        rt_memset(&wdg_mutex, 0, sizeof(wdg_mutex));

        rt_sem_detach(&wdg_thd_suspend_sem);
        rt_memset(&wdg_thd_suspend_sem, 0, sizeof(wdg_thd_suspend_sem));
        rt_thread_detach(&wdg_thread);
        rt_memset(&wdg_thread, 0, sizeof(wdg_thread));
    }

    wdf_init_over = RT_FALSE;

    return WDG_NO_ERROR;
}

wdg_error_code_e wdg_create_soft(rt_uint8_t *wdg_id, rt_uint32_t timeout, thread_block_type_e block_type, void *block_handle)
{
    if(wdf_init_over == RT_FALSE)
    {
        log_error("Software wdg creation failed.");
        return WDG_NOT_INIT;
    }
    if (timeout > (RT_UINT32_MAX >> 1))
    {
        log_error("Software wdg creation timeout should be smaller %ld.", (RT_UINT32_MAX >> 1));
        return WDF_INVALID_ARGS;
    }

    rt_thread_t thread_id  = rt_thread_self();

    rt_mutex_take(&wdg_mutex, RT_WAITING_FOREVER);
    for(int i = 0; i < WDG_INDEX_MAX; i++)
    {
        if(wdg_reg_list[i].thread_id == 0 || wdg_reg_list[i].thread_id == thread_id)
        {
            wdg_reg_list[i].thread_id = thread_id;
            wdg_reg_list[i].is_fed = RT_TRUE;
            wdg_reg_list[i].timeout = timeout;

            wdg_reg_list[i].block_type = block_type;
            wdg_reg_list[i].block_handle = block_handle;

            *wdg_id = i;
            break;
        }
    }
    rt_mutex_release(&wdg_mutex);
    log_info("Software wdg creation OK: %d", *wdg_id);

    return WDG_NO_ERROR;
}

wdg_error_code_e wdg_destroy_soft(rt_uint8_t wdg_id)
{
    if(wdf_init_over == RT_FALSE)
    {
        return WDG_NOT_INIT;
    }

    if(wdg_id >= WDG_INDEX_MAX)
    {
        return WDG_UNREG_THREAD;
    }
    wdg_error_code_e ret = 0;

    rt_mutex_take(&wdg_mutex, RT_WAITING_FOREVER);

    if(wdg_reg_list[wdg_id].thread_id != 0)
    {
        wdg_reg_list[wdg_id].thread_id = 0;
        log_debug("Software wdg destroy OK: %d", wdg_id);
        ret = WDG_NO_ERROR;
    }
    else
    {
        ret = WDG_UNREG_THREAD;
    }

    rt_mutex_release(&wdg_mutex);

    log_debug("unregister success %d", wdg_id);

    return ret;
}

wdg_error_code_e wdg_feed_soft(rt_uint8_t wdg_id)
{
    if(wdf_init_over == RT_FALSE)
    {
        return WDG_NOT_INIT;
    }

    if(wdg_id >= WDG_INDEX_MAX)
    {
        return WDG_UNREG_THREAD;
    }
    wdg_error_code_e ret = WDG_NO_ERROR;

    rt_mutex_take(&wdg_mutex, RT_WAITING_FOREVER);

    if(wdg_reg_list[wdg_id].thread_id != 0)
    {
        wdg_reg_list[wdg_id].is_fed = RT_TRUE;
        ret = WDG_NO_ERROR;
    }
    else
    {
        ret = WDG_UNREG_THREAD;
    }

    rt_mutex_release(&wdg_mutex);

    return ret;
}

void wdg_msg_set(void *msg, rt_uint32_t msg_len)
{
    if(msg == RT_NULL || msg_len == 0)
    {
        return;
    }
    wdg_msg.wdg_msg = msg;
    wdg_msg.msg_len = msg_len;
}
