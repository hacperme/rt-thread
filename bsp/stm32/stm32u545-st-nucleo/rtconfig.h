#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

#define SOC_STM32U535RE
#define BOARD_STM32U535_NUCLEO

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_CPUS_NR 1
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_HOOK
#define RT_HOOK_USING_FUNC_PTR
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 1024
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 1024
#define RT_USING_CPU_USAGE_TRACER

/* kservice optimization */

/* end of kservice optimization */

/* klibc optimization */

/* end of klibc optimization */
#define RT_USING_DEBUG
#define RT_DEBUGING_ASSERT
#define RT_DEBUGING_CONTEXT
#define RT_USING_OVERFLOW_CHECK

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
/* end of Inter-Thread communication */

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_SMALL_MEM_AS_HEAP
#define RT_USING_HEAP
/* end of Memory Management */
#define RT_USING_DEVICE
#define RT_USING_DEVICE_OPS
#define RT_USING_SCHED_THREAD_CTX
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 512
#define RT_CONSOLE_DEVICE_NAME "lpuart1"
#define RT_VER_NUM 0x50200
#define RT_USING_STDC_ATOMIC
#define RT_BACKTRACE_LEVEL_MAX_NR 32
/* end of RT-Thread Kernel */
#define RT_USING_HW_ATOMIC
#define RT_USING_CPU_FFS
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M33

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 8192
#define RT_MAIN_THREAD_PRIORITY 10
#define RT_USING_MSH
#define RT_USING_FINSH
#define FINSH_USING_MSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 8192
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_CMD_SIZE 80
#define MSH_USING_BUILT_IN_COMMANDS
#define FINSH_USING_DESCRIPTION
#define FINSH_ARG_MAX 10
#define FINSH_USING_OPTION_COMPLETION

/* DFS: device virtual file system */

#define RT_USING_DFS
#define DFS_USING_POSIX
#define DFS_USING_WORKDIR
#define DFS_FD_MAX 16
#define RT_USING_DFS_V1
#define DFS_FILESYSTEMS_MAX 4
#define DFS_FILESYSTEM_TYPES_MAX 4
#define RT_USING_DFS_DEVFS
/* end of DFS: device virtual file system */
#define RT_USING_FAL
#define FAL_DEBUG_CONFIG
#define FAL_DEBUG 1
#define FAL_PART_HAS_TABLE_CFG

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_UNAMED_PIPE_NUMBER 64
#define RT_USING_SYSTEM_WORKQUEUE
#define RT_SYSTEM_WORKQUEUE_STACKSIZE 2048
#define RT_SYSTEM_WORKQUEUE_PRIORITY 23
#define RT_USING_SERIAL
#define RT_USING_SERIAL_V1
#define RT_SERIAL_RB_BUFSZ 1024
#define RT_USING_I2C
#define RT_I2C_DEBUG
#define RT_USING_I2C_BITOPS
#define RT_USING_ADC
#define RT_USING_NULL
#define RT_USING_ZERO
#define RT_USING_RANDOM
#define RT_USING_PWM
#define RT_USING_MTD_NOR
#define RT_USING_PM
#define PM_TICKLESS_THRESHOLD_TIME 2
#define PM_ENABLE_DEBUG
#define RT_USING_RTC
#define RT_USING_ALARM
#define RT_USING_SPI
#define RT_USING_SENSOR
#define RT_USING_SENSOR_V2
#define RT_USING_SENSOR_CMD
#define RT_USING_HWCRYPTO
#define RT_HWCRYPTO_DEFAULT_NAME "hwcryto"
#define RT_HWCRYPTO_IV_MAX_SIZE 16
#define RT_HWCRYPTO_KEYBIT_MAX_SIZE 256
#define RT_HWCRYPTO_USING_CRC
#define RT_HWCRYPTO_USING_CRC_07
#define RT_HWCRYPTO_USING_CRC_04C11DB7
#define RT_USING_PIN
#define RT_USING_KTIME
/* end of Device Drivers */

/* C/C++ and POSIX layer */

/* ISO-ANSI C layer */

/* Timezone and Daylight Saving Time */

#define RT_LIBC_USING_LIGHT_TZ_DST
#define RT_LIBC_TZ_DEFAULT_HOUR 8
#define RT_LIBC_TZ_DEFAULT_MIN 0
#define RT_LIBC_TZ_DEFAULT_SEC 0
/* end of Timezone and Daylight Saving Time */
/* end of ISO-ANSI C layer */

/* POSIX (Portable Operating System Interface) layer */

#define RT_USING_POSIX_FS
#define RT_USING_POSIX_DEVIO
#define RT_USING_POSIX_DELAY
#define RT_USING_POSIX_CLOCK
#define RT_USING_POSIX_TIMER

/* Interprocess Communication (IPC) */


/* Socket is in the 'Network' category */

/* end of Interprocess Communication (IPC) */
/* end of POSIX (Portable Operating System Interface) layer */
#define RT_USING_CPLUSPLUS
/* end of C/C++ and POSIX layer */

/* Network */

#define RT_USING_AT
#define AT_DEBUG
#define AT_USING_CLIENT
#define AT_CLIENT_NUM_MAX 2
#define AT_USING_CLI
#define AT_PRINT_RAW_CMD
#define AT_SW_VERSION_NUM 0x10301
/* end of Network */

/* Memory protection */

#define RT_USING_MEM_PROTECTION
#define RT_USING_HW_STACK_GUARD
#define USE_MEM_PROTECTION_EXAMPLES
#define NUM_MEM_REGIONS 8
#define NUM_EXCLUSIVE_REGIONS 2
#define NUM_CONFIGURABLE_REGIONS 3
/* end of Memory protection */

/* Utilities */

#define RT_USING_RESOURCE_ID
/* end of Utilities */

/* Using USB legacy version */

/* end of Using USB legacy version */
/* end of RT-Thread Components */

/* RT-Thread Utestcases */

/* end of RT-Thread Utestcases */

/* RT-Thread online packages */

/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */

/* end of Marvell WiFi */

/* Wiced WiFi */

/* end of Wiced WiFi */

/* CYW43012 WiFi */

/* end of CYW43012 WiFi */

/* BL808 WiFi */

/* end of BL808 WiFi */

/* CYW43439 WiFi */

/* end of CYW43439 WiFi */
/* end of Wi-Fi */

/* IoT Cloud */

/* end of IoT Cloud */
/* end of IoT - internet of things */

/* security packages */

/* end of security packages */

/* language packages */

/* JSON: JavaScript Object Notation, a lightweight data-interchange format */

#define PKG_USING_CJSON
#define PKG_USING_CJSON_V1717
/* end of JSON: JavaScript Object Notation, a lightweight data-interchange format */

/* XML: Extensible Markup Language */

/* end of XML: Extensible Markup Language */
/* end of language packages */

/* multimedia packages */

/* LVGL: powerful and easy-to-use embedded GUI library */

/* end of LVGL: powerful and easy-to-use embedded GUI library */

/* u8g2: a monochrome graphic library */

/* end of u8g2: a monochrome graphic library */
/* end of multimedia packages */

/* tools packages */

/* end of tools packages */

/* system packages */

/* enhanced kernel services */

#define PKG_USING_RT_VSNPRINTF_FULL
#define PKG_VSNPRINTF_SUPPORT_DECIMAL_SPECIFIERS
#define PKG_VSNPRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
#define PKG_VSNPRINTF_SUPPORT_WRITEBACK_SPECIFIER
#define PKG_VSNPRINTF_SUPPORT_LONG_LONG
#define PKG_VSNPRINTF_CHECK_FOR_NUL_IN_FORMAT_SPECIFIER
#define PKG_VSNPRINTF_INTEGER_BUFFER_SIZE 32
#define PKG_VSNPRINTF_DECIMAL_BUFFER_SIZE 32
#define PKG_VSNPRINTF_DEFAULT_FLOAT_PRECISION 6
#define PKG_VSNPRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL 9
#define PKG_VSNPRINTF_LOG10_TAYLOR_TERMS 4
#define PKG_USING_RT_VSNPRINTF_FULL_LATEST_VERSION
/* end of enhanced kernel services */

/* acceleration: Assembly language or algorithmic acceleration packages */

/* end of acceleration: Assembly language or algorithmic acceleration packages */

/* CMSIS: ARM Cortex-M Microcontroller Software Interface Standard */

/* end of CMSIS: ARM Cortex-M Microcontroller Software Interface Standard */

/* Micrium: Micrium software products porting for RT-Thread */

/* end of Micrium: Micrium software products porting for RT-Thread */
#define PKG_USING_PERF_COUNTER
#define PKG_USING_PERF_COUNTER_V2241
#define PKG_USING_LITTLEFS
#define PKG_USING_LITTLEFS_LATEST_VERSION
#define LFS_READ_SIZE 2048
#define LFS_PROG_SIZE 2048
#define LFS_BLOCK_SIZE 8192
#define LFS_CACHE_SIZE 2048
#define LFS_BLOCK_CYCLES 100
#define LFS_THREADSAFE
#define LFS_LOOKAHEAD_MAX 8
/* end of system packages */

/* peripheral libraries and drivers */

/* HAL & SDK Drivers */

/* STM32 HAL & SDK Drivers */

/* end of STM32 HAL & SDK Drivers */

/* Infineon HAL Packages */

/* end of Infineon HAL Packages */

/* Kendryte SDK */

/* end of Kendryte SDK */
/* end of HAL & SDK Drivers */

/* sensors drivers */

/* end of sensors drivers */

/* touch drivers */

/* end of touch drivers */
/* end of peripheral libraries and drivers */

/* AI packages */

/* end of AI packages */

/* Signal Processing and Control Algorithm Packages */

/* end of Signal Processing and Control Algorithm Packages */

/* miscellaneous packages */

/* project laboratory */

/* end of project laboratory */

/* samples: kernel and components samples */

/* end of samples: kernel and components samples */

/* entertainment: terminal games and other interesting software packages */

/* end of entertainment: terminal games and other interesting software packages */
#define PKG_USING_LWGPS
#define GPS_MODULE_BAUD_RATE 115200
#define LWGPS_CFG_DOUBLE 0
#define LWGPS_CFG_STATUS 0
#define LWGPS_CFG_STATEMENT_GPGGA 1
#define LWGPS_CFG_STATEMENT_GPGSA 1
#define LWGPS_CFG_STATEMENT_GPRMC 1
#define LWGPS_CFG_STATEMENT_GPGSV 1
#define LWGPS_CFG_STATEMENT_GPGSV_SAT_DET 0
#define LWGPS_CFG_STATEMENT_PUBX 0
#define LWGPS_CFG_STATEMENT_PUBX_TIME 0
#define PKG_USING_LWGPS_LATEST_VERSION
/* end of miscellaneous packages */

/* Arduino libraries */

#define PKG_USING_RTDUINO
#define RTDUINO_THREAD_SIZE 2048
#define RTDUINO_THREAD_PRIO 30
#define RTDUINO_SUPPORT_HIGH_PRECISION_MICROS
#define RTDUINO_USING_WIRE
#define RTDUINO_WIRE_BUFFER_LENGTH 32
#define RTDUINO_USING_SPI
#define PKG_USING_RTDUINO_LATEST_VERSION

/* Projects and Demos */

/* end of Projects and Demos */

/* Sensors */

#define PKG_USING_ARDUINO_ADAFRUIT_SENSOR
#define PKG_USING_ARDUINO_ADAFRUIT_SENSOR_LATEST_VERSION
/* end of Sensors */

/* Display */

/* end of Display */

/* Timing */

/* end of Timing */

/* Data Processing */

/* end of Data Processing */

/* Data Storage */

/* Communication */

/* end of Communication */

/* Device Control */

/* end of Device Control */

/* Other */

/* end of Other */

/* Signal IO */

#define PKG_USING_ARDUINO_ADAFRUIT_BUSIO
#define PKG_USING_ARDUINO_ADAFRUIT_BUSIO_LATEST_VERSION
/* end of Signal IO */

/* Uncategorized */

/* end of Arduino libraries */
/* end of RT-Thread online packages */
#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32U5
#define BOARD_SERIES_STM32_NUCLEO_64

/* Hardware Drivers Config */

/* Onboard Peripheral Drivers */

/* end of Onboard Peripheral Drivers */

/* On-chip Peripheral Drivers */

#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART1
#define BSP_USING_UART3
#define BSP_USING_UART4
#define BSP_USING_UART5
#define BSP_USING_LPUART1
#define BSP_USING_ADC
#define BSP_USING_ADC1
#define BSP_USING_SPI
#define BSP_USING_SPI1
#define BSP_USING_I2C
#define BSP_USING_I2C1

/* Notice: PB8 --> 24; PB9 --> 25 */

#define BSP_I2C1_SCL_PIN 24
#define BSP_I2C1_SDA_PIN 19
#define BSP_USING_ONCHIP_RTC
#define BSP_RTC_USING_LSE
#define BSP_USING_CRC
#define BSP_USING_ON_CHIP_FLASH
/* end of On-chip Peripheral Drivers */

/* Board extended module Drivers */

/* end of Hardware Drivers Config */

#endif
