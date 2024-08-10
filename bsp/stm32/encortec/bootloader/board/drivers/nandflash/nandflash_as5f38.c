#include "stm32u5xx_hal.h"
#include "nandflash_as5f38.h"
#include "drv_common.h"

OSPI_HandleTypeDef hospi;

void MX_OSPI_Init(void) {
    // 启用 OSPI 外设的时钟
    __HAL_RCC_OSPI1_CLK_ENABLE();

    hospi.Instance = OCTOSPI1;
    hospi.Init.FifoThreshold = 4;
    hospi.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
    hospi.Init.MemoryType = HAL_OSPI_MEMTYPE_MICRON;
    hospi.Init.DeviceSize = POSITION_VAL(4096) - 1;
    hospi.Init.ChipSelectHighTime = 1;
    hospi.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
    hospi.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
    hospi.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
    hospi.Init.ClockPrescaler = 2;
    hospi.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
    hospi.Init.ChipSelectBoundary = 0;
    hospi.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
    hospi.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED;
    hospi.Init.MaxTran = 0;
    hospi.Init.Refresh = 0;

    if (HAL_OSPI_Init(&hospi) != HAL_OK) {
        Error_Handler();
    }

    // 检查 NAND Flash 初始化是否成功
    if (HAL_OSPI_GetState(&hospi) != HAL_OSPI_STATE_READY) {
        Error_Handler(); // 如果初始化失败，进入错误处理
    }
}

void HAL_OSPI_MspInit(OSPI_HandleTypeDef *ospiHandle) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (ospiHandle->Instance == OCTOSPI1) {
        // 启用相关 GPIO 时钟
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOE_CLK_ENABLE();

        // 配置 NCS (片选) 引脚: PA4
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // 配置 CLK 引脚: PB10
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // 配置 IO0 (PB1), IO1 (PB0)
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // 配置 IO2 (PE14), IO3 (PE15)
        GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        // 启用 OSPI 时钟
        __HAL_RCC_OSPI1_CLK_ENABLE();
    }
}

void NAND_EnableECC(void) {
    OSPI_RegularCmdTypeDef sCommand;
    uint8_t data = ECC_ENABLE_BIT;

    sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction = SET_FEATURE_CMD;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = HAL_OSPI_ADDRESS_8_BITS;
    sCommand.Address = ECC_FEATURE_ADDRESS;
    sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData = 1;
    sCommand.DummyCycles = 0;
    sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK ||
        HAL_OSPI_Transmit(&hospi, &data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }
}

bool NAND_CheckECC(void) {
    uint8_t status;
    OSPI_RegularCmdTypeDef sCommand = {
        .OperationType = HAL_OSPI_OPTYPE_COMMON_CFG,
        .FlashId = HAL_OSPI_FLASH_ID_1,
        .Instruction = GET_FEATURE_CMD,
        .InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE,
        .AddressMode = HAL_OSPI_ADDRESS_1_LINE,
        .AddressSize = HAL_OSPI_ADDRESS_8_BITS,
        .Address = ECC_STATUS_REGISTER,
        .DataMode = HAL_OSPI_DATA_1_LINE,
        .NbData = 1,
        .DummyCycles = 0,
        .DQSMode = HAL_OSPI_DQS_DISABLE,
        .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD
    };

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK ||
        HAL_OSPI_Receive(&hospi, &status, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
        return false;
    }

    if ((status & ECC_UNCORRECTABLE_ERROR) == ECC_UNCORRECTABLE_ERROR) {
        return false;
    } else if ((status & ECC_CORRECTABLE_ERROR) == ECC_CORRECTABLE_ERROR) {
        return true;
    } else {
        return true;
    }
}


bool CheckIfBadBlock(uint32_t blockAddr) {
    uint8_t marker = 0xFF;
    // 坏块标记通常位于每个块的第一页的备用区域中
    NAND_Read(blockAddr + BAD_BLOCK_PAGE * PAGE_SIZE + SPARE_AREA_SIZE - 1, &marker, 1);
    return marker != 0xFF; // 如果标记为非0xFF，则判断为坏块
}

void MarkBlockAsBad(uint32_t blockAddr) {
    uint8_t marker = BAD_BLOCK_MARKER;
    OSPI_RegularCmdTypeDef sCommand;

    // 设置 OSPI 命令，使用 0x32 命令写入备用区域
    sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction = WRITE_CMD_32;  // 使用 0x32 命令
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.Address = blockAddr + BAD_BLOCK_PAGE * PAGE_SIZE + SPARE_AREA_SIZE - 1;
    sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData = 1;
    sCommand.DummyCycles = 0;
    sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK ||
        HAL_OSPI_Transmit(&hospi, &marker, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }
    
    // Program Execute (PE) 命令
    sCommand.Instruction = PROGRAM_EXECUTE_CMD;  // 发送 10H 命令，执行写入
    sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;  // PE 命令不需要地址

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }
}

void NAND_Read(uint32_t address, uint8_t *pData, uint32_t size) {
    uint32_t blockAddr = address & ~(BLOCK_SIZE - 1);
    if (CheckIfBadBlock(blockAddr)) {
        return;
    }

    NAND_EnableECC();

    OSPI_RegularCmdTypeDef sCommand = {
        .OperationType = HAL_OSPI_OPTYPE_COMMON_CFG,
        .FlashId = HAL_OSPI_FLASH_ID_1,
        .Instruction = PAGE_READ_CMD,
        .InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE,
        .AddressMode = HAL_OSPI_ADDRESS_1_LINE,
        .AddressSize = HAL_OSPI_ADDRESS_24_BITS,
        .Address = address,
        .DataMode = HAL_OSPI_DATA_NONE,
        .DummyCycles = 0,
        .DQSMode = HAL_OSPI_DQS_DISABLE,
        .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD
    };

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    sCommand.Instruction = READ_FROM_CACHE_CMD;
    sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData = size;
    sCommand.DummyCycles = 8;

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK ||
        HAL_OSPI_Receive(&hospi, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    if (!NAND_CheckECC()) {
        MarkBlockAsBad(blockAddr);
        Error_Handler();
    }
}

void NAND_Write(uint32_t address, uint8_t *pData, uint32_t size, bool updateSpareArea) {
    uint32_t blockAddr = address & ~(BLOCK_SIZE - 1);
    if (CheckIfBadBlock(blockAddr)) {
        MarkBlockAsBad(blockAddr);
        return;
    }

    NAND_EnableECC();

    OSPI_RegularCmdTypeDef sCommand = {
        .OperationType = HAL_OSPI_OPTYPE_COMMON_CFG,
        .FlashId = HAL_OSPI_FLASH_ID_1,
        .Instruction = updateSpareArea ? WRITE_CMD_32 : WRITE_CMD_02,
        .InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE,
        .AddressMode = HAL_OSPI_ADDRESS_1_LINE,
        .AddressSize = HAL_OSPI_ADDRESS_24_BITS,
        .Address = address,
        .DataMode = HAL_OSPI_DATA_1_LINE,
        .NbData = size,
        .DummyCycles = 0,
        .DQSMode = HAL_OSPI_DQS_DISABLE,
        .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD
    };

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK ||
        HAL_OSPI_Transmit(&hospi, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    sCommand.Instruction = PROGRAM_EXECUTE_CMD;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    if (!NAND_CheckECC()) {
        MarkBlockAsBad(blockAddr);
        Error_Handler();
    }
}

