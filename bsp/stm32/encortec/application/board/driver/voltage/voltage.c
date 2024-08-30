/*
 * @FilePath: voltage.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-30 16:27:26
 * @copyright : Copyright (c) 2024
 */

#include "voltage.h"
#include "logging.h"

#define ADC_NAME "adc1"
#define CUR_ADC_CHANNEL 1
#define VCAT_ADC_CHANNEL 2
#define VBAT_ADC_CHANNEL RT_ADC_INTERN_CH_VBAT

static rt_err_t adc_vol_read(rt_int8_t channel, rt_uint16_t *value)
{
    rt_err_t res;
    rt_adc_device_t adc_dev;
    rt_uint16_t vol = 0;

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_NAME);
    log_error("find %s device %s.", ADC_NAME, (adc_dev == RT_NULL ? "falied" : "success"));
    res = !adc_dev ? RT_ERROR : RT_EOK;
    if (res != RT_EOK)
    {
        return res;
    }

    /* vref 3305 */
    vol = rt_adc_voltage(adc_dev, channel);
    log_debug("rt_adc_voltage channel %d value %d", channel, vol);
    res = vol > 0 ? RT_EOK : RT_ERROR;
    if (res == RT_EOK)
    {
        *value = vol;
    }

    return res;
}

rt_err_t cur_vol_read(rt_uint16_t *value)
{
    rt_err_t res = RT_EOK;
    res = adc_vol_read((rt_int8_t)CUR_ADC_CHANNEL, value);
    return res;
}

rt_err_t vcat_vol_read(rt_uint16_t *value)
{
    rt_err_t res = RT_EOK;
    res = adc_vol_read((rt_int8_t)VCAT_ADC_CHANNEL, value);
    return res;
}

rt_err_t vbat_vol_read(rt_uint16_t *value)
{
    rt_err_t res = RT_EOK;
    res = adc_vol_read((rt_int8_t)VBAT_ADC_CHANNEL, value);
    *value *= 4;
    // TODO: Delete test code.
    *value = *value / 3 * 2;
    return res;
}

rt_err_t vrefint_vol_read(rt_uint16_t *value)
{
    rt_err_t res = RT_EOK;
    res = adc_vol_read((rt_int8_t)RT_ADC_INTERN_CH_VREF, value);
    return res;
}

#ifdef RT_USING_MSH
static void test_read_voltage(int argc, char *argv[])
{
    rt_err_t res;
    rt_uint16_t cur_vol = 0, vcat_vol = 0, vbat_vol = 0, vrefint_vol = 0;
    res = cur_vol_read(&cur_vol);
    log_debug("cur_vol_read %s, cur_vol %dmv", res == RT_EOK ? "success" : "failed", cur_vol);
    res = vcat_vol_read(&vcat_vol);
    log_debug("vcat_vol_read %s, vcat_vol %dmv", res == RT_EOK ? "success" : "failed", vcat_vol);
    res = vbat_vol_read(&vbat_vol);
    log_debug("vbat_vol_read %s, vbat_vol %dmv", res == RT_EOK ? "success" : "failed", vbat_vol);
    // res = vrefint_vol_read(&vrefint_vol);
    // log_debug("vrefint_vol_read %s, vrefint_vol %d", res == RT_EOK ? "success" : "failed", vrefint_vol);
}

// MSH_CMD_EXPORT(test_read_voltage, TEST READ voltage);
#endif
