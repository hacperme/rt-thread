#include "nbiot.h"
#include <stdio.h>
#include "cJSON.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h> // For strcasecmp
#include <ctype.h>   // For isxdigit

#include "logging.h"
#include "gnss.h"
#include "upgrade_manager.h"
#include <dfs_fs.h> 

// #define DBG_TAG "nbiot"
// #define DBG_LVL DBG_LOG
// #include <rtdbg.h>

#define NBIOT_OTA_DATA_MSG_BUFFER_LENGTH (1024)
typedef struct 
{
    uint32_t data_offset;
    uint32_t data_size;
    uint8_t data[NBIOT_OTA_DATA_MSG_BUFFER_LENGTH];
}nbiot_ota_data_msg_t;


#define NBIOT_OTA_TASK_CONFIG_FILE "/fota/ota_task.conf"

extern void read_app_version_information(uint8_t **app_version, uint8_t **app_subedition, uint8_t **app_build_time);
extern rt_err_t cat1_at_query_version(char *cat1_version, rt_size_t size);
rt_err_t esp32_at_query_version(char *esp32_version, rt_size_t size);

static struct rt_mutex qiot_event_mutex;
static int QIOT_SUBSCRIBE_EVENT_CODE = -1;
static int QIOT_CONNECT_EVENT_CODE = -1;
static int QIOT_AUTH_EVENT_CODE = -1;
static int QIOT_DATA_SEND_EVENT_CODE = -1;
static int QIOT_DATA_RECV_EVENT_CODE = -1;
static int QIOT_DISCONNECT_EVENT_CODE = -1;
static int QIOT_OTA_EVENT_CODE = -1;
static int QIOT_PLATFORM_EVENT_CODE = -1;
static int QIOT_WAKEUP_EVENT_CODE = -1;

static rt_mq_t cclk_data_urc_queue = RT_NULL;
static rt_mq_t ota_data_urc_queue = RT_NULL;

typedef struct
{
	char name[50];
    char source_version[128];
    char target_version[128];
    char fileName[256];
    int batteryLimit;
    int minSignalIntensity;
    int compomentSize;
    unsigned char md5Bytes[16];
    int startaddr;
    int downloaded_size;
    int piece_length;
    int state;
    FILE *fd;
}ota_compoment_t;


typedef enum
{
    FIRM_TYPE_MODULE = 0, /*模组升级*/
    FIRM_TYPE_MCU /*MCU SOTA升级*/
}firm_type_t;




typedef struct 
{
    ota_task_state_t state;
    firm_type_t type;
    UpgradeModule module;
    int current_copmment;
    int compoment_cnt;
    int compoment_downlaoded_cnt;
    ota_compoment_t compoment[5];
}ota_task_t;

static ota_task_t g_ota_task = {0};


static int save_ota_task_to_file(ota_task_t *task)
{
    FILE *file = fopen(NBIOT_OTA_TASK_CONFIG_FILE, "wb");
    if (file == NULL)
    {
        log_error("open ota task flie err");
        return -1;
    }

    if(fwrite(task, 1, sizeof(ota_task_t), file) != sizeof(ota_task_t))
    {
        log_error("save ota task flie err");
        fclose(file);
        return -1;
    }
    fclose(file);

    return 0;

}

static int read_ota_task_from_file(ota_task_t *task)
{
    FILE *file = fopen(NBIOT_OTA_TASK_CONFIG_FILE, "rb");
    if (file == NULL)
    {
        log_error("open ota task flie err");
        return -1;
    }

    if(fread(task, 1, sizeof(ota_task_t), file) != sizeof(ota_task_t))
    {
        log_error("read ota task flie err");
        fclose(file);
        return -1;
    }
    fclose(file);

    return 0;

}

static int clean_ota_task(ota_task_t *task)
{
    if(task)
    {
        for (size_t i = 0; i < task->compoment_cnt; i++)
        {
            if(access(task->compoment[i].fileName, F_OK) == 0)
            {
                remove(task->compoment[i].fileName);
            }
        }

        rt_memset(task, 0, sizeof(ota_task_t));
    }
    remove(NBIOT_OTA_TASK_CONFIG_FILE);
    return 0;

}


static int get_file_index_by_componentNo(char *componentNo)
{
    int ret = 0;
    if(componentNo == NULL || strcasecmp(componentNo, "NB") == 0)
    {
        return ret;
    }
    if(strcasecmp(componentNo, "STM32-A") == 0)
    {
        ret = 0;
    }
    else if(strcasecmp(componentNo, "STM32-B") == 0)
    {
        ret = 1;
    }
    else if(strcasecmp(componentNo, "ESP32") == 0)
    {
        ret = 0;
    }
    else if(strcasecmp(componentNo, "CAT1") == 0)
    {
        ret = 0;
    }
    else if(strcasecmp(componentNo, "GNSS-P0") == 0)
    {
        // da_uart_115200.bin
        ret = 0;
    }
    else if(strcasecmp(componentNo, "GNSS-P1") == 0)
    {
        // partition_table.bin
        ret = 1;
    }
    else if(strcasecmp(componentNo, "GNSS-P2") == 0)
    {
        // bootloader.bin
        ret = 2;
    }
    else if(strcasecmp(componentNo, "GNSS-P3") == 0)
    {
        // LC76GPANR12A03S.bin
        ret = 3;
    }
    else if(strcasecmp(componentNo, "GNSS-P4") == 0)
    {
        // gnss_config.bin
        ret = 4;
    }

    return ret;
}

static UpgradeModule get_module_by_componentNo(char *componentNo)
{
    UpgradeModule ret = UPGRADE_MODULE_NB;
    if(componentNo == NULL || strcasecmp(componentNo, "NB") == 0)
    {
        return ret;
    }
    if(strcasecmp(componentNo, "STM32-A") == 0)
    {
        ret = UPGRADE_MODULE_ST;
    }
    else if(strcasecmp(componentNo, "STM32-B") == 0)
    {
        ret = UPGRADE_MODULE_ST;
    }
    else if(strcasecmp(componentNo, "ESP32") == 0)
    {
        ret = UPGRADE_MODULE_ESP;
    }
    else if(strcasecmp(componentNo, "CAT1") == 0)
    {
        ret = UPGRADE_MODULE_CAT1;
    }
    else
    {
        ret = UPGRADE_MODULE_GNSS;
    }

    return ret;
}

static void parse_ota_task_data(const char* urcString, ota_task_t *ctl)
{
    char cleanedString[256] = {0};
    char componentNo[50] = {0};
    char source_version[128];
    char target_version[128];
    int batteryLimit;
    int minSignalIntensity;
    int useSpace;
    strncpy(cleanedString, urcString, sizeof(cleanedString) - 1);
    cleanedString[strcspn(cleanedString, "\r\n")] = 0;

    const char* start = cleanedString;

    char* end = strchr(start, '\"'); 
    if (end) {
        start = end + 1; 
        end = strchr(start, '\"'); 
        if (end) {
            strncpy(componentNo, start, end - start);
            componentNo[end - start] = '\0'; 
            start = end + 2; 
        }
    }

    end = strchr(start, '\"');
    if (end) {
        start = end + 1;
        end = strchr(start, '\"');
        if (end) {
            strncpy(source_version, start, end - start);
            source_version[end - start] = '\0';
            start = end + 2;
        }
    }

    end = strchr(start, '\"');
    if (end) {
        start = end + 1;
        end = strchr(start, '\"');
        if (end) {
            strncpy(target_version, start, end - start);
            target_version[end - start] = '\0';
            start = end + 2; 
        }
    }

    batteryLimit = atoi(start);
    while (*start && *start != ',') start++; 
    start++; 

    minSignalIntensity = atoi(start);
    while (*start && *start != ',') start++; 
    start++; 

    useSpace = atoi(start);


    if((strlen(componentNo) == 0) || (strcasecmp(componentNo, "NB") == 0))
    {
        ctl->type = FIRM_TYPE_MODULE;
    }
    else
    {
        ctl->type = FIRM_TYPE_MCU;
    }

    // if(ctl->type == FIRM_TYPE_MCU)
    {
        ctl->current_copmment = get_file_index_by_componentNo(componentNo);
        ctl->module = get_module_by_componentNo(componentNo);
        if(ctl->module == UPGRADE_MODULE_ST)
        {
            ctl->compoment_cnt = 2;
        }
        else if(ctl->module == UPGRADE_MODULE_GNSS)
        {
            ctl->compoment_cnt = 5;
        }
        else if(ctl->module == UPGRADE_MODULE_NB)
        {
            ctl->compoment_cnt = 0;
        }
        else
        {
            ctl->compoment_cnt = 1;
        }

        ctl->compoment_downlaoded_cnt = 0;
        ctl->state = OTA_TASK_STATE_RECV; // 有下发升级计划了
        
        rt_memset(&ctl->compoment[ctl->current_copmment], 0, sizeof(ota_compoment_t));
        rt_strcpy(ctl->compoment[ctl->current_copmment].name, componentNo);
        rt_strcpy(ctl->compoment[ctl->current_copmment].source_version, source_version);
        rt_strcpy(ctl->compoment[ctl->current_copmment].target_version, target_version);
        ctl->compoment[ctl->current_copmment].batteryLimit = batteryLimit;
        ctl->compoment[ctl->current_copmment].minSignalIntensity = minSignalIntensity;
        ctl->compoment[ctl->current_copmment].compomentSize = useSpace;
        save_ota_task_to_file(ctl);
        
    }

    log_debug("Component No: %s\n", strlen(componentNo) > 0 ? componentNo : "Empty");
    log_debug("Source Version: %s\n", strlen(source_version) > 0 ? source_version : "Empty");
    log_debug("Target Version: %s\n", strlen(target_version) > 0 ? target_version : "Empty");
    log_debug("Battery Limit: %d\n", batteryLimit);
    log_debug("Min Signal Intensity: %d\n", minSignalIntensity);
    log_debug("Use Space: %d\n", useSpace);
    log_debug("firm type: %d\n", ctl->type);
}

static int convertMD5ToBytes(const char* md5String, unsigned char* md5Bytes, size_t byteArraySize) {
    size_t length = strlen(md5String);
    if (length != 32 || byteArraySize < 16) {
        return -1;
    }

    for (size_t i = 0; i < 16; ++i) {
        char hexByte[3] = { md5String[i * 2], md5String[i * 2 + 1], '\0' };
        if (!isxdigit(hexByte[0]) || !isxdigit(hexByte[1])) {
            return -1;
        }
        md5Bytes[i] = (unsigned char)strtol(hexByte, NULL, 16);
    }
    return 0;
}

static int parse_ota_firm_data(const char* urcString, ota_task_t *ctl)
{
    char componentNo[50] = {0};
    int length = 0;
    char md5[50] = {0};
    unsigned char md5Bytes[16] = {0};

    char cleanedString[256] = {0};
    if(ctl->type != FIRM_TYPE_MODULE)
    {
        strncpy(cleanedString, urcString, sizeof(cleanedString) - 1);

        cleanedString[strcspn(cleanedString, "\r\n")] = 0;

        const char* start = cleanedString;
        char* end;

        end = strchr(start, '\"'); 
        if (end) {
            start = end + 1; 
            end = strchr(start, '\"');
            if (end) {
                strncpy(componentNo, start, end - start);
                componentNo[end - start] = '\0'; 
                start = end + 2; 
            }
        }

        log_debug("Component No: %s\n", strlen(componentNo) > 0 ? componentNo : "Empty");
        ctl->current_copmment = get_file_index_by_componentNo(componentNo);
        
        ctl->module = get_module_by_componentNo(componentNo);
        log_debug("ctl->module: %d\n", ctl->module);

        if (*start != '\0') {
            length = atoi(start);
            while (*start && *start != ',') start++; 
            start++; 
        }

        log_debug("Length: %d\n", length);

        if(length > 0)
        {
            ctl->compoment[ctl->current_copmment].compomentSize = length;
        }

        end = strchr(start, '\"');
        if (end) {
            start = end + 1; 
            end = strchr(start, '\"'); 
            if (end) {
                strncpy(md5, start, end - start);
                md5[end - start] = '\0';
            }
        }

        if (convertMD5ToBytes(md5, md5Bytes, sizeof(md5Bytes)) == 0) 
        {
#if 0
            log_debug("MD5 Bytes: ");
            for (size_t i = 0; i < sizeof(md5Bytes); ++i) {
                log_debug("%02x", md5Bytes[i]);
                if (i < sizeof(md5Bytes) - 1) log_debug(" ");
            }
            log_debug("\n");
#endif
            rt_memcpy(ctl->compoment[ctl->current_copmment].md5Bytes, md5Bytes, sizeof(md5Bytes));
        } else {
            log_error("Invalid MD5 format.\n");
            return -1;
        }
    }
    
    ctl->state = OTA_TASK_STATE_DOWNLOADING;
    save_ota_task_to_file(ctl);

    return 0;
}

static int parse_ota_downloaded_data(const char* urcString,  ota_task_t *ctl) 
{
    char componentNo[50] = {0};
    int length = 0;
    int startaddr = 0;
    int piece_length = 0;

    char cleanedString[256] = {0};
    if(ctl->type != FIRM_TYPE_MODULE)
    {
        strncpy(cleanedString, urcString, sizeof(cleanedString) - 1);

        cleanedString[strcspn(cleanedString, "\r\n")] = 0;

        const char* start = cleanedString;
        char* end;


        end = strchr(start, '\"'); 
        if (end) {
            start = end + 1; 
            end = strchr(start, '\"'); 
            if (end) {
                strncpy(componentNo, start, end - start);
                componentNo[end - start] = '\0'; 
                start = end + 2; 
            }
        }

        log_debug("Component No: %s\n", strlen(componentNo) > 0 ? componentNo : "Empty");
        ctl->current_copmment = get_file_index_by_componentNo(componentNo);
        if(strcasecmp(ctl->compoment[ctl->current_copmment].name, componentNo) != 0)
        {
            rt_memset(ctl->compoment[ctl->current_copmment].name, 0, sizeof(ctl->compoment[ctl->current_copmment].name));
            rt_strcpy(ctl->compoment[ctl->current_copmment].name, componentNo);
        }
        ctl->module = get_module_by_componentNo(componentNo);
        log_debug("ctl->module: %d\n", ctl->module);

        if (*start != '\0') {
            length = atoi(start);
            while (*start && *start != ',') start++; 
            start++;
        }

        log_debug("Length: %d\n", length);
        if(length != ctl->compoment[ctl->current_copmment].compomentSize)
        {
            log_error("firm length mismatch: %d,%d", length, ctl->compoment[ctl->current_copmment].compomentSize);
            ctl->compoment[ctl->current_copmment].compomentSize = length;
        }

        if (*start != '\0') {
            startaddr = atoi(start);
            while (*start && *start != ',') start++; 
            start++; 
        }

        ctl->compoment[ctl->current_copmment].startaddr = startaddr;

        if (*start != '\0') {
            piece_length = atoi(start);
        }

        ctl->compoment[ctl->current_copmment].piece_length = piece_length;
        rt_snprintf(ctl->compoment[ctl->current_copmment].fileName, 
                        sizeof(ctl->compoment[ctl->current_copmment].fileName), "/fota/%s.bin", componentNo);
        
        log_debug("old fd=%0x", ctl->compoment[ctl->current_copmment].fd);
        if(ctl->compoment[ctl->current_copmment].fd != RT_NULL)
        {
            log_debug("close old fd=%0x", ctl->compoment[ctl->current_copmment].fd);
            fclose(ctl->compoment[ctl->current_copmment].fd);
            ctl->compoment[ctl->current_copmment].fd = RT_NULL;
        }

        if(startaddr == 0)
        {
            ctl->compoment[ctl->current_copmment].fd = fopen(ctl->compoment[ctl->current_copmment].fileName, "wb+");
            ctl->compoment[ctl->current_copmment].downloaded_size = 0;
        }
        else
        {
            ctl->compoment[ctl->current_copmment].fd = fopen(ctl->compoment[ctl->current_copmment].fileName, "rb+");
        }

        log_debug("new old fd=%0x", ctl->compoment[ctl->current_copmment].fd);
        
        if(ctl->compoment[ctl->current_copmment].fd == NULL)
        {
            log_debug("open ota file falid");
            return -1;
        }
    }
    ctl->state = OTA_TASK_STATE_DOWNLOADED;
    save_ota_task_to_file(ctl);

    return 0;
}

void nbiot_qiotevt_urc_handler(struct at_client *client, const char *data, rt_size_t size)
{
    int event_type;
    int event_code;
    char event_data[256] = {0};

    // +QIOTEVT: <event_type>,<event_code>[,<data>]
    sscanf(data, "+QIOTEVT: %d,%d,%s", &event_type, &event_code, event_data);
    log_debug("event_type: %d; event_code: %d; event_data: %s", event_type, event_code, event_data);
    
    switch (event_type)
    {
        case 1:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_AUTH_EVENT_CODE = event_code;
            log_debug("QIOT_AUTH_EVENT_CODE: %d", QIOT_AUTH_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 2:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_CONNECT_EVENT_CODE = event_code;
            log_debug("QIOT_CONNECT_EVENT_CODE: %d", QIOT_CONNECT_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 3:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_SUBSCRIBE_EVENT_CODE = event_code;
            log_debug("QIOT_SUBSCRIBE_EVENT_CODE: %d", QIOT_SUBSCRIBE_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 4:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_DATA_SEND_EVENT_CODE = event_code;
            log_debug("QIOT_DATA_SEND_EVENT_CODE: %d", QIOT_DATA_SEND_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 5:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_DATA_RECV_EVENT_CODE = event_code;
            log_debug("QIOT_DATA_RECV_EVENT_CODE: %d", QIOT_DATA_RECV_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 6:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_DISCONNECT_EVENT_CODE = event_code;
            log_debug("QIOT_DISCONNECT_EVENT_CODE: %d", QIOT_DISCONNECT_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 7:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_OTA_EVENT_CODE = event_code;
            log_debug("QIOT_OTA_EVENT_CODE: %d", QIOT_OTA_EVENT_CODE);
            switch (QIOT_OTA_EVENT_CODE)
            {
            case QIOT_OTA_TASK_NOTIFY:
                // read_ota_task_from_file(&g_ota_task);
                parse_ota_task_data((const char *)event_data, &g_ota_task);
                nbiot_ota_update_action(1);
                break;
            case QIOT_OTA_START:
                read_ota_task_from_file(&g_ota_task);
                if(parse_ota_firm_data((const char *)event_data, &g_ota_task) != 0)
                {

                }
                break;
            case QIOT_OTA_DOWNLOADING:
                break;
            case QIOT_OTA_DOWNLOADED:
                read_ota_task_from_file(&g_ota_task);
                parse_ota_downloaded_data((const char *)event_data, &g_ota_task);
                break;
            case QIOT_OTA_UPDATING:
                break;
            case QIOT_OTA_UPDATE_OK:
                read_ota_task_from_file(&g_ota_task);
                if(g_ota_task.type == FIRM_TYPE_MODULE)
                {
                    g_ota_task.state = OTA_TASK_STATE_FINISH;
                    save_ota_task_to_file(&g_ota_task);
                }
                else
                {
                    if(g_ota_task.state == OTA_TASK_STATE_UPGRADEING)
                    {
                        g_ota_task.state = OTA_TASK_STATE_FINISH;
                        save_ota_task_to_file(&g_ota_task);
                    }
                    
                }
                break;
            case QIOT_OTA_UPDATE_FAIL:
                log_debug("QIOT_OTA_UPDATE_FAIL, old fd=%0x", g_ota_task.compoment[g_ota_task.current_copmment].fd);
                if(g_ota_task.compoment[g_ota_task.current_copmment].fd != RT_NULL)
                {
                    log_debug("close old fd=%0x", g_ota_task.compoment[g_ota_task.current_copmment].fd);
                    fclose(g_ota_task.compoment[g_ota_task.current_copmment].fd);
                    g_ota_task.compoment[g_ota_task.current_copmment].fd = RT_NULL;
                }
                read_ota_task_from_file(&g_ota_task);
                
                if(g_ota_task.type == FIRM_TYPE_MODULE)
                {
                    g_ota_task.state = OTA_TASK_STATE_FINISH;
                    save_ota_task_to_file(&g_ota_task);
                }
                else
                {
                    if(g_ota_task.state == OTA_TASK_STATE_UPGRADEING)
                    {
                        g_ota_task.state = OTA_TASK_STATE_FINISH;
                        save_ota_task_to_file(&g_ota_task);
                    }
                }
                break;
            case QIOT_OTA_UPDATE_FLAG:
                break;
            default:
                break;
            }
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 8:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_PLATFORM_EVENT_CODE = event_code;
            log_debug("QIOT_PLATFORM_EVENT_CODE: %d", QIOT_PLATFORM_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        case 9:
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            QIOT_WAKEUP_EVENT_CODE = event_code;
            log_debug("QIOT_WAKEUP_EVENT_CODE: %d", QIOT_WAKEUP_EVENT_CODE);
            rt_mutex_release(&qiot_event_mutex);
            break;
        default:
            break;
    }
}

void nbiot_cclk_urc_handler(struct at_client *client, const char *data, rt_size_t size)
{
    // +CCLK: 24/08/16,06:28:32+32
    log_error("nbiot_cclk_urc_handler called, size: %d; data: %s\n", size, data);
    rt_err_t result = rt_mq_send(cclk_data_urc_queue, data, size);
    if (result != RT_EOK) {
        log_error("nbiot_cclk_urc_handler rt_mq_send ERR\n");
    }
}

void nbiot_ota_read_data_urc_handler(struct at_client *client, const char *data, rt_size_t size)
{
    // +QIOTOTARD: 37012,974\r\n
    int data_offset = 0;
    int data_size = 0;
    nbiot_ota_data_msg_t send_msg = {0};
    int read_size = 0;
    rt_err_t result;
    rt_memset(&send_msg, 0, sizeof(nbiot_ota_data_msg_t));
    log_info("nbiot_ota_read_data_urc_handler called, size: %d; data: %s\n", size, data);
    sscanf(data, "+QIOTOTARD: %d,%d", &data_offset, &data_size);
    log_info(" data_offset %d; data_size %d\n", data_offset, data_size);
    send_msg.data_offset = data_offset;
    send_msg.data_size = 0;
    if(data_size > 0)
    {
        do
        {
            read_size = at_client_obj_recv(client, send_msg.data, data_size, rt_tick_from_millisecond(10*1000));
            // log_info("read_size %d\n", read_size);
            if(read_size > 0)
            {
                send_msg.data_size += read_size;
                data_size -= read_size;
            }
        } while (data_size);  
    }

    log_info("send_msg.data_size %d\n", send_msg.data_size);

    if(ota_data_urc_queue)
    {
        result = rt_mq_send_wait(ota_data_urc_queue, &send_msg, sizeof(nbiot_ota_data_msg_t), RT_WAITING_FOREVER);
        if (result != RT_EOK) {
            log_error("nbiot_ota_read_data_urc_handler %d\n", result);
        }
    }
    

}

static struct at_urc nbiot_urc_table[] = {
    {"+QIOTEVT: ", "\r\n", nbiot_qiotevt_urc_handler},
    {"+CCLK: ", "\r\n", nbiot_cclk_urc_handler},
    {"+QIOTOTARD: ", "\r\n", nbiot_ota_read_data_urc_handler},
};

rt_err_t nbiot_at_client_init(void)
{
    rt_err_t result = RT_EOK;

    if (at_client_get(NBIOT_AT_UART_NAME) != RT_NULL) {
        log_error("nbiot at client already inited.\n");
        return RT_ERROR;
    }

    result = at_client_init(NBIOT_AT_UART_NAME, 1024, 512);
    if (result != RT_EOK) {
        log_error("nbiot at client init failed: %d\n", result);
        return result;
    }
    log_debug("nbiot at client init success.");

    result = at_obj_set_urc_table(at_client_get(NBIOT_AT_UART_NAME), nbiot_urc_table, 
                                  sizeof(nbiot_urc_table) / sizeof(nbiot_urc_table[0]));
    if (result != RT_EOK) {
        log_error("nbiot at set urc failed: %d\n", result);
        return result;
    }
    log_debug("nbiot at set urc success.");

    result = rt_mutex_init(&qiot_event_mutex, "qiot_event_mutex", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK) {
        log_error("qiot_event_mutex init failed: %d\n", result);
        return result;
    }
    log_debug("qiot_event_mutex init success.");

    cclk_data_urc_queue = rt_mq_create("cclk_data_urc_queue", 64, 1, RT_IPC_FLAG_FIFO);
    if (cclk_data_urc_queue == RT_NULL) {
        log_error("cclk_data_urc_queue create failed!\n");
        return -RT_ERROR;
    }

    ota_data_urc_queue = rt_mq_create("ota_data_urc_queue", sizeof(nbiot_ota_data_msg_t), 1, RT_IPC_FLAG_FIFO);
    if (ota_data_urc_queue == RT_NULL) {
        log_error("ota_data_urc_queue create failed!\n");
        return -RT_ERROR;
    }

    return result;
}
// MSH_CMD_EXPORT(nbiot_at_client_init, nbiot_at_client_init);

rt_err_t nbiot_disable_sleep_mode()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_obj_exec_cmd(client, RT_NULL, "AT");
    rt_thread_mdelay(200);

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(1000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        result = RT_ERROR;
        goto __ERROR__;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QSCLK=0");
    if (result == RT_EOK) {
        log_debug("nbiot disable sleep mode ok");
    }
    else {
        log_debug("nbiot disable sleep mode failed");
    }

__ERROR__:
    at_delete_resp(resp);
    return result;
}
// MSH_CMD_EXPORT(nbiot_disable_sleep_mode, nbiot_disable_sleep_mode);

rt_err_t nbiot_enable_sleep_mode()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(5000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }
    result = at_obj_exec_cmd(client, resp, "AT+QSCLK=1");
    if (result == RT_EOK) {
        log_debug("nbiot enable sleep mode ok");
    }
    else {
        log_debug("nbiot enable sleep mode failed");
    }

    at_delete_resp(resp);
    return result;
}
// MSH_CMD_EXPORT(nbiot_enable_sleep_mode, nbiot_enable_sleep_mode);

rt_err_t nbiot_enable_echo(int enable)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    // disable at echo
    result = at_obj_exec_cmd(client, resp, enable ? "ATE1" : "ATE0");
    if (result != RT_EOK) {
        log_error("disable at echo failed: %d", result);
    }

    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_check_lwm2m_config(lwm2m_config_t config)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    // check production info
    char pk[64] = {0};
    char ps[64] = {0};
    char ver[64] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"productinfo\"");
    if (result != RT_EOK) {
        log_error("check production info failed: %d", result);
        goto __ERROR__;
    }
    if (at_resp_parse_line_args(resp, 2, "+QIOTCFG: \"productinfo\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", pk, ps, ver) <= 0) {
        result = RT_ERROR;
        goto __ERROR__;
    }
    log_debug("pk: %s; ps: %s; ver: %s", pk, ps, ver);
    if (rt_strcmp(pk, config->pk) != 0) {
        log_warn("productinfo pk not equal to configure.");
        result = RT_ERROR;
        goto __ERROR__;
    }

    // check server
    int server_type = -1;
    char server_URL[128] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"server\"");
    if (result != RT_EOK) {
        log_error("check server failed: %d", result);
        goto __ERROR__;
    }
    if (at_resp_parse_line_args(resp, 2, "+QIOTCFG: \"server\",%d,\"%[^\"]\"", &server_type, server_URL) <= 0) {
        result = RT_ERROR;
        goto __ERROR__;
    }
    log_debug("server_type: %d; server_URL: %s", server_type, server_URL);
    if (rt_strcmp(server_URL, config->server_URL) != 0) {
        log_warn("productinfo server not equal to configure.");
        result = RT_ERROR;
        goto __ERROR__;
    }

    // check lifetime
    int lifetime = -1;
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"lifetime\"");
    if (result != RT_EOK) {
        log_error("check lifetime failed: %d", result);
        goto __ERROR__;
    }
    if (at_resp_parse_line_args(resp, 2, "+QIOTCFG: \"lifetime\",%d", &lifetime) <= 0) {
        result = RT_ERROR;
        goto __ERROR__;
    }
    log_debug("lifetime: %d", lifetime);
    if (lifetime != config->lifetime) {
        log_warn("productinfo liftime not equal to configure.");
        result = RT_ERROR;
        goto __ERROR__;
    }

__ERROR__:
    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_set_lwm2m_config(lwm2m_config_t config)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    // set productinfo
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"productinfo\",\"%s\",\"%s\"", config->pk, config->ps);
    if (result != RT_EOK) {
        log_error("set productinfo failed: %d", result);
        goto FINAL;
    }
    log_debug("set productinfo success");

    // set server
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"server\",%d,\"%s\"", config->server_type, config->server_URL);
    if (result != RT_EOK) {
        log_error("set server failed: %d", result);
        goto FINAL;
    }
    log_debug("set server success");

    // set lifetime
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"lifetime\",%d", config->lifetime);
    if (result != RT_EOK) {
        log_error("set lifetime failed: %d", result);
        goto FINAL;
    }
    log_debug("set lifetime success");

    // set buffer
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"buffer\",%d", config->buffer_mode);
    if (result != RT_EOK) {
        log_error("set buffer failed: %d", result);
        goto FINAL;
    }
    log_debug("set buffer success");

    // set act
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"act\",%d", config->context_id);
    if (result != RT_EOK) {
        log_error("set act failed: %d", result);
        goto FINAL;
    }
    log_debug("set act success");

    // set tsl
    result = at_obj_exec_cmd(client, resp, "AT+QIOTCFG=\"tsl\",%d", config->tsl_mode);
    if (result != RT_EOK) {
        log_error("set tsl failed: %d", result);
        goto FINAL;
    }
    log_debug("set tsl success");

FINAL:
    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_set_cfun_mode(int mode)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    char s[20] = {0};
    snprintf(s, 20, "AT+CFUN=%d", mode);
    result = at_obj_exec_cmd(client, resp, s);
    if (result != RT_EOK) {
        log_error("nbiot cfun err: %d", result);
    }

    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_check_network(int retry_times)
{
    int n = -1;
    int stat = -1;
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    for (int i=0; i < retry_times; i++) {
        result = at_obj_exec_cmd(client, resp, "AT+CEREG?");
        at_resp_parse_line_args(resp, 2, "+CEREG: %d,%d", &n, &stat);
        log_debug("check status, n: %d, stat: %d", n, stat);
        if (stat == 1 || stat == 5) {
            at_delete_resp(resp);
            return RT_EOK;
        }
        rt_thread_mdelay(5000);
    }

    at_delete_resp(resp);
    return RT_ERROR;
}


rt_err_t nbiot_set_network_config(network_config_t config)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_debug("create resp failed.");
        return RT_ERROR;
    }

    char PDP_type[12] = {0};
    char APN[99] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+QCGDEFCONT?");
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+QCGDEFCONT=? failed");
        goto __ERROR__;
    }
    if (at_resp_parse_line_args(resp, 2, "+QCGDEFCONT: \"%[^,\"]\",\"%[^,\"]\",", PDP_type, APN) <= 0)
    {
        log_debug("at_resp_parse_line_args AT+QCGDEFCONT? failed");
        goto __ERROR__;
    }
    log_debug("read PDP_type: %s", PDP_type);
    log_debug("read APN: %s", APN);
    if (rt_strcmp(APN, config->apn) != 0) {
        result = at_obj_exec_cmd(client, resp, "AT+QCGDEFCONT=\"IP\",\"%s\"", config->apn);
        if (result != RT_EOK) {
            log_error("set apn result: %d", result);
            goto __ERROR__;
        }
        log_debug("set apn success");
    }

    char band_info[64] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+QBAND?");
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+QBAND? failed");
        goto __ERROR__;
    }
    if (at_resp_parse_line_args(resp, 2, "+QBAND: %s", band_info) <= 0) {
        log_debug("at_resp_parse_line_args AT+AT+QBAND? failed");
        goto __ERROR__;
    }
    log_debug("read band_info: %s", band_info);
    if (rt_strcmp(band_info, config->band + 2) != 0) {
        result = at_obj_exec_cmd(client, resp, "AT+QBAND=%s", config->band);
        if (result != RT_EOK) {
            log_error("nbiot set band result: %d", result);
            goto __ERROR__;
        }
        log_debug("set band success");
    }

    char cipca_info[64] = {0};
    result = at_obj_exec_cmd(client, resp, "AT+CIPCA?");
    if (result != RT_EOK) {
        log_debug("at_obj_exec_cmd AT+CIPCA? failed");
        goto __ERROR__;
    }
    if (at_resp_parse_line_args(resp, 2, "+CIPCA: %s", cipca_info) <= 0) {
        log_debug("at_resp_parse_line_args AT+AT+CIPCA? failed");
        goto __ERROR__;
    }
    if (rt_strcmp(cipca_info, config->cipca) != 0) {
        result = at_obj_exec_cmd(client, resp, "AT+CIPCA=%s", config->cipca);
        if (result != RT_EOK) {
            log_error("nbiot set cipca result: %d", result);
            goto __ERROR__;
        }
        log_debug("set cipca success");
    }

__ERROR__:
    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_lwm2m_register()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QIOTREG=1");
    if (result != RT_EOK) {
        log_error("send qiotreg failed: %d", result);
        at_delete_resp(resp);
        return result;
    }
    log_debug("send qiotreg=1 success");

    rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
    QIOT_SUBSCRIBE_EVENT_CODE = -1;
    rt_mutex_release(&qiot_event_mutex);
    for (int retry_times=0; retry_times < 10; retry_times++) {
        rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
        if (QIOT_SUBSCRIBE_EVENT_CODE == 10200) {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            log_debug("wait 3,10200 success, lwm2m has connect ready");
            return RT_EOK;
        }
        rt_mutex_release(&qiot_event_mutex);
        rt_thread_mdelay(5000);
    }

    log_debug("wait 3,10200 failed, lwm2m connect failed");
    at_delete_resp(resp);
    return RT_ERROR;
}
// MSH_CMD_EXPORT(nbiot_lwm2m_register, nbiot_lwm2m_register);

rt_err_t nbiot_lwm2m_deregister()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QIOTREG=0");
    if (result != RT_EOK) {
        log_error("send qiotreg failed: %d", result);
        at_delete_resp(resp);
        return result;
    }
    log_debug("send qiotreg=0 success");

    at_delete_resp(resp);
    return result;
}
// MSH_CMD_EXPORT(nbiot_lwm2m_deregister, nbiot_lwm2m_deregister);

rt_err_t nbiot_check_qiotstate(int retry_times)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }
    
    while (retry_times) {
        int state = -1;
        result = at_obj_exec_cmd(client, resp, "AT+QIOTSTATE?");
        if (result == RT_EOK) {
            if (at_resp_parse_line_args(resp, 2, "+QIOTSTATE: %d", &state) > 0) {
                log_debug("get qiotstate state: %d", state);
                if (state == 8) {
                    at_delete_resp(resp);
                    return RT_EOK;
                }
            }
        }
        retry_times--;
        rt_thread_mdelay(1000);
    }
    at_delete_resp(resp);  // after at_resp_parse_line_args, delete resp will cause dump
    return RT_ERROR;
}
// MSH_CMD_EXPORT(nbiot_check_qiotstate, nbiot_check_qiotstate);

rt_err_t nbiot_report_model_data(const char *data, rt_size_t length)
{
    log_debug("report_model_data: %s; length: %d\n", data, length);
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 2, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QIOTMODELTD=100,%d", length);
    if (result != RT_EOK) {
        log_error("qiotmodeltd set failed: %d\n", result);
        at_delete_resp(resp);
        return RT_ERROR;
    }
    rt_thread_mdelay(500);  // After the > response, it is recommended for the MCU to wait for 500 ms before sending the data.

    rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
    QIOT_DATA_SEND_EVENT_CODE = -1;
    rt_mutex_release(&qiot_event_mutex);
    at_client_obj_send(client, data, length);
    at_client_obj_send(client, "\x1A", 1);

    for (int i=0; i < 30; i++) {
        rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
        if (QIOT_DATA_SEND_EVENT_CODE == -1) {
            rt_mutex_release(&qiot_event_mutex);
            rt_thread_mdelay(1000);
            continue;
        }
        if (QIOT_DATA_SEND_EVENT_CODE == 10210) {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            return RT_EOK;
        }
        else {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            return RT_ERROR;
        }
    }
    at_delete_resp(resp);
    return RT_ERROR;
}

rt_err_t nbiot_report_ctrl_data(const char *data, rt_size_t length)
{
    log_debug("report_ctrl_data: %s; length: %d\n", data, length);
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 2, rt_tick_from_millisecond(5000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QIOTMODELTD=100,%d", length);
    if (result != RT_EOK) {
        log_error("qiotmodeltd set failed: %d\n", result);
        at_delete_resp(resp);
        return RT_ERROR;
    }
    rt_thread_mdelay(500);  // After the > response, it is recommended for the MCU to wait for 500 ms before sending the data.

    rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
    QIOT_DATA_SEND_EVENT_CODE = -1;
    rt_mutex_release(&qiot_event_mutex);
    at_client_obj_send(client, data, length);
    at_client_obj_send(client, "\x1A", 1);

    for (int i=0; i < 30; i++) {
        rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
        if (QIOT_DATA_SEND_EVENT_CODE == -1) {
            rt_mutex_release(&qiot_event_mutex);
            rt_thread_mdelay(1000);
            continue;
        }
        if (QIOT_DATA_SEND_EVENT_CODE == 10210) {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            return RT_EOK;
        }
        else {
            rt_mutex_release(&qiot_event_mutex);
            at_delete_resp(resp);
            return RT_ERROR;
        }
    }
    at_delete_resp(resp);
    return RT_ERROR;
}

rt_err_t nbiot_recv_ctrl_data(int req_length, struct ServerCtrlData *server_ctrl_data_ptr)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 2, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    int retry_times = 0;
    cJSON *temp;
    while (1) {
        int cur_len = 0;
        int remain_len = 0;
        int remain_pieces = 0;

        result = at_obj_exec_cmd(client, resp, "AT+QIOTMODELRD=%d", req_length);
        if (result != RT_EOK) {
            log_error("qiotmodelrd set failed: %d\n", result);
            break;
        }

        int result = at_resp_parse_line_args(resp, 1, "+QIOTMODELRD: %d,%d,%d", &cur_len, &remain_len, &remain_pieces);
        log_debug("at_resp_parse_line_args result: %d", result);
        if (result > 0) {
            log_debug("cur_len: %d, remain_len: %d, remain_pieces: %d", cur_len, remain_len, remain_pieces);
            if (cur_len <= 0 && remain_pieces <= 0) {
                if (retry_times >= 10) {
                    break;
                }
            }
            const char *data = at_resp_get_line(resp, 2);
            log_debug("recv ctrl data from server: %s", data);
            cJSON *root = cJSON_Parse(data);
            if (root != NULL) {
                cJSON *cat1_collect_interval_item = cJSON_GetObjectItem(root, "12");
                if (cat1_collect_interval_item != NULL) {
                    log_debug("key: %s, value: %d", cat1_collect_interval_item->string, cat1_collect_interval_item->valueint);
                    server_ctrl_data_ptr->Cat1_CollectInterval = cat1_collect_interval_item->valueint;
                }
                cJSON *nb_collect_interval_item = cJSON_GetObjectItem(root, "24");
                if (nb_collect_interval_item != NULL) {
                    log_debug("key: %s, value: %d", nb_collect_interval_item->string, nb_collect_interval_item->valueint);
                    server_ctrl_data_ptr->NB_CollectInterval = nb_collect_interval_item->valueint;
                }
                cJSON *esp32_ap_switch_item = cJSON_GetObjectItem(root, "21");
                if (esp32_ap_switch_item != NULL) {
                    log_debug("key: %s, value: %d", esp32_ap_switch_item->string, esp32_ap_switch_item->valueint);
                    server_ctrl_data_ptr->Esp32_AP_Switch = esp32_ap_switch_item->valueint;
                }
                cJSON *cat1_file_upload = cJSON_GetObjectItem(root, "13");
                if (cat1_file_upload != NULL) {
                    temp = cJSON_GetObjectItem(cat1_file_upload, "1");
                    if (temp) {
                        log_debug("key: %s, value: %s", temp->string, temp->valuestring);
                        strcat(server_ctrl_data_ptr->Cat1_File_Upload_File_Times, temp->valuestring);
                    }
                    temp = cJSON_GetObjectItem(cat1_file_upload, "2");
                    if (temp) {
                        log_debug("key: %s, value: %d", temp->string, temp->valueint);
                        server_ctrl_data_ptr->Cat1_File_Upload_File_Type = temp->valueint;
                    }
                    temp = cJSON_GetObjectItem(cat1_file_upload, "3");
                    if (temp) {
                        log_debug("key: %s, value: %d", temp->string, temp->valueint);
                        server_ctrl_data_ptr->Cat1_File_Upload_Switch = temp->valueint;
                    }
                }
                cJSON_Delete(root);
            }
        }

        retry_times += 1;
        log_debug("nbiot_recv_ctrl_data restry_times: %d", retry_times);
        rt_thread_mdelay(1000);
    }

    at_delete_resp(resp);
    return RT_ERROR;
}

rt_err_t nbiot_get_current_datetime(datetime_t dt)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 1, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+CCLK?");
    if (result != RT_EOK) {
        log_error("get current datetime failed: %d\n", result);
        at_delete_resp(resp);
        return result;
    }

    char data[64] = {0};
    if(rt_mq_recv(cclk_data_urc_queue, data, 64, rt_tick_from_millisecond(3000)) > 0) {
        log_debug("recv cclk data: %s", data);
        sscanf(data, "+CCLK: %d/%d/%d,%d:%d:%d+%d", &(dt->year), &(dt->month), &(dt->day), &(dt->hour), &(dt->minute), &(dt->second), &(dt->zone));
        at_delete_resp(resp);
        return RT_EOK;
    }

    at_delete_resp(resp);
    return RT_ERROR;
}

rt_err_t nbiot_set_qiotlocext(char *nmea_string)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 1, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
    QIOT_DATA_RECV_EVENT_CODE = -1;
    rt_mutex_release(&qiot_event_mutex);

    result = at_obj_exec_cmd(client, resp, "AT+QIOTLOCEXT=\"%s\"", nmea_string);
    log_debug("AT+QIOTLOCEXT send nmea cmd result: %d", result);
    if (result == RT_EOK) {
        for (int i=0; i < 30; i++) {
            rt_mutex_take(&qiot_event_mutex, RT_WAITING_FOREVER);
            if (QIOT_DATA_RECV_EVENT_CODE == -1) {
                rt_mutex_release(&qiot_event_mutex);
                rt_thread_mdelay(1000);
                continue;
            }
            if (QIOT_DATA_RECV_EVENT_CODE == 10220) {
                rt_mutex_release(&qiot_event_mutex);
                at_delete_resp(resp);
                return RT_EOK;
            }
            else {
                rt_mutex_release(&qiot_event_mutex);
                at_delete_resp(resp);
                return RT_ERROR;
            }
            rt_thread_mdelay(1000);
        }
    }
    log_error("set qiotlocext result: %d\n", result);
    at_delete_resp(resp);
    return result;
}


// AT+CGSN=1
rt_err_t get_nbiot_imei(char *output)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;
    char imei[64] = {0};

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+CGSN=1");
    if (result == RT_EOK) {
        const char *resp_line = at_resp_get_line(resp, 2);
        if (resp_line == RT_NULL) {
            result = RT_ERROR;
            goto __ERROR__;
        }
        memcpy(output, resp_line + 7, strlen(resp_line + 7));
        output[15] = '\0';
    }

__ERROR__:
    at_delete_resp(resp);
    return result;
}


// AT+CSQ
rt_err_t get_nbiot_csq(int *rssi, int *ber)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+CSQ");
    if (result == RT_EOK) {
        const char *resp_line = at_resp_get_line(resp, 2);
        if (resp_line == RT_NULL) {
            result = RT_ERROR;
            goto __ERROR__;
        }
        sscanf(resp_line, "+CSQ: %d,%d", rssi, ber);
    }

__ERROR__:
    at_delete_resp(resp);
    return result;
}


rt_err_t check_and_set_macrai()
{
    // AT+CFUN=0
    // AT+QCFG="MacRAI",1
    // AT+CNMPSD
    // AT+QR14FEATURE
    // AT+CFUN=1

    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    int macrai = 0;
    result = at_obj_exec_cmd(client, resp, "AT+QCFG=\"MacRAI\"");
    if (result == RT_EOK) {
        const char *resp_line = at_resp_get_line(resp, 2);
        if (resp_line == RT_NULL) {
            result = RT_ERROR;
            goto __ERROR__;
        }
        sscanf(resp_line, "+QCFG: \"MacRAI\",%d", &macrai);
    }

    if (macrai == 0) {
        at_obj_exec_cmd(client, resp, "AT+CFUN=0");
        rt_thread_mdelay(200);
        at_obj_exec_cmd(client, resp, "AT+QCFG=\"MacRAI\",1");
        at_obj_exec_cmd(client, resp, "AT+CNMPSD");
        at_obj_exec_cmd(client, resp, "AT+QR14FEATURE");
        at_obj_exec_cmd(client, resp, "AT+CFUN=1");
    }

__ERROR__:
    at_delete_resp(resp);
    return result;
}


rt_err_t check_and_set_psms()
{
    // AT+QPSMS=2,36000

    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    int nt1 = -1;
    int nt2 = -1;
    result = at_obj_exec_cmd(client, resp, "AT+QPSMS?");
    if (result == RT_EOK) {
        const char *resp_line = at_resp_get_line(resp, 2);
        if (resp_line == RT_NULL) {
            result = RT_ERROR;
            goto __ERROR__;
        }
        sscanf(resp_line, "+QPSMS: %d,%d", &nt1, &nt2);
    }

    if (nt1 != 2 || nt2 != 36000) {
        at_obj_exec_cmd(client, resp, "AT+QPSMS=10,36000");
    }

__ERROR__:
    at_delete_resp(resp);
    return result;
}


rt_err_t nbiot_report_component_version(char *component, char *version)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;
    at_response_t resp = RT_NULL;
    char mcu_ver[512] = {0};
    int mcu_ver_len = 0;
    
    rt_memset(mcu_ver, 0, sizeof(mcu_ver));

    mcu_ver_len += rt_snprintf(mcu_ver+mcu_ver_len,sizeof(mcu_ver)-mcu_ver_len, "AT+QIOTMCUVER=\"%s\",\"%s\"", component, version);
    
    log_debug("MCU VER,len%d,ver:%s", mcu_ver_len, mcu_ver);

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }
    resp = at_create_resp(128, 0, rt_tick_from_millisecond(1000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, mcu_ver);
    if (result != RT_EOK) {
        result = RT_ERROR;
    }   
    at_delete_resp(resp);
    return result;
}

rt_err_t nbiot_config_mcu_version(void)
{
    rt_err_t result = RT_EOK;

    uint8_t *app_version = NULL, *app_subedition, *app_build_time;
    char esp32_version[64];
    char cat1_version[64];
    char *gnss_version = NULL;
    
	read_app_version_information(&app_version, &app_subedition, &app_build_time);
    if(app_version != RT_NULL)
    {
        log_debug("stm32 version:%s", app_version);
        nbiot_report_component_version("STM32-A", app_version);
        nbiot_report_component_version("STM32-B", app_version);
    }
    rt_memset(cat1_version, 0, sizeof(cat1_version));
    cat1_at_query_version(cat1_version, sizeof(cat1_version));
    if(rt_strlen(cat1_version) > 0)
    {
        log_debug("CAT1 version:%s", cat1_version);
        nbiot_report_component_version("CAT1", cat1_version);
    }
    rt_memset(esp32_version, 0, sizeof(esp32_version));
    esp32_at_query_version(esp32_version, sizeof(esp32_version));
    if(rt_strlen(esp32_version) > 0)
    {
        log_debug("ESP32 version:%s", esp32_version);
        nbiot_report_component_version("ESP32", esp32_version);
    }

    result = gnss_open();
    rt_thread_mdelay(100);
    gnss_query_version(&gnss_version);
    if(gnss_version != NULL)
    {
        nbiot_report_component_version("GNSS-P0", gnss_version);
        nbiot_report_component_version("GNSS-P1", gnss_version);
        nbiot_report_component_version("GNSS-P2", gnss_version);
        nbiot_report_component_version("GNSS-P3", gnss_version);
        nbiot_report_component_version("GNSS-P4", gnss_version);
    }
    result = gnss_close();

    return result;
}



rt_err_t nbiot_ota_req(void)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;
    at_response_t resp = RT_NULL;

    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }
    resp = at_create_resp(512, 0, rt_tick_from_millisecond(1000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+QIOTOTAREQ=0");
    if (result != RT_EOK) {
        result = RT_ERROR;
    }

    at_delete_resp(resp);
    return result;
}

/**
 * @brief 
 * 
 * @param [in] action 
 * 0: Reject upgrade
 * 1: Confirm upgrade
 * 2: MCU requests to download the next firmware block,
 * 3: MCU reports updating status
 * @return rt_err_t 
 */
rt_err_t nbiot_ota_update_action(int action)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;
    at_response_t resp = RT_NULL;
    char at_cmd[128] = {0};

    if(action < 0 || action > 3)
    {
        return RT_ERROR;
    }
    rt_memset(at_cmd, 0, sizeof(at_cmd));
    rt_snprintf(at_cmd, sizeof(at_cmd)-1, "AT+QIOTUPDATE=%d", action);
    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        return RT_ERROR;
    }
    resp = at_create_resp(128, 0, rt_tick_from_millisecond(1000));
    if (resp == RT_NULL) {
        log_error("create resp failed.");
        return RT_ERROR;
    }

    result = at_obj_exec_cmd(client, resp, at_cmd);
    if (result != RT_EOK) {
        result = RT_ERROR;
    }

    at_delete_resp(resp);
    return result;
}

ota_task_state_t nbiot_get_ota_task_state(void)
{
    int state = OTA_TASK_STATE_NONE;
    ota_task_t *ctl = &g_ota_task;
    if(access(NBIOT_OTA_TASK_CONFIG_FILE, F_OK) != 0)
    {
        return state;
    }

    state = ctl->state;
    return state;
}



/**
 * @brief 
 * 
 * @return int 
 *  -- 0  ALL firmware download completed
 *  -- 1  Currrent firmware download completed, neet to dowload the next firmware component
 *  -- 2  Firmware fragment download completed, need to request the next firmware fragment 
 *  -- 3  Need to continue reading data
 *  -- -1 save data error 
 */
int nbiot_save_ota_data(void)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;
    char at_cmd[128] = {0};
    const char *resp_line = RT_NULL;
    int data_offset = 0;
    int data_size = 0;
    ota_task_t *ctl = &g_ota_task;
    UpgradeNode ota_node = {0};
    UpgradePlan plan = {0};
    static nbiot_ota_data_msg_t recv_msg = {0};
    int ret = 0;

    uint32_t offset = 0;

    if(ctl->type == FIRM_TYPE_MODULE)
    {
        // log_debug("module firmware ota, no need downlaod to mcu");
        return 0;
    }

    if(ctl->state == OTA_TASK_STATE_UPGRADEING)
    {
        log_debug("in upgrading state");
        return 0;
    }
    else if(ctl->state == OTA_TASK_STATE_DOWNLOADING)
    {
        log_debug("in downloading state");
        return 1;
    }
    
    offset = ctl->compoment[ctl->current_copmment].startaddr+ctl->compoment[ctl->current_copmment].downloaded_size;

    rt_memset(at_cmd, 0, sizeof(at_cmd));
    rt_snprintf(at_cmd, sizeof(at_cmd)-1, "AT+QIOTOTARD=%d,%d", offset, NBIOT_OTA_DATA_MSG_BUFFER_LENGTH-50);
    client = at_client_get(NBIOT_AT_UART_NAME);
    if (client == RT_NULL) {
        log_error("nbiot at client not inited!");
        result =  -1;
        goto EXIT;
    }

    if(ota_data_urc_queue == NULL)
    {
        log_error("ota_data_urc_queue not inited!");
        result =  -1;
        goto EXIT;
    }


    result = at_obj_exec_cmd(client, NULL, at_cmd);
    if (result != RT_EOK) {
        result =  -1;
        goto EXIT;
    }
    else
    {
 
        rt_memset(&recv_msg, 0, sizeof(nbiot_ota_data_msg_t));
        ret = rt_mq_recv(ota_data_urc_queue, &recv_msg, sizeof(nbiot_ota_data_msg_t), RT_WAITING_FOREVER);
        log_info("rt_mq_recv ret %d\n", ret);
        if(ret > 0)
        {   
            if(ctl->compoment[ctl->current_copmment].fd == RT_NULL)
            {
                log_error("flie fd is null");
                result =  -1;
                goto EXIT;
            }
            
            if(recv_msg.data_offset == offset && recv_msg.data_size > 0)
            {
                fseek(ctl->compoment[ctl->current_copmment].fd, offset, SEEK_SET);
                size_t written = fwrite(recv_msg.data, 1, recv_msg.data_size, ctl->compoment[ctl->current_copmment].fd);
                if(written == recv_msg.data_size)
                {
                    ctl->compoment[ctl->current_copmment].downloaded_size += recv_msg.data_size;
                    if(ctl->compoment[ctl->current_copmment].downloaded_size == ctl->compoment[ctl->current_copmment].piece_length)
                    {
                        // 文件分片下载完成
                        if(ctl->compoment[ctl->current_copmment].downloaded_size+ctl->compoment[ctl->current_copmment].startaddr == ctl->compoment[ctl->current_copmment].compomentSize)
                        {
                            if(ctl->compoment_downlaoded_cnt == (ctl->compoment_cnt - 1))
                            {
                                // 整个文件下载完成
                                
                                fclose(ctl->compoment[ctl->current_copmment].fd);
                                ctl->compoment[ctl->current_copmment].fd = RT_NULL;

                                rt_memset(&plan, 0, sizeof(UpgradePlan));
                                plan.file_cnt = ctl->compoment_cnt;

                                rt_strcpy(plan.target_version, ctl->compoment[ctl->current_copmment].target_version);
                                for (size_t i = 0; i < ctl->compoment_cnt; i++)
                                {
                                    rt_strcpy(plan.file[i].file_name,  ctl->compoment[i].fileName);
                                    log_debug("file:%s", plan.file[i].file_name);
                                    rt_memcpy(plan.file[i].file_md5, ctl->compoment[i].md5Bytes, 16);
                                    log_debug(
                                        "md5 %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                                        plan.file[i].file_md5[0], plan.file[i].file_md5[1], plan.file[i].file_md5[2], plan.file[i].file_md5[3],
                                        plan.file[i].file_md5[4], plan.file[i].file_md5[5], plan.file[i].file_md5[6], plan.file[i].file_md5[7],
                                        plan.file[i].file_md5[8], plan.file[i].file_md5[9], plan.file[i].file_md5[10], plan.file[i].file_md5[11],
                                        plan.file[i].file_md5[12], plan.file[i].file_md5[13], plan.file[i].file_md5[14], plan.file[i].file_md5[15]
                                    );
                                }
                                log_debug("ctl->module=%d", ctl->module);
                                set_module(ctl->module, &plan);
                                get_module(ctl->module, &ota_node);
                                log_debug("ota_node.status=%d", ota_node.status);
                                log_debug("ota_node.plan.file_cnt=%d", ota_node.plan.file_cnt);
                                log_debug("ota_node.plan.file[0].file_name=%s", ota_node.plan.file[0].file_name);
                                log_debug("ota_node.ops.prepare=0x%08X", ota_node.ops.prepare);
                                log_debug("ota_node.ops.apply=0x%08X", ota_node.ops.apply);
                                log_debug("ota_node.ops.finish=0x%08X", ota_node.ops.finish);
                                ota_node.status = UPGRADE_STATUS_DOWNLOADED;
                                log_debug("ota_node.status=%d", ota_node.status);
                                save_module(&ota_node);
                                ctl->state = OTA_TASK_STATE_UPGRADEING;
                                save_ota_task_to_file(ctl);

                                nbiot_ota_update_action(3);
                                result = 0;
                                goto EXIT;
                            }
                            else
                            {
                                // 下载下一个文件
                                fclose(ctl->compoment[ctl->current_copmment].fd);
                                ctl->compoment[ctl->current_copmment].fd = RT_NULL;
                                ctl->compoment[ctl->current_copmment].piece_length = 0;
                                ctl->compoment[ctl->current_copmment].downloaded_size = 0;
                                ctl->compoment_downlaoded_cnt++;
                                save_ota_task_to_file(ctl);
                                nbiot_ota_update_action(3);
                                // 上报组件新版本才能下载下一个组件
                                nbiot_report_component_version(ctl->compoment[ctl->current_copmment].name,ctl->compoment[ctl->current_copmment].target_version);
                                ctl->state = OTA_TASK_STATE_DOWNLOADING;
                                save_ota_task_to_file(ctl);
                                result = 1;
                                goto EXIT;
                            }
                            
                        }
                        
                        // 需要下载下一个片段
                        ctl->compoment[ctl->current_copmment].piece_length = 0;
                        ctl->compoment[ctl->current_copmment].downloaded_size = 0;
                        fclose(ctl->compoment[ctl->current_copmment].fd);
                        ctl->compoment[ctl->current_copmment].fd = RT_NULL;
                        nbiot_ota_update_action(2);
                        ctl->state = OTA_TASK_STATE_DOWNLOADING;
                        save_ota_task_to_file(ctl);
                        result = 2;
                        goto EXIT;
                    }

                    // 持续读取固件数据
                    result = 3;
                    goto EXIT;
                }
            }
        }
        else
        {
            result =  -1;
            goto EXIT;
        }
    }

EXIT:
    
    return (int)result;

}

void nbiot_clean_ota_task(void)
{
    ota_task_t *ctl = &g_ota_task;
    ctl->state = OTA_TASK_STATE_NONE;
    clean_ota_task(ctl);
}