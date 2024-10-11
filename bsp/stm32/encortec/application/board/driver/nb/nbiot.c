#include "nbiot.h"
#include <stdio.h>
#include "cJSON.h"
#include <string.h>

#include "logging.h"
// #define DBG_TAG "nbiot"
// #define DBG_LVL DBG_LOG
// #include <rtdbg.h>

static struct rt_mutex qiot_event_mutex;
static int QIOT_SUBSCRIBE_EVENT_CODE = -1;
static int QIOT_CONNECT_EVENT_CODE = -1;
static int QIOT_AUTH_EVENT_CODE = -1;
static int QIOT_DATA_SEND_EVENT_CODE = -1;
static int QIOT_DATA_RECV_EVENT_CODE = -1;
static int QIOT_DISCONNECT_EVENT_CODE = -1;
static int QIOT_OTA_EVENT_CODE = -1;
static int QIOT_PLATFORM_EVENT_CODE = -1;
static int QIOT_WAKEUP_EVENT_CODE = -1;

static rt_mq_t cclk_data_urc_queue = RT_NULL;

void nbiot_qiotevt_urc_handler(struct at_client *client, const char *data, rt_size_t size)
{
    int event_type;
    int event_code;
    char event_data[256] = {0};

    // +QIOTEVT: <event_type>,<event_code>[,<data>]
    sscanf(data, "+QIOTEVT: %d,%d,%s", &event_type, &event_code, event_data);
    log_debug("event_type: %d; event_code: %d; event_data: %s", event_type, event_code, event_data);
    
    switch (event_type)
    {
        case 1:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_AUTH_EVENT_CODE = event_code;
            log_debug("QIOT_AUTH_EVENT_CODE: %d", QIOT_AUTH_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 2:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_CONNECT_EVENT_CODE = event_code;
            log_debug("QIOT_CONNECT_EVENT_CODE: %d", QIOT_CONNECT_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 3:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_SUBSCRIBE_EVENT_CODE = event_code;
            log_debug("QIOT_SUBSCRIBE_EVENT_CODE: %d", QIOT_SUBSCRIBE_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 4:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_DATA_SEND_EVENT_CODE = event_code;
            log_debug("QIOT_DATA_SEND_EVENT_CODE: %d", QIOT_DATA_SEND_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 5:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_DATA_RECV_EVENT_CODE = event_code;
            log_debug("QIOT_DATA_RECV_EVENT_CODE: %d", QIOT_DATA_RECV_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 6:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_DISCONNECT_EVENT_CODE = event_code;
            log_debug("QIOT_DISCONNECT_EVENT_CODE: %d", QIOT_DISCONNECT_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 7:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_OTA_EVENT_CODE = event_code;
            log_debug("QIOT_OTA_EVENT_CODE: %d", QIOT_OTA_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 8:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_PLATFORM_EVENT_CODE = event_code;
            log_debug("QIOT_PLATFORM_EVENT_CODE: %d", QIOT_PLATFORM_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 9:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_WAKEUP_EVENT_CODE = event_code;
            log_debug("QIOT_WAKEUP_EVENT_CODE: %d", QIOT_WAKEUP_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        default:
            break;
    }
}

void nbiot_cclk_urc_handler(struct at_client *client, const char *data, rt_size_t size)
{
    // +CCLK: 24/08/16,06:28:32+32
    log_error("nbiot_cclk_urc_handler called, size: %d; data: %s\n", size, data);
    rt_err_t result = rt_mq_send(cclk_data_urc_queue, data, size);
    if (result != RT_EOK) {
        log_error("nbiot_cclk_urc_handler rt_mq_send ERR\n");
    }
}

static struct at_urc nbiot_urc_table[] = {
    {"+QIOTEVT: ", "\r\n", nbiot_qiotevt_urc_handler},
    {"+CCLK: ", "\r\n", nbiot_cclk_urc_handler},
};

rt_err_t nbiot_at_client_init(void)
{
    rt_err_t result = RT_EOK;

    if (at_client_get(NBIOT_AT_UART_NAME) != RT_NULL) {
        log_error("nbiot at client already inited.\n");
        return RT_ERROR;
    }

    result = at_client_init(NBIOT_AT_UART_NAME, 256, 256);
    if (result != RT_EOK) {
        log_error("nbiot at client init failed: %d\n", result);
        return result;
    }
    log_debug("nbiot at client init success.");

    result = at_obj_set_urc_table(at_client_get(NBIOT_AT_UART_NAME), nbiot_urc_table, 
                                  sizeof(nbiot_urc_table) / sizeof(nbiot_urc_table[0]));
    if (result != RT_EOK) {
        log_error("nbiot at set urc failed: %d\n", result);
        return result;
    }
    log_debug("nbiot at set urc success.");

    result = rt_mutex_init(&qiot_event_mutex, "qiot_event_mutex", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK) {
        log_error("qiot_event_mutex init failed: %d\n", result);
        return result;
    }
    log_debug("qiot_event_mutex init success.");

    cclk_data_urc_queue = rt_mq_create("cclk_data_urc_queue", 64, 1, RT_IPC_FLAG_FIFO);
    if (cclk_data_urc_queue == RT_NULL) {
        log_error("cclk_data_urc_queue create failed!\n");
        return -RT_ERROR;
    }

    return result;
}
// MSH_CMD_EXPORT(nbiot_at_client_init, nbiot_at_client_init);

rt_err_t nbiot_disable_sleep_mode()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_obj_exec_cmd(client, RT_NULL, "AT");
    rt_thread_mdelay(200);

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(1000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        result = RT_ERROR;
        goto ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QSCLK=0");
    if (result == RT_EOK) {
        log_debug("nbiot disable sleep mode ok");
    }
    else {
        log_debug("nbiot disable sleep mode failed");
    }

ERROR:
    at_delete_resp(resp);
    return result;
}
// MSH_CMD_EXPORT(nbiot_disable_sleep_mode, nbiot_disable_sleep_mode);

rt_err_t nbiot_enable_sleep_mode()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(5000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }
    result = at_obj_exec_cmd(client, resp, "AT+QSCLK=1");
    if (result == RT_EOK) {
        log_debug("nbiot enable sleep mode ok");
    }
    else {
        log_debug("nbiot enable sleep mode failed");
    }

    at_delete_resp(resp);
    return result;
}
// MSH_CMD_EXPORT(nbiot_enable_sleep_mode, nbiot_enable_sleep_mode);

rt_err_t nbiot_enable_echo(int enable)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    // disable at echo
    result = at_obj_exec_cmd(client, resp, enable ? "ATE1" : "ATE0");
    if (result != RT_EOK) {
        log_error("disable at echo failed: %d", result);
    }

    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_check_lwm2m_config(lwm2m_config_t config)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    // check production info
    char pk[64] = {0};
    char ps[64] = {0};
    char ver[64] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"productinfo\"");
    if (result != RT_EOK) {
        log_error("check production info failed: %d", result);
        goto ERROR;
    }
    if (at_resp_parse_line_args(resp, 2, "+QIOTCFG: \"productinfo\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", pk, ps, ver) <= 0) {
        result = RT_ERROR;
        goto ERROR;
    }
    log_debug("pk: %s; ps: %s; ver: %s", pk, ps, ver);
    if (rt_strcmp(pk, config->pk) != 0) {
        log_warn("productinfo pk not equal to configure.");
        result = RT_ERROR;
        goto ERROR;
    }

    // check server
    int server_type = -1;
    char server_URL[128] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"server\"");
    if (result != RT_EOK) {
        log_error("check server failed: %d", result);
        goto ERROR;
    }
    if (at_resp_parse_line_args(resp, 2, "+QIOTCFG: \"server\",%d,\"%[^\"]\"", &server_type, server_URL) <= 0) {
        result = RT_ERROR;
        goto ERROR;
    }
    log_debug("server_type: %d; server_URL: %s", server_type, server_URL);
    if (rt_strcmp(server_URL, config->server_URL) != 0) {
        log_warn("productinfo server not equal to configure.");
        result = RT_ERROR;
        goto ERROR;
    }

ERROR:
    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_set_lwm2m_config(lwm2m_config_t config)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    // set productinfo
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"productinfo\",\"%s\",\"%s\"", config->pk, config->ps);
    if (result != RT_EOK) {
        log_error("set productinfo failed: %d", result);
        goto FINAL;
    }
    log_debug("set productinfo success");

    // set server
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"server\",%d,\"%s\"", config->server_type, config->server_URL);
    if (result != RT_EOK) {
        log_error("set server failed: %d", result);
        goto FINAL;
    }
    log_debug("set server success");

    // set lifetime
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"lifetime\",%d", config->lifetime);
    if (result != RT_EOK) {
        log_error("set lifetime failed: %d", result);
        goto FINAL;
    }
    log_debug("set lifetime success");

    // set buffer
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"buffer\",%d", config->buffer_mode);
    if (result != RT_EOK) {
        log_error("set buffer failed: %d", result);
        goto FINAL;
    }
    log_debug("set buffer success");

    // set act
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"act\",%d", config->context_id);
    if (result != RT_EOK) {
        log_error("set act failed: %d", result);
        goto FINAL;
    }
    log_debug("set act success");

    // set tsl
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"tsl\",%d", config->tsl_mode);
    if (result != RT_EOK) {
        log_error("set tsl failed: %d", result);
        goto FINAL;
    }
    log_debug("set tsl success");

FINAL:
    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_set_cfun_mode(int mode)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    char s[20] = {0};
    snprintf(s, 20, "AT+CFUN=%d", mode);
    result = at_obj_exec_cmd(client, resp, s);
    if (result != RT_EOK) {
        log_error("nbiot cfun0 err: %d", result);
    }

    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_check_network(int retry_times)
{
    int n = -1;
    int stat = -1;
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    for (int i=0; i < retry_times; i++) {
        result = at_obj_exec_cmd(client, resp, "AT+CEREG?");
        at_resp_parse_line_args(resp, 2, "+CEREG: %d,%d", &n, &stat);
        log_debug("check status, n: %d, stat: %d", n, stat);
        if (stat == 1 || stat == 5) {
            at_delete_resp(resp);
            return RT_EOK;
        }
        rt_thread_mdelay(5000);
    }

    at_delete_resp(resp);
    return RT_ERROR;
}


rt_err_t nbiot_set_network_config(network_config_t config)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    char PDP_type[12] = {0};
    char APN[99] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+QCGDEFCONT?");
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+QCGDEFCONT=? failed");
        goto ERROR;
    }
    if (at_resp_parse_line_args(resp, 2, "+QCGDEFCONT: \"%[^,\"]\",\"%[^,\"]\",", PDP_type, APN) <= 0)
    {
        log_debug("at_resp_parse_line_args AT+QCGDEFCONT? failed");
        goto ERROR;
    }
    log_debug("read PDP_type: %s", PDP_type);
    log_debug("read APN: %s", APN);
    if (rt_strcmp(APN, config->apn) != 0) {
        result = at_obj_exec_cmd(client, resp, "AT+QCGDEFCONT=\"IP\",\"%s\"", config->apn);
        if (result != RT_EOK) {
            log_error("set apn result: %d", result);
            goto ERROR;
        }
        log_debug("set apn success");
    }

    char band_info[64] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+QBAND?");
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+QBAND? failed");
        goto ERROR;
    }
    if (at_resp_parse_line_args(resp, 2, "+QBAND: %s", band_info) <= 0) {
        log_debug("at_resp_parse_line_args AT+AT+QBAND? failed");
        goto ERROR;
    }
    log_debug("read band_info: %s", band_info);
    if (rt_strcmp(band_info, config->band + 2) != 0) {
        result = at_obj_exec_cmd(client, resp, "AT+QBAND=%s", config->band);
        if (result != RT_EOK) {
            log_error("nbiot set band result: %d", result);
            goto ERROR;
        }
        log_debug("set band success");
    }

    char cipca_info[64] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+CIPCA?");
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+CIPCA? failed");
        goto ERROR;
    }
    if (at_resp_parse_line_args(resp, 2, "+CIPCA: %s", cipca_info) <= 0) {
        log_debug("at_resp_parse_line_args AT+AT+CIPCA? failed");
        goto ERROR;
    }
    if (rt_strcmp(cipca_info, config->cipca) != 0) {
        result = at_obj_exec_cmd(client, resp, "AT+CIPCA=%s", config->cipca);
        if (result != RT_EOK) {
            log_error("nbiot set cipca result: %d", result);
            goto ERROR;
        }
        log_debug("set cipca success");
    }

ERROR:
    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_lwm2m_register()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QIOTREG=1");
    if (result != RT_EOK) {
        log_error("send qiotreg failed: %d", result);
        at_delete_resp(resp);
        return result;
    }
    log_debug("send qiotreg=1 success");

    rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
    QIOT_SUBSCRIBE_EVENT_CODE = -1;
    rt_mutex_release(&qiot_event_mutex);
    for (int retry_times=0; retry_times < 10; retry_times++) {
        rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
        if (QIOT_SUBSCRIBE_EVENT_CODE == 10200) {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            log_debug("wait 3,10200 success, lwm2m has connect ready");
            return RT_EOK;
        }
        rt_mutex_release(&qiot_event_mutex);
        rt_thread_mdelay(5000);
    }

    log_debug("wait 3,10200 failed, lwm2m connect failed");
    at_delete_resp(resp);
    return RT_ERROR;
}
// MSH_CMD_EXPORT(nbiot_lwm2m_register, nbiot_lwm2m_register);

rt_err_t nbiot_lwm2m_deregister()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QIOTREG=0");
    if (result != RT_EOK) {
        log_error("send qiotreg failed: %d", result);
        at_delete_resp(resp);
        return result;
    }
    log_debug("send qiotreg=0 success");

    at_delete_resp(resp);
    return result;
}
// MSH_CMD_EXPORT(nbiot_lwm2m_deregister, nbiot_lwm2m_deregister);

rt_err_t nbiot_check_qiotstate(int retry_times)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }
    
    int state = -1;
    result = at_obj_exec_cmd(client, resp, "AT+QIOTSTATE?");
    if (result == RT_EOK) {
        if (at_resp_parse_line_args(resp, 2, "+QIOTSTATE: %d", &state) > 0) {
            log_debug("get qiotstate state: %d", state);
            if (state == 8) {
                at_delete_resp(resp);
                return RT_EOK;
            }
            else {
                at_delete_resp(resp);
                return RT_ERROR;
            }
        }
    }
    at_delete_resp(resp);  // after at_resp_parse_line_args, delete resp will cause dump
    return RT_ERROR;
}
// MSH_CMD_EXPORT(nbiot_check_qiotstate, nbiot_check_qiotstate);

rt_err_t nbiot_report_model_data(const char *data, rt_size_t length)
{
    log_debug("report_model_data: %s; length: %d\n", data, length);
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 2, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QIOTMODELTD=100,%d", length);
    if (result != RT_EOK) {
        log_error("qiotmodeltd set failed: %d\n", result);
        at_delete_resp(resp);
        return RT_ERROR;
    }
    rt_thread_mdelay(500);  // After the > response, it is recommended for the MCU to wait for 500 ms before sending the data.

    rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
    QIOT_DATA_SEND_EVENT_CODE = -1;
    rt_mutex_release(&qiot_event_mutex);
    at_client_obj_send(client, data, length);
    at_client_obj_send(client, "\x1A", 1);

    for (int i=0; i < 30; i++) {
        rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
        if (QIOT_DATA_SEND_EVENT_CODE == -1) {
            rt_mutex_release(&qiot_event_mutex);
            rt_thread_mdelay(1000);
            continue;
        }
        if (QIOT_DATA_SEND_EVENT_CODE == 10210) {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            return RT_EOK;
        }
        else {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            return RT_ERROR;
        }
    }
    at_delete_resp(resp);
    return RT_ERROR;
}

rt_err_t nbiot_report_ctrl_data(const char *data, rt_size_t length)
{
    log_debug("report_ctrl_data: %s; length: %d\n", data, length);
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 2, rt_tick_from_millisecond(5000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QIOTMODELTD=100,%d", length);
    if (result != RT_EOK) {
        log_error("qiotmodeltd set failed: %d\n", result);
        at_delete_resp(resp);
        return RT_ERROR;
    }
    rt_thread_mdelay(500);  // After the > response, it is recommended for the MCU to wait for 500 ms before sending the data.

    rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
    QIOT_DATA_SEND_EVENT_CODE = -1;
    rt_mutex_release(&qiot_event_mutex);
    at_client_obj_send(client, data, length);
    at_client_obj_send(client, "\x1A", 1);

    for (int i=0; i < 30; i++) {
        rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
        if (QIOT_DATA_SEND_EVENT_CODE == -1) {
            rt_mutex_release(&qiot_event_mutex);
            rt_thread_mdelay(1000);
            continue;
        }
        if (QIOT_DATA_SEND_EVENT_CODE == 10210) {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            return RT_EOK;
        }
        else {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            return RT_ERROR;
        }
    }
    at_delete_resp(resp);
    return RT_ERROR;
}

rt_err_t nbiot_recv_ctrl_data(int req_length, struct ServerCtrlData *server_ctrl_data_ptr)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 2, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    int retry_times = 0;
    int got_collect_interval_item_flag = 0;
    int got_cat1_upload_file_type = 0;
    int got_cat1_upload_file_times = 0;
    int got_esp32_ap_switch_item_flag = 0;
    while (1) {
        int cur_len = 0;
        int remain_len = 0;
        int remain_pieces = 0;

        result = at_obj_exec_cmd(client, resp, "AT+QIOTMODELRD=%d", req_length);
        if (result != RT_EOK) {
            log_error("qiotmodelrd set failed: %d\n", result);
            break;
        }

        int result = at_resp_parse_line_args(resp, 1, "+QIOTMODELRD: %d,%d,%d", &cur_len, &remain_len, &remain_pieces);
        log_debug("at_resp_parse_line_args result: %d", result);
        if (result > 0) {
            log_debug("cur_len: %d, remain_len: %d, remain_pieces: %d", cur_len, remain_len, remain_pieces);
            if (cur_len <= 0 && remain_pieces <= 0) {
                if (retry_times >= 30) {
                    break;
                }
            }
            const char *data = at_resp_get_line(resp, 2);
            log_debug("recv ctrl data from server: %s", data);
            cJSON *root = cJSON_Parse(data);
            if (root != NULL) {
                cJSON *collect_interval_item = cJSON_GetObjectItem(root, "12");
                if (collect_interval_item != NULL) {
                    log_debug("key: %s, value: %d", collect_interval_item->string, collect_interval_item->valueint);
                    server_ctrl_data_ptr->CollectInterval = collect_interval_item->valueint;
                    got_collect_interval_item_flag = 1;
                }
                cJSON *esp32_ap_switch_item = cJSON_GetObjectItem(root, "21");
                if (esp32_ap_switch_item != NULL) {
                    log_debug("key: %s, value: %d", esp32_ap_switch_item->string, esp32_ap_switch_item->valueint);
                    server_ctrl_data_ptr->Esp32_AP_Switch = esp32_ap_switch_item->valueint;
                    got_esp32_ap_switch_item_flag = 1;
                }
                cJSON *cat1_upload_file_times = cJSON_GetObjectItem(root, "4");
                if (cat1_upload_file_times != NULL) {
                    log_debug("key: %s, value: %s", cat1_upload_file_times->string, cat1_upload_file_times->valuestring);
                    strcat(server_ctrl_data_ptr->Cat1_File_Upload_File_Times, cat1_upload_file_times->valuestring);
                    got_cat1_upload_file_times = 1;
                }
                cJSON *cat1_upload_file_type = cJSON_GetObjectItem(root, "6");
                if (cat1_upload_file_type != NULL) {
                    log_debug("key: %s, value: %d", cat1_upload_file_type->string, cat1_upload_file_type->valueint);
                    server_ctrl_data_ptr->Cat1_File_Upload_File_Type = cat1_upload_file_type->valueint;
                    got_cat1_upload_file_type = 1;
                }
                cJSON_Delete(root);
            }
        }
        
        if (got_collect_interval_item_flag && got_esp32_ap_switch_item_flag && got_cat1_upload_file_times && got_cat1_upload_file_type) {
            log_debug("got cat1_upload_file_item and collect_interval_item");
            break;
        }

        retry_times += 1;
        log_debug("nbiot_recv_ctrl_data restry_times: %d", retry_times);
        rt_thread_mdelay(1000);
    }

    at_delete_resp(resp);
    return RT_ERROR;
}

rt_err_t nbiot_get_current_datetime(datetime_t dt)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 1, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+CCLK?");
    if (result != RT_EOK) {
        log_error("get current datetime failed: %d\n", result);
        at_delete_resp(resp);
        return result;
    }

    char data[64] = {0};
    if(rt_mq_recv(cclk_data_urc_queue, data, 64, rt_tick_from_millisecond(3000)) > 0) {
        log_debug("recv cclk data: %s", data);
        sscanf(data, "+CCLK: %d/%d/%d,%d:%d:%d+%d", &(dt->year), &(dt->month), &(dt->day), &(dt->hour), &(dt->minute), &(dt->second), &(dt->zone));
        at_delete_resp(resp);
        return RT_EOK;
    }

    at_delete_resp(resp);
    return RT_ERROR;
}

rt_err_t nbiot_set_qiotlocext(char *nmea_string)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 1, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
    QIOT_DATA_RECV_EVENT_CODE = -1;
    rt_mutex_release(&qiot_event_mutex);

    result = at_obj_exec_cmd(client, resp, "AT+QIOTLOCEXT=\"%s\"", nmea_string);
    log_debug("AT+QIOTLOCEXT send nmea cmd result: %d", result);
    if (result == RT_EOK) {
        for (int i=0; i < 30; i++) {
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            if (QIOT_DATA_RECV_EVENT_CODE == -1) {
                rt_mutex_release(&qiot_event_mutex);
                rt_thread_mdelay(1000);
                continue;
            }
            if (QIOT_DATA_RECV_EVENT_CODE == 10220) {
                rt_mutex_release(&qiot_event_mutex);
                at_delete_resp(resp);
                return RT_EOK;
            }
            else {
                rt_mutex_release(&qiot_event_mutex);
                at_delete_resp(resp);
                return RT_ERROR;
            }
            rt_thread_mdelay(1000);
        }
    }
    log_error("set qiotlocext result: %d\n", result);
    at_delete_resp(resp);
    return result;
}


// AT+CGSN=1
rt_err_t get_nbiot_imei(char *output)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;
    char imei[64] = {0};

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+CGSN=1");
    if (result == RT_EOK) {
        const char *resp_line = at_resp_get_line(resp, 2);
        if (resp_line == RT_NULL) {
            result = RT_ERROR;
            goto ERROR;
        }
        memcpy(output, resp_line + 7, strlen(resp_line + 7));
        output[15] = '\0';
    }

ERROR:
    at_delete_resp(resp);
    return result;
}
