#include "hdl_ports/hdl_uart.h"
#include "tools.h"
#include "logging.h"

#ifdef HDL_VIA_UART

static rt_device_t gnss_serial = RT_NULL;

bool hdl_uart_init()
{
    bool ret = false;
    rt_err_t res;

    gnss_pwron_pin_init();
    gnss_rst_pin_init();
    eg915_gnssen_pin_init();

    if (gnss_serial == RT_NULL)
    {
        gnss_serial = rt_device_find(GNSS_UART_NAME);
        ret = gnss_serial == RT_NULL ? false : true;
        if (ret != true)
        {
            log_debug("find %s failed!\n", GNSS_UART_NAME);
            return ret;
        }
    }

    res = rt_device_open(gnss_serial, RT_DEVICE_FLAG_INT_RX);
    log_debug("rt_device_open gnss_serial %s", res_msg(res == RT_EOK));
    ret = res == RT_EOK ? true : false;
    if (ret != true)
    {
        return ret;
    }

    res = gnss_swith_source(0);
    ret = res == RT_EOK ? true : false;
    log_debug("gnss_swith_source(0) %s", res_msg(res == RT_EOK));
    if (ret != true)
    {
        return ret;
    }
    res = gnss_power_on();
    ret = res == RT_EOK ? true : false;
    log_debug("gnss_power_on %s", res_msg(res == RT_EOK));
    if (ret != true)
    {
        gnss_power_off();
        return ret;
    }
    res = gnss_reset_init();
    ret = res == RT_EOK ? true : false;
    log_debug("gnss_reset_init %s", res_msg(res == RT_EOK));
    if (ret != true)
    {
        gnss_power_off();
        return ret;
    }
}

bool hdl_uart_deinit()
{
    bool ret;
    rt_err_t res;
    res = rt_device_close(gnss_serial);
    ret = res == RT_EOK ? true : false;
    res = gnss_power_off();
    ret = res == RT_EOK ? true : false;
    return ret;
}

uint8_t hdl_uart_get_byte()
{
    uint8_t data = 0;
    rt_device_read(gnss_serial, -1, &data, 1);
    return data;
}

uint32_t hdl_uart_get_byte_buffer(uint8_t *buf, uint32_t length)
{
    return (uint32_t)rt_device_read(gnss_serial, -1, buf, length);
}

void hdl_uart_put_byte(uint8_t data)
{
    rt_device_write(gnss_serial, 0, &data, 1);
}

uint32_t hdl_uart_put_byte_buffer(uint8_t *buf, uint32_t length)
{
    return rt_device_write(gnss_serial, 0, buf, length);
}

uint16_t hdl_uart_get_data16()
{
    uint8_t byte_data[2] = {0};
    rt_device_read(gnss_serial, -1, byte_data, 2);
    uint16_t ret = (byte_data[0] << 8) | byte_data[1];
    return ret;
}

void hdl_uart_put_data16(uint16_t data)
{
    uint8_t byte_data[2] = {0};
    byte_data[0] = (data >> 8) & 0xFF;
    byte_data[1] = data & 0xFF;
    rt_device_write(gnss_serial, 0, byte_data, 2);
}

uint32_t hdl_uart_get_data32()
{
    uint8_t byte_data[4] = {0};
    rt_device_read(gnss_serial, -1, byte_data, 4);
    uint32_t ret = (byte_data[0] << 24) | (byte_data[1] << 16) | (byte_data[2] << 8) | byte_data[3];
    return ret;
}

void hdl_uart_put_data32(uint32_t data)
{
    uint8_t byte_data[4] = {0};
    byte_data[0] = (data >> 24) & 0xFF;
    byte_data[1] = (data >> 16) & 0xFF;
    byte_data[2] = (data >> 8) & 0xFF;
    byte_data[3] = data & 0xFF;
    rt_device_write(gnss_serial, 0, byte_data, 4);
}

void hdl_uart_set_baudrate(uint32_t baud_rate)
{
    rt_err_t res;
    res = rt_device_close(gnss_serial);
    log_info("rt_device_close gnss_serial %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return;
    }

    struct serial_configure gnss_serial_cfg = RT_SERIAL_CONFIG_DEFAULT;
    gnss_serial_cfg.baud_rate = baud_rate;

    res = rt_device_control(gnss_serial, RT_DEVICE_CTRL_CONFIG, &gnss_serial_cfg);
    log_info("rt_device_control baud_rate=%ld %s", baud_rate, res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return;
    }

    res = rt_device_open(gnss_serial, RT_DEVICE_FLAG_INT_RX);
    log_info("rt_device_open gnss_serial %s", res_msg(res == RT_EOK));
}

#endif
