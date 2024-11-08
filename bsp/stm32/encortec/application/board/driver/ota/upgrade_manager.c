#include "upgrade_manager.h"
#include "dfs_fs.h"
#include "drv_hash.h"
#include "logging.h"

#define MD5_FILE_BUFFER_SIZE 1024

static UpgradeNode ota_node = {0};
static char upgrade_manager_init_tag = 0;

int upgrade_manager_init(void)
{
    if (access(OTA_PATH, F_OK) != 0) mkdir(OTA_PATH, 0755);

    if (access(OTA_CFG_FILE, F_OK) != 0)
    {
        FILE *ota_file = fopen(OTA_CFG_FILE, "ab");
        if (ota_file == NULL) return -1;
    
        size_t wres;
        for (int i = 0; i < 5; i++)
        {
            ota_node.module = (UpgradeModule)i;
            ota_node.status = UPGRADE_STATUS_NO_PLAN;
            wres = fwrite(&ota_node, 1, sizeof(ota_node), ota_file);
            if (wres != sizeof(ota_node)) break;
        }
        fclose(ota_file);
        if (wres != sizeof(ota_node)) return -2;
    }

    upgrade_manager_init_tag = 1;
    return 0;
}

int exit_upgrade_plan(void)
{
    int res = 0;
    for (int i = 0; i < 5; i++)
    {
        if (get_module((UpgradeModule)i, &ota_node) == 0)
        {
            if (ota_node.status != UPGRADE_STATUS_NO_PLAN)
            {
                res++;
                break;
            }
        }
    }
    return res;
}

int get_module(UpgradeModule module, UpgradeNode *node)
{
    size_t rres;
    FILE *ota_file = fopen(OTA_CFG_FILE, "rb");
    if (ota_file == NULL) return -1;
    fseek(ota_file, sizeof(*node) * module, SEEK_SET);
    rt_memset(node, 0, sizeof(*node));
    rres = fread(node, 1, sizeof(*node), ota_file);
    fclose(ota_file);
    return rres == sizeof(*node) ? 0 : -1;
}

void save_module(UpgradeNode *node)
{
    FILE *ota_file = fopen(OTA_CFG_FILE, "ab");
    fseek(ota_file, sizeof(*node) * node->module, SEEK_SET);
    fwrite(node, 1, sizeof(*node), ota_file);
    fclose(ota_file);
}

void clear_module(UpgradeNode *node) {
    node->status = UPGRADE_STATUS_NO_PLAN;
    node->download_progress = 0;
    node->upgrade_progress = 0;
    node->try_count = 0;
    rt_memset(&node->plan, 0, sizeof(node->plan));
    rt_memset(&node->ops, 0, sizeof(node->ops));
    save_module(node);
}

void init_module(UpgradeNode *node, UpgradePlan *plan, UpgradeModuleOps *ops)
{
    node->status = UPGRADE_STATUS_ON_PLAN;
    node->download_progress = 0;
    node->upgrade_progress = 0;
    node->try_count = 0;
    rt_memset(&node->plan, 0, sizeof(node->plan));
    rt_memcpy(&node->plan, plan, sizeof(*plan));
    rt_memset(&node->ops, 0, sizeof(node->ops));
    rt_memcpy(&node->ops, ops, sizeof(*ops));
}

void set_module(UpgradeModule module, UpgradePlan *plan, UpgradeModuleOps *ops)
{
    get_module(module, &ota_node);
    init_module(&ota_node, plan, ops);
    save_module(&ota_node);
}

static int md5cmp(char *str1, char *str2)
{
    int res = 0;
    for (int i = 0; i < 16; i++)
    {
        res = str1[i] == str2[i] ? 0 : -1;
        if (res != 0) break;
    }
    return res;
}

// TODO: 这里应该是一个同一的接口直接下载，这里看是否还需要每个模块单独一个download方法
static void start_download(UpgradeNode* node) {
    char retry_cnt = 0;
    node->status = UPGRADE_STATUS_DOWNLOADING;
    save_module(node);
    while (retry_cnt < 2)
    {
        node->ops.download(&node->download_progress, (void *)node);
        node->status = node->ops.get_status();
        log_debug("Download progress for module %d: %d%%", node->module, node->download_progress);
        if (node->status != UPGRADE_STATUS_DOWNLOADED)
        {
            retry_cnt++;
            log_error("Download failed for module %d, will retry...\n", node->module);
        }
        save_module(node);
    }
}

static void start_verify(UpgradeNode* node)
{
    if (node->status != UPGRADE_STATUS_DOWNLOADED) goto _exit_;
    if (node->module == UPGRADE_MODULE_NB)
    {
        node->status = UPGRADE_STATUS_VERIFIED;
        goto _exit_;
    }
    char file_buff[MD5_FILE_BUFFER_SIZE] = {0};
    uint32_t i, j;
    uint32_t file_size = 0;
    uint16_t last_size = 0;
    uint32_t file_range = 0;
    char file_cmp_md5[16] = {0};
    for (i = 0; i < node->plan.file_cnt; i++)
    {
        FILE *ota_file = fopen(node->plan.file[i].file_name, "rb");
        if (ota_file == NULL)
        {
            log_error("OTA File %s is opened failed.", node->plan.file[i].file_name);
            node->status = UPGRADE_STATUS_VERIFY_FAILED;
            goto _exit_;
        }

        // Get file size.
        fseek(ota_file, 0, SEEK_END);
        file_size = ftell(ota_file);

        // Compute file MD5.
        drv_hash_md5_create();
        fseek(ota_file, 0, SEEK_SET);
        file_range = (file_size % MD5_FILE_BUFFER_SIZE == 0) ? (file_size / MD5_FILE_BUFFER_SIZE - 1) : (file_size / MD5_FILE_BUFFER_SIZE);
        for (j = 0; j < file_range; j++)
        {
            rt_memset(file_buff, 0, MD5_FILE_BUFFER_SIZE);
            fread(file_buff, 1, MD5_FILE_BUFFER_SIZE, ota_file);
            drv_hash_md5_update(file_buff, MD5_FILE_BUFFER_SIZE);
        }
        rt_memset(file_buff, 0, MD5_FILE_BUFFER_SIZE);
        last_size = (file_size % MD5_FILE_BUFFER_SIZE == 0) ? MD5_FILE_BUFFER_SIZE : (file_size - j * MD5_FILE_BUFFER_SIZE);
        fread(file_buff, 1, last_size, ota_file);
        drv_hash_md5_finsh(file_buff, last_size, file_cmp_md5);
        fclose(ota_file);
    
        if (md5cmp(node->plan.file[i].file_md5, file_cmp_md5) == 0)
        {
            node->status = UPGRADE_STATUS_VERIFIED;
        }
        else
        {
            node->status = UPGRADE_STATUS_VERIFY_FAILED;
            goto _exit_;
        }
    }

_exit_:
    save_module(node);
    return;
}

static void start_upgrade(UpgradeNode* node) {
    node->status = UPGRADE_STATUS_UPGRADING;
    save_module(node);
    node->ops.prepare();
    node->ops.apply(&node->upgrade_progress, (void *)node);
    log_debug("Upgrade progress for module %d: %d%%", node->module, node->upgrade_progress);
    node->status = node->ops.get_status();
    if (node->status == UPGRADE_STATUS_SUCCESS) {
        log_debug("Upgrade successful for module %d at %s", node->module, node->plan.file[0].file_name);
    } else {
        log_debug("Upgrade failed for module %d", node->module);
    }
    node->ops.finish((void *)node);
    save_module(node);
}

// TODO: Report OTA Result to cloud by NB.
void report_upgrade_results(void)
{
    // 1. Report over to call clear_module.
    // 2. Report ST Upgrade result not in here.
}

// TODO: Enable NB enviroment to download OTA file.
void prepare_upgrade_module(void)
{

}

void upgrade_all_module(void) {
    prepare_upgrade_module();

    int i;
    char st_upgrade = 0;
    for (i = 0; i < 5; i++)
    {
        if (get_module((UpgradeModule)i, &ota_node) == 0)
        {
            if (ota_node.status == UPGRADE_STATUS_NO_PLAN || ota_node.status == UPGRADE_STATUS_SUCCESS || \
                ota_node.status == UPGRADE_STATUS_VERIFIED || ota_node.status == UPGRADE_STATUS_UPGRADING)
            {
                continue;
            }
            if (ota_node.try_count < OTA_RETRY_CNT)
            {
                ota_node.try_count++;
                save_module(&ota_node);
                start_download(&ota_node);
                start_verify(&ota_node);
            }
            else
            {
                ota_node.status = UPGRADE_STATUS_FAILED;
                save_module(&ota_node);
            }
        }
        else
        {
            log_debug("get module %d failed.", i);
        }
    }

    for (i = 0; i < 5; i++)
    {
        if (get_module((UpgradeModule)i, &ota_node) == 0)
        {
            if (ota_node.status == UPGRADE_STATUS_VERIFIED || ota_node.status == UPGRADE_STATUS_UPGRADING)
            {
                start_upgrade(&ota_node);
                if (i == UPGRADE_MODULE_ST && ota_node.status == UPGRADE_STATUS_UPGRADING)
                {
                    st_upgrade = 1;
                }
            }
        }
    }

    report_upgrade_results();
    if (st_upgrade == 1)
    {
        rt_hw_cpu_reset();
    }
}

extern UpgradeModuleOps esp_module;

static void test_set_esp_ota_plan(void)
{
    // 收到 ESP 升级计划, 记录升级相关信息并设置升级记录
    UpgradePlan esp_plan = {0};
    char test_msg[] = "xxx";
    rt_memcpy(esp_plan.source_version, test_msg, sizeof(test_msg));
    rt_memcpy(esp_plan.target_version, test_msg, sizeof(test_msg));
    esp_plan.file_cnt = 1;
    rt_memcpy(esp_plan.file[0].file_name, test_msg, sizeof(test_msg));
    rt_memcpy(esp_plan.file[0].download_addr, test_msg, sizeof(test_msg));
    rt_memcpy(esp_plan.file[0].file_md5, test_msg, sizeof(test_msg));
    set_module(UPGRADE_MODULE_ESP, &esp_plan, &esp_module);
}

void test_upgrade_process(void)
{
    // 设备上电即进行初始化
    upgrade_manager_init();
    // TODO: 开机初始化后，检测ST是否有升级结果，并进行上报。

    test_set_esp_ota_plan();
    // set_module(UPGRADE_MODULE_NB, &nb_plan, &nb_module);
    // set_module(UPGRADE_MODULE_CAT1, &cat1_plan, &cat1_module);
    // set_module(UPGRADE_MODULE_GNSS, &gnss_plan, &gnss_module);
    // set_module(UPGRADE_MODULE_ST &st_plan, &st_module);

    // 业务结束后 & 业务开始之前, 先检测是否有升级, 有则开启升级
    if (exit_upgrade_plan() > 0)
    {
        upgrade_all_module();
    }
}