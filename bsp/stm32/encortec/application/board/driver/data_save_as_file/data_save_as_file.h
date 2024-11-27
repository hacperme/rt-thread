#ifndef __DATA_SAVE_AS_FILE_H__
#define __DATA_SAVE_AS_FILE_H__

#include <stdbool.h>

#define DIR_MAX_LEN 64
#define FILE_SUFFIX_MAX_LEN 10
#define FILE_NAME_MAX_LEN 64
#define FILE_SYSTEM_SIZE 512 * 1024 * 1024                // 文件系统大小 512 MB
#define SINGLE_FILE_SIZE_LIMIT_DFT 8 * 1024               // 单个文件最大大小 4MB
#define MIN_FREE_BLOCKS 2                                 // 最小空闲空间 1024 个 block


struct FileSystem {
    char oldest_dir_name[DIR_MAX_LEN];
    char latest_file_name[FILE_NAME_MAX_LEN];
    int single_file_size_limit;
    char suffix[FILE_SUFFIX_MAX_LEN];
    char base[DIR_MAX_LEN];
    int save_period;
};

extern void data_save_as_file_init(struct FileSystem *fs, int single_file_size_limit, const char *suffix, const char *base, int save_period);
extern char *get_oldest_file_name(struct FileSystem *fs);
extern char *get_latest_file_name(struct FileSystem *fs);
extern size_t get_file_size(const char *filename);
extern void delete_oldest_file(struct FileSystem *fs);
extern int data_save_as_file(struct FileSystem *fs, const char *buffer, size_t length, bool disable_single_file_size_limit, bool append);
extern int data_save_as_file_v2(struct FileSystem *fs, const char *buffer, size_t length, bool disable_single_file_size_limit);
extern void list_files(const char *path);
extern void delete_old_dirs(struct FileSystem *fs);

#endif