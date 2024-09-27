/*
 * @FilePath: dhara_blk_device.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-23 14:40:39
 * @copyright : Copyright (c) 2024
 */
#ifndef __DHARA_BLK_DEVICE_H__
#define __DHARA_BLK_DEVICE_H__

#define DHARA_BLK_DEV_NAME "dharadev"

rt_err_t dhara_blk_device_init(void);
void refresh_dhara_map(void);

#endif