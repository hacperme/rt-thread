/*
 * @FilePath: ota_cat1.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-11-05 09:42:47
 * @copyright : Copyright (c) 2024
 */

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include "rtthread.h"
#include "rtdevice.h"
#include "logging.h"
#include "upgrade_manager.h"
#include "tools.h"
#include "board.h"
#include "at.h"
#include "lpm.h"

#define CAT1_AT_BUFF_SIZE 512
#define CAT1_SEND_FILE_SIZE 32

#define AT_RDY_HEAD "RDY"
#define AT_POWERDOWN_HEAD "POWERED DOWN"
#define AT_QIND_HEAD "+QIND:"
#define AT_QIND_FOTA_FILESTART "+QIND: \"FOTA\",\"FILESTART\""
#define AT_QIND_FOTA_FILEDOWNLOADING "+QIND: \"FOTA\",\"DOWNLOADING\""
#define AT_QIND_FOTA_FILEEND "+QIND: \"FOTA\",\"FILEEND\""
#define AT_QIND_FOTA_START "+QIND: \"FOTA\",\"START\""
#define AT_QIND_FOTA_UPDATING "+QIND: \"FOTA\",\"UPDATING\""
#define AT_QIND_FOTA_END "+QIND: \"FOTA\",\"END\""
#define AT_FOTA_PERCENT "100"

typedef struct {
    struct rt_semaphore file_start;
    struct rt_semaphore file_end;
    struct rt_semaphore fota_start;
    struct rt_semaphore fota_updating;
    struct rt_semaphore fota_end;
    struct rt_semaphore rdy;
    struct rt_semaphore power_down;
} cat1_sem_st;

typedef enum {
    CAT1_OTA_DEFAULT,
    CAT1_POWER_DOWN,
    CAT1_RDY,
    CAT1_OTA_FILE_START,
    CAT1_OTA_FILE_DOWNLOADING,
    CAT1_OTA_FILE_END,
    CAT1_OTA_START,
    CAT1_OTA_UPDATEING,
    CAT1_OTA_END
} cat1_state_enum;

static at_client_t cat1_at_client = RT_NULL;
static at_response_t cat1_at_resp = RT_NULL;

static cat1_sem_st cat1_sem;
static char trans_file_res = 0;
static char fota_res_code[4] = {0};
static char fota_updating_proc[4] = {0};
static rt_uint16_t fota_process_size = 0;
static cat1_state_enum cat1_ota_state = CAT1_OTA_DEFAULT;

static void cat1_ota_urc(struct at_client *client ,const char *data, rt_size_t size);
rt_err_t cat1_at_set_regression(rt_uint8_t enable);
rt_err_t cat1_at_query_version(char *cat1_version, rt_size_t size);
rt_err_t cat1_at_query_subedition(char *cat1_sub_edition, rt_size_t size);

static struct at_urc cat1_ota_urc_table[] = {
    {AT_QIND_HEAD,       "\r\n", cat1_ota_urc},
    {AT_RDY_HEAD,        "\r\n", cat1_ota_urc},
	{AT_POWERDOWN_HEAD,   "\r\n", cat1_ota_urc}
};

static void cat1_ota_urc(struct at_client *client ,const char *data, rt_size_t size)
{
    log_info("[cat1_ota_urc] %s", data);
    if (rt_strncmp(data, AT_QIND_HEAD, rt_strlen(AT_QIND_HEAD)) == 0)
    {
        if (rt_strncmp(data, AT_QIND_FOTA_FILESTART, rt_strlen(AT_QIND_FOTA_FILESTART)) == 0)
        {
            cat1_ota_state = CAT1_OTA_FILE_START;
            rt_sem_release(&cat1_sem.file_start);
        }
        else if (rt_strncmp(data, AT_QIND_FOTA_FILEDOWNLOADING, rt_strlen(AT_QIND_FOTA_FILEDOWNLOADING)) == 0)
        {
            cat1_ota_state = CAT1_OTA_FILE_DOWNLOADING;
        }
        else if (rt_strncmp(data, AT_QIND_FOTA_FILEEND, rt_strlen(AT_QIND_FOTA_FILEEND)) == 0)
        {
            trans_file_res = data[rt_strlen(AT_QIND_FOTA_FILEEND) + 1] == '0' ? 1 : 0;
            cat1_ota_state = CAT1_OTA_FILE_END;
            rt_sem_release(&cat1_sem.file_end);
        }
        else if (rt_strncmp(data, AT_QIND_FOTA_START, rt_strlen(AT_QIND_FOTA_START)) == 0 || \
                 rt_strncmp(data + 2, AT_QIND_FOTA_START, rt_strlen(AT_QIND_FOTA_START)) == 0)
        {
            cat1_ota_state = CAT1_OTA_START;
            rt_sem_release(&cat1_sem.fota_start);
        }
        else if (rt_strncmp(data, AT_QIND_FOTA_UPDATING, rt_strlen(AT_QIND_FOTA_UPDATING)) == 0)
        {
            fota_process_size = rt_strlen(AT_QIND_FOTA_UPDATING) + 1;
            rt_memset(fota_updating_proc, 0, sizeof(fota_updating_proc));
            rt_memcpy(fota_updating_proc, data + fota_process_size, size - 2 - fota_process_size);
            log_debug("size=%d, fota_process_size=%d, fota_updating_proc=%s", size, fota_process_size, fota_updating_proc);
            cat1_ota_state = CAT1_OTA_UPDATEING;
            rt_sem_release(&cat1_sem.fota_updating);
        }
        else if (rt_strncmp(data, AT_QIND_FOTA_END, rt_strlen(AT_QIND_FOTA_END)) == 0)
        {
            cat1_ota_state = CAT1_OTA_END;
            rt_uint16_t fota_res_code_size = rt_strlen(AT_QIND_FOTA_END) + 1;
            rt_memset(fota_res_code, 0, sizeof(fota_res_code));
            rt_memcpy(fota_res_code, data + fota_res_code_size, size - 2 - fota_res_code_size);
            rt_sem_release(&cat1_sem.fota_end);
        }
    }
    else if (rt_strncmp(data, AT_RDY_HEAD, rt_strlen(AT_RDY_HEAD)) == 0)
    {
        cat1_ota_state = CAT1_RDY;
        rt_sem_release(&cat1_sem.rdy);
    }
    else if (rt_strncmp(data, AT_POWERDOWN_HEAD, rt_strlen(AT_POWERDOWN_HEAD)) == 0)
    {
        cat1_ota_state = CAT1_POWER_DOWN;
        rt_sem_release(&cat1_sem.power_down);
    }
}

rt_err_t cat1_sem_init(void)
{
    rt_err_t res;
    res = rt_sem_init(&cat1_sem.file_start, "cat1files", 0, RT_IPC_FLAG_PRIO);
    res = rt_sem_init(&cat1_sem.file_end, "cat1filee", 0, RT_IPC_FLAG_PRIO);
    res = rt_sem_init(&cat1_sem.fota_start, "cat1fotas", 0, RT_IPC_FLAG_PRIO);
    res = rt_sem_init(&cat1_sem.fota_updating, "cat1fotau", 0, RT_IPC_FLAG_PRIO);
    res = rt_sem_init(&cat1_sem.fota_end, "cat1fotae", 0, RT_IPC_FLAG_PRIO);
    res = rt_sem_init(&cat1_sem.rdy, "cat1fotae", 0, RT_IPC_FLAG_PRIO);
    res = rt_sem_init(&cat1_sem.power_down, "cat1fotae", 0, RT_IPC_FLAG_PRIO);
    return res;
}

rt_err_t cat1_sem_deinit(void)
{
    rt_err_t res;
    res = rt_sem_detach(&cat1_sem.file_start);
    res = rt_sem_detach(&cat1_sem.file_end);
    res = rt_sem_detach(&cat1_sem.fota_start);
    res = rt_sem_detach(&cat1_sem.fota_updating);
    res = rt_sem_detach(&cat1_sem.fota_end);
    res = rt_sem_detach(&cat1_sem.rdy);
    res = rt_sem_detach(&cat1_sem.power_down);
    return res;
}

void cat1_ota_prepare(void *node)
{
    // 1. AT模块初始化
    rt_err_t res;
    rt_uint8_t cnt;
    UpgradeNode *_node = (UpgradeNode *)node;

    res = cat1_sem_init();
    if (res != RT_EOK) goto _failed_;

    cat1_at_client = at_client_get(CAT1_UART);
    if (cat1_at_client == RT_NULL)
    {
        res = at_client_init(CAT1_UART, CAT1_AT_BUFF_SIZE, CAT1_AT_BUFF_SIZE);
        if (res != RT_EOK)
        {
            log_error("CAT1_UART init failed.");
        }
        cat1_at_client = at_client_get(CAT1_UART);
    }
    log_info("CAT1_UART at client get %s.", res_msg(cat1_at_client != RT_NULL));
    if (cat1_at_client == RT_NULL)
    {
        cat1_sem_deinit();
        goto _failed_;
    }
    cat1_at_resp = at_create_resp(CAT1_AT_BUFF_SIZE, 0, 3000);
    if (!cat1_at_resp)
    {
        cat1_at_client = RT_NULL;
        cat1_sem_deinit();
        goto _failed_;
    }

    int ret = at_obj_set_urc_table(cat1_at_client, cat1_ota_urc_table, sizeof(cat1_ota_urc_table) / sizeof(cat1_ota_urc_table[0]));
    log_info("at_obj_set_urc_table %s", res_msg(ret == 0));
    if (ret != 0)
    {
        at_delete_resp(cat1_at_resp);
        cat1_at_client = RT_NULL;
        cat1_sem_deinit();
        goto _failed_;
    }

_cat1_power_on_:
    res = cat1_power_on();

    cnt = 20;
    log_debug("wait cat1 rdy or fota start.");
    do {
        res = rt_sem_take(&cat1_sem.fota_start, 500);
        if (res == RT_EOK) break;
        res = rt_sem_take(&cat1_sem.fota_updating, 500);
        if (res == RT_EOK) break;
        res = rt_sem_take(&cat1_sem.fota_end, 500);
        if (res == RT_EOK)
        {
            rt_sem_release(&cat1_sem.fota_end);
            break;
        }
        res = rt_sem_take(&cat1_sem.rdy, 500);
        if (res == RT_EOK) break;
        res = rt_sem_take(&cat1_sem.power_down, 500);
        if (res == RT_EOK)
        {
            rt_thread_mdelay(1000);
            goto _cat1_power_on_;
        }
        cnt--;
    } while (res != RT_EOK && cnt > 0);
    log_debug("wait cat1 rdy or fota start over.");
    
    _node->status = (cat1_ota_state == CAT1_RDY || cat1_ota_state >= CAT1_OTA_START) ? UPGRADE_STATUS_PREPARED : UPGRADE_STATUS_PREPARE_FAILED;
    return;

_failed_:
    _node->status = UPGRADE_STATUS_PREPARE_FAILED;
    return;
}

rt_err_t open_cat1_fota_urc(void)
{
    int ret;
    rt_err_t res = cat1_at_client == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    char cmd[] = "AT+QINDCFG=\"all\",1";
    at_resp_set_info(cat1_at_resp, CAT1_AT_BUFF_SIZE, 0, 3000);
    ret = at_obj_exec_cmd(cat1_at_client, cat1_at_resp, cmd);
    res = ret == 0 ? RT_EOK : RT_ERROR;
    return res;
}

rt_err_t open_cat1_hw_flow_ctrl(void)
{
    int ret;
    rt_err_t res = cat1_at_client == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    char cmd[] = "AT+IFC=2,2";
    at_resp_set_info(cat1_at_resp, CAT1_AT_BUFF_SIZE, 0, 3000);
    ret = at_obj_exec_cmd(cat1_at_client, cat1_at_resp, cmd);
    res = ret == 0 ? RT_EOK : RT_ERROR;
    return res;
}

rt_err_t start_cat1_ota_cmd(uint32_t file_size)
{
    int ret;
    rt_err_t res = cat1_at_client == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    char cmd[64] = {0};
    rt_snprintf(cmd, sizeof(cmd), "AT+QFOTADL=\"FILE:%d\",1,100,100", file_size);
    at_resp_set_info(cat1_at_resp, CAT1_AT_BUFF_SIZE, 0, 3000);
    ret = at_obj_exec_cmd(cat1_at_client, cat1_at_resp, cmd);
    res = ret == 0 ? RT_EOK : RT_ERROR;
    if (res == RT_EOK) res = rt_sem_take(&cat1_sem.file_start, 10 * 1000);
    return res;
}

rt_err_t transfer_cat1_ota_file(char *file_name)
{
    int ret;
    rt_err_t res = cat1_at_client == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    char data_buf[CAT1_SEND_FILE_SIZE] = {0};
    FILE *ota_file = fopen(file_name, "rb");
    res = ota_file != NULL ? RT_EOK : RT_ERROR;
    if (res != RT_EOK) return res;

    fseek(ota_file, 0, SEEK_SET);
    size_t read_size;
    do
    {
        read_size = ret = 0;
        rt_memset(data_buf, 0, CAT1_SEND_FILE_SIZE);
        read_size = fread(data_buf, 1, CAT1_SEND_FILE_SIZE, ota_file);
        if (read_size > 0) 
        {
            ret = at_client_obj_send(cat1_at_client, data_buf, read_size);
            res = read_size == ret ? RT_EOK : RT_ERROR;
            if (res != RT_EOK) break;
            rt_thread_mdelay(10);
        }
    } while (read_size > 0 && ret > 0);

    res = rt_sem_take(&cat1_sem.file_end, 30 * 1000);
    if (res == RT_EOK) res = trans_file_res == 1 ? RT_EOK : RT_ERROR;

    return res;
}

void check_cat1_version(void *node)
{
    UpgradeNode *_node = (UpgradeNode *)node;
    rt_err_t res;
    char cat1_version[64] = {0};
    res = cat1_at_query_version(cat1_version, 64);
    log_debug("cat1_at_query_version %s", res_msg(res == RT_EOK));
    if (res == RT_EOK)
    {
        _node->status = (rt_strcmp(cat1_version, _node->plan.target_version) == 0) ? UPGRADE_STATUS_SUCCESS : UPGRADE_STATUS_FAILED;
    }
}

void cat1_ota_apply(int* progress, void *node)
{
    UpgradeNode *_node = (UpgradeNode *)node;
    if (cat1_at_client == RT_NULL) goto _failed_;

    rt_err_t res;
    if (cat1_ota_state == CAT1_RDY)
    {
        // 打开 CAT1 AT 回显
        res = cat1_at_set_regression(1);
        log_info("cat1_at_set_regression %s", res_msg(res == RT_EOK));

        // 打开 CAT1 URC
        res = open_cat1_fota_urc();
        log_info("open_cat1_fota_urc %s", res_msg(res == RT_EOK));
        if (res != RT_EOK) goto _failed_;

        res = start_cat1_ota_cmd(_node->plan.file[0].file_size);
        log_info("start_cat1_ota_cmd %s", res_msg(res == RT_EOK));
        if (res != RT_EOK) goto _failed_;

        res = transfer_cat1_ota_file(_node->plan.file[0].file_name);
        log_info("transfer_cat1_ota_file %s", res_msg(res == RT_EOK));
        if (res != RT_EOK) goto _failed_;

        res = rt_sem_take(&cat1_sem.fota_start, 60 * 1000);
        log_info("rt_sem_take cat1_sem.fota_start %s", res_msg(res == RT_EOK));
        if (res != RT_EOK) goto _failed_;
    }
    if (cat1_ota_state < CAT1_OTA_END)
    {
        do {
            res = rt_sem_take(&cat1_sem.fota_updating, 120 * 1000);
            log_info("rt_sem_take cat1_sem.fota_updating %s, process %s", res_msg(res == RT_EOK), fota_updating_proc);
        } while (res == RT_EOK && rt_strcmp(fota_updating_proc, AT_FOTA_PERCENT) != 0);
        if (res != RT_EOK) goto _failed_;
    }

    res = rt_sem_take(&cat1_sem.fota_end, 60 * 1000);
    log_info("rt_sem_take cat1_sem.fota_end %s, fota_res_code %s", res_msg(res == RT_EOK), fota_res_code);

    res = rt_sem_take(&cat1_sem.rdy, 60 * 1000);
    log_info("rt_sem_take cat1_sem.rdy %s", res_msg(res == RT_EOK));

    check_cat1_version(node);
    return;

_failed_:
    _node->status = UPGRADE_STATUS_FAILED;
    return;
}

void cat1_ota_finish(void *node)
{
    // Press power key to shutdown cat1.
    cat1_power_on();
    // Waiting shutdown urc.
    rt_err_t res = rt_sem_take(&cat1_sem.power_down, 30 * 1000);
    // Turn off power for cat1.
    cat1_power_off();

    cat1_sem_deinit();
    at_delete_resp(cat1_at_resp);
    cat1_at_client = RT_NULL;
    log_debug("cat1_power_off");
}

UpgradeModuleOps cat1_ota_ops = {
    .prepare = cat1_ota_prepare,
    .apply = cat1_ota_apply,
    .finish = cat1_ota_finish,
};

rt_err_t cat1_at_set_regression(rt_uint8_t enable)
{
    rt_err_t res = cat1_at_client == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;
    res = enable == 0 || enable == 1 ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    char cmd[8] = {0};
    rt_snprintf(cmd, sizeof(cmd), "ATE%d", enable);
    at_resp_set_info(cat1_at_resp, CAT1_AT_BUFF_SIZE, 0, 3000);
    res = at_obj_exec_cmd(cat1_at_client, cat1_at_resp, cmd);
    return res;
}

rt_err_t cat1_at_query_version(char *cat1_version, rt_size_t size)
{
    rt_err_t res = cat1_at_client == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    char cmd[] = "AT+QGMR";
    at_resp_set_info(cat1_at_resp, CAT1_AT_BUFF_SIZE, 4, 3000);
    res = at_obj_exec_cmd(cat1_at_client, cat1_at_resp, cmd);
    if (res >= 0)
    {
        rt_memset(cat1_version, 0, size);
        int ret = at_resp_parse_line_args(cat1_at_resp, 2, "%s\r\n", cat1_version);
        res = ret > 0 ? RT_EOK : RT_ERROR;
        if (res == RT_EOK) log_debug("CAT1version %s, size=%d", cat1_version, rt_strlen(cat1_version));
    }
    return res;
}

rt_err_t cat1_at_query_subedition(char *cat1_sub_edition, rt_size_t size)
{
    rt_err_t res = cat1_at_client == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    char cmd[] = "AT+CSUB";
    at_resp_set_info(cat1_at_resp, CAT1_AT_BUFF_SIZE, 4, 3000);
    res = at_obj_exec_cmd(cat1_at_client, cat1_at_resp, cmd);
    if (res >= 0)
    {
        rt_memset(cat1_sub_edition, 0, size);
        int ret = at_resp_parse_line_args(cat1_at_resp, 2, "SubEdition: %s", cat1_sub_edition);
        res = ret > 0 ? RT_EOK : RT_ERROR;
        if (res == RT_EOK)
        {
            log_debug("CAT1 SUB Edition %s, size=%d", cat1_sub_edition, rt_strlen(cat1_sub_edition));
        }
    }
    return res;
}

void test_cat1_at_ota(void)
{
    UpgradeNode cat1_node = {0};
    cat1_node.module = UPGRADE_MODULE_CAT1;
    cat1_node.status = UPGRADE_STATUS_ON_PLAN;
    cat1_node.ops = cat1_ota_ops;
    cat1_node.ops.prepare(&cat1_node);
    log_debug("cat1_node prepare %s", res_msg(cat1_node.status == UPGRADE_STATUS_PREPARED));
    if (cat1_node.status != UPGRADE_STATUS_PREPARED) return;

    rt_err_t res;
    char cat1_version[64] = {0};
    res = cat1_at_query_version(cat1_version, 64);
    log_debug("cat1_at_query_version %s", res_msg(res == RT_EOK));

    char cat1_sub_edition[8] = {0};
    res = cat1_at_query_subedition(cat1_sub_edition, 8);
    log_debug("cat1_at_query_subedition %s", res_msg(res == RT_EOK));

    // cat1_node.ops.finish(&cat1_node);
}