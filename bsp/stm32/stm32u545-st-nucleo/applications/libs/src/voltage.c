/*
 * @FilePath: voltage.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-30 16:27:26
 * @copyright : Copyright (c) 2024
 */

#include "rtthread.h"
#include "rtdevice.h"

#define DBG_SECTION_NAME "VOLTAGE"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define ADC_NAME "adc1"
#define CUR_ADC_CHANNEL 1
#define VCAT_ADC_CHANNEL 2

static rt_err_t cur_vol_read(int argc, char *argv[])
{
    rt_adc_device_t adc_dev;
    rt_uint32_t value, vol;
    rt_err_t ret = RT_EOK;

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_NAME);
    if (adc_dev == RT_NULL)
    {
        LOG_E("Can not find %s device!\r\n", ADC_NAME);
        return RT_ERROR;
    }

    ret = rt_adc_enable(adc_dev, CUR_ADC_CHANNEL);
    value = rt_adc_read(adc_dev, CUR_ADC_CHANNEL);
    LOG_D("CUR_ADC value %d\r\n", value);
    ret = rt_adc_disable(adc_dev, CUR_ADC_CHANNEL);
    return ret;
}

MSH_CMD_EXPORT(cur_vol_read, CUR VOL Read);

static rt_err_t vcat_vol_read(int argc, char *argv[])
{
    rt_adc_device_t adc_dev;
    rt_uint32_t value, vol;
    rt_err_t ret = RT_EOK;

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_NAME);
    if (adc_dev == RT_NULL)
    {
        LOG_E("Can not find %s device!\r\n", ADC_NAME);
        return RT_ERROR;
    }

    ret = rt_adc_enable(adc_dev, VCAT_ADC_CHANNEL);
    value = rt_adc_read(adc_dev, VCAT_ADC_CHANNEL);
    LOG_D("VCAT_ADC value %d\r\n", value);
    ret = rt_adc_disable(adc_dev, VCAT_ADC_CHANNEL);
    return ret;
}

MSH_CMD_EXPORT(vcat_vol_read, VCAT VOL Read);
