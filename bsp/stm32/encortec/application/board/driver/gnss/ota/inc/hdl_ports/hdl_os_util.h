#ifndef __HDL_UTIL_H__
#define __HDL_UTIL_H__

#include "hdl_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

// ToDo Porting: Please use your platform API to implement it.
// FreeRTOS memory
#define hdl_mem_malloc(X)           rt_malloc(X)
#define hdl_mem_free(X)             rt_free(X)

// ToDo Porting: Please use your platform API to implement it.
// FreeRTOS Task Delay
// #define hdl_delay(X)                vTaskDelay(X / portTICK_RATE_MS)
#define hdl_delay(X)                rt_thread_mdelay(X)

// ToDo Porting: Please use your platform API to implement it.
// FreeRTOS Task Create Function
#define APP_TASK_NAME               "hdl_app"
#define APP_TASK_STACKSIZE          (10 * 1024)
#define APP_TASK_PRIO               4
// #define hdl_create_main_task(X)     xTaskCreate(X, APP_TASK_NAME, \
//                                     APP_TASK_STACKSIZE / sizeof(StackType_t), NULL, APP_TASK_PRIO, NULL);

// #define hdl_create_brom_start_task(X)    xTaskCreate(X, "BromStartTask", \
//                                     (5 * 1024) / sizeof(StackType_t), NULL, APP_TASK_PRIO, NULL);

// // ToDo Porting: Please use your platform API to implement it.
// // FreeRTOS Task Delete Function
// #define hdl_delete_task(X)          vTaskDelete(X);

#define min(a, b)    (((a) < (b)) ? (a) : (b))

uint16_t hdl_compute_checksum(uint8_t *buf, uint32_t buf_len);

#ifdef __cplusplus
}
#endif

#endif
