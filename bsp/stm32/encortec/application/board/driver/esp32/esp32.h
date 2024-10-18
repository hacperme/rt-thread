#ifndef __H_ESP32__
#define __H_ESP32__

#include <rtthread.h>

rt_err_t esp32_at_client_init(void);
rt_err_t esp32_cwinit(int flag);
rt_err_t esp32_cwsap(const char *ssid, const char *pwd);
rt_err_t esp32_qdk(const char *dk_str);
rt_err_t esp32_qtransf(int start);

rt_err_t wait_esp32_finished(rt_int32_t time);
rt_err_t wait_esp32_ready(rt_int32_t time);

#endif