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

#define flash_page_size 0x2000

static mbr_t mbr = RT_NULL;
static char mbr_buffer[flash_page_size];
static const struct fal_partition *mbr_part = RT_NULL;

static char file_page_buffer[flash_page_size];
static const struct fal_partition *app_part = RT_NULL;

extern char __appa_rom_start[];
extern char __appa_rom_end[];
extern char __appb_rom_start[];
extern char __appb_rom_end[];

mbr_t mbr_init(void)
{
    int res = 0;

    if (mbr_part == RT_NULL)
    {
        mbr_part = fal_partition_find("mbr");
    }

    if (mbr_part == RT_NULL)
    {
        LOG_E("mbr_part is not exits.");
        goto _exit_;
    }

    rt_memset(mbr_buffer, 0xFF, flash_page_size);
    res = fal_partition_read(mbr_part, 0, mbr_buffer, flash_page_size);
    LOG_D("fal_partition_read addr=0, size=%p, res=%p", flash_page_size, res);
    if (res < 0)
    {
        LOG_E("fal_partition_read failed.");
        goto _exit_;
    }
    mbr = (mbr_t)mbr_buffer;

_exit_:
    return mbr;
}

rt_err_t mbr_save(void)
{
    rt_err_t ret = RT_ERROR;
    int res = 0;

    if (mbr_part == RT_NULL)
    {
        LOG_E("mbr_part is not exits.");
        goto _exit_;
    }

    res = fal_partition_erase(mbr_part, 0, flash_page_size);
    LOG_I("fal_partition_erase mbr addr=0, size=%p %s", flash_page_size, res == flash_page_size ? "succces" : "failed");
    if (res < 0)
    {
        goto _exit_;
    }

    res = fal_partition_write(mbr_part, 0, mbr_buffer, flash_page_size);
    if (res < 0)
    {
        LOG_E("fal_partition_write mbr failed.");
    }
    ret = RT_EOK;

_exit_:
    return ret;
}

void ota_app_status_save(ota_tag_e ota_tag_val, ota_state_e ota_proc_state, app_parition_e app_part_no)
{
    mbr->ota_tag = ota_tag_val;
    mbr->ota_state = ota_proc_state;
    mbr->app_part = app_part_no;
    mbr_save();
}

void ota_app_over(ota_tag_e ota_tag_val, ota_state_e ota_proc_state, app_parition_e app_part_no)
{
    ota_app_status_save(ota_tag_val, ota_proc_state, app_part_no);
    rt_hw_cpu_reset();
}

static struct rt_semaphore fs_mnt_sem;
static void fs_mnt_cb(fdnfs_init_status_e *status)
{
    LOG_D("fs_mnt_cb status=%d", *status);
    if (*status == 0)
    {
        rt_sem_release(&fs_mnt_sem);
    }
}

static fdnfs_init_status_e mnt_status;
void ota_app_process(void)
{
    rt_err_t res;
    int ret;
    if (rt_strlen(mbr->ota_file) <= 0)
    {
        ota_app_over(OTA_NO, OTA_FILE_NAME_NOT_EXISTS, mbr->app_part);
        return;
    }

    app_part = fal_partition_find(mbr->app_part == APP_B_PART ? "app_a" : "app_b");
    res = app_part == RT_NULL ? RT_ERROR : RT_EOK;
    LOG_I(
        "find %s partition in onchip flash %s.",
        mbr->app_part == APP_B_PART ? "app_a" : "app_b",
        res == RT_EOK ? "success" : "failed"
    );
    if (res == RT_ERROR)
    {
        ota_app_over(OTA_NO, OTA_FS_INIT_FAILED, mbr->app_part);
        return;
    }

    res = rt_sem_init(&fs_mnt_sem, "fsmntsem", 0, RT_IPC_FLAG_PRIO);
    LOG_I("rt_sem_init fsmntsem res=%d", res);
    if (res != RT_EOK)
    {
        ota_app_over(OTA_NO, OTA_FS_INIT_FAILED, mbr->app_part);
        return;
    }

    fatfs_dhara_nand_init(fs_mnt_cb, &mnt_status);

    res = rt_sem_take(&fs_mnt_sem, 10 * 1000);
    LOG_D("rt_sem_take fs_mnt_sem");

    if (res != RT_EOK)
    {
        ota_app_over(OTA_NO, OTA_FS_INIT_FAILED, mbr->app_part);
        return;
    }

    ret = access(mbr->ota_file, F_OK);
    LOG_I("check ota file %s exists ret=%d", mbr->ota_file, ret);
    if (ret != 0)
    {
        ota_app_over(OTA_NO, OTA_FILE_NOT_EXISTS, mbr->app_part);
        return;
    }

    FILE *ota_file = fopen(mbr->ota_file, "rb");
    LOG_I("ota file %s open %s.", mbr->ota_file, !ota_file ? "failed" : "success");
    if (!ota_file)
    {
        ota_app_over(OTA_NO, OTA_FILE_NOT_EXISTS, mbr->app_part);
        return;
    }

    size_t read_size;
    int blk_no = 0;
    int blk_size = (int)(__appa_rom_end - __appa_rom_start) / flash_page_size;
    // blk_no = mbr->ota_state > 0 && mbr->ota_state < (blk_size - 1) ? mbr->ota_state : 0;
    // fseek(ota_file, blk_no * flash_page_size, SEEK_SET);
    for (; blk_no < blk_size; blk_no++)
    {
        rt_memset(file_page_buffer, 0xFF, flash_page_size);
        read_size = fread(file_page_buffer, 1, flash_page_size, ota_file);
        ret = fal_partition_erase(app_part, blk_no * flash_page_size, flash_page_size);
        LOG_I(
            "fal_partition_erase app_part addr=%p, size=%p ret=%d",
            blk_no * flash_page_size, flash_page_size, ret
        );
        if (ret < 0)
        {
            fclose(ota_file);
            ota_app_over(OTA_NO, OTA_ERASE_FAILED, mbr->app_part);
            return;
        } 
        if (read_size > 0)
        {
            ret = fal_partition_write(app_part, blk_no * flash_page_size, file_page_buffer, flash_page_size);
            LOG_I(
                "fal_partition_write app_part addr=%p, size=%p ret=%d",
                blk_no * flash_page_size, flash_page_size, ret
            );
            if (ret < 0)
            {
                fclose(ota_file);
                ota_app_over(OTA_NO, OTA_WRITE_FAILED, mbr->app_part);
                return;
            }
            // else
            // {
            //     ota_app_status_save(mbr->ota_tag, blk_no, mbr->app_part);
            // }
        }
    }
    fclose(ota_file);

    app_header_t *app_header;
    app_header = (app_header_t *)(mbr->app_part == APP_B_PART ? __appa_rom_start : __appb_rom_start);
    if(rt_memcmp(app_header->app_magic_number, APP_HEADER_MAGIC_NUMBER, sizeof(app_header->app_magic_number)) == 0)
    {
        ota_app_over(OTA_NO, OTA_SUCCESS, mbr->app_part == APP_B_PART ? APP_A_PART : APP_B_PART);
    }
    else
    {
        ota_app_over(OTA_NO, OTA_JUMP_APP_FAILED, mbr->app_part);
    }
}
