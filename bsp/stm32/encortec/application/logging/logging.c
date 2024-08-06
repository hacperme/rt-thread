
#include "logging.h"
#include <rtthread.h>
#include <stdarg.h>
#include <string.h>

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
        memset(&app_log_mutex, 0, sizeof(app_log_mutex));
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

char *get_ts(void) {
    return "20240808";
    // TODO
}

void app_log_print(const char *fmt, ...)
{
    if(app_log_initilized) {
        va_list args;
        va_start(args, fmt);

        rt_mutex_take(&app_log_mutex, RT_WAITING_FOREVER);

        #ifdef APP_LOG_PRINT_TO_UART
        rt_kprintf(fmt, args);
        #endif

        #ifdef APP_LOG_PRINT_TO_FILE
        app_log_print_to_file(fmt, args);
        #endif

        rt_mutex_release(&app_log_mutex);

        va_end(args);
    }
}

#endif
