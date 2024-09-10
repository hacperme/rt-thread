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

// void NAND_Flash_Init(void);

// void NAND_EnableECC(void);

// bool NAND_CheckECC(void);

// bool CheckIfBadBlock(uint32_t blockAddr);

// void MarkBlockAsBad(uint32_t blockAddr);

// void NAND_ReadPage(uint32_t address, uint8_t *pData, uint32_t size);

// void NAND_WritePage(uint32_t address, uint8_t *pData, uint32_t size, bool updateSpareArea);

// bool NAND_MovePage(uint32_t sourceAddr, uint32_t destAddr);

// void NAND_EraseBlock(uint32_t blockAddr);

// uint8_t NAND_ReadStatus(void);

// void NAND_Reset(void);

// void NAND_ReadID(uint8_t *id, uint32_t size);


#endif