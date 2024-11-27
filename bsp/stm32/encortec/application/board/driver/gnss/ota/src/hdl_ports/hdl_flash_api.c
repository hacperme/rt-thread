#include "hdl_ports/hdl_flash_api.h"
#include <stdio.h>

static FILE *ota_file = NULL;
/**
 * @brief Open file
 * @param file 
 * @return 
 */
bool hdl_flash_init(char *file)
{
    bool res = ota_file != NULL ? true : false;
    if (res) goto _exit_;

    ota_file = fopen(file, "rb");
    res = ota_file != NULL ? true : false;

_exit_:
    return res;
}

/**
 * @brief Read file.
 * @param start_address 
 * @param buffer 
 * @param length 
 * @return 
 */
bool hdl_flash_read(uint32_t start_address, uint8_t *buffer, uint32_t length)
{
    bool res = ota_file != NULL ? true : false;
    if (!res) goto _exit_;

    int ret = fseek(ota_file, start_address, SEEK_SET);
    res = ret == 0 ? true : false;
    if (!res) goto _exit_;

    size_t read_size = fread(buffer, 1, length, ota_file);
    res = read_size >= 0 ? true : false;

_exit_:
    return res;
}

/**
 * @brief Close file.
 * @return 
 */
bool hdl_flash_deinit()
{
    bool res = ota_file == NULL ? true : false;
    if (res) goto _exit_;

    fclose(ota_file);
    ota_file = NULL;
    res = true;

_exit_:
    return res;
}
