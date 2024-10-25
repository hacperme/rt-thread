#ifndef UPGRADE_MANAGER_H
#define UPGRADE_MANAGER_H

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
    UPGRADE_STATUS_DOWNLOADING,
    UPGRADE_STATUS_VERIFIED,
    UPGRADE_STATUS_UPGRADING,
    UPGRADE_STATUS_SUCCESS,
    UPGRADE_STATUS_FAILED,
} UpgradeStatus;

typedef struct {
    void (*prepare)(void);
    void (*download)(int* progress);
    void (*verify)(void);
    void (*apply)(int* progress);
    UpgradeStatus (*get_status)(void);
} UpgradeModule;

typedef struct UpgradeNode {
    UpgradeModule module;
    UpgradeModuleType type;
    char path[128];
    UpgradeStatus status;
    int download_progress;
    int upgrade_progress;
    struct UpgradeNode* next;
} UpgradeNode;

void add_module(UpgradeModule module, UpgradeModuleType type, const char* path);
void start_download(UpgradeNode* node);
void start_upgrade(UpgradeNode* node);
void retry_failed_module(UpgradeNode* node);
void print_upgrade_results(void);
UpgradeStatus check_overall_status(void);

#endif // UPGRADE_MANAGER_H
