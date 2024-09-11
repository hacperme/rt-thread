
#include "logging.h"
#include <rtthread.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>

#if defined APP_USING_LOG && defined APP_LOG_PRINT_CHANNEL

static struct rt_mutex app_log_mutex = {0};
static int app_log_initilized = 0;

rt_err_t app_log_init(void)
{
    rt_err_t err;
    err = rt_mutex_init(&app_log_mutex, "app_log_mutex", RT_IPC_FLAG_FIFO);
    if (err == RT_EOK)
    {
        app_log_initilized = 1;
    }
    return err;
}

void app_log_deinit(void)
{
    if(app_log_initilized) {
        rt_mutex_detach(&app_log_mutex);
        rt_memset(&app_log_mutex, 0, sizeof(app_log_mutex));
        app_log_initilized = 0;
    }
}

char * get_short_file_name(char * file_name)
{
	return (strrchr(file_name, '/') == NULL ? (strrchr(file_name, '\\') == NULL ? file_name : strrchr(file_name, '\\')+1) : strrchr(file_name, '/')+1);
}

#ifdef APP_LOG_PRINT_TO_FILE
static void app_log_print_to_file(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    // TODO
    va_end(args);
}
#endif

static char ts_buf[20];
static time_t cur_time;
static struct tm time_now;
char *get_ts(void) {
    time(&cur_time);
    static struct tm *_time_now;
    _time_now = localtime(&cur_time);
    rt_memcpy(&time_now, _time_now, sizeof(*_time_now));
    rt_memset(ts_buf, 0, 20);
    strftime(ts_buf, sizeof(ts_buf), "%Y-%m-%d %H:%M:%S", &time_now);
    return ts_buf;
}

void app_log_print(const char *fmt, ...)
{
#if APP_LOG_BUF_SIZE
    if(app_log_initilized) {
        va_list args;
        va_start(args, fmt);
        static char app_log_buf[APP_LOG_BUF_SIZE];

        rt_mutex_take(&app_log_mutex, RT_WAITING_FOREVER);

        vsnprintf(app_log_buf, sizeof(app_log_buf) - 1, fmt, args);

        #ifdef APP_LOG_PRINT_TO_UART
        rt_kprintf("%s", app_log_buf);
        #endif

        #ifdef APP_LOG_PRINT_TO_FILE
        app_log_print_to_file("%s", app_log_buf);
        #endif

        rt_mutex_release(&app_log_mutex);

        va_end(args);
    }
#endif
}

#endif
