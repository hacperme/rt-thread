/*
 * @FilePath: tools.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-22 14:43:19
 * @copyright : Copyright (c) 2024
 */

#include "tools.h"

#define DBG_TAG "TOOLS"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

rt_err_t crc8_check(const rt_uint8_t *input, rt_size_t length, const rt_uint8_t *cmp_val)
{
    rt_err_t res = RT_ERROR;
    rt_uint32_t crc_val = 0;
    struct rt_hwcrypto_ctx *ctx;
    struct hwcrypto_crc_cfg cfg = {
        .last_val = 0xFF,
        .poly = 0x31,
        .width = 8,
        .xorout = 0x00,
        .flags = 0,
    };
    ctx = rt_hwcrypto_crc_create(rt_hwcrypto_dev_default(), HWCRYPTO_CRC_CRC8);
    if (ctx == RT_NULL)
    {
        LOG_E("rt_hwcrypto_crc_create failed");
        return res;
    }
    rt_hwcrypto_crc_cfg(ctx, &cfg);
    crc_val = rt_hwcrypto_crc_update(ctx, input, length);
    LOG_D("rt_hwcrypto_crc_update res=0x%02X", crc_val);
    rt_hwcrypto_crc_destroy(ctx);

    res = (rt_uint8_t)crc_val == *cmp_val ? RT_EOK : RT_ERROR;
    return res;
}
