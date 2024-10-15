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

        extern void test_fs_option(void);
        test_fs_option();
    }

    // extern void test_hal_hmac_sha256(void);
    // test_hal_hmac_sha256();

    // extern void test_adxl372(int argc, char **argv);
    // test_adxl372(argc, argv);

    // extern rt_err_t test_fdc1004(int argc, char **argv);
    // test_fdc1004(argc, argv);

    extern void test_gnss(int argc, char **argv);
    test_gnss(argc, argv);

    // extern rt_err_t test_hdc3021(void);
    // test_hdc3021();

    // extern rt_err_t test_temp116(void);
    // test_temp116();

    // extern void test_read_voltage(int argc, char *argv[]);
    // test_read_voltage(argc, argv);

    // extern void test_rtc(void);
    // rt_thread_mdelay(5 * 1000);
    // test_rtc();

    return 0;
}
#endif