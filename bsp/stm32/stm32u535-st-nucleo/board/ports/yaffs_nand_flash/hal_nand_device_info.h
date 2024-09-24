#ifndef __NANDFLASH_INFO_H
#define __NANDFLASH_INFO_H
#include <stdbool.h>
#include <stdint.h>
#include "stm32u5xx_hal.h"

#define PAGE_SIZE                       4096
#define SPARE_SIZE                      256
#define SPARE_FREE_SIZE                 143
#define PAGE_TOTAL_SIZE                 (PAGE_SIZE + SPARE_SIZE)
#define PAGE_PER_BLOCK                  64
#define BLOCK_SIZE                      (PAGE_PER_BLOCK * PAGE_SIZE)
// #define BLOCK_NUM_PER_CHIP              4096
// #define BLOCK_NUM_PER_CHIP              1360
#define BLOCK_NUM_PER_CHIP              64
#define PAGE_PER_CHIP                   (PAGE_PER_BLOCK * BLOCK_NUM_PER_CHIP)

//#define NAND_MODEL_AUTO_DETECT
/****************************************************/
/************* FLASH MODEL SUPPORT LIST *************/
/****************************************************/
#define NAND_FACTORY_SUPPORT_ALLIANCE

#ifdef NAND_FACTORY_SUPPORT_ALLIANCE
// #define NAND_MODEL_AS5F31G04SND_08LIN_SUPPORT
// #define NAND_MODEL_AS5F32G04SND_08LIN_SUPPORT
// #define NAND_MODEL_AS5F34G04SND_08LIN_SUPPORT
#define NAND_MODEL_AS5F38G04SND_08LIN_SUPPORT
#endif


/****************************************************/
/************* FLASH CMD SUPPORT LIST *************/
/****************************************************/

#define SPINAND_CMD_RESET			            0xff
#define SPINAND_CMD_READ_ID			            0x9f
#define SPINAND_CMD_GET_FEATURE			        0x0f
#define SPINAND_CMD_SET_FEATURE			        0x1f
#define SPINAND_CMD_PAGE_READ			        0x13
#define SPINAND_CMD_READ_PAGE_CACHE_RDM		    0x30
#define SPINAND_CMD_READ_PAGE_CACHE_LAST	    0x3f
#define SPINAND_CMD_READ_FROM_CACHE		        0x03
#define SPINAND_CMD_READ_FROM_CACHE_FAST	    0x0b
#define SPINAND_CMD_READ_FROM_CACHE_X2		    0x3b
#define SPINAND_CMD_READ_FROM_CACHE_DUAL_IO	    0xbb
#define SPINAND_CMD_READ_FROM_CACHE_X4		    0x6b
#define SPINAND_CMD_READ_FROM_CACHE_QUAD_IO	    0xeb
#define SPINAND_CMD_BLK_ERASE			        0xd8
#define SPINAND_CMD_PROG_EXC			        0x10
#define SPINAND_CMD_PROG_LOAD			        0x02
#define SPINAND_CMD_PROG_LOAD_RDM_DATA		    0x84
#define SPINAND_CMD_PROG_LOAD_X4		        0x32
#define SPINAND_CMD_PROG_LOAD_RDM_DATA_X4	    0x34
#define SPINAND_CMD_PROG_LOAD_RDM_QUAD          0x72
#define SPINAND_CMD_WR_DISABLE			        0x04
#define SPINAND_CMD_WR_ENABLE			        0x06
#define SPINAND_CMD_END				            0x00

#define REG_BLOCK_LOCK		                    0xa0
#define REG_CFG			                        0xb0
#define REG_STATUS		                        0xc0

/* status */
#define STATUS_OIP_MASK     		            0x01
#define STATUS_READY        		            (0 << 0)
#define STATUS_BUSY     		                (1 << 0)

#define STATUS_WEL                              (1 << 1)

#define STATUS_E_FAIL_MASK      	            0x04
#define STATUS_E_FAIL       		            (1 << 2)

#define STATUS_P_FAIL_MASK      	            0x08
#define STATUS_P_FAIL       		            (1 << 3)


/*Configuration register defines*/
#define CFG_ECC_MASK		                    0X10
#define CFG_ECC_ENABLE	                        0x10
#define CFG_OTP_MASK		                    0xc2
#define CFG_OTP_ENTER		                    0x40
#define CFG_OTP_EXIT		                    0x00
#define CFG_OTP_PROTECT	                        0xc0
#define CFG_SNOR_ENABLE	                        0x82

/* block lock */
#define BL_LOCKED_MASK			                0x3c
#define BL_ALL_LOCKED		                    0x38
#define BL_U_1_64_LOCKED		                0x08
#define BL_U_1_32_LOCKED		                0x10
#define BL_U_1_16_LOCKED		                0x18
#define BL_U_1_8_LOCKED		                    0x20
#define BL_U_1_4_LOCKED		                    0x21
#define BL_U_1_2_LOCKED		                    0x30
#define BL_L_1_64_LOCKED		                0x0c
#define BL_L_1_32_LOCKED		                0x14
#define BL_L_1_16_LOCKED		                0x1c
#define BL_L_1_8_LOCKED		                    0x24
#define BL_L_1_4_LOCKED		                    0x2c
#define BL_L_1_2_LOCKED		                    0x34
#define BL_ALL_UNLOCKED		                    0x00

#define SPI_NAND_AS5F_ECC_MASK                  0x30
#define SPI_NAND_AS5F_ECC_MAX_BIT               0x30
#define SPI_NAND_AS5F_ECC_SHIFT                 0x10
#define SPI_NAND_AS5F_ECC_0_BIT                 0x00
#define SPI_NAND_AS5F_ECC_UNCORR                0x20

// NAND flash的存储特性，根据FLASH特性填写（参见flash的datasheet）
typedef struct NAND_memory_info
{
    uint32_t page_size;                     // 每页正常数据区大小
    uint32_t spare_size;                    // 每页空闲区大小
    uint32_t page_total_size;               // 每页总大小，上述二者之和   
    uint32_t block_size;                    // 每块大小
    uint32_t page_per_block;                // 每块的页数
    uint32_t block_num_per_chip;            // 每个chip的总块数
    uint32_t page_per_chip;
    // uint32_t total_size;                 // NAND FLASH总大小
    // uint32_t block_per_plane;            // 每平面的块数（直接使用总块数）
    // uint32_t plane_per_die;              // 每Die的平面数
    // uint32_t die_per_chip;               // 每个芯片的Die数
    // uint32_t chip_count;                 // 芯片数量
} NAND_memory_info_t;   

// NAND FLASH ECC存储特性，根据FLASH特性填写（参见flash的datasheet）
typedef struct NAND_ECC_info
{
    uint32_t strength;                      // ECC强度
    uint32_t step_size;                     // 步长大小
    uint32_t hw_ecc;                        // 硬件ECC支持
} NAND_ecc_info_t;

// NAND传输模式
typedef enum
{
    NAND_XFER_NONE = 0 ,
    NAND_XFER_SINGLE = 1,                   // 单线传输
    NAND_XFER_DUAL,                         // 双线传输
    NAND_XFER_QUAD,                         // 四线传输
    NAND_XFER_MAX                           // 最大传输模式
} NAND_xfer_mode_e;

// NAND指令地址模式
typedef enum
{
    NAND_ADDR_NONE = 0,
    NAND_ADDR_1BYTE = 1,                    // 单字节地址，多用于指令和寄存器读取
    NAND_ADDR_2BYTE = 2,                    // 2字节地址，多用于指令和寄存器读取
    NAND_ADDR_3BYTE = 3,                    // 3字节地址，多用于小容量flash读写擦的寻址
    NAND_ADDR_4BYTE = 4                     // 4字节地址，多用于大容量flash读写擦的寻址
} NAND_addr_lenth_e;

// 定义NAND FLASH指令结构体
typedef struct NAND_cmd
{
    uint8_t cmd;                            // 指令本身
    NAND_xfer_mode_e cmd_xfer_mode;         // 命令传输模式
    NAND_addr_lenth_e addr_bytes;           // 地址模式
    NAND_xfer_mode_e addr_xfer_mode;        // 地址传输模式
    uint8_t dummy_bytes;                    // 空闲周期数
    NAND_xfer_mode_e data_xfer_mode;        // 数据传输模式
} NAND_cmd_t;

// 定义NAND 总体信息结构体
typedef struct NAND_flash_info
{
    char model[32];                         // 模型名称
    uint8_t manufacturer_id;                // 制造商ID
    uint8_t device_id;                      // 设备ID
    NAND_memory_info_t* memory_info;        // 存储特性
    NAND_ecc_info_t* ecc_info;              // ECC特性
    NAND_cmd_t* cmd_list;                   // 指令列表的指针
    uint8_t cmd_index_max;
} NAND_flash_info_t;

#endif

extern NAND_flash_info_t hal_flash_info_table[];