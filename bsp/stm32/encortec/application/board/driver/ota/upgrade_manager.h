#ifndef UPGRADE_MANAGER_H
#define UPGRADE_MANAGER_H

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
    UPGRADE_MODULE_NONE
} UpgradeModule;

typedef enum {
    UPGRADE_STATUS_NO_PLAN,
    UPGRADE_STATUS_ON_PLAN,
    UPGRADE_STATUS_DOWNLOADING,
    UPGRADE_STATUS_DOWNLOADED,
    UPGRADE_STATUS_DOWNLOADING_FAILED,
    UPGRADE_STATUS_VERIFY_FAILED,
    UPGRADE_STATUS_VERIFIED,
    UPGRADE_STATUS_UPGRADING,
    UPGRADE_STATUS_SUCCESS,
    UPGRADE_STATUS_FAILED,
} UpgradeStatus;

typedef struct {
    void (*download)(int* progress, UpgradeNode *node);
    void (*prepare)(void);// 包含掉电，上电过程
    void (*apply)(int* progress, UpgradeNode *node);
    void (*finish)(UpgradeNode *node);
    UpgradeStatus (*get_status)(void);
} UpgradeModuleOps;

typedef struct {
    char file_name[MODULE_FILE_NAME_LENGTH];
    char download_addr[MODULE_DOWNLOAD_ADDR_LENGTH];
    char file_md5[16];
    uint32_t file_size;
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
void set_module(UpgradeModule module, UpgradePlan *plan, UpgradeModuleOps *ops);
static void start_download(UpgradeNode* node);
static void start_upgrade(UpgradeNode* node);
void report_upgrade_results(void);
void prepare_upgrade_module(void);
void upgrade_all_module(void);

#endif // UPGRADE_MANAGER_H
