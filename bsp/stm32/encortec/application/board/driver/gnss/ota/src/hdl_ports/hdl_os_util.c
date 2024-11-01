#include "hdl_ports/hdl_os_util.h"
#include "rtthread.h"
#include "logging.h"

uint16_t hdl_compute_checksum(uint8_t *buf, uint32_t buf_len)
{
    uint16_t checksum = 0;
    if (buf == NULL || buf_len == 0) {
        log_debug("hdl_compute_checksum, invalid arg");
        return 0;
    }

    int i = 0;
    for (i = 0; i < buf_len / 2; i++) {
        checksum ^= *(uint16_t *)(buf + i * 2);
    }

    if ((buf_len % 2) == 1) {
        checksum ^= buf[i * 2];
    }
    return checksum;
}
