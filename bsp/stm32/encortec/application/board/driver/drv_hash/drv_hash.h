/*
 * @FilePath: drv_hash.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-11-01 11:51:02
 * @copyright : Copyright (c) 2024
 */

#ifndef __DRV_HASH_H__
#define __DRV_HASH_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"

rt_err_t drv_hash_init(void);
rt_err_t drv_hash_deinit(void);
rt_err_t drv_hash_hmac_sha256(rt_uint8_t *key, rt_uint32_t key_length, rt_uint8_t *input, rt_uint32_t input_length, rt_uint8_t *output);
rt_err_t drv_hash_md5_create(void);
rt_err_t drv_hash_md5_update(rt_uint8_t *in_buffer, rt_uint32_t in_buffer_size);
rt_err_t drv_hash_md5_finsh(rt_uint8_t *in_buffer, rt_uint32_t in_buffer_size, rt_uint8_t *out_buffer);

#endif