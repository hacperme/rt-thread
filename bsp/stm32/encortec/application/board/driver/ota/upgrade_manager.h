#ifndef UPGRADE_MANAGER_H
#define UPGRADE_MANAGER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include "rtthread.h"
#include "dfs_fs.h"

#define MODULE_MAX_OTA_FILE_COUNT   5
#define MODULE_FILE_NAME_LENGTH     256
#define MODULE_DOWNLOAD_ADDR_LENGTH 1024

#define OTA_PATH                    "/fota"
#define OTA_CFG_FILE                "/fota/config.bin"
#define OTA_RETRY_CNT               3

typedef enum {
    UPGRADE_MODULE_NB = 0,
    UPGRADE_MODULE_CAT1,
    UPGRADE_MODULE_GNSS,
    UPGRADE_MODULE_ESP,
    UPGRADE_MODULE_ST,
} UpgradeModule;

typedef enum {
    UPGRADE_STATUS_NO_PLAN,
    UPGRADE_STATUS_ON_PLAN,
    UPGRADE_STATUS_DOWNLOADING,
    UPGRADE_STATUS_DOWNLOADING_FAILED,
    UPGRADE_STATUS_DOWNLOADED,
    UPGRADE_STATUS_VERIFY_FAILED,
    UPGRADE_STATUS_VERIFIED,
    UPGRADE_STATUS_PREPARE_FAILED,
    UPGRADE_STATUS_PREPARED,
    UPGRADE_STATUS_UPGRADING,
    UPGRADE_STATUS_SUCCESS,
    UPGRADE_STATUS_FAILED,
} UpgradeStatus;

typedef struct {
    void (*prepare)(void *node);  // 包含掉电，上电过程
    void (*apply)(int* progress, void *node);
    void (*finish)(void *node);
} UpgradeModuleOps;

typedef struct {
    char file_name[MODULE_FILE_NAME_LENGTH];
#if 0
    char download_addr[MODULE_DOWNLOAD_ADDR_LENGTH];
#endif
    char file_md5[16];
    uint32_t file_size;
    uint32_t downloaded_size;
    uint32_t piece_length;
    uint32_t start_addr;
    FILE *fd;
    char verified;
} UpgradeFile;

typedef struct {
    char source_version[128];
    char target_version[128];
    char file_cnt;
    UpgradeFile file[MODULE_MAX_OTA_FILE_COUNT];  // GNSS 特殊，有五个文件
} UpgradePlan;

typedef struct {
    UpgradeModule module;
    UpgradeStatus status;
    int download_progress;
    int upgrade_progress;
    char try_count;  // 重试计数
    UpgradePlan plan;
    UpgradeModuleOps ops;
} UpgradeNode;  // 存储文件系统内的配置，要更新module

int upgrade_manager_init(void);
int exit_upgrade_plan(void);
int get_module(UpgradeModule module, UpgradeNode *node);
void save_module(UpgradeNode *node);
void clear_module(UpgradeNode *node);
void init_module(UpgradeNode *node, UpgradePlan *plan, UpgradeModuleOps *ops);
void init_module_ops(UpgradeModule module, UpgradeModuleOps **ops);
void set_module(UpgradeModule module, UpgradePlan *plan);
// static void start_download(UpgradeNode* node);
// static void start_verify(UpgradeNode* node);
// static void start_upgrade(UpgradeNode* node);
void report_upgrade_results(void);
void prepare_upgrade_module(void);
void upgrade_all_module(void);

#endif // UPGRADE_MANAGER_H
