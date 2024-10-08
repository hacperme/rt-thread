#include <rtthread.h>
#include <at.h>
#include "nbiot.h"

#include "logging.h"
// #define DBG_TAG "test"
// #define DBG_LVL DBG_LOG
// #include <rtdbg.h>


rt_err_t test_nbiot_send_at_cmd(int argc, char **argv)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(5000));

    result = at_obj_exec_cmd(client, resp, argv[1]);
    if (result != RT_EOK) {
        log_error("nbiot_send_at_cmd failed: %d", result);
    }
    for (int i=1; i <= resp->line_counts; i++) {
        log_debug("recv line %d: %s", i, at_resp_get_line(resp, i));
    }

    return result;
}
// MSH_CMD_EXPORT(test_nbiot_send_at_cmd, test_nbiot_send_at_cmd);


void test_nbiot_report_model_data()
{   
    char *data = "{\"12\":0,\"13\":false}";
    rt_err_t result = nbiot_report_model_data(data, rt_strlen(data));
    log_debug("report result: %d", result);
}

// MSH_CMD_EXPORT(test_nbiot_report_model_data, test_nbiot_report_model_data);

void test_nbiot_get_current_datetime()
{
    struct datetime dt = {0};

    rt_err_t result = nbiot_get_current_datetime(&dt);
    log_debug("nbiot_get_current_datetime result: %d", result);
    if (result == RT_EOK) {
        log_debug("year: %d", dt.year);
        log_debug("month: %d", dt.month);
        log_debug("day: %d", dt.day);
        log_debug("hour: %d", dt.hour);
        log_debug("minute: %d", dt.minute);
        log_debug("second: %d", dt.second);
        log_debug("zone: %d", dt.zone);
    }
    
}
// MSH_CMD_EXPORT(test_nbiot_get_current_datetime, test_nbiot_get_current_datetime);

void test_nbiot_set_lwm2m_config()
{
    struct lwm2m_config config = {
        "pe15TE",
        "aXp5Y0hudFBkbmho",
        0,
        "coap://iot-south.quecteleu.com:5683",
        86400,
        1,
        1,
        1
    };
    rt_err_t result = nbiot_set_lwm2m_config(&config);
    log_debug("nbiot_set_lwm2m_config result: %d", result);
}

// MSH_CMD_EXPORT(test_nbiot_set_lwm2m_config, test_nbiot_set_lwm2m_config);

void test_nbiot_set_network_config()
{
    struct network_config config = {
        "LPWA.VODAFONE.IOT",
        "2,8,20",
        "3,0"
    };
    nbiot_set_network_config(&config);
}

// MSH_CMD_EXPORT(test_nbiot_set_network_config, test_nbiot_set_network_config);

void test_nbiot_check_network()
{
    rt_err_t result = nbiot_check_network(3);
    log_debug("check network result: %d", result);
}

// MSH_CMD_EXPORT(test_nbiot_check_network, test_nbiot_check_network);

void test_nbiot_recv_ctrl_data()
{   
    struct ServerCtrlData server_ctrl_data = {0};
    rt_err_t result = nbiot_recv_ctrl_data(64, &server_ctrl_data);
    log_debug("nbiot_recv_ctrl_data result: %d", result);
}

// MSH_CMD_EXPORT(test_nbiot_recv_ctrl_data, test_nbiot_recv_ctrl_data);


void test_nbiot_set_qiotlocext()
{
    nbiot_set_qiotlocext("$GPGGA,042523.0,3116.552,N,12138.7385,E,1,05,2.6,438.5,M,-28.0,M,,*78");
}
// MSH_CMD_EXPORT(test_nbiot_set_qiotlocext, test_nbiot_set_qiotlocext);


#include "lpm.h"
void test_esp32_at_connection()
{
    // esp32 "uart5"
    at_client_t client = RT_NULL;
    rt_err_t result = RT_EOK;

    client = at_client_get("uart5");
    if (client == RT_NULL) {
        at_client_init("uart5", 512, 512);
        client = at_client_get("uart5");
    }

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(1000));
    if (resp == RT_NULL) {
        log_debug("no enought mem for resp");
        return;
    }
    
    esp32_power_on();
    esp32_en_on();

    result = at_obj_exec_cmd(client, resp, "AT");
    log_debug("at_obj_exec_cmd result: %d", result);

    esp32_en_off();
    esp32_power_off();
}
// MSH_CMD_EXPORT(test_esp32_at_connection, test_esp32_at_connection);

void test_cat1_at_client_init()
{
    // cat1 "uart1"
    at_client_t client = RT_NULL;
    rt_err_t result = RT_EOK;

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        at_client_init("uart1", 512, 512);
        client = at_client_get("uart1");
    }

}
// MSH_CMD_EXPORT(test_cat1_at_client_init, test_cat1_at_client_init);

void test_cat1_power_on()
{
    extern rt_err_t cat1_init();
    cat1_init();
}
// MSH_CMD_EXPORT(test_cat1_power_on, test_cat1_power_on);

void test_cat1_power_off()
{
    extern rt_err_t cat1_power_ctrl(int state);
    cat1_power_ctrl(0);

}
// MSH_CMD_EXPORT(test_cat1_power_off, test_cat1_power_off);

void cat1_send_test_at()
{
    // cat1 "uart1"
    at_client_t client = RT_NULL;
    rt_err_t result = RT_EOK;

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(5000));
    if (resp == RT_NULL) {
        log_debug("no enought mem for resp");
        return;
    }

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        at_client_init("uart1", 512, 512);
        client = at_client_get("uart1");
    }

    result = at_obj_exec_cmd(client, resp, "ATI");
    log_debug("at_obj_exec_cmd result: %d", result);
}
// MSH_CMD_EXPORT(cat1_send_test_at, cat1_send_test_at);

rt_err_t test_cat1_send_at_cmd(int argc, char **argv)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        log_error("cat1 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(5000));

    result = at_obj_exec_cmd(client, resp, argv[1]);
    if (result != RT_EOK) {
        log_error("cat1_send_at_cmd failed: %d", result);
    }
    for (int i=1; i <= resp->line_counts; i++) {
        log_debug("recv line %d: %s", i, at_resp_get_line(resp, i));
    }

    return result;
}
// MSH_CMD_EXPORT(test_cat1_send_at_cmd, test_cat1_send_at_cmd);