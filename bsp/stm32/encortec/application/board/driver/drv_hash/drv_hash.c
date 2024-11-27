/*
 * @FilePath: drv_hash.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-11-01 11:50:53
 * @copyright : Copyright (c) 2024
 */
// #include "stm32u5xx_hal_hash.h"
#include "stm32u5xx_hal.h"
#include "logging.h"
#include "tools.h"

static HASH_HandleTypeDef hhash;
static struct rt_mutex drv_hash_mutex;
static rt_uint8_t drv_hash_init_over = 0;
static rt_uint8_t drv_hash_md5_create_tag = 0;

rt_err_t drv_hash_init(void)
{
    rt_err_t res = drv_hash_init_over == 1 ? RT_EOK : RT_ERROR;
    if (res == RT_EOK) return res;

    rt_memset(&drv_hash_mutex, 0, sizeof(drv_hash_mutex));
    res = rt_mutex_init(&drv_hash_mutex, "hashmtx", RT_IPC_FLAG_PRIO);
    drv_hash_init_over = res == RT_EOK ? 1 : 0;

    return res;
}

rt_err_t drv_hash_deinit(void)
{
    rt_err_t res = drv_hash_init_over == 0 ? RT_EOK : RT_ERROR;
    if (res == RT_EOK) return res;

    res = rt_mutex_detach(&drv_hash_mutex);
    drv_hash_init_over = res == RT_EOK ? 0 : 1;

    return res;
}

rt_err_t drv_hash_hmac_sha256(rt_uint8_t *key, rt_uint32_t key_length, rt_uint8_t *input, rt_uint32_t input_length, rt_uint8_t *output)
{
    rt_err_t res = drv_hash_init_over == 0 ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    res = drv_hash_md5_create_tag == 1 ? RT_ERROR : RT_EOK;
    if (res != RT_EOK) return res;

    res = (key == RT_NULL || key_length == 0) ? RT_EINVAL : RT_EOK;
    if (res != RT_EOK) return res;

    res = (input == RT_NULL || input_length == 0) ? RT_EINVAL : RT_EOK;
    if (res != RT_EOK) return res;

    res = output == RT_NULL ? RT_EINVAL : RT_EOK;
    if (res != RT_EOK) return res;

    rt_mutex_take(&drv_hash_mutex, RT_WAITING_FOREVER);

    rt_memset(&hhash, 0, sizeof(hhash));
    hhash.Init.DataType = HASH_DATATYPE_8B;
    hhash.Init.KeySize = key_length;
    hhash.Init.pKey = (uint8_t *)key;
    res = HAL_HASH_Init(&hhash) == HAL_OK ? RT_EOK : RT_ERROR;
    if (res == RT_EOK)
    {
        res = HAL_HMACEx_SHA256_Start(&hhash, (uint8_t *)input, input_length, output, 0xFF) == HAL_OK ? RT_EOK : RT_ERROR;
    }

    __HAL_HASH_RESET_HANDLE_STATE(&hhash);
    HAL_HASH_DeInit(&hhash);
    rt_mutex_release(&drv_hash_mutex);
    return res;
}

rt_err_t drv_hash_md5_create(void)
{
    rt_err_t res = drv_hash_init_over == 1 ? RT_EOK : RT_ERROR;
    if (res != RT_EOK) return res;

    res = drv_hash_md5_create_tag == 1 ? RT_EOK : RT_ERROR;
    if (res == RT_EOK) return res;

    rt_mutex_take(&drv_hash_mutex, RT_WAITING_FOREVER);

    rt_memset(&hhash, 0, sizeof(hhash));
    hhash.Init.DataType = HASH_DATATYPE_8B;
    res = HAL_HASH_Init(&hhash) == HAL_OK ? RT_EOK : RT_ERROR;
    drv_hash_md5_create_tag = res == RT_EOK ? 1 : 0;

    rt_mutex_release(&drv_hash_mutex);

    return res;
}

rt_err_t drv_hash_md5_update(rt_uint8_t *in_buffer, rt_uint32_t in_buffer_size)
{
    rt_err_t res = drv_hash_md5_create_tag == 1 ? RT_EOK : RT_ERROR;
    if (res != RT_EOK) return res;

    res = (in_buffer == RT_NULL || in_buffer_size == 0 || in_buffer_size % 4 != 0) ? RT_EINVAL : RT_EOK;
    if (res != RT_EOK) return res;

    rt_mutex_take(&drv_hash_mutex, RT_WAITING_FOREVER);

    int ret = HAL_HASH_MD5_Accmlt(&hhash, in_buffer, in_buffer_size);
    // log_debug("HAL_HASH_MD5_Accmlt ret=%d", ret);
    res = ret == HAL_OK ? RT_EOK : RT_ERROR;

    rt_mutex_release(&drv_hash_mutex);

    return res;
}

rt_err_t drv_hash_md5_finsh(rt_uint8_t *in_buffer, rt_uint32_t in_buffer_size, rt_uint8_t *out_buffer)
{
    rt_err_t res = drv_hash_md5_create_tag == 1 ? RT_EOK : RT_ERROR;
    if (res != RT_EOK) return res;

    res = (in_buffer == RT_NULL || in_buffer_size == 0) ? RT_EINVAL : RT_EOK;
    if (res != RT_EOK) return res;

    res = out_buffer != RT_NULL ? RT_EOK : RT_EINVAL;
    if (res != RT_EOK) return res;

    rt_mutex_take(&drv_hash_mutex, RT_WAITING_FOREVER);

    int ret = HAL_HASH_MD5_Accmlt_End(&hhash, in_buffer, in_buffer_size, out_buffer, 0xFF);
    // log_debug("HAL_HASH_MD5_Accmlt_End ret=%d", ret);
    res = ret == HAL_OK ? RT_EOK : RT_ERROR;

    rt_mutex_release(&drv_hash_mutex);

    return res;
}

rt_err_t drv_hash_md5_destroy(void)
{
    rt_err_t res = drv_hash_md5_create_tag == 0 ? RT_EOK : RT_ERROR;
    if (res == RT_EOK) return res;

    rt_mutex_take(&drv_hash_mutex, RT_WAITING_FOREVER);

    __HAL_HASH_RESET_HANDLE_STATE(&hhash);
    HAL_HASH_DeInit(&hhash);
    drv_hash_md5_create_tag = 0;
    res = RT_EOK;

    rt_mutex_release(&drv_hash_mutex);

    return res;
}

static rt_err_t test_hash_hmac_sha256(void)
{
    rt_err_t res;

    static rt_uint8_t key_buf[4] = {1, 2, 3, 4};
    static rt_uint8_t input_buf[32] = {0};
    static rt_uint8_t output_buf[32] = {0};
    rt_uint8_t i;
    
    for (i = 0; i < 32; i++)
    {
        input_buf[i] = i;
    }

    res = drv_hash_hmac_sha256(key_buf, 4, input_buf, 32, output_buf);
    log_debug("drv_hash_hmac_sha256 %s", res_msg(res == RT_EOK));
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
    res = drv_hash_hmac_sha256(key_buf, 4, input_buf, 32, output_buf);
    log_debug("drv_hash_hmac_sha256 %s", res_msg(res == RT_EOK));
    if (res == RT_EOK)
    {
        for (i = 0; i < 32; i++)
        {
            log_debug("%02X", output_buf[i]);
        }
    }
    
    return res;
}

#define test_md5_buffer_size 255
static rt_err_t test_hash_md5(void)
{
    rt_err_t res;

    /* 模拟文件内容MD5校验, 需要多次读取文件内容进行更新 */
    res = drv_hash_md5_create();
    log_debug("drv_hash_md5_create %s", res_msg(res == RT_EOK));

    rt_uint8_t in_buff[test_md5_buffer_size] = {0};
    rt_uint8_t out_buff[16] = {0};
    rt_uint16_t i;
    rt_uint16_t buff_range = sizeof(in_buff) % 32 == 0 ? (sizeof(in_buff) / 32 - 1) : (sizeof(in_buff) / 32);
    log_debug("buff_range=%d", buff_range);
    for (i = 0; i < test_md5_buffer_size; i++) in_buff[i] = i;
    for (i = 0; i < buff_range; i++)
    {
        res = drv_hash_md5_update(in_buff + (i * 32), 32);
        log_debug("drv_hash_md5_update %s", res_msg(res == RT_EOK));
        if (res != RT_EOK) break;
    }
    if (res == RT_EOK)
    {
        rt_uint16_t last_buff_size = sizeof(in_buff) % 32 == 0 ? 32 : (sizeof(in_buff) % 32);
        log_debug("i=%d, last_buff_size=%d", i, last_buff_size);
        drv_hash_md5_finsh(in_buff + (i * 32), last_buff_size, out_buff);
        log_debug("drv_hash_md5_finsh %s", res_msg(res == RT_EOK));
        log_debug(
            "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
            out_buff[0], out_buff[1], out_buff[2], out_buff[3], out_buff[4], out_buff[5], out_buff[6],
            out_buff[7], out_buff[8], out_buff[9], out_buff[10], out_buff[11], out_buff[12], out_buff[13],
            out_buff[14], out_buff[15]
        );
    }
    drv_hash_md5_destroy();

    /* 模拟小字符串MD5加密 */
    res = drv_hash_md5_create();
    log_debug("drv_hash_md5_create %s", res_msg(res == RT_EOK));
    rt_memset(out_buff, 0, 16);
    drv_hash_md5_finsh(in_buff, test_md5_buffer_size, out_buff);
    log_debug("drv_hash_md5_finsh %s", res_msg(res == RT_EOK));
    log_debug(
        "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
        out_buff[0], out_buff[1], out_buff[2], out_buff[3], out_buff[4], out_buff[5], out_buff[6],
        out_buff[7], out_buff[8], out_buff[9], out_buff[10], out_buff[11], out_buff[12], out_buff[13],
        out_buff[14], out_buff[15]
    );
    drv_hash_md5_destroy();

    return res;
}

void test_drv_hash(void)
{
    rt_err_t res;
    res = drv_hash_init();
    log_debug("drv_hash_init %s", res_msg(res == RT_EOK));

    // res = test_hash_hmac_sha256();
    // log_debug("test_hash_hmac_sha256 %s", res_msg(res == RT_EOK));

    res = test_hash_md5();
    log_debug("test_hash_md5 %s", res_msg(res == RT_EOK));

    res = drv_hash_deinit();
    log_debug("drv_hash_deinit %s", res_msg(res == RT_EOK));
}