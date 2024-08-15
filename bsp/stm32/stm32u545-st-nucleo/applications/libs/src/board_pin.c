/*
 * @FilePath: board_pin.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-07 09:21:32
 * @copyright : Copyright (c) 2024
 */
#include "board_pin.h"

#define DBG_SECTION_NAME "BOARD_PIN"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define GNSS_PWRON_PIN          GET_PIN(E, 0)
#define GNSS_RST_PIN            GET_PIN(E, 1)
#define EG915_GNSSEN_PIN        GET_PIN(B, 5)
#define SENSOR_PWRON_PIN        GET_PIN(D, 8)
// #ifdef SOC_STM32U545RE
// #define PWRCTRL_PWR_WKUP3       GET_PIN(B, 7)
// #else
#define PWRCTRL_PWR_WKUP3       GET_PIN(E, 6)
// #endif
#define NBIOT_PWRON_PIN         GET_PIN(E, 2)
#define ESP32_PWRON_PIN         GET_PIN(H, 1)
#define ESP32_EN_PIN            GET_PIN(E, 5)
#define CAT1_PWRON_PIN          GET_PIN(A, 8)
#define SIM_SELECT_PIN          GET_PIN(C, 9)
#define NB_CAT1_RF_PIN          GET_PIN(E, 3)
#define INTN_EXT_ANT_PIN        GET_PIN(D, 7)
#define ANTENNA_ACTIVE_PIN      GET_PIN(E, 4)
#define FLASH_PWRON_PIN         GET_PIN(D, 14)


void gnss_pwron_pin_init(void)
{
    rt_pin_mode(GNSS_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t gnss_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(GNSS_PWRON_PIN, mode);
    return rt_pin_read(GNSS_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

void gnss_rst_pin_init(void)
{
    rt_pin_mode(GNSS_RST_PIN, PIN_MODE_OUTPUT);
}

rt_err_t gnss_rst_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(GNSS_RST_PIN, mode);
    return rt_pin_read(GNSS_RST_PIN) == mode ? RT_EOK : RT_ERROR;
}

void eg915_gnssen_pin_init(void)
{
    rt_pin_mode(EG915_GNSSEN_PIN, PIN_MODE_OUTPUT);
}

rt_err_t eg915_gnssen_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(EG915_GNSSEN_PIN, mode);
    return rt_pin_read(EG915_GNSSEN_PIN) == mode ? RT_EOK : RT_ERROR;
}

void sensor_pwron_pin_init(void)
{
    rt_pin_mode(SENSOR_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t sensor_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(SENSOR_PWRON_PIN, mode);
    return rt_pin_read(SENSOR_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

void pwrctrl_pwr_wkup3_init(void)
{
    rt_pin_mode(PWRCTRL_PWR_WKUP3, PIN_MODE_INPUT_PULLDOWN);
}

void pwrctrl_pwr_wkup3_irq(void *args)
{
    LOG_D("pwrctrl_pwr_wkup3_irq");
}

rt_err_t pwrctrl_pwr_wkup3_irq_enable(void)
{
    rt_err_t res;
    res = rt_pin_attach_irq(PWRCTRL_PWR_WKUP3, PIN_IRQ_MODE_RISING_FALLING, pwrctrl_pwr_wkup3_irq, RT_NULL);
    LOG_D("rt_pin_attach_irq PWRCTRL_PWR_WKUP3 PIN_IRQ_MODE_RISING_FALLING res=%d");
    if (res != RT_EOK)
    {
        return res;
    }
    res = rt_pin_irq_enable(PWRCTRL_PWR_WKUP3, PIN_IRQ_ENABLE);
    LOG_D("rt_pin_irq_enable PWRCTRL_PWR_WKUP3 PIN_IRQ_ENABLE res=%d", res);
    /* Power harvster/tracker monitor wakeup pin irq enable. */
    if (res != RT_EOK)
    {
        return res;
    }
// #ifdef SOC_STM32U545RE
//     HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4_HIGH_2);
// #else
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN3_HIGH_0);
// #endif
    return res;
}

void nbiot_pwron_pin_init(void)
{
    rt_pin_mode(NBIOT_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t nbiot_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(NBIOT_PWRON_PIN, mode);
    return rt_pin_read(NBIOT_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

void esp32_pwron_pin_init(void)
{
    rt_pin_mode(ESP32_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t esp32_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(ESP32_PWRON_PIN, mode);
    return rt_pin_read(ESP32_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

void esp32_en_pin_init(void)
{
    rt_pin_mode(ESP32_EN_PIN, PIN_MODE_OUTPUT);
}

rt_err_t esp32_en_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(ESP32_EN_PIN, mode);
    return rt_pin_read(ESP32_EN_PIN) == mode ? RT_EOK : RT_ERROR;
}

void cat1_pwron_pin_init(void)
{
    rt_pin_mode(CAT1_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t cat1_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(CAT1_PWRON_PIN, mode);
    return rt_pin_read(CAT1_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

void sim_select_pin_init(void)
{
    rt_pin_mode(SIM_SELECT_PIN, PIN_MODE_OUTPUT);
}

rt_err_t sim_select_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(SIM_SELECT_PIN, mode);
    return rt_pin_read(SIM_SELECT_PIN) == mode ? RT_EOK : RT_ERROR;
}

void nb_cat1_rf_pin_init(void)
{
    rt_pin_mode(NB_CAT1_RF_PIN, PIN_MODE_OUTPUT);
}

rt_err_t nb_cat1_rf_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(NB_CAT1_RF_PIN, mode);
    return rt_pin_read(NB_CAT1_RF_PIN) == mode ? RT_EOK : RT_ERROR;
}

void intn_ext_ant_pin_init(void)
{
    rt_pin_mode(INTN_EXT_ANT_PIN, PIN_MODE_OUTPUT);
}

rt_err_t intn_ext_ant_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(INTN_EXT_ANT_PIN, mode);
    return rt_pin_read(INTN_EXT_ANT_PIN) == mode ? RT_EOK : RT_ERROR;
}

void antenna_active_pin_init(void)
{
    rt_pin_mode(ANTENNA_ACTIVE_PIN, PIN_MODE_OUTPUT);
}

rt_err_t antenna_active_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(ANTENNA_ACTIVE_PIN, mode);
    return rt_pin_read(ANTENNA_ACTIVE_PIN) == mode ? RT_EOK : RT_ERROR;
}

void flash_pwron_pin_init(void)
{
    rt_pin_mode(FLASH_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t flash_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(FLASH_PWRON_PIN, mode);
    return rt_pin_read(FLASH_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

int board_pins_init(void)
{
    rt_err_t res = RT_EOK;

    gnss_pwron_pin_init();
    gnss_rst_pin_init();
    eg915_gnssen_pin_init();
    sensor_pwron_pin_init();
    pwrctrl_pwr_wkup3_init();
    nbiot_pwron_pin_init();
    esp32_pwron_pin_init();
    esp32_en_pin_init();
    cat1_pwron_pin_init();
    sim_select_pin_init();
    nb_cat1_rf_pin_init();
    intn_ext_ant_pin_init();
    antenna_active_pin_init();
    flash_pwron_pin_init();

    return res;
}

INIT_BOARD_EXPORT(board_pins_init);
