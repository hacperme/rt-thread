#ifndef __HDL_FLASH_API_H__
#define __HDL_FLASH_API_H__

#include "hdl_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

// ToDo Porting: Please use your platform API to implement them.
// Host Flash API
bool hdl_flash_init();
bool hdl_flash_read(uint32_t start_address, uint8_t *buffer, uint32_t length);
bool hdl_flash_deinit();

#ifdef __cplusplus
}
#endif

#endif //__HDL_FLASH_API_H__
