#include "hdl_ports/hdl_flash_api.h"

bool hdl_flash_init()
{
    hal_flash_status_t status = hal_flash_init();
    return (status == HAL_FLASH_STATUS_OK);
}

bool hdl_flash_read(uint32_t start_address, uint8_t *buffer, uint32_t length)
{
    hal_flash_status_t status = hal_flash_read(start_address, buffer, length);
    return (status == HAL_FLASH_STATUS_OK);
}

bool hdl_flash_deinit()
{
    hal_flash_status_t status = hal_flash_deinit();
    return (status == HAL_FLASH_STATUS_OK);
}
