/*
 * @FilePath: drv_fatfs_dhara_nand.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-27 09:35:18
 * @copyright : Copyright (c) 2024
 */
#include "drv_fatfs_dhara_nand.h"
#include "dfs_fs.h"
#include "drv_nand_flash.h"
#include "dhara_blk_device.h"

#define DBG_SECTION_NAME "DRV_FATFS_DHARA_NAND"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define fatfs_thd_stack_size 0x1000
static struct rt_thread fatfs_thd;
static char fatfs_thd_stack[fatfs_thd_stack_size];

struct fs_mount_cb {
    void (*callback)(fdnfs_init_status_e *status);
    fdnfs_init_status_e *status;
};
typedef struct fs_mount_cb fs_mount_cb_t;
static fs_mount_cb_t fs_mount_res = {0};
static fdnfs_init_status_e fs_mnt_status = FDNFS_INIT_STATUS_MAX;

static void fatfs_dhara_nand_entry(void *args)
{
    int res;

    fs_mount_cb_t *fsmnt_res = (fs_mount_cb_t *)args;

    if (fs_mnt_status != FDNFS_INIT_STATUS_MAX)
    {
        goto _fail_;
    }

    res = rt_hw_nand_flash_init();
    LOG_I("rt_hw_nand_flash_init %s.", res == 0 ? "success" : "failed");
    if (res != 0)
    {
        fs_mnt_status = FDNFS_NAND_FLASH_INIT_FAILED;
        goto _fail_;
    }

    res = dhara_blk_device_init();
    LOG_I("dhara_blk_device_init %s", res == 0 ? "success" : "failed");
    if (res != 0)
    {
        fs_mnt_status = FDNFS_DHARA_BLK_DEV_INIT_FAILED;
        goto _fail_;
    }

    res = (int)fatfs_dhara_nand_mount();
    LOG_I("dfs_mount dharadev elm %s", res == 0 ? "success" : "failed");
    if (res != 0)
    {
        goto _fail_;
    }

_fail_:
    *fsmnt_res->status = fs_mnt_status;
    fsmnt_res->callback(fsmnt_res->status);
    return;
} 

rt_err_t fatfs_dhara_nand_init(void (*callback)(fdnfs_init_status_e *status), fdnfs_init_status_e *status)
{
    rt_err_t res;
    RT_ASSERT(callback != RT_NULL && status != RT_NULL);

    fs_mount_res.callback = callback;
    fs_mount_res.status = status;

    res = rt_thread_init(
        &fatfs_thd, "fatfsthd", fatfs_dhara_nand_entry, &fs_mount_res,
        fatfs_thd_stack, fatfs_thd_stack_size, 25, 10
    );
    LOG_I("rt_thread_init fatfsthd %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }
    res = rt_thread_startup(&fatfs_thd);
    LOG_I("rt_thread_startup fatfsthd %s", res == RT_EOK ? "success" : "failed");

    return res;
}

rt_err_t fatfs_dhara_nand_unmount(void)
{
    int res = -1;

    if (fs_mnt_status == FDNFS_MOUNTED)
    {
        res = dfs_unmount(FATFS_BASE_PATH);
        fs_mnt_status = res == 0 ? FDNFS_UNMOUNT : fs_mnt_status;
    }
    else
    {
        LOG_E("fatfs_dhara_nand is not mounted.");
    }

    return res == 0 ? RT_EOK : RT_ERROR;
}

rt_err_t fatfs_dhara_nand_mount(void)
{
    int res = -1;
    if (fs_mnt_status == FDNFS_MOUNTED)
    {
        res = 0;
    }
    else if (fs_mnt_status == FDNFS_UNMOUNT || fs_mnt_status == FDNFS_INIT_STATUS_MAX)
    {
        if (fs_mnt_status == FDNFS_UNMOUNT)
        {
            refresh_dhara_map();
        }
        res = dfs_mount(DHARA_BLK_DEV_NAME, FATFS_BASE_PATH, FATFS_NAME, 0, 0);
        if (res != 0)
        {
            res = dfs_mkfs(FATFS_NAME, DHARA_BLK_DEV_NAME);
            LOG_I("dfs_mkfs elm dharadev %s", res == 0 ? "success" : "failed");
            if (res == 0)
            {
                res = dfs_mount(DHARA_BLK_DEV_NAME, FATFS_BASE_PATH, FATFS_NAME, 0, 0);
            }
        }
        LOG_I("dfs_mount dharadev elm %s", res == 0 ? "success" : "failed");
        fs_mnt_status = res == 0 ? FDNFS_MOUNTED : FDNFS_DFS_MOUNT_FAILED;
    }
    else
    {
        LOG_E("fatfs_dhara_nand init failed.");
    }
    return res == 0 ? RT_EOK : RT_ERROR;
}

rt_err_t fatfs_dhara_nand_remount(void)
{
    rt_err_t res;
    res = fatfs_dhara_nand_unmount();
    if (res == RT_EOK)
    {
        res = fatfs_dhara_nand_mount();
    }
    return res;
}

static void test_nand_fs_mount_result_cb(fdnfs_init_status_e *status)
{
    LOG_D("test_nand_fs_mount_result_cb status %d", *status);
}

static fdnfs_init_status_e fs_status;
static void test_nand_fs_init(void)
{
    rt_err_t res = fatfs_dhara_nand_init(test_nand_fs_mount_result_cb, &fs_status);
    LOG_D("fatfs_dhara_nand_init res=%d", fs_status);
}
MSH_CMD_EXPORT(test_nand_fs_init, test nand fs init);

static void test_nand_fs_unmount(void)
{
    rt_err_t res = fatfs_dhara_nand_unmount();
    LOG_D("fatfs_dhara_nand_unmount res=%d", res);
}
MSH_CMD_EXPORT(test_nand_fs_unmount, test nand fs unmount);

static void test_nand_fs_mount(void)
{
    rt_err_t res = fatfs_dhara_nand_mount();
    LOG_D("fatfs_dhara_nand_mount res=%d", res);
}
MSH_CMD_EXPORT(test_nand_fs_mount, test nand fs mount);
