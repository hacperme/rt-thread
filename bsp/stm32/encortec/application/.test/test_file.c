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

    // rmdir("/test_dir");
    // log_debug("rmdir test_dir");
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