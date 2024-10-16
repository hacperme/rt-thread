/*
 * @FilePath: ota_app.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-11 20:49:20
 * @copyright : Copyright (c) 2024
 */
#ifndef __OTA_APP_H__
#define __OTA_APP_H__

#include <rtthread.h>
#include <board.h>

typedef enum {
    OTA_NO = 0,
    OTA_YES = 1,
} ota_tag_e;

typedef enum {
    OTA_SUCCESS = 0,
    OTA_FILE_NAME_NOT_EXISTS = 1,
    OTA_FILE_NOT_EXISTS = 2,
    OTA_ERASE_FAILED = 3,
    OTA_WRITE_FAILED = 4,
    OTA_JUMP_APP_FAILED = 5,
    OTA_WAITING = 6,
    OTA_FS_INIT_FAILED = 7,
    OTA_DEFAULT = 0xFF
} ota_state_e;

typedef enum {
    APP_A_PART = 0,
    APP_B_PART = 1
} app_parition_e;

struct mbr {
    rt_uint8_t ota_tag;  			// 0 - Not OTA; 1 - Do OTA.
    rt_uint8_t ota_state;
    rt_uint8_t app_part;  			// 0 - app0 partition; 1 - app1 partition.
    char ota_file[255];
};                              // 该结构体单独放在 onchip flash data 分区中存储.
typedef struct mbr *mbr_t;

mbr_t mbr_init(void);
rt_err_t mbr_save(void);

void ota_app_status_save(ota_tag_e ota_tag_val, ota_state_e ota_proc_state, app_parition_e app_part_no);
void ota_app_over(ota_tag_e ota_tag_val, ota_state_e ota_proc_state, app_parition_e app_part_no);
void ota_app_process(void);

#endif