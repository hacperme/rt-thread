#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <rtdbg.h>
#include "at_data_transf.h"
#include <sys/stat.h>
#include "rtthread.h"
#include "at.h"

#define ESP_UART_NUM "uart5"
#define RESP_BUFFER_SIZE 512

#define PRINTF_RESP(X) do{	\
	if(X && X->buf){		\
		LOG_E("resp_buf-> lene:%d : data:%s\n",__LINE__, X->buf);	\
	}	\
}while(0)

#define AT_SEND_CMD(X, Y, Z, M) do{						\
		if (at_obj_exec_cmd(X, Y, Z) < 0)	\
		{											\
			LOG_E("Failed to execut %s\n",X);		\
			M;							\
		}											\
	}while(0)


typedef struct
{
    rt_sem_t _rdy;
    rt_sem_t _stop;
} qat_sem_s;

static qat_sem_s _ql_at_sem = {0};

static at_response_t resp = NULL;
static void urc_func(struct at_client *client ,const char *data, rt_size_t size);
static void dt_urc_func(struct at_client *client ,const char *data, rt_size_t size);

static struct at_urc urc_table[] = {
    {"+STA_CONNECTED:",   	"\r\n",     	urc_func},
    {"+STA_DISCONNECTED:", 		"\r\n",		urc_func},
    {"+AP_STARTED:", 		"\r\n",		urc_func},
    {"+AP_STOPPED:",   	"\r\n",     	urc_func},
    {"ready",   	"\r\n",     	urc_func},
    {"+DT:", 		"\r\n",		dt_urc_func},};


static void urc_func(struct at_client *client ,const char *data, rt_size_t size)
{
    LOG_E("urc data : %s, size:%d\n", data, size);
    if(strncmp(data, "ready", strlen("ready")) == 0)
    {
        LOG_E("send ready sem\n");
        rt_sem_release(_ql_at_sem._rdy);
    }
    else if (strncmp(data, "+AP_STOPPED", strlen("+AP_STOPPED")) == 0) {
        LOG_E("send AP_STOPPED sem\n");
        rt_sem_release(_ql_at_sem._stop);
    }
}

rt_err_t esp_wait_stop()
{
    return rt_sem_take(_ql_at_sem._stop, RT_WAITING_FOREVER);
}

static void dt_urc_func(struct at_client *client ,const char *data, rt_size_t size)
{
    LOG_E("dt urc data : %s, size:%d\n", data, size);
	if(strncmp(data, "+DT:START", strlen("+DT:START")) == 0)
	{
	} else if (strncmp(data, "+DT:ERR", strlen("+DT:ERR")) == 0)
	{
        at_client_t client_urc = at_client_get(ESP_UART_NUM);
        AT_SEND_CMD(client_urc, resp, "AT+QTRANSF=0", ;);
	} else if (strncmp(data, "+DT:SUCCESS", strlen("+DT:SUCCESS")) == 0)
	{
        at_client_t client_urc = at_client_get(ESP_UART_NUM);
        AT_SEND_CMD(client_urc, resp, "AT+QTRANSF=0", ;);
	}else if (strncmp(data, "+DT:CLOSE", strlen("+DT:CLOSE")) == 0) 
    {
    }
}
bool esp_at_init(void)
{
    /* 初始化第一个 AT Client */
	if(RT_EOK != at_client_init(ESP_UART_NUM, RESP_BUFFER_SIZE, RESP_BUFFER_SIZE))
	{
		LOG_E("AT Client1 (uart1) initialize failed\n");
	}

	resp = at_create_resp(RESP_BUFFER_SIZE, 0, 3000);
	if (!resp) {
        LOG_E("Failed to create AT response object\n");
        return false;
    }

    _ql_at_sem._rdy = rt_sem_create("rdy_sem", 0, RT_IPC_FLAG_PRIO);
    _ql_at_sem._stop = rt_sem_create("stop_sem", 0, RT_IPC_FLAG_PRIO);
	
    LOG_I("AT Client1 (uart5) initialized success\n");
	at_obj_set_urc_table(at_client_get(ESP_UART_NUM), urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

	return true;
}

bool esp_at_deinit(void)
{
    if(_ql_at_sem._rdy)
	{
		rt_sem_delete(_ql_at_sem._rdy);
		_ql_at_sem._rdy = NULL;
	}

    if(resp)
	{
		at_delete_resp(resp);
		resp = NULL;
	}

	return true;
}

rt_err_t esp_wait_rdy()
{
    return rt_sem_take(_ql_at_sem._rdy, rt_tick_from_millisecond(15000));
}
rt_err_t esp32_transf_data(const char* ssid, size_t ssid_len, const char* psw, size_t psw_len, const char* dk_str, size_t dk_len)
{
    rt_err_t result = RT_ERROR;
    at_client_t client = RT_NULL;
    char send_cmd[512] = {0};
    #define AT_DT_SEND_CMD AT_SEND_CMD(client, resp, send_cmd, goto dt_err)

    if(!(ssid && psw && dk_str && ssid_len > 0 && psw_len > 0 && dk_len > 0))
    {
        return RT_ERROR;
    }

    client = at_client_get(ESP_UART_NUM);
    if (client == RT_NULL) {
        LOG_E("cat1 at client not inited!");
        return RT_ERROR;
    }


    memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+CWINIT=1");
	AT_DT_SEND_CMD;

    memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+CWSAP=\"%s\",\"%s\",5,3",ssid, psw);
	AT_DT_SEND_CMD;

    memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QDK=\"%s\"",dk_str);
	AT_DT_SEND_CMD;

    memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QTRANSF=1");
	AT_DT_SEND_CMD;

    result = RT_EOK;

dt_err:
    return result;

}

void esp_data_stransf_example(void)
{
    LOG_E("esp_data_stransf_example start!");
    esp_at_init();

    esp32_transf_data("ESP_TEST", strlen("ESP_TEST"), "1234567890", strlen("1234567890"),"THIS IS DT TEST", strlen("THIS IS DT TEST"));


}