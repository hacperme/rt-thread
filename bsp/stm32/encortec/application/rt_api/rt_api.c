#include "common.h"
#include <stdio.h>
#include <stdarg.h>
#include "rt_api_addr.h"
#include "rt_api_typedef.h"
#include "rttypes.h"
#include "logging.h"
#include <stddef.h>
#include <sys/stat.h>

rt_err_t rt_thread_init(struct rt_thread *thread, const char *name, void (*entry)(void *parameter), void *parameter, void *stack_start, rt_uint32_t stack_size, rt_uint8_t priority, rt_uint32_t tick) {
    return ((rt_thread_init_api_ptr_t)(rt_thread_init_addr))(thread, name, entry, parameter, stack_start, stack_size, priority, tick);
}

rt_err_t rt_thread_detach(rt_thread_t thread) {
    return ((rt_thread_detach_api_ptr_t)(rt_thread_detach_addr))(thread);
}

rt_thread_t rt_thread_create(const char *name, void (*entry)(void *parameter), void *parameter, rt_uint32_t stack_size, rt_uint8_t priority, rt_uint32_t tick) {
    return ((rt_thread_create_api_ptr_t)(rt_thread_create_addr))(name, entry, parameter, stack_size, priority, tick);
}

rt_err_t rt_thread_delete(rt_thread_t thread) {
    return ((rt_thread_delete_api_ptr_t)(rt_thread_delete_addr))(thread);
}

rt_err_t rt_thread_close(rt_thread_t thread) {
    return ((rt_thread_close_api_ptr_t)(rt_thread_close_addr))(thread);
}

rt_thread_t rt_thread_self(void) {
    return ((rt_thread_self_api_ptr_t)(rt_thread_self_addr))();
}

rt_thread_t rt_thread_find(char *name) {
    return ((rt_thread_find_api_ptr_t)(rt_thread_find_addr))(name);
}

rt_err_t rt_thread_startup(rt_thread_t thread) {
    return ((rt_thread_startup_api_ptr_t)(rt_thread_startup_addr))(thread);
}

rt_err_t rt_thread_yield(void) {
    return ((rt_thread_yield_api_ptr_t)(rt_thread_yield_addr))();
}

rt_err_t rt_thread_delay(rt_tick_t tick) {
    return ((rt_thread_delay_api_ptr_t)(rt_thread_delay_addr))(tick);
}

rt_err_t rt_thread_delay_until(rt_tick_t *tick, rt_tick_t inc_tick) {
    return ((rt_thread_delay_until_api_ptr_t)(rt_thread_delay_until_addr))(tick, inc_tick);
}

rt_err_t rt_thread_mdelay(rt_int32_t ms) {
    return ((rt_thread_mdelay_api_ptr_t)(rt_thread_mdelay_addr))(ms);
}

rt_err_t rt_thread_control(rt_thread_t thread, int cmd, void *arg) {
    return ((rt_thread_control_api_ptr_t)(rt_thread_control_addr))(thread, cmd, arg);
}

rt_err_t rt_thread_suspend(rt_thread_t thread) {
    return ((rt_thread_suspend_api_ptr_t)(rt_thread_suspend_addr))(thread);
}

rt_err_t rt_thread_suspend_with_flag(rt_thread_t thread, int suspend_flag) {
    return ((rt_thread_suspend_with_flag_api_ptr_t)(rt_thread_suspend_with_flag_addr))(thread, suspend_flag);
}

rt_err_t rt_thread_resume(rt_thread_t thread) {
    return ((rt_thread_resume_api_ptr_t)(rt_thread_resume_addr))(thread);
}

rt_err_t rt_thread_get_name(rt_thread_t thread, char *name, rt_uint8_t name_size) {
    return ((rt_thread_get_name_api_ptr_t)(rt_thread_get_name_addr))(thread, name, name_size);
}

void rt_thread_alloc_sig(rt_thread_t tid) {
    return ((rt_thread_alloc_sig_api_ptr_t)(rt_thread_alloc_sig_addr))(tid);
}

void rt_thread_free_sig(rt_thread_t tid) {
    return ((rt_thread_free_sig_api_ptr_t)(rt_thread_free_sig_addr))(tid);
}

int rt_thread_kill(rt_thread_t tid, int sig) {
    return ((rt_thread_kill_api_ptr_t)(rt_thread_kill_addr))(tid, sig);
}

rt_base_t rt_enter_critical(void) {
    return ((rt_enter_critical_api_ptr_t)(rt_enter_critical_addr))();
}

void rt_exit_critical(void) {
    return ((rt_exit_critical_api_ptr_t)(rt_exit_critical_addr))();
}

void rt_exit_critical_safe(rt_base_t critical_level) {
    return ((rt_exit_critical_safe_api_ptr_t)(rt_exit_critical_safe_addr))(critical_level);
}

rt_uint16_t rt_critical_level(void) {
    return ((rt_critical_level_api_ptr_t)(rt_critical_level_addr))();
}

rt_err_t rt_sem_init(rt_sem_t sem, const char *name, rt_uint32_t value, rt_uint8_t flag) {
    return ((rt_sem_init_api_ptr_t)(rt_sem_init_addr))(sem, name, value, flag);
}

rt_err_t rt_sem_detach(rt_sem_t sem) {
    return ((rt_sem_detach_api_ptr_t)(rt_sem_detach_addr))(sem);
}

rt_sem_t rt_sem_create(const char *name, rt_uint32_t value, rt_uint8_t flag) {
    return ((rt_sem_create_api_ptr_t)(rt_sem_create_addr))(name, value, flag);
}

rt_err_t rt_sem_delete(rt_sem_t sem) {
    return ((rt_sem_delete_api_ptr_t)(rt_sem_delete_addr))(sem);
}

rt_err_t rt_sem_take(rt_sem_t sem, rt_int32_t time) {
    return ((rt_sem_take_api_ptr_t)(rt_sem_take_addr))(sem, time);
}

rt_err_t rt_sem_take_interruptible(rt_sem_t sem, rt_int32_t time) {
    return ((rt_sem_take_interruptible_api_ptr_t)(rt_sem_take_interruptible_addr))(sem, time);
}

rt_err_t rt_sem_take_killable(rt_sem_t sem, rt_int32_t time) {
    return ((rt_sem_take_killable_api_ptr_t)(rt_sem_take_killable_addr))(sem, time);
}

rt_err_t rt_sem_trytake(rt_sem_t sem) {
    return ((rt_sem_trytake_api_ptr_t)(rt_sem_trytake_addr))(sem);
}

rt_err_t rt_sem_release(rt_sem_t sem) {
    return ((rt_sem_release_api_ptr_t)(rt_sem_release_addr))(sem);
}

rt_err_t rt_sem_control(rt_sem_t sem, int cmd, void *arg) {
    return ((rt_sem_control_api_ptr_t)(rt_sem_control_addr))(sem, cmd, arg);
}

rt_err_t rt_mutex_init(rt_mutex_t mutex, const char *name, rt_uint8_t flag) {
    return ((rt_mutex_init_api_ptr_t)(rt_mutex_init_addr))(mutex, name, flag);
}

rt_err_t rt_mutex_detach(rt_mutex_t mutex) {
    return ((rt_mutex_detach_api_ptr_t)(rt_mutex_detach_addr))(mutex);
}

rt_mutex_t rt_mutex_create(const char *name, rt_uint8_t flag) {
    return ((rt_mutex_create_api_ptr_t)(rt_mutex_create_addr))(name, flag);
}

rt_err_t rt_mutex_delete(rt_mutex_t mutex) {
    return ((rt_mutex_delete_api_ptr_t)(rt_mutex_delete_addr))(mutex);
}

void rt_mutex_drop_thread(rt_mutex_t mutex, rt_thread_t thread) {
    return ((rt_mutex_drop_thread_api_ptr_t)(rt_mutex_drop_thread_addr))(mutex, thread);
}

rt_uint8_t rt_mutex_setprioceiling(rt_mutex_t mutex, rt_uint8_t priority) {
    return ((rt_mutex_setprioceiling_api_ptr_t)(rt_mutex_setprioceiling_addr))(mutex, priority);
}

rt_uint8_t rt_mutex_getprioceiling(rt_mutex_t mutex) {
    return ((rt_mutex_getprioceiling_api_ptr_t)(rt_mutex_getprioceiling_addr))(mutex);
}

rt_err_t rt_mutex_take(rt_mutex_t mutex, rt_int32_t time) {
    return ((rt_mutex_take_api_ptr_t)(rt_mutex_take_addr))(mutex, time);
}

rt_err_t rt_mutex_trytake(rt_mutex_t mutex) {
    return ((rt_mutex_trytake_api_ptr_t)(rt_mutex_trytake_addr))(mutex);
}

rt_err_t rt_mutex_take_interruptible(rt_mutex_t mutex, rt_int32_t time) {
    return ((rt_mutex_take_interruptible_api_ptr_t)(rt_mutex_take_interruptible_addr))(mutex, time);
}

rt_err_t rt_mutex_take_killable(rt_mutex_t mutex, rt_int32_t time) {
    return ((rt_mutex_take_killable_api_ptr_t)(rt_mutex_take_killable_addr))(mutex, time);
}

rt_err_t rt_mutex_release(rt_mutex_t mutex) {
    return ((rt_mutex_release_api_ptr_t)(rt_mutex_release_addr))(mutex);
}

rt_err_t rt_mutex_control(rt_mutex_t mutex, int cmd, void *arg) {
    return ((rt_mutex_control_api_ptr_t)(rt_mutex_control_addr))(mutex, cmd, arg);
}

rt_err_t rt_event_init(rt_event_t event, const char *name, rt_uint8_t flag) {
    return ((rt_event_init_api_ptr_t)(rt_event_init_addr))(event, name, flag);
}

rt_err_t rt_event_detach(rt_event_t event) {
    return ((rt_event_detach_api_ptr_t)(rt_event_detach_addr))(event);
}

rt_event_t rt_event_create(const char *name, rt_uint8_t flag) {
    return ((rt_event_create_api_ptr_t)(rt_event_create_addr))(name, flag);
}

rt_err_t rt_event_delete(rt_event_t event) {
    return ((rt_event_delete_api_ptr_t)(rt_event_delete_addr))(event);
}

rt_err_t rt_event_send(rt_event_t event, rt_uint32_t set) {
    return ((rt_event_send_api_ptr_t)(rt_event_send_addr))(event, set);
}

rt_err_t rt_event_recv(rt_event_t event, rt_uint32_t set, rt_uint8_t option, rt_int32_t timeout, rt_uint32_t *recved) {
    return ((rt_event_recv_api_ptr_t)(rt_event_recv_addr))(event, set, option, timeout, recved);
}

rt_err_t rt_event_recv_interruptible(rt_event_t event, rt_uint32_t set, rt_uint8_t option, rt_int32_t timeout, rt_uint32_t *recved) {
    return ((rt_event_recv_interruptible_api_ptr_t)(rt_event_recv_interruptible_addr))(event, set, option, timeout, recved);
}

rt_err_t rt_event_recv_killable(rt_event_t event, rt_uint32_t set, rt_uint8_t option, rt_int32_t timeout, rt_uint32_t *recved) {
    return ((rt_event_recv_killable_api_ptr_t)(rt_event_recv_killable_addr))(event, set, option, timeout, recved);
}

rt_err_t rt_event_control(rt_event_t event, int cmd, void *arg) {
    return ((rt_event_control_api_ptr_t)(rt_event_control_addr))(event, cmd, arg);
}

rt_err_t rt_mq_init(rt_mq_t mq, const char *name, void *msgpool, rt_size_t msg_size, rt_size_t pool_size, rt_uint8_t flag) {
    return ((rt_mq_init_api_ptr_t)(rt_mq_init_addr))(mq, name, msgpool, msg_size, pool_size, flag);
}

rt_err_t rt_mq_detach(rt_mq_t mq) {
    return ((rt_mq_detach_api_ptr_t)(rt_mq_detach_addr))(mq);
}

rt_mq_t rt_mq_create(const char *name, rt_size_t msg_size, rt_size_t max_msgs, rt_uint8_t flag) {
    return ((rt_mq_create_api_ptr_t)(rt_mq_create_addr))(name, msg_size, max_msgs, flag);
}

rt_err_t rt_mq_delete(rt_mq_t mq) {
    return ((rt_mq_delete_api_ptr_t)(rt_mq_delete_addr))(mq);
}

rt_err_t rt_mq_send(rt_mq_t mq, const void *buffer, rt_size_t size) {
    return ((rt_mq_send_api_ptr_t)(rt_mq_send_addr))(mq, buffer, size);
}

rt_err_t rt_mq_send_interruptible(rt_mq_t mq, const void *buffer, rt_size_t size) {
    return ((rt_mq_send_interruptible_api_ptr_t)(rt_mq_send_interruptible_addr))(mq, buffer, size);
}

rt_err_t rt_mq_send_killable(rt_mq_t mq, const void *buffer, rt_size_t size) {
    return ((rt_mq_send_killable_api_ptr_t)(rt_mq_send_killable_addr))(mq, buffer, size);
}

rt_err_t rt_mq_send_wait(rt_mq_t mq, const void *buffer, rt_size_t size, rt_int32_t timeout) {
    return ((rt_mq_send_wait_api_ptr_t)(rt_mq_send_wait_addr))(mq, buffer, size, timeout);
}

rt_err_t rt_mq_send_wait_interruptible(rt_mq_t mq, const void *buffer, rt_size_t size, rt_int32_t timeout) {
    return ((rt_mq_send_wait_interruptible_api_ptr_t)(rt_mq_send_wait_interruptible_addr))(mq, buffer, size, timeout);
}

rt_err_t rt_mq_send_wait_killable(rt_mq_t mq, const void *buffer, rt_size_t size, rt_int32_t timeout) {
    return ((rt_mq_send_wait_killable_api_ptr_t)(rt_mq_send_wait_killable_addr))(mq, buffer, size, timeout);
}

rt_err_t rt_mq_urgent(rt_mq_t mq, const void *buffer, rt_size_t size) {
    return ((rt_mq_urgent_api_ptr_t)(rt_mq_urgent_addr))(mq, buffer, size);
}

rt_ssize_t rt_mq_recv(rt_mq_t mq, void *buffer, rt_size_t size, rt_int32_t timeout) {
    return ((rt_mq_recv_api_ptr_t)(rt_mq_recv_addr))(mq, buffer, size, timeout);
}

rt_ssize_t rt_mq_recv_interruptible(rt_mq_t mq, void *buffer, rt_size_t size, rt_int32_t timeout) {
    return ((rt_mq_recv_interruptible_api_ptr_t)(rt_mq_recv_interruptible_addr))(mq, buffer, size, timeout);
}

rt_ssize_t rt_mq_recv_killable(rt_mq_t mq, void *buffer, rt_size_t size, rt_int32_t timeout) {
    return ((rt_mq_recv_killable_api_ptr_t)(rt_mq_recv_killable_addr))(mq, buffer, size, timeout);
}

rt_err_t rt_mq_control(rt_mq_t mq, int cmd, void *arg) {
    return ((rt_mq_control_api_ptr_t)(rt_mq_control_addr))(mq, cmd, arg);
}

void rt_interrupt_enter(void) {
    return ((rt_interrupt_enter_api_ptr_t)(rt_interrupt_enter_addr))();
}

void rt_interrupt_leave(void) {
    return ((rt_interrupt_leave_api_ptr_t)(rt_interrupt_leave_addr))();
}

void rt_assert_handler(const char *ex_string, const char *func, rt_size_t line) {
    return ((rt_assert_handler_api_ptr_t)(rt_assert_handler_addr))(ex_string, func, line);
}

void rt_kputs(const char *str) {
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
#else
    return 0;
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

void * _malloc_r(struct _reent *ptr, size_t size) {
    return ((_malloc_r_api_ptr_t)(_malloc_r_addr))(ptr, size);
}

void * _realloc_r(struct _reent *ptr, void *old, size_t newlen) {
    return ((_realloc_r_api_ptr_t)(_realloc_r_addr))(ptr, old, newlen);
}

void * _calloc_r(struct _reent *ptr, size_t size, size_t len) {
    return ((_calloc_r_api_ptr_t)(_calloc_r_addr))(ptr, size, len);
}

void _free_r(struct _reent *ptr, void *addr) {
    return ((_free_r_api_ptr_t)(_free_r_addr))(ptr, addr);
}

int _getpid_r(struct _reent *ptr) {
    return ((_getpid_r_api_ptr_t)(_getpid_r_addr))(ptr);
}

int _close_r(struct _reent *ptr, int fd) {
    return ((_close_r_api_ptr_t)(_close_r_addr))(ptr, fd);
}

int _execve_r(struct _reent *ptr, const char * name, char *const *argv, char *const *env) {
    return ((_execve_r_api_ptr_t)(_execve_r_addr))(ptr, name, argv, env);
}

int _fcntl_r(struct _reent *ptr, int fd, int cmd, int arg) {
    return ((_fcntl_r_api_ptr_t)(_fcntl_r_addr))(ptr, fd, cmd, arg);
}

int _fork_r(struct _reent *ptr) {
    return ((_fork_r_api_ptr_t)(_fork_r_addr))(ptr);
}

int _fstat_r(struct _reent *ptr, int fd, struct stat *pstat) {
    return ((_fstat_r_api_ptr_t)(_fstat_r_addr))(ptr, fd, pstat);
}

int _isatty_r(struct _reent *ptr, int fd) {
    return ((_isatty_r_api_ptr_t)(_isatty_r_addr))(ptr, fd);
}

int _kill_r(struct _reent *ptr, int pid, int sig) {
    return ((_kill_r_api_ptr_t)(_kill_r_addr))(ptr, pid, sig);
}

int _link_r(struct _reent *ptr, const char *old, const char *new) {
    return ((_link_r_api_ptr_t)(_link_r_addr))(ptr, old, new);
}

int _wait_r(struct _reent *ptr, int *status) {
    return ((_wait_r_api_ptr_t)(_wait_r_addr))(ptr, status);
}

_off_t _lseek_r(struct _reent *ptr, int fd, _off_t pos, int whence) {
    return ((_lseek_r_api_ptr_t)(_lseek_r_addr))(ptr, fd, pos, whence);
}

int _mkdir_r(struct _reent *ptr, const char *name, int mode) {
    return ((_mkdir_r_api_ptr_t)(_mkdir_r_addr))(ptr, name, mode);
}

int _open_r(struct _reent *ptr, const char *file, int flags, int mode) {
    return ((_open_r_api_ptr_t)(_open_r_addr))(ptr, file, flags, mode);
}

_ssize_t _read_r(struct _reent *ptr, int fd, void *buf, size_t nbytes) {
    return ((_read_r_api_ptr_t)(_read_r_addr))(ptr, fd, buf, nbytes);
}

int _rename_r(struct _reent *ptr, const char *old, const char *new) {
    return ((_rename_r_api_ptr_t)(_rename_r_addr))(ptr, old, new);
}

int _stat_r(struct _reent *ptr, const char *file, struct stat *pstat) {
    return ((_stat_r_api_ptr_t)(_stat_r_addr))(ptr, file, pstat);
}

int _unlink_r(struct _reent *ptr, const char *file) {
    return ((_unlink_r_api_ptr_t)(_unlink_r_addr))(ptr, file);
}

_ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t nbytes) {
    return ((_write_r_api_ptr_t)(_write_r_addr))(ptr, fd, buf, nbytes);
}

__attribute__((noreturn)) void _exit(int status) {
    ((_exit_api_ptr_t)(_exit_addr))(status);
}

void rt_memory_info(rt_size_t *total, rt_size_t *used, rt_size_t *max_used) {
    return ((rt_memory_info_api_ptr_t)(rt_memory_info_addr))(total, used, max_used);
}

int rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args) {
    return ((rt_vsnprintf_api_ptr_t)(rt_vsnprintf_addr))(buf, size, fmt, args);
}

rt_tick_t rt_tick_get(void) {
    return ((rt_tick_get_api_ptr_t)(rt_tick_get_addr))();
}

void rt_tick_set(rt_tick_t tick) {
    return ((rt_tick_set_api_ptr_t)(rt_tick_set_addr))(tick);
}

void rt_tick_increase(void) {
    return ((rt_tick_increase_api_ptr_t)(rt_tick_increase_addr))();
}

rt_tick_t rt_tick_from_millisecond(rt_int32_t ms) {
    return ((rt_tick_from_millisecond_api_ptr_t)(rt_tick_from_millisecond_addr))(ms);
}

rt_tick_t rt_tick_get_millisecond(void) {
    return ((rt_tick_get_millisecond_api_ptr_t)(rt_tick_get_millisecond_addr))();
}

void rt_system_timer_init(void) {
    return ((rt_system_timer_init_api_ptr_t)(rt_system_timer_init_addr))();
}

void rt_system_timer_thread_init(void) {
    return ((rt_system_timer_thread_init_api_ptr_t)(rt_system_timer_thread_init_addr))();
}

void rt_timer_init(rt_timer_t timer, const char *name, void (*timeout)(void *parameter), void *parameter, rt_tick_t time, rt_uint8_t flag) {
    return ((rt_timer_init_api_ptr_t)(rt_timer_init_addr))(timer, name, timeout, parameter, time, flag);
}

rt_err_t rt_timer_detach(rt_timer_t timer) {
    return ((rt_timer_detach_api_ptr_t)(rt_timer_detach_addr))(timer);
}

rt_timer_t rt_timer_create(const char *name, void (*timeout)(void *parameter), void *parameter, rt_tick_t time, rt_uint8_t flag) {
    return ((rt_timer_create_api_ptr_t)(rt_timer_create_addr))(name, timeout, parameter, time, flag);
}

rt_err_t rt_timer_delete(rt_timer_t timer) {
    return ((rt_timer_delete_api_ptr_t)(rt_timer_delete_addr))(timer);
}

rt_err_t rt_timer_start(rt_timer_t timer) {
    return ((rt_timer_start_api_ptr_t)(rt_timer_start_addr))(timer);
}

rt_err_t rt_timer_stop(rt_timer_t timer) {
    return ((rt_timer_stop_api_ptr_t)(rt_timer_stop_addr))(timer);
}

rt_err_t rt_timer_control(rt_timer_t timer, int cmd, void *arg) {
    return ((rt_timer_control_api_ptr_t)(rt_timer_control_addr))(timer, cmd, arg);
}

rt_tick_t rt_timer_next_timeout_tick(void) {
    return ((rt_timer_next_timeout_tick_api_ptr_t)(rt_timer_next_timeout_tick_addr))();
}

void rt_timer_check(void) {
    return ((rt_timer_check_api_ptr_t)(rt_timer_check_addr))();
}

void rt_system_scheduler_init(void) {
    return ((rt_system_scheduler_init_api_ptr_t)(rt_system_scheduler_init_addr))();
}

void rt_system_scheduler_start(void) {
    return ((rt_system_scheduler_start_api_ptr_t)(rt_system_scheduler_start_addr))();
}

void rt_schedule(void) {
    return ((rt_schedule_api_ptr_t)(rt_schedule_addr))();
}

void rt_signal_mask(int signo) {
    return ((rt_signal_mask_api_ptr_t)(rt_signal_mask_addr))(signo);
}

void rt_signal_unmask(int signo) {
    return ((rt_signal_unmask_api_ptr_t)(rt_signal_unmask_addr))(signo);
}

rt_sighandler_t rt_signal_install(int signo, rt_sighandler_t handler) {
    return ((rt_signal_install_api_ptr_t)(rt_signal_install_addr))(signo, handler);
}

int rt_signal_wait(const rt_sigset_t *set, rt_siginfo_t *si, rt_int32_t timeout) {
    return ((rt_signal_wait_api_ptr_t)(rt_signal_wait_addr))(set, si, timeout);
}

int rt_system_signal_init(void) {
    return ((rt_system_signal_init_api_ptr_t)(rt_system_signal_init_addr))();
}

void * rt_malloc_align(rt_size_t size, rt_size_t align) {
    return ((rt_malloc_align_api_ptr_t)(rt_malloc_align_addr))(size, align);
}

void rt_free_align(void *ptr) {
    return ((rt_free_align_api_ptr_t)(rt_free_align_addr))(ptr);
}

rt_err_t rt_mb_init(rt_mailbox_t mb, const char *name, void *msgpool, rt_size_t size, rt_uint8_t flag) {
    return ((rt_mb_init_api_ptr_t)(rt_mb_init_addr))(mb, name, msgpool, size, flag);
}

rt_err_t rt_mb_detach(rt_mailbox_t mb) {
    return ((rt_mb_detach_api_ptr_t)(rt_mb_detach_addr))(mb);
}

rt_mailbox_t rt_mb_create(const char *name, rt_size_t size, rt_uint8_t flag) {
    return ((rt_mb_create_api_ptr_t)(rt_mb_create_addr))(name, size, flag);
}

rt_err_t rt_mb_delete(rt_mailbox_t mb) {
    return ((rt_mb_delete_api_ptr_t)(rt_mb_delete_addr))(mb);
}

rt_err_t rt_mb_send(rt_mailbox_t mb, rt_ubase_t value) {
    return ((rt_mb_send_api_ptr_t)(rt_mb_send_addr))(mb, value);
}

rt_err_t rt_mb_send_interruptible(rt_mailbox_t mb, rt_ubase_t value) {
    return ((rt_mb_send_interruptible_api_ptr_t)(rt_mb_send_interruptible_addr))(mb, value);
}

rt_err_t rt_mb_send_killable(rt_mailbox_t mb, rt_ubase_t value) {
    return ((rt_mb_send_killable_api_ptr_t)(rt_mb_send_killable_addr))(mb, value);
}

rt_err_t rt_mb_send_wait(rt_mailbox_t mb, rt_ubase_t value, rt_int32_t timeout) {
    return ((rt_mb_send_wait_api_ptr_t)(rt_mb_send_wait_addr))(mb, value, timeout);
}

rt_err_t rt_mb_send_wait_interruptible(rt_mailbox_t mb, rt_ubase_t value, rt_int32_t timeout) {
    return ((rt_mb_send_wait_interruptible_api_ptr_t)(rt_mb_send_wait_interruptible_addr))(mb, value, timeout);
}

rt_err_t rt_mb_send_wait_killable(rt_mailbox_t mb, rt_ubase_t value, rt_int32_t timeout) {
    return ((rt_mb_send_wait_killable_api_ptr_t)(rt_mb_send_wait_killable_addr))(mb, value, timeout);
}

rt_err_t rt_mb_urgent(rt_mailbox_t mb, rt_ubase_t value) {
    return ((rt_mb_urgent_api_ptr_t)(rt_mb_urgent_addr))(mb, value);
}

rt_err_t rt_mb_recv(rt_mailbox_t mb, rt_ubase_t *value, rt_int32_t timeout) {
    return ((rt_mb_recv_api_ptr_t)(rt_mb_recv_addr))(mb, value, timeout);
}

rt_err_t rt_mb_recv_interruptible(rt_mailbox_t mb, rt_ubase_t *value, rt_int32_t timeout) {
    return ((rt_mb_recv_interruptible_api_ptr_t)(rt_mb_recv_interruptible_addr))(mb, value, timeout);
}

rt_err_t rt_mb_recv_killable(rt_mailbox_t mb, rt_ubase_t *value, rt_int32_t timeout) {
    return ((rt_mb_recv_killable_api_ptr_t)(rt_mb_recv_killable_addr))(mb, value, timeout);
}

rt_err_t rt_mb_control(rt_mailbox_t mb, int cmd, void *arg) {
    return ((rt_mb_control_api_ptr_t)(rt_mb_control_addr))(mb, cmd, arg);
}

rt_device_t rt_device_find(const char *name) {
    return ((rt_device_find_api_ptr_t)(rt_device_find_addr))(name);
}

rt_err_t rt_device_register(rt_device_t dev, const char *name, rt_uint16_t flags) {
    return ((rt_device_register_api_ptr_t)(rt_device_register_addr))(dev, name, flags);
}

rt_err_t rt_device_unregister(rt_device_t dev) {
    return ((rt_device_unregister_api_ptr_t)(rt_device_unregister_addr))(dev);
}

rt_device_t rt_device_create(int type, int attach_size) {
    return ((rt_device_create_api_ptr_t)(rt_device_create_addr))(type, attach_size);
}

void rt_device_destroy(rt_device_t device) {
    return ((rt_device_destroy_api_ptr_t)(rt_device_destroy_addr))(device);
}

rt_err_t rt_device_set_rx_indicate(rt_device_t dev, rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size)) {
    return ((rt_device_set_rx_indicate_api_ptr_t)(rt_device_set_rx_indicate_addr))(dev, rx_ind);
}

rt_err_t rt_device_set_tx_complete(rt_device_t dev, rt_err_t (*tx_done)(rt_device_t dev, void *buffer)) {
    return ((rt_device_set_tx_complete_api_ptr_t)(rt_device_set_tx_complete_addr))(dev, tx_done);
}

rt_err_t rt_device_init(rt_device_t dev) {
    return ((rt_device_init_api_ptr_t)(rt_device_init_addr))(dev);
}

rt_err_t rt_device_open(rt_device_t dev, rt_uint16_t oflag) {
    return ((rt_device_open_api_ptr_t)(rt_device_open_addr))(dev, oflag);
}

rt_err_t rt_device_close(rt_device_t dev) {
    return ((rt_device_close_api_ptr_t)(rt_device_close_addr))(dev);
}

rt_ssize_t rt_device_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size) {
    return ((rt_device_read_api_ptr_t)(rt_device_read_addr))(dev, pos, buffer, size);
}

rt_ssize_t rt_device_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size) {
    return ((rt_device_write_api_ptr_t)(rt_device_write_addr))(dev, pos, buffer, size);
}

rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg) {
    return ((rt_device_control_api_ptr_t)(rt_device_control_addr))(dev, cmd, arg);
}

rt_uint32_t rt_adc_read(rt_adc_device_t dev, rt_int8_t channel) {
    return ((rt_adc_read_api_ptr_t)(rt_adc_read_addr))(dev, channel);
}

rt_err_t rt_adc_enable(rt_adc_device_t dev, rt_int8_t channel) {
    return ((rt_adc_enable_api_ptr_t)(rt_adc_enable_addr))(dev, channel);
}

rt_err_t rt_adc_disable(rt_adc_device_t dev, rt_int8_t channel) {
    return ((rt_adc_disable_api_ptr_t)(rt_adc_disable_addr))(dev, channel);
}

rt_int16_t rt_adc_voltage(rt_adc_device_t dev, rt_int8_t channel) {
    return ((rt_adc_voltage_api_ptr_t)(rt_adc_voltage_addr))(dev, channel);
}

rt_alarm_t rt_alarm_create(rt_alarm_callback_t callback, struct rt_alarm_setup *setup) {
    return ((rt_alarm_create_api_ptr_t)(rt_alarm_create_addr))(callback, setup);
}

rt_err_t rt_alarm_control(rt_alarm_t alarm, int cmd, void *arg) {
    return ((rt_alarm_control_api_ptr_t)(rt_alarm_control_addr))(alarm, cmd, arg);
}

void rt_alarm_update(rt_device_t dev, rt_uint32_t event) {
    return ((rt_alarm_update_api_ptr_t)(rt_alarm_update_addr))(dev, event);
}

rt_err_t rt_alarm_delete(rt_alarm_t alarm) {
    return ((rt_alarm_delete_api_ptr_t)(rt_alarm_delete_addr))(alarm);
}

rt_err_t rt_alarm_start(rt_alarm_t alarm) {
    return ((rt_alarm_start_api_ptr_t)(rt_alarm_start_addr))(alarm);
}

rt_err_t rt_alarm_stop(rt_alarm_t alarm) {
    return ((rt_alarm_stop_api_ptr_t)(rt_alarm_stop_addr))(alarm);
}

int rt_alarm_system_init(void) {
    return ((rt_alarm_system_init_api_ptr_t)(rt_alarm_system_init_addr))();
}

struct rt_hwcrypto_ctx * rt_hwcrypto_crc_create(struct rt_hwcrypto_device *device, hwcrypto_crc_mode mode) {
    return ((rt_hwcrypto_crc_create_api_ptr_t)(rt_hwcrypto_crc_create_addr))(device, mode);
}

void rt_hwcrypto_crc_destroy(struct rt_hwcrypto_ctx *ctx) {
    return ((rt_hwcrypto_crc_destroy_api_ptr_t)(rt_hwcrypto_crc_destroy_addr))(ctx);
}

rt_uint32_t rt_hwcrypto_crc_update(struct rt_hwcrypto_ctx *ctx, const rt_uint8_t *input, rt_size_t length) {
    return ((rt_hwcrypto_crc_update_api_ptr_t)(rt_hwcrypto_crc_update_addr))(ctx, input, length);
}

void rt_hwcrypto_crc_cfg(struct rt_hwcrypto_ctx *ctx, struct hwcrypto_crc_cfg *cfg) {
    return ((rt_hwcrypto_crc_cfg_api_ptr_t)(rt_hwcrypto_crc_cfg_addr))(ctx, cfg);
}

struct rt_hwcrypto_device * rt_hwcrypto_dev_default(void) {
    return ((rt_hwcrypto_dev_default_api_ptr_t)(rt_hwcrypto_dev_default_addr))();
}

rt_err_t rt_hw_adc_register(rt_adc_device_t adc,const char *name, const struct rt_adc_ops *ops, const void *user_data) {
    return ((rt_hw_adc_register_api_ptr_t)(rt_hw_adc_register_addr))(adc, name, ops, user_data);
}

rt_err_t rt_i2c_bus_device_register(struct rt_i2c_bus_device *bus, const char *bus_name) {
    return ((rt_i2c_bus_device_register_api_ptr_t)(rt_i2c_bus_device_register_addr))(bus, bus_name);
}

struct rt_i2c_bus_device * rt_i2c_bus_device_find(const char *bus_name) {
    return ((rt_i2c_bus_device_find_api_ptr_t)(rt_i2c_bus_device_find_addr))(bus_name);
}

rt_ssize_t rt_i2c_transfer(struct rt_i2c_bus_device *bus, struct rt_i2c_msg msgs[], rt_uint32_t num) {
    return ((rt_i2c_transfer_api_ptr_t)(rt_i2c_transfer_addr))(bus, msgs, num);
}

rt_err_t rt_i2c_control(struct rt_i2c_bus_device *bus, int cmd, void *args) {
    return ((rt_i2c_control_api_ptr_t)(rt_i2c_control_addr))(bus, cmd, args);
}

rt_ssize_t rt_i2c_master_send(struct rt_i2c_bus_device *bus, rt_uint16_t addr, rt_uint16_t flags, const rt_uint8_t *buf, rt_uint32_t count) {
    return ((rt_i2c_master_send_api_ptr_t)(rt_i2c_master_send_addr))(bus, addr, flags, buf, count);
}

rt_ssize_t rt_i2c_master_recv(struct rt_i2c_bus_device *bus, rt_uint16_t addr, rt_uint16_t flags, rt_uint8_t *buf, rt_uint32_t count) {
    return ((rt_i2c_master_recv_api_ptr_t)(rt_i2c_master_recv_addr))(bus, addr, flags, buf, count);
}

int rt_device_pin_register(const char *name, const struct rt_pin_ops *ops, void *user_data) {
    return ((rt_device_pin_register_api_ptr_t)(rt_device_pin_register_addr))(name, ops, user_data);
}

void rt_pin_mode(rt_base_t pin, rt_uint8_t mode) {
    return ((rt_pin_mode_api_ptr_t)(rt_pin_mode_addr))(pin, mode);
}

void rt_pin_write(rt_base_t pin, rt_ssize_t value) {
    return ((rt_pin_write_api_ptr_t)(rt_pin_write_addr))(pin, value);
}

rt_ssize_t rt_pin_read(rt_base_t pin) {
    return ((rt_pin_read_api_ptr_t)(rt_pin_read_addr))(pin);
}

rt_base_t rt_pin_get(const char *name) {
    return ((rt_pin_get_api_ptr_t)(rt_pin_get_addr))(name);
}

rt_err_t rt_pin_attach_irq(rt_base_t pin, rt_uint8_t mode, void (*hdr)(void *args), void *args) {
    return ((rt_pin_attach_irq_api_ptr_t)(rt_pin_attach_irq_addr))(pin, mode, hdr, args);
}

rt_err_t rt_pin_detach_irq(rt_base_t pin) {
    return ((rt_pin_detach_irq_api_ptr_t)(rt_pin_detach_irq_addr))(pin);
}

rt_err_t rt_pin_irq_enable(rt_base_t pin, rt_uint8_t enabled) {
    return ((rt_pin_irq_enable_api_ptr_t)(rt_pin_irq_enable_addr))(pin, enabled);
}

rt_err_t set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t day) {
    return ((set_date_api_ptr_t)(set_date_addr))(year, month, day);
}

rt_err_t set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second) {
    return ((set_time_api_ptr_t)(set_time_addr))(hour, minute, second);
}

rt_err_t set_timestamp(time_t timestamp) {
    return ((set_timestamp_api_ptr_t)(set_timestamp_addr))(timestamp);
}

rt_err_t get_timestamp(time_t *timestamp) {
    return ((get_timestamp_api_ptr_t)(get_timestamp_addr))(timestamp);
}

rt_err_t rt_spi_bus_register(struct rt_spi_bus *bus, const char *name, const struct rt_spi_ops *ops) {
    return ((rt_spi_bus_register_api_ptr_t)(rt_spi_bus_register_addr))(bus, name, ops);
}

rt_err_t rt_spi_bus_attach_device(struct rt_spi_device *device, const char *name, const char *bus_name, void *user_data) {
    return ((rt_spi_bus_attach_device_api_ptr_t)(rt_spi_bus_attach_device_addr))(device, name, bus_name, user_data);
}

rt_err_t rt_spi_bus_attach_device_cspin(struct rt_spi_device *device, const char *name, const char *bus_name, rt_base_t cs_pin, void *user_data) {
    return ((rt_spi_bus_attach_device_cspin_api_ptr_t)(rt_spi_bus_attach_device_cspin_addr))(device, name, bus_name, cs_pin, user_data);
}

rt_err_t rt_spi_bus_configure(struct rt_spi_device *device) {
    return ((rt_spi_bus_configure_api_ptr_t)(rt_spi_bus_configure_addr))(device);
}

rt_err_t rt_spi_take_bus(struct rt_spi_device *device) {
    return ((rt_spi_take_bus_api_ptr_t)(rt_spi_take_bus_addr))(device);
}

rt_err_t rt_spi_release_bus(struct rt_spi_device *device) {
    return ((rt_spi_release_bus_api_ptr_t)(rt_spi_release_bus_addr))(device);
}

rt_err_t rt_spi_take(struct rt_spi_device *device) {
    return ((rt_spi_take_api_ptr_t)(rt_spi_take_addr))(device);
}

rt_err_t rt_spi_release(struct rt_spi_device *device) {
    return ((rt_spi_release_api_ptr_t)(rt_spi_release_addr))(device);
}

rt_err_t rt_spi_configure(struct rt_spi_device *device, struct rt_spi_configuration *cfg) {
    return ((rt_spi_configure_api_ptr_t)(rt_spi_configure_addr))(device, cfg);
}

rt_err_t rt_spi_send_then_recv(struct rt_spi_device *device, const void *send_buf, rt_size_t send_length, void *recv_buf, rt_size_t recv_length) {
    return ((rt_spi_send_then_recv_api_ptr_t)(rt_spi_send_then_recv_addr))(device, send_buf, send_length, recv_buf, recv_length);
}

rt_err_t rt_spi_send_then_send(struct rt_spi_device *device, const void *send_buf1, rt_size_t send_length1, const void *send_buf2, rt_size_t send_length2) {
    return ((rt_spi_send_then_send_api_ptr_t)(rt_spi_send_then_send_addr))(device, send_buf1, send_length1, send_buf2, send_length2);
}

rt_ssize_t rt_spi_transfer(struct rt_spi_device *device, const void *send_buf, void *recv_buf, rt_size_t length) {
    return ((rt_spi_transfer_api_ptr_t)(rt_spi_transfer_addr))(device, send_buf, recv_buf, length);
}

rt_err_t rt_spi_sendrecv8(struct rt_spi_device *device, rt_uint8_t senddata, rt_uint8_t *recvdata) {
    return ((rt_spi_sendrecv8_api_ptr_t)(rt_spi_sendrecv8_addr))(device, senddata, recvdata);
}

rt_err_t rt_spi_sendrecv16(struct rt_spi_device *device, rt_uint16_t senddata, rt_uint16_t *recvdata) {
    return ((rt_spi_sendrecv16_api_ptr_t)(rt_spi_sendrecv16_addr))(device, senddata, recvdata);
}

struct rt_spi_message * rt_spi_transfer_message(struct rt_spi_device *device, struct rt_spi_message *message) {
    return ((rt_spi_transfer_message_api_ptr_t)(rt_spi_transfer_message_addr))(device, message);
}

rt_size_t at_vprintfln(rt_device_t device, char *send_buf, rt_size_t buf_size, const char *format, va_list args) {
    return ((at_vprintfln_api_ptr_t)(at_vprintfln_addr))(device, send_buf, buf_size, format, args);
}

int at_client_init(const char *dev_name, rt_size_t recv_bufsz, rt_size_t send_bufsz) {
    return ((at_client_init_api_ptr_t)(at_client_init_addr))(dev_name, recv_bufsz, send_bufsz);
}

at_client_t at_client_get(const char *dev_name) {
    return ((at_client_get_api_ptr_t)(at_client_get_addr))(dev_name);
}

at_client_t at_client_get_first(void) {
    return ((at_client_get_first_api_ptr_t)(at_client_get_first_addr))();
}

int at_client_obj_wait_connect(at_client_t client, rt_uint32_t timeout) {
    return ((at_client_obj_wait_connect_api_ptr_t)(at_client_obj_wait_connect_addr))(client, timeout);
}

rt_size_t at_client_obj_send(at_client_t client, const char *buf, rt_size_t size) {
    return ((at_client_obj_send_api_ptr_t)(at_client_obj_send_addr))(client, buf, size);
}

rt_size_t at_client_obj_recv(at_client_t client, char *buf, rt_size_t size, rt_int32_t timeout) {
    return ((at_client_obj_recv_api_ptr_t)(at_client_obj_recv_addr))(client, buf, size, timeout);
}

void at_obj_set_end_sign(at_client_t client, char ch) {
    return ((at_obj_set_end_sign_api_ptr_t)(at_obj_set_end_sign_addr))(client, ch);
}

int at_obj_set_urc_table(at_client_t client, const struct at_urc * table, rt_size_t size) {
    return ((at_obj_set_urc_table_api_ptr_t)(at_obj_set_urc_table_addr))(client, table, size);
}

int at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *cmd_expr, ...) {
    va_list args;
    rt_err_t result = RT_EOK;

    RT_ASSERT(cmd_expr);

    if (client == RT_NULL)
    {
        log_error("input AT Client object is NULL, please create or get AT Client object!");
        return -RT_ERROR;
    }

    /* check AT CLI mode */
    if (client->status == AT_STATUS_CLI && resp)
    {
        return -RT_EBUSY;
    }

    rt_mutex_take(client->lock, RT_WAITING_FOREVER);

    client->resp_status = AT_RESP_OK;

    if (resp != RT_NULL)
    {
        resp->buf_len = 0;
        resp->line_counts = 0;
    }

    client->resp = resp;
    rt_sem_control(client->resp_notice, RT_IPC_CMD_RESET, RT_NULL);

    va_start(args, cmd_expr);
    client->last_cmd_len = at_vprintfln(client->device, client->send_buf, client->send_bufsz, cmd_expr, args);
    if (client->last_cmd_len > 2)
    {
        client->last_cmd_len -= 2; /* "\r\n" */
    }
    va_end(args);

    if (resp != RT_NULL)
    {
        if (rt_sem_take(client->resp_notice, resp->timeout) != RT_EOK)
        {
            log_warn("execute command (%.*s) timeout (%d ticks)!", client->last_cmd_len, client->send_buf, resp->timeout);
            client->resp_status = AT_RESP_TIMEOUT;
            result = -RT_ETIMEOUT;
        }
        else if (client->resp_status != AT_RESP_OK)
        {
            log_error("execute command (%.*s) failed!", client->last_cmd_len, client->send_buf);
            result = -RT_ERROR;
        }
    }

    client->resp = RT_NULL;

    rt_mutex_release(client->lock);

    return result;
}

at_response_t at_create_resp(rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout) {
    return ((at_create_resp_api_ptr_t)(at_create_resp_addr))(buf_size, line_num, timeout);
}

void at_delete_resp(at_response_t resp) {
    return ((at_delete_resp_api_ptr_t)(at_delete_resp_addr))(resp);
}

at_response_t at_resp_set_info(at_response_t resp, rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout) {
    return ((at_resp_set_info_api_ptr_t)(at_resp_set_info_addr))(resp, buf_size, line_num, timeout);
}

const char * at_resp_get_line(at_response_t resp, rt_size_t resp_line) {
    return ((at_resp_get_line_api_ptr_t)(at_resp_get_line_addr))(resp, resp_line);
}

const char * at_resp_get_line_by_kw(at_response_t resp, const char *keyword) {
    return ((at_resp_get_line_by_kw_api_ptr_t)(at_resp_get_line_by_kw_addr))(resp, keyword);
}

int at_resp_parse_line_args(at_response_t resp, rt_size_t resp_line, const char *resp_expr, ...) {
    va_list args;
    int resp_args_num = 0;
    const char *resp_line_buf = RT_NULL;

    RT_ASSERT(resp);
    RT_ASSERT(resp_expr);

    if ((resp_line_buf = at_resp_get_line(resp, resp_line)) == RT_NULL)
    {
        return -1;
    }

    va_start(args, resp_expr);

    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;
}

int at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...) {
    va_list args;
    int resp_args_num = 0;
    const char *resp_line_buf = RT_NULL;

    RT_ASSERT(resp);
    RT_ASSERT(resp_expr);

    if ((resp_line_buf = at_resp_get_line_by_kw(resp, keyword)) == RT_NULL)
    {
        return -1;
    }

    va_start(args, resp_expr);

    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;
}

int rt_vsprintf(char *dest, const char *format, va_list arg_ptr) {
    return ((rt_vsprintf_api_ptr_t)(rt_vsprintf_addr))(dest, format, arg_ptr);
}

int rt_sprintf(char *buf, const char *format, ...) {
    rt_int32_t n = 0;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    n = rt_vsprintf(buf, format, arg_ptr);
    va_end(arg_ptr);

    return n;
}

int rt_snprintf(char *buf, rt_size_t size, const char *format, ...) {
    rt_int32_t n = 0;
    va_list args;

    va_start(args, format);
    n = rt_vsnprintf(buf, size, format, args);
    va_end(args);

    return n;
}

rt_err_t rt_get_errno(void) {
    return ((rt_get_errno_api_ptr_t)(rt_get_errno_addr))();
}

void rt_set_errno(rt_err_t no) {
    return ((rt_set_errno_api_ptr_t)(rt_set_errno_addr))(no);
}

int * _rt_errno(void) {
    return ((_rt_errno_api_ptr_t)(_rt_errno_addr))();
}

const char * rt_strerror(rt_err_t error) {
    return ((rt_strerror_api_ptr_t)(rt_strerror_addr))(error);
}

void * rt_memset(void *src, int c, rt_ubase_t n) {
    return ((rt_memset_api_ptr_t)(rt_memset_addr))(src, c, n);
}

void * rt_memcpy(void *dest, const void *src, rt_ubase_t n) {
    return ((rt_memcpy_api_ptr_t)(rt_memcpy_addr))(dest, src, n);
}

void * rt_memmove(void *dest, const void *src, rt_size_t n) {
    return ((rt_memmove_api_ptr_t)(rt_memmove_addr))(dest, src, n);
}

rt_int32_t rt_memcmp(const void *cs, const void *ct, rt_size_t count) {
    return ((rt_memcmp_api_ptr_t)(rt_memcmp_addr))(cs, ct, count);
}

char * rt_strdup(const char *s) {
    return ((rt_strdup_api_ptr_t)(rt_strdup_addr))(s);
}

rt_size_t rt_strnlen(const char *s, rt_ubase_t maxlen) {
    return ((rt_strnlen_api_ptr_t)(rt_strnlen_addr))(s, maxlen);
}

char * rt_strstr(const char *str1, const char *str2) {
    return ((rt_strstr_api_ptr_t)(rt_strstr_addr))(str1, str2);
}

rt_int32_t rt_strcasecmp(const char *a, const char *b) {
    return ((rt_strcasecmp_api_ptr_t)(rt_strcasecmp_addr))(a, b);
}

char * rt_strcpy(char *dst, const char *src) {
    return ((rt_strcpy_api_ptr_t)(rt_strcpy_addr))(dst, src);
}

char * rt_strncpy(char *dest, const char *src, rt_size_t n) {
    return ((rt_strncpy_api_ptr_t)(rt_strncpy_addr))(dest, src, n);
}

rt_int32_t rt_strncmp(const char *cs, const char *ct, rt_size_t count) {
    return ((rt_strncmp_api_ptr_t)(rt_strncmp_addr))(cs, ct, count);
}

rt_int32_t rt_strcmp(const char *cs, const char *ct) {
    return ((rt_strcmp_api_ptr_t)(rt_strcmp_addr))(cs, ct);
}

rt_size_t rt_strlen(const char *src) {
    return ((rt_strlen_api_ptr_t)(rt_strlen_addr))(src);
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
    return ((gettimeofday_api_ptr_t)(gettimeofday_addr))(tv, tz);
}

void HAL_PWR_EnableWakeUpPin(uint32_t WakeUpPin) {
    return ((HAL_PWR_EnableWakeUpPin_api_ptr_t)(HAL_PWR_EnableWakeUpPin_addr))(WakeUpPin);
}

void HAL_PWREx_EnterSHUTDOWNMode(void) {
    return ((HAL_PWREx_EnterSHUTDOWNMode_api_ptr_t)(HAL_PWREx_EnterSHUTDOWNMode_addr))();
}

int stime(const time_t *t) {
    return ((stime_api_ptr_t)(stime_addr))(t);
}

time_t timegm(struct tm * const t) {
    return ((timegm_api_ptr_t)(timegm_addr))(t);
}

int settimeofday(const struct timeval *tv, const struct timezone *tz) {
    return ((settimeofday_api_ptr_t)(settimeofday_addr))(tv, tz);
}

struct tm * gmtime_r(const time_t *timep, struct tm *r) {
    return ((gmtime_r_api_ptr_t)(gmtime_r_addr))(timep, r);
}

char * asctime_r(const struct tm *t, char *buf) {
    return ((asctime_r_api_ptr_t)(asctime_r_addr))(t, buf);
}

char * ctime_r(const time_t * tim_p, char * result) {
    return ((ctime_r_api_ptr_t)(ctime_r_addr))(tim_p, result);
}

struct tm* localtime_r(const time_t* t, struct tm* r) {
    return ((localtime_r_api_ptr_t)(localtime_r_addr))(t, r);
}

rt_uint8_t rt_interrupt_get_nest(void) {
    return ((rt_interrupt_get_nest_api_ptr_t)(rt_interrupt_get_nest_addr))();
}

struct tm * gmtime(const time_t* t) {
    return ((gmtime_api_ptr_t)(gmtime_addr))(t);
}

struct tm * localtime(const time_t* t) {
    return ((localtime_api_ptr_t)(localtime_addr))(t);
}

time_t mktime(struct tm* const t) {
    return ((mktime_api_ptr_t)(mktime_addr))(t);
}

char * ctime(const time_t* tim_p) {
    return ((ctime_api_ptr_t)(ctime_addr))(tim_p);
}

time_t time(time_t* t) {
    return ((time_api_ptr_t)(time_addr))(t);
}

