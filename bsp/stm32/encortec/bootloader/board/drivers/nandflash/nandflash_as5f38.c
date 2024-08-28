#include "stm32u5xx_hal.h"
#include "nandflash_as5f38.h"
#include "drv_common.h"
#include <string.h>

OSPI_HandleTypeDef hospi;

void MX_OSPI_Init(void) {
    // 启用 OSPI 外设的时钟
    __HAL_RCC_OSPI1_CLK_ENABLE();

    hospi.Instance = OCTOSPI1;
    hospi.Init.FifoThreshold = 4;
    hospi.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
    hospi.Init.MemoryType = HAL_OSPI_MEMTYPE_MICRON;
    hospi.Init.DeviceSize = 30;
    hospi.Init.ChipSelectHighTime = 3;
    hospi.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
    hospi.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
    hospi.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
    hospi.Init.ClockPrescaler = 32;
    hospi.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE; //可能会存在信号延迟
    hospi.Init.ChipSelectBoundary = 0;
    hospi.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE; //可能会存在信号延迟
    hospi.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED; //可能会存在信号延迟
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
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pull = GPIO_NOPULL;

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
    OSPI_RegularCmdTypeDef sCommand;
    
    // 读取块的第一个页面的备用区中的坏块标记
    sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction = PAGE_READ_CMD;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.Address = blockAddr + BAD_BLOCK_PAGE * PAGE_SIZE + BAD_BLOCK_MARKER_OFFSET;
    sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData = 1;  // 读取一个字节的坏块标记
    sCommand.DummyCycles = 8;
    sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_OSPI_Receive(&hospi, &marker, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    // 如果标记为0x00，则判断为坏块
    return marker == 0x00;
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

void NAND_ReadPage(uint32_t address, uint8_t *pData, uint32_t size) {
    uint32_t blockAddr = address & ~(BLOCK_SIZE - 1);
    if (CheckIfBadBlock(blockAddr)) {
        return;
    }

    NAND_EnableECC();

    uint8_t buffer[PAGE_SIZE]; // 用于存储整个页面的数据，包括备用区

    // Step 1: 使用 PAGE_READ_CMD 读取页面数据到缓存
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

    // Step 2: 使用 READ_FROM_CACHE_CMD 从缓存读取数据到 buffer
    sCommand.Instruction = READ_FROM_CACHE_CMD;
    sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData = PAGE_SIZE;  // 读取整个页面（包括备用区）
    sCommand.DummyCycles = 8;

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK ||
        HAL_OSPI_Receive(&hospi, buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    // Step 3: 检查 ECC 状态
    if (!NAND_CheckECC()) {
        MarkBlockAsBad(blockAddr);
        Error_Handler();
    }

    // 将前 4096 字节的数据区内容复制到用户提供的缓冲区中
    memcpy(pData, buffer, size);
}

void NAND_WritePage(uint32_t address, uint8_t *pData, uint32_t size, bool updateSpareArea) {
    uint32_t blockAddr = address & ~(BLOCK_SIZE - 1);
    if (CheckIfBadBlock(blockAddr)) {
        MarkBlockAsBad(blockAddr);
        return;
    }

    NAND_EnableECC();

    // 限制写入大小为前 4096 字节
    if (size > 4096) {
        size = 4096; // 防止用户写入超出数据区的内容
    }

    uint8_t buffer[PAGE_SIZE] = {0}; // 用于存储要写入的数据，包括备用区
    memcpy(buffer, pData, size);     // 将用户数据复制到缓冲区

    OSPI_RegularCmdTypeDef sCommand = {
        .OperationType = HAL_OSPI_OPTYPE_COMMON_CFG,
        .FlashId = HAL_OSPI_FLASH_ID_1,
        .Instruction = updateSpareArea ? WRITE_CMD_32 : WRITE_CMD_02,
        .InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE,
        .AddressMode = HAL_OSPI_ADDRESS_1_LINE,
        .AddressSize = HAL_OSPI_ADDRESS_24_BITS,
        .Address = address,
        .DataMode = HAL_OSPI_DATA_1_LINE,
        .NbData = PAGE_SIZE,
        .DummyCycles = 0,
        .DQSMode = HAL_OSPI_DQS_DISABLE,
        .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD
    };

    // 将数据和备用区一起写入
    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK ||
        HAL_OSPI_Transmit(&hospi, buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
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

bool NAND_MovePage(uint32_t sourceAddr, uint32_t destAddr) {
    uint8_t buffer[PAGE_SIZE]; // 用于存储页面数据，包括备用区

    // Step 1: 读取源页面的数据
    NAND_ReadPage(sourceAddr, buffer, PAGE_SIZE - SPARE_AREA_SIZE);

    // 检查是否成功读取
    if (CheckIfBadBlock(sourceAddr & ~(BLOCK_SIZE - 1))) {
        return false; // 如果源页面在坏块中，则不进行移动
    }

    // Step 2: 将数据写入目标页面
    NAND_WritePage(destAddr, buffer, PAGE_SIZE - SPARE_AREA_SIZE, true);

    // 检查是否成功写入
    if (CheckIfBadBlock(destAddr & ~(BLOCK_SIZE - 1))) {
        MarkBlockAsBad(destAddr & ~(BLOCK_SIZE - 1)); // 如果目标页面在坏块中，标记该块为坏块
        return false;
    }

    return true; // 成功移动页面
}

void NAND_EraseBlock(uint32_t blockAddr) {
    if (CheckIfBadBlock(blockAddr)) {
        MarkBlockAsBad(blockAddr);
        return;
    }

    OSPI_RegularCmdTypeDef sCommand = {
        .OperationType = HAL_OSPI_OPTYPE_COMMON_CFG,
        .FlashId = HAL_OSPI_FLASH_ID_1,
        .Instruction = BLOCK_ERASE_CMD,
        .InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE,
        .AddressMode = HAL_OSPI_ADDRESS_1_LINE,
        .AddressSize = HAL_OSPI_ADDRESS_24_BITS,
        .Address = blockAddr,
        .DataMode = HAL_OSPI_DATA_NONE,
        .DummyCycles = 0,
        .DQSMode = HAL_OSPI_DQS_DISABLE,
        .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD
    };

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    // 等待擦除操作完成
    while (HAL_OSPI_GetState(&hospi) != HAL_OSPI_STATE_READY) {
        // 可以添加超时或其他处理机制
    }

    if (!NAND_CheckECC()) {
        MarkBlockAsBad(blockAddr);
        Error_Handler();
    }

    // 检查操作状态并处理错误
    uint8_t status = NAND_ReadStatus();
    if ((status & 0x01) == 0x01) {  // 检查是否发生错误 (假设状态位0表示错误)
        NAND_Reset();
        Error_Handler();  // 可选择进行复位或其他错误处理
    }
}

uint8_t NAND_ReadStatus(void) {
    uint8_t status;
    OSPI_RegularCmdTypeDef sCommand = {
        .OperationType = HAL_OSPI_OPTYPE_COMMON_CFG,
        .FlashId = HAL_OSPI_FLASH_ID_1,
        .Instruction = READ_STATUS_CMD,
        .InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE,
        .AddressMode = HAL_OSPI_ADDRESS_NONE,
        .DataMode = HAL_OSPI_DATA_1_LINE,
        .NbData = 1,
        .DummyCycles = 0,
        .DQSMode = HAL_OSPI_DQS_DISABLE,
        .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD
    };

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK ||
        HAL_OSPI_Receive(&hospi, &status, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    return status;
}

void NAND_Reset(void) {
    OSPI_RegularCmdTypeDef sCommand = {
        .OperationType = HAL_OSPI_OPTYPE_COMMON_CFG,
        .FlashId = HAL_OSPI_FLASH_ID_1,
        .Instruction = RESET_CMD,
        .InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE,
        .AddressMode = HAL_OSPI_ADDRESS_NONE,
        .DataMode = HAL_OSPI_DATA_NONE,
        .DummyCycles = 0,
        .DQSMode = HAL_OSPI_DQS_DISABLE,
        .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD
    };

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    // 等待复位操作完成
    while (HAL_OSPI_GetState(&hospi) != HAL_OSPI_STATE_READY) {
        // 可以添加超时或其他处理机制
    }
}

void NAND_ReadID(uint8_t *id, uint32_t size) {
    OSPI_RegularCmdTypeDef sCommand = {
        .OperationType = HAL_OSPI_OPTYPE_COMMON_CFG,
        .FlashId = HAL_OSPI_FLASH_ID_1,
        .Instruction = READ_ID_CMD,
        .InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE,
        .AddressMode = HAL_OSPI_ADDRESS_1_LINE,  // 使用1线模式传递地址
        .AddressSize = HAL_OSPI_ADDRESS_8_BITS,  // 地址大小为8位
        .Address = 0x00,  // 初始地址字节为0x00
        .DataMode = HAL_OSPI_DATA_1_LINE,
        .NbData = size,
        .DummyCycles = 0,
        .DQSMode = HAL_OSPI_DQS_DISABLE,
        .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD
    };

    if (HAL_OSPI_Command(&hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK ||
        HAL_OSPI_Receive(&hospi, id, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }
}
