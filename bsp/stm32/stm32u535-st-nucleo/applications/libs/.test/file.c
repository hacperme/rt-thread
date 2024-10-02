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
#include "sys/stat.h"
#include "sys/unistd.h"

#define DBG_SECTION_NAME "TESTFILE"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

extern void ls(const char *pathname);
extern void cat(const char *filename);
// extern void test_nand_fs_init(void);


int test_file(void)
{
    // test_nand_fs_init();
    // rt_thread_mdelay(1000);

    ls("/");
    cat("/123.txt");
    int res;

    struct stat file_stat;
    res = stat("/123.txt", &file_stat);
    LOG_D("stat /123.txt res=%d", res);
    LOG_D(
        "file_stat.st_size=%ld, file_stat.st_blksize=%ld, file_stat.st_blocks=%ld, " \
        "file_stat.st_atime=%ld, file_stat.st_mtime=%ld, file_stat.st_ctime=%ld",
        file_stat.st_size, file_stat.st_blksize, file_stat.st_blocks,
        file_stat.st_atime, file_stat.st_mtime, file_stat.st_ctime
    );

    struct statfs dir_stat;
    res = statfs("/", &dir_stat);
    LOG_D("statfs / res=%d", res);
    if (res == 0)
    {
        LOG_D(
            "dir_stat.f_bsize=0x%09X, dir_stat.f_blocks=0x%09X, dir_stat.f_bfree=0x%09X, dir_stat.f_bavail=0x%09X",
            dir_stat.f_bsize, dir_stat.f_blocks, dir_stat.f_bfree, dir_stat.f_bavail
        );
    }

    return 0;

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

    return 0;
}

MSH_CMD_EXPORT(test_file, test file);
