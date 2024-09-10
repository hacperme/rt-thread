#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include "dfs_fs.h"
#include <ctype.h>
#include "data_save_as_file.h"

#define DBG_TAG "DATASAVE"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

// 检查文件名是否符合时间戳格式
static int is_valid_timestamp_filename(const char *filename) {
    if (rt_strlen(filename) != 16) return 0; // 长度不符合要求
    if (rt_strncmp(filename + 12, ".dat", 4) != 0) return 0; // 后缀不符合要求

    // 检查每个部分是否都是数字
    for (int i = 0; i < 12; i++) {
        if (!isdigit(filename[i])) return 0;
    }

    return 1;
}

// 将字符串转换为 long long 类型的时间值
static long long string_to_long_long(const char *timestamp_str) {
    long long time_value = 0;
    int year, month, day, hour, minute, second;

    // 读取年份
    year = (timestamp_str[0] - '0') * 10 + (timestamp_str[1] - '0');
    // 读取月份
    month = (timestamp_str[2] - '0') * 10 + (timestamp_str[3] - '0');
    // 读取日期
    day = (timestamp_str[4] - '0') * 10 + (timestamp_str[5] - '0');
    // 读取小时
    hour = (timestamp_str[6] - '0') * 10 + (timestamp_str[7] - '0');
    // 读取分钟
    minute = (timestamp_str[8] - '0') * 10 + (timestamp_str[9] - '0');
    // 读取秒
    second = (timestamp_str[10] - '0') * 10 + (timestamp_str[11] - '0');

    // 构建时间值
    time_value = year * 10000000000LL; // 年份
    time_value += month * 100000000LL; // 月份
    time_value += day * 1000000LL;     // 日
    time_value += hour * 10000LL;      // 小时
    time_value += minute * 100LL;       // 分钟
    time_value += second;              // 秒

    return time_value;
}

// 生成时间戳文件名
static void generate_timestamp_filename(char *filename) {
    time_t now = time(NULL);
    strftime(filename, FILE_NAME_MAX_LEN, TIMESTAMP_FORMAT, localtime(&now));
}

// 初始化文件系统
static void data_save_as_file_info_refresh(struct FileSystem *fs) {
	char tmp[FILE_NAME_MAX_LEN];
    DIR *dir;
    struct dirent *ent;
    char oldest_file_name[FILE_NAME_MAX_LEN] = {0};
    char latest_file_name[FILE_NAME_MAX_LEN] = {0};
    long long oldest_time = 0x0FFFFFFFFFFFFFFF;
    long long latest_time = 0;

    if ((dir = opendir(fs->base_dir)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // 检查文件名是否符合时间戳格式
            if (is_valid_timestamp_filename(ent->d_name)) {
                rt_memset(tmp, 0, FILE_NAME_MAX_LEN);
                rt_strncpy(tmp, ent->d_name, strlen(ent->d_name));
                long long current_time = string_to_long_long(tmp);

                if (current_time < oldest_time) {
                    rt_memset(oldest_file_name, 0, FILE_NAME_MAX_LEN);
                    rt_strncpy(oldest_file_name, fs->base_dir, strlen(fs->base_dir));
                    rt_strncpy(oldest_file_name + strlen(fs->base_dir), ent->d_name, rt_strlen(ent->d_name));
                    oldest_time = current_time;
                }

                if (current_time > latest_time) {
                    rt_memset(latest_file_name, 0, FILE_NAME_MAX_LEN);
                    rt_strncpy(latest_file_name, fs->base_dir, strlen(fs->base_dir));
                    rt_strncpy(latest_file_name + strlen(fs->base_dir), ent->d_name, rt_strlen(ent->d_name));
                    latest_time = current_time;
                }
            }
        }
        closedir(dir);
    }

    rt_memset(fs->oldest_file_name, 0, FILE_NAME_MAX_LEN);
    rt_memset(fs->latest_file_name, 0, FILE_NAME_MAX_LEN);
    rt_strncpy(fs->oldest_file_name, oldest_file_name, rt_strlen(oldest_file_name));
    rt_strncpy(fs->latest_file_name, latest_file_name, rt_strlen(latest_file_name));
}

void data_save_as_file_init(struct FileSystem *fs, int single_file_size_limit, char *base_dir) {
    if(single_file_size_limit) {
        fs->single_file_size_limit = single_file_size_limit;
    } else {
        fs->single_file_size_limit = SINGLE_FILE_SIZE_LIMIT_DFT;
    }
    if(base_dir) {
        rt_strncpy(fs->base_dir, base_dir, rt_strlen(base_dir));
    }
    data_save_as_file_info_refresh(fs);
}

char *get_oldest_file_name(struct FileSystem *fs) {
    data_save_as_file_info_refresh(fs);
    if(fs && fs->oldest_file_name[0] != '\0') {
        return fs->oldest_file_name;
    }

    return NULL;
}

char *get_latest_file_name(struct FileSystem *fs) {
    data_save_as_file_info_refresh(fs);
    if(fs && fs->latest_file_name[0] != '\0') {
        return fs->latest_file_name;
    }

    return NULL;
}

// 获取文件大小
size_t get_file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return 0;
    }

    // 移动文件指针到文件末尾
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fclose(file);

    return size;
}

// 检查文件系统剩余空间
int check_free_space(const char *path) {
    struct statfs stat = {0};
    statfs(path, &stat);
    return stat.f_bfree;
}

// 删除最早的文件
void delete_oldest_file(struct FileSystem *fs) {
    char *oldest_file_name = NULL;
    if((oldest_file_name = get_oldest_file_name(fs))) {
        LOG_D("xx Deleting oldest file: %s", oldest_file_name);
        remove(oldest_file_name);
        data_save_as_file_info_refresh(fs);
    }
}

int oldest_file_exists_over_30_days(struct FileSystem *fs)
{
    int year, month, day, hour, minute, second;
    char *oldest_filename = get_oldest_file_name(fs);
    if (!oldest_filename) {
        return 0;
    }

    int offset = strlen(fs->base_dir);
    
    // 读取年份
    year = (oldest_filename[offset + 0] - '0') * 10 + (oldest_filename[offset + 1] - '0');
    // 读取月份
    month = (oldest_filename[offset + 2] - '0') * 10 + (oldest_filename[offset + 3] - '0');
    // 读取日期
    day = (oldest_filename[offset + 4] - '0') * 10 + (oldest_filename[offset + 5] - '0');
    // 读取小时
    hour = (oldest_filename[offset + 6] - '0') * 10 + (oldest_filename[offset + 7] - '0');
    // 读取分钟
    minute = (oldest_filename[offset + 8] - '0') * 10 + (oldest_filename[offset + 9] - '0');
    // 读取秒
    second = (oldest_filename[offset + 10] - '0') * 10 + (oldest_filename[offset + 11] - '0');
    
    LOG_D("file create time:\r\n");
    LOG_D("year: %d; month: %d; day: %d; hour: %d; minute: %d; second: %d\r\n", year + 2000, month, day, hour, minute, second);

    struct tm file_create_time = {
        .tm_year = year + 100,
        .tm_mon = month - 1,
        .tm_mday = day,
        .tm_hour = hour,
        .tm_min = minute,
        .tm_sec = second,
        .tm_isdst = -1,
    };
    time_t file_create_timestamp = mktime(&file_create_time);
    LOG_D("file_create_time: %s, file_create_timestamp: %d\r\n", ctime(&file_create_timestamp), file_create_timestamp);

    time_t now = time(NULL);
    LOG_D("now: %s, stamp: %d", ctime(&now), now);

    int remaining = now - file_create_timestamp;
    LOG_D("%s exists %d seconds", oldest_filename, remaining);

    return remaining >= 30 * 86400 ? 1 : 0;
}

// 追加或创建文件
int data_save_as_file(struct FileSystem *fs, const char *buffer, size_t length, bool disable_single_file_size_limit) {
    // 获取最新文件的大小
    char *latest_file_name = NULL;
    size_t latest_file_size = 0;

    if((latest_file_name = get_latest_file_name(fs))) {
        latest_file_size = get_file_size(latest_file_name);
    }
    LOG_D("latest_file_name: %s", latest_file_name);

    // 检查是否有足够的空闲空间
    int free_blocks = check_free_space("/");
    if (free_blocks <= MIN_FREE_BLOCKS || oldest_file_exists_over_30_days(fs)) {
        delete_oldest_file(fs); // 删除最早的文件
    }

    // 判断是否需要新建文件
    if (latest_file_name == NULL || (!disable_single_file_size_limit && (latest_file_size + length > fs->single_file_size_limit))) {
        char temp_filename[FILE_NAME_MAX_LEN] = {0};
        generate_timestamp_filename(temp_filename);

        char filename[FILE_NAME_MAX_LEN] = {0};
        rt_strncpy(filename, fs->base_dir, strlen(fs->base_dir));
        rt_strncpy(filename + strlen(fs->base_dir), temp_filename, strlen(temp_filename));

        // 创建新文件
        LOG_D("++ Creating file: %s", filename);
        FILE *file = fopen(filename, "ab");
        if (!file) {
            return -1; // 打开文件失败
        }

        int retry = 0;
        for(retry = 0; retry < 5 && fwrite(buffer, 1, length, file) != length; retry++){
            LOG_E("?? Write file failed (%d)", retry);
        }

        fclose(file);

        data_save_as_file_info_refresh(fs);

        if(retry >= 5) {
            return -1;
        }
    } else {
        // 追加到现有文件
        LOG_D("-- Append to file %s", latest_file_name);
        FILE *file = fopen(latest_file_name, "ab");
        if (!file) {
            return -1; // 打开文件失败
        }

        int retry = 0;
        for(retry = 0; retry < 5 && fwrite(buffer, 1, length, file) != length; retry++){
            LOG_E("?? Write file failed, retry (%d)", retry);
        }

        fclose(file);

        if(retry >= 5) {
            return -1;
        }
    }

    return 0; // 成功
}

void list_files(const char *path) {
    DIR *dir;
    struct dirent *ent;
    char temp[FILE_NAME_MAX_LEN] = {0};
    rt_kprintf("dir \"%s\" files:\r\n", path);
    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            rt_memset(temp, 0, FILE_NAME_MAX_LEN);
            rt_strncpy(temp, path, strlen(path));
            rt_strncpy(temp + strlen(path), ent->d_name, strlen(ent->d_name));
            rt_kprintf("%s    %d Bytes\r\n", temp, get_file_size(temp));
        }
        closedir(dir);
    }
}

//-------------------- Test Code --------------------

char data_buffer[1024];

// void del_files(const char *path) {
static void del_files() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("/")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            LOG_I("Deleting %s", ent->d_name);
            remove(ent->d_name);
        }
        closedir(dir);
    }
}
// MSH_CMD_EXPORT(del_files, del files);

static void data_save_as_file_test() {
    struct FileSystem fs;

    data_save_as_file_init(&fs, 0, "/data/");
    
    rt_memset(data_buffer, '1', 1024);

    while (1)
    {
        LOG_I("<<<<<<<<<<<< Before written to file");
        list_files(fs.base_dir);
        if (data_save_as_file(&fs, data_buffer, 1024, false) == 0) {
            LOG_I("Data saved successfully.");
        } else {
            LOG_I("Data saved failed.");
        }
        LOG_I("----------------------------------------------------------------------");
        rt_thread_mdelay(500);
    }
}

MSH_CMD_EXPORT(data_save_as_file_test, data save as file test);
