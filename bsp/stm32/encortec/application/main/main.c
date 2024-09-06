/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include "common.h"
#include <string.h>
#include "rtthread.h"
#include "logging.h"

int main(int argc, char *argv[]);
static void startup(app_startup_params_t *app_startup_params);

extern uint8_t __app_data_load_start[];
extern uint8_t __app_data_load_end[];
extern uint8_t _sdata[];
extern uint8_t _edata[];
extern uint8_t _sbss[];
extern uint8_t _ebss[];

static app_startup_params_t sg_app_startup_params = {0};

static const mbr_t mbr __attribute__((unused, section(".mbr"))) = {
    .app_magic_number = MBR_APP_MAGIC_NUMBER,
    .app_main_entry = main,
    .app_startup_entry = startup
};

static void copy_data(void) {
    uint32_t *src = (uint32_t *)__app_data_load_start;
    uint32_t *dst = (uint32_t *)_sdata;
    uint32_t *end = (uint32_t *)__app_data_load_end;

    while (src < end) {
        *dst++ = *src++;
    }
}

static void bss_fill_zero(void) {
    uint32_t *start = (uint32_t *)_sbss;
    uint32_t *end = (uint32_t *)_ebss;

    while (start < end) {
        *start++ = 0;
    }
}

static void startup(app_startup_params_t *app_startup_params) {
    copy_data();
    bss_fill_zero();
    if(app_startup_params) {
        rt_memcpy(&sg_app_startup_params, app_startup_params, sizeof(app_startup_params_t));
    }
}

uint32_t get_per_api_ptr(const char * func_name) {
    if(sg_app_startup_params.get_per_api_ptr) {
        return sg_app_startup_params.get_per_api_ptr(func_name);
    } else {
        return 0;
    }
}

void set_bootloader_startup_params(bootloader_startup_params_t *bootloader_startup_params) {
    if(sg_app_startup_params.set_bootloader_startup_params) {
        return sg_app_startup_params.set_bootloader_startup_params(bootloader_startup_params);
    }
}

bootloader_startup_params_t *get_bootloader_startup_params(void) {
    if(sg_app_startup_params.get_bootloader_startup_params) {
        return sg_app_startup_params.get_bootloader_startup_params();
    } else {
        return NULL;
    }
}

__attribute__((weak)) int application_start(int argc, char *argv[]) {
    app_log_init();
    while(1) {
        log_debug("Hello RT-Thread Application!");
        rt_thread_mdelay(500);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    return application_start(argc, argv);
}
