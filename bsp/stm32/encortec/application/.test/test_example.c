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

int application_start(int argc, char *argv[]) {
    app_log_init();

    // extern void test_adxl372(int argc, char **argv);
    // test_adxl372(argc, argv);

    // extern rt_err_t test_fdc1004(int argc, char **argv);
    // test_fdc1004(argc, argv);

    // extern void test_gnss(int argc, char **argv);
    // test_gnss(argc, argv);

    // extern rt_err_t test_hdc3021(void);
    // test_hdc3021();

    // extern rt_err_t test_temp116(void);
    // test_temp116();

    // extern void test_read_voltage(int argc, char *argv[]);
    // test_read_voltage(argc, argv);

    // extern void test_rtc(void);
    // rt_thread_mdelay(10);
    // test_rtc();

    return 0;
}
