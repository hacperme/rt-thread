#include "hdl_channel.h"

g_hdl_channel_t g_hdl_channel;

HDL_CHANNEL_TYPE g_hdl_channel_type = CHANNEL_TYPE_UART;

bool hdl_channel_init(void)
{
#ifdef HDL_VIA_UART
    g_hdl_channel_type                    = CHANNEL_TYPE_UART;
    g_hdl_channel.m_com_init              = hdl_uart_init;
    g_hdl_channel.m_com_get_byte          = hdl_uart_get_byte;
    g_hdl_channel.m_com_put_byte          = hdl_uart_put_byte;
    g_hdl_channel.m_com_get_byte_buffer   = hdl_uart_get_byte_buffer;
    g_hdl_channel.m_com_put_byte_buffer   = hdl_uart_put_byte_buffer;
    g_hdl_channel.m_com_get_data16        = hdl_uart_get_data16;
    g_hdl_channel.m_com_put_data16        = hdl_uart_put_data16;
    g_hdl_channel.m_com_get_data32        = hdl_uart_get_data32;
    g_hdl_channel.m_com_put_data32        = hdl_uart_put_data32;
    g_hdl_channel.m_com_set_baudrate      = hdl_uart_set_baudrate;
    g_hdl_channel.m_com_deinit            = hdl_uart_deinit;
    HDL_LOGI("hdl_channel_type: UART");
#endif

    return HDL_COM_Init();
}

uint8_t hdl_get_channel_type()
{
    return g_hdl_channel_type;
}

