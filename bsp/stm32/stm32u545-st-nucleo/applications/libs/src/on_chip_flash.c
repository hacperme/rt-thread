/*
 * @FilePath: on_chip_flash.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-12 15:15:33
 * @copyright : Copyright (c) 2024
 */
#include "on_chip_flash.h"

#define DBG_SECTION_NAME "ON_CHIP_FLASH"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

const struct fal_partition *bl_part;
const struct fal_partition *app_part;
static rt_uint32_t FAL_PAGE_SIZE = 0x2000;
static rt_uint8_t PAGE_BUFF[0x2000] = {0};

#define FLASH_USER_START_ADDR ((uint32_t)0x08060000)
#define FLASH_USER_END_ADDR   (FLASH_USER_START_ADDR + FLASH_PAGE_SIZE - 1)
uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
uint32_t Address = 0, PageError = 0;
uint32_t QuadWord[4] = {0x12345678, 0x87654321, 0x12344321, 0x56788765};
static FLASH_EraseInitTypeDef EraseInitStruct;

extern uint32_t GetPage(uint32_t Addr);
extern uint32_t GetBank(uint32_t Addr);

rt_err_t find_app_partition(void)
{
    rt_err_t res = RT_ERROR;
    app_part = fal_partition_find("app");
    res = app_part == RT_NULL ? RT_ERROR : RT_EOK;
    LOG_D("fal_partition_find app %s", res == RT_EOK ? "success" : "failed");
    return res;
}

rt_err_t read_app_partition(rt_uint32_t addr, rt_uint8_t *buf, rt_int32_t size)
{
    rt_err_t res = RT_ERROR;
    rt_int32_t ret;
    ret = fal_partition_read(app_part, addr, buf, size);
    res = ret == size ? RT_EOK : RT_ERROR;
    LOG_D("fal_partition_read %s ret=%d", res == RT_EOK ? "success" : "failed", ret);
    // if (res == RT_EOK)
    // {
    //     char msgs[128];
    //     char msg[8];
    //     char index=0;
    //     for (rt_uint8_t i = 0; i < size; i++)
    //     {
    //         rt_sprintf(msg, "0x%02X", buf[i]);
    //         rt_memcpy(msgs + index, msg, rt_strlen(msg));
    //         index += rt_strlen(msg);
    //     }
    //     LOG_D("fal_partition_read addr=0x%02X, size=%d, buf=%s", addr, size, msgs);
    // }
    return res;
}

rt_err_t erase_app_partition(rt_uint32_t addr, rt_int32_t size)
{
    rt_err_t res = RT_ERROR;
    rt_int32_t ret;
    ret = fal_partition_erase(app_part, addr, size);
    res = ret == size ? RT_EOK : RT_ERROR;
    LOG_D("fal_partition_erase %s ret=%d", res == RT_EOK ? "success" : "failed", ret);
    return res;
}

rt_err_t write_app_partition(rt_uint32_t addr, rt_uint8_t *buf, rt_int32_t size)
{
    rt_err_t res = RT_ERROR;
    rt_int32_t ret;
    LOG_D("write_app_partition addr=0x%02X buf=0x%02X size=0x%02X");
    ret = fal_partition_write(app_part, addr, buf, size);
    res = ret == size ? RT_EOK : RT_ERROR;
    LOG_D("fal_partition_read %s ret=%d", res == RT_EOK ? "success" : "failed", ret);
    return res;
}

rt_err_t test_on_chip_flash(void)
{
    rt_err_t res = RT_ERROR;

    res = find_app_partition();
    LOG_D("find_app_partition %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    rt_uint32_t addr = 0;
    rt_int32_t size = 16;

    // rt_uint8_t read_buf[size];
    // rt_memset(read_buf, 0, size);
    res = read_app_partition(addr, PAGE_BUFF, FAL_PAGE_SIZE);
    LOG_D("read_app_partition %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = erase_app_partition(addr, FAL_PAGE_SIZE);
    LOG_D("erase_app_partition %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    // rt_uint8_t write_buf[size] ;
    // rt_memset(write_buf, 0x11, size);
    // LOG_D("write_buf=0x%02X", write_buf);

    rt_memset(PAGE_BUFF, 0x11, FAL_PAGE_SIZE);
    LOG_D("PAGE_BUFF=0x%02X", PAGE_BUFF);
    res = write_app_partition(addr, PAGE_BUFF, FAL_PAGE_SIZE);
    LOG_D("write_app_partition %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    return res;
}

rt_err_t test_on_chip_flash_hal(void)
{

    /* Disable instruction cache prior to internal cacheable memory update */
    if (HAL_ICACHE_Disable() != HAL_OK)
    {
        Error_Handler();
    }

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    /* Get the 1st page to erase */
    FirstPage = GetPage(FLASH_USER_START_ADDR);
    LOG_D("FirstPage %d", FirstPage);

    /* Get the number of pages to erase from 1st page */
    NbOfPages = GetPage(FLASH_USER_END_ADDR) - FirstPage + 1;
    LOG_D("NbOfPages %d", NbOfPages);

    /* Get the bank */
    BankNumber = GetBank(FLASH_USER_START_ADDR);
    LOG_D("BankNumber %d", BankNumber);

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = BankNumber;
    EraseInitStruct.Page        = FirstPage;
    EraseInitStruct.NbPages     = NbOfPages;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
    /*
        Error occurred while page erase.
        User can add here some code to deal with this error.
        PageError will contain the faulty page and then to know the code error on this page,
        user can call function 'HAL_FLASH_GetError()'
    */
    /* Infinite loop */
    while (1)
    {
        Error_Handler();
    }
    }
    LOG_D("HAL_FLASHEx_Erase OK.");

    /* Program the user Flash area word by word
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    Address = FLASH_USER_START_ADDR;
    LOG_D("Address=0x%08X", Address);

    while (Address < FLASH_USER_END_ADDR)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, ((uint32_t)Address), ((uint32_t)QuadWord)) == HAL_OK)
        {
            Address = Address + 16; /* increment to next quad word*/
        }
        else
        {
            /* Error occurred while writing data in Flash memory.
                User can add here some code to deal with this error */
            while (1)
            {
                Error_Handler();
            }
        }
    }

    /* Lock the Flash to disable the flash control register access (recommended
        to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    /* Re-enable instruction cache */
    if (HAL_ICACHE_Enable() != HAL_OK)
    {
        Error_Handler();
    }
}

MSH_CMD_EXPORT(test_on_chip_flash, test on chip flash);
MSH_CMD_EXPORT(test_on_chip_flash_hal, test on chip flash hal);
