#include "stm32u5xx_hal.h"
#include "stm32u5xx_hal_ospi.h"
#include "hal_spi_nand_driver.h"
#include "hal_nand_device_info.h"
#include "drv_common.h"
#include <string.h>

#define DBG_TAG "NAND_G_DRV"
#define DBG_LVL DBG_WARNING
#include <rtdbg.h>

/*************************************************************************/
/********************* 全局变量 硬件&FLASH本身的操作句柄 *******************/
/*************************************************************************/


HAL_NAND_Device_t hal_nand_device =
{
    .spi_device = NULL,
    .nand_flash_info = &hal_flash_info_table[0],
};


/*************************************************************************/
/**************************** 指令转换系统 ********************************/
/************* 如果通用参数和主控的参数不一致，在此处配置转换 ****************/
/**由于指令解析次数较多，不建议使用for循环进行匹配，可以使用list来提高查找效率**/
/*************************************************************************/

#if 0
//无需转换时使用下列宏控
#define CMD_XFER_MODE(x) x
#define ADDR_XFER_MODE(x) x
#define DATA_XFER_MODE(x) x
#define ADDR_SIZE_MODE(x) x
#else
const uint32_t CMD_XFER_MODE_LIST[]=
{
    HAL_OSPI_INSTRUCTION_NONE,
    HAL_OSPI_INSTRUCTION_1_LINE,
    HAL_OSPI_INSTRUCTION_2_LINES,
    HAL_OSPI_INSTRUCTION_4_LINES,
    HAL_OSPI_INSTRUCTION_8_LINES
};

#define CMD_XFER_MODE(x) CMD_XFER_MODE_LIST[x]

const uint32_t ADDR_XFER_MODE_LIST[]=
{
    HAL_OSPI_ADDRESS_NONE,
    HAL_OSPI_ADDRESS_1_LINE,
    HAL_OSPI_ADDRESS_2_LINES,
    HAL_OSPI_ADDRESS_4_LINES,
    HAL_OSPI_ADDRESS_8_LINES
};

#define ADDR_XFER_MODE(x) ADDR_XFER_MODE_LIST[x]

const uint32_t DATA_XFER_MODE_LIST[]=
{
    HAL_OSPI_DATA_NONE,
    HAL_OSPI_DATA_1_LINE,
    HAL_OSPI_DATA_2_LINES,
    HAL_OSPI_DATA_4_LINES,
    HAL_OSPI_DATA_8_LINES
};

#define DATA_XFER_MODE(x) DATA_XFER_MODE_LIST[x]

const uint32_t ADDR_SIZE_LIST[]=
{
    0,
    HAL_OSPI_ADDRESS_8_BITS,
    HAL_OSPI_ADDRESS_16_BITS,
    HAL_OSPI_ADDRESS_24_BITS,
    HAL_OSPI_ADDRESS_32_BITS
};

#define ADDR_SIZE_MODE(x) ADDR_SIZE_LIST[x]

#endif

/*************************************************************************/
/*****************************硬件层面的初始化操作**************************/
/**如果FLASH初始化时需要使能硬件，在该处实现，并适配到HAL_NAND_BSP_Init()函数**/
/*************************************************************************/
#include "board.h"
#include "drv_common.h"

OSPI_HandleTypeDef hal_nand_ospi;

// typedef enum {
//     NAND_STM32_DIRECTION = 0,
//     NAND_ESP32_DIRECTION = 1,
// } encore_nand_direction_e;

// typedef enum {
//     NAND_POWERON = 1,
//     NAND_POWEROFF = 0,
// } encore_nand_poweron_e;

// static inline void encore_nand_direction_switch(encore_nand_direction_e direction)
// {
//     rt_pin_mode(QSPI_CPUN_ESP_PIN, PIN_MODE_OUTPUT);
//     rt_pin_write(QSPI_CPUN_ESP_PIN, direction);
// }

// static inline void encore_nand_power_switch(encore_nand_poweron_e poweron)
// {
//     rt_pin_mode(FLASH_PWRON_PIN, PIN_MODE_OUTPUT);
//     rt_pin_write(FLASH_PWRON_PIN, poweron);
// }

void MX_OSPI_Init(void) {

    // encore_nand_power_switch(NAND_POWERON);
    // encore_nand_direction_switch(NAND_STM32_DIRECTION);

    // 启用 OSPI 外设的时钟
    __HAL_RCC_OSPI1_CLK_ENABLE();

    hal_nand_ospi.Instance = OCTOSPI1;
    hal_nand_ospi.Init.FifoThreshold = 4;
    hal_nand_ospi.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
    hal_nand_ospi.Init.MemoryType = HAL_OSPI_MEMTYPE_MICRON;
    hal_nand_ospi.Init.DeviceSize = 32;
    hal_nand_ospi.Init.ChipSelectHighTime = 1;
    hal_nand_ospi.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
    hal_nand_ospi.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
    hal_nand_ospi.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
    hal_nand_ospi.Init.ClockPrescaler = 4;
    hal_nand_ospi.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_HALFCYCLE; //可能会存在信号延迟
    hal_nand_ospi.Init.ChipSelectBoundary = 0;
    hal_nand_ospi.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE; //可能会存在信号延迟
    hal_nand_ospi.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED; //可能会存在信号延迟
    hal_nand_ospi.Init.MaxTran = 0;
    hal_nand_ospi.Init.Refresh = 0;

    if (HAL_OSPI_Init(&hal_nand_ospi) != HAL_OK) {
        Error_Handler();
    }else{
        LOG_D("OSPI Success");
    }

    // 检查 NAND Flash 初始化是否成功
    if (HAL_OSPI_GetState(&hal_nand_ospi) != HAL_OSPI_STATE_READY) {
        Error_Handler(); // 如果初始化失败，进入错误处理
    }else{
        LOG_D("OSPI ready");
    }

    hal_nand_device.spi_device = (void*)&hal_nand_ospi;
}

void HAL_OSPI_MspInit(OSPI_HandleTypeDef* hospi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hospi->Instance==OCTOSPI1)
  {
  /* USER CODE BEGIN OCTOSPI1_MspInit 0 */

  /* USER CODE END OCTOSPI1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_OSPI;
    PeriphClkInit.OspiClockSelection = RCC_OSPICLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_OSPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**OCTOSPI1 GPIO Configuration
    PA4     ------> OCTOSPI1_NCS
    PB0     ------> OCTOSPI1_IO1
    PB1     ------> OCTOSPI1_IO0
    PE14     ------> OCTOSPI1_IO2
    PE15     ------> OCTOSPI1_IO3
    PB10     ------> OCTOSPI1_CLK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF3_OCTOSPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPI1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* USER CODE BEGIN OCTOSPI1_MspInit 1 */

  /* USER CODE END OCTOSPI1_MspInit 1 */

  }

}

/**
 * @brief HAL_NAND_BSP_Init BSP初始化函数
 */
void HAL_NAND_BSP_Init(void)
{
    MX_OSPI_Init();
}

/***********************************************************************/
/***************************SPI/QSPI 通信函数***************************/
/*******************根据硬件的SPI/QSPI收发函数进行适配********************/
/***********************************************************************/

/**
 * @brief  HAL_SPI_NAND_xfer SPI/QSPI通信函数
 * @param  nand_device      HAL_NAND_Device_t类型，绑定NAND FLASH设备和SPI通信函数
 * @param  cmd              uint8_t类型，指令本身，函数内部会根据指令自动查找指令参数
 * @param  addr             uint32_t类型，指令地址，在NAND中生效的地址
 * @param  din              uint8_t*类型，输入buffer地址，需要接收数据时传入，否则置为NULL
 * @param  dout             uint8_t*类型，输出buffer地址，需要发送数据时传入，否则置为NULL
 * @param  len              uint32_t类型，指令需要发送或接收的数据长度，不用置0
 * @param  xfer_mode        spi_xfer_mode_e类型，暂时未使用，保留作为控制SPI发送方式的参数（即选择FIFO发送还是DMA发送）
 * @return int 成功为0，失败时可以根据平台特性编制<0的错误码
 */
static int HAL_SPI_NAND_xfer(HAL_NAND_Device_t nand_device, uint8_t cmd, uint32_t addr, uint8_t *din, uint8_t *dout, uint32_t len, spi_xfer_mode_e xfer_mode)
{
    OSPI_HandleTypeDef *hospi = (OSPI_HandleTypeDef *)(nand_device.spi_device);
    OSPI_RegularCmdTypeDef sCommand =
    {
        .OperationType = HAL_OSPI_OPTYPE_COMMON_CFG,
        .FlashId = HAL_OSPI_FLASH_ID_1,
        .DQSMode = HAL_OSPI_DQS_DISABLE,
        .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD
    };
    // LOG_D("Model %s", nand_device.nand_flash_info->model);
    NAND_cmd_t *cmd_list = nand_device.nand_flash_info->cmd_list;
    uint8_t cmd_index = 0;

    //LOG_D("array size %d ", nand_device.nand_flash_info->cmd_index_max);

    for(cmd_index = 0; cmd_index < nand_device.nand_flash_info->cmd_index_max; cmd_index++)
    {
        //LOG_D("find CMD %02X", cmd_list[cmd_index].cmd);
        if(cmd == cmd_list[cmd_index].cmd)
        {
            break;
        }
    }

    if(cmd_index >= nand_device.nand_flash_info->cmd_index_max)
    {
        LOG_E("cmd not found");
        return -1;
    }
    else
    {
        sCommand.Instruction = cmd_list[cmd_index].cmd;
        sCommand.InstructionMode = CMD_XFER_MODE(cmd_list[cmd_index].cmd_xfer_mode);
        sCommand.Address = addr;
        sCommand.AddressMode = ADDR_XFER_MODE(cmd_list[cmd_index].addr_xfer_mode);
        sCommand.AddressSize = ADDR_SIZE_MODE(cmd_list[cmd_index].addr_bytes);
        sCommand.DummyCycles = 8 * cmd_list[cmd_index].dummy_bytes;
        sCommand.NbData = len;
        sCommand.DataMode = DATA_XFER_MODE(cmd_list[cmd_index].data_xfer_mode);
    }

    if(HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        LOG_E("cmd send failed");
        return -2;
    }

    if(din != NULL && dout != NULL)
    {
        LOG_E("don't support send and receive at the same time");
        return -3;
    }
    else if(din != NULL)
    {
        if(HAL_OSPI_Receive(hospi, din, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            LOG_E("data receive send failed");
            return -4;
        }
    }
    else if(dout != NULL)
    {
        if(HAL_OSPI_Transmit(hospi, dout, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            LOG_E("data send send failed");
            return -5;
        }
    }

    return 0;
}

/******************system tick 适配，和当前系统对接即可*******************/

static inline unsigned long long get_ticks(void)
{
	return (unsigned long long)rt_tick_get();
}

static inline void udelay(unsigned int us)
{
	unsigned int ms = us/1000;
	rt_thread_mdelay(ms);
	return;
}

static inline void mdelay(unsigned int ms)
{
	rt_thread_mdelay(ms);
	return;
}

/*****************以下函数为固定流程，根据硬件情况微调即可******************/

/**
 * @brief HAL_SPI_NAND_Init 初始化硬件，挂载FLASH并且检查ID是否匹配
 * @param  hal_nand_device  HAL_NAND_Device_t类型，绑定NAND FLASH设备和SPI通信函数
 * @return true 成功
 * @return false 失败
 */
int HAL_SPI_NAND_Init(HAL_NAND_Device_t nand_device)
{
    uint8_t nand_jedec_id[3] = {0};
    //uint8_t addr = 0x00;


    mdelay(100);
    HAL_SPI_NAND_xfer(nand_device, SPINAND_CMD_RESET, 0, NULL, NULL, 0, SPI_FIFO);
    mdelay(100);

    HAL_SPI_NAND_xfer(nand_device, SPINAND_CMD_READ_ID, 0x00, nand_jedec_id, NULL, 2, SPI_FIFO);

    LOG_I("MFR ID: %02X  DEV ID: %02X", nand_jedec_id[0], nand_jedec_id[1]);

    if(hal_nand_device.nand_flash_info->manufacturer_id != nand_jedec_id[0] ||\
    hal_nand_device.nand_flash_info->device_id != nand_jedec_id[1])
    {
        LOG_E("nand id not match");
        return -1;
    }

    return 0;
}

/**
 * @brief HAL_SPI_NAND_Read_ID 读取ID
 * @param  hal_nand_device  HAL_NAND_Device_t类型，绑定NAND FLASH设备和SPI通信函数
 * @param  din_id           uint8_t*类型，ID接收地址
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Read_ID(HAL_NAND_Device_t hal_nand_device, uint8_t *din_id)
{
    return HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_READ_ID, 0x00, din_id, NULL, 2, SPI_FIFO);
}

/**
 * @brief HAL_SPI_NAND_Get_Feature 读取特性寄存器
 * @param  hal_nand_device  NAND设备
 * @param  addr             状态寄存器地址，分别是A0，B0，C0，代表三个八位寄存器
 * @param  din_feature      接收寄存器值的地址，一个uint8_t即可
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Get_Feature(HAL_NAND_Device_t hal_nand_device, uint32_t addr, uint8_t *din_feature)
{

    return HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_GET_FEATURE, addr, din_feature, NULL, 1, SPI_FIFO);
}

/**
 * @brief HAL_SPI_NAND_Set_Feature 写入特性寄存器
 * @param  hal_nand_device  NAND设备
 * @param  addr             状态寄存器地址，分别是A0，B0，C0，代表三个八位寄存器
 * @param  dout_feature     写入寄存器值的地址，一个uint8_t即可
 * @return int 通信函数的返回值
 */ 
int HAL_SPI_NAND_Set_Feature(HAL_NAND_Device_t hal_nand_device, uint32_t addr, uint8_t *dout_feature)
{

    return HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_SET_FEATURE, addr, NULL, dout_feature, 1, SPI_FIFO);
}

/**
 * @brief HAL_SPI_NAND_Read_Status 读取状态寄存器，即地址为0xc0的特性寄存器
 * @param  hal_nand_device  NAND设备
 * @param  status           状态寄存器值，一个uint8_t即可
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Read_Status(HAL_NAND_Device_t hal_nand_device, uint8_t *status)
{
    return HAL_SPI_NAND_Get_Feature(hal_nand_device, REG_STATUS, status);
}

/**
 * @brief HAL_SPI_NAND_Get_Cfg 读取配置寄存器，即地址为0xb0的特性寄存器
 * @param  hal_nand_device  NAND设备
 * @param  cfg              配置寄存器的值，一个uint8_t即可
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Get_Cfg(HAL_NAND_Device_t hal_nand_device, uint8_t *cfg)
{
    return HAL_SPI_NAND_Get_Feature(hal_nand_device, REG_CFG, cfg);
}

/**
 * @brief HAL_SPI_NAND_Set_Cfg 设定配置寄存器值，即地址为0xb0的特性寄存器
 * @param  hal_nand_device  NAND设备
 * @param  cfg              配置寄存器的值，一个uint8_t即可
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Set_Cfg(HAL_NAND_Device_t hal_nand_device, uint8_t *cfg)
{
	return HAL_SPI_NAND_Set_Feature(hal_nand_device, REG_CFG, cfg);
}

/**
 * @brief HAL_SPI_NAND_Lock_Block 配置写保护（OTP）寄存器的值，即地址为0xa0的特性寄存器
 * @param  hal_nand_device  NAND设备
 * @param  lock            OTP寄存器的值，一个uint8_t即可，一般直接写为0，全片解锁即可
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Lock_Block(HAL_NAND_Device_t hal_nand_device, uint8_t *lock)
{
	return HAL_SPI_NAND_Set_Feature(hal_nand_device, REG_BLOCK_LOCK, lock);
}

/**
 * @brief HAL_SPI_NAND_Get_Lock_Block 读取写保护（OTP）寄存器的值，即地址为0xa0的特性寄存器
 * @param  hal_nand_device  NAND设备
 * @param  lock            OTP寄存器的值，一个uint8_t即可，上电默认为0x38，即全片锁定
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Get_Lock_Block(HAL_NAND_Device_t hal_nand_device, uint8_t *lock)
{
	return HAL_SPI_NAND_Get_Feature(hal_nand_device, REG_BLOCK_LOCK, lock);
}

/**
 * @brief HAL_SPI_NAND_Enable_Ecc 使能ECC，每写入512byte数据，会自动触发一次ECC校验，结果存在0xc0特性寄存器中
 * @param  hal_nand_device  NAND设备
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Enable_Ecc(HAL_NAND_Device_t hal_nand_device)
{
    uint8_t cfg = 0;

    HAL_SPI_NAND_Get_Cfg(hal_nand_device, &cfg);
    if ((cfg & CFG_ECC_MASK) == CFG_ECC_ENABLE)
    {
		return 0;
    }

    cfg |= CFG_ECC_ENABLE;
    return HAL_SPI_NAND_Set_Cfg(hal_nand_device, &cfg);
}

/**
 * @brief HAL_SPI_NAND_Enable_Ecc 关闭ECC，每写入512byte数据，会自动触发一次ECC校验，结果存在0xc0特性寄存器中
 * @param  hal_nand_device  NAND设备
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Disable_Ecc(HAL_NAND_Device_t hal_nand_device)
{
    uint8_t cfg = 0;

    HAL_SPI_NAND_Get_Cfg(hal_nand_device, &cfg);
	if ((cfg & CFG_ECC_MASK) == CFG_ECC_ENABLE) {
		cfg &= ~CFG_ECC_ENABLE;
		return HAL_SPI_NAND_Set_Cfg(hal_nand_device, &cfg);
	}
	return 0;

}

/**
 * @brief HAL_SPI_NAND_Check_Ecc_Status
 * @param  status           ECC状态，直接从0xc0特性寄存器中读取，或者使用从HAL_SPI_NAND_Wait中获取的s参数
 * @param  corrected        NAND是否能够纠正错误
 * @param  ecc_error        NAND是否存在ECC错误
 */
void HAL_SPI_NAND_Check_Ecc_Status(uint32_t status, uint32_t *corrected, uint32_t *ecc_error)
{
	unsigned int ecc_status = status & SPI_NAND_AS5F_ECC_MASK;

	*ecc_error = (ecc_status == SPI_NAND_AS5F_ECC_UNCORR);
	switch (ecc_status) {
	case SPI_NAND_AS5F_ECC_0_BIT:
		*corrected = 0;
		break;
	case SPI_NAND_AS5F_ECC_SHIFT:
		*corrected = 1;
		break;
	case SPI_NAND_AS5F_ECC_MAX_BIT:
		*corrected = 3;
		break;
	}
}

// 使能 QE, 配置 OTP 寄存器的 QE bit，打开 QSPI 功能特性
int HAL_SPI_NAND_Enable_QE(HAL_NAND_Device_t hal_nand_device)
{
    uint8_t cfg = 0;

    HAL_SPI_NAND_Get_Cfg(hal_nand_device, &cfg);
    if ((cfg & CFG_OTP_QE_MASK) == CFG_OTP_QE_ENABLE)
    {
		return 0;
    }

    cfg |= CFG_OTP_QE_ENABLE;
    return HAL_SPI_NAND_Set_Cfg(hal_nand_device, &cfg);
}

// 关闭 QE, 配置 OTP 寄存器的 QE bit，关闭 QSPI 功能特性
int HAL_SPI_NAND_Disable_QE(HAL_NAND_Device_t hal_nand_device)
{
    uint8_t cfg = 0;

    HAL_SPI_NAND_Get_Cfg(hal_nand_device, &cfg);
	if ((cfg & CFG_OTP_QE_MASK) == CFG_OTP_QE_ENABLE) {
		cfg &= ~CFG_OTP_QE_ENABLE;
		return HAL_SPI_NAND_Set_Cfg(hal_nand_device, &cfg);
	}
	return 0;
}

/**
 * @brief HAL_SPI_NAND_Wait 通过读取NAND的状态寄存器（0xc0），获取NAND是否处于BUSY状态，直到操作完成时退出
 * @param  status           状态寄存器读取的值，可以判断BUSY结束瞬间，进行的操作是否成功
 * @return int              需要配置超时退出时，使用负值标记超时退出，否则恒为0
 */
int HAL_SPI_NAND_Wait(HAL_NAND_Device_t hal_nand_device, uint8_t *s)
{
	unsigned long long start = get_ticks();
	uint8_t status;
    uint32_t ret = -1;
	/* set timeout to 1 second */
	// int timeout = start + 100;
	// unsigned long ret = -ETIMEDOUT;//可以使用ticks比较的方法使能超时退出

	while (1) 
    {
		HAL_SPI_NAND_Read_Status(hal_nand_device, &status);
        LOG_I("wait status Read %02x", status);
		if ((status & STATUS_OIP_MASK) == STATUS_READY) {
			ret = 0;
			goto out;
		}
        else
        {
            LOG_I("busy");
        }
        mdelay(1);  // delay 1ms
	}
out:
	if (s)
		*s = status;

	return ret;
}

/**
 * @brief HAL_SPI_NAND_Write_Enable 使能写操作，写数据之前必须使能该操作
 * @param  hal_nand_device  NAND设备
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Write_Enable(HAL_NAND_Device_t hal_nand_device)
{
    return  HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_WR_ENABLE, 0x00, NULL, NULL, 0, SPI_FIFO);
}

/**
 * @brief HAL_SPI_NAND_Write_Disable 停止写操作
 * @param  hal_nand_device  NAND设备
 * @return int 通信函数的返回值
 */
int HAL_SPI_NAND_Write_Disable(HAL_NAND_Device_t hal_nand_device)
{
    return  HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_WR_DISABLE, 0x00, NULL, NULL, 0, SPI_FIFO);
}

/**
 * @brief HAL_SPI_NAND_Read_Page_To_Cache 读取NAND数据到缓存区（该缓存时NAND自带的，一般和page的物理大小对齐，不需要消耗本机的heap）
 * @param  hal_nand_device  NAND设备
 * @param  page_addr        page 地址,假设一个block有64个page，那么page就是0~63（低六位），所属的block数就是后续的高位，发送固定为24个bit，不足的在低位补0（直接uint32表示即可）
 * @return int 通信函数的返回值 
 */
int HAL_SPI_NAND_Read_Page_To_Cache(HAL_NAND_Device_t hal_nand_device, uint32_t page_addr)
{
    return HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_PAGE_READ, page_addr, NULL, NULL, 0, SPI_FIFO);
}

/**
 * @brief HAL_SPI_NAND_Read_From_Cache 读取NAND缓存区数据
 * @param  hal_nand_device  NAND设备
 * @param  page_addr        page 地址,实际上没用
 * @param  column           列地址，即page内特定bit的地址，可以直接操作到bit
 * @param  len              数据长度
 * @param  din_buf          数据buffer
 * @return int通信函数的返回值 
 */
int HAL_SPI_NAND_Read_From_Cache(HAL_NAND_Device_t hal_nand_device, uint32_t page_addr, uint32_t column, size_t len, uint8_t *din_buf)
{
    HAL_SPI_NAND_Disable_QE(hal_nand_device);
    return HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_READ_FROM_CACHE, column, din_buf, NULL, len, SPI_FIFO);
}

/**
 * @brief HAL_SPI_NAND_Program_Data_To_Cache 将数据写入缓存区
 * @param  hal_nand_device  NAND设备
 * @param  page_addr        page 地址,实际上没用
 * @param  column           列地址，即page内特定bit的地址，可以直接操作到bit
 * @param  len              数据长度
 * @param  dout_buf         数据buffer
 * @param  clr_cache        是否清空缓存区，1为清空；0为不清空，但是改写cache对应位置的数据
 * @return int通信函数的返回值 
 */
int HAL_SPI_NAND_Program_Data_To_Cache(HAL_NAND_Device_t hal_nand_device, uint32_t page_addr, uint32_t column, size_t len, uint8_t *dout_buf, bool clr_cache)
{
    uint8_t cmd = 0;

    if (clr_cache) {
        cmd = SPINAND_CMD_PROG_LOAD;
    } else {
        cmd = SPINAND_CMD_PROG_LOAD_RDM_DATA;
    }
    HAL_SPI_NAND_Disable_QE(hal_nand_device);
    return HAL_SPI_NAND_xfer(hal_nand_device, cmd, column, NULL, dout_buf, len, SPI_FIFO);
}
    
#ifdef NAND_FLASH_QSPI_SUPPORT
// 读取NAND缓存区数据
int HAL_QSPI_NAND_Read_From_Cache(HAL_NAND_Device_t hal_nand_device, uint32_t page_addr, uint32_t column, size_t len, uint8_t *din_buf)
{
    
    HAL_SPI_NAND_Enable_QE(hal_nand_device);
    return HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_READ_FROM_CACHE_X4, column, din_buf, NULL, len, SPI_FIFO);
}

// 将数据写入缓存区
int HAL_QSPI_NAND_Program_Data_To_Cache(HAL_NAND_Device_t hal_nand_device, uint32_t page_addr, uint32_t column, size_t len, uint8_t *dout_buf, bool clr_cache)
{
    uint8_t cmd = 0;

    if (clr_cache) {
        cmd = SPINAND_CMD_PROG_LOAD_X4;
    } else {
        cmd = SPINAND_CMD_PROG_LOAD_RDM_DATA_X4;
    }
    HAL_SPI_NAND_Enable_QE(hal_nand_device);
    
    return HAL_SPI_NAND_xfer(hal_nand_device, cmd, column, NULL, dout_buf, len, SPI_FIFO);
}
#endif

/**
 * @brief HAL_SPI_NAND_Program_Execute 写入缓存区数据到NAND
 * @param  hal_nand_device  NAND设备
 * @param  page_addr        page 地址
 * @return int 
 */
int HAL_SPI_NAND_Program_Execute(HAL_NAND_Device_t hal_nand_device , uint32_t page_addr)
{
	return HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_PROG_EXC, page_addr, NULL, NULL, 0, SPI_FIFO);
}

/**
 * @brief HAL_SPI_NAND_Erase_Block 擦除NAND的block
 * @param  hal_nand_device  NAND设备
 * @param  block_addr       block地址，和page地址格式一样的，只是处理时会忽略page地址，只按block操作
 * @return int 通信函数的返回值 
 */
int HAL_SPI_NAND_Erase_Block(HAL_NAND_Device_t hal_nand_device, uint32_t block_addr)
{
    return HAL_SPI_NAND_xfer(hal_nand_device, SPINAND_CMD_BLK_ERASE, block_addr, NULL, NULL, 0, SPI_FIFO);
}

/**
 * @brief HAL_SPI_NAND_Internal_Data_Move 内部数据移动，将NAND的某页数据移动到另一个页 
 * @param  hal_nand_device  NAND设备
 * @param  page_src_addr    源数据的页地址
 * @param  page_dst_addr    移动的目标页地址
 * @param  offset           如果移动过程中需要改写数据，这就是改写的列地址，不改写就置空
 * @param  buf              数据buf
 * @param  len              数据长度
 * @return int 
 */
int HAL_SPI_NAND_Internal_Data_Move(HAL_NAND_Device_t hal_nand_device, uint32_t page_src_addr,  \
    uint32_t page_dst_addr, uint32_t offset, uint8_t *buf, size_t len)
{
	uint8_t status;

	HAL_SPI_NAND_Read_Page_To_Cache(hal_nand_device, page_src_addr);
	if (HAL_SPI_NAND_Wait(hal_nand_device, &status)) 
    {
		return -ETIMEDOUT;
	}
	HAL_SPI_NAND_Write_Enable(hal_nand_device);
	if (buf != NULL)
    {
#ifdef NAND_FLASH_QSPI_SUPPORT
        HAL_QSPI_NAND_Program_Data_To_Cache(hal_nand_device, page_dst_addr, offset, len, buf, 0);
#else
        HAL_SPI_NAND_Program_Data_To_Cache(hal_nand_device, page_dst_addr, offset, len, buf, 0);
#endif
    }

	HAL_SPI_NAND_Program_Execute(hal_nand_device, page_dst_addr);
	if (HAL_SPI_NAND_Wait(hal_nand_device, &status)) 
    {
		return -ETIMEDOUT;
	}
	if ((status & STATUS_P_FAIL_MASK) == STATUS_P_FAIL) 
    {
		return -EIO;
	}

	return 0;
}

/**
 * @brief HAL_SPI_NAND_Check_Bad_Block 检查坏块
 * @param  hal_nand_device  NAND设备
 * @param  blk_addr        需要检查的块地址，传入页地址也可，会自动忽略page addr
 * @param  buf              数据buf
 * @return int 0：非坏块 1：坏块
 */
int HAL_SPI_NAND_Check_Bad_Block(HAL_NAND_Device_t hal_nand_device, uint32_t blk_addr)
{
    uint8_t bad_blk_flag[2] = {0};
    uint32_t blk_only_addr_mask = ~((1 << 6) - 1);
    
    blk_addr &= blk_only_addr_mask;

    HAL_SPI_NAND_Read_Page_To_Cache(hal_nand_device, blk_addr);

    HAL_SPI_NAND_Wait(hal_nand_device, NULL);
#ifdef NAND_FLASH_QSPI_SUPPORT
    HAL_QSPI_NAND_Read_From_Cache(hal_nand_device, blk_addr, hal_nand_device.nand_flash_info->memory_info->page_size, 2, bad_blk_flag);
#else
    HAL_SPI_NAND_Read_From_Cache(hal_nand_device, blk_addr, hal_nand_device.nand_flash_info->memory_info->page_size, 2, bad_blk_flag);
#endif
    LOG_D(
        "HAL_SPI_NAND_Check_Bad_Block bad_blk_flag[0]=0x%02X, bad_blk_flag[1]=0x%02X",
        bad_blk_flag[0], bad_blk_flag[1]
    );

    return (bad_blk_flag[0] != 0xFF) || (bad_blk_flag[1] != 0xFF);
}

/**
 * @brief HAL_SPI_NAND_Mark_Bad_Block 标记坏块
 * @param  hal_nand_device  NAND设备
 * @param  blk_addr        需要检查的块地址，传入页地址也可，会自动忽略page addr
 * @param  buf              数据buf
 * @return int 0：标记成功，否则失败
 */
int HAL_SPI_NAND_Mark_Bad_Block(HAL_NAND_Device_t hal_nand_device, uint32_t blk_addr)
{
	uint8_t status;
    uint8_t bad_blk_flag[2] = {0x00, 0x00};//需要标记的位置
    uint32_t blk_only_addr_mask = ~((1 << 6) - 1);
    uint32_t is_bad_blk = 0;
    
    blk_addr &= blk_only_addr_mask;
    is_bad_blk = HAL_SPI_NAND_Check_Bad_Block(hal_nand_device, blk_addr);
    if(is_bad_blk)
    {
        return 0;//如果已经被标记，返回0
    }
    

    HAL_SPI_NAND_Write_Enable(hal_nand_device);
    HAL_SPI_NAND_Erase_Block(hal_nand_device, blk_addr);
    HAL_SPI_NAND_Wait(hal_nand_device, NULL);
    
    HAL_SPI_NAND_Write_Enable(hal_nand_device);
    #ifdef NAND_FLASH_QSPI_SUPPORT
    HAL_QSPI_NAND_Program_Data_To_Cache(hal_nand_device, blk_addr, hal_nand_device.nand_flash_info->memory_info->page_size, 2, bad_blk_flag, true);
    #else
    HAL_SPI_NAND_Program_Data_To_Cache(hal_nand_device, blk_addr, hal_nand_device.nand_flash_info->memory_info->page_size, 2, bad_blk_flag, true);
    #endif
    HAL_SPI_NAND_Program_Execute(hal_nand_device, blk_addr);
    HAL_SPI_NAND_Wait(hal_nand_device, &status);                    
    
	return ((status & STATUS_P_FAIL_MASK) == STATUS_P_FAIL);
}

#if 0
/*测试函数，忽略*/
void hal_spi_test_oneline(void)
{
    uint8_t id[2] = {0};
    uint32_t page_addr_for_test = 0x100;
    uint32_t test_column = 0x100;
    uint8_t status = 0;
    uint8_t cfg = 0;
    // char dout_buff[4096] = {0};
    uint8_t din_buff[16] = {0};
    uint8_t lock_map = 0;
    nand_to_stm32();
    HAL_NAND_BSP_Init();
    if(HAL_SPI_NAND_Init(hal_nand_device) != 0)
    {
        return;
    }

    HAL_SPI_NAND_Read_ID(hal_nand_device, id);

    LOG_I("READ ID: %02X %02X", id[0], id[1]);
    HAL_SPI_NAND_Get_Lock_Block(hal_nand_device, &lock_map);
    LOG_I("DEFAULT lock %02X", lock_map);
    lock_map = 0x00;
    HAL_SPI_NAND_Lock_Block(hal_nand_device, &lock_map);
    lock_map = 0;
    HAL_SPI_NAND_Get_Lock_Block(hal_nand_device, &lock_map);
    LOG_I("Now lock %02X", lock_map);


    status = 0;

    // HAL_SPI_NAND_Get_Cfg(hal_nand_device, &cfg);

    // cfg = cfg | 0x01;
    // HAL_SPI_NAND_Set_Cfg(hal_nand_device, &cfg);
    // cfg=0;
    // HAL_SPI_NAND_Get_Cfg(hal_nand_device, &cfg);
    LOG_I("cfg %02X", cfg);

    //memcpy(dout_buff, "ABCDEFG", 8);


    
    HAL_SPI_NAND_Write_Enable(hal_nand_device);
    HAL_SPI_NAND_Read_Status(hal_nand_device,&status);

    LOG_I("status before erase %02X", status);
    HAL_SPI_NAND_Erase_Block(hal_nand_device, page_addr_for_test);//擦除第10块

    HAL_SPI_NAND_Wait(hal_nand_device, &status);

    // LOG_D("Erase END");

    // HAL_SPI_NAND_Read_Page_To_Cache(hal_nand_device, page_addr_for_test);

    // HAL_SPI_NAND_Wait(hal_nand_device, &status);

    // HAL_SPI_NAND_Read_From_Cache(hal_nand_device, page_addr_for_test, test_column, 16, din_buff);

    // LOG_D("Read END 1 %02X %02X %02X %02X %02X %02X %02X %02X",\
    // din_buff[0], din_buff[1], din_buff[2], din_buff[3], din_buff[4], din_buff[5], din_buff[6], din_buff[7]);

    HAL_SPI_NAND_Write_Enable(hal_nand_device);

    HAL_SPI_NAND_Read_Status(hal_nand_device,&status);
    LOG_I("status %02X", status);
    status = 0;


    HAL_SPI_NAND_Program_Data_To_Cache(hal_nand_device, page_addr_for_test, test_column, 8, (uint8_t *)"ABCDEFG", true);

    HAL_SPI_NAND_Read_From_Cache(hal_nand_device, page_addr_for_test, test_column, 8, din_buff);

    LOG_I("Read END 1 %02X %02X %02X %02X %02X %02X %02X %02X \r\n str %s",\
    din_buff[0], din_buff[1], din_buff[2], din_buff[3], din_buff[4], din_buff[5], din_buff[6], din_buff[7], din_buff);

    HAL_SPI_NAND_Program_Execute(hal_nand_device, page_addr_for_test);

    HAL_SPI_NAND_Wait(hal_nand_device, &status);

    LOG_I("Program END");

    HAL_SPI_NAND_Read_Page_To_Cache(hal_nand_device, page_addr_for_test);

    HAL_SPI_NAND_Wait(hal_nand_device, &status);

    //memset(din_buff, 0x00, sizeof(din_buff));

    HAL_SPI_NAND_Read_From_Cache(hal_nand_device, page_addr_for_test, test_column, 8, din_buff);

    LOG_I("Read END 2 %02X %02X %02X %02X %02X %02X %02X %02X \r\n str %s",\
    din_buff[0], din_buff[1], din_buff[2], din_buff[3], din_buff[4], din_buff[5], din_buff[6], din_buff[7], din_buff);

}

// MSH_CMD_EXPORT(hal_spi_test_oneline, HAL SPI TEST ONELINE);


void flashspeed(int page_start, int page_cnt, int page_size)
{
    int index, length;
    char *buff_ptr;
    int total_length = 0;
    rt_tick_t tick;
    uint8_t status = 0;
    uint32_t ecc_err, ecc_corrected;
    int err_occ = 0;

    total_length = page_cnt * page_size;

    buff_ptr = rt_malloc(page_size);
    if (buff_ptr == RT_NULL)
    {
        rt_kprintf("no memory\n");
        return;
    }

    /* prepare write data */
    for (index = 0; index < page_size; index++)
    {
        buff_ptr[index] = index%256;
    }
    index = 0;
    /* get the beginning tick */
    tick = rt_tick_get();
    while (index < page_cnt)
    {
        if((index % 64 ) == 0)
        {
            // 擦block
            HAL_SPI_NAND_Write_Enable(hal_nand_device);
            HAL_SPI_NAND_Erase_Block(hal_nand_device, index);
            HAL_SPI_NAND_Wait(hal_nand_device, &status);
            if((status & STATUS_E_FAIL_MASK) == STATUS_E_FAIL)
            {
                LOG_E(" SPI NAND Erase error");
                err_occ = 1;
                break;
            }  
        }
        HAL_SPI_NAND_Write_Enable(hal_nand_device);
        // HAL_QSPI_NAND_Program_Data_To_Cache(hal_nand_device, index, index, page_size, (uint8_t *)buff_ptr, false);
        HAL_SPI_NAND_Program_Data_To_Cache(hal_nand_device, index, index, page_size, (uint8_t *)buff_ptr, false);
        HAL_SPI_NAND_Wait(hal_nand_device, &status);
        HAL_SPI_NAND_Program_Execute(hal_nand_device, index);
        HAL_SPI_NAND_Wait(hal_nand_device, &status);
        if((status & STATUS_P_FAIL_MASK) == STATUS_P_FAIL)
        {
            LOG_E(" SPI NAND Program error");
            err_occ = 1;
            break;
        }   

        index ++;
    }
    tick = rt_tick_get() - tick;

    if(err_occ == 0)
    {
        /* calculate write speed */
        rt_kprintf("\nflash write speed: %d byte/s\ntotal_length: %d bytes, cost: %d s\n", 
            total_length / tick * RT_TICK_PER_SECOND, total_length, tick / RT_TICK_PER_SECOND);
    }
    else
    {
        /* calculate write speed */
        rt_kprintf("flash write speed: %d byte/s\n", 0);
    }
    
    err_occ = 0;
    tick = rt_tick_get();
    index = 0;
    while (index < page_cnt)
    {

        HAL_SPI_NAND_Read_Page_To_Cache(hal_nand_device, index);
        HAL_SPI_NAND_Wait(hal_nand_device, &status);
        HAL_SPI_NAND_Check_Ecc_Status((uint32_t)status, &ecc_corrected, &ecc_err);
        if(ecc_err)
        {
            LOG_E(" SPI NAND Read Ecc error");
            err_occ = 1;
            break;		
        }
        // HAL_QSPI_NAND_Read_From_Cache(hal_nand_device, index, index, page_size, buff_ptr);
        HAL_SPI_NAND_Read_From_Cache(hal_nand_device, index, index, page_size, buff_ptr);

        index ++;
    }
    tick = rt_tick_get() - tick;

    if(err_occ == 0)
    {
        rt_kprintf("\nflash read speed: %d byte/s\ntotal_length: %d bytes, cost: %d s\n", 
            total_length /tick * RT_TICK_PER_SECOND, total_length, tick / RT_TICK_PER_SECOND);
    }
    else
    {
        rt_kprintf("flash read speed: %d byte/s\n", 0);
    }

    index = 0;
    /* get the beginning tick */
    tick = rt_tick_get();
    while (index < page_cnt)
    {
        if((index % 64 ) == 0)
        {
            // 擦block
            HAL_SPI_NAND_Write_Enable(hal_nand_device);
            HAL_SPI_NAND_Erase_Block(hal_nand_device, index);
            HAL_SPI_NAND_Wait(hal_nand_device, &status);
            if((status & STATUS_E_FAIL_MASK) == STATUS_E_FAIL)
            {
                LOG_E(" SPI NAND Erase error");
                err_occ = 1;
                break;
            }  
        }
        index += 64;
    }
    tick = rt_tick_get() - tick;

    if(err_occ == 0)
    {
        /* calculate write speed */
        rt_kprintf("\nflash Erase speed: %d byte/s\ntotal_length: %d bytes, cost: %d s\n", 
            total_length / tick * RT_TICK_PER_SECOND, total_length, tick / RT_TICK_PER_SECOND);
    }
    else
    {
        /* calculate write speed */
        rt_kprintf("flash write speed: %d byte/s\n", 0);
    }
    

    /* close file and release memory */
    rt_free(buff_ptr);
}


#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(flashspeed, perform flash rw test);

static void cmd_flashspeed(int argc, char *argv[])
{
    int page_start;
    int page_cnt;
    int page_size;

    if(argc == 4)
    {
        page_start = atoi(argv[1]);
        page_cnt = atoi(argv[2]);
        page_size = atoi(argv[3]);
    }
    else if(argc == 2)
    {
        page_start = 0;
        page_cnt = 1024;
        page_size = 4*1024;
    }
    else
    {
       rt_kprintf("Usage:\flashspeed [page_start] [page_cnt] [page_size]\n");
       rt_kprintf("flashspeed [page 0] with default length 4MB and page size 4k\n");
       return;
    }
    flashspeed(page_start, page_cnt, page_size);
}
MSH_CMD_EXPORT_ALIAS(cmd_flashspeed, flashspeed, test flash system rw speed);
#endif /* RT_USING_FINSH */
#endif