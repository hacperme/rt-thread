/*
 * @FilePath: file.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-18 12:23:16
 * @copyright : Copyright (c) 2024
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "dfs_fs.h"
#include "dfs_file.h"
#include "fal.h"

#define DBG_SECTION_NAME "TESTFILE"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

extern void ls(const char *pathname);
extern void cat(const char *filename);

static int test_file(void)
{
    ls("/");
    cat("/123.txt");

    struct dfs_file fd;

    fd_init(&fd);

    if (dfs_file_open(&fd, "/123.txt", O_WRONLY | O_CREAT | O_TRUNC) < 0) {
        LOG_E("open failed");
        return RT_ERROR;
    }

    int length = dfs_file_write(&fd, "hello, world", strlen("hello, world"));
    if(length != strlen("hello, world")) {
        LOG_E("write failed");
        dfs_file_close(&fd);
        return RT_ERROR;
    }

    dfs_file_close(&fd);

    ls("/");
    cat("/123.txt");
}

MSH_CMD_EXPORT(test_file, test file);
