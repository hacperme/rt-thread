/*
 * @FilePath: test_example.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-10 17:30:16
 * @copyright : Copyright (c) 2024
 */
#include "logging.h"
#include "rtthread.h"
#include "drv_fatfs_dhara_nand.h"

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

int application_start(int argc, char *argv[]) {
    app_log_init();

    extern void test_show_reset_status(void);
    test_show_reset_status();

    rt_err_t res;
    res = rt_sem_init(&mnt_sem, "mntsem", 0, RT_IPC_FLAG_PRIO);
    log_debug("rt_sem_init mntsem res=%d", res);
    if (res != RT_EOK)
    {
        return -1;
    }

    fatfs_dhara_nand_init(fatfs_dhara_nand_mnt_cb, &mnt_status);

    res = rt_sem_take(&mnt_sem, RT_WAITING_FOREVER);

    if (res == RT_EOK)
    {
        // extern void data_save_as_file_test();
        // data_save_as_file_test();

        // extern void test_fs_option(void);
        // test_fs_option();
        log_debug("fatfs mount success.");
    }

    // extern void test_hal_hmac_sha256(void);
    // test_hal_hmac_sha256();

    // extern void test_adxl372(int argc, char **argv);
    // test_adxl372(argc, argv);

    extern void test_vol_read(void);
    test_vol_read();

    // extern rt_err_t test_fdc1004(int argc, char **argv);
    // test_fdc1004(argc, argv);

    // extern void test_gnss(int argc, char **argv);
    // test_gnss(argc, argv);

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

    // extern void test_rtc(void);
    // rt_thread_mdelay(5 * 1000);
    // test_rtc();

    // extern void data_save_as_file_test();
    // data_save_as_file_test();

    // extern int delete_directory(const char *dir);
    // delete_directory("/data");

    // extern void main_business_entry(void);
    // main_business_entry();

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