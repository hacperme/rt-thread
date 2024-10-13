#include "rtthread.h"
#include "at.h"
#include "logging.h"

#define ESP32_AT_UART_NAME "uart5"

static rt_sem_t esp32_finished_sem;
static rt_sem_t esp_32_rdy_sem;

int IS_ESP32_READY = 0;
void esp32_ready_handler(struct at_client *client, const char *data, rt_size_t size)
{
    log_debug("got esp32 ready flag");
    rt_sem_release(esp_32_rdy_sem);
}

void esp32_finished_handler(struct at_client *client, const char *data, rt_size_t size)
{
    log_debug("got esp32 finished flag");
    rt_sem_release(esp32_finished_sem);
}

static struct at_urc esp32_urc_table[] = {
    {"ready", "\r\n", esp32_ready_handler},
    {"finished", "\r\n", esp32_finished_handler},
};

rt_err_t wait_esp32_ready(rt_int32_t time)
{
    return rt_sem_take(esp_32_rdy_sem, time);
}

rt_err_t wait_esp32_finished(rt_int32_t time)
{
    return rt_sem_take(esp32_finished_sem, time);
}

rt_err_t esp32_at_client_init(void)
{
    rt_err_t result = RT_EOK;

    if (at_client_get(ESP32_AT_UART_NAME) != RT_NULL) {
        log_error("esp32 at client already inited.\n");
        return RT_ERROR;
    }

    result = at_client_init(ESP32_AT_UART_NAME, 256, 256);
    if (result != RT_EOK) {
        log_error("esp32 at client init failed: %d\n", result);
        return result;
    }
    log_debug("esp32 at client init success.");

    result = at_obj_set_urc_table(at_client_get(ESP32_AT_UART_NAME), esp32_urc_table, 
                                  sizeof(esp32_urc_table) / sizeof(esp32_urc_table[0]));
    if (result != RT_EOK) {
        log_error("esp32 at set urc failed: %d\n", result);
        return result;
    }
    log_debug("esp32 at set urc success.");

    esp_32_rdy_sem = rt_sem_create("rdy_sem", 0, RT_IPC_FLAG_PRIO);
    esp32_finished_sem = rt_sem_create("finished_sem", 0, RT_IPC_FLAG_PRIO);

    // cclk_data_urc_queue = rt_mq_create("cclk_data_urc_queue", 64, 1, RT_IPC_FLAG_FIFO);
    // if (cclk_data_urc_queue == RT_NULL) {
    //     log_error("cclk_data_urc_queue create failed!\n");
    //     return -RT_ERROR;
    // }

    return result;
}

// AT+CWSAP=<ssid>,<pwd>,<chl>,<ecn>[,<max conn>][,<ssid hidden>]
rt_err_t esp32_cwsap(const char *ssid, const char *pwd)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(ESP32_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("esp32 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(
        client, 
        resp, 
        "AT+CWSAP=\"%s\",\"%s\",5,3",
        ssid, pwd
    );
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+CWSAP failed");
        goto ERROR;
    }

ERROR:
    at_delete_resp(resp);
    return result;
}

// AT+CWINIT=<init>
rt_err_t esp32_cwinit(int flag)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(ESP32_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("esp32 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(
        client, 
        resp, 
        "AT+CWINIT=%d",
        flag
    );
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+CWINIT failed");
        goto ERROR;
    }

ERROR:
    at_delete_resp(resp);
    return result;
}

// AT+CWMODE=<mode>[,<auto_connect>]
rt_err_t esp32_cwmode(int mode, int auto_connect)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(ESP32_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("esp32 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(
        client, 
        resp, 
        "AT+CWMODE=%d,%d",
        mode, auto_connect
    );
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+CWMODE failed");
        goto ERROR;
    }

ERROR:
    at_delete_resp(resp);
    return result;
}

// AT+QDK=<dk_str>
rt_err_t esp32_qdk(const char *dk_str)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(ESP32_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("esp32 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(
        client, 
        resp, 
        "AT+QDK=\"%s\"",
        dk_str
    );
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+QDK failed");
        goto ERROR;
    }

ERROR:
    at_delete_resp(resp);
    return result;
}

// AT+QTRANSF=<start>
rt_err_t esp32_qtransf(int start)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(ESP32_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("esp32 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(
        client, 
        resp, 
        "AT+QTRANSF=%d",
        start
    );
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+QTRANSF failed");
        goto ERROR;
    }

ERROR:
    at_delete_resp(resp);
    return result;
}

// AT+QOTA=<ota_file_path>
rt_err_t esp32_qota(const char *ota_file_path)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(ESP32_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("esp32 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(
        client, 
        resp, 
        "AT+QOTA=%s",
        ota_file_path
    );
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+QOTA failed");
        goto ERROR;
    }

ERROR:
    at_delete_resp(resp);
    return result;
}
