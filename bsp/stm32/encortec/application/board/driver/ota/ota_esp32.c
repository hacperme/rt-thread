#include "upgrade_manager.h"
#include <stdio.h>

static UpgradeStatus esp_status;

void esp_download(int* progress) {
    printf("Downloading ESP firmware...\n");

    //下发命令控制NB下载ESP32升级包，同时接受NB反馈的下载进度比

    // 需要阻塞等待下载结果

    esp_status = UPGRADE_STATUS_VERIFIED;
    //保存到文件系统
}

void esp_prepare(void) {
    printf("Preparing ESP upgrade...\n");
    // 可在此处打开esp32 电源，//check 版本号*
    esp_status = UPGRADE_STATUS_DOWNLOADING;
    //
}

void esp_apply(int* progress) {
    printf("Applying ESP firmware...\n");

    //下发命令控制ESP32升级,同时接受esp32 升级进度的百分比， 以5%位单位

    esp_status = UPGRADE_STATUS_SUCCESS;
}

UpgradeStatus esp_get_status(void) {
    return esp_status;
}

UpgradeModuleOps esp_module = {
    .download = esp_download,
    .prepare = esp_prepare,
    .apply = esp_apply,
    .get_status = esp_get_status
};
