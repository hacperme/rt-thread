/*
 * @FilePath: watch_dog.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-29 18:57:56
 * @copyright : Copyright (c) 2024
 */
#ifndef __WATCH_DOG_H__
#define __WATCH_DOG_H__

#include "rtthread.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define WDG_INDEX_MAX 16

typedef void (*wdg_feed_impl_t)(void);

typedef enum
{
    WDG_NO_ERROR = 0,
    WDG_NULL_TYPE = -1,
    WDG_UNSUPPORT_BLOCK = -2,
    WDG_UNREG_THREAD = -3,
    WDG_FULL_INDEX = -4,
    WDG_NOT_INIT = -5,
    WDG_ERR_MAX = -6,
    WDF_INVALID_ARGS = -7,
} wdg_error_code_e;

typedef enum
{
    BLOCK_TYPE_NO_BLOCK = 0,
#ifdef RT_USING_SEMAPHORE
    BLOCK_TYPE_SEMAPHORE,
#endif
#ifdef RT_USING_MUTEX
    BLOCK_TYPE_MUTEX,
#endif
#ifdef RT_USING_EVENT
    BLOCK_TYPE_EVENT,
#endif
#ifdef RT_USING_MESSAGEQUEUE
    BLOCK_TYPE_MSGQ,
#endif
#ifdef RT_USING_MAILBOX
    BLOCK_TYPE_MAILBOX,
#endif
#ifdef RT_USING_SIGNALS
    BLOCK_TYPE_SIGNAL,
#endif
    BLOCK_TYPE_MAX,
} thread_block_type_e;

typedef struct
{
    void* wdg_msg;
    rt_uint32_t msg_len;
} wdg_msg_t;

typedef struct
{
    rt_thread_t thread_id;
    rt_bool_t is_fed;
    rt_uint32_t timeout;
    rt_uint32_t last_feed_timestamp;
    thread_block_type_e block_type;
    void *block_handle;
} wdg_t;

/**
 * @brief  看门狗机制初始化
 * @param  feed_cycle  硬件看门狗喂狗周期，单位：毫秒
 * @param  feed_impl   硬件看门狗的函数实现，用户使用不同的硬件看门狗，喂狗方式可能不同，此处由用户自己实现喂狗函数，原型为：void (*wdg_feed_impl_t)(void)
 * @return wdg_error_code_e 错误码
 */
wdg_error_code_e wdg_init(rt_uint32_t feed_cycle, wdg_feed_impl_t feed_impl);

/**
 * @brief 软看门狗反初始化，销毁看门狗机制，一般使用不到该接口。
 * @return wdg_error_code_e 错误码
 */
wdg_error_code_e wdg_deinit(void);

/**
 * @brief  创建软件看门狗。每一个需要喂狗的应用线程均需要调用此接口，创建一个属于该线程的软件看门狗。
 * @param  wdg_id  看门狗 ID。
 * @param  timeout  软件看门狗的超时时间。如果一个线程超过该时长仍然没有喂狗，则认为该线程出现异常，会通过不喂硬件看门狗的方式，引起系统复位。
 * @param  block_type 线程阻塞类型，详见 thread_block_type_e。
 * @param  block_handle - 有些线程可能在等待消息，而消息的触发有可能是无限期的，此时该线程就会出现长时间不喂狗的现象。
 *                      - 为了避免该问题，允许用户把消息队列或信号量或锁的句柄传递进来，底层的看门狗机制会周期性向该线程发送消息，用来触发线程向后运行，进而调用到喂狗接口。
 *                      - 调用 wdg_msg_set() 接口可设置看门狗机制向应用线程发送的消息内容。
 *                      - 不需要添加消息队列是，该参数为 RT_NULL。
 *
 * @return  看门狗 ID
 */
wdg_error_code_e wdg_create_soft(rt_uint8_t *wdg_id, rt_uint32_t timeout, thread_block_type_e block_type, void *block_handle);

/**
 * @brief  销毁软件看门狗。一般使用不到该接口。
 * @param  wdg_id 看门狗 ID
 * @return wdg_error_code_e
 */
wdg_error_code_e wdg_destroy_soft(rt_uint8_t wdg_id);

/**
 * @brief  喂软件看门狗。
 * @param  wdg_id  看门狗ID
 * @return wdg_error_code_e 错误码
 */
wdg_error_code_e wdg_feed_soft(rt_uint8_t wdg_id);

/**
 * @brief  设置看门狗机制向应用线程发送的消息内容。
 * @param  msg   消息的指针
 * @param  msg_len   消息的长度
 */
void wdg_msg_set(void* msg, rt_uint32_t msg_len);

#endif // __WATCH_DOG_H__