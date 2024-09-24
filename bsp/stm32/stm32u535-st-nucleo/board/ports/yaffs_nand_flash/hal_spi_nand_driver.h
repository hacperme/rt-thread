#ifndef __NANDFLASH_DRV_H
#define __NANDFLASH_DRV_H
#include <stdbool.h>
#include <stdint.h>

#include "hal_nand_device_info.h"

typedef enum
{
    SPI_FIFO,
    SPI_DMA
} spi_xfer_mode_e;

typedef struct
{
    void* spi_device;
    NAND_flash_info_t* nand_flash_info;
} HAL_NAND_Device_t;

bool HAL_SPI_NAND_Init(HAL_NAND_Device_t *hal_nand_device);
int HAL_SPI_NAND_Reset(HAL_NAND_Device_t *hal_nand_device);
int HAL_SPI_NAND_Read_ID(HAL_NAND_Device_t *hal_nand_device, uint8_t *din_id);
int HAL_SPI_NAND_Get_Feature(HAL_NAND_Device_t *hal_nand_device, uint32_t addr, uint8_t *din_feature);
int HAL_SPI_NAND_Set_Feature(HAL_NAND_Device_t *hal_nand_device, uint32_t addr, uint8_t *dout_feature);
int HAL_SPI_NAND_Read_Status(HAL_NAND_Device_t *hal_nand_device, uint8_t *status);
int HAL_SPI_NAND_Get_Cfg(HAL_NAND_Device_t *hal_nand_device, uint8_t *cfg);
int HAL_SPI_NAND_Set_Cfg(HAL_NAND_Device_t *hal_nand_device, uint8_t *cfg);
int HAL_SPI_NAND_Lock_Block(HAL_NAND_Device_t *hal_nand_device, uint8_t *lock);
int HAL_SPI_NAND_Get_Lock_Block(HAL_NAND_Device_t *hal_nand_device, uint8_t *lock);
int HAL_SPI_NAND_Enable_Ecc(HAL_NAND_Device_t *hal_nand_device);
int HAL_SPI_NAND_Disable_Ecc(HAL_NAND_Device_t *hal_nand_device);
void HAL_SPI_NAND_Check_Ecc_Status(uint32_t status, uint32_t *corrected, uint32_t *ecc_error);
int HAL_SPI_NAND_Wait(HAL_NAND_Device_t *hal_nand_device, uint8_t *s);
int HAL_SPI_NAND_Write_Enable(HAL_NAND_Device_t *hal_nand_device);
int HAL_SPI_NAND_Write_Disable(HAL_NAND_Device_t *hal_nand_device);
int HAL_SPI_NAND_Read_Page_To_Cache(HAL_NAND_Device_t *hal_nand_device, uint32_t page_addr);
int HAL_SPI_NAND_Read_From_Cache(HAL_NAND_Device_t *hal_nand_device, uint32_t page_addr, uint32_t column, size_t len, uint8_t *din_buf);
int HAL_SPI_NAND_Program_Data_To_Cache(HAL_NAND_Device_t *hal_nand_device, uint32_t page_addr, uint32_t column, size_t len, uint8_t *dout_buf, bool clr_cache);
int HAL_SPI_NAND_Program_Execute(HAL_NAND_Device_t *hal_nand_device , uint32_t page_addr);
int HAL_SPI_NAND_Erase_Block(HAL_NAND_Device_t *hal_nand_device, uint32_t block_addr);
int HAL_SPI_NAND_Internal_Data_Move(HAL_NAND_Device_t *hal_nand_device, uint32_t page_src_addr,
                                    uint32_t page_dst_addr, uint32_t offset, uint8_t *buf, size_t len);

extern HAL_NAND_Device_t hal_nand_device;

#endif