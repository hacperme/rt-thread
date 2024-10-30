#include "hdl_demo.h"

void init_cb_demo(void *usr_arg)
{
    if (usr_arg != NULL && strlen((const char *)usr_arg) > 0) {
        HDL_MAIN_LOG("%s", usr_arg);
    }
}

void da_send_cb_demo(void *usr_arg, uint32_t sent_bytes, uint32_t da_total_bytes)
{
    uint8_t percent = sent_bytes * 100 / da_total_bytes;
    HDL_MAIN_LOG("DA Send Progress %d%% (%d/%d Bytes)", percent, sent_bytes, da_total_bytes);
}

void download_cb_demo(void *usr_arg, char *cur_image_name, uint32_t sent_bytes, uint32_t total_bytes)
{
    uint8_t percent = sent_bytes * 100 / total_bytes;
    HDL_MAIN_LOG("Image (%s) Download Progress %d%% (%d/%d Bytes)",
                 cur_image_name, percent, sent_bytes, total_bytes);
}

// Host Local Flash Read Buffer, only for Test Readback
void readback_cb_demo(void *usr_arg, char *cur_image_name,
                      uint32_t read_len, uint32_t readback_total_len)
{
    uint8_t percent = read_len * 100 / readback_total_len;
    HDL_MAIN_LOG("Readback (%s) Progress %d%% (%d/%d Bytes)", 
        cur_image_name, percent, read_len, readback_total_len);
}

void progress_cb_demo(void *usr_arg, uint8_t percent)
{
    HDL_MAIN_LOG("%s %d%%", (usr_arg != NULL ? usr_arg : "percent "), percent);
}


void hdl_host_dl_demo()
{
    // Init Connect ARG
    hdl_da_info_t da_info;
    da_info.da_flash_addr = HDL_DA_FLASH_POS;
    da_info.da_run_addr = HDL_DA_RUN_ADDR;
    da_info.da_len = HDL_DA_SIZE;
    hdl_connect_arg_t connect_arg;
    connect_arg.conn_da_init_cb = init_cb_demo;
    connect_arg.conn_da_init_cb_arg = "Download DA now...";
    connect_arg.conn_da_send_cb = da_send_cb_demo;
    connect_arg.conn_da_send_cb_arg = "";
    hdl_da_report_t da_report;
    memset((void *)&da_report, 0, sizeof(da_report));

    // Connect Device
    bool success = hdl_connect(&da_info, &connect_arg, &da_report);
    HDL_Require_Noerr_Action(success, exit, "hdl_connect");

    // Format Flash
    hdl_format_arg_t format_arg;
    format_arg.format_init_cb = init_cb_demo;
    format_arg.format_init_cb_arg = "Format Flash now...";
    format_arg.format_progress_cb = progress_cb_demo;
    format_arg.format_progress_cb_arg = "Format Progress";
    format_arg.format_flash_addr = da_report.flash_base_addr;
    format_arg.format_len = da_report.flash_size;
    success = hdl_format(&format_arg, &da_report);
    HDL_Require_Noerr_Action(success, exit, "hdl_format");

    // Download 4 Images
    hdl_image_t partition_image;
    hdl_image_t bl_image;
    hdl_image_t cm4_image;
    hdl_image_t config_image;
    
    partition_image.image_host_flash_addr = HDL_PARTITION_IMAGE_HOST_FLASH_POS;
    partition_image.image_slave_flash_addr = HDL_PARTITION_IMAGE_SLAVE_FLASH_POS;
    partition_image.image_len = HDL_PARTITION_IMAGE_SIZE;
    partition_image.image_name = HDL_PARTITION_IMAGE_NAME;
    partition_image.image_is_bootloader = false;
    partition_image.next = &bl_image;
    bl_image.image_host_flash_addr = HDL_BL_IMAGE_HOST_FLASH_POS;
    bl_image.image_slave_flash_addr = HDL_BL_IMAGE_SLAVE_FLASH_POS;
    bl_image.image_len = HDL_BL_IMAGE_SIZE;
    bl_image.image_name = HDL_BL_IMAGE_NAME;
    bl_image.image_is_bootloader = true;
    bl_image.next = &cm4_image;
    cm4_image.image_host_flash_addr = HDL_GNSS_DEMO_IMAGE_HOST_FLASH_POS;
    cm4_image.image_slave_flash_addr = HDL_GNSS_DEMO_IMAGE_SLAVE_FLASH_POS;
    cm4_image.image_len = HDL_GNSS_DEMO_IMAGE_SIZE;
    cm4_image.image_name = HDL_GNSS_DEMO_IMAGE_NAME;
    cm4_image.image_is_bootloader = false;
    cm4_image.next = &config_image;
    config_image.image_host_flash_addr = HDL_GNSS_CONFIG_IMAGE_HOST_FLASH_POS;
    config_image.image_slave_flash_addr = HDL_GNSS_CONFIG_IMAGE_SLAVE_FLASH_POS;
    config_image.image_len = HDL_GNSS_CONFIG_IMAGE_SIZE;
    config_image.image_name = HDL_GNSS_CONFIG_IMAGE_NAME;
    config_image.image_is_bootloader = false;
    config_image.next = NULL;

    hdl_download_arg_t download_arg;
    download_arg.download_init_cb = init_cb_demo;
    download_arg.download_init_cb_arg = "Download Images now...";
    download_arg.download_cb = download_cb_demo;
    download_arg.download_cb_arg = "";
    download_arg.download_images = &partition_image;
    success = hdl_download(&download_arg, &da_report);
    HDL_Require_Noerr_Action(success, exit, "hdl_download");

    // Disconnect Device
    success = hdl_disconnect_auto_reboot(true);
    HDL_Require_Noerr_Action(success, exit, "hdl_disconnect_auto_reboot");

exit:
    HDL_SUCCESS_LOG(__func__);
}

static void vHostDLTask(void *pvParameters)
{
    LOG_I(main, "hdl_host_dl_demo start");
    
    hdl_host_dl_demo();
    
    hdl_delete_task(NULL);
}

void hdl_main_entry()
{
    hdl_create_main_task(vHostDLTask);
}

