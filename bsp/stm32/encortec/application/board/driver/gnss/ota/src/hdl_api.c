#include "hdl_api.h"
#include "hdl_brom_base.h"

#define HDL_HW_CODE_REG                 0x80000008
#define HDL_HW_SUBCODE_REG              0x8000000C

bool hdl_sync_with_da(hdl_da_report_t *da_report)
{
    bool ret = false;

    // Need DA init done
    hdl_delay(100);

/*************************************************************/

    uint32_t flash_base_addr = 0;
    ret = hdl_da_get_flash_address(&flash_base_addr);
    HDL_Require_Noerr_Action(ret, exit, "hdl_da_get_flash_address");

    uint32_t flash_size = 0;
    ret = hdl_da_get_flash_size(&flash_size);
    HDL_Require_Noerr_Action(ret, exit, "hdl_da_get_flash_size");

    uint8_t manufacturer_id = 0;
    uint8_t device_id1 = 0;
    uint8_t device_id2 = 0;
    ret = hdl_da_get_flash_id(&manufacturer_id, &device_id1, &device_id2);
    HDL_Require_Noerr_Action(ret, exit, "hdl_da_get_flash_id");

    // DA Report
    da_report->flash_manufacturer_id = manufacturer_id;
    da_report->flash_id1 = device_id1;
    da_report->flash_id2 = device_id2;
    da_report->flash_base_addr = flash_base_addr;
    da_report->flash_size = 0x001F8000; //The flash cannot be completely erased, the original flash size is 0x00200000.
    ret = true;

exit:
    return ret;
}

bool check_address_range(uint32_t address, uint32_t len, const hdl_da_report_t *da_report)
{
    const uint32_t base_addr = da_report->flash_base_addr;
    const uint32_t flash_size = da_report->flash_size;

    if(base_addr > address || (base_addr + flash_size) < address + len) {
        return false;
    }
    return true;
}


bool hdl_connect(const hdl_da_info_t *da_info, const hdl_connect_arg_t *connect_arg, hdl_da_report_t *da_report)
{
    if (da_info == NULL || connect_arg == NULL || da_report == NULL) {
        HDL_LOGE("hdl_connect fail, invalid parameter");
        return false;
    }

    // Init Channel & HandShake with Chip BROM via Brom_StartCMD
    bool success = hdl_channel_init();
    HDL_Require_Noerr_Action(success, exit, "hdl_channel_init");

    hdl_brom_start();

    // Brom Disable WDT
    success = hdl_brom_disable_wdt();
    HDL_Require_Noerr_Action(success, exit, "hdl_brom_disable_wdt");

#if defined (HDL_VIA_UART)
    // Set BTROM baudrate
    success = hdl_brom_set_baudrate(921600);
    HDL_Require_Noerr_Action(success, exit, "hdl_set_baudrate");

    // Set host baudrate
    HDL_COM_SetBaudRate(921600);
    hdl_delay(100);
#endif

    // Brom Send DA
    success = hdl_brom_send_da(connect_arg, da_info->da_flash_addr, da_info->da_run_addr, da_info->da_len);
    HDL_Require_Noerr_Action(success, exit, "hdl_brom_send_da");

    // Brom Jump to DA
    success = hdl_brom_jump_da(da_info->da_run_addr);
    HDL_Require_Noerr_Action(success, exit, "hdl_brom_jump_da");

    // DA Sync
    success = hdl_sync_with_da(da_report);
    HDL_MAIN_LOG("DA_Report Flash ID=0x%04X 0x%04X 0x%04X, Flash Base Addr=0x%08X, Flash Size=0x%08X",
                 da_report->flash_manufacturer_id,
                 da_report->flash_id1, da_report->flash_id2, da_report->flash_base_addr,
                 da_report->flash_size);

exit:
    return success;
}

bool hdl_format(const hdl_format_arg_t *format_arg, const hdl_da_report_t *da_report)
{
    if (format_arg == NULL && da_report == NULL) {
        return false;
    }

    // Format Init Callback
    if (format_arg != NULL && format_arg->format_init_cb != NULL) {
        format_arg->format_init_cb(format_arg->format_init_cb_arg);
    }

    if (!check_address_range(format_arg->format_flash_addr, format_arg->format_len, da_report))
    {    
        HDL_LOGE("hdl_format fail, para out of flash range: format_addr(0x%X)/format_len(0x%X)", format_arg->format_flash_addr, format_arg->format_len);
        return false;
    }
    
    return hdl_da_format(format_arg);
}

bool hdl_download(const hdl_download_arg_t *download_arg, const hdl_da_report_t *da_report)
{
    bool success = false;
    // Check Empty Parameter
    if (download_arg == NULL || download_arg->download_images == NULL) {
        HDL_LOGE("hdl_download fail, invalid parameter");
        return false;
    }

    // Check Image Flash_Addr & Size
    hdl_image_t *image = download_arg->download_images;
    while (image != NULL) {
        HDL_LOGI("Download Image (%s) BL=%d 0x%08X->0x%08X %d",
                 image->image_name, image->image_is_bootloader, image->image_host_flash_addr,
                 image->image_slave_flash_addr, image->image_len);

        if (!check_address_range(image->image_slave_flash_addr, image->image_len, da_report))
        {
            HDL_LOGE("hdl_download fail, Image (%s) out of flash range! flash_addr(0x%X)/len(0x%X)", image->image_name, image->image_slave_flash_addr, image->image_len);
            return false;
        }
        
        // image slave_flash_addr must be 4K aligned
        if (image->image_slave_flash_addr % DA_FW_PACKET_LEN != 0) {
            HDL_LOGE("hdl_download fail, Image (%s) Slave_Flash_Addr not 4K Align", image->image_name);
            return false;
        }
        
        // cur image_slave_flash_addr plus image_len must smaller than next image_slave_flash_addr
        hdl_image_t *next = image->next;
        if (next != NULL && image->image_slave_flash_addr+image->image_len > next->image_slave_flash_addr) {
            HDL_LOGE("hdl_download fail, Image (%s) and next Image (%s) overlaps",
                     image->image_name, next->image_name);
            return false;
        }
        image = image->next;
    }

    // Download init callback
    if (download_arg->download_init_cb != NULL) {
        download_arg->download_init_cb(download_arg->download_init_cb_arg);
    }

    // Download Every Image
    image = download_arg->download_images;
    while (image != NULL) {
        HDL_LOGI("Start download image(%s), addr(0x%x), len(0x%x)", image->image_name, image->image_slave_flash_addr, image->image_len);
        success = hdl_da_download(image, download_arg->download_cb, download_arg->download_cb_arg);
        HDL_Require_Noerr_Action(success, exit, "hdl_da_download");
        image = image->next;
    }

exit:
    return success;
}

bool hdl_readback(const hdl_readback_arg_t *readback_arg, const hdl_da_report_t *da_report)
{
    // Check Empty Parameter
    if (readback_arg == NULL || da_report == NULL) {
        HDL_LOGE("hdl_readback fail, invalid parameter");
        return false;
    }

    // Readback init callback
    if (readback_arg->readback_init_cb != NULL) {
        readback_arg->readback_init_cb(readback_arg->readback_init_cb_arg);
    }

    if (!check_address_range(readback_arg->readback_flash_addr, readback_arg->readback_len, da_report))
    {    
        HDL_LOGE("hdl_readback fail, Image(%s) out of flash range: readback_addr(0x%X)/readback_len(0x%X)", readback_arg->image_name, readback_arg->readback_flash_addr, readback_arg->readback_len);
        return false;
    }
    
    return hdl_da_readback(readback_arg);
}

bool hdl_disconnect_auto_reboot(bool enable)
{
    bool ret = hdl_finish_race(enable);

    HDL_COM_Deinit();
    
    return ret;
}

