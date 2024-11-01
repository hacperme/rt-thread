#include "hdl_da_cmd.h"
#include "tools.h"

#define RACE_DA_GET_FLASH_ADDRESS   (0x210E)
#define RACE_DA_GET_FLASH_SIZE      (0x210F)
#define RACE_DA_GET_FLASH_ID        (0x2110)

#define RACE_DA_WRITE_BYTES         (0x2100)
#define RACE_DA_READ_BYTES          (0x2101)
#define RACE_DA_ERASE_BYTES         (0x2104)
#define RACE_DA_FINISH              (0x2106)

#define LEN_4K                      (0x1000)
#define LEN_64K                     (0x10000)

static uint8_t g_hdl_fw_data_buf[DA_FW_PACKET_LEN] = {0};
static uint8_t g_hdl_readback_data_buf[DA_FW_PACKET_LEN] = {0};

bool hdl_da_get_flash_address(uint32_t *pData)
{
    HDL_LOGI("hdl_da_get_flash_address start");

    // send
    RACE_ADDR_SEND send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = sizeof(send.id_);
    send.id_ = RACE_DA_GET_FLASH_ADDRESS;
    HDL_COM_PutByte_Buffer((uint8_t *)&send, sizeof(send));

    // response
    RACE_ADDR_RES res;
    HDL_COM_GetByte_Buffer((uint8_t *)&res, sizeof(res));
    if (res.head_ == 0x05 &&
        res.type_ == 0x5B &&
        res.len_ == (sizeof(res.id_)+sizeof(res.status_)+sizeof(res.addr_)) &&
        res.id_ == RACE_DA_GET_FLASH_ADDRESS &&
        res.status_ == DA_S_DONE)
    {
        HDL_LOGI("hdl_da_get_flash_address res done");
        *pData = res.addr_;
        return true;
    }
    HDL_LOGI("hdl_da_get_flash_address res fail");
    
    return false;
}

bool hdl_da_get_flash_size(uint32_t *pData)
{
    HDL_LOGI("hdl_da_get_flash_size start");

    // send
    RACE_SIZE_SEND send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = sizeof(send.id_);
    send.id_ = RACE_DA_GET_FLASH_SIZE;
    HDL_COM_PutByte_Buffer((uint8_t *)&send, sizeof(send));

    // response
    RACE_SIZE_RES res;
    HDL_COM_GetByte_Buffer((uint8_t *)&res, sizeof(res));
    if (res.head_ == 0x05 &&
        res.type_ == 0x5B &&
        res.len_ == (sizeof(res.id_)+sizeof(res.status_)+sizeof(res.size_)) &&
        res.id_ == RACE_DA_GET_FLASH_SIZE &&
        res.status_ == DA_S_DONE)
    {
        HDL_LOGI("hdl_da_get_flash_size res done");
        *pData = res.size_;
        return true;
    }
    HDL_LOGI("hdl_da_get_flash_size res fail");
    
    return false;
}

bool hdl_da_get_flash_id(uint8_t *pManufacturerId, uint8_t *pDeviceId1, uint8_t *pDeviceId2)
{
    HDL_LOGI("hdl_da_get_flash_id start");

    // send
    RACE_ID_SEND send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = sizeof(send.id_);
    send.id_ = RACE_DA_GET_FLASH_ID;
    HDL_COM_PutByte_Buffer((uint8_t *)&send, sizeof(send));

    // response
    RACE_ID_RES res;
    HDL_COM_GetByte_Buffer((uint8_t *)&res, sizeof(res));
    if (res.head_ == 0x05 &&
        res.type_ == 0x5B &&
        res.len_ == (sizeof(res.id_)+sizeof(res.status_)+sizeof(res.flash_id_)) &&
        res.id_ == RACE_DA_GET_FLASH_ID &&
        res.status_ == DA_S_DONE)
    {
        HDL_LOGI("hdl_da_get_flash_id res done");
        *pManufacturerId = res.flash_id_[0];
        *pDeviceId1 = res.flash_id_[1];
        *pDeviceId2 = res.flash_id_[2];
        return true;
    }
    HDL_LOGI("hdl_da_get_flash_id res fail");
    
    return false;
}

bool hdl_format_race(uint32_t addr, uint32_t len)
{
    for (int i=0; i<3; i++)
    {
        // send
        RACE_FM_SEND send;
        send.head_ = 0x05;
        send.type_ = 0x5A;
        send.len_ = sizeof(send.id_)+sizeof(send.addr_)+sizeof(send.size_)+sizeof(send.crc_);
        send.id_ = RACE_DA_ERASE_BYTES;
        send.addr_ = addr;
        send.size_ = len;
        hwcrypto_crc32(&send, sizeof(send)-sizeof(send.crc_), &send.crc_);
        // send.crc_ = CRC32(&send, sizeof(send)-sizeof(send.crc_));
        HDL_COM_PutByte_Buffer((uint8_t *)&send, sizeof(send));

        // response
        RACE_FM_RES res;
        HDL_COM_GetByte_Buffer((uint8_t *)&res, sizeof(res));
        if (res.head_ == 0x05 &&
            res.type_ == 0x5B &&
            res.len_ == (sizeof(res.id_)+sizeof(res.status_)+sizeof(res.addr_)) &&
            res.id_ == RACE_DA_ERASE_BYTES &&
            res.status_ == DA_S_DONE &&
            res.addr_ == addr)
        {
            HDL_LOGI("hdl_format_race res done: addr=0x%X, len=0x%X", addr, len);
            return true;
        }
        
        HDL_LOGI("hdl_format_race res retry: addr=0x%X, len=0x%X", addr, len);
    }

    return false;
}

bool hdl_da_format(const hdl_format_arg_t *format_arg)
{
    const uint32_t format_begin_address = format_arg->format_flash_addr;
    const uint32_t format_size = format_arg->format_len;
    const uint32_t format_end_address = format_begin_address+format_size;

    uint32_t format_address = format_begin_address;
    while (format_address < format_end_address)
    {
        uint32_t block_size = 0;
        if (format_size >= LEN_64K && (format_address%LEN_64K) == 0) {
            block_size = LEN_64K;
        } else {
            block_size = LEN_4K;
        }
        
        if (!hdl_format_race(format_address, block_size))
        {
            HDL_LOGE("hdl_format_race addr=0x%08X fail!", format_address);
            return false;
        }

        format_address+=block_size;

        uint8_t progress = (format_address-format_begin_address)*100/format_size;
        progress = (progress > 100) ? 100: progress;
        if (format_arg != NULL && format_arg->format_progress_cb != NULL) 
        {
            format_arg->format_progress_cb(format_arg->format_progress_cb_arg, progress);
        }
    }
    return true;
}

bool hdl_download_race(uint32_t addr, const uint8_t *data)
{
    for (int i=0; i<3; i++)
    {
        // send
        RACE_DL_SEND send;
        send.head_ = 0x05;
        send.type_ = 0x5A;
        send.len_ = sizeof(send.id_)+sizeof(send.addr_)+sizeof(send.size_)+DA_SEND_PACKET_LEN+sizeof(send.crc_);
        send.id_ = RACE_DA_WRITE_BYTES;
        send.addr_ = addr;
        send.size_ = DA_SEND_PACKET_LEN;
        memcpy(send.buf_, data, send.size_);
        send.crc_ = CRC32(&send, sizeof(send)-sizeof(send.crc_));
        HDL_COM_PutByte_Buffer((uint8_t *)&send, sizeof(send));

        // response
        RACE_DL_RES res;
        HDL_COM_GetByte_Buffer((uint8_t *)&res, sizeof(res));
        if (res.head_ == 0x05 &&
            res.type_ == 0x5B &&
            res.len_ == (sizeof(res.id_)+sizeof(res.status_)+sizeof(res.addr_)) &&
            res.id_ == RACE_DA_WRITE_BYTES &&
            res.status_ == DA_S_DONE &&
            res.addr_ == addr)
        {
            HDL_LOGI("hdl_download_race res done: addr=0x%X", addr);
            return true;
        }
        
        HDL_LOGE("hdl_download_race res retry: addr=0x%X", addr);
    }

    return false;
}

bool hdl_da_download(hdl_image_t *image, hdl_download_cb download_cb, void *download_cb_arg)
{
    bool res = false;
    const uint32_t start_addr = image->image_slave_flash_addr;
    const uint32_t image_len = image->image_len;
    const uint32_t packet_num = ((image_len - 1) / DA_SEND_PACKET_LEN) + 1;
    hdl_flash_init(image->image_host_file);
    
    uint32_t packet_sent_num = 0;
    while (packet_sent_num < packet_num) 
    {
        const bool is_last_packet = (packet_sent_num == (packet_num-1));
        const uint32_t start_offset = packet_sent_num * DA_SEND_PACKET_LEN;
        const uint32_t cur_packet_len = is_last_packet ? (image_len-start_offset) : DA_SEND_PACKET_LEN;

        memset(g_hdl_fw_data_buf, 0, sizeof(g_hdl_fw_data_buf));

        const uint32_t host_addr = start_offset;
        if (hdl_flash_read(host_addr, g_hdl_fw_data_buf, cur_packet_len))
        {
            uint8_t *packet_buf = g_hdl_fw_data_buf;
            
            if (is_last_packet && cur_packet_len < DA_SEND_PACKET_LEN) {
                // DA Must received whole DA_SEND_PACKET_LEN, so Fill 0xFF to packet_buf if it is last packet
                memset(packet_buf+cur_packet_len, 0xFF, DA_SEND_PACKET_LEN-cur_packet_len);
            }
            
            const uint32_t slave_addr = start_addr+start_offset;
            if (!hdl_download_race(slave_addr, packet_buf))
            {
                HDL_LOGE("hdl_download_race addr=0x%08X fail!", slave_addr);
                goto _exit_;
            }
            
            // Download Progress callback
            if (download_cb != NULL) 
            {
                download_cb(download_cb_arg, image->image_name, start_offset+cur_packet_len, image_len);
            }
        }
        else
        {
            HDL_LOGE("hdl_flash_read host addr=0x%08X fail!", host_addr);
            continue;
        }
        
        packet_sent_num++;
    }
    res = true;

_exit_:
    hdl_flash_deinit();
    return res;
}

bool hdl_finish_race(bool enable)
{
    // send
    RACE_RST_SEND send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = 3;
    send.id_ = RACE_DA_FINISH;
    send.flag_ = enable;
    HDL_COM_PutByte_Buffer((uint8_t *)&send, sizeof(send));

    // response
    RACE_RST_RES res;
    HDL_COM_GetByte_Buffer((uint8_t *)&res, sizeof(res));
    if (res.head_ == 0x05 &&
        res.type_ == 0x5B &&
        res.len_ == 3 &&
        res.id_ == RACE_DA_FINISH &&
        res.status_ == DA_S_DONE)
    {
        HDL_LOGI("hdl_finish_race res done");
        return true;
    }
    HDL_LOGI("hdl_finish_race res fail");

    return false;
}

