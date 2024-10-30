#ifndef __HDL_CHANNEL_H__
#define __HDL_CHANNEL_H__

#include "hdl_ports/hdl_uart.h"
// #include "hdl_ports/hdl_spi.h"
// #include "hdl_ports/hdl_i2c.h"

#include "hdl_data.h"
#include "hdl_ports/hdl_os_util.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CHANNEL_TYPE_UART = 0,
} HDL_CHANNEL_TYPE;

typedef struct {
    bool     (*m_com_init)                      (void);
    uint8_t  (*m_com_get_byte)                  (void);
    uint32_t (*m_com_get_byte_buffer)           (uint8_t* buf, uint32_t length);
    void     (*m_com_put_byte)                  (uint8_t data);
    uint32_t (*m_com_put_byte_buffer)           (uint8_t* buf, uint32_t length);
    uint16_t (*m_com_get_data16)                (void);
    void     (*m_com_put_data16)                (uint16_t data);
    uint32_t (*m_com_get_data32)                (void);
    void     (*m_com_put_data32)                (uint32_t data);
    void     (*m_com_set_baudrate)              (uint32_t baud_rate);
    bool     (*m_com_deinit)                    (void);
} g_hdl_channel_t;

extern g_hdl_channel_t g_hdl_channel;

#define HDL_COM_Init                            g_hdl_channel.m_com_init
#define HDL_COM_GetByte                         g_hdl_channel.m_com_get_byte
#define HDL_COM_GetByte_Buffer(buf,length)      g_hdl_channel.m_com_get_byte_buffer(buf, length)
#define HDL_COM_PutByte(data)                   g_hdl_channel.m_com_put_byte(data)
#define HDL_COM_PutByte_Buffer(buf,length)      g_hdl_channel.m_com_put_byte_buffer(buf, length)
#define HDL_COM_GetData16                       g_hdl_channel.m_com_get_data16
#define HDL_COM_PutData16(data)                 g_hdl_channel.m_com_put_data16(data)
#define HDL_COM_GetData32                       g_hdl_channel.m_com_get_data32
#define HDL_COM_PutData32(data)                 g_hdl_channel.m_com_put_data32(data)
#define HDL_COM_SetBaudRate(baud_rate)          g_hdl_channel.m_com_set_baudrate(baud_rate)
#define HDL_COM_Deinit                          g_hdl_channel.m_com_deinit

bool    hdl_channel_init(void);
uint8_t hdl_get_channel_type();

#ifdef __cplusplus
}
#endif

#endif //__HDL_CHANNEL_H__

