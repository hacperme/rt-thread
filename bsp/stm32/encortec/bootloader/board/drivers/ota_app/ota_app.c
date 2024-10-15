/*
 * @FilePath: ota_app.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-11 20:49:11
 * @copyright : Copyright (c) 2024
 */

#include "ota_app.h"
#include "fal.h"
#include "common.h"
#include "drv_fatfs_dhara_nand.h"
#include <unistd.h>
#include <stdio.h>

#define DBG_SECTION_NAME "OTA"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define ota_data_buffer_size 0x2000

static ota_data_t ota_app_data = RT_NULL;
static char ota_data_buffer[ota_data_buffer_size];
static struct fal_partition *data_fal_part = RT_NULL;

extern char __appa_rom_start[];
extern char __appa_rom_end[];
extern char __appb_rom_start[];
extern char __appb_rom_end[];

rt_err_t ota_app_data_init(void)
{
    rt_err_t ret = RT_ERROR;
    ret = data_fal_part == RT_NULL ? RT_ERROR : RT_EOK;
    if (data_fal_part == RT_NULL)
    {
        data_fal_part = fal_partition_find("data");
        ret = data_fal_part == RT_NULL ? RT_ERROR : RT_EOK;
        if (ret == RT_ERROR)
        {
            LOG_E("Can not find data partition in onchip flash.");
        }
    }
    return ret;
}

rt_err_t ota_app_data_query(void)
{
    rt_err_t ret = RT_ERROR;
    int res = 0;

    if (data_fal_part == RT_NULL)
    {
        LOG_E("data_fal_part is not exits.");
        goto _exit_;
    }

    rt_memset(ota_data_buffer, 0xFF, ota_data_buffer_size);
    res = fal_partition_read(data_fal_part, 0, ota_data_buffer, ota_data_buffer_size);
    if (res < 0)
    {
        LOG_E("fal_partition_read failed.");
        goto _exit_;
    }
    ota_app_data = (ota_data_t)ota_data_buffer;
    ret = RT_EOK;

_exit_:
    return ret;
}

rt_err_t ota_app_data_save(void)
{
    rt_err_t ret = RT_ERROR;
    int res = 0;

    if (data_fal_part == RT_NULL)
    {
        LOG_E("data_fal_part is not exits.");
        goto _exit_;
    }

    res = fal_partition_erase(data_fal_part, 0, ota_data_buffer_size);
    LOG_I("fal_partition_erase data addr=0, size=%p %s", ota_data_buffer_size, res == RT_EOK ? "succces" : "failed");

    res = fal_partition_write(data_fal_part, 0, ota_data_buffer, ota_data_buffer_size);
    if (res != RT_EOK)
    {
        LOG_E("fal_partition_write failed.");
    }

_exit_:
    return ret;
}

void ota_app_over(ota_tag_e ota_tag_val, ota_process_e ota_proc_state, app_parition_e app_part_no)
{
    ota_app_data->ota_tag = ota_tag_val;
    ota_app_data->ota_process = ota_proc_state;
    ota_app_data->app_part = app_part_no;
    ota_app_data_save();
    rt_hw_cpu_reset();
}

static struct rt_semaphore fs_mnt_sem;
static void fs_mnt_cb(fdnfs_init_status_e *status)
{
    log_debug("fs_mnt_cb status=%d", *status);
    if (*status == 0)
    {
        rt_sem_release(&fs_mnt_sem);
    }
}

static fdnfs_init_status_e mnt_status;
void ota_app_process(void)
{
    rt_err_t res;
    if (rt_strlen(ota_app_data->ota_file) <= 0)
    {
        ota_app_over(OTA_NO, OTA_FILE_NAME_NOT_EXISTS, ota_app_data->app_part);
        return;
    }

    res = rt_sem_init(&fs_mnt_sem, "fsmntsem", 0, RT_IPC_FLAG_PRIO);
    LOG_I("rt_sem_init fsmntsem res=%d", res);
    if (res != RT_EOK)
    {
        ota_app_over(OTA_NO, OTA_FS_INIT_FAILED, ota_app_data->app_part);
        return;
    }

    fatfs_dhara_nand_init(fs_mnt_cb, &mnt_status);

    res = rt_sem_take(&fs_mnt_sem, 10 * 1000);

    if (res != RT_EOK)
    {
        ota_app_over(OTA_NO, OTA_FS_INIT_FAILED, ota_app_data->app_part);
        return;
    }

    if (access(ota_app_data->ota_file, F_EOK) != 0)
    {
        ota_app_over(OTA_NO, OTA_FILE_NOT_EXISTS, ota_app_data->app_part);
        return;
    }
}

void jump_to_app(void)
{
    app_header_t *app_header;
    if (ota_app_data->app_part == APP_B_PART)
    {
        app_header = (app_header_t *)__appb_rom_start;
    }
    else
    {
        app_header = (app_header_t *)__appa_rom_start;
    }

    if(memcmp(app_header->app_magic_number, APP_HEADER_MAGIC_NUMBER, sizeof(app_header->app_magic_number)) == 0) {
        LOG_I("app_magic_number: %s", app_header->app_magic_number);
        LOG_I("app_startup_entry: %p", app_header->app_startup_entry);
        LOG_I("app_main_entry: %p", app_header->app_main_entry);

        /* Don't use api mapping method to let App call bootloader apis */
        app_startup_params_t params = {NULL, NULL, NULL};
        app_header->app_startup_entry(&params);
        app_header->app_main_entry(0, NULL);
    } else {
        LOG_W("No application program.");
    }

}

void bootloader_entry(void)
{
    rt_err_t res;

    res = ota_app_data_init();
    log_info("ota_app_data_init %s.", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        goto _exit_;
    }

    if (ota_app_data->ota_tag == OTA_YES)
    {
        ota_app_process();
    }
    else
    {
        jump_to_app();
    }

_exit_:
    return;
}