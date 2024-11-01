/*
 * @FilePath: hal_hmac_sha256.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-11 15:59:50
 * @copyright : Copyright (c) 2024
 */

#include "hal_hmac_sha256.h"
#include "stm32u5xx_hal_hash.h"
#include "logging.h"
#include "tools.h"

static HASH_HandleTypeDef hhash;
static struct rt_mutex hal_hmac_sha256_mutex;
static rt_uint8_t hal_hmac_sha256_init_over = 0;

rt_err_t hal_hmac_sha256_init(void)
{
    rt_err_t res = RT_ERROR;
    if (hal_hmac_sha256_init_over == 1)
    {
        res = RT_EOK;
        return res;
    }

    res = rt_mutex_init(&hal_hmac_sha256_mutex, "hhs256", RT_IPC_FLAG_PRIO);
    hal_hmac_sha256_init_over = res == RT_EOK ? 1 : 0;

    return res;
}

rt_err_t hal_hmac_sha256_deinit(void)
{
    rt_err_t res = RT_ERROR;
    if (hal_hmac_sha256_init_over == 0)
    {
        res = RT_EOK;
        return res;
    }

    res = rt_mutex_detach(&hal_hmac_sha256_mutex);
    hal_hmac_sha256_init_over = res == RT_EOK ? 0 : 1;

    return res;
}

rt_err_t hal_hmac_sha256(rt_uint8_t *key, rt_uint32_t key_length, rt_uint8_t *input, rt_uint32_t input_length, rt_uint8_t *output)
{
    rt_err_t res = RT_ERROR;
    if (hal_hmac_sha256_init_over == 0)
    {
        return res;
    }
    if (key == RT_NULL || (key != RT_NULL && key_length == 0))
    {
        res = RT_EINVAL;
        return res;
    }
    if (input == RT_NULL || (input != RT_NULL && input_length == 0))
    {
        res = RT_EINVAL;
        return res;
    }
    if (output == RT_NULL)
    {
        res = RT_EINVAL;
        return res;
    }

    rt_mutex_take(&hal_hmac_sha256_mutex, RT_WAITING_FOREVER);

    hhash.Init.DataType = HASH_DATATYPE_8B;
    hhash.Init.KeySize = key_length;
    hhash.Init.pKey = (uint8_t *)key;
    if (HAL_HASH_Init(&hhash) != HAL_OK)
    {
        goto _exit_;
    }

    if (HAL_HMACEx_SHA256_Start(&hhash, (uint8_t *)input, input_length, output, 0xFF) != HAL_OK)
    {
        goto _exit_;
    }
    res = RT_EOK;

_exit_:
    __HAL_HASH_RESET_HANDLE_STATE(&hhash);
    HAL_HASH_DeInit(&hhash);
    rt_mutex_release(&hal_hmac_sha256_mutex);
    return res;
}

void test_hal_hmac_sha256(void)
{
    rt_err_t res;
    res = hal_hmac_sha256_init();
    log_debug("hal_hmac_sha256_init %s", res_msg(res == RT_EOK));

    static rt_uint8_t key_buf[4] = {1, 2, 3, 4};
    static rt_uint8_t input_buf[32] = {0};
    static rt_uint8_t output_buf[32] = {0};
    rt_uint8_t i;
    
    for (i = 0; i < 32; i++)
    {
        input_buf[i] = i;
    }

    res = hal_hmac_sha256(key_buf, 4, input_buf, 32, output_buf);
    log_debug("hal_hmac_sha256 %s", res_msg(res == RT_EOK));
    if (res == RT_EOK)
    {
        for (i = 0; i < 32; i++)
        {
            log_debug("%02X", output_buf[i]);
        }
    }

    for (i = 0; i < 32; i++)
    {
        input_buf[i] = i + 32;
    }
    res = hal_hmac_sha256(key_buf, 4, input_buf, 32, output_buf);
    log_debug("hal_hmac_sha256 %s", res_msg(res == RT_EOK));
    if (res == RT_EOK)
    {
        for (i = 0; i < 32; i++)
        {
            log_debug("%02X", output_buf[i]);
        }
    }
}