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

rt_err_t nbiot_at_client_init(void)
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


