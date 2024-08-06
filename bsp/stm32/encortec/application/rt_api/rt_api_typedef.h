#ifndef __ENCORTEC_API_TYPEDEF_H
#define __ENCORTEC_API_TYPEDEF_H

#include "rttypes.h"
#include "rtthread.h"
#include <stdarg.h>

typedef rt_err_t (*rt_thread_init_api_ptr_t)(struct rt_thread *thread,
                        const char       *name,
                        void (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        rt_uint32_t       stack_size,
                        rt_uint8_t        priority,
                        rt_uint32_t       tick);
typedef rt_err_t (*rt_thread_detach_api_ptr_t)(rt_thread_t thread);
typedef rt_thread_t (*rt_thread_create_api_ptr_t)(const char *name,
                             void (*entry)(void *parameter),
                             void       *parameter,
                             rt_uint32_t stack_size,
                             rt_uint8_t  priority,
                             rt_uint32_t tick);
typedef rt_err_t (*rt_thread_delete_api_ptr_t)(rt_thread_t thread);
typedef rt_err_t (*rt_thread_close_api_ptr_t)(rt_thread_t thread);
typedef rt_thread_t (*rt_thread_self_api_ptr_t)(void);
typedef rt_thread_t (*rt_thread_find_api_ptr_t)(char *name);
typedef rt_err_t (*rt_thread_startup_api_ptr_t)(rt_thread_t thread);
typedef rt_err_t (*rt_thread_yield_api_ptr_t)(void);
typedef rt_err_t (*rt_thread_delay_api_ptr_t)(rt_tick_t tick);
typedef rt_err_t (*rt_thread_delay_until_api_ptr_t)(rt_tick_t *tick, rt_tick_t inc_tick);
typedef rt_err_t(*rt_thread_mdelay_api_ptr_t)(rt_int32_t ms);
typedef rt_err_t (*rt_thread_control_api_ptr_t)(rt_thread_t thread, int cmd, void *arg);
typedef rt_err_t (*rt_thread_suspend_api_ptr_t)(rt_thread_t thread);
typedef rt_err_t (*rt_thread_suspend_with_flag_api_ptr_t)(rt_thread_t thread, int suspend_flag);
typedef rt_err_t (*rt_thread_resume_api_ptr_t)(rt_thread_t thread);
typedef rt_err_t (*rt_thread_get_name_api_ptr_t)(rt_thread_t thread, char *name, rt_uint8_t name_size);
typedef rt_base_t (*rt_enter_critical_api_ptr_t)(void);
typedef void (*rt_exit_critical_api_ptr_t)(void);
typedef void (*rt_exit_critical_safe_api_ptr_t)(rt_base_t critical_level);
typedef rt_uint16_t (*rt_critical_level_api_ptr_t)(void);
typedef rt_err_t (*rt_sem_init_api_ptr_t)(rt_sem_t    sem,
                     const char *name,
                     rt_uint32_t value,
                     rt_uint8_t  flag);
typedef rt_err_t (*rt_sem_detach_api_ptr_t)(rt_sem_t sem);
typedef rt_sem_t (*rt_sem_create_api_ptr_t)(const char *name, rt_uint32_t value, rt_uint8_t flag);
typedef rt_err_t (*rt_sem_delete_api_ptr_t)(rt_sem_t sem);
typedef rt_err_t (*rt_sem_take_api_ptr_t)(rt_sem_t sem, rt_int32_t time);
typedef rt_err_t (*rt_sem_take_interruptible_api_ptr_t)(rt_sem_t sem, rt_int32_t time);
typedef rt_err_t (*rt_sem_take_killable_api_ptr_t)(rt_sem_t sem, rt_int32_t time);
typedef rt_err_t (*rt_sem_trytake_api_ptr_t)(rt_sem_t sem);
typedef rt_err_t (*rt_sem_release_api_ptr_t)(rt_sem_t sem);
typedef rt_err_t (*rt_sem_control_api_ptr_t)(rt_sem_t sem, int cmd, void *arg);
typedef rt_err_t (*rt_mutex_init_api_ptr_t)(rt_mutex_t mutex, const char *name, rt_uint8_t flag);
typedef rt_err_t (*rt_mutex_detach_api_ptr_t)(rt_mutex_t mutex);
typedef rt_mutex_t (*rt_mutex_create_api_ptr_t)(const char *name, rt_uint8_t flag);
typedef rt_err_t (*rt_mutex_delete_api_ptr_t)(rt_mutex_t mutex);
typedef void (*rt_mutex_drop_thread_api_ptr_t)(rt_mutex_t mutex, rt_thread_t thread);
typedef rt_uint8_t (*rt_mutex_setprioceiling_api_ptr_t)(rt_mutex_t mutex, rt_uint8_t priority);
typedef rt_uint8_t (*rt_mutex_getprioceiling_api_ptr_t)(rt_mutex_t mutex);
typedef rt_err_t (*rt_mutex_take_api_ptr_t)(rt_mutex_t mutex, rt_int32_t time);
typedef rt_err_t (*rt_mutex_trytake_api_ptr_t)(rt_mutex_t mutex);
typedef rt_err_t (*rt_mutex_take_interruptible_api_ptr_t)(rt_mutex_t mutex, rt_int32_t time);
typedef rt_err_t (*rt_mutex_take_killable_api_ptr_t)(rt_mutex_t mutex, rt_int32_t time);
typedef rt_err_t (*rt_mutex_release_api_ptr_t)(rt_mutex_t mutex);
typedef rt_err_t (*rt_mutex_control_api_ptr_t)(rt_mutex_t mutex, int cmd, void *arg);
typedef rt_err_t (*rt_event_init_api_ptr_t)(rt_event_t event, const char *name, rt_uint8_t flag);
typedef rt_err_t (*rt_event_detach_api_ptr_t)(rt_event_t event);
typedef rt_event_t (*rt_event_create_api_ptr_t)(const char *name, rt_uint8_t flag);
typedef rt_err_t (*rt_event_delete_api_ptr_t)(rt_event_t event);
typedef rt_err_t (*rt_event_send_api_ptr_t)(rt_event_t event, rt_uint32_t set);
typedef rt_err_t (*rt_event_recv_api_ptr_t)(rt_event_t   event,
                       rt_uint32_t  set,
                       rt_uint8_t   option,
                       rt_int32_t   timeout,
                       rt_uint32_t *recved);
typedef rt_err_t (*rt_event_recv_interruptible_api_ptr_t)(rt_event_t   event,
                       rt_uint32_t  set,
                       rt_uint8_t   option,
                       rt_int32_t   timeout,
                       rt_uint32_t *recved);
typedef rt_err_t (*rt_event_recv_killable_api_ptr_t)(rt_event_t   event,
                       rt_uint32_t  set,
                       rt_uint8_t   option,
                       rt_int32_t   timeout,
                       rt_uint32_t *recved);
typedef rt_err_t (*rt_event_control_api_ptr_t)(rt_event_t event, int cmd, void *arg);
typedef rt_err_t (*rt_mq_init_api_ptr_t)(rt_mq_t     mq,
                    const char *name,
                    void       *msgpool,
                    rt_size_t   msg_size,
                    rt_size_t   pool_size,
                    rt_uint8_t  flag);
typedef rt_err_t (*rt_mq_detach_api_ptr_t)(rt_mq_t mq);
typedef rt_mq_t (*rt_mq_create_api_ptr_t)(const char *name,
                     rt_size_t   msg_size,
                     rt_size_t   max_msgs,
                     rt_uint8_t  flag);
typedef rt_err_t (*rt_mq_delete_api_ptr_t)(rt_mq_t mq);
typedef rt_err_t (*rt_mq_send_api_ptr_t)(rt_mq_t mq, const void *buffer, rt_size_t size);
typedef rt_err_t (*rt_mq_send_interruptible_api_ptr_t)(rt_mq_t mq, const void *buffer, rt_size_t size);
typedef rt_err_t (*rt_mq_send_killable_api_ptr_t)(rt_mq_t mq, const void *buffer, rt_size_t size);
typedef rt_err_t (*rt_mq_send_wait_api_ptr_t)(rt_mq_t     mq,
                         const void *buffer,
                         rt_size_t   size,
                         rt_int32_t  timeout);
typedef rt_err_t (*rt_mq_send_wait_interruptible_api_ptr_t)(rt_mq_t     mq,
                         const void *buffer,
                         rt_size_t   size,
                         rt_int32_t  timeout);
typedef rt_err_t (*rt_mq_send_wait_killable_api_ptr_t)(rt_mq_t     mq,
                         const void *buffer,
                         rt_size_t   size,
                         rt_int32_t  timeout);
typedef rt_err_t (*rt_mq_urgent_api_ptr_t)(rt_mq_t mq, const void *buffer, rt_size_t size);
typedef rt_ssize_t (*rt_mq_recv_api_ptr_t)(rt_mq_t    mq,
                    void      *buffer,
                    rt_size_t  size,
                    rt_int32_t timeout);
typedef rt_ssize_t (*rt_mq_recv_interruptible_api_ptr_t)(rt_mq_t    mq,
                    void      *buffer,
                    rt_size_t  size,
                    rt_int32_t timeout);
typedef rt_ssize_t (*rt_mq_recv_killable_api_ptr_t)(rt_mq_t    mq,
                    void      *buffer,
                    rt_size_t  size,
                    rt_int32_t timeout);
typedef rt_err_t (*rt_mq_control_api_ptr_t)(rt_mq_t mq, int cmd, void *arg);
typedef void (*rt_interrupt_enter_api_ptr_t)(void);
typedef void (*rt_interrupt_leave_api_ptr_t)(void);
typedef void (*rt_assert_handler_api_ptr_t)(const char *ex_string, const char *func, rt_size_t line);
typedef void (*rt_kputs_api_ptr_t)(const char *str);
typedef int(*rt_kprintf_api_ptr_t)(const char *fmt, ...);
typedef void *(*rt_malloc_api_ptr_t)(rt_size_t size);
typedef void(*rt_free_api_ptr_t)(void *ptr);
typedef void *(*rt_realloc_api_ptr_t)(void *ptr, rt_size_t newsize);
typedef void *(*rt_calloc_api_ptr_t)(rt_size_t count, rt_size_t size);
typedef void (*rt_memory_info_api_ptr_t)(rt_size_t *total,
                            rt_size_t *used,
                            rt_size_t *max_used);
typedef int (*rt_vsnprintf_api_ptr_t)(char *buf, rt_size_t size, const char *fmt, va_list args);
#endif
