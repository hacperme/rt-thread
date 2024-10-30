#ifndef __HDL_BROM_BASE_H__
#define __HDL_BROM_BASE_H__

#include "hdl_channel.h"
#include "hdl_ports/hdl_flash_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BROM_CMD_READ16 = 0xD0,
    BROM_CMD_WRITE16 = 0xD2,
    BROM_CMD_JUMP_DA = 0xD5,
    BROM_CMD_SEND_DA = 0xD7,
    BROM_CMD_SET_BAUD = 0xDC,
} HDL_BROM_CMD;

#define BROM_ERROR   (0x1000)
#define DA_DATA_UNIT (1024)

bool hdl_brom_start();
bool hdl_brom_disable_wdt(void);
bool hdl_brom_read16(uint32_t addr, uint16_t* data);
bool hdl_brom_write16(uint32_t addr, uint16_t data);
bool hdl_brom_send_da(const hdl_connect_arg_t* connect_arg, uint32_t da_flash_pos, uint32_t da_start_addr, uint32_t da_len);
bool hdl_brom_jump_da(uint32_t addr);
bool hdl_brom_set_baudrate(uint32_t bd);

#ifdef __cplusplus
}
#endif

#endif //__HDL_BROM_BASE_H__

