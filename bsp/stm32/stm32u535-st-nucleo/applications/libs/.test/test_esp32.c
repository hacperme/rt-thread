/*
 * @FilePath: test_esp32.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-27 17:12:48
 * @copyright : Copyright (c) 2024
 */
#include "rtthread.h"
#include "rtdevice.h"

#define DBG_SECTION_NAME "TEST_ESP32"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define ESP32_SERIAL_NAME "uart5"
#define RECV_BUFF_SIZE 1024

static struct rt_thread ESP32_RECV_THD;
static char ESP32_RECV_THD_STACK[0x800];

static rt_uint8_t RECV_BUFF[RECV_BUFF_SIZE] = {0};
static rt_device_t ESP32_SERIAL = RT_NULL;
static rt_uint8_t ESP32_RECV_EXIST = 0;

static void esp32_msg_recv(void *parameter)
{
    while (ESP32_RECV_EXIST == 0)
    {
        rt_memset(&RECV_BUFF, 0, RECV_BUFF_SIZE);
        if (rt_device_read(ESP32_SERIAL, -1, &RECV_BUFF, RECV_BUFF_SIZE) > 0)
        {
            LOG_D("ESP32 Recv Msg: %s", RECV_BUFF);
        }
        rt_thread_mdelay(500);
    }
}

rt_err_t esp32_init(void)
{
    rt_err_t res;

    if (ESP32_SERIAL == RT_NULL)
    {
        ESP32_SERIAL = rt_device_find(ESP32_SERIAL_NAME);
        res = ESP32_SERIAL == RT_NULL ? RT_ERROR : RT_EOK;
        LOG_D("find ESP32_SERIAL %s %s!\n", ESP32_SERIAL_NAME, res == RT_EOK ? "success" : "failed");
        if (res != RT_EOK)
        {
            return res;
        }
    }

    /* 以中断接收及轮询发送模式打开串口设备 */
    res = rt_device_open(ESP32_SERIAL, RT_DEVICE_FLAG_INT_RX);
    LOG_D("rt_device_open ESP32_SERIAL %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    /* 创建 ESP32 RECV 线程 */
    res = rt_thread_init(&ESP32_RECV_THD, "ESP32", esp32_msg_recv, RT_NULL, ESP32_RECV_THD_STACK, 0x800, 25, 10);
    LOG_D("rt_thread_init ESP32_RECV_THD %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        rt_device_close(ESP32_SERIAL);
        return res;
    }

    ESP32_RECV_EXIST = 0;
    res = rt_thread_startup(&ESP32_RECV_THD);
    LOG_D("ESP32_RECV_THD start %s", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        rt_device_close(ESP32_SERIAL);
    }

    return res;
}

#ifdef RT_USING_MSH
#include "lpm.h"
#include "board_pin.h"
static void test_esp32(void)
{
    rt_err_t res;
    rt_ssize_t ret;

    res = esp32_init();
    LOG_D("esp32_init res=%d", res);
    char send_buff[32] = "TEST ESP32 UART RW.";

    res = esp32_power_on();
    LOG_D("esp32_power_on %s", res == RT_EOK ? "success" : "failed");

    if (res == RT_EOK)
    {
        while (1)
        {
            ret = rt_device_write(ESP32_SERIAL, 0, send_buff, sizeof(send_buff));
            res = ret == sizeof(send_buff) ? RT_EOK : RT_ERROR;
            LOG_D("ESP32 Send Msg %s.", res == RT_EOK ? "success" : "failed");
            rt_thread_mdelay(1000);
        }
    }
}

MSH_CMD_EXPORT(test_esp32, test esp32);

static void test_esp32_download(int argc, char **argv)
{
    rt_err_t res;
    rt_uint8_t mode = 0;
    if (argc >= 2)
    {
        mode = atoi(argv[1]);
    }
    if (mode == 0)
    {
        rt_pin_mode(FLASH_PWRON_PIN, PIN_MODE_OUTPUT);
        rt_pin_write(FLASH_PWRON_PIN, PIN_HIGH);
        res = rt_pin_read(FLASH_PWRON_PIN) == PIN_HIGH ? RT_EOK : RT_ERROR;
        LOG_D("FLASH_PWRON %s", res == RT_EOK ? "success" : "failed");
        rt_pin_mode(QSPI_CPUN_ESP_PIN, PIN_MODE_OUTPUT);
        rt_pin_write(QSPI_CPUN_ESP_PIN, PIN_HIGH);
        res = rt_pin_read(QSPI_CPUN_ESP_PIN) == PIN_HIGH ? RT_EOK : RT_ERROR;
        LOG_D("QSPI_CPUN_ESP %s", res == RT_EOK ? "success" : "failed");
        res = esp32_power_on();
        LOG_D("esp32_power_on %s", res == RT_EOK ? "success" : "failed");
    }
    else
    {
        res = esp32_start_download();
        LOG_D("esp32_start_download %s", res == RT_EOK ? "success" : "failed");
    }
}

MSH_CMD_EXPORT(test_esp32_download, test esp32 donwload);

#define UART5_RX_PIN        GET_PIN(D, 2)
#define UART5_TX_PIN        GET_PIN(C, 12)

static void test_uart5_pin_init(void)
{
    rt_pin_mode(UART5_RX_PIN, PIN_MODE_INPUT);
    rt_pin_mode(UART5_TX_PIN, PIN_MODE_INPUT);
}

// MSH_CMD_EXPORT(test_uart5_pin_init, test uart5 pin init);

#endif