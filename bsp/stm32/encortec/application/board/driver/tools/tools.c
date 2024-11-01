/*
 * @FilePath: tools.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-22 14:43:19
 * @copyright : Copyright (c) 2024
 */

#include "tools.h"
#include "logging.h"

rt_err_t hwcrypto_crc8(const rt_uint8_t *input, rt_size_t length, rt_uint32_t *value)
{
    rt_err_t res = RT_ERROR;
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
        log_error("rt_hwcrypto_crc_create failed");
        return res;
    }
    rt_hwcrypto_crc_cfg(ctx, &cfg);
    *value = rt_hwcrypto_crc_update(ctx, input, length);
    log_debug("rt_hwcrypto_crc_update CRC8 value=0x%02X", *value);
    rt_hwcrypto_crc_destroy(ctx);
    res = RT_EOK;
    return res;
}

rt_err_t crc8_check(const rt_uint8_t *input, rt_size_t length, const rt_uint8_t *cmp_val)
{
    rt_err_t res = RT_ERROR;
    rt_uint32_t crc_val = 0;
    res = hwcrypto_crc8(input, length, &crc_val);
    if (res == RT_EOK)
    {
        res = (rt_uint8_t)crc_val == *cmp_val ? RT_EOK : RT_ERROR;
    }
    return res;
}

rt_err_t hwcrypto_crc32(const rt_uint8_t *input, rt_size_t length, rt_uint32_t *value)
{
    rt_err_t res = RT_ERROR;
    struct rt_hwcrypto_ctx *ctx;
    struct hwcrypto_crc_cfg cfg = {
        .last_val = 0xFFFFFFFF,
        .poly = 0x04C11DB7,
        .width = 32,
        .xorout = 0xFFFFFFFF,
        .flags = CRC_FLAG_REFIN|CRC_FLAG_REFOUT,
    };
    ctx = rt_hwcrypto_crc_create(rt_hwcrypto_dev_default(), HWCRYPTO_CRC_CRC32);
    if (ctx == RT_NULL)
    {
        log_error("rt_hwcrypto_crc_create failed");
        return res;
    }
    rt_hwcrypto_crc_cfg(ctx, &cfg);
    *value = rt_hwcrypto_crc_update(ctx, input, length);
    log_debug("rt_hwcrypto_crc_update res=0x%02X", *value);
    rt_hwcrypto_crc_destroy(ctx);

    res = RT_EOK;
    return res;
}

#define BUFFER_SIZE    114
void test_crc32_check(void)
{
    uint32_t aDataBuffer[BUFFER_SIZE] =
    {
        0x00001021, 0x20423063, 0x408450a5, 0x60c670e7, 0x9129a14a, 0xb16bc18c,
        0xd1ade1ce, 0xf1ef1231, 0x32732252, 0x52b54294, 0x72f762d6, 0x93398318,
        0xa35ad3bd, 0xc39cf3ff, 0xe3de2462, 0x34430420, 0x64e674c7, 0x44a45485,
        0xa56ab54b, 0x85289509, 0xf5cfc5ac, 0xd58d3653, 0x26721611, 0x063076d7,
        0x569546b4, 0xb75ba77a, 0x97198738, 0xf7dfe7fe, 0xc7bc48c4, 0x58e56886,
        0x78a70840, 0x18612802, 0xc9ccd9ed, 0xe98ef9af, 0x89489969, 0xa90ab92b,
        0x4ad47ab7, 0x6a961a71, 0x0a503a33, 0x2a12dbfd, 0xfbbfeb9e, 0x9b798b58,
        0xbb3bab1a, 0x6ca67c87, 0x5cc52c22, 0x3c030c60, 0x1c41edae, 0xfd8fcdec,
        0xad2abd0b, 0x8d689d49, 0x7e976eb6, 0x5ed54ef4, 0x2e321e51, 0x0e70ff9f,
        0xefbedfdd, 0xcffcbf1b, 0x9f598f78, 0x918881a9, 0xb1caa1eb, 0xd10cc12d,
        0xe16f1080, 0x00a130c2, 0x20e35004, 0x40257046, 0x83b99398, 0xa3fbb3da,
        0xc33dd31c, 0xe37ff35e, 0x129022f3, 0x32d24235, 0x52146277, 0x7256b5ea,
        0x95a88589, 0xf56ee54f, 0xd52cc50d, 0x34e224c3, 0x04817466, 0x64475424,
        0x4405a7db, 0xb7fa8799, 0xe75ff77e, 0xc71dd73c, 0x26d336f2, 0x069116b0,
        0x76764615, 0x5634d94c, 0xc96df90e, 0xe92f99c8, 0xb98aa9ab, 0x58444865,
        0x78066827, 0x18c008e1, 0x28a3cb7d, 0xdb5ceb3f, 0xfb1e8bf9, 0x9bd8abbb,
        0x4a755a54, 0x6a377a16, 0x0af11ad0, 0x2ab33a92, 0xed0fdd6c, 0xcd4dbdaa,
        0xad8b9de8, 0x8dc97c26, 0x5c644c45, 0x3ca22c83, 0x1ce00cc1, 0xef1fff3e,
        0xdf7caf9b, 0xbfba8fd9, 0x9ff86e17, 0x7e364e55, 0x2e933eb2, 0x0ed11ef0
    };

    rt_uint32_t result = 0;
    rt_err_t res;
    res = hwcrypto_crc32((uint8_t *)aDataBuffer, BUFFER_SIZE * 4, &result);
    log_debug("hwcrypto_crc32 res=%d, result=%p", res, result);

    uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    res = hwcrypto_crc32((uint8_t *)data, 8, &result);
    log_debug("hwcrypto_crc32 res=%d, result=%p", res, result);

    // extern uint32_t CRC32(uint8_t *data, uint32_t size);
    // result = 0;
    // result = CRC32((uint8_t *)aDataBuffer, BUFFER_SIZE * 4);
    // log_debug("CRC32 result=%p", result);
}