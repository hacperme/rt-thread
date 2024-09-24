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

void HAL_NAND_BSP_Init(void);

// 初始化硬件，挂载FLASH并且检查ID是否匹配
int HAL_SPI_NAND_Init(HAL_NAND_Device_t nand_device);

// 读取ID
int HAL_SPI_NAND_Read_ID(HAL_NAND_Device_t hal_nand_device, uint8_t *din_id);

// 读取特性寄存器
int HAL_SPI_NAND_Get_Feature(HAL_NAND_Device_t hal_nand_device, uint32_t addr, uint8_t *din_feature);

// 写入特性寄存器
int HAL_SPI_NAND_Set_Feature(HAL_NAND_Device_t hal_nand_device, uint32_t addr, uint8_t *dout_feature);

// 读取状态寄存器，即地址为0xc0的特性寄存器
int HAL_SPI_NAND_Read_Status(HAL_NAND_Device_t hal_nand_device, uint8_t *status);

// 读取配置寄存器，即地址为0xb0的特性寄存器
int HAL_SPI_NAND_Get_Cfg(HAL_NAND_Device_t hal_nand_device, uint8_t *cfg);

// 设定配置寄存器值，即地址为0xb0的特性寄存器
int HAL_SPI_NAND_Set_Cfg(HAL_NAND_Device_t hal_nand_device, uint8_t *cfg);

// 配置写保护（OTP）寄存器的值，即地址为0xa0的特性寄存器
int HAL_SPI_NAND_Lock_Block(HAL_NAND_Device_t hal_nand_device, uint8_t *lock);

// 读取写保护（OTP）寄存器的值，即地址为0xa0的特性寄存器
int HAL_SPI_NAND_Get_Lock_Block(HAL_NAND_Device_t hal_nand_device, uint8_t *lock);

// 使能ECC，每写入512byte数据，会自动触发一次ECC校验，结果存在0xc0特性寄存器中
int HAL_SPI_NAND_Enable_Ecc(HAL_NAND_Device_t hal_nand_device);

// 关闭ECC，每写入512byte数据，会自动触发一次ECC校验，结果存在0xc0特性寄存器中
int HAL_SPI_NAND_Disable_Ecc(HAL_NAND_Device_t hal_nand_device);

// 检查ECC状态
void HAL_SPI_NAND_Check_Ecc_Status(uint32_t status, uint32_t *corrected, uint32_t *ecc_error);

// 通过读取NAND的状态寄存器（0xc0），获取NAND是否处于BUSY状态，直到操作完成时退出
int HAL_SPI_NAND_Wait(HAL_NAND_Device_t hal_nand_device, uint8_t *s);

// 使能写操作，写数据之前必须使能该操作
int HAL_SPI_NAND_Write_Enable(HAL_NAND_Device_t hal_nand_device);

// 停止写操作
int HAL_SPI_NAND_Write_Disable(HAL_NAND_Device_t hal_nand_device);

// 读取NAND数据到缓存区（该缓存时NAND自带的，一般和page的物理大小对齐，不需要消耗本机的heap）
int HAL_SPI_NAND_Read_Page_To_Cache(HAL_NAND_Device_t hal_nand_device, uint32_t page_addr);

// 读取NAND缓存区数据
int HAL_SPI_NAND_Read_From_Cache(HAL_NAND_Device_t hal_nand_device, uint32_t page_addr, uint32_t column, size_t len, uint8_t *din_buf);

// 将数据写入缓存区
int HAL_SPI_NAND_Program_Data_To_Cache(HAL_NAND_Device_t hal_nand_device, uint32_t page_addr, uint32_t column, size_t len, uint8_t *dout_buf, bool clr_cache);

// 写入缓存区数据到NAND
int HAL_SPI_NAND_Program_Execute(HAL_NAND_Device_t hal_nand_device, uint32_t page_addr);

// 擦除NAND的block
int HAL_SPI_NAND_Erase_Block(HAL_NAND_Device_t hal_nand_device, uint32_t block_addr);

// 内部数据移动，将NAND的某页数据移动到另一个页
int HAL_SPI_NAND_Internal_Data_Move(HAL_NAND_Device_t hal_nand_device, uint32_t page_src_addr, uint32_t page_dst_addr, uint32_t offset, uint8_t *buf, size_t len);

/**
 * @brief HAL_SPI_NAND_Check_Bad_Block 检查坏块
 * @param  hal_nand_device  NAND设备
 * @param  blk_addr        需要检查的块地址，传入页地址也可，会自动忽略page addr
 * @param  buf              数据buf
 * @return int 0：非坏块 1：坏块
 */
int HAL_SPI_NAND_Check_Bad_Block(HAL_NAND_Device_t hal_nand_device, uint32_t blk_addr);

/**
 * @brief HAL_SPI_NAND_Mark_Bad_Block 标记坏块
 * @param  hal_nand_device  NAND设备
 * @param  blk_addr        需要检查的块地址，传入页地址也可，会自动忽略page addr
 * @param  buf              数据buf
 * @return int 0：标记成功，否则失败
 */
int HAL_SPI_NAND_Mark_Bad_Block(HAL_NAND_Device_t hal_nand_device, uint32_t blk_addr);

#endif