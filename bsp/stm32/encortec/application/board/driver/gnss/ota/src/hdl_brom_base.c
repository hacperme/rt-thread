#include "hdl_brom_base.h"
#include "rtthread.h"

static uint8_t g_hdl_da_data_buf[DA_DATA_UNIT] = {0};

bool g_brom_task_stop = false;
const unsigned char HDL_START_CMD[4]    = {0xA0, 0x0A, 0x50, 0x05};
const unsigned char HDL_BROM_RSP_CMD[4] = {0x5F, 0xF5, 0xAF, 0xFA};

#define brom_task_size 0x1000
static struct rt_thread brom_task;
static char brom_task_stack[brom_task_size];
static struct rt_semaphore brom_task_sem;
bool brom_task_init_tag = false;
bool brom_task_start_tag = false;

static void brom_task_entry(void *pvParameters);
static rt_err_t brom_task_init(void);
static rt_err_t hdl_create_brom_task(void);
static void hdl_delete_brom_task(void);

static void brom_task_entry(void *pvParameters)
{
    while (!g_brom_task_stop) {
        HDL_COM_PutByte(HDL_START_CMD[0]);
        HDL_LOGI("brom_task put_char 0xA0");
        hdl_delay(50);
    }

    rt_sched_lock_level_t slvl;
    rt_sched_lock(&slvl);
    brom_task_start_tag = false;
    rt_err_t res = rt_sem_release(&brom_task_sem);
    log_debug("rt_sem_release brom_task_sem %s", res == RT_EOK ? "success" : "failed");
    rt_sched_unlock(slvl);
    res = rt_thread_suspend(rt_thread_self());
    log_debug("rt_thread_suspend rt_thread_self %s", res == RT_EOK ? "success" : "failed");
}

static rt_err_t brom_task_init(void)
{
    rt_err_t res = brom_task_init_tag == true ? RT_EOK : RT_ERROR;
    if (brom_task_init_tag == true) return res;

    res = rt_sem_init(&brom_task_sem, "BromTaskSem", 0, RT_IPC_FLAG_PRIO);
    log_info("rt_sem_init BromTaskSem %s.", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        return res;
    }

    res = rt_thread_init(
        &brom_task,
        "BromTask",
        brom_task_entry,
        RT_NULL,
        brom_task_stack,
        brom_task_size,
        25,
        4
    );
    log_info("rt_thread_init BromTask %s.", res == RT_EOK ? "success" : "failed");
    if (res != RT_EOK)
    {
        rt_sem_detach(&brom_task_sem);
        return res;
    }

    brom_task_init_tag = res == RT_EOK ? true : false;
    return res;
}

static rt_err_t hdl_create_brom_task(void)
{
    rt_err_t res = brom_task_init();
    log_info("brom_task_init %s.", res == RT_EOK ? "success" : "failed");

    res = brom_task_init_tag == true ? RT_EOK : RT_ERROR;
    if (brom_task_init_tag != true) return res;

    g_brom_task_stop = false;
    res = rt_thread_startup(&brom_task);
    log_info("rt_thread_startup BromTask %s.", res == RT_EOK ? "success" : "failed");
    brom_task_start_tag = res == RT_EOK ? true : false;

    return res;
}

static void hdl_delete_brom_task(void)
{
    g_brom_task_stop = true;
    rt_err_t res;
    char _cnt_ = 3;
    while (_cnt_ > 0)
    {
        res = rt_sem_take(&brom_task_sem, 500);
        if (res == RT_EOK)
        {
            rt_sem_detach(&brom_task_sem);
            rt_thread_detach(&brom_task);
            brom_task_init_tag = false;
            log_info("hdl_delete_brom_task success.");
            break;
        }
        else
        {
            log_error("hdl_delete_brom_task failed.");
        }
        _cnt_--;
    }
}

static void brom_write8_echo(uint8_t data)
{
    HDL_LOGI("brom_write8_echo 0x%02X", data);
    HDL_COM_PutByte(data);
    uint8_t rx_data = HDL_COM_GetByte();
    if (rx_data != data)
    {
        HDL_LOGE("brom_write8_echo fail: 0x%02X - 0x%02X", data, rx_data);
    }
}

static void brom_write16_echo(uint16_t data)
{
    HDL_LOGI("brom_write16_echo 0x%04X", data);
    HDL_COM_PutData16(data);
    uint16_t rx_data = HDL_COM_GetData16();
    if (rx_data != data)
    {
        HDL_LOGE("brom_write16_echo fail: 0x%04X - 0x%04X", data, rx_data);
    }
}

static void brom_write32_echo(uint32_t data)
{
    HDL_LOGI("brom_write32_echo 0x%08X", data);
    HDL_COM_PutData32(data);
    uint32_t rx_data = HDL_COM_GetData32();
    if (rx_data != data)
    {
        HDL_LOGE("brom_write32_echo fail: 0x%08X - 0x%08X", data, rx_data);
    }
}

bool hdl_brom_start()
{
    char rx_data = 0;
    uint8_t retry_cnt = 0;

brom_start_retry:
    retry_cnt++;
    if (retry_cnt >= 4)
    {
        return false;
    }

#if defined (HDL_VIA_UART)
    // ToDo Note: For our UART, need to create asnyc_task to send START_CMD[0]=0xA0
    // continuously in UART Polling mode.
    // For your UART, you could try to use UART DMA mode, or create task like us.
    hdl_create_brom_task();
    uint16_t cnt = 100;
    while (cnt > 0) {
        hdl_delay(20);
        rx_data = HDL_COM_GetByte();
        if (rx_data == HDL_BROM_RSP_CMD[0]) {
            HDL_LOGI("hdl_brom_start 0xA0<->0x5F");
            break;
        }
        cnt--;
    }
    hdl_delete_brom_task();
    if (rx_data != HDL_BROM_RSP_CMD[0] && cnt == 0)
    {
        goto brom_start_retry;
    }
#endif

    HDL_COM_PutByte(HDL_START_CMD[1]);
    rx_data = HDL_COM_GetByte();
    if (rx_data == HDL_BROM_RSP_CMD[1]) {
        HDL_LOGI("hdl_brom_start 0x0A<->0xF5");
    } else {
        HDL_LOGE("hdl_brom_start 0x0A ->(error) 0x%02X, retry", rx_data);
        hdl_delay(2000);
        goto brom_start_retry;
    }

    HDL_COM_PutByte(HDL_START_CMD[2]);
    rx_data = HDL_COM_GetByte();
    if (rx_data == HDL_BROM_RSP_CMD[2]) {
        HDL_LOGI("hdl_brom_start 0x50<->0xAF");
    } else {
        HDL_LOGE("hdl_brom_start 0x50 ->(error) 0x%02X, retry", rx_data);
        hdl_delay(2000);
        goto brom_start_retry;
    }

    HDL_COM_PutByte(HDL_START_CMD[3]);
    rx_data = HDL_COM_GetByte();
    if (rx_data == HDL_BROM_RSP_CMD[3]) {
        HDL_LOGI("hdl_brom_start 0x05<->0xFA");
    } else {
        HDL_LOGE("hdl_brom_start 0x05 ->(error) 0x%02X, retry", rx_data);
        hdl_delay(2000);
        goto brom_start_retry;
    }

    HDL_LOGI("hdl_brom_start pass");
    return true;
}

bool hdl_brom_disable_wdt(void)
{
    hdl_brom_write16(0xA2080000, 0x0010);
    hdl_brom_write16(0xA2080030, 0x0040);
    return true;
}

bool hdl_brom_read16(uint32_t addr, uint16_t *data)
{
    bool success = false;
    brom_write8_echo(BROM_CMD_READ16);
    brom_write32_echo(addr);
    brom_write32_echo(1);

    uint16_t status = HDL_COM_GetData16();
    HDL_Require_Noerr_Action(status < BROM_ERROR, exit, "hdl_brom_read16");
    *data = HDL_COM_GetData16();
    status = HDL_COM_GetData16();
    HDL_Require_Noerr_Action(status < BROM_ERROR, exit, "hdl_brom_read16");
    success = true;

exit:
    return success;
}

bool hdl_brom_write16(uint32_t addr, uint16_t data)
{
    bool success = false;
    brom_write8_echo(BROM_CMD_WRITE16);
    brom_write32_echo(addr);
    brom_write32_echo(1);

    uint16_t status = HDL_COM_GetData16();
    HDL_Require_Noerr_Action(status < BROM_ERROR, exit, "hdl_brom_write16");
    brom_write16_echo(data);
    status = HDL_COM_GetData16();
    HDL_Require_Noerr_Action(status < BROM_ERROR, exit, "hdl_brom_write16");
    success = true;

exit:
    return success;
}

bool hdl_brom_send_da(const hdl_connect_arg_t *connect_arg, char *da_file, uint32_t da_run_addr, uint32_t da_len)
{
    bool success = false;
    bool ret = hdl_flash_init(da_file);
    HDL_Require_Noerr_Action(ret, exit, "hdl_flash_init");
    HDL_LOGI("hdl_brom_send_da %s 0x%08X %d", da_file, da_run_addr, da_len);

    // DA Init Callback
    if (connect_arg != NULL && connect_arg->conn_da_init_cb != NULL) {
        connect_arg->conn_da_init_cb(connect_arg->conn_da_init_cb_arg);
    }

    brom_write8_echo(BROM_CMD_SEND_DA);
    brom_write32_echo(da_run_addr);
    brom_write32_echo(da_len);
    brom_write32_echo(0);
    
    uint16_t status = HDL_COM_GetData16();
    HDL_Require_Noerr_Action(status < BROM_ERROR, exit, "hdl_brom_send_da");

    uint32_t total_sent_len = 0;
    uint32_t local_checksum = 0;
    while (total_sent_len < da_len) {
        memset(g_hdl_da_data_buf, 0, DA_DATA_UNIT);
        
        const unsigned int to_send_len = min(DA_DATA_UNIT, da_len - total_sent_len);
        ret = hdl_flash_read(total_sent_len, g_hdl_da_data_buf, to_send_len);
        if (ret) {
            const uint32_t sent_len = HDL_COM_PutByte_Buffer(g_hdl_da_data_buf, to_send_len);
            total_sent_len += sent_len;
            HDL_LOGI("hdl_brom_send_da, %d sent %d", to_send_len, sent_len);

            // DA Send Callback
            if (connect_arg != NULL && connect_arg->conn_da_send_cb != NULL) {
                connect_arg->conn_da_send_cb(connect_arg->conn_da_send_cb_arg, total_sent_len, da_len);
            }

             local_checksum ^= hdl_compute_checksum(g_hdl_da_data_buf, sent_len);
        } else {
            HDL_LOGI("hdl_brom_send_da, flash read fail %d", ret);
        }
    }
    
    uint16_t brom_checksum = HDL_COM_GetData16();
    HDL_LOGI("hdl_brom_send_da, local_checksum=%d brom_checksum=%d", local_checksum, brom_checksum);

    status = HDL_COM_GetData16();
    HDL_LOGI("hdl_brom_send_da, status=%d", status);

    HDL_Require_Noerr_Action(local_checksum == brom_checksum, exit, "hdl_brom_send_da, checksum");
    HDL_Require_Noerr_Action(status < BROM_ERROR, exit, "hdl_brom_send_da");
    
    success = true;

exit:
    hdl_flash_deinit();
    return success;
}

bool hdl_brom_jump_da(uint32_t addr)
{
    HDL_LOGI("hdl_brom_jump_da 0x%08X", addr);
    brom_write8_echo(BROM_CMD_JUMP_DA);
    brom_write32_echo(addr);
    uint16_t status = HDL_COM_GetData16();
    return (status < BROM_ERROR);
}

bool hdl_brom_set_baudrate(uint32_t bd)
{
    HDL_LOGI("hdl_set_baudrate 0x%08X", bd);
    brom_write8_echo(BROM_CMD_SET_BAUD);
    brom_write32_echo(bd);
    uint16_t status = HDL_COM_GetData16();
    return (status < BROM_ERROR);
}


