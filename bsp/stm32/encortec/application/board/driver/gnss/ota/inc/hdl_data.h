#ifndef __HDL_DATA_H__
#define __HDL_DATA_H__

#include "hdl_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*hdl_init_cb)(void *usr_arg);
typedef void (*hdl_da_send_cb)(void *usr_arg, uint32_t sent_bytes, uint32_t da_total_bytes);
typedef void (*hdl_download_cb)(void *usr_arg, char *cur_image_name, uint32_t sent_bytes, uint32_t total_bytes);
typedef void (*hdl_progress_cb)(void *usr_arg, uint8_t percent);
typedef void (*hdl_readback_cb)(void *usr_arg, char *cur_image_name, uint32_t read_len, uint32_t readback_total_len);

typedef struct {
    // uint32_t        da_flash_addr; // Or da_file if your host have file system
    char            *da_file;
    uint32_t         da_run_addr;
    uint32_t         da_len;
} hdl_da_info_t;

typedef struct {
    hdl_init_cb      conn_da_init_cb;
    void            *conn_da_init_cb_arg;
    hdl_da_send_cb   conn_da_send_cb;
    void            *conn_da_send_cb_arg;
} hdl_connect_arg_t;

typedef struct {
    uint16_t        hw_code;
    uint16_t        hw_subcode;
    uint16_t        flash_manufacturer_id;
    uint16_t        flash_id1;
    uint16_t        flash_id2;
    uint32_t        flash_base_addr;
    uint32_t        flash_size;
} hdl_da_report_t;

typedef struct {
    hdl_init_cb     format_init_cb;
    void           *format_init_cb_arg;
    hdl_progress_cb format_progress_cb;
    void           *format_progress_cb_arg;
    uint32_t        format_flash_addr;
    uint32_t        format_len;
} hdl_format_arg_t;

typedef struct _hdl_image_t {
    // uint32_t        image_host_flash_addr; // Or image_file if your host have file system
    char *          image_host_file;
    uint32_t        image_slave_flash_addr;
    uint32_t        image_len;
    char           *image_name;
    bool            image_is_bootloader;
    struct _hdl_image_t *next;
} hdl_image_t;

typedef struct {
    hdl_init_cb     download_init_cb;
    void           *download_init_cb_arg;
    hdl_download_cb download_cb;
    void           *download_cb_arg;
    hdl_image_t    *download_images;
} hdl_download_arg_t;

typedef struct {
    hdl_init_cb     readback_init_cb;
    void           *readback_init_cb_arg;
    hdl_readback_cb readback_cb;
    void           *readback_cb_arg;
    uint32_t        readback_flash_addr;
    uint32_t        readback_len;
    uint32_t        host_flash_addr;
    char           *image_name;
} hdl_readback_arg_t;

#ifdef __cplusplus
}
#endif

#endif //__HDL_DATA_H__

