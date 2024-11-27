/*
 * @FilePath: ota_stm32u575.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-15 15:09:22
 * @copyright : Copyright (c) 2024
 */

#include "rtthread.h"
#include "rtdevice.h"
#include "fal.h"
#include "logging.h"
#include <unistd.h>
#include <stdio.h>
#include "ota_app.h"
#include "upgrade_manager.h"
#include "tools.h"

rt_err_t set_stm32u575_ota_option(ota_tag_e ota_tag, char *ota_file_name)
{
    rt_err_t res = RT_ERROR;

    mbr_t mbr = RT_NULL;
    mbr = mbr_init();
    if (mbr == RT_NULL) goto _exit_;

    mbr->ota_tag = ota_tag;
    mbr->ota_state = OTA_DEFAULT;
    rt_memset(mbr->ota_file, 0, sizeof(mbr->ota_file));
    rt_memcpy(mbr->ota_file, ota_file_name, rt_strlen(ota_file_name));

    res = mbr_save();

_exit_:
    return res;
}

rt_err_t clear_stm32u575_ota_option(void)
{
    rt_err_t res = RT_ERROR;

    mbr_t mbr = RT_NULL;
    mbr = mbr_init();
    if (mbr == RT_NULL) goto _exit_;

    mbr->ota_tag = OTA_NO;
    mbr->ota_state = OTA_DEFAULT;
    rt_memset(mbr->ota_file, 0, sizeof(mbr->ota_file));
    res = RT_EOK;

_exit_:
    return res;
}

rt_err_t get_stm32u575_ota_status(ota_state_e *st_ota_state)
{
    rt_err_t res = RT_ERROR;

    mbr_t mbr = RT_NULL;
    mbr = mbr_init();
    res = mbr == RT_NULL ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) goto _exit_;

    *st_ota_state = mbr->ota_state;

_exit_:
    return res;
}

void test_stm32u575_ota_app(void)
{
    rt_err_t res = set_stm32u575_ota_option(OTA_YES, "app_a.bin");
    if (res == RT_EOK)
    {
        rt_hw_cpu_reset();
    }
}

void stm32u575_ota_prepare(void *node)
{
    UpgradeNode *_node = (UpgradeNode *)node;
    _node->status = UPGRADE_STATUS_PREPARED;
}

void stm32u575_ota_apply(int* progress, void *node)
{
    UpgradeNode *_node = (UpgradeNode *)node;
    rt_err_t res = set_stm32u575_ota_option(OTA_YES, _node->plan.file[0].file_name);
    log_debug("set_stm32u575_ota_option %s", res_msg(res == RT_EOK));
    _node->status = res == RT_EOK ? UPGRADE_STATUS_UPGRADING : UPGRADE_STATUS_FAILED;
}

void stm32u575_ota_finish(void *node){}

UpgradeModuleOps stm32u575_ota_ops = {
    .prepare = stm32u575_ota_prepare,
    .apply = stm32u575_ota_apply,
    .finish = stm32u575_ota_finish,
};
