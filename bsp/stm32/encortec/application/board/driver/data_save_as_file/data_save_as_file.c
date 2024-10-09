#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include "dfs_fs.h"
#include <ctype.h>
#include "data_save_as_file.h"
#include "logging.h"

// 检查文件名是否符合时间戳格式
static int is_valid_timestamp_dirname(struct FileSystem *fs, const char *dirname) {
    // dirname: 20241009
    if (rt_strlen(dirname) != 8) return 0; // 长度不符合要求

    // 检查每个部分是否都是数字
    for (int i = 0; i < 8; i++) {
        if (!isdigit(dirname[i])) return 0;
    }

    return 1;
}

// 将字符串转换为 long long 类型的时间值
static long long timestring_to_long_long(const char *timestamp_str) {
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

static long long datestring_to_long_long(const char *datestamp_str) {
    // datestamp_str: 20241009

    long long time_value = 0;
    int year, month, day;

    // 读取年份
    year = (datestamp_str[0] - '0') * 1000 + (datestamp_str[1] - '0') * 100 + (datestamp_str[2] - '0') * 10 + (datestamp_str[3] - '0');
    // 读取月份
    month = (datestamp_str[4] - '0') * 10 + (datestamp_str[5] - '0');
    // 读取日期
    day = (datestamp_str[6] - '0') * 10 + (datestamp_str[7] - '0');

    // 构建时间值
    time_value = year * 10000LL; // 年份
    time_value += month * 100LL; // 月份
    time_value += day;     // 日

    return time_value;
}

// 生成时间戳文件名
static void generate_timestamp_filename(struct tm *cur, struct FileSystem *fs, const char *dir_with_date, char *filename) {
    char filename_with_time[20] = {0};

    strftime(filename_with_time, 20, "%Y%m%d%H%M%S", cur);

    // 目录结构：/data/20241009/20241009100850.dat
    sprintf(
        filename,
        "%s/%s/%s/%s%s",
        fs->base, dir_with_date, filename_with_time, fs->suffix
    );
}

// 初始化文件系统
static void data_save_as_file_info_refresh(struct FileSystem *fs) {
	char tmp[DIR_MAX_LEN];
    DIR *dir;
    struct dirent *ent;
    char oldest_dir_name[DIR_MAX_LEN] = {0};
    char latest_dir_name[DIR_MAX_LEN] = {0};
    char latest_file_name[FILE_NAME_MAX_LEN] = {0};
    long long oldest_time = 0x0FFFFFFFFFFFFFFF;
    long long latest_time = 0;

    // 获取最新和最旧的文件夹名称
    if ((dir = opendir(fs->base)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // 检查文件名是否符合时间戳格式
            if (is_valid_timestamp_dirname(fs, ent->d_name)) {
                rt_memset(tmp, 0, DIR_MAX_LEN);
                rt_strncpy(tmp, ent->d_name, rt_strlen(ent->d_name));
                log_debug("tmp: %s\n", tmp);

                long long current_time = datestring_to_long_long(tmp);
                log_debug("current_time: %lld\n", current_time);
                log_debug("oldest_time: %lld\n", oldest_time);
                log_debug("latest_time: %lld\n", latest_time);

                if (current_time < oldest_time) {
                    rt_memset(oldest_dir_name, 0, DIR_MAX_LEN);
                    rt_strncpy(oldest_dir_name, ent->d_name, rt_strlen(ent->d_name));
                    oldest_time = current_time;
                    log_debug("set oldest_dir_name: %s\n", oldest_dir_name);
                }

                if (current_time > latest_time) {
                    rt_memset(latest_dir_name, 0, DIR_MAX_LEN);
                    rt_strncpy(latest_dir_name, ent->d_name, rt_strlen(ent->d_name));
                    latest_time = current_time;
                    log_debug("set latest_dir_name: %s\n", latest_dir_name);
                }
            }
        }
        closedir(dir);
    }

    // 记录最旧文件夹名称
    rt_memset(fs->oldest_dir_name, 0, DIR_MAX_LEN);
    rt_strncpy(fs->oldest_dir_name, oldest_dir_name, rt_strlen(oldest_dir_name));
    log_debug("fs->oldest_dir_name: %s\n", fs->oldest_dir_name);
    
    // 获取最新文件名称
    char temp_latest[64] = {0};
    sprintf(temp_latest, "%s/%s", fs->base, latest_dir_name);
    log_debug("latest dir: %s\n", temp_latest);
    if (rt_strlen(latest_dir_name) != 0) {
        if ((dir = opendir(temp_latest)) != NULL) {
            oldest_time = 0x0FFFFFFFFFFFFFFF;
            latest_time = 0;

            while ((ent = readdir(dir)) != NULL) {
                rt_memset(temp_latest, 0, 64);  // 20241009152701.dat
                rt_strncpy(temp_latest, ent->d_name, rt_strlen(ent->d_name));
                long long current_time = timestring_to_long_long(temp_latest);

                if (current_time > latest_time) {
                    rt_memset(latest_file_name, 0, FILE_NAME_MAX_LEN);
                    rt_strncpy(latest_file_name, ent->d_name, rt_strlen(ent->d_name));
                    latest_time = current_time;
                } 
            }
            closedir(dir);
        }

    }

    rt_memset(fs->latest_file_name, 0, FILE_NAME_MAX_LEN);
    // rt_strncpy(fs->latest_file_name, latest_file_name, rt_strlen(latest_file_name));
    if (rt_strlen(latest_file_name) != 0 && rt_strlen(latest_dir_name) != 0) {
        sprintf(fs->latest_file_name, "%s/%s/%s", fs->base, latest_dir_name, latest_file_name);
    }
    log_debug("fs->latest_file_name: %s\n", fs->latest_file_name);
}

void data_save_as_file_init(struct FileSystem *fs, int single_file_size_limit, const char *suffix, const char *base, int save_period) {
    if(single_file_size_limit) {
        fs->single_file_size_limit = single_file_size_limit;
    } else {
        fs->single_file_size_limit = SINGLE_FILE_SIZE_LIMIT_DFT;
    }
    fs->save_period = save_period <= 0 ? 30 : save_period;
    strcpy(fs->suffix, suffix);
    strcpy(fs->base, base);

    // 判断目录是否存在
    DIR* dir = opendir(fs->base);
    if (dir == NULL) {
        mkdir(fs->base, 0755);
    }
    data_save_as_file_info_refresh(fs);
}

char *get_oldest_dir_name(struct FileSystem *fs) {
    data_save_as_file_info_refresh(fs);
    if(fs && fs->oldest_dir_name[0] != '\0') {
        return fs->oldest_dir_name;
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
    log_debug(
        "stat.f_bsize=%d, stat.f_blocks=%d, stat.f_bfree=%d, stat.f_bavail=%d",
        stat.f_bsize, stat.f_blocks, stat.f_bfree, stat.f_bavail
    );
    return stat.f_bfree;
}

int delete_directory(const char *dir);
// 删除最早的文件
void delete_oldest_dir(struct FileSystem *fs) {
    char *oldest_dir_name = NULL;
    if((oldest_dir_name = get_oldest_dir_name(fs))) {
        log_debug("xx Deleting oldest dir: %s", oldest_dir_name);
        delete_directory(oldest_dir_name);
        data_save_as_file_info_refresh(fs);
    }
}

// 追加或创建文件
int data_save_as_file(struct FileSystem *fs, const char *buffer, size_t length, bool disable_single_file_size_limit, bool append) {
    // 获取最新文件的大小
    char *latest_file_name = NULL;
    size_t latest_file_size = 0;

    if((latest_file_name = get_latest_file_name(fs))) {
        latest_file_size = get_file_size(latest_file_name);
    }

    // 判断是否需要新建文件
    // if (latest_file_name == NULL || (!disable_single_file_size_limit && (latest_file_size + length > fs->single_file_size_limit))) {
    if (!append || latest_file_name == NULL) {
        char filename[FILE_NAME_MAX_LEN] = {0};
        time_t now = time(NULL);
        struct tm *cur = localtime(&now);

        char dir_with_date[20] = {0};
        strftime(dir_with_date, 20, "%Y%m%d", cur);
        // 判断二级目录是否存在，不存在则创建
        char sub_dir_path[30] = {0};
        sprintf(sub_dir_path, "%s/%s", fs->base, dir_with_date);
        DIR *dir = opendir(sub_dir_path);
        if (dir == NULL) {
            mkdir(sub_dir_path, 0755);
        }

        char filename_with_time[20] = {0};
        strftime(filename_with_time, 20, "%Y%m%d%H%M%S", cur);
        // 目录结构：/data/20241009/20241009100850.dat
        sprintf(
            filename,
            "%s/%s/%s%s",
            fs->base, dir_with_date, filename_with_time, fs->suffix
        );

        // 创建新文件
        log_debug("++ Creating file: %s\n", filename);
        FILE *file = fopen(filename, "ab");
        if (!file) {
            log_error("open file %s failed.");
            return -1; // 打开文件失败
        }

        int retry = 0;
        for(retry = 0; retry < 5 && fwrite(buffer, 1, length, file) != length; retry++){
            log_error("?? Write file failed (%d)", retry);
        }

        fclose(file);

        data_save_as_file_info_refresh(fs);

        if(retry >= 5) {
            return -1;
        }
    } else {
        // 追加到现有文件
        latest_file_name = get_latest_file_name(fs);
        log_debug("-- Append to file %s", latest_file_name);
        FILE *file = fopen(latest_file_name, "ab");
        if (!file) {
            log_error("append to file open file %s failed.", latest_file_name);
            return -1; // 打开文件失败
        }

        int retry = 0;
        for(retry = 0; retry < 5 && fwrite(buffer, 1, length, file) != length; retry++){
            log_error("?? Write file failed, retry (%d)", retry);
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
    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            rt_kprintf("%s    %d Bytes\r\n", ent->d_name, get_file_size(ent->d_name));
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
            log_info("Deleting %s", ent->d_name);
            remove(ent->d_name);
        }
        closedir(dir);
    }
}
// MSH_CMD_EXPORT(del_files, del files);

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
 
int delete_directory(const char *dir) {
    DIR *dp;
    struct dirent *entry;
    char subdir[256];
 
    dp = opendir(dir);
    if (dp == NULL) {
        perror("opendir");
        return -1;
    }
 
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
 
        snprintf(subdir, sizeof(subdir), "%s/%s", dir, entry->d_name);
        if (entry->d_type == DT_DIR) {
            delete_directory(subdir);
        } else {
            if (unlink(subdir) == -1) {
                perror("unlink");
                closedir(dp);
                return -1;
            }
        }
    }
    closedir(dp);
 
    if (rmdir(dir) == -1) {
        perror("rmdir");
        return -1;
    }
 
    return 0;
}


void delete_old_dirs(struct FileSystem *fs)
{
    int year;
    int month;
    int day;
    struct tm timeinfo = {0};
    time_t timestamp = 0;
    char temp_path[64] = {0};


    time_t cur_timestamp;
    time(&cur_timestamp);
    log_debug("cur_timestamp: %d\n", cur_timestamp);

    while (1) {
        char *oldest_dir = get_oldest_dir_name(fs);  // 20241009
        if (oldest_dir == NULL) {
            log_debug("oldest_dir is NULL");
            break;
        }
        log_debug("oldest_dir: %s\n", oldest_dir);

        timeinfo.tm_year = (oldest_dir[0] - '0') * 1000 + (oldest_dir[1] - '0') * 100 + (oldest_dir[2] - '0') * 10 + (oldest_dir[3] - '0') - 1900; 
        timeinfo.tm_mon = (oldest_dir[4] - '0') * 10 + (oldest_dir[5] - '0') - 1;
        timeinfo.tm_mday = (oldest_dir[6] - '0') * 10 + (oldest_dir[7] - '0');
        timestamp = mktime(&timeinfo);
        log_debug("oldest_dir timestamp: %d\n", timestamp);

        if (cur_timestamp - timestamp > fs->save_period * 86400) {
            log_debug("oldest_dir \"%s\" over 30 days, will be deleted", oldest_dir);
            sprintf(temp_path, "%s/%s", fs->base, oldest_dir);
            delete_directory(temp_path);
            data_save_as_file_info_refresh(fs);
        }
        else {
            break;
        }
    }
}

void data_save_as_file_test() {
    struct FileSystem fs;

    data_save_as_file_init(&fs, 0, ".dat", "/data", -1);
    
    rt_memset(data_buffer, '1', 1024);
    int cnt = 5;

    while (cnt > 0)
    {
        log_info("<<<<<<<<<<<< Before written to file");
        list_files(fs.base);
        if (data_save_as_file(&fs, data_buffer, 1024, true, cnt >= 3 ? false : true) == 0) {
            log_info("Data saved successfully.");
        } else {
            log_info("Data saved failed.");
        }
        log_info("----------------------------------------------------------------------");
        rt_thread_mdelay(500);
        cnt--;
    }
    log_debug("-------delete old dirs--------");
    delete_old_dirs(&fs);
    log_debug("------------------------------");
}

// MSH_CMD_EXPORT(data_save_as_file_test, data save as file test);
