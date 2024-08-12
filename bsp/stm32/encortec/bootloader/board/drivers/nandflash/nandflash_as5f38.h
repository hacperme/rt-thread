#ifndef __NANDFLASH_DRV_H
#define __NANDFLASH_DRV_H

#include <stdbool.h>
#include <stdint.h>

#define PAGE_SIZE 4096               // 页面数据区大小为 4096 字节
#define SPARE_AREA_SIZE 256          // 备用区大小为 256 字节
#define BLOCK_SIZE (PAGE_SIZE * 64)  // 每个块包含 64 页
#define BAD_BLOCK_MARKER 0x00        // 坏块标记值，通常为非0xFF值
#define BAD_BLOCK_PAGE 0             // 坏块标记所在的页

#define SET_FEATURE_CMD 0x1F           // 设置设备特性的命令代码
#define ECC_FEATURE_ADDRESS 0xB0       // ECC 特性寄存器地址
#define ECC_ENABLE_BIT 0x10            // 启用 ECC 的位 (第4位, 0x10)

#define GET_FEATURE_CMD 0x0F           // 读取特性寄存器的命令代码
#define ECC_STATUS_REGISTER 0xC0       // ECC 状态寄存器地址
#define ECC_CORRECTABLE_ERROR 0x02     // ECCS1 = 0, ECCS0 = 1 表示检测到并纠正了错误
#define ECC_UNCORRECTABLE_ERROR 0x04   // ECCS1 = 1, ECCS0 = 0 表示检测到但未能纠正错误

#define PAGE_READ_CMD 0x13          // 读取页面到缓存的命令
#define READ_FROM_CACHE_CMD 0x0B    // 从缓存读取数据的快速读取命令
#define WRITE_CMD_02 0x02           // 标准写入命令，适用于普通数据区
#define WRITE_CMD_32 0x32           // 扩展写入命令，适用于数据及备用区
#define PROGRAM_EXECUTE_CMD 0x10    // 执行写入的命令

extern void MX_OSPI_Init(void);
extern void NAND_EnableECC(void);
extern bool NAND_CheckECC(void);
extern bool CheckIfBadBlock(uint32_t blockAddr);
extern void MarkBlockAsBad(uint32_t blockAddr);
extern void NAND_Read(uint32_t address, uint8_t *pData, uint32_t size);
extern void NAND_Write(uint32_t address, uint8_t *pData, uint32_t size, bool updateSpareArea);

#endif
