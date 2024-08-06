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
#define VBAT_ADC_CHANNEL RT_ADC_INTERN_CH_VBAT

static rt_err_t adc_vol_read(rt_int8_t channel, rt_uint32_t *value)
{
    rt_adc_device_t adc_dev;
    rt_err_t ret = RT_EOK;

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_NAME);
    LOG_E("find %s device %s.", ADC_NAME, (adc_dev == RT_NULL ? "falied" : "success"));
    if (adc_dev == RT_NULL)
    {
        return RT_ERROR;
    }

    *value = rt_adc_voltage(adc_dev, channel);
    LOG_D("rt_adc_voltage channel %d value %d", channel, *value);

    // ret = rt_adc_enable(adc_dev, channel);
    // LOG_D("rt_adc_enable channel %d res %s. value %d", channel, (ret == RT_EOK ? "success" : "failed"), *value);
    // if (ret != RT_EOK)
    // {
    //     return ret;
    // }
    // *value = rt_adc_read(adc_dev, channel);
    // LOG_D("CUR_ADC value %d", *value);
    // ret = rt_adc_disable(adc_dev, channel);
    // LOG_D("rt_adc_disable channel %d res %s. ret %d", channel, (ret == RT_EOK ? "success" : "failed"), ret);
    // if (*value > 0)
    // {
    //     ret = RT_EOK;
    // }

    return ret;
}

rt_err_t cur_vol_read(rt_uint32_t *value)
{
    rt_err_t res = RT_EOK;
    res = adc_vol_read((rt_int8_t)CUR_ADC_CHANNEL, value);
    return res;
}

rt_err_t vcat_vol_read(rt_uint32_t *value)
{
    rt_err_t res = RT_EOK;
    res = adc_vol_read((rt_int8_t)VCAT_ADC_CHANNEL, value);
    return res;
}

rt_err_t vbat_vol_read(rt_uint32_t *value)
{
    rt_err_t res = RT_EOK;
    res = adc_vol_read((rt_int8_t)VBAT_ADC_CHANNEL, value);
    return res;
}

rt_err_t vrefint_vol_read(rt_uint32_t *value)
{
    rt_err_t res = RT_EOK;
    res = adc_vol_read((rt_int8_t)RT_ADC_INTERN_CH_VREF, value);
    return res;
}

static void test_read_voltage(int argc, char *argv[])
{
    rt_err_t res;
    rt_uint32_t cur_vol = 0, vcat_vol = 0, vbat_vol = 0, vrefint_vol = 0;
    res = cur_vol_read(&cur_vol);
    LOG_D("cur_vol_read %s, cur_vol %d", res == RT_EOK ? "success" : "failed", cur_vol);
    res = vcat_vol_read(&vcat_vol);
    LOG_D("vcat_vol_read %s, vcat_vol %d", res == RT_EOK ? "success" : "failed", vcat_vol);
    res = vbat_vol_read(&vbat_vol);
    LOG_D("vbat_vol_read %s, vbat_vol %d", res == RT_EOK ? "success" : "failed", vbat_vol);
    res = vrefint_vol_read(&vrefint_vol);
    LOG_D("vrefint_vol_read %s, vrefint_vol %d", res == RT_EOK ? "success" : "failed", vrefint_vol);
}

MSH_CMD_EXPORT(test_read_voltage, TEST READ voltage);
