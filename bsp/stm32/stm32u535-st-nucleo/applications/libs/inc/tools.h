/*
 * @FilePath: tools.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-22 14:07:45
 * @copyright : Copyright (c) 2024
 */
#ifndef __TOOLS_H__
#define __TOOLS_H__

#include "rtthread.h"
#include "rtdevice.h"

#define rt_tick_diff(a, b) (a <= b ? b - a : UINT32_MAX - a + b + 1)
#define set_bit(x, y) (x |= (1 << y))
#define clr_bit(x, y) (x &= ~(1 << y))

rt_err_t crc8_check(const rt_uint8_t *input, rt_size_t length, const rt_uint8_t *cmp_val);

#endif  // __TOOLS_H__