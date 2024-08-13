/*
 * @FilePath: on_chip_flash.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-12 15:15:33
 * @copyright : Copyright (c) 2024
 */
#include "on_chip_flash.h"

const struct fal_partition *bl_part;
const struct fal_partition *app_part;

rt_err_t find_app_partition(void)
{
    rt_err_t res = RT_ERROR;
    app_part = fal_partition_find("app");
    res = app_part == RT_NULL ? RT_ERROR : RT_EOK;
    LOG_D("fal_partition_find app %s", res == RT_EOK ? "success" : "failed");
    return res;
}

rt_err_t read_app_partition(rt_uint32_t addr, rt_uint8_t *buf, rt_int32_t size)
{
    rt_err_t res = RT_ERROR;
    rt_int32_t ret;
    ret = fal_partition_read(app_part, addr, buf, size);
    res = ret == size ? RT_EOK : RT_ERROR;
    LOG_D("fal_partition_read %s ret=%d", res == RT_EOK ? "success" : "failed", ret);
    if (res == RT_EOK)
    {
        char msgs[128];
        char msg[8];
        char index=0;
        for (rt_uint8_t i = 0; i < size; i++)
        {
            rt_sprintf(msg, "0x%02X", buf[i]);
            rt_memcpy(msgs + index, msg, rt_strlen(msg));
            index += rt_strlen(msg);
        }
        LOG_D("fal_partition_read addr=0x%02X, size=%d, buf=%s", addr, size, msgs);
    }
    return res;
}

rt_err_t erase_app_partition(rt_uint32_t addr, rt_int32_t size)
{
    rt_err_t res = RT_ERROR;
    rt_int32_t ret;
    ret = fal_partition_erase(app_part, addr, size);
    res = ret == size ? RT_EOK : RT_ERROR;
    LOG_D("fal_partition_erase %s ret=%d", res == RT_EOK ? "success" : "failed", ret);
    return res;
}

rt_err_t write_boolload_partition(rt_uint32_t addr, rt_uint8_t *buf, rt_int32_t size)
{
    rt_err_t res = RT_ERROR;
    rt_int32_t ret;
    LOG_D("write_boolload_partition addr=0x%02X buf=0x%02X size=0x%02X");
    ret = fal_partition_write(app_part, addr, buf, size);
    res = ret == size ? RT_EOK : RT_ERROR;
    LOG_D("fal_partition_read %s ret=%d", res == RT_EOK ? "success" : "failed", ret);
    return res;
}

rt_err_t test_on_chip_flash(void)
{
    rt_err_t res = RT_ERROR;

    res = find_app_partition();
    LOG_D("find_app_partition %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    rt_uint32_t addr = 0;
    rt_int32_t size = 16;

    rt_uint8_t read_buf[size];
    rt_memset(read_buf, 0, size);
    res = read_app_partition(addr, read_buf, size);
    LOG_D("read_app_partition %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = erase_app_partition(addr, size);
    LOG_D("erase_app_partition %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    rt_uint8_t write_buf[size] ;
    rt_memset(write_buf, 0x01, size);
    LOG_D("write_buf=0x%02X", write_buf);
    res = write_boolload_partition(addr, write_buf, size);
    LOG_D("write_boolload_partition %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    rt_memset(read_buf, 0, size);
    res = read_app_partition(addr, read_buf, size);
    LOG_D("read_app_partition %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    return res;
}

MSH_CMD_EXPORT(test_on_chip_flash, test on chip flash);
