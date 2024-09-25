#include "nand.h"
#include "error.h"
#include "sys/errno.h"
#include "hal_spi_nand_driver.h"
#include "hal_nand_device_info.h"
#include <math.h>
#include <rtthread.h>

#define DRV_DEBUG
#define LOG_TAG "NAND_TO_DHARA"
#include <drv_log.h>

#ifndef __containerof

#include <stddef.h>
#define __containerof(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
    
#endif

typedef struct dhara_bind_spi_nand
{
    uint8_t is_binded;
    struct dhara_nand *dhara_nand;
    HAL_NAND_Device_t *spi_nand_device;	
} dhara_bind_spi_nand_t;

#define DHARA_NAND_BIND_SPI_NAND_INDEX_MAX (3)

static dhara_bind_spi_nand_t dhara_bind_spi_nand_list[DHARA_NAND_BIND_SPI_NAND_INDEX_MAX] = {0};

int dhara_bind_with_spi_nand_device(struct dhara_nand *n, HAL_NAND_Device_t* spi_nand_device)
{
    int index=0;
    for(index=0; index < DHARA_NAND_BIND_SPI_NAND_INDEX_MAX; index++)
    {
        if(dhara_bind_spi_nand_list[index].is_binded == 0)
        {
            break;
        }
        else if(index >= (DHARA_NAND_BIND_SPI_NAND_INDEX_MAX - 1))
        {
            return -1;
        }		
    }

    dhara_bind_spi_nand_list[index].is_binded = 1;

    if(n->num_blocks == 0) n->num_blocks = spi_nand_device->nand_flash_info->memory_info->block_num_per_chip;
    if(n->log2_page_size == 0) n->log2_page_size = (uint8_t)log2((double)spi_nand_device->nand_flash_info->memory_info->page_size);
    if(n->log2_ppb == 0) n->log2_ppb = (uint8_t)log2((double)spi_nand_device->nand_flash_info->memory_info->page_per_block);
    
    dhara_bind_spi_nand_list[index].dhara_nand = n;
    dhara_bind_spi_nand_list[index].spi_nand_device = spi_nand_device; 
    
    return 0;
}

static HAL_NAND_Device_t* dhara_find_spi_nand_device(const struct dhara_nand *n)
{
    int index = 0;
    for(index = 0; index < DHARA_NAND_BIND_SPI_NAND_INDEX_MAX; index)
    {
        if(dhara_bind_spi_nand_list[index].is_binded == 1 && dhara_bind_spi_nand_list[index].dhara_nand == n)
        {
            return dhara_bind_spi_nand_list[index].spi_nand_device;
        }
    }

    return NULL;
}

/* Is the given block bad? */
int dhara_nand_is_bad(const struct dhara_nand *n, dhara_block_t b)
{
    // //通过nand_dev的指针索引到dhara_bind_spi_nand_t结构体
    // dhara_bind_spi_nand_t *nand_dev = __containerof(&n, dhara_bind_spi_nand_t, dhara_nand);
    // //通过dhara_bind_spi_nand_t取得dhara_nand对应的HAL_NAND_Device的指针，解引用拿到HAL_NAND_Device结构体本身
    // HAL_NAND_Device_t spi_nand_device = *(nand_dev->spi_nand_device);
    HAL_NAND_Device_t spi_nand_device = *(dhara_find_spi_nand_device(n));

    //(1 << n->log2_ppb)实际上就是对数的反运算，得2的log2_ppb次幂，即page_per_block本身
    //与block_num相乘，获得该block的首页地址，也即块地址
    dhara_page_t blk_first_page_addr = b * (1 << n->log2_ppb);

    return HAL_SPI_NAND_Check_Bad_Block(spi_nand_device, (uint32_t)blk_first_page_addr);	
}

/* Mark bad the given block (or attempt to). No return value is
 * required, because there's nothing that can be done in response.
 */
void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b)
{
    // dhara_bind_spi_nand_t *nand_dev = __containerof(&n, dhara_bind_spi_nand_t, dhara_nand);
    // HAL_NAND_Device_t spi_nand_device = *(nand_dev->spi_nand_device);
    HAL_NAND_Device_t spi_nand_device = *(dhara_find_spi_nand_device(n));

    dhara_page_t blk_first_page_addr = b * (1 << n->log2_ppb);
    
    HAL_SPI_NAND_Mark_Bad_Block(spi_nand_device, (uint32_t)blk_first_page_addr);

    return;
}

/* Erase the given block. This function should return 0 on success or -1
 * on failure.
 *
 * The status reported by the chip should be checked. If an erase
 * operation fails, return -1 and set err to E_BAD_BLOCK.
 */
int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b, dhara_error_t *err)
{
    // dhara_bind_spi_nand_t *nand_dev = __containerof(&n, dhara_bind_spi_nand_t, dhara_nand);
    // HAL_NAND_Device_t spi_nand_device = *(nand_dev->spi_nand_device);
    HAL_NAND_Device_t spi_nand_device = *(dhara_find_spi_nand_device(n));
    uint8_t status,ret = 0;

    dhara_page_t blk_first_page_addr = b * (1 << n->log2_ppb);
    
    if(HAL_SPI_NAND_Write_Enable(spi_nand_device) != 0)
    {
        ret = -1;
        goto end;
    }
    
    if(HAL_SPI_NAND_Erase_Block(spi_nand_device, (uint32_t)blk_first_page_addr) != 0)
    {
        ret = -2;
        goto end;		
    }
    
    if(HAL_SPI_NAND_Wait(spi_nand_device, &status) != 0)
    {
        ret = -3;
        goto end;
    }
    
    if((status & STATUS_E_FAIL_MASK) == STATUS_E_FAIL)
    {
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        ret = -4;
    }
    
end:
    LOG_I("DHARA Erase blk %X, ret %d", b, ret);
    return ret;	
}

/* Program the given page. The data pointer is a pointer to an entire
 * page ((1 << log2_page_size) bytes). The operation status should be
 * checked. If the operation fails, return -1 and set err to
 * E_BAD_BLOCK.
 *
 * Pages will be programmed sequentially within a block, and will not be
 * reprogrammed.
 */
int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p, const uint8_t *data, dhara_error_t *err)
{
    // dhara_bind_spi_nand_t *nand_dev = __containerof(&n, dhara_bind_spi_nand_t, dhara_nand);
    // HAL_NAND_Device_t spi_nand_device = *(nand_dev->spi_nand_device);
    HAL_NAND_Device_t spi_nand_device = *(dhara_find_spi_nand_device(n));
    uint8_t status,ret = 0;
    
    uint32_t nand_page_size = 1 << n->log2_page_size;
    uint16_t used_marker = 0x0000;//标记，该块已经被使用
    
    if(HAL_SPI_NAND_Read_Page_To_Cache(spi_nand_device, (uint32_t)p) != 0)
    {
        ret = -1;
        goto end;
    }
    
    if(HAL_SPI_NAND_Write_Enable(spi_nand_device) != 0)
    {
        ret = -2;
        goto end;
    }
    
    if(HAL_SPI_NAND_Program_Data_To_Cache(spi_nand_device, (uint32_t)p, 0, nand_page_size, (uint8_t *)data, false) != 0)
    {
        ret = -3;
        goto end;
    }
    
    if(HAL_SPI_NAND_Program_Data_To_Cache(spi_nand_device, (uint32_t)p, nand_page_size + 2, sizeof(uint16_t), (uint8_t *)&used_marker, false) != 0)
    {
        ret = -4;
        goto end;
    }

    if(HAL_SPI_NAND_Program_Execute(spi_nand_device, p) != 0)
    {
        ret = -4;
        goto end;		
    }
    
    if(HAL_SPI_NAND_Wait(spi_nand_device, &status) != 0)
    {
        ret = -5;
        goto end;
    }
    
    if((status & STATUS_P_FAIL_MASK) == STATUS_P_FAIL)
    {
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        ret = -6;
    }   

end:
    LOG_I("DHARA Program page %X, size %d, ret %d", p, sizeof(data), ret);
    return ret;			
}

/* Check that the given page is erased */
int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
    // dhara_bind_spi_nand_t *nand_dev = __containerof(&n, dhara_bind_spi_nand_t, dhara_nand);
    // HAL_NAND_Device_t spi_nand_device = *(nand_dev->spi_nand_device);
    HAL_NAND_Device_t spi_nand_device = *(dhara_find_spi_nand_device(n));
    uint8_t status,ret = 0;

    uint32_t nand_page_size = 1 << n->log2_page_size;
    uint16_t used_marker;

    if(HAL_SPI_NAND_Read_Page_To_Cache(spi_nand_device, (uint32_t)p) != 0)
    {
        ret = -1;
        goto end;
    }
    
    HAL_SPI_NAND_Wait(spi_nand_device, &status);
    
    if(HAL_SPI_NAND_Read_From_Cache(spi_nand_device, (uint32_t)p, nand_page_size + 2, sizeof(uint16_t), (uint8_t *)&used_marker) != 0)
    {
        ret = -2;
        goto end;
    }
    
    return (used_marker == 0xFFFF);

end:
    return 0;		
}

/* Read a portion of a page. ECC must be handled by the NAND
 * implementation. Returns 0 on sucess or -1 if an error occurs. If an
 * uncorrectable ECC error occurs, return -1 and set err to E_ECC.
 */
int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p, size_t offset, size_t length, uint8_t *data, dhara_error_t *err)
{
    // dhara_bind_spi_nand_t *nand_dev = __containerof(&n, dhara_bind_spi_nand_t, dhara_nand);
    // HAL_NAND_Device_t spi_nand_device = *(nand_dev->spi_nand_device);
    HAL_NAND_Device_t spi_nand_device = *(dhara_find_spi_nand_device(n));
    uint8_t status,ret = 0;
    uint32_t ecc_err, ecc_corrected;

    uint32_t nand_page_size = 1 << n->log2_page_size;

    if(HAL_SPI_NAND_Read_Page_To_Cache(spi_nand_device, (uint32_t)p) != 0)
    {
        ret = -1;
        goto end;
    }
    
    if(HAL_SPI_NAND_Wait(spi_nand_device, &status) != 0)
    {
        ret = -2;
        goto end;
    }
    HAL_SPI_NAND_Check_Ecc_Status((uint32_t)status, &ecc_corrected, &ecc_err);
    
    if(ecc_err)
    {
        dhara_set_error(err, DHARA_E_ECC);
        ret = -3;
        goto end;		
    }

    if(HAL_SPI_NAND_Read_From_Cache(spi_nand_device, (uint32_t)p, offset, length, data) != 0)
    {
        ret = -4;
        goto end;
    }
    
end:
    LOG_I("DHARA Read page %X, size %d, ret %d", p, length, ret);
    return ret;	
    
}

/* Read a page from one location and reprogram it in another location.
 * This might be done using the chip's internal buffers, but it must use
 * ECC.
 */
int dhara_nand_copy(const struct dhara_nand *n, dhara_page_t src, dhara_page_t dst, dhara_error_t *err)
{
    // dhara_bind_spi_nand_t *nand_dev = __containerof(&n, dhara_bind_spi_nand_t, dhara_nand);
    // HAL_NAND_Device_t spi_nand_device = *(nand_dev->spi_nand_device);
    HAL_NAND_Device_t spi_nand_device = *(dhara_find_spi_nand_device(n));
    
    uint8_t status,ret = 0;
    uint32_t ecc_err, ecc_corrected;


    if(HAL_SPI_NAND_Read_Page_To_Cache(spi_nand_device, src) != 0)
    {
        ret = -1;
        goto end;
    }
    
    if(HAL_SPI_NAND_Wait(spi_nand_device, &status) != 0)
    {
        ret = -2;
        goto end;		
    }
    
    HAL_SPI_NAND_Check_Ecc_Status((uint32_t)status, &ecc_corrected, &ecc_err);
    
    if(ecc_err)
    {
        dhara_set_error(err, DHARA_E_ECC);
        ret = -3;
        goto end;		
    }	

    
    if(HAL_SPI_NAND_Write_Enable(spi_nand_device) != 0)
    {
        ret = -4;
        goto end;
    }
    
    if(HAL_SPI_NAND_Program_Execute(spi_nand_device, dst) != 0)
    {
        ret = -5;
        goto end;		
    }
    
    if(HAL_SPI_NAND_Wait(spi_nand_device, &status) != 0)
    {
        ret = -6;
        goto end;		
    }
    
    if ((status & STATUS_P_FAIL_MASK) == STATUS_P_FAIL) 
    {
        return -7;
    }

end:
    return ret;
}
