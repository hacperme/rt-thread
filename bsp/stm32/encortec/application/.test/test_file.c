/*
 * @FilePath: test_file.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-29 20:04:32
 * @copyright : Copyright (c) 2024
 */

#include <rtthread.h>
#include <stdio.h>
#include <dirent.h>
#include "logging.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "sys/unistd.h"

#define filename "test_file.txt"
#define res_msg(check) check ? "success" : "failed"

extern void list_files(const char *path);

void test_dir_option(void)
{
    char dir_buff[64];
    mkdir("/test_dir", 0);
    log_debug("mkdir test_dir");
    list_files("/");

    getcwd(dir_buff, 64);
    log_debug("getcwd %s", dir_buff);

    FILE *file;
    file = fopen("/test_dir/data.1", "wb");
    fclose(file);

    chdir("/test_dir");
    log_debug("chdir /test_dir");

    rt_memset(dir_buff, 0, 64);
    getcwd(dir_buff, 64);
    log_debug("getcwd %s", dir_buff);
    list_files("/test_dir");
    // rename("/test_dir/data.1", "/test_dir/data.2");
    // list_files("/test_dir");

    struct statfs dir_stat;
    int res;
    res = statfs("/", &dir_stat);
    log_debug("statfs / res=%d", res);
    if (res == 0)
    {
        log_debug(
            "dir_stat.f_bsize=%ld, dir_stat.f_blocks=%ld, dir_stat.f_bfree=%ld, dir_stat.f_bavail=%ld",
            dir_stat.f_bsize, dir_stat.f_blocks, dir_stat.f_bfree, dir_stat.f_bavail
        );
    }

    // rmdir("/test_dir");
    // log_debug("rmdir test_dir");
    chdir("/");
    list_files("/");
}

void test_fs_option(void)
{
    int res;

    list_files("/");

    test_dir_option();

    return;

    FILE *file = fopen(filename, "wb");
    log_debug("open file %s %s.", filename, res_msg(file != NULL));
    if (!file)
    {
        goto _close_file_;
    }

    char buff[64] = "Test File Option.";
    res = fwrite(buff, 1, 64, file);
    log_debug("write file %s.", res_msg(res == 64));
    if (res != 64)
    {
        goto _close_file_;
    }
    fclose(file);
    file = NULL;

    // res = fflush(file);
    // log_debug("fflush res=%d", res);
    file = fopen(filename, "rb");

    res = fseek(file, 0, SEEK_END);
    log_debug("fseek res=%d", res);

    size_t size = ftell(file);
    log_debug("file size=%d", size);

    res = fseek(file, 0, SEEK_SET);
    log_debug("fseek res=%d", res);

    char read_buff[64];
    rt_memset(buff, 0, 64);
    res = fread(read_buff, 1, 64, file);
    log_debug("fread res=%d, read_msg=%s", res, read_buff);

_close_file_:
    fclose(file);
    list_files("/");

    remove(filename);
    list_files("/");

    return;
}

typedef struct
{
    char a;
    char b[10];
    char c[10];
} test_st;

#define test_file "./test.cfg"

void read_struct(test_st *data)
{
    int ret;
    FILE *tf = fopen(test_file, "rb");

    ret = fread(data, 1, sizeof(*data), tf);
    log_debug("fread ret=%d", ret);

    ret = fclose(tf);
    log_debug("fclose ret=%d", ret);

}

void save_struct(test_st *data)
{
    int ret;
    FILE *tf = fopen(test_file, "ab");

    ret = fwrite(data, 1, sizeof(*data), tf);
    log_debug("fwrite ret=%d", ret);

    ret = fclose(tf);
    log_debug("fclose ret=%d", ret);
}

void test_struct_file_option(void)
{
    // test_st data = {
    //     .a = 1,
    //     .b = "testb",
    //     .c = "testc"
    // };

    // log_debug("111 data.a=%d", data.a);
    // log_debug("111 data.b=%s", data.b);
    // log_debug("111 data.c=%s", data.c);

    // save_struct(&data);

    // rt_memset(&data, 0, sizeof(data));
    // log_debug("222 data.a=%d", data.a);
    // log_debug("222 data.b=%s", data.b);
    // log_debug("222 data.c=%s", data.c);

    // read_struct(&data);

    // log_debug("333 data.a=%d", data.a);
    // log_debug("333 data.b=%s", data.b);
    // log_debug("333 data.c=%s", data.c);

    list_files("/");
    list_files("/fota");
}