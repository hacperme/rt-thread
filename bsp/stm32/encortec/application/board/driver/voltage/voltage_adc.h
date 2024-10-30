/*
 * @FilePath: voltage_adc.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-25 14:35:02
 * @copyright : Copyright (c) 2024
 */
#ifndef __VOLTAGE_ADC_H__
#define __VOLTAGE_ADC_H__

#include "rtthread.h"
#include "board.h"

rt_err_t adc_dma_init(void);
rt_err_t vcap_vol_read(rt_uint16_t *value);
rt_err_t vbat_vol_read(rt_uint16_t *value);
rt_err_t cur_vol_read_start(void);
rt_err_t cur_vol_read_stop(void);
rt_err_t cur_vol_read(rt_uint16_t **cur_buff, rt_uint16_t *buff_size);

#endif