/*
 * @FilePath: test_example.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-10 17:30:16
 * @copyright : Copyright (c) 2024
 */
#include "rtthread.h"
#include "tools.h"
#include "drv_fatfs_dhara_nand.h"
#include "drv_nand_flash.h"
#include "lpm.h"
#include "logging.h"

#if 1

static struct rt_semaphore mnt_sem;

void fatfs_dhara_nand_mnt_cb(fdnfs_init_status_e *status)
{
    log_debug("fatfs_dhara_nand_mnt_cb status=%d", *status);
    if (*status == 0)
    {
        rt_sem_release(&mnt_sem);
    }
}

static fdnfs_init_status_e mnt_status;

void test_show_app_version(void)
{
    extern void read_app_version_information(uint8_t **app_version, uint8_t **app_subedition, uint8_t **app_build_time);
    uint8_t *app_version, *app_subedition, *app_build_time;
    read_app_version_information(&app_version, &app_subedition, &app_build_time);
    log_debug("app_version=%s, app_subedition=%s, app_build_time=%s", app_version, app_subedition, app_build_time);
}

int application_start(int argc, char *argv[]) {

    app_log_init();

    rt_err_t res;
    res = rt_sem_init(&mnt_sem, "mntsem", 0, RT_IPC_FLAG_PRIO);
    log_debug("rt_sem_init mntsem res=%d\n", res);
    if (res != RT_EOK)
    {
        return -1;
    }

    fatfs_dhara_nand_init(fatfs_dhara_nand_mnt_cb, &mnt_status);

    res = rt_sem_take(&mnt_sem, RT_WAITING_FOREVER);
    log_debug("fatfs mount %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        // extern void data_save_as_file_test();
        // data_save_as_file_test();

        // extern void test_fs_option(void);
        // test_fs_option();
        rt_kprintf("fatfs mount success.\n");
    }

    // extern void test_show_reset_status(void);
    // test_show_reset_status();

    // extern void test_show_wkup_status(void);
    // test_show_wkup_status();

    // extern void test_rtc(void);
    // rt_thread_mdelay(5 * 1000);
    // test_rtc();

    // extern int delete_directory(const char *dir);
    // delete_directory("/data");
    // delete_directory("/log/");

    // test_show_app_version();

    // esp32_power_pin_init();

    // esp32_power_off();
    // rt_thread_mdelay(1000);
    // nand_to_esp32();
    // esp32_power_on();

    // esp32_start_download();

    // extern void test_cat1_at_ota(void);
    // test_cat1_at_ota();

    // extern void test_struct_file_option(void);
    // test_struct_file_option();

    // extern void test_gnss(int argc, char **argv);
    // test_gnss(argc, argv);

    // extern void test_upgrade_process(void);
    // test_upgrade_process();

    // extern void test_st_at(void);
    // test_st_at();

    // extern void test_hal_hmac_sha256(void);
    // test_hal_hmac_sha256();

    // extern void test_adxl372(int argc, char **argv);
    // test_adxl372(argc, argv);

    // extern rt_err_t test_fdc1004(int argc, char **argv);
    // test_fdc1004(argc, argv);

    // extern void test_drv_hash(void);
    // test_drv_hash();

    // extern void save_app_bin(void);
    // save_app_bin();

    // extern void test_show_app_bin(void);
    // test_show_app_bin();

    // extern void test_ota_app(void);
    // test_ota_app();

    // extern rt_err_t test_watch_dog(void);
    // test_watch_dog();

    // extern rt_err_t test_hdc3021(void);
    // test_hdc3021();

    // extern rt_err_t test_temp116(void);
    // test_temp116();

    // extern void test_crc32_check(void);
    // test_crc32_check();

    // extern void test_read_voltage(int argc, char *argv[]);
    // test_read_voltage(argc, argv);

    // extern void data_save_as_file_test();
    // data_save_as_file_test();

    // extern void test_debug_led1_flash(void);
    // test_debug_led1_flash();

    // extern void test_antenna_auto_switch(void);
    // test_antenna_auto_switch();
    extern void main_business_entry(void);
    main_business_entry();

    // extern void esp_data_stransf_example(void);
    // esp_data_stransf_example();

    // for test
    // extern void esp32_power_pin_init(void);
    // extern rt_err_t esp32_start_download(void);
    // esp32_power_pin_init();
    // esp32_start_download();

    // extern rt_err_t esp32_wifi_transfer();
    // esp32_wifi_transfer();
}
#endif