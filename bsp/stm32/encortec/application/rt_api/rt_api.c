#include "common.h"
#include <stdarg.h>
#include "rt_api_addr.h"
#include "rt_api_typedef.h"
#include "rttypes.h"
#include <stddef.h>

rt_err_t  rt_thread_init(struct rt_thread *thread,
                        const char       *name,
                        void (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        rt_uint32_t       stack_size,
                        rt_uint8_t        priority,
                        rt_uint32_t       tick) {
    return ((rt_thread_init_api_ptr_t)(rt_thread_init_addr))(thread, name, entry, parameter, stack_start, stack_size, priority, tick);
}

rt_err_t  rt_thread_detach(rt_thread_t thread) {
    return ((rt_thread_detach_api_ptr_t)(rt_thread_detach_addr))(thread);
}

rt_thread_t  rt_thread_create(const char *name,
                             void (*entry)(void *parameter),
                             void       *parameter,
                             rt_uint32_t stack_size,
                             rt_uint8_t  priority,
                             rt_uint32_t tick) {
    return ((rt_thread_create_api_ptr_t)(rt_thread_create_addr))(name, entry, parameter, stack_size, priority, tick);
}

rt_err_t  rt_thread_delete(rt_thread_t thread) {
    return ((rt_thread_delete_api_ptr_t)(rt_thread_delete_addr))(thread);
}

rt_err_t  rt_thread_close(rt_thread_t thread) {
    return ((rt_thread_close_api_ptr_t)(rt_thread_close_addr))(thread);
}

rt_thread_t  rt_thread_self(void) {
    return ((rt_thread_self_api_ptr_t)(rt_thread_self_addr))();
}

rt_thread_t  rt_thread_find(char *name) {
    return ((rt_thread_find_api_ptr_t)(rt_thread_find_addr))(name);
}

rt_err_t  rt_thread_startup(rt_thread_t thread) {
    return ((rt_thread_startup_api_ptr_t)(rt_thread_startup_addr))(thread);
}

rt_err_t  rt_thread_yield(void) {
    return ((rt_thread_yield_api_ptr_t)(rt_thread_yield_addr))();
}

rt_err_t  rt_thread_delay(rt_tick_t tick) {
    return ((rt_thread_delay_api_ptr_t)(rt_thread_delay_addr))(tick);
}

rt_err_t  rt_thread_delay_until(rt_tick_t *tick, rt_tick_t inc_tick) {
    return ((rt_thread_delay_until_api_ptr_t)(rt_thread_delay_until_addr))(tick, inc_tick);
}

rt_err_t rt_thread_mdelay(rt_int32_t ms) {
    return ((rt_thread_mdelay_api_ptr_t)(rt_thread_mdelay_addr))(ms);
}

rt_err_t  rt_thread_control(rt_thread_t thread, int cmd, void *arg) {
    return ((rt_thread_control_api_ptr_t)(rt_thread_control_addr))(thread, cmd, arg);
}

rt_err_t  rt_thread_suspend(rt_thread_t thread) {
    return ((rt_thread_suspend_api_ptr_t)(rt_thread_suspend_addr))(thread);
}

rt_err_t  rt_thread_suspend_with_flag(rt_thread_t thread, int suspend_flag) {
    return ((rt_thread_suspend_with_flag_api_ptr_t)(rt_thread_suspend_with_flag_addr))(thread, suspend_flag);
}

rt_err_t  rt_thread_resume(rt_thread_t thread) {
    return ((rt_thread_resume_api_ptr_t)(rt_thread_resume_addr))(thread);
}

rt_err_t  rt_thread_get_name(rt_thread_t thread, char *name, rt_uint8_t name_size) {
    return ((rt_thread_get_name_api_ptr_t)(rt_thread_get_name_addr))(thread, name, name_size);
}

void  rt_thread_alloc_sig(rt_thread_t tid) {
    return ((rt_thread_alloc_sig_api_ptr_t)(rt_thread_alloc_sig_addr))(tid);
}

void  rt_thread_free_sig(rt_thread_t tid) {
    return ((rt_thread_free_sig_api_ptr_t)(rt_thread_free_sig_addr))(tid);
}

int   rt_thread_kill(rt_thread_t tid, int sig) {
    return ((rt_thread_kill_api_ptr_t)(rt_thread_kill_addr))(tid, sig);
}

rt_base_t  rt_enter_critical(void) {
    return ((rt_enter_critical_api_ptr_t)(rt_enter_critical_addr))();
}

void  rt_exit_critical(void) {
    return ((rt_exit_critical_api_ptr_t)(rt_exit_critical_addr))();
}

void  rt_exit_critical_safe(rt_base_t critical_level) {
    return ((rt_exit_critical_safe_api_ptr_t)(rt_exit_critical_safe_addr))(critical_level);
}

rt_uint16_t  rt_critical_level(void) {
    return ((rt_critical_level_api_ptr_t)(rt_critical_level_addr))();
}

rt_err_t  rt_sem_init(rt_sem_t    sem,
                     const char *name,
                     rt_uint32_t value,
                     rt_uint8_t  flag) {
    return ((rt_sem_init_api_ptr_t)(rt_sem_init_addr))(sem, name, value, flag);
}

rt_err_t  rt_sem_detach(rt_sem_t sem) {
    return ((rt_sem_detach_api_ptr_t)(rt_sem_detach_addr))(sem);
}

rt_sem_t  rt_sem_create(const char *name, rt_uint32_t value, rt_uint8_t flag) {
    return ((rt_sem_create_api_ptr_t)(rt_sem_create_addr))(name, value, flag);
}

rt_err_t  rt_sem_delete(rt_sem_t sem) {
    return ((rt_sem_delete_api_ptr_t)(rt_sem_delete_addr))(sem);
}

rt_err_t  rt_sem_take(rt_sem_t sem, rt_int32_t time) {
    return ((rt_sem_take_api_ptr_t)(rt_sem_take_addr))(sem, time);
}

rt_err_t  rt_sem_take_interruptible(rt_sem_t sem, rt_int32_t time) {
    return ((rt_sem_take_interruptible_api_ptr_t)(rt_sem_take_interruptible_addr))(sem, time);
}

rt_err_t  rt_sem_take_killable(rt_sem_t sem, rt_int32_t time) {
    return ((rt_sem_take_killable_api_ptr_t)(rt_sem_take_killable_addr))(sem, time);
}

rt_err_t  rt_sem_trytake(rt_sem_t sem) {
    return ((rt_sem_trytake_api_ptr_t)(rt_sem_trytake_addr))(sem);
}

rt_err_t  rt_sem_release(rt_sem_t sem) {
    return ((rt_sem_release_api_ptr_t)(rt_sem_release_addr))(sem);
}

rt_err_t  rt_sem_control(rt_sem_t sem, int cmd, void *arg) {
    return ((rt_sem_control_api_ptr_t)(rt_sem_control_addr))(sem, cmd, arg);
}

rt_err_t  rt_mutex_init(rt_mutex_t mutex, const char *name, rt_uint8_t flag) {
    return ((rt_mutex_init_api_ptr_t)(rt_mutex_init_addr))(mutex, name, flag);
}

rt_err_t  rt_mutex_detach(rt_mutex_t mutex) {
    return ((rt_mutex_detach_api_ptr_t)(rt_mutex_detach_addr))(mutex);
}

rt_mutex_t  rt_mutex_create(const char *name, rt_uint8_t flag) {
    return ((rt_mutex_create_api_ptr_t)(rt_mutex_create_addr))(name, flag);
}

rt_err_t  rt_mutex_delete(rt_mutex_t mutex) {
    return ((rt_mutex_delete_api_ptr_t)(rt_mutex_delete_addr))(mutex);
}

void  rt_mutex_drop_thread(rt_mutex_t mutex, rt_thread_t thread) {
    return ((rt_mutex_drop_thread_api_ptr_t)(rt_mutex_drop_thread_addr))(mutex, thread);
}

rt_uint8_t  rt_mutex_setprioceiling(rt_mutex_t mutex, rt_uint8_t priority) {
    return ((rt_mutex_setprioceiling_api_ptr_t)(rt_mutex_setprioceiling_addr))(mutex, priority);
}

rt_uint8_t  rt_mutex_getprioceiling(rt_mutex_t mutex) {
    return ((rt_mutex_getprioceiling_api_ptr_t)(rt_mutex_getprioceiling_addr))(mutex);
}

rt_err_t  rt_mutex_take(rt_mutex_t mutex, rt_int32_t time) {
    return ((rt_mutex_take_api_ptr_t)(rt_mutex_take_addr))(mutex, time);
}

rt_err_t  rt_mutex_trytake(rt_mutex_t mutex) {
    return ((rt_mutex_trytake_api_ptr_t)(rt_mutex_trytake_addr))(mutex);
}

rt_err_t  rt_mutex_take_interruptible(rt_mutex_t mutex, rt_int32_t time) {
    return ((rt_mutex_take_interruptible_api_ptr_t)(rt_mutex_take_interruptible_addr))(mutex, time);
}

rt_err_t  rt_mutex_take_killable(rt_mutex_t mutex, rt_int32_t time) {
    return ((rt_mutex_take_killable_api_ptr_t)(rt_mutex_take_killable_addr))(mutex, time);
}

rt_err_t  rt_mutex_release(rt_mutex_t mutex) {
    return ((rt_mutex_release_api_ptr_t)(rt_mutex_release_addr))(mutex);
}

rt_err_t  rt_mutex_control(rt_mutex_t mutex, int cmd, void *arg) {
    return ((rt_mutex_control_api_ptr_t)(rt_mutex_control_addr))(mutex, cmd, arg);
}

rt_err_t  rt_event_init(rt_event_t event, const char *name, rt_uint8_t flag) {
    return ((rt_event_init_api_ptr_t)(rt_event_init_addr))(event, name, flag);
}

rt_err_t  rt_event_detach(rt_event_t event) {
    return ((rt_event_detach_api_ptr_t)(rt_event_detach_addr))(event);
}

rt_event_t  rt_event_create(const char *name, rt_uint8_t flag) {
    return ((rt_event_create_api_ptr_t)(rt_event_create_addr))(name, flag);
}

rt_err_t  rt_event_delete(rt_event_t event) {
    return ((rt_event_delete_api_ptr_t)(rt_event_delete_addr))(event);
}

rt_err_t  rt_event_send(rt_event_t event, rt_uint32_t set) {
    return ((rt_event_send_api_ptr_t)(rt_event_send_addr))(event, set);
}

rt_err_t  rt_event_recv(rt_event_t   event,
                       rt_uint32_t  set,
                       rt_uint8_t   option,
                       rt_int32_t   timeout,
                       rt_uint32_t *recved) {
    return ((rt_event_recv_api_ptr_t)(rt_event_recv_addr))(event, set, option, timeout, recved);
}

rt_err_t  rt_event_recv_interruptible(rt_event_t   event,
                       rt_uint32_t  set,
                       rt_uint8_t   option,
                       rt_int32_t   timeout,
                       rt_uint32_t *recved) {
    return ((rt_event_recv_interruptible_api_ptr_t)(rt_event_recv_interruptible_addr))(event, set, option, timeout, recved);
}

rt_err_t  rt_event_recv_killable(rt_event_t   event,
                       rt_uint32_t  set,
                       rt_uint8_t   option,
                       rt_int32_t   timeout,
                       rt_uint32_t *recved) {
    return ((rt_event_recv_killable_api_ptr_t)(rt_event_recv_killable_addr))(event, set, option, timeout, recved);
}

rt_err_t  rt_event_control(rt_event_t event, int cmd, void *arg) {
    return ((rt_event_control_api_ptr_t)(rt_event_control_addr))(event, cmd, arg);
}

rt_err_t  rt_mq_init(rt_mq_t     mq,
                    const char *name,
                    void       *msgpool,
                    rt_size_t   msg_size,
                    rt_size_t   pool_size,
                    rt_uint8_t  flag) {
    return ((rt_mq_init_api_ptr_t)(rt_mq_init_addr))(mq, name, msgpool, msg_size, pool_size, flag);
}

rt_err_t  rt_mq_detach(rt_mq_t mq) {
    return ((rt_mq_detach_api_ptr_t)(rt_mq_detach_addr))(mq);
}

rt_mq_t  rt_mq_create(const char *name,
                     rt_size_t   msg_size,
                     rt_size_t   max_msgs,
                     rt_uint8_t  flag) {
    return ((rt_mq_create_api_ptr_t)(rt_mq_create_addr))(name, msg_size, max_msgs, flag);
}

rt_err_t  rt_mq_delete(rt_mq_t mq) {
    return ((rt_mq_delete_api_ptr_t)(rt_mq_delete_addr))(mq);
}

rt_err_t  rt_mq_send(rt_mq_t mq, const void *buffer, rt_size_t size) {
    return ((rt_mq_send_api_ptr_t)(rt_mq_send_addr))(mq, buffer, size);
}

rt_err_t  rt_mq_send_interruptible(rt_mq_t mq, const void *buffer, rt_size_t size) {
    return ((rt_mq_send_interruptible_api_ptr_t)(rt_mq_send_interruptible_addr))(mq, buffer, size);
}

rt_err_t  rt_mq_send_killable(rt_mq_t mq, const void *buffer, rt_size_t size) {
    return ((rt_mq_send_killable_api_ptr_t)(rt_mq_send_killable_addr))(mq, buffer, size);
}

rt_err_t  rt_mq_send_wait(rt_mq_t     mq,
                         const void *buffer,
                         rt_size_t   size,
                         rt_int32_t  timeout) {
    return ((rt_mq_send_wait_api_ptr_t)(rt_mq_send_wait_addr))(mq, buffer, size, timeout);
}

rt_err_t  rt_mq_send_wait_interruptible(rt_mq_t     mq,
                         const void *buffer,
                         rt_size_t   size,
                         rt_int32_t  timeout) {
    return ((rt_mq_send_wait_interruptible_api_ptr_t)(rt_mq_send_wait_interruptible_addr))(mq, buffer, size, timeout);
}

rt_err_t  rt_mq_send_wait_killable(rt_mq_t     mq,
                         const void *buffer,
                         rt_size_t   size,
                         rt_int32_t  timeout) {
    return ((rt_mq_send_wait_killable_api_ptr_t)(rt_mq_send_wait_killable_addr))(mq, buffer, size, timeout);
}

rt_err_t  rt_mq_urgent(rt_mq_t mq, const void *buffer, rt_size_t size) {
    return ((rt_mq_urgent_api_ptr_t)(rt_mq_urgent_addr))(mq, buffer, size);
}

rt_ssize_t  rt_mq_recv(rt_mq_t    mq,
                    void      *buffer,
                    rt_size_t  size,
                    rt_int32_t timeout) {
    return ((rt_mq_recv_api_ptr_t)(rt_mq_recv_addr))(mq, buffer, size, timeout);
}

rt_ssize_t  rt_mq_recv_interruptible(rt_mq_t    mq,
                    void      *buffer,
                    rt_size_t  size,
                    rt_int32_t timeout) {
    return ((rt_mq_recv_interruptible_api_ptr_t)(rt_mq_recv_interruptible_addr))(mq, buffer, size, timeout);
}

rt_ssize_t  rt_mq_recv_killable(rt_mq_t    mq,
                    void      *buffer,
                    rt_size_t  size,
                    rt_int32_t timeout) {
    return ((rt_mq_recv_killable_api_ptr_t)(rt_mq_recv_killable_addr))(mq, buffer, size, timeout);
}

rt_err_t  rt_mq_control(rt_mq_t mq, int cmd, void *arg) {
    return ((rt_mq_control_api_ptr_t)(rt_mq_control_addr))(mq, cmd, arg);
}

void  rt_interrupt_enter(void) {
    return ((rt_interrupt_enter_api_ptr_t)(rt_interrupt_enter_addr))();
}

void  rt_interrupt_leave(void) {
    return ((rt_interrupt_leave_api_ptr_t)(rt_interrupt_leave_addr))();
}

void  rt_assert_handler(const char *ex_string, const char *func, rt_size_t line) {
    return ((rt_assert_handler_api_ptr_t)(rt_assert_handler_addr))(ex_string, func, line);
}

void  rt_kputs(const char *str) {
    return ((rt_kputs_api_ptr_t)(rt_kputs_addr))(str);
}

int rt_kprintf(const char *fmt, ...) {
#if APP_LOG_BUF_SIZE
    va_list args;
    int len;
    static char rt_kprintf_app_buf[APP_LOG_BUF_SIZE];

    va_start(args, fmt);
    rt_vsnprintf(rt_kprintf_app_buf, sizeof(rt_kprintf_app_buf) - 1, fmt, args);
    len = ((rt_kprintf_api_ptr_t)(rt_kprintf_addr))("%s", rt_kprintf_app_buf);
    va_end(args);

    return len;
#endif
}

void * rt_malloc(rt_size_t size) {
    return ((rt_malloc_api_ptr_t)(rt_malloc_addr))(size);
}

void rt_free(void *ptr) {
    return ((rt_free_api_ptr_t)(rt_free_addr))(ptr);
}

void * rt_realloc(void *ptr, rt_size_t newsize) {
    return ((rt_realloc_api_ptr_t)(rt_realloc_addr))(ptr, newsize);
}

void * rt_calloc(rt_size_t count, rt_size_t size) {
    return ((rt_calloc_api_ptr_t)(rt_calloc_addr))(count, size);
}

void  rt_memory_info(rt_size_t *total,
                            rt_size_t *used,
                            rt_size_t *max_used) {
    return ((rt_memory_info_api_ptr_t)(rt_memory_info_addr))(total, used, max_used);
}

int  rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args) {
    return ((rt_vsnprintf_api_ptr_t)(rt_vsnprintf_addr))(buf, size, fmt, args);
}

