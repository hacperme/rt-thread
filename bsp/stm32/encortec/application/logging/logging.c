
#include "logging.h"
#include <rtthread.h>
#include <stdarg.h>
#include <string.h>
#ifdef APP_LOG_USING_TS
#include <sys/time.h>
#endif
#include <stdio.h>

#include "data_save_as_file.h"


#if defined APP_USING_LOG && defined APP_LOG_PRINT_CHANNEL

static struct rt_mutex app_log_mutex = {0};
static int app_log_initilized = 0;


extern void read_imei_from_file(char *output, int read_length);
struct FileSystem fs_log;

rt_err_t app_log_init(void)
{
    rt_err_t err;
    err = rt_mutex_init(&app_log_mutex, "app_log_mutex", RT_IPC_FLAG_FIFO);
    if (err == RT_EOK)
    {
        app_log_initilized = 1;
    }

    char nbiot_imei_string[16] = {0};
    read_imei_from_file(nbiot_imei_string, 15);
    char temp_base[32] = {0};
    if (strlen(nbiot_imei_string)) {
        sprintf(temp_base, "/log/%s", nbiot_imei_string);
        data_save_as_file_init(&fs_log, 0, ".log", temp_base, -1);
    }
    else {
        data_save_as_file_init(&fs_log, 0, ".log", "/log", -1);
    }

    delete_old_dirs(&fs_log);
    
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
    char buff[1024] = {0};
    sprintf(buff, fmt, args);
    rt_kprintf("buff: %s\n", buff);
    data_save_as_file_v2(&fs_log, buff, strlen(buff), 0);
    va_end(args);
}
#endif

#ifdef APP_LOG_USING_TS
char *get_ts(void) {
    static char ts_buf[20];
    time_t cur_time;
    struct tm *time_now;
    time(&cur_time);
    time_now = localtime(&cur_time);
    rt_memset(ts_buf, 0, 20);
    strftime(ts_buf, sizeof(ts_buf), "%Y-%m-%d %H:%M:%S", time_now);
    return ts_buf;
}
#endif

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
        // app_log_print_to_file("%s", app_log_buf);
        data_save_as_file_v2(&fs_log, app_log_buf, strlen(app_log_buf), 0);
        #endif

        rt_mutex_release(&app_log_mutex);

        va_end(args);
    }
#endif
}

#endif
