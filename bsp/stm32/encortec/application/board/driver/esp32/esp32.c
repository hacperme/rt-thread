#include "rtthread.h"
#include "at.h"
#include "logging.h"

#define ESP32_AT_UART_NAME "uart5"

void es32_urc_example_handler(struct at_client *client, const char *data, rt_size_t size)
{

}

static struct at_urc esp32_urc_table[] = {
    {"+QIOTEVT: ", "\r\n", es32_urc_example_handler},
};

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

    // result = rt_mutex_init(&qiot_event_mutex, "qiot_event_mutex", RT_IPC_FLAG_FIFO);
    // if (result != RT_EOK) {
    //     log_error("qiot_event_mutex init failed: %d\n", result);
    //     return result;
    // }
    // log_debug("qiot_event_mutex init success.");

    // cclk_data_urc_queue = rt_mq_create("cclk_data_urc_queue", 64, 1, RT_IPC_FLAG_FIFO);
    // if (cclk_data_urc_queue == RT_NULL) {
    //     log_error("cclk_data_urc_queue create failed!\n");
    //     return -RT_ERROR;
    // }

    return result;
}

// AT+CWSAP=<ssid>,<pwd>,<chl>,<ecn>[,<max conn>][,<ssid hidden>]
rt_err_t esp32_cwsap(const char *ssid, const char *pwd, int chl, int ecn, int max_conn, int ssid_hidden)
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
        "AT+CWSAP=%s,%s,%d,%d,%d,%d",
        ssid, pwd, chl, ecn, max_conn, ssid_hidden
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
        "AT+QDK=%s",
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
