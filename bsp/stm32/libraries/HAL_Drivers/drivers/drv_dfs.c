/*
 * @FilePath: drv_dfs.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-18 11:27:35
 * @copyright : Copyright (c) 2024
 */
#include <board.h>
#include <rtthread.h>

#if defined(RT_USING_DFS)
#include "drv_config.h"
#include "drv_dfs.h"

// #define DRV_DEBUG
#define LOG_TAG             "drv.dfs"
#include <drv_log.h>

#define FS_PARTITION_NAME "fs"

int rt_hw_fs_mount(void) {
    struct rt_device *mtd_dev = RT_NULL;
    mtd_dev = fal_mtd_nor_device_create(FS_PARTITION_NAME);
    if (!mtd_dev)
    {
        LOG_E("Can't create a mtd device on '%s' partition.", FS_PARTITION_NAME);
    }
    else
    {
        /* 挂载 littlefs */
        if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) == 0)
        {
            LOG_I("%s initialized!", FS_PARTITION_NAME);
            return RT_EOK;
        }
        else
        {
            /* 格式化文件系统 */
            dfs_mkfs("lfs", FS_PARTITION_NAME);
            LOG_I("%s formatted!", FS_PARTITION_NAME);
            /* 挂载 littlefs */
            if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) == 0)
            {
                LOG_I("%s initialized!", FS_PARTITION_NAME);
                return RT_EOK;
            }
            else
            {
                LOG_E("Failed to initialize filesystem!");
            }
        }
    }
    return -RT_ERROR;
}

#endif