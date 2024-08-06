#ifndef __ENCORTEC_LOGGING_H
#define __ENCORTEC_LOGGING_H

#include "rtconfig.h"
#include "rttypes.h"

#if defined APP_USING_LOG && defined APP_LOG_PRINT_CHANNEL

extern rt_err_t app_log_init(void);
extern void app_log_deinit(void);
extern char * get_short_file_name(char * file_name);
extern void app_log_print(const char *fmt, ...);

#define	__SHORT_FILE__ get_short_file_name(__FILE__)

#define	log_print(fmt, ...) app_log_print("[%s, %s, %d] " fmt "\r\n", __SHORT_FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef APP_LOG_LEVEL

#define APP_LOG_LVL_DEFINE(lvl, fmt, ...)     app_log_print("[%s][%s, %s, %d] "fmt"\r\n", #lvl, __SHORT_FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef APP_LOG_USING_TS
extern char *get_ts(void);
#define APP_LOG_LVL_TS_DEFINE(lvl, fmt, ...)  app_log_print("[%s][%s][%s, %s, %d] "fmt"\r\n", #lvl,  get_ts(), __SHORT_FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef APP_LOG_LEVEL_DBG
#ifndef APP_LOG_USING_TS
#define	log_debug(fmt, ...)	APP_LOG_LVL_DEFINE(D, fmt, ##__VA_ARGS__)
#else
#define	log_debug(fmt, ...)	APP_LOG_LVL_TS_DEFINE(D, fmt, ##__VA_ARGS__)
#endif
#else
#define	log_debug(fmt, ...)
#endif

#ifdef APP_LOG_LEVEL_INF
#ifndef APP_LOG_USING_TS
#define	log_info(fmt, ...)	APP_LOG_LVL_DEFINE(I, fmt, ##__VA_ARGS__)
#else
#define	log_info(fmt, ...)	APP_LOG_LVL_TS_DEFINE(I, fmt, ##__VA_ARGS__)
#endif
#else
#define	log_info(fmt, ...)
#endif

#ifdef APP_LOG_LEVEL_WAR
#ifndef APP_LOG_USING_TS
#define	log_warn(fmt, ...)	APP_LOG_LVL_DEFINE(W, fmt, ##__VA_ARGS__)
#else
#define	log_warn(fmt, ...)	APP_LOG_LVL_TS_DEFINE(W, fmt, ##__VA_ARGS__)
#endif
#else
#define	log_warn(fmt, ...)
#endif

#ifdef APP_LOG_LEVEL_ERR
#ifndef APP_LOG_USING_TS
#define	log_error(fmt, ...)	APP_LOG_LVL_DEFINE(E, fmt, ##__VA_ARGS__)
#else
#define	log_error(fmt, ...)	APP_LOG_LVL_TS_DEFINE(E, fmt, ##__VA_ARGS__)
#endif
#else
#define	log_error(fmt, ...)
#endif

#else

#define log_debug(fmt, ...)
#define log_info(fmt, ...)
#define log_warn(fmt, ...)
#define log_error(fmt, ...)

#endif

#else

#define	log_print(fmt, ...)
#define log_debug(fmt, ...)
#define log_info(fmt, ...)
#define log_warn(fmt, ...)
#define log_error(fmt, ...)

#endif

#endif
