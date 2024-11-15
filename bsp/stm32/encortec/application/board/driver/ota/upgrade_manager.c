#include "upgrade_manager.h"
#include "dfs_fs.h"
#include "drv_hash.h"
#include "tools.h"
#include "logging.h"

#define MD5_FILE_BUFFER_SIZE 1024

static UpgradeNode ota_node = {0};
static char upgrade_manager_init_tag = 0;

int upgrade_manager_init(void)
{
    rt_err_t res = drv_hash_init();
    log_debug("drv_hash_init %s", res_msg(res == RT_EOK));

    if (access(OTA_PATH, F_OK) != 0) mkdir(OTA_PATH, 0755);

    if (access(OTA_CFG_FILE, F_OK) != 0)
    {
        FILE *ota_file = fopen(OTA_CFG_FILE, "wb");
        if (ota_file == NULL) return -1;
    
        size_t wres;
        for (int i = 0; i < 5; i++)
        {
            ota_node.module = (UpgradeModule)i;
            ota_node.status = UPGRADE_STATUS_NO_PLAN;
            wres = fwrite(&ota_node, 1, sizeof(ota_node), ota_file);
            log_debug("init module %d %s", ota_node.module, res_msg(wres == sizeof(ota_node)));
            if (wres != sizeof(ota_node)) break;
        }
        fclose(ota_file);
        if (wres != sizeof(ota_node)) return -2;
    }

    upgrade_manager_init_tag = 1;
    return 0;
}

rt_err_t delete_ota_cfg_file(void)
{
    rt_err_t res;
    res = upgrade_manager_init_tag == 0 ? RT_EOK : RT_ERROR;
    if (res != RT_EOK) return res;
    res = access(OTA_CFG_FILE, F_OK) != 0 ? RT_EOK : RT_ERROR;
    if (res == RT_EOK) return res;

    res = remove(OTA_CFG_FILE);
    return res;
}

int exit_upgrade_plan(void)
{
    log_debug("exit_upgrade_plan");
    int res = 0;
    for (int i = 0; i < 5; i++)
    {
        if (get_module((UpgradeModule)i, &ota_node) == 0)
        {
            log_debug("ota_node.module=%d ota_node.status=%d", ota_node.module, ota_node.status);
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
    int ret;
    size_t rres;
    rt_memset(node, 0, sizeof(*node));

    FILE *ota_file = fopen(OTA_CFG_FILE, "rb");
    if (ota_file == NULL) return -1;
    ret = fseek(ota_file, sizeof(*node) * module, SEEK_SET);
    log_debug("fseek offset=%d, ret=%d", sizeof(*node) * module, ret);
    rres = fread(node, 1, sizeof(*node), ota_file);
    log_debug("fread rres=%d, sizeof(*node)=%d", rres, sizeof(*node));
    ret = fclose(ota_file);
    log_debug("fclose ret=%d", ret);
    return rres == sizeof(*node) ? 0 : -1;
}

void save_module(UpgradeNode *node)
{
    int ret;
    FILE *ota_file = fopen(OTA_CFG_FILE, "wb");
    ret = fseek(ota_file, sizeof(*node) * node->module, SEEK_SET);
    log_debug("fseek offset=%d, ret=%d", sizeof(*node) * node->module, ret);
    size_t wret = fwrite(node, 1, sizeof(*node), ota_file);
    log_debug("fwrite wret=%d, sizeof(*node)=%d", wret, sizeof(*node));
    ret = fclose(ota_file);
    log_debug("fclose ret=%d", ret);
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
    log_debug("ota_node.status=%d", ota_node.status);
    log_debug("ota_node.plan.file_cnt=%d", ota_node.plan.file_cnt);
    log_debug("ota_node.plan.file[0].file_name=%s", ota_node.plan.file[0].file_name);
    log_debug("ota_node.ops.prepare=0x%08X", ota_node.ops.prepare);
    log_debug("ota_node.ops.apply=0x%08X", ota_node.ops.apply);
    log_debug("ota_node.ops.finish=0x%08X", ota_node.ops.finish);
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
void start_download(UpgradeNode* node) {
    char retry_cnt = 0;
    node->status = UPGRADE_STATUS_DOWNLOADING;
    save_module(node);
    while (retry_cnt < 2)
    {
        // TODO: Download File Option.

        node->status = UPGRADE_STATUS_DOWNLOADED;
        save_module(node);
        retry_cnt++;
        if (node->status == UPGRADE_STATUS_DOWNLOADED) break;
    }
}

void start_verify(UpgradeNode* node)
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
    rt_err_t res;
    size_t ret;
    if (res != RT_EOK)
    {
        node->status = UPGRADE_STATUS_VERIFY_FAILED;
        return;
    }
    for (i = 0; i < node->plan.file_cnt; i++)
    {
        FILE *ota_file = fopen(node->plan.file[i].file_name, "rb");
        if (ota_file == NULL)
        {
            log_error("OTA File %s is opened failed.", node->plan.file[i].file_name);
            goto _exit_;
        }

        // Get file size.
        fseek(ota_file, 0, SEEK_END);
        file_size = ftell(ota_file);
        log_debug("file %s size=%d", node->plan.file[i].file_name, file_size);

        // Compute file MD5.
        res = drv_hash_md5_create();
        log_debug("drv_hash_md5_create %s", res_msg(res == RT_EOK));
        if (res != RT_EOK)
        {
            fclose(ota_file);
            goto _exit_;
        }
        fseek(ota_file, 0, SEEK_SET);
        file_range = (file_size % MD5_FILE_BUFFER_SIZE == 0) ? (file_size / MD5_FILE_BUFFER_SIZE - 1) : (file_size / MD5_FILE_BUFFER_SIZE);
        log_debug("file_range %d", file_range);
        for (j = 0; j < file_range; j++)
        {
            rt_memset(file_buff, 0, MD5_FILE_BUFFER_SIZE);
            ret = fread(file_buff, 1, MD5_FILE_BUFFER_SIZE, ota_file);
            res = ret == MD5_FILE_BUFFER_SIZE ? RT_EOK : RT_ERROR;
            if (res != RT_EOK) break;
            res = drv_hash_md5_update(file_buff, MD5_FILE_BUFFER_SIZE);
            // log_debug("j=%d ftell=%d fread ret=%d drv_hash_md5_update %s", j, ftell(ota_file), ret, res_msg(res == RT_EOK));
            if (res != RT_EOK) break;
            rt_thread_mdelay(10);
        }
        rt_memset(file_buff, 0, MD5_FILE_BUFFER_SIZE);
        last_size = (file_size % MD5_FILE_BUFFER_SIZE == 0) ? MD5_FILE_BUFFER_SIZE : (file_size % MD5_FILE_BUFFER_SIZE);
        ret = fread(file_buff, 1, last_size, ota_file);
        res = drv_hash_md5_finsh((rt_uint8_t *)file_buff, (rt_uint32_t)last_size, (rt_uint8_t *)file_cmp_md5);
        log_debug("ftell=%d fread ret=%d drv_hash_md5_finsh %s, last_size=%d", ftell(ota_file), ret, res_msg(res == RT_EOK), last_size);
        log_debug(
            "file_cmp_md5 %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            file_cmp_md5[0], file_cmp_md5[1], file_cmp_md5[2], file_cmp_md5[3],
            file_cmp_md5[4], file_cmp_md5[5], file_cmp_md5[6], file_cmp_md5[7],
            file_cmp_md5[8], file_cmp_md5[9], file_cmp_md5[10], file_cmp_md5[11],
            file_cmp_md5[12], file_cmp_md5[13], file_cmp_md5[14], file_cmp_md5[15]
        );
        drv_hash_md5_destroy();
        fclose(ota_file);
    
        if (rt_strncmp(node->plan.file[i].file_md5, file_cmp_md5, sizeof(file_cmp_md5)) == 0)
        {
            log_debug("file %s md5 verfy success.", node->plan.file[i].file_name);
            node->plan.file[i].verified = 1;
            if (node->plan.file[i].file_size != file_size) node->plan.file[i].file_size = file_size;
            node->status = UPGRADE_STATUS_VERIFIED;
        }
        else
        {
            log_debug("file %s md5 verfy failed.", node->plan.file[i].file_name);
            node->plan.file[i].verified = 0;
            goto _exit_;
        }
    }
    return;

_exit_:
    log_debug("start_verify _exit_");
    node->status = UPGRADE_STATUS_VERIFY_FAILED;
    return;
}

void start_upgrade(UpgradeNode* node)
{
    log_debug("start_upgrade");
    node->ops.prepare((void *)node);
    save_module(node);
    if (node->status == UPGRADE_STATUS_PREPARED)
    {
        node->status = UPGRADE_STATUS_UPGRADING;
        save_module(node);
        node->ops.apply(&node->upgrade_progress, (void *)node);
        save_module(node);
        log_info("Upgrade %s for module %d", res_msg(node->status == UPGRADE_STATUS_SUCCESS), node->module);
        node->ops.finish((void *)node);
        save_module(node);
    }
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

void upgrade_all_module(void)
{
    log_debug("upgrade_all_module");
    prepare_upgrade_module();

    int i;
    char st_upgrade = 0;
    for (i = 0; i < 5; i++)
    {
        if (get_module((UpgradeModule)i, &ota_node) == 0)
        {
            log_debug("ota_node.module=%d ota_node.status=%d", ota_node.module, ota_node.status);
            if (ota_node.status < UPGRADE_STATUS_ON_PLAN || ota_node.status >= UPGRADE_STATUS_VERIFIED)
            {
                continue;
            }
            if (ota_node.try_count < OTA_RETRY_CNT)
            {
                ota_node.try_count++;
                save_module(&ota_node);
                start_download(&ota_node);
                if (ota_node.status == UPGRADE_STATUS_DOWNLOADED)
                {
                    start_verify(&ota_node);
                    save_module(&ota_node);
                }
            }
            else
            {
                ota_node.status = UPGRADE_STATUS_DOWNLOADING_FAILED;
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
            log_debug("ota_node.module=%d ota_node.status=%d", ota_node.module, ota_node.status);
            if (ota_node.status >= UPGRADE_STATUS_VERIFIED && ota_node.status < UPGRADE_STATUS_SUCCESS)
            {
                start_upgrade(&ota_node);
                save_module(&ota_node);
                if (i == UPGRADE_MODULE_ST && ota_node.status == UPGRADE_STATUS_UPGRADING)
                {
                    st_upgrade = 1;
                }
            }
        }
    }

    report_upgrade_results();
    if (st_upgrade == 1) rt_hw_cpu_reset();
}


static void test_set_esp_ota_plan(void)
{
    // 收到 ESP 升级计划, 记录升级相关信息并设置升级记录
    extern UpgradeModuleOps esp_ota_ops;

    UpgradePlan esp_plan = {0};
    char test_msg[] = "xxx";
    rt_memcpy(esp_plan.source_version, test_msg, sizeof(test_msg));
    rt_memcpy(esp_plan.target_version, test_msg, sizeof(test_msg));
    esp_plan.file_cnt = 1;
    rt_memcpy(esp_plan.file[0].file_name, test_msg, sizeof(test_msg));
    rt_memcpy(esp_plan.file[0].download_addr, test_msg, sizeof(test_msg));
    rt_memcpy(esp_plan.file[0].file_md5, test_msg, sizeof(test_msg));
    set_module(UPGRADE_MODULE_ESP, &esp_plan, &esp_ota_ops);
}

static void test_set_cat1_ota_plan(void)
{
    extern UpgradeModuleOps cat1_ota_ops;

    char file_name[] = "./cat1-qth-v01.bin";
    char cat1_target_version[] = "EG915NEUAPR03A04M16_01.200.01.200";
    char file_md5[] = {110, 102, 151, 255, 226, 166, 190, 8, 249, 55, 140, 118, 227, 119, 15, 192};

    // char file_name[] = "./cat1-v01-qth.bin";
    // char cat1_target_version[] = "EG915NEUAPR03A03M16_QTH_01.200.01.200";
    // char file_md5[] = {206, 5, 194, 244, 21, 18, 238, 214, 207, 96, 187, 214, 24, 38, 115, 83};

    UpgradePlan cat1_plan = {0};
    cat1_plan.file_cnt = 1;
    rt_memcpy(cat1_plan.target_version, cat1_target_version, sizeof(cat1_target_version));

    rt_memcpy(cat1_plan.file[0].file_name, file_name, sizeof(file_name));
    log_debug("cat1_plan.file[0].file_name %s", cat1_plan.file[0].file_name);
    rt_memcpy(cat1_plan.file[0].file_md5, file_md5, sizeof(file_md5));
    log_debug("cat1_plan.file[0].file_md5 %s", cat1_plan.file[0].file_md5);

    set_module(UPGRADE_MODULE_CAT1, &cat1_plan, &cat1_ota_ops);

    int res = get_module(UPGRADE_MODULE_CAT1, &ota_node);
    log_debug("get_module UPGRADE_MODULE_CAT1 %s", res_msg(res == 0));
    if (res != 0) return;
    log_debug("ota_node.status=%d", ota_node.status);
    log_debug("ota_node.plan.file_cnt=%d", ota_node.plan.file_cnt);
    log_debug("ota_node.plan.file[0].file_name=%s", ota_node.plan.file[0].file_name);
    log_debug("ota_node.ops.prepare=0x%08X", ota_node.ops.prepare);
    log_debug("ota_node.ops.apply=0x%08X", ota_node.ops.apply);
    log_debug("ota_node.ops.finish=0x%08X", ota_node.ops.finish);
    ota_node.status = UPGRADE_STATUS_DOWNLOADED;
    log_debug("ota_node.status=%d", ota_node.status);
    save_module(&ota_node);
}

static void test_set_gnss_ota_plan(void)
{
    extern UpgradeModuleOps gnss_ota_ops;

    char file0_name[] = "/fota/da_uart_115200.bin";
    char file0_md5[] = {175, 87, 90, 84, 136, 88, 127, 203, 180, 154, 90, 188, 117, 13, 157, 245};
    char file1_name[] = "/fota/partition_table.bin";
    char file1_md5[] = {45, 180, 78, 183, 87, 108, 139, 217, 154, 103, 63, 74, 92, 246, 80, 249};
    char file2_name[] = "/fota/bootloader.bin";
    char file2_md5[] = {41, 253, 239, 179, 200, 113, 245, 0, 108, 84, 171, 70, 187, 147, 155, 46};
    char file3_name[] = "/fota/LC76GPANR12A03S.bin";
    char file3_md5[] = {120, 91, 149, 191, 168, 224, 78, 163, 36, 66, 59, 208, 143, 44, 212, 222};
    char file4_name[] = "/fota/gnss_config.bin";
    char file4_md5[] = {171, 124, 243, 255, 61, 5, 41, 20, 207, 50, 249, 218, 141, 227, 61, 129};

    UpgradePlan gnss_plan = {0};
    gnss_plan.file_cnt = 5;
    char gnss_target_verion[] = "LC76GPANR12A03S,2024/04/14,15:42:19";
    rt_memcpy(gnss_plan.target_version, gnss_target_verion, sizeof(gnss_target_verion));
    log_debug("gnss_plan.target_version %s", gnss_plan.target_version);

    rt_memcpy(gnss_plan.file[0].file_name, file0_name, sizeof(file0_name));
    log_debug("gnss_plan.file[0].file_name %s", gnss_plan.file[0].file_name);
    rt_memcpy(gnss_plan.file[0].file_md5, file0_md5, sizeof(file0_md5));
    log_debug("gnss_plan.file[0].file_md5 %s", gnss_plan.file[0].file_md5);
    gnss_plan.file[0].file_size = 43956;

    rt_memcpy(gnss_plan.file[1].file_name, file1_name, sizeof(file1_name));
    log_debug("gnss_plan.file[1].file_name %s", gnss_plan.file[1].file_name);
    rt_memcpy(gnss_plan.file[1].file_md5, file1_md5, sizeof(file1_md5));
    log_debug("gnss_plan.file[1].file_md5 %s", gnss_plan.file[1].file_md5);
    gnss_plan.file[1].file_size = 432;

    rt_memcpy(gnss_plan.file[2].file_name, file2_name, sizeof(file2_name));
    log_debug("gnss_plan.file[2].file_name %s", gnss_plan.file[2].file_name);
    rt_memcpy(gnss_plan.file[2].file_md5, file2_md5, sizeof(file2_md5));
    log_debug("gnss_plan.file[2].file_md5 %s", gnss_plan.file[2].file_md5);
    gnss_plan.file[2].file_size = 22416;

    rt_memcpy(gnss_plan.file[3].file_name, file3_name, sizeof(file3_name));
    log_debug("gnss_plan.file[3].file_name %s", gnss_plan.file[3].file_name);
    rt_memcpy(gnss_plan.file[3].file_md5, file3_md5, sizeof(file3_md5));
    log_debug("gnss_plan.file[3].file_md5 %s", gnss_plan.file[3].file_md5);
    gnss_plan.file[3].file_size = 1118672;

    rt_memcpy(gnss_plan.file[4].file_name, file4_name, sizeof(file4_name));
    log_debug("gnss_plan.file[4].file_name %s", gnss_plan.file[4].file_name);
    rt_memcpy(gnss_plan.file[4].file_md5, file4_md5, sizeof(file4_md5));
    log_debug("gnss_plan.file[4].file_md5 %s", gnss_plan.file[4].file_md5);
    gnss_plan.file[4].file_size = 1024;

    set_module(UPGRADE_MODULE_GNSS, &gnss_plan, &gnss_ota_ops);

    int res = get_module(UPGRADE_MODULE_GNSS, &ota_node);
    log_debug("get_module UPGRADE_MODULE_GNSS %s", res_msg(res == 0));
    if (res != 0) return;

    ota_node.status = UPGRADE_STATUS_VERIFIED;
    log_debug("ota_node.status=%d", ota_node.status);
    save_module(&ota_node);
}

void test_upgrade_process(void)
{
    rt_err_t res;

    // TODO: 待删除，测试代码，用于清除升级配置文件。
    res = delete_ota_cfg_file();
    log_debug("delete_ota_cfg_file res=%d", res);

    // 设备上电即进行初始化
    res = upgrade_manager_init();
    log_debug("upgrade_manager_init %s", res_msg(res == 0));

    // TODO: 开机初始化后，检测ST是否有升级结果，并进行上报。

    // TODO: 收到升级计划后，进行设置
    // test_set_esp_ota_plan();
    test_set_cat1_ota_plan();
    // test_set_gnss_ota_plan();

    // 业务结束后 & 业务开始之前, 先检测是否有升级, 有则开启升级
    if (exit_upgrade_plan() > 0)
    {
        upgrade_all_module();
    }
}
