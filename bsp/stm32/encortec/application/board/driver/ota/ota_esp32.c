#include "upgrade_manager.h"
#include <stdio.h>

void esp_prepare(void *node) {
    printf("Preparing ESP upgrade...\n");
    UpgradeNode *_node = (UpgradeNode *)node;
    // 可在此处打开esp32 电源，//check 版本号*
    _node->status = UPGRADE_STATUS_DOWNLOADING;
}

void esp_apply(int* progress, void *node) {
    printf("Applying ESP firmware...\n");

    //下发命令控制ESP32升级,同时接受esp32 升级进度的百分比， 以5%位单位

    UpgradeNode *_node = (UpgradeNode *)node;
    _node->status = UPGRADE_STATUS_SUCCESS;
}

void esp_finish(void *node) {}

UpgradeModuleOps esp_module = {
    .prepare = esp_prepare,
    .apply = esp_apply,
    .finish = esp_finish,
};
