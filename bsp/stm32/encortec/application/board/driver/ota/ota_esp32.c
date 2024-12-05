/*
 * @FilePath: ota_esp32.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-11-15 10:31:08
 * @copyright : Copyright (c) 2024
 */
#include "upgrade_manager.h"
#include "drv_nand_flash.h"
#include "drv_fatfs_dhara_nand.h"
#include "logging.h"
#include "tools.h"
#include "lpm.h"
#include "board.h"
#include "at.h"

#define ESP32_AT_BUFF_SIZE 512

#define ESP32_URC_REDY "+ESPRDY"
#define ESP32_URC_OTA "+OTA:"
#define ESP32_URC_OTA_START "START"
#define ESP32_URC_OTA_SUCCESS "SUCCESS"
#define ESP32_URC_OTA_ERR "ERR"

static at_client_t esp32_at_client = RT_NULL;
static at_response_t esp32_at_resp = RT_NULL;
static char esp32_ota_res_code[16] = {0};

typedef struct {
    struct rt_semaphore rdy;
    struct rt_semaphore ota_start;
    struct rt_semaphore ota_resp;
} esp32_sem_st;

static esp32_sem_st esp32_sem = {0};

static void esp32_ota_urc(struct at_client *client ,const char *data, rt_size_t size);
rt_err_t esp32_at_query_version(char *esp32_version, rt_size_t size);
void esp32_check_version(void *node);
rt_err_t esp32_start_ota_cmd(char* file_name);

static struct at_urc esp32_ota_urc_table[] = {
    {ESP32_URC_REDY,       "\r\n", esp32_ota_urc},
    {ESP32_URC_OTA,        "\r\n", esp32_ota_urc}
};

static void esp32_ota_urc(struct at_client *client ,const char *data, rt_size_t size)
{
    log_info("[esp32_ota_urc] %s", data);
    if (rt_strncmp(data, ESP32_URC_REDY, rt_strlen(ESP32_URC_REDY)) == 0)
    {
        rt_sem_release(&esp32_sem.rdy);
    }
    else if (rt_strncmp(data, ESP32_URC_OTA, rt_strlen(ESP32_URC_OTA)) == 0)
    {
        rt_memset(esp32_ota_res_code, 0, sizeof(esp32_ota_res_code));
        rt_memcpy(esp32_ota_res_code, data + rt_strlen(ESP32_URC_OTA), size - 2 - rt_strlen(ESP32_URC_OTA));
        log_debug("esp32_ota_res_code=%s", esp32_ota_res_code);
        if (rt_strncmp(esp32_ota_res_code, ESP32_URC_OTA_START, sizeof(ESP32_URC_OTA_START)) == 0)
        {
            rt_sem_release(&esp32_sem.ota_start);
        }
        else
        {
            rt_sem_release(&esp32_sem.ota_resp);
        }
    }
}

rt_err_t esp32_sem_init(void)
{
    rt_err_t res;
    res = rt_sem_init(&esp32_sem.rdy, "esprdy", 0, RT_IPC_FLAG_PRIO);
    res = rt_sem_init(&esp32_sem.ota_start, "espotastart", 0, RT_IPC_FLAG_PRIO);
    res = rt_sem_init(&esp32_sem.ota_resp, "espotaresp", 0, RT_IPC_FLAG_PRIO);
    return res;
}

rt_err_t esp32_sem_deinit(void)
{
    rt_err_t res;
    res = rt_sem_detach(&esp32_sem.rdy);
    res = rt_sem_detach(&esp32_sem.ota_start);
    res = rt_sem_detach(&esp32_sem.ota_resp);
    return res;
}

void esp_prepare(void *node) {
    rt_err_t res;
    UpgradeNode *_node = (UpgradeNode *)node;

    res = esp32_sem_init();
    if (res != RT_EOK) goto _failed_;

    esp32_at_client = at_client_get(ESP32_UART);
    if (esp32_at_client == RT_NULL)
    {
        res = at_client_init(ESP32_UART, ESP32_AT_BUFF_SIZE, ESP32_AT_BUFF_SIZE);
        if (res != RT_EOK)
        {
            log_error("ESP32_UART init failed.");
        }
        esp32_at_client = at_client_get(ESP32_UART);
    }
    log_info("esp32_UART at client get %s.", res_msg(esp32_at_client != RT_NULL));
    if (esp32_at_client == RT_NULL)
    {
        esp32_sem_deinit();
        goto _failed_;
    }
    esp32_at_resp = at_create_resp(ESP32_AT_BUFF_SIZE, 0, 3000);
    if (!esp32_at_resp)
    {
        esp32_at_client = RT_NULL;
        esp32_sem_deinit();
        goto _failed_;
    }

    int ret = at_obj_set_urc_table(esp32_at_client, esp32_ota_urc_table, sizeof(esp32_ota_urc_table) / sizeof(esp32_ota_urc_table[0]));
    log_info("at_obj_set_urc_table %s", res_msg(ret == 0));
    if (ret != 0)
    {
        at_delete_resp(esp32_at_resp);
        esp32_at_client = RT_NULL;
        esp32_sem_deinit();
        goto _failed_;
    }

    esp32_power_pin_init();
    esp32_power_off();
    rt_thread_mdelay(100);
    nand_to_esp32();
    esp32_power_on();

    res = rt_sem_take(&esp32_sem.rdy, 10 * 1000);
    _node->status = res == RT_EOK ? UPGRADE_STATUS_PREPARED : UPGRADE_STATUS_PREPARE_FAILED;
    return;

_failed_:
    _node->status = UPGRADE_STATUS_PREPARE_FAILED;
    return;
}

void esp_apply(int* progress, void *node) {
    UpgradeNode *_node = (UpgradeNode *)node;
    _node->status = UPGRADE_STATUS_UPGRADING;
    rt_err_t res;

    esp32_check_version(node);
    if (_node->status == UPGRADE_STATUS_SUCCESS) return;
    
    _node->status = UPGRADE_STATUS_UPGRADING;
    res = esp32_start_ota_cmd(_node->plan.file[0].file_name);
    // TODO: Enable this code
    // if (res != RT_EOK) goto _failed_;

    res = rt_sem_take(&esp32_sem.ota_resp, 30 * 1000);
    log_info("rt_sem_take esp32_sem.ota_resp %s, esp32_ota_res_code %s", res_msg(res == RT_EOK), esp32_ota_res_code);

    esp32_power_off();
    rt_thread_mdelay(1000);
    esp32_power_on();

    res = rt_sem_take(&esp32_sem.rdy, 10 * 1000);
    if (res != RT_EOK) goto _failed_;
    esp32_check_version(node);

_failed_:
    _node->status = UPGRADE_STATUS_FAILED;
    return;
}

void esp_finish(void *node)
{
    esp32_power_off();
    nand_to_stm32();
    fatfs_dhara_nand_remount();
}

UpgradeModuleOps esp_ota_ops = {
    .prepare = esp_prepare,
    .apply = esp_apply,
    .finish = esp_finish,
};

rt_err_t esp32_at_query_version(char *esp32_version, rt_size_t size)
{
    UpgradeNode esp32_node = {0};
    esp32_node.module = UPGRADE_MODULE_ESP;
    esp32_node.status = UPGRADE_STATUS_ON_PLAN;
    esp32_node.ops = esp_ota_ops;
    int need_close_esp32 = 0;

    if(esp32_at_client == RT_NULL)
    {
        esp32_node.ops.prepare(&esp32_node);
        need_close_esp32 = 1;
    }
    rt_err_t res = esp32_at_client == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK){
        
        if(need_close_esp32 == 1)
        {
            esp32_node.ops.finish(&esp32_node);
        }
        return res;
    }

    char cmd[] = "ATI?";
    at_resp_set_info(esp32_at_resp, ESP32_AT_BUFF_SIZE, 3, 3000);
    res = at_obj_exec_cmd(esp32_at_client, esp32_at_resp, cmd);
    if (res >= 0)
    {
        rt_memset(esp32_version, 0, size);
        int ret = at_resp_parse_line_args(esp32_at_resp, 2, "+VERSION:%s\r\n", esp32_version);
        res = ret > 0 ? RT_EOK : RT_ERROR;
        if (res == RT_EOK) log_debug("esp32version %s, size=%d", esp32_version, rt_strlen(esp32_version));
    }
    if(need_close_esp32 == 1)
    {
        esp32_node.ops.finish(&esp32_node);
    }
    return res;
}

void esp32_check_version(void *node)
{
    UpgradeNode *_node = (UpgradeNode *)node;
    rt_err_t res;
    char esp32_version[8] = {0};
    res = esp32_at_query_version(esp32_version, 8);
    log_debug("esp32_at_query_version %s", res_msg(res == RT_EOK));
    if (res == RT_EOK)
    {
        _node->status = (rt_strcmp(esp32_version, _node->plan.target_version) == 0) ? UPGRADE_STATUS_SUCCESS : UPGRADE_STATUS_FAILED;
    }
}

rt_err_t esp32_start_ota_cmd(char* file_name)
{
    int ret;
    rt_err_t res = esp32_at_client == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    char cmd[64] = {0};
    rt_snprintf(cmd, sizeof(cmd), "AT+QOTA=\"%s\"", file_name);
    at_resp_set_info(esp32_at_resp, ESP32_AT_BUFF_SIZE, 0, 1000);
    ret = at_obj_exec_cmd(esp32_at_client, esp32_at_resp, cmd);
    res = ret == 0 ? RT_EOK : RT_ERROR;
    // TODO: Enable this code
    // if (res == RT_EOK) res = rt_sem_take(&esp32_sem.ota_start, 5000);
    res = rt_sem_take(&esp32_sem.ota_start, 5000);
    return res;
}
