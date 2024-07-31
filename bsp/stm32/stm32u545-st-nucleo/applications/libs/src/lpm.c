/*
 * @FilePath: lpm.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-31 13:45:56
 * @copyright : Copyright (c) 2024
 */
#include "lpm.h"

#define DBG_SECTION_NAME "LPM"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define SEN_PWR_WKUP7  GET_PIN(E, 8)
// #define BUTTON_PIN  GET_PIN(C, 13)


void shut_down(void)
{
    /* Sensor wakeup pin irq enable. */
    rt_pin_mode(SEN_PWR_WKUP7, PIN_MODE_INPUT);
    rt_pin_irq_enable(SEN_PWR_WKUP7, PIN_IRQ_ENABLE);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_2);

    // // /* Enable WakeUp Pin PWR_WAKEUP_PIN2 connected to PC.13 */
    // rt_pin_mode(BUTTON_PIN, PIN_MODE_INPUT);
    // rt_pin_irq_enable(BUTTON_PIN, PIN_IRQ_ENABLE);
    // HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_HIGH_1);

    LOG_D("Start Shut Down.");

    /* Clear all related wakeup flags*/
    __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG2);

    /* Enter the Shutdown mode */
    HAL_PWREx_EnterSHUTDOWNMode();
}
