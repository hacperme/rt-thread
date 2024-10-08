/*
 * @FilePath: drv_yaffs_nand.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-27 13:58:42
 * @copyright : Copyright (c) 2024
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "dfs_fs.h"
#include "yaffsfs.h"

#define DBG_SECTION_NAME "DRV_YAFFS_NAND"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>


rt_err_t yaffs_nand_init(void)
{
    int res;
    static struct rt_mtd_nand_device *nand_dev;
    nand_dev = (struct rt_mtd_nand_device *)rt_device_find("nand");
    
    // Set log enable.
    yaffs_set_trace(0xFFFF);
    res = yaffs_start_up(nand_dev, "/");
    LOG_I("yaffs_start_up %s", res == RT_EOK ? "success" : "failed");

    res = dfs_mount("nand", "/", "yaffs", 0, 0);
    if (res != 0)
    {
        res = dfs_mkfs("yaffs", "nand");
        if (res == 0)
        {
            res = dfs_mount("nand", "/", "yaffs", 0, 0);
        }
        else
        {
            rt_err_t err_no = rt_get_errno();
            LOG_E("dfs_mount faield error_code=%d", err_no);
            LOG_E("dfs_mkfs yaffs nand failed. res=%d", res);
        }
    }
    LOG_D("dfs_mount nand yaffs %s res=%d", res == 0 ? "success" : "failed", res);

    int mem_total, mem_used, mem_used_max;
    mem_total = mem_used = mem_used_max = 0;
    rt_memory_info(&mem_total, &mem_used, &mem_used_max);
    LOG_I(
        "mem_total=%dKB, mem_used=%dKB, mem_used_max=%dKB, mem_free=%dKB",
        mem_total / 1024, mem_used / 1024, mem_used_max / 1024,
        (mem_total - mem_used) / 1024
    );

    return res == 0 ? RT_EOK : RT_ERROR;
}