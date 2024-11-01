#ifndef UPGRADE_MANAGER_H
#define UPGRADE_MANAGER_H

#define MODULE_MAX_OTA_FILE_COUNT 5

typedef enum {
    UPGRADE_MODULE_NB,
    UPGRADE_MODULE_CAT1,
    UPGRADE_MODULE_GNSS,
    UPGRADE_MODULE_ESP,
    UPGRADE_MODULE_ST,
    UPGRADE_MODULE_NONE
} UpgradeModuleType;

typedef enum {
    UPGRADE_STATUS_IDLE,
    UPGRADE_STATUS_ON,
    UPGRADE_STATUS_DOWNLOADING,
    UPGRADE_STATUS_VERIFIED,
    UPGRADE_STATUS_UPGRADING,
    UPGRADE_STATUS_SUCCESS,
    UPGRADE_STATUS_FAILED,
} UpgradeStatus;

typedef struct {
    void (*download)(int* progress);
    void (*verify)(void);
    void (*prepare)(void);// 包含掉电，上电过程
    void (*apply)(int* progress);
    UpgradeStatus (*get_status)(void);
} UpgradeModule;

typedef struct UpgradeNode {
    UpgradeModule module; //ops
    UpgradeModuleType type; //module
    char try_count;
    char file_name[MODULE_MAX_OTA_FILE_COUNT][64]; //GNSS 特殊，有五个文件
    char download_addr[MODULE_MAX_OTA_FILE_COUNT][1024]; //存在多个下载文件链接
    UpgradeStatus status;
    int download_progress;
    int upgrade_progress;
    char source_version[128];
    char target_version[128];
    // struct UpgradeNode* next;
} UpgradeNode; //使用1个静态数组，存储文件系统内的配置，要更新module

void add_module(UpgradeModule module, UpgradeModuleType type, const char* path); //保存至文件,不需要链表
void start_download(UpgradeNode* node);
void start_upgrade(UpgradeNode* node);
void retry_failed_module(UpgradeNode* node);
void print_upgrade_results(void);
UpgradeStatus check_overall_status(void);

#endif // UPGRADE_MANAGER_H
