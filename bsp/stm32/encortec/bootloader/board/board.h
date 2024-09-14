/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-5      SummerGift   first version
 * 2019-04-24     yangjie      Use the end of ZI as HEAP_BEGIN
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtthread.h>
#include <stm32u5xx.h>
#include "drv_common.h"
#include "drv_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STM32_FLASH_START_ADRESS       ((uint32_t)0x08000000)
#define STM32_FLASH_SIZE               (512 * 1024)
#define STM32_FLASH_END_ADDRESS        ((uint32_t)(STM32_FLASH_START_ADRESS + STM32_FLASH_SIZE))

#define STM32_SRAM1_SIZE               (192)
#define STM32_SRAM1_START              (0x20000000)
#define STM32_SRAM1_END                (STM32_SRAM1_START + STM32_SRAM1_SIZE * 1024)

#define STM32_FLASH_MPU_SIZE           ((uint32_t)256 * 1024)

#if defined(__ARMCC_VERSION)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="CSTACK"
#define HEAP_BEGIN      (__segment_end("CSTACK"))
#else
extern char __heap_start[];
#define HEAP_BEGIN      ((void *)&__heap_start)
#endif

extern char __heap_end[];
#define HEAP_END         ((void *)&__heap_end)

#ifdef RT_USING_MEM_PROTECTION
#define NUM_STATIC_REGIONS 1
#endif

void SystemClock_Config(void);
void MX_ICACHE_Init(void);

#define USART1_TX_PIN           GET_PIN(A, 9)
#define USART1_RX_PIN           GET_PIN(A, 10)
#define USART3_TX_PIN           GET_PIN(C, 4)
#define USART3_RX_PIN           GET_PIN(C, 5)
#define UART4_TX_PIN            GET_PIN(A, 0)
#define UART4_RX_PIN            GET_PIN(A, 1)
#define UART5_TX_PIN            GET_PIN(C, 12)
#define UART5_RX_PIN            GET_PIN(D, 2)
#define LPUART1_TX_PIN          GET_PIN(A, 2)
#define LPUART1_RX_PIN          GET_PIN(A, 3)
#define ADC1_IN1_PIN            GET_PIN(C, 0)
#define ADC1_IN2_PIN            GET_PIN(C, 1)
#define I2C1_SCL_PIN            GET_PIN(B, 8)
#define I2C1_SDA_PIN            GET_PIN(B, 3)
#define SPI1_SCK_PIN            GET_PIN(A, 5)
#define SPI1_MISO_PIN           GET_PIN(A, 6)
#define SPI1_MOSI_PIN           GET_PIN(A, 7)

#ifdef SOC_STM32U545RE
#define PWRCTRL_PWR_WKUP3       GET_PIN(B, 7)
#else
#define PWRCTRL_PWR_WKUP3       GET_PIN(E, 6)
#endif
#define NB_CAT1_RF_PIN          GET_PIN(E, 3)
#define INTN_EXT_ANT_PIN        GET_PIN(D, 7)
#define ANTENNA_ACTIVE_PIN      GET_PIN(E, 4)

#define SENSOR_PWRON_PIN        GET_PIN(D, 8)
#define SENSOR_PWR_LED_PIN      GET_PIN(B, 4)

#define CAT1_PWRON_PIN          GET_PIN(A, 8)
#define CAT1_BOOT_PIN           GET_PIN(C, 6)
#define CAT1_STATUS_PIN         GET_PIN(D, 4)
#define CAT1_RESET_STM_PIN      GET_PIN(C, 8)
#define CAT1_PWRKEY_STM_PIN     GET_PIN(D, 1)
#define CAT1_PSM_IND_STM_PIN    GET_PIN(C, 11)
#define CAT1_PSM_INT_STM_PIN    GET_PIN(C, 10)

#define ESP32_PWRON_PIN         GET_PIN(H, 1)
#define ESP32_EN_PIN            GET_PIN(E, 5)
#define ESP32_DOWNLOAD_PIN      GET_PIN(D, 6)

#define NBIOT_PWRON_PIN         GET_PIN(E, 2)
#define NBIOT_BOOT_PIN          GET_PIN(B, 9)
#define NBIOT_RESET_PIN         GET_PIN(D, 13)
#define NBIOT_PSMEINT_PIN       GET_PIN(A, 15)

#ifdef SOC_STM32U545RE
#define ADXL372_CS_PIN          GET_PIN(C, 9)
#define ADXL372_INT1_PIN        GET_PIN(B, 7)
#else
#define ADXL372_CS_PIN          GET_PIN(E, 9)
#define ADXL372_INT1_PIN        GET_PIN(E, 8)
#endif

#define GNSS_PWRON_PIN          GET_PIN(E, 0)
#define GNSS_RST_PIN            GET_PIN(E, 1)
#define EG915_GNSSEN_PIN        GET_PIN(B, 5)

#define FLASH_PWRON_PIN         GET_PIN(D, 14)
#define QSPI_CSN_PIN            GET_PIN(A, 4)
#define QSPI_D1_PIN             GET_PIN(B, 0)
#define QSPI_D0_PIN             GET_PIN(B, 1)
#define QSPI_D2_PIN             GET_PIN(E, 14)
#define QSPI_D3_PIN             GET_PIN(E, 15)
#define QSPI_CLK_PIN            GET_PIN(B, 10)
#define QSPI_CPUN_ESP_PIN       GET_PIN(D, 5)

#define SIM_SELECT_PIN          GET_PIN(C, 9)
#define SIM_DET_2_PIN           GET_PIN(D, 15)
#define SIM_DET_1_PIN           GET_PIN(C, 7)
#define SIM_DET_VCC_PIN         GET_PIN(D, 0)

#ifdef __cplusplus
}
#endif

#endif
