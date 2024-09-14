#include <stdbool.h>
#include <stdint.h>
#include "hal_nand_device_info.h"


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) \
    ((int)((sizeof(array) / sizeof((array)[0]))))
#endif

#ifdef NAND_FACTORY_SUPPORT_ALLIANCE

/** NAND command list for Alliance **/
static NAND_cmd_t Alliance_cmd_list[] = 
{
    // NAND CHIP OPERATION CMD
    // CMD                      CMD_XFER_TYPE     ADDR_TYPE        ADDR_XFER_TYPE    DUMMY_BYTES    DATA_XFER_TYPE
    {SPINAND_CMD_RESET,         NAND_XFER_SINGLE, NAND_ADDR_NONE,  NAND_XFER_NONE,   0,             NAND_XFER_NONE},
    {SPINAND_CMD_READ_ID,       NAND_XFER_SINGLE, NAND_ADDR_1BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_SINGLE},
    {SPINAND_CMD_GET_FEATURE,   NAND_XFER_SINGLE, NAND_ADDR_1BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_SINGLE},
    {SPINAND_CMD_SET_FEATURE,   NAND_XFER_SINGLE, NAND_ADDR_1BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_SINGLE},
    {SPINAND_CMD_WR_DISABLE,    NAND_XFER_SINGLE, NAND_ADDR_NONE,  NAND_XFER_NONE,   0,             NAND_XFER_NONE},
    {SPINAND_CMD_WR_ENABLE,     NAND_XFER_SINGLE, NAND_ADDR_NONE,  NAND_XFER_NONE,   0,             NAND_XFER_NONE},

    // NAND READ CMD
    // CMD                                  CMD_XFER_TYPE     ADDR_TYPE        ADDR_XFER_TYPE    DUMMY_BYTES    DATA_XFER_TYPE
    {SPINAND_CMD_PAGE_READ,                 NAND_XFER_SINGLE, NAND_ADDR_3BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_NONE},
    {SPINAND_CMD_READ_FROM_CACHE,           NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_SINGLE, 1,             NAND_XFER_SINGLE},
    {SPINAND_CMD_READ_FROM_CACHE_X2,        NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_SINGLE, 1,             NAND_XFER_DUAL},
    {SPINAND_CMD_READ_FROM_CACHE_X4,        NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_SINGLE, 1,             NAND_XFER_QUAD},
    {SPINAND_CMD_READ_FROM_CACHE_DUAL_IO,   NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_DUAL  , 1,             NAND_XFER_DUAL},
    {SPINAND_CMD_READ_FROM_CACHE_QUAD_IO,   NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_QUAD  , 1,             NAND_XFER_QUAD},

    // NAND ERASE CMD
    // CMD                                  CMD_XFER_TYPE     ADDR_TYPE        ADDR_XFER_TYPE    DUMMY_BYTES    DATA_XFER_TYPE
    {SPINAND_CMD_BLK_ERASE,                 NAND_XFER_SINGLE, NAND_ADDR_3BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_NONE},

    // NAND ERASE CMD
    // CMD                                  CMD_XFER_TYPE     ADDR_TYPE        ADDR_XFER_TYPE    DUMMY_BYTES    DATA_XFER_TYPE    
    {SPINAND_CMD_PROG_EXC,                  NAND_XFER_SINGLE, NAND_ADDR_3BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_NONE},
    {SPINAND_CMD_PROG_LOAD,                 NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_SINGLE},
    {SPINAND_CMD_PROG_LOAD_X4,              NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_QUAD},
    {SPINAND_CMD_PROG_LOAD_RDM_DATA,        NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_SINGLE},
    {SPINAND_CMD_PROG_LOAD_RDM_DATA_X4,     NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_SINGLE, 0,             NAND_XFER_QUAD},
    {SPINAND_CMD_PROG_LOAD_RDM_QUAD,        NAND_XFER_SINGLE, NAND_ADDR_2BYTE, NAND_XFER_QUAD,   0,             NAND_XFER_NONE},
};

#ifdef NAND_MODEL_AS5F38G04SND_08LIN_SUPPORT
/** NAND ecc &memory info for AS5F38G04SND_08LIN **/
static NAND_ecc_info_t AS5F38G04SND_08LIN_ecc_info =
{
    .strength = 8,
    .step_size = 512,
    .hw_ecc = 0
};

static NAND_memory_info_t AS5F38G04SND_08LIN_memory_info =
{
    .page_size = 4096,
    .spare_size = 256,
    .page_total_size = 4096 + 256,
    .block_size = 4096 * 64,
    .page_per_block = 64,
    .block_num_per_chip = 4096
};

#endif
#endif


NAND_flash_info_t hal_flash_info_table[] = 
{
#ifdef NAND_MODEL_AS5F38G04SND_08LIN_SUPPORT
    {
        .model = "AS5F38G04SND-08LIN",
        .manufacturer_id = 0x52,
        .device_id = 0x2D,
        .cmd_list = Alliance_cmd_list,
        .ecc_info = &AS5F38G04SND_08LIN_ecc_info,
        .memory_info = &AS5F38G04SND_08LIN_memory_info,
        .cmd_index_max = ARRAY_SIZE(Alliance_cmd_list)
    }
#endif
};