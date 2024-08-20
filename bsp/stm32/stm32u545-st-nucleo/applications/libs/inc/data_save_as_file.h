#ifndef __DATA_SAVE_AS_FILE_H__
#define __DATA_SAVE_AS_FILE_H__

#define FILE_NAME_MAX_LEN 20
#define FILE_SYSTEM_SIZE 128 * 1024         // 文件系统大小 128KB
#define MAX_FILE_SIZE 8 * 1024              // 单个文件最大大小 8KB
#define MIN_FREE_BLOCKS 2                   // 最小空闲空间 2 个 block
#define TIMESTAMP_FORMAT "%y%m%d%H%M%S.dat" // 时间戳格式

struct FileSystem {
    char oldest_file_name[FILE_NAME_MAX_LEN];
    char latest_file_name[FILE_NAME_MAX_LEN];
};

extern void data_save_as_file_init(struct FileSystem *fs);
extern char *get_oldest_file_name(struct FileSystem *fs);
extern char *get_latest_file_name(struct FileSystem *fs);
extern size_t get_file_size(const char *filename);
extern void delete_oldest_file(struct FileSystem *fs);
extern int data_save_as_file(struct FileSystem *fs, const char *buffer, size_t length);
extern void list_files(const char *path);

#endif