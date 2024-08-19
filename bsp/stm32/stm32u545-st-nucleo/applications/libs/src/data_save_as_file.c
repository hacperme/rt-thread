#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include "dfs_fs.h"
#include <ctype.h>

#define DBG_TAG "DATASAVE"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define FILE_SYSTEM_SIZE 128 * 1024 // 文件系统大小 128KB
#define MAX_FILE_SIZE 4 * 1024      // 单个文件最大大小 4KB
#define MIN_FREE_BLOCKS 3            // 最小空闲空间 3 个 block
#define TIMESTAMP_FORMAT "%y%m%d%H%M%S.dat" // 时间戳格式

// 文件系统结构体
struct FileSystem {
    char *files[32]; // 假设最多 32 个文件
    int file_count;
};

// 检查文件名是否符合时间戳格式
int is_valid_timestamp_filename(const char *filename) {
    if (strlen(filename) != 16) return 0; // 长度不符合要求
    if (strncmp(filename + 12, ".dat", 4) != 0) return 0; // 后缀不符合要求

    // 检查每个部分是否都是数字
    for (int i = 0; i < 12; i++) {
        if (!isdigit(filename[i])) return 0;
    }

    return 1;
}

void data_save_as_file_init(struct FileSystem *fs) {
    DIR *dir;
    struct dirent *ent;
    rt_memset(fs, 0, sizeof(struct FileSystem));
    if ((dir = opendir("/")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if(is_valid_timestamp_filename(ent->d_name)) {
                fs->files[fs->file_count] = rt_strdup(ent->d_name);
                fs->file_count++;
            }
        }
        closedir(dir);
    }
}

// 将字符串转换为 long long 类型的时间值
long long string_to_long_long(const char *timestamp_str) {
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
void generate_timestamp_filename(char *filename) {
    time_t now = time(NULL);
    strftime(filename, 20, TIMESTAMP_FORMAT, localtime(&now));
}

// 删除最早的文件
void delete_oldest_file(struct FileSystem *fs) {
    if (fs->file_count > 0) {
        char tmp[20] = {0};
        DIR *dir;
        struct dirent *ent;
        char oldest_filename[20] = {0};
        time_t _time = time(NULL);
        long long current_time = 0;
        long long oldest_time = 0;
        strftime(tmp, 20, TIMESTAMP_FORMAT, localtime(&_time));
        oldest_time = string_to_long_long(tmp);

        // 打开跟目录
        if ((dir = opendir("/")) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                // 检查文件名是否符合时间戳格式
                LOG_D("Checking existed file: %s", ent->d_name);
                rt_memset(tmp, 0, sizeof(tmp));
                if(is_valid_timestamp_filename(ent->d_name)) {
                    rt_snprintf(tmp, 12, ent->d_name);
                    current_time = string_to_long_long(tmp);
                    if (current_time < oldest_time) {
                        oldest_time = current_time;
                        rt_memset(oldest_filename, 0, sizeof(oldest_filename));
                        rt_strncpy(oldest_filename, ent->d_name, rt_strlen(ent->d_name));
                    }
                }
            }

            // 删除最早的文件
            if (strlen(oldest_filename)) {
                LOG_D("Deleting oldest file: %s", oldest_filename);
                remove(oldest_filename);
                fs->file_count--; // 减少文件计数

                // 更新文件列表
                int i;
                for (i = 0; i < fs->file_count; i++) {
                    LOG_D("fs->files[i]: %d, %s", i, fs->files[i]);
                    if (strcmp(fs->files[i], oldest_filename) == 0) {
                        LOG_D("RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR");
                        rt_free(fs->files[i]);
                        LOG_D("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
                        fs->files[i] = NULL;
                        break;
                    }
                }
                LOG_D("MMMMMMMMMMMMMMMMMMMMMMMMMMMM");
                memmove(&fs->files[i], &fs->files[i + 1], (fs->file_count - i - 1) * sizeof(char *));
                fs->files[fs->file_count] = NULL;
            }
            closedir(dir);
        }
    }
}

// 获取最新文件名
int get_latest_file_name(char *filename) {
    DIR *dir;
    struct dirent *ent;
    char *latest_filename = NULL;
    long long current_time = 0;
    long long latest_time = 0;

    if(filename == NULL) {
        return -1;
    }

    // 打开根目录
    if ((dir = opendir("/")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // 检查文件名是否符合时间戳格式
            char tmp[20];
            rt_memset(tmp, 0, sizeof(tmp));
            if(is_valid_timestamp_filename(ent->d_name)) {
                rt_snprintf(tmp, 12, ent->d_name);
                current_time = string_to_long_long(tmp);
                if (current_time > latest_time) {
                    latest_time = current_time;
                    latest_filename = ent->d_name;
                }
            }
        }
        closedir(dir);
    }

    if(latest_filename) {
        rt_snprintf(filename, 17, latest_filename);
        return 0;
    }
        
    return -1;
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

// 追加或创建文件
int data_save_as_file(struct FileSystem *fs, const char *buffer, size_t length) {
    char filename[20] = {0};
    generate_timestamp_filename(filename);

    // 检查是否有足够的空闲空间
    int free_blocks = check_free_space("/");
    LOG_D("Free blocks: %d", free_blocks);
    if (free_blocks < MIN_FREE_BLOCKS) {
        delete_oldest_file(fs); // 删除最早的文件
    }

    // 获取最新文件的大小
    char latest_filename[20] = {0};
    get_latest_file_name(latest_filename);
    size_t latest_file_size = 0;

    if(strlen(latest_filename) == 0) {
        latest_file_size = 0;
    } else {
        LOG_D("Latest file name: %s", latest_filename);
        latest_file_size = get_file_size(latest_filename);
    }

    // 判断是否需要新建文件
    if (strlen(latest_filename) == 0 || latest_file_size + length > MAX_FILE_SIZE) {
        LOG_D("Creating new file: %s", filename);
        // 创建新文件
        FILE *file = fopen(filename, "ab");
        if (!file) {
            return -1; // 打开文件失败
        }
        fwrite(buffer, 1, length, file);
        fclose(file);

        fs->files[fs->file_count] = rt_strdup(filename); // 添加文件名到文件系统
        fs->file_count++; // 增加文件计数
    } else {
        // 追加到现有文件
        LOG_D("Appending to existed file: %s", latest_filename);
        FILE *file = fopen(latest_filename, "ab");
        if (!file) {
            return -1; // 打开文件失败
        }
        fwrite(buffer, 1, length, file);
        fclose(file);
    }

    return 0; // 成功
}


#include "rtthread.h"
char buffer[1024];

void data_save_as_file_test() {
    struct FileSystem fs;

    data_save_as_file_init(&fs);

    // char *buffer = "This is a test message.";
    // size_t length = 1024;
    rt_memset(buffer, '1', 1024);

    while (1)
    {
        // 测试函数
        if (data_save_as_file(&fs, buffer, 1024) == 0) {
            LOG_D("================");
        } else {
            LOG_D("xxxxxxxxxxxxxxxx");
        }
        rt_thread_mdelay(500);
    }
}

MSH_CMD_EXPORT(data_save_as_file_test, data save as file test);

