#ifndef __AT_DATA_TRANSF_H__
#define __AT_DATA_TRANSF_H__
#include "rtthread.h"

#define STA_CONNECT_EVENT       (1 << 0)
#define STA_DISCONNECT_EVENT    (1 << 1)
#define AP_STARTED_EVENT        (1 << 2)
#define AP_STOPPED_EVENT        (1 << 3)
#define DT_STARTED_EVENT        (1 << 4)
#define DT_ERROR_EVENT          (1 << 5)
#define DT_SUCCESS_EVENT        (1 << 6)
#define DT_CLOSE_EVENT          (1 << 7)
#define DT_NO_CONN_LONG_TIME    (1 << 8)
#define ESP_ALL_EVENT           (0xFF)


extern rt_err_t esp_wait_rdy();
extern bool esp_at_init(void);
extern void nand_to_esp32(void);
extern rt_err_t esp32_transf_data(const char* ssid, size_t ssid_len, const char* psw, size_t psw_len, const char* dk_str, size_t dk_len);
extern rt_err_t esp_recv_event(rt_uint32_t *event);


#endif