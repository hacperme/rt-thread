#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#define STARTUP_FLAG            "========STARTUP_TO_APP=========\0"
#define APP_HEADER_MAGIC_NUMBER "========APP_MAGIC_NUMBER=======\0"

typedef struct {
    uint8_t startup_flag[32];
} bootloader_startup_params_t;

typedef uint32_t(*get_per_api_ptr_t)(const char * func_name);
typedef void(*set_bootloader_startup_params_t)(bootloader_startup_params_t *bootloader_startup_params);
typedef bootloader_startup_params_t *(*get_bootloader_startup_params_t)(void);

typedef struct {
    get_per_api_ptr_t get_per_api_ptr;
    set_bootloader_startup_params_t set_bootloader_startup_params;
    get_bootloader_startup_params_t get_bootloader_startup_params;
} app_startup_params_t;

typedef int(*app_main_entry_t)(int argc, char *argv[]);
typedef void(*app_startup_entry_t)(app_startup_params_t *app_startup_params);

typedef struct {
    uint8_t app_magic_number[32];
    app_main_entry_t app_main_entry;
    app_startup_entry_t app_startup_entry;
    uint8_t app_version[64];
    uint8_t app_subedition[8];
    uint8_t app_build_time[32];
} app_header_t;

extern uint32_t get_per_api_ptr(const char * func_name);
extern void set_bootloader_startup_params(bootloader_startup_params_t *bootloader_startup_params);
extern bootloader_startup_params_t *get_bootloader_startup_params(void);

#endif
