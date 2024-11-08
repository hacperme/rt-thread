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

#define CAT1_AT_BUFF_SIZE 512
#define CAT1_SEND_FILE_SIZE 32

#define AT_RDY_HEAD "RDY"
#define AT_QIND_HEAD "+QIND:"
#define AT_QIND_FILESTART "+QIND: \"FOTA\",\"FILESTART\""
#define AT_QIND_FILEEND "+QIND: \"FOTA\",\"FILEEND\""
#define AT_QIND_FOTA_END "+QIND: \"FOTA\",\"END\""

static at_client_t cat1_at_client = RT_NULL;
static at_response_t cat1_at_resp = RT_NULL;

static void cat1_ota_urc(struct at_client *client ,const char *data, rt_size_t size);
static struct rt_semaphore start_trans_file_sem;
static struct rt_semaphore end_trans_file_sem;
static struct rt_semaphore fota_end_sem;
static struct rt_semaphore rdy_sem;
static char trans_file_res = 0;
static char fota_res = 0;

static struct at_urc cat1_ota_urc_table[] = {
    {AT_QIND_HEAD,       "\r\n", cat1_ota_urc},
    {AT_RDY_HEAD,        "\r\n", cat1_ota_urc}
};

static void cat1_ota_urc(struct at_client *client ,const char *data, rt_size_t size)
{
    log_info("[cat1_ota_urc][%s]", data);
    if (rt_strncmp(data, AT_QIND_HEAD, rt_strlen(AT_QIND_HEAD)) == 0)
    {
        if (rt_strncmp(data, AT_QIND_FILESTART, rt_strlen(AT_QIND_FILESTART)) == 0)
        {
            rt_sem_release(&start_trans_file_sem);
        }
        else if (rt_strncmp(data, AT_QIND_FILEEND, rt_strlen(AT_QIND_FILEEND)) == 0)
        {
            trans_file_res = data[rt_strlen(AT_QIND_FILEEND) + 1] == '0' ? 1 : 0;
            rt_sem_release(&end_trans_file_sem);
        }
        else if (rt_strncmp(data, AT_QIND_FOTA_END, rt_strlen(AT_QIND_FOTA_END)) == 0)
        {
            fota_res = data[rt_strlen(AT_QIND_FOTA_END) + 1] == '0' ? 1 : 0;
            rt_sem_release(&fota_end_sem);
        }
    }
    else if (rt_strncmp(data, AT_RDY_HEAD, rt_strlen(AT_RDY_HEAD)) == 0)
    {
        rt_sem_release(&rdy_sem);
    }
}

void cat1_ota_prepare(void *node)
{
    // 1. AT模块初始化
    rt_err_t res;
    UpgradeNode *_node = (UpgradeNode *)node;

    res = rt_sem_init(&start_trans_file_sem, "cat1stfs", 0, RT_IPC_FLAG_PRIO);
    if (res != RT_EOK) goto _failed_;
    res = rt_sem_init(&end_trans_file_sem, "cat1etfs", 0, RT_IPC_FLAG_PRIO);
    if (res != RT_EOK)
    {
        rt_sem_detach(&start_trans_file_sem);
        goto _failed_;
    }
    res = rt_sem_init(&fota_end_sem, "cat1fes", 0, RT_IPC_FLAG_PRIO);
    if (res != RT_EOK)
    {
        rt_sem_detach(&start_trans_file_sem);
        rt_sem_detach(&end_trans_file_sem);
        goto _failed_;
    }
    res = rt_sem_init(&rdy_sem, "cat1rdy", 0, RT_IPC_FLAG_PRIO);
    if (res != RT_EOK)
    {
        rt_sem_detach(&start_trans_file_sem);
        rt_sem_detach(&end_trans_file_sem);
        rt_sem_detach(&fota_end_sem);
        goto _failed_;
    }

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
        rt_sem_detach(&start_trans_file_sem);
        rt_sem_detach(&end_trans_file_sem);
        rt_sem_detach(&fota_end_sem);
        rt_sem_detach(&rdy_sem);
        goto _failed_;
    }
    cat1_at_resp = at_create_resp(CAT1_AT_BUFF_SIZE, 0, 3000);
    if (!cat1_at_resp)
    {
        cat1_at_client = RT_NULL;
        rt_sem_detach(&start_trans_file_sem);
        rt_sem_detach(&end_trans_file_sem);
        rt_sem_detach(&fota_end_sem);
        rt_sem_detach(&rdy_sem);
        goto _failed_;
    }

    int ret = at_obj_set_urc_table(cat1_at_client, cat1_ota_urc_table, sizeof(cat1_ota_urc_table) / sizeof(cat1_ota_urc_table[0]));
    log_info("at_obj_set_urc_table %s", res_msg(ret == 0));
    if (ret != 0)
    {
        at_delete_resp(cat1_at_resp);
        cat1_at_client = RT_NULL;
        rt_sem_detach(&start_trans_file_sem);
        rt_sem_detach(&end_trans_file_sem);
        rt_sem_detach(&fota_end_sem);
        rt_sem_detach(&rdy_sem);
        goto _failed_;
    }

    // TODO: 2. CAT1 上电

    _node->status = UPGRADE_STATUS_PREPARE_OK;
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
    if (res == RT_EOK) res = rt_sem_take(&start_trans_file_sem, 5000);
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

    res = rt_sem_take(&end_trans_file_sem, 20 * 1000);
    if (res == RT_EOK) res = trans_file_res == 1 ? RT_EOK : RT_ERROR;

    return res;
}

void cat1_ota_apply(int* progress, void *node)
{
    UpgradeNode *_node = (UpgradeNode *)node;
    if (cat1_at_client == RT_NULL) goto _failed_;

    rt_err_t res;
    res = open_cat1_fota_urc();
    log_info("open_cat1_fota_urc %s", res_msg(res == RT_EOK));
    if (res != RT_EOK) goto _failed_;

    res = start_cat1_ota_cmd(_node->plan.file[0].file_size);
    log_info("start_cat1_ota_cmd %s", res_msg(res == RT_EOK));
    if (res != RT_EOK) goto _failed_;

    res = transfer_cat1_ota_file(_node->plan.file[0].file_name);
    log_info("transfer_cat1_ota_file %s", res_msg(res == RT_EOK));
    if (res != RT_EOK) goto _failed_;

    res = rt_sem_take(&fota_end_sem, 120 * 1000);
    log_info("rt_sem_take fota_end_sem %s", res_msg(res == RT_EOK));
    if (res == RT_EOK) res = fota_res == 1 ? RT_EOK : RT_ERROR;
    log_info("fota_res %s", res_msg(res == RT_EOK));
    if (res != RT_EOK) goto _failed_;

    res = rt_sem_take(&rdy_sem, 30 * 1000);
    log_info("rt_sem_take rdy_sem %s", res_msg(res == RT_EOK));
    _node->status = res == RT_EOK ? UPGRADE_STATUS_SUCCESS : UPGRADE_STATUS_FAILED;
    return;

_failed_:
    _node->status = UPGRADE_STATUS_FAILED;
    return;
}

void cat1_ota_finish(void *node)
{
    at_delete_resp(cat1_at_resp);
    cat1_at_client = RT_NULL;
    rt_sem_detach(&start_trans_file_sem);
    rt_sem_detach(&end_trans_file_sem);
    rt_sem_detach(&fota_end_sem);
    rt_sem_detach(&rdy_sem);

    // TODO: Shutdown CAT1.
}

UpgradeModuleOps cat1_ota_ops = {
    .prepare = cat1_ota_prepare,
    .apply = cat1_ota_apply,
    .finish = cat1_ota_finish,
};

