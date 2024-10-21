/*
 * @FilePath: stm32u575_ota.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-15 15:09:22
 * @copyright : Copyright (c) 2024
 */

#include "stm32u575_ota.h"
#include "fal.h"
#include "logging.h"
#include <unistd.h>
#include <stdio.h>
#include "ota_app.h"

rt_err_t set_ota_option(ota_tag_e ota_tag, char *ota_file_name)
{
    rt_err_t res = RT_ERROR;

    mbr_t mbr = RT_NULL;
    mbr = mbr_init();
    if (mbr == RT_NULL)
    {
        goto _exit_;
    }

    mbr->ota_tag = ota_tag;
    mbr->ota_state = OTA_DEFAULT;
    rt_memset(mbr->ota_file, 0, sizeof(mbr->ota_file));
    rt_memcpy(mbr->ota_file, ota_file_name, rt_strlen(ota_file_name));

    res = mbr_save();

_exit_:
    return res;
}

rt_err_t clear_ota_option(void)
{
    rt_err_t res = RT_ERROR;

    mbr_t mbr = RT_NULL;
    mbr = mbr_init();
    if (mbr == RT_NULL)
    {
        goto _exit_;
    }

    mbr->ota_tag = OTA_NO;
    mbr->ota_state = OTA_DEFAULT;
    rt_memset(mbr->ota_file, 0, sizeof(mbr->ota_file));
    res = RT_EOK;

_exit_:
    return res;
}

void test_ota_app(void)
{
    rt_err_t res = set_ota_option(OTA_YES, "app_a.bin");
    if (res == RT_EOK)
    {
        rt_hw_cpu_reset();
    }
}

#if 0
#define flash_page_size 0x2000
static const struct fal_partition *app_part = RT_NULL;
static char flash_page_buffer[flash_page_size];

void save_app_bin(void)
{
    app_part = fal_partition_find("app_a");
    log_debug("fal_partition_find app_a %s", app_part == RT_NULL ? "failed" : "success");

    FILE *app_file = fopen("app_a.bin", "wb");

    int i, res;
    for (i = 0; i < 20; i++)
    {
        rt_memset(flash_page_buffer, 0xFF, flash_page_size);
        res = fal_partition_read(app_part, i * flash_page_size, flash_page_buffer, flash_page_size);
        log_debug("fal_partition_read app_a addr=%p, size=%p, res=%p", i * flash_page_size, flash_page_size, res);
        if (res > 0)
        {
            fwrite(flash_page_buffer, 1, flash_page_size, app_file);
            log_debug(
                "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                flash_page_buffer[0],
                flash_page_buffer[0 + 1],
                flash_page_buffer[0 + 2],
                flash_page_buffer[0 + 3],
                flash_page_buffer[0 + 4],
                flash_page_buffer[0 + 5],
                flash_page_buffer[0 + 6],
                flash_page_buffer[0 + 7],
                flash_page_buffer[0 + 8],
                flash_page_buffer[0 + 9]
            );
        }
    }
    fclose(app_file);
}

void test_show_app_bin(void)
{
    FILE *app_file = fopen("app_a.bin", "rb");
    int i, j, res;
    for (i = 0; i < 20; i++)
    {
        rt_memset(flash_page_buffer, 0xFF, flash_page_size);
        res = fread(flash_page_buffer, 1, flash_page_size, app_file);
        log_debug("fread res=%p", res);
        for (j = 0; j < flash_page_size; j += 10)
        {
            log_debug(
                "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                flash_page_buffer[j],
                flash_page_buffer[j + 1],
                flash_page_buffer[j + 2],
                flash_page_buffer[j + 3],
                flash_page_buffer[j + 4],
                flash_page_buffer[j + 5],
                flash_page_buffer[j + 6],
                flash_page_buffer[j + 7],
                flash_page_buffer[j + 8],
                flash_page_buffer[j + 9]
            );
            if (i != 19)
            {
                break;
            }
        }
    }
    fclose(app_file);
}
#endif
