#ifndef __HDL_UART_H__
#define __HDL_UART_H__

#include "hdl_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

// ToDo Porting: Please use your platform API to implement them.
// UART API
bool hdl_uart_init();
bool hdl_uart_deinit();

uint8_t hdl_uart_get_byte();
uint32_t hdl_uart_get_byte_buffer(uint8_t *buf, uint32_t length);
void hdl_uart_put_byte(uint8_t data);
uint32_t hdl_uart_put_byte_buffer(uint8_t *buf, uint32_t length);

uint16_t hdl_uart_get_data16();
void hdl_uart_put_data16(uint16_t data);
uint32_t hdl_uart_get_data32();
void hdl_uart_put_data32(uint32_t data);

void hdl_uart_set_baudrate(uint32_t baud_rate);


#ifdef __cplusplus
}
#endif

#endif //__HDL_UART_H__
