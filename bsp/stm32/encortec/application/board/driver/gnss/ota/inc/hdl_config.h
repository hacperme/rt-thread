#ifndef __HDL_CONFIG_H__
#define __HDL_CONFIG_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// ToDo Porting: Please use your platform API to implement them.
// #include "syslog.h"
// #include "FreeRTOS.h"
// #include "task.h"
// #include "hal.h"
// #include "hal_gpio.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "logging.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HDL_DEBUG

#define HDL_VIA_UART

// ToDo Porting: Please use your platform API to implement them.
#ifdef HDL_VIA_UART
// UART0 is used for flash tool and logging tool, so choose UART1 for host download
#define HDL_UART_PORT                           HAL_UART_1
#endif

// DA Flash Position
#define HDL_DA_FLASH_POS                        0x00100000
#define HDL_DA_FILE                             "/fota/gnss_da.bin"
#define HDL_DA_SIZE                             (45056)
#define HDL_DA_RUN_ADDR                         (0x04200000)

// Test Images Flash Position in Host & Target Device
#define HDL_PARTITION_IMAGE_HOST_FLASH_POS      0x00200000
#define HDL_PARTITION_IMAGE_HOST_FILE           "/fota/gnss_partition.bin"
#define HDL_PARTITION_IMAGE_SLAVE_FLASH_POS     0x08000000
#define HDL_PARTITION_IMAGE_SIZE                4096
#define HDL_PARTITION_IMAGE_NAME                "PartitionTable"

#define HDL_BL_IMAGE_HOST_FLASH_POS             0x00201000
#define HDL_BL_IMAGE_HOST_FILE                  "/fota/gnss_bl.bin"
#define HDL_BL_IMAGE_SLAVE_FLASH_POS            0x08001000
#define HDL_BL_IMAGE_SIZE                       24576
#define HDL_BL_IMAGE_NAME                       "BootLoader"

#define HDL_GNSS_DEMO_IMAGE_HOST_FLASH_POS      0x00209000
#define HDL_GNSS_DEMO_IMAGE_HOST_FILE           "/fota/gnss_fw.bin"
#define HDL_GNSS_DEMO_IMAGE_SLAVE_FLASH_POS     0x08009000
#define HDL_GNSS_DEMO_IMAGE_SIZE                1040384
#define HDL_GNSS_DEMO_IMAGE_NAME                "MCU_FW"

#define HDL_GNSS_CONFIG_IMAGE_HOST_FLASH_POS    0x00380000
#define HDL_GNSS_CONFIG_IMAGE_HOST_FILE         "/fota/gnss_cfg.bin"
#define HDL_GNSS_CONFIG_IMAGE_SLAVE_FLASH_POS   0x081F7000
#define HDL_GNSS_CONFIG_IMAGE_SIZE              4096
#define HDL_GNSS_CONFIG_IMAGE_NAME              "GNSS_CFG"

#ifdef __cplusplus
}
#endif

#endif //__HDL_CONFIG_H__

