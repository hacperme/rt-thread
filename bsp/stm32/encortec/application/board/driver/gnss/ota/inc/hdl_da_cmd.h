#ifndef __HDL_DA_CMD_H__
#define __HDL_DA_CMD_H__

#include "hdl_channel.h"
#include "hdl_ports/hdl_flash_api.h"
#include "hdl_api.h"


#ifdef __cplusplus
extern "C" {
#endif

// DA Return
#define DA_S_DONE                               0x00

#define DA_SYNC_CHAR                            0xC0
#define DA_CONT_CHAR                            0x69
#define DA_STOP_CHAR                            0x96
#define DA_ACK                                  0x5A
#define DA_NACK                                 0xA5
#define DA_UNKNOWN_CMD                          0xBB
#define DA_FLUSH_CONT                           0xE1
#define DA_FLUSH_DONE                           0xE2

#define DA_S_IN_PROGRESS                        3021
#define DA_S_UART_GET_DATA_TIMEOUT              3061
#define DA_S_UART_GET_CHKSUM_LSB_TIMEOUT        3062
#define DA_S_UART_GET_CHKSUM_MSB_TIMEOUT        3063
#define DA_S_UART_DATA_CKSUM_ERROR              3064
#define DA_S_UART_RX_BUF_FULL                   3065

// DA CMD
#define DA_FORMAT_CMD                           0xD4
#define DA_NOR_WRITE_DATA                       0xB2
#define DA_READ_CMD                             0xD6
#define DA_RESET_CMD                               0xD9

#define DA_FW_PACKET_LEN                          (0x1000)
#define DA_SEND_PACKET_LEN                      (DA_FW_PACKET_LEN)
#define DA_RECV_PACKET_LEN                      (DA_FW_PACKET_LEN)

#pragma pack(1)
typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
}RACE_FLOW_CTRL_SEND;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
}RACE_FLOW_CTRL_RES;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint32_t rate_;
}RACE_BD_SEND;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
}RACE_BD_RES;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
}RACE_ID_SEND;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint8_t flash_id_[3];
}RACE_ID_RES;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
}RACE_SIZE_SEND;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t size_;
}RACE_SIZE_RES;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
}RACE_ADDR_SEND;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t addr_;
}RACE_ADDR_RES;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint32_t addr_;
    uint32_t size_;
    uint32_t crc_;
}RACE_FM_SEND;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t addr_;
}RACE_FM_RES;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint32_t addr_;
    uint16_t size_;
    uint8_t buf_[DA_SEND_PACKET_LEN];
    uint32_t crc_;
}RACE_DL_SEND;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t addr_;
}RACE_DL_RES;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint32_t addr_;
    uint16_t size_;
    uint32_t crc_;
}RACE_RB_SEND;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t addr_;
    uint32_t crc_;
    uint8_t buf_[DA_RECV_PACKET_LEN];
}RACE_RB_RES;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t flag_;
}RACE_RST_SEND;

typedef struct
{
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
}RACE_RST_RES;
#pragma pack()

#ifdef HDL_VIA_UART
bool hdl_open_slave_flow_ctrl();
bool hdl_speedup_slave_baudrate(const uint32_t baudrate);
#endif

bool hdl_da_get_flash_address(uint32_t *pData);
bool hdl_da_get_flash_size(uint32_t *pData);
bool hdl_da_get_flash_id(uint8_t *pManufacturerId, uint8_t *pDeviceId1, uint8_t *pDeviceId2);

bool hdl_da_format(const hdl_format_arg_t *format_arg);
bool hdl_da_readback(const hdl_readback_arg_t *readback_arg);
bool hdl_da_download(hdl_image_t *image, hdl_download_cb download_cb, void *download_cb_arg);
bool hdl_finish_race(bool enable);


#ifdef __cplusplus
}
#endif

#endif //__HDL_DA_CMD_H__

