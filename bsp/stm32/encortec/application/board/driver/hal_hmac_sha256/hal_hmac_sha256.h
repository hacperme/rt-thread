/*
 * @FilePath: hal_hmac_sha256.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-11 15:59:35
 * @copyright : Copyright (c) 2024
 */

#ifndef __HAL_HMAC_SHA256_H__
#define __HAL_HMAC_SHA256_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"

rt_err_t hal_hmac_sha256_init(void);
rt_err_t hal_hmac_sha256_deinit(void);
rt_err_t hal_hmac_sha256(rt_uint8_t *key, rt_uint32_t key_length, rt_uint8_t *input, rt_uint32_t input_length, rt_uint8_t *output);

#endif