#include "upgrade_manager.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static UpgradeNode* head = NULL;
static UpgradeStatus overall_status = UPGRADE_STATUS_IDLE;
void add_module(UpgradeModule module, UpgradeModuleType type, const char* path) {
    UpgradeNode* new_node = (UpgradeNode*)malloc(sizeof(UpgradeNode));
    if (!new_node) {
        printf("Memory allocation failed!\n");
        return;
    }
    new_node->module = module;
    new_node->type = type;
    strncpy(new_node->path, path, sizeof(new_node->path) - 1);
    new_node->path[sizeof(new_node->path) - 1] = '\0';
    new_node->status = UPGRADE_STATUS_IDLE;
    new_node->download_progress = 0;
    new_node->upgrade_progress = 0;
    new_node->next = NULL;

    if (!head) {
        head = new_node;
    } else {
        UpgradeNode* current = head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }
}

void remove_module(UpgradeNode* node) {
    if (!head || !node) return;

    if (head == node) {
        head = head->next;
        free(node);
        return;
    }

    UpgradeNode* current = head;
    while (current->next && current->next != node) {
        current = current->next;
    }

    if (current->next == node) {
        current->next = node->next;
        free(node);
    }
}

void start_download(UpgradeNode* node) {
    int progress = 0;
    node->module.prepare();
    node->module.download(&progress);
    node->download_progress = progress;

    printf("Download progress for module %d: %d%%\n", node->type, progress);

    if (node->module.get_status() == UPGRADE_STATUS_FAILED) {
        printf("Download failed for module %d at %s, will retry...\n", node->type, node->path);
        node->status = UPGRADE_STATUS_FAILED;
    } else {
        node->module.verify();
        if (node->module.get_status() == UPGRADE_STATUS_VERIFIED) {
            node->status = UPGRADE_STATUS_VERIFIED;
        } else {
            printf("Verification failed for module %d at %s, will retry...\n", node->type, node->path);
            node->status = UPGRADE_STATUS_FAILED;
        }
    }
}

void start_upgrade(UpgradeNode* node) {
    if (node->status == UPGRADE_STATUS_VERIFIED) {
        int progress = 0;
        node->module.apply(&progress);
        node->upgrade_progress = progress;

        printf("Upgrade progress for module %d: %d%%\n", node->type, progress);

        if (node->module.get_status() == UPGRADE_STATUS_SUCCESS) {
            node->status = UPGRADE_STATUS_SUCCESS;
            printf("Upgrade successful for module %d at %s\n", node->type, node->path);
            remove_module(node);
        } else {
            node->status = UPGRADE_STATUS_FAILED;
            printf("Upgrade failed for module %d at %s, will retry...\n", node->type, node->path);
        }
    }
}

void retry_failed_module(UpgradeNode* node) {
    if (node->status == UPGRADE_STATUS_FAILED) {
        printf("Retrying module %d at %s...\n", node->type, node->path);
        start_download(node);
        if (node->status == UPGRADE_STATUS_VERIFIED) {
            start_upgrade(node);
        }
    }
}

void print_upgrade_results(void) {
    UpgradeNode* current = head;
    printf("Upgrade Results:\n");
    while (current) {
        printf("Module %d at %s - Status: %d, Download Progress: %d%%, Upgrade Progress: %d%%\n",
               current->type, current->path, current->status, current->download_progress, current->upgrade_progress);
        current = current->next;
    }
}

UpgradeStatus check_overall_status(void) {
    UpgradeNode* current = head;
    while (current) {
        if (current->status == UPGRADE_STATUS_FAILED) {
            return UPGRADE_STATUS_FAILED;
        }
        current = current->next;
    }
    return UPGRADE_STATUS_SUCCESS;
}


extern UpgradeModule esp_module;
int upgrade_all_module(void) {
    // add_module(nb_module, UPGRADE_MODULE_NB, "/fota/nb/");
    // add_module(cat1_module, UPGRADE_MODULE_CAT1, "/fota/cat1/");
    // add_module(gnss_module, UPGRADE_MODULE_GNSS, "/fota/gnss/");
    // add_module(esp_module, UPGRADE_MODULE_ESP, "/fota/esp/");
    // add_module(st_module, UPGRADE_MODULE_ST, "/fota/st/");

    UpgradeNode* current;

    // 下载和升级过程
    while ((current = head) != NULL) {
        // 遍历链表中的每个模块进行下载和升级
        while (current != NULL) {
            if (current->status == UPGRADE_STATUS_IDLE || current->status == UPGRADE_STATUS_FAILED) {
                // 先进行下载
                start_download(current);

                // 如果下载成功，进行升级
                if (current->status == UPGRADE_STATUS_VERIFIED) {
                    start_upgrade(current);
                }

                // 如果升级仍然失败，则重新尝试
                if (current->status == UPGRADE_STATUS_FAILED) {
                    retry_failed_module(current);
                }
            }

            // 继续下一个节点
            current = current->next;
        }

        // 打印当前所有模块的升级结果
        print_upgrade_results();

        // 检查整体状态
        if (check_overall_status() == UPGRADE_STATUS_SUCCESS) {
            printf("All modules upgraded successfully.\n");
            break;
        }
    }

    // 确保所有节点都已被释放
    printf("Upgrade process completed.\n");

    return 0;
}