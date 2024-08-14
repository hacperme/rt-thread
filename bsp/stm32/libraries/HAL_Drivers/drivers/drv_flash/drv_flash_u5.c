/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-5      SummerGift   first version
 */

#include <rtconfig.h>
#include <rtdef.h>

#ifdef BSP_USING_ON_CHIP_FLASH
#include "drv_config.h"
#include "drv_flash.h"
#include <board.h>

#if defined(RT_USING_FAL)
#include "fal.h"
#endif

//#define DRV_DEBUG
#define LOG_TAG                "drv.flash"
#include <drv_log.h>

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
uint32_t GetPage(uint32_t Addr)
{
    uint32_t page = 0;

    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
        /* Bank 1 */
        page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
    }
    else
    {
        /* Bank 2 */
        page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
    }

    return page;
}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
uint32_t GetBank(uint32_t Addr)
{
    uint32_t bank = 0;
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
        bank = FLASH_BANK_1;
    }
    else
    {
        bank = FLASH_BANK_2;
    }
    return bank;
}

/**
  * @brief  Check program operation.
  * param StartAddress Area start address
  * param EndAddress Area end address
  * param Data Expected data
  * @retval FailCounter
  */
static uint32_t Check_Program(uint32_t StartAddress, uint32_t *Data)
{
  uint32_t Address;
  uint32_t index, FailCounter = 0;
  uint32_t data32;

  /* check the user Flash area word by word */
  Address = StartAddress;

  while(Address < STM32_FLASH_END_ADDRESS)
  {
    for(index = 0; index<4; index++)
    {
      data32 = *(uint32_t*)Address;
      if(data32 != Data[index])
      {
        FailCounter++;
        return FailCounter;
      }
      Address +=4;
    }
  }
  return FailCounter;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
int stm32_flash_read(rt_uint32_t addr, rt_uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E("read outrange flash size! addr is (%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */

int stm32_flash_write(rt_uint32_t addr, const rt_uint8_t *buf, size_t size)
{
    size_t i, j, k, n;
    rt_err_t result = 0;
    uint32_t write_data[4] = {0};
    uint32_t temp_data = 0;
    LOG_D("stm32_flash_write addr=0x%08X, buf=0x%08X, size=0x%08X", addr, buf, size);

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E("ERROR: write outrange flash size! addr is (0x%p)\n", (void*)(addr + size));
        return -RT_EINVAL;
    }

    if(addr % 16 != 0)
    {
        LOG_E("write addr must be 16-byte alignment");
        return -RT_EINVAL;
    }

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);

    if (size < 1)
    {
        return -RT_ERROR;
    }

    for (i = 0; i < size;)
    {
        for (k = 0; k < 4; k++)
        {
            n = (size - i) < 4 ? (size - i) : 4;
            for (j = 0; j < n; j++, i++)
            {
                temp_data = *buf;
                write_data[k] = (write_data[k]) | (temp_data << 8 * j);
                buf ++;
            }
        }
        LOG_D(
            "write_data_addr=0x%08X write_data[0]=0x%08X write_data[1]=0x%08X write_data[2]=0x%08X write_data[3]=0x%08X",
            write_data, write_data[0], write_data[1], write_data[2], write_data[3]
        );

        /* write data */
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, (uint32_t)addr, (uint32_t)write_data) == HAL_OK)
        {
            /* Check the written value */
            if (Check_Program((uint32_t)addr, (uint32_t *)write_data) != 0)
            {
                LOG_E("ERROR: write data != read data\n");
                result = -RT_ERROR;
                goto __exit;
            }
        }
        else
        {
            result = -RT_ERROR;
            goto __exit;
        }

        temp_data = 0;
        rt_memset(write_data, 0, 4);

        addr += 16;
    }

__exit:
    HAL_FLASH_Lock();

    if (result != 0)
    {
        return result;
    }

    return size;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
int stm32_flash_erase(rt_uint32_t addr, size_t size)
{
    rt_err_t result = RT_EOK;
    uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
    uint32_t PAGEError = 0;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E("ERROR: erase outrange flash size! addr is (0x%p)\n", (void*)(addr + size));
        return -RT_EINVAL;
    }

    /*Variable used for Erase procedure*/
    FLASH_EraseInitTypeDef EraseInitStruct;
    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Get the 1st page to erase */
    FirstPage = GetPage((uint32_t)addr);
    /* Get the number of pages to erase from 1st page */
    NbOfPages = GetPage((uint32_t)addr + size - 1) - FirstPage + 1;
    /* Get the bank */
    BankNumber = GetBank((uint32_t)addr);
    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    // EraseInitStruct.TypeErase   = FLASH_TYPEERASE_MASSERASE;
    EraseInitStruct.Banks       = BankNumber;
    EraseInitStruct.Page        = FirstPage;
    EraseInitStruct.NbPages     = NbOfPages;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        result = -RT_ERROR;
        goto __exit;
    }

__exit:
    HAL_FLASH_Lock();

    if (result != RT_EOK)
    {
        return result;
    }

    LOG_D("erase done: addr (%p), size %d", (void*)addr, size);
    return size;
}

#if defined(RT_USING_FAL)

static int fal_flash_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_flash_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_flash_erase(long offset, size_t size);

const struct fal_flash_dev stm32u5_onchip_flash = {
    "onchip_flash",
    STM32_FLASH_START_ADRESS,
    STM32_FLASH_SIZE,
    FLASH_PAGE_SIZE,
    {NULL, fal_flash_read, fal_flash_write, fal_flash_erase}
};

static int fal_flash_read(long offset, rt_uint8_t *buf, size_t size)
{
    return stm32_flash_read(stm32u5_onchip_flash.addr + offset, buf, size);
}

static int fal_flash_write(long offset, const rt_uint8_t *buf, size_t size)
{
    return stm32_flash_write(stm32u5_onchip_flash.addr + offset, buf, size);
}

static int fal_flash_erase(long offset, size_t size)
{
    return stm32_flash_erase(stm32u5_onchip_flash.addr + offset, size);
}

INIT_BOARD_EXPORT(fal_init);

#endif
#endif /* BSP_USING_ON_CHIP_FLASH */
