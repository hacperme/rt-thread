/*
 * @FilePath: drv_fatfs_dhara_nand.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-27 09:36:03
 * @copyright : Copyright (c) 2024
 */
#ifndef __DRV_FATFS_DHARA_NAND_H__
#define __DRV_FATFS_DHARA_NAND_H__

#include <rtthread.h>
#include <rtdevice.h>

#define FATFS_NAME "elm"
#define FATFS_BASE_PATH "/"

typedef enum {
    FDNFS_MOUNTED = 0,
    FDNFS_UNMOUNT,
    FDNFS_DFS_MOUNT_FAILED,
    FDNFS_DHARA_BLK_DEV_INIT_FAILED,
    FDNFS_NAND_FLASH_INIT_FAILED,
    FDNFS_INIT_STATUS_MAX
} fdnfs_init_status_e;

rt_err_t fatfs_dhara_nand_init(void (*callback)(fdnfs_init_status_e *status), fdnfs_init_status_e *status);
rt_err_t fatfs_dhara_nand_unmount(void);
rt_err_t fatfs_dhara_nand_mount(void);
rt_err_t fatfs_dhara_nand_remount(void);

#endif