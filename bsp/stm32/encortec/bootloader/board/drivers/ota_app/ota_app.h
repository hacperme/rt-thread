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
    OTA_SUCCESS = 0xF0,
    OTA_FILE_NAME_NOT_EXISTS = 0xF1,
    OTA_FILE_NOT_EXISTS = 0xF2,
    OTA_ERASE_WRITE_FAILED = 0xF3,
    OTA_JUMP_APP_FAILED = 0xF4,
    OTA_WAITING = 0xF5,
    OTA_FS_INIT_FAILED = 0xF6,
    OTA_DEFAULT = 0xFF
} ota_process_e;

typedef enum {
    APP_A_PART = 0,
    APP_B_PART = 1
} app_parition_e;

struct ota_data {
    char ota_tag;  				// 0 - Not OTA; 1 - Do OTA.
    char ota_process;  			// 0 ~ 95 升级page编号; 0xF0 - 成功; 0xF1 - ota 文件名不存在; 0xF2 - ota 文件不存在; 0xF3 - 擦写失败; 0xF4 - 跳转失败， 0xFA - 待升级; 0xFF 默认值无升级.
    char app_part;  			// 0 - app0 partition; 1 - app1 partition.
    char ota_file[255];
};                              // 该结构体单独放在 onchip flash data 分区中存储.
typedef struct ota_data *ota_data_t;

#endif