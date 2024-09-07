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
#include "stm32u5xx_hal.h"

#define ADC_NAME "adc1"
#define CUR_ADC_CHANNEL 1
#define VCAP_ADC_CHANNEL 2
#define VBAT_ADC_CHANNEL RT_ADC_INTERN_CH_VBAT
static rt_adc_device_t adc_dev = RT_NULL;

static void verfbuf_disable(void)
{
    HAL_SYSCFG_DisableVREFBUF();
    HAL_SYSCFG_VREFBUF_VoltageScalingConfig(SYSCFG_VREFBUF_VOLTAGE_SCALE0);
    HAL_SYSCFG_VREFBUF_HighImpedanceConfig(SYSCFG_VREFBUF_HIGH_IMPEDANCE_ENABLE);
    __HAL_RCC_VREF_CLK_DISABLE();
}

static rt_err_t verfbuf_enable(void)
{
    rt_err_t res;
    rt_tick_t stime, etime;
    __HAL_RCC_VREF_CLK_ENABLE();
    stime = rt_tick_get_millisecond();
    HAL_SYSCFG_VREFBUF_VoltageScalingConfig(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
    HAL_SYSCFG_VREFBUF_HighImpedanceConfig(SYSCFG_VREFBUF_HIGH_IMPEDANCE_DISABLE);
    rt_uint16_t cnt = 500;
    HAL_StatusTypeDef ret;
    do {
        ret = HAL_SYSCFG_EnableVREFBUF();
        cnt--;
    } while (ret != HAL_OK && cnt > 0);
    etime = rt_tick_get_millisecond();
    
    res = ret == HAL_OK ? RT_EOK : RT_ERROR;
    log_debug("HAL_SYSCFG_EnableVREFBUF %s", (res != RT_EOK ? "falied" : "success"));
    if (res != RT_EOK)
    {
        verfbuf_disable();
    }
    return res;
}

static rt_err_t adc_read_channel_vol(rt_int8_t channel, rt_uint32_t *value)
{
    rt_err_t res;

    if (adc_dev == RT_NULL)
    {
        adc_dev = (rt_adc_device_t)rt_device_find(ADC_NAME);
        log_debug("find %s device %s.", ADC_NAME, (adc_dev == RT_NULL ? "falied" : "success"));
        res = !adc_dev ? RT_ERROR : RT_EOK;
        if (res != RT_EOK)
        {
            return res;
        }
    }

    res = rt_adc_enable(adc_dev, channel);
    log_debug("rt_adc_enable %s %s.", ADC_NAME, (res != RT_EOK ? "falied" : "success"));
    if (res != RT_EOK)
    {
        return res;
    }

    *value = rt_adc_read(adc_dev, channel);

    res = rt_adc_disable(adc_dev, channel);
    log_debug("rt_adc_disable %s %s.", ADC_NAME, (res != RT_EOK ? "falied" : "success"));

    return res;
}

static rt_err_t adc_read_vol(rt_int8_t channel, float *value)
{
    rt_err_t res;
    rt_adc_device_t adc_dev;

    rt_uint16_t VREFINT_CAL = *(__IO uint16_t *)(0x0BFA07A5);
    rt_uint32_t c_val, vrefint_val;
    res = adc_read_channel_vol(channel, &c_val);
    if (res != RT_EOK)
    {
        return res;
    }

    res = adc_read_channel_vol((rt_uint8_t)RT_ADC_INTERN_CH_VREF, &vrefint_val);
    if (res != RT_EOK)
    {
        return res;
    }

    log_debug("VREFINT_CAL=%d Channel=%d Val=%d VREFINT_VAL=%d", VREFINT_CAL, channel, c_val, vrefint_val);
    *value = 3.0 * VREFINT_CAL * c_val / (vrefint_val * ((1 << 14) - 1));

    return res;
}

// static rt_err_t adc_vol_read(rt_int8_t channel, rt_uint16_t *value)
// {
//     rt_err_t res;
//     rt_adc_device_t adc_dev;
//     rt_uint16_t vol = 0;

//     adc_dev = (rt_adc_device_t)rt_device_find(ADC_NAME);
//     log_error("find %s device %s.", ADC_NAME, (adc_dev == RT_NULL ? "falied" : "success"));
//     res = !adc_dev ? RT_ERROR : RT_EOK;
//     if (res != RT_EOK)
//     {
//         return res;
//     }

//     /* vref 3305 */
//     vol = rt_adc_voltage(adc_dev, channel);
//     log_debug("rt_adc_voltage channel %d value %d", channel, vol);
//     res = vol > 0 ? RT_EOK : RT_ERROR;
//     if (res == RT_EOK)
//     {
//         *value = vol;
//     }

//     return res;
// }

rt_err_t cur_vol_read(rt_uint16_t *value)
{
    rt_err_t res = RT_EOK;
    // res = adc_vol_read((rt_int8_t)CUR_ADC_CHANNEL, value);

    float vol;
    res = adc_read_vol(CUR_ADC_CHANNEL, &vol);
    if (res == RT_EOK)
    {
        *value = (rt_uint16_t)(vol * 1000);
    }

    return res;
}

rt_err_t vcap_vol_read(rt_uint16_t *value)
{
    rt_err_t res = RT_EOK;
    // res = adc_vol_read((rt_int8_t)VCAP_ADC_CHANNEL, value);

    float vol;
    res = adc_read_vol(VCAP_ADC_CHANNEL, &vol);
    if (res == RT_EOK)
    {
        *value = (rt_uint16_t)(vol * 1000);
    }

    return res;
}

rt_err_t vbat_vol_read(rt_uint16_t *value)
{
    rt_err_t res = RT_EOK;
    // res = adc_vol_read((rt_int8_t)VBAT_ADC_CHANNEL, value);
    // *value *= 4;
    float vol;
    res = adc_read_vol(VBAT_ADC_CHANNEL, &vol);
    if (res == RT_EOK)
    {
        *value = (rt_uint16_t)(vol * 4 * 1000);
    }

    return res;
}

#ifdef RT_USING_MSH
static void test_read_voltage(int argc, char *argv[])
{
    rt_err_t res;
    rt_uint16_t cur_vol = 0, vcap_vol = 0, vbat_vol = 0, vrefint_vol = 0;
    res = cur_vol_read(&cur_vol);
    log_debug("cur_vol_read %s, cur_vol %dmv", res == RT_EOK ? "success" : "failed", cur_vol);
    res = vcap_vol_read(&vcap_vol);
    log_debug("vcap_vol_read %s, vcap_vol %dmv", res == RT_EOK ? "success" : "failed", vcap_vol);
    res = vbat_vol_read(&vbat_vol);
    log_debug("vbat_vol_read %s, vbat_vol %dmv", res == RT_EOK ? "success" : "failed", vbat_vol);
}

// MSH_CMD_EXPORT(test_read_voltage, TEST READ voltage);

static void test_adc_read(int argc, char *argv[])
{
    rt_err_t res;
    float vol;

    res = adc_read_vol(CUR_ADC_CHANNEL, &vol);
    log_debug("ADC READ CUR_ADC_CHANNEL=%d vol=%f", res, vol);

    res = adc_read_vol(VCAP_ADC_CHANNEL, &vol);
    log_debug("ADC READ VCAP_ADC_CHANNEL=%d vol=%f", res, vol);

    res = adc_read_vol(VBAT_ADC_CHANNEL, &vol);
    vol *= 4;
    log_debug("ADC READ VBAT_ADC_CHANNEL=%d vol=%f", res, vol);
}

// MSH_CMD_EXPORT(test_adc_read, test adc read);

#endif
