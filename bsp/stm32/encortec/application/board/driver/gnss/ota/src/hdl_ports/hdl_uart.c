#include "hdl_ports/hdl_uart.h"

#ifdef HDL_VIA_UART
bool hdl_uart_init()
{
    //Initialize the host serial port.
}

bool hdl_uart_deinit()
{
    //Destroy the host serial port.
}

uint8_t hdl_uart_get_byte()
{
    return hal_uart_get_char(HDL_UART_PORT);
}

uint32_t hdl_uart_get_byte_buffer(uint8_t *buf, uint32_t length)
{
    return hal_uart_receive_polling(HDL_UART_PORT, buf, length);
}

void hdl_uart_put_byte(uint8_t data)
{
    hal_uart_put_char(HDL_UART_PORT, data);
}

uint32_t hdl_uart_put_byte_buffer(uint8_t *buf, uint32_t length)
{
    return hal_uart_send_polling(HDL_UART_PORT, buf, length);
}

uint16_t hdl_uart_get_data16()
{
    uint8_t byte_data[2] = {0};
    hal_uart_receive_polling(HDL_UART_PORT, byte_data, 2);
    uint16_t ret = (byte_data[0]<<8)|byte_data[1];
    return ret;
}

void hdl_uart_put_data16(uint16_t data)
{
    uint8_t byte_data[2] = {0};
    byte_data[0] = (data>>8)&0xFF;
    byte_data[1] = data&0xFF;
    hal_uart_send_polling(HDL_UART_PORT, byte_data, 2);
}

uint32_t hdl_uart_get_data32()
{
    uint8_t byte_data[4] = {0};
    hal_uart_receive_polling(HDL_UART_PORT, byte_data, 4);
    uint32_t ret = (byte_data[0]<<24)|(byte_data[1]<<16)|(byte_data[2]<<8)|byte_data[3];
    return ret;
}

void hdl_uart_put_data32(uint32_t data)
{
    uint8_t byte_data[4] = {0};
    byte_data[0] = (data>>24)&0xFF;
    byte_data[1] = (data>>16)&0xFF;
    byte_data[2] = (data>>8)&0xFF;
    byte_data[3] = data&0xFF;
    hal_uart_send_polling(HDL_UART_PORT, byte_data, 4);
}

void hdl_uart_set_baudrate(uint32_t baud_rate)
{
    hal_uart_status_t status = hal_uart_set_baudrate(HDL_UART_PORT, baud_rate);
    if (status != HAL_UART_STATUS_OK)
    {
        HDL_LOGI("hdl_uart_set_baudrate fail: status=%d baud_rate=%d", status, baud_rate);
    }
}

#endif
