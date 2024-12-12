
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <rtdbg.h>
#include "at_client_ssl.h"
#include <sys/stat.h>
#include "rtthread.h"

#define BUFFER_CHUNK_SIZE 1024
#define RESP_BUFFER_SIZE 512


#define PRINTF_RESP(X) do{	\
	if(X && X->buf){		\
		LOG_E("resp_buf-> lene:%d : data:%s\n",__LINE__, X->buf);	\
	}	\
}while(0);


#define AT_SEND_CMD(X, Y, Z, M) do{						\
		if (at_obj_exec_cmd(X, Y, Z) < 0)	\
		{											\
			LOG_E("Failed to execut %s\n",X);		\
			M;							\
		}											\
	}while(0)


typedef struct
{
	rt_sem_t _qsslopen;
	rt_sem_t _qsslurc;
	rt_sem_t _qfupl;
    rt_sem_t _rdy;
	rt_sem_t _powered_down;
} qat_sem_s;

static qat_sem_s _ql_at_sem = {0};

static at_response_t resp = NULL;

static void urc_func(struct at_client *client ,const char *data, rt_size_t size);
bool at_ssl_close(at_client_t client);


static struct at_urc urc_table[] = {
    {"+QSSLOPEN:",   	"\r\n",     	urc_func},
    {"+QSSLURC:", 		"\r\n",		urc_func},
    {"+QFUPL:", 		"\r\n",		urc_func},
    {"RDY",         "\r\n",         urc_func},
	{"POWERED DOWN", "\r\n",		urc_func}
};


static void urc_func(struct at_client *client ,const char *data, rt_size_t size)
{
    LOG_E("urc data : %s, size:%d\n", data, size);
	if(strncmp(data, "+QSSLOPEN", strlen("+QSSLOPEN")) == 0 && data[13] == '0')
	{
		LOG_E("send _qsslopen sem\n");
		rt_sem_release(_ql_at_sem._qsslopen);
	} else if (strncmp(data, "+QSSLURC", strlen("+QSSLURC")) == 0)
	{
		LOG_E("send _qsllurc sem\n");
		rt_sem_release(_ql_at_sem._qsslurc);
	} else if (strncmp(data, "+QFUPL", strlen("+QFUPL")) == 0)
	{
		LOG_E("send _qfupl sem\n");
		rt_sem_release(_ql_at_sem._qfupl);
	}else if (strncmp(data, "RDY", strlen("RDY")) == 0) {
        LOG_E("send _rdy sem\n");
        rt_sem_release(_ql_at_sem._rdy);
    }
	else if (strncmp(data, "POWERED DOWN", strlen("POWERED DOWN")) == 0) {
        LOG_E("send _powered_down sem\n");
        rt_sem_release(_ql_at_sem._powered_down);
    }
}

bool at_ssl_client_init(void)
{
    /* 初始化第一个 AT Client */
	if(RT_EOK != at_client_init("uart1", RESP_BUFFER_SIZE, RESP_BUFFER_SIZE))
	{
		LOG_E("AT Client1 (uart1) initialize failed\n");
	}

	resp = at_create_resp(RESP_BUFFER_SIZE, 0, 3000);
	if (!resp) {
        LOG_E("Failed to create AT response object\n");
        return false;
    }

	_ql_at_sem._qsslopen = rt_sem_create("sslopen_sem", 0, RT_IPC_FLAG_PRIO);
	_ql_at_sem._qsslurc = rt_sem_create("sslurc_sem", 0, RT_IPC_FLAG_PRIO);
    _ql_at_sem._rdy = rt_sem_create("rdy_sem", 0, RT_IPC_FLAG_PRIO);
	_ql_at_sem._qfupl = rt_sem_create("fupl_sem", 0, RT_IPC_FLAG_PRIO);
	_ql_at_sem._powered_down = rt_sem_create("_powered_down", 0, RT_IPC_FLAG_PRIO);
	
    LOG_I("AT Client1 (uart1) initialized success\n");
	at_obj_set_urc_table(at_client_get("uart1"), urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

	return true;
}

bool at_ssl_client_deinit(void)
{
	if(_ql_at_sem._qfupl)
	{
		rt_sem_delete(_ql_at_sem._qfupl);
		_ql_at_sem._qfupl = NULL;
	}
	if(_ql_at_sem._rdy)
	{
		rt_sem_delete(_ql_at_sem._rdy);
		_ql_at_sem._rdy = NULL;
	}
	if(_ql_at_sem._qsslurc)
	{
		rt_sem_delete(_ql_at_sem._qsslurc);
		_ql_at_sem._qsslurc = NULL;
	}
	if(_ql_at_sem._qsslopen)
	{
		rt_sem_delete(_ql_at_sem._qsslopen);
		_ql_at_sem._qsslopen = NULL;
	}
	
	if(resp)
	{
		at_delete_resp(resp);
		resp = NULL;
	}
	return true;
}

bool at_ssl_connect(at_client_t client, const char *cacert_filename, const char*serveraddr, int server_port)
{
	char* check_cacert_file_cmd = NULL;
	char send_cmd[1024] = {0};
	int error_num = 0;
	int file_handle = 0;
	int pdp_status = 0;
	bool ret = false;
	#define AT_SSL_SEND_CMD AT_SEND_CMD(client, resp, send_cmd, goto ssl_end)

	if(!(client && cacert_filename && serveraddr && server_port && resp))
	{
		return false;
	}

	at_obj_set_end_sign(client, '>');

	/*--------------check cacer file start-----------------*/
	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QFOPEN=\"%s\",2", cacert_filename);
	AT_SSL_SEND_CMD;
 	
	rt_thread_mdelay(10);
	PRINTF_RESP(resp);
	
	if (at_resp_parse_line_args(resp, 2, "+QFOPEN: %d", &file_handle) <= 0) {
        LOG_E("Failed to parse AT+QFOPEN? response\n");
		goto ssl_end;
    }
	LOG_E("file %s exists, file_handle is %d\n", cacert_filename, file_handle);

	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QFCLOSE=%d", file_handle);
	AT_SSL_SEND_CMD;
 	/*--------------check cacer file end-----------------*/

	/*--------------check network start-----------------*/
	
	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QIACT?" );
	AT_SSL_SEND_CMD;
	
 	rt_thread_mdelay(10);
	
	if (at_resp_parse_line_args(resp, 2, "+QIACT: %*d,%d", &pdp_status) <= 0) {
        LOG_E("Failed to parse AT+QIACT? response\n");
		pdp_status = 0;
    }

	PRINTF_RESP(resp);
	LOG_E("pdp status is %d\n", pdp_status);
 	
    if (pdp_status != 1) {
        LOG_E("PDP context is not active, cannot proceed\n");
		pdp_status = 0;
        //return;
    }

	// ���� PDP ������ (�����Ҫ)
    if (pdp_status == 0) {
		
		memset(send_cmd, 0, sizeof(send_cmd));
		snprintf(send_cmd, sizeof(send_cmd), "AT+QIACT=1");
		AT_SSL_SEND_CMD;
		
		rt_thread_mdelay(10);
    }
	/*--------------check network end-----------------*/

	/*--------------config ssl start-----------------*/
	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QSSLCFG=\"sslversion\",1,4");
	AT_SSL_SEND_CMD;

	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QSSLCFG=\"ciphersuite\",1,0XFFFF");
	AT_SSL_SEND_CMD;

	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QSSLCFG=\"seclevel\",1,0");
	AT_SSL_SEND_CMD;

	
	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QSSLCFG=\"cacert\",1,\"UFS:%s\"", cacert_filename);
	AT_SSL_SEND_CMD;	
	/*--------------config ssl end-----------------*/


	/*--------------ssl handle start-----------------*/
	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QSSLOPEN=1,1,4,\"%s\",%d,0", serveraddr, server_port);
	AT_SSL_SEND_CMD;
	
	if(rt_sem_take(_ql_at_sem._qsslopen, 60000) != RT_EOK)
	{
		LOG_E("QSSLOPEN no urc, timeout\n");
		goto ssl_end;
	}

	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QSSLSTATE");
	AT_SSL_SEND_CMD;

	ret = true;

	/*--------------ssl handle end-----------------*/

ssl_end:

	if(ret == false)
	{
		at_ssl_close(client);
	}
	return ret;
}


bool at_ssl_send(at_client_t client, const char* data, size_t data_len)
{
	char send_cmd[32] = {0};

	if(!(client && resp && data && data_len))
	{
		return false;
	}
	
	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QSSLSEND=4,%d", data_len);
	AT_SEND_CMD(client, resp, send_cmd, return false);
	
	if (at_client_obj_send(client, data, data_len) < 0) {
		LOG_E("Failed to send ssl data\n");
		return false;
	}
	return true;
}

bool at_ssl_check(at_client_t client)
{
	bool ret = false;
	
	if(!(client && resp))
	{
		return false;
	}

	
	if(rt_sem_take(_ql_at_sem._qsslurc, 8000) != RT_EOK)
	{
		LOG_E("QIURC no urc, timeout.\n");
		return false; 
	}
	
    if (at_obj_exec_cmd(client,resp, "AT+QSSLRECV=4,1024") < 0) {
        LOG_E("Failed to read server response\n");
    } else {
    	PRINTF_RESP(resp);
        char response_line[1024];
        if (at_resp_parse_line_args(resp, 3, "%[^\r\n]", response_line) > 0) {
            LOG_E("Server response: %s\n", response_line);

			if(!strstr(response_line, "200 OK"))
			{
				LOG_E("response is failed\n");
			} else 
			{
				ret = true;
			}
			
        } else {
            LOG_E("Failed to parse server response\n");
        }
    }
	
	return ret;
}

bool at_ssl_close(at_client_t client)
{
	if(!(client && resp))
	{
		return false;
	}
		
	if (at_obj_exec_cmd(client, resp, "AT+QSSLCLOSE=4") < 0) {
        LOG_E("Failed to execute AT+QSSLCLOSE=4\n");
    }
	
	return true;
}


bool at_ssl_cacert_save(at_client_t client, const char* cacert_name, const char* cacert_data, size_t len)
{
	
	char send_cmd[1024] = {0};
	bool ret = false;
	int save_len = 0, crc = 0;
	at_response_t resp1 = NULL;

	if(!(client && cacert_name && cacert_data && len != 0 && resp))
	{
		LOG_E("para error , please check input para\n");
	}
	
	resp1 = at_create_resp(RESP_BUFFER_SIZE, 2, 3000);
	if (!resp1) {
        LOG_E("Failed to create AT response object\n");
        goto cacert_ret;
    }

	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QFDEL=\"UFS:%s\"", cacert_name);
	if (at_obj_exec_cmd(client, resp, send_cmd) < 0)
	{
		LOG_E("Failed to execut %s\n",send_cmd);
	}

	
	memset(send_cmd, 0, sizeof(send_cmd));
	snprintf(send_cmd, sizeof(send_cmd), "AT+QFUPL=\"%s\",%d,100,0", cacert_name, len);
	if (at_obj_exec_cmd(client, resp1, send_cmd) < 0)
	{
		LOG_E("Failed to execut %s\n",send_cmd);
		goto cacert_ret;
	}
	
	if (at_client_obj_send(client, cacert_data, len) < 0) {
		LOG_E("Failed to send ssl data\n");
		goto cacert_ret;
	}

	if(rt_sem_take(_ql_at_sem._qfupl, 8000) != RT_EOK)
	{
		LOG_E("QFUPL no urc, timeout\n");
		goto cacert_ret;
	}	
		
	ret = true;
	
cacert_ret:
	
	if(resp1)
	{
		at_delete_resp(resp1);
		resp1 = NULL;
	}
	return ret;
}


static int creat_fs(char* filename, char *date) {
    // ���ļ���ʹ�� "w" ģʽ��ʾд�루����ļ��������򴴽�����������գ�
    FILE *file = fopen(filename, "w");
	int i = 0;
    
    // ����ļ��Ƿ�ɹ���
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }
    
    // Ҫд�������
    const char *content = date;
    
    // ʹ�� fwrite д�����ݵ��ļ�
    size_t result = 0;
	size_t count = 0;

	result = fwrite("start", sizeof(char), 5, file);

	do{
		result = fwrite(content, sizeof(char), strlen(content), file);
		count += result;
		if(count > 10240)
		{
			break;
		}
	}while(1);
    
    result = fwrite("end", sizeof(char), 3, file);
    
//    // ����Ƿ�ɹ�д��
//    if (result < strlen(content)) {
//        perror("Failed to write to file");
//        fclose(file);
//        return 1;
//    }
    
    // �ر��ļ�
    fclose(file);
    
    LOG_E("File 'felix.txt' created and content written successfully.\n");
    
    return 0;
}

#define QTH_AWS_X509_CA "-----BEGIN CERTIFICATE-----\r\n" \
                    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n" \
                    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n" \
                    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n" \
                    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n" \
                    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n" \
                    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n" \
                    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n" \
                    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n" \
                    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n" \
                    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n" \
                    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n" \
                    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n" \
                    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n" \
                    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n" \
                    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n" \
                    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n" \
                    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n" \
                    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n" \
                    "-----END CERTIFICATE-----"


#define QST_SSL_CA "-----BEGIN CERTIFICATE-----\r\n" \
				"MIIEtDCCA5ygAwIBAgIJAL/pgY5ZFAJZMA0GCSqGSIb3DQEBCwUAMIGMMQswCQYD\r\n" \
				"VQQGEwJDTjEOMAwGA1UECBMFQW5IdWkxDjAMBgNVBAcTBUhlRmVpMRAwDgYDVQQK\r\n" \
				"EwdRdWVjdGVsMQswCQYDVQQLEwJTVDEWMBQGA1UEAxMNMTEyLjMxLjg0LjE2NDEm\r\n" \
				"MCQGCSqGSIb3DQEJARYXam9tYW4uamlhbmdAcXVlY3RlbC5jb20wHhcNMjIwMjA5\r\n" \
				"MDYwOTAwWhcNMzIwMjA3MDYwOTAwWjCBjDELMAkGA1UEBhMCQ04xDjAMBgNVBAgT\r\n" \
				"BUFuSHVpMQ4wDAYDVQQHEwVIZUZlaTEQMA4GA1UEChMHUXVlY3RlbDELMAkGA1UE\r\n" \
				"CxMCU1QxFjAUBgNVBAMTDTExMi4zMS44NC4xNjQxJjAkBgkqhkiG9w0BCQEWF2pv\r\n" \
				"bWFuLmppYW5nQHF1ZWN0ZWwuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\r\n" \
				"CgKCAQEAxPA3lJd/HwTtgDu8tgOIemI6LiFySsDhZvZf3jh/zatJkb6U8AdTLbHc\r\n" \
				"r/aDUgEKhzpxTfuB4JLRqF+/geKq9k89ulmmC+1V7AK1TihR+aVXNsbSuXSMaN3m\r\n" \
				"zfNH42WE/4EgaASGrZaHaWh33Cv6VUAq6O9SVgMrdvPn4fjvk+py69nQ++qvsGFK\r\n" \
				"8i1GOsi6wv69v+f+o0IfrDkQpVx/1+RBmL981ZpjAOBDNaJherx845ZLHTOTPXhJ\r\n" \
				"7XFMGbBtSRnjJtGGtWk028FE84UDnQ2CkAhb1VJvbxUeGhxqCxBSEaSqjPBh7UO1\r\n" \
				"rAXQ+0VBsM2u4tJDskTpIo09x/oKwQIDAQABo4IBFTCCAREwDAYDVR0TBAUwAwEB\r\n" \
				"/zAdBgNVHQ4EFgQUdgLJhI16sNJZF9awAJkeb3TBMW8wgcEGA1UdIwSBuTCBtoAU\r\n" \
				"dgLJhI16sNJZF9awAJkeb3TBMW+hgZKkgY8wgYwxCzAJBgNVBAYTAkNOMQ4wDAYD\r\n" \
				"VQQIEwVBbkh1aTEOMAwGA1UEBxMFSGVGZWkxEDAOBgNVBAoTB1F1ZWN0ZWwxCzAJ\r\n" \
				"BgNVBAsTAlNUMRYwFAYDVQQDEw0xMTIuMzEuODQuMTY0MSYwJAYJKoZIhvcNAQkB\r\n" \
				"Fhdqb21hbi5qaWFuZ0BxdWVjdGVsLmNvbYIJAL/pgY5ZFAJZMAsGA1UdDwQEAwIB\r\n" \
				"BjARBglghkgBhvhCAQEEBAMCAAcwDQYJKoZIhvcNAQELBQADggEBAB2eFfablKpx\r\n" \
				"uM3xaZ4RmGjjBFclPKDdW/qGNMeCq1IvBZ5Hd7R/FjUT3STojUFAMW+LRwbV5v7f\r\n" \
				"RQGZmlavQQNmA75Clwt9XD8oob9KnfMu8JQ9RB99xptu9DfYCxXYTFavSjzN3Tkg\r\n" \
				"i8d/6itOQI0ISQtvtOKhCbzT2Nrv5meegOrB3utdpEZ5FkHGmdVPTnj2bhRUPCYR\r\n" \
				"lLXxvA2+Rcaw7LcgIIdsv47rFCsLO9bkaCNAvGw8LUVEP3TbtvxfW6uKjrMErUox\r\n" \
				"m5Ny+VnmlWN7Kcb8o4jwkwTXPa57fumkhRorgWO5f6+A8gjHjfpWK8Ew8xfd3T1W\r\n" \
				"Xh270MLwxk4=\r\n" \
				"-----END CERTIFICATE-----"



#define CHECK_FUNC_RUN(X) do{	\
	if(true != X) {				\
		LOG_E("func: %s, line:%d failed\n",__func__, __LINE__);	\
		return -1;			\
	}							\
}while(0)

int example_at_ssl(void)
{

	
	CHECK_FUNC_RUN(at_ssl_client_init());


	at_client_t client1 = at_client_get("uart1");

	CHECK_FUNC_RUN(at_ssl_cacert_save(client1, "cacert_st.pem", QST_SSL_CA, strlen(QST_SSL_CA)));

	CHECK_FUNC_RUN(at_ssl_connect(client1, "cacert_st.pem", "112.31.84.164", 8381));

	CHECK_FUNC_RUN(at_ssl_send(client1, "0123456789", 10));

	CHECK_FUNC_RUN(at_ssl_check(client1));

	CHECK_FUNC_RUN(at_ssl_close(client1));

	CHECK_FUNC_RUN(at_ssl_client_deinit());

	return 0;
}


int example_at_ssl1(void)
{
    /* ��ʼ����� AT Client ʵ�� */
//	creat_fs("felix.txt", "this is felix file test");

	
	CHECK_FUNC_RUN(at_ssl_client_init());


    /* ��ȡ�ͻ���ʵ�� */
    at_client_t client1 = at_client_get("uart1");

	CHECK_FUNC_RUN(at_ssl_cacert_save(client1, "cacert1.pem", QTH_AWS_X509_CA, strlen(QTH_AWS_X509_CA)));

	CHECK_FUNC_RUN(at_ssl_connect(client1, "cacert1.pem", "quecs3demo.s3.eu-central-1.amazonaws.com", 443));

	CHECK_FUNC_RUN(at_ssl_send(client1, "0123456789", 10));

	CHECK_FUNC_RUN(at_ssl_check(client1));

	CHECK_FUNC_RUN(at_ssl_close(client1));

	CHECK_FUNC_RUN(at_ssl_client_deinit());

    return 0;
}

rt_err_t cat1_wait_rdy()
{
    return rt_sem_take(_ql_at_sem._rdy, rt_tick_from_millisecond(15000));
}

rt_err_t cat1_wait_powered_down()
{
    return rt_sem_take(_ql_at_sem._powered_down, rt_tick_from_millisecond(15000));
}

rt_err_t cat1_qpowd()
{
    // cat1 "uart1"
    at_client_t client = RT_NULL;
    rt_err_t result = RT_EOK;

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(5000));
    if (resp == RT_NULL) {
        LOG_D("no enought mem for cat1 at resp");
        return RT_ERROR;
    }

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        at_client_init("uart1", 512, 512);
        client = at_client_get("uart1");
    }

    result = at_obj_exec_cmd(client, resp, "AT+QPOWD");
    LOG_D("at_obj_exec_cmd result: %d", result);
	return result;
}

rt_err_t cat1_check_state()
{
    at_client_t client = RT_NULL;
    rt_err_t result = RT_EOK;

    at_response_t resp = at_create_resp(512, 0, rt_tick_from_millisecond(1000));
    if (resp == RT_NULL) {
        LOG_D("no enought mem for cat1 at resp");
        return RT_ERROR;
    }

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        at_client_init("uart1", 512, 512);
        client = at_client_get("uart1");
    }

    for (int i=0; i < 10; i++) {
        result = at_obj_exec_cmd(client, resp, "AT");
        LOG_D("at_obj_exec_cmd result: %d", result);
        if (result == RT_EOK) {
            break;
        }
    }
	return result;
}

rt_err_t cat1_set_cfun_mode(int mode)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        LOG_E("nbiot at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        LOG_D("create resp failed.");
        return RT_ERROR;
    }

    char s[20] = {0};
    snprintf(s, 20, "AT+CFUN=%d", mode);
    result = at_obj_exec_cmd(client, resp, s);
    if (result != RT_EOK) {
        LOG_E("nbiot cfun0 err: %d", result);
    }

    at_delete_resp(resp);
    return result;
}

rt_err_t cat1_check_network(int retry_times)
{
    int n = -1;
    int stat = -1;
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        LOG_E("cat1 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        LOG_D("create resp failed.");
        return RT_ERROR;
    }

    for (int i=0; i < retry_times; i++) {
        result = at_obj_exec_cmd(client, resp, "AT+CEREG?");
        at_resp_parse_line_args(resp, 2, "+CEREG: %d,%d", &n, &stat);
        LOG_D("cat1 check status, n: %d, stat: %d", n, stat);
        if (stat == 1 || stat == 5) {
            at_delete_resp(resp);
            return RT_EOK;
        }
        rt_thread_mdelay(5000);
    }

    at_delete_resp(resp);
    return RT_ERROR;
}

rt_err_t cat1_enable_echo(int enable)
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        LOG_E("cat1 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        LOG_E("create resp failed.");
        return RT_ERROR;
    }

    // disable at echo
    result = at_obj_exec_cmd(client, resp, enable ? "ATE1" : "ATE0");
    if (result != RT_EOK) {
        LOG_E("cat1 disable at echo failed: %d", result);
    }

    at_delete_resp(resp);
    return result;
}

rt_err_t cat1_set_band()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        LOG_E("cat1 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        LOG_E("create resp failed.");
        return RT_ERROR;
    }

    // 锁 4G 网络
    result = at_obj_exec_cmd(client, resp, "AT*BAND=5");
    if (result != RT_EOK) {
        LOG_E("cat1 disable at AT*BAND=5 failed: %d", result);
    }

    at_delete_resp(resp);
    return result;
}

rt_err_t cat1_set_network_config()
{
    rt_err_t result = RT_EOK;
    at_client_t client = RT_NULL;

    client = at_client_get("uart1");
    if (client == RT_NULL) {
        LOG_E("cat1 at client not inited!");
        return RT_ERROR;
    }

    at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(3000));
    if (resp == RT_NULL) {
        LOG_D("create resp failed.");
        return RT_ERROR;
    }

    // 先检查下 AT+QGMR 确认模组上面的固件是可以上云的包含 QTH字样的固件
    result = at_obj_exec_cmd(client, resp, "AT+QGMR");
    if (result != RT_EOK) {
        LOG_D("at_obj_exec_cmd AT+QEREG=2 failed");
        goto ERROR;
    }

    result = at_obj_exec_cmd(client, resp, "AT+CEREG=2");
    if (result != RT_EOK) {
        LOG_D("at_obj_exec_cmd AT+QEREG=2 failed");
        goto ERROR;
    }
  
    result = at_obj_exec_cmd(client, resp, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1", "QUECTEL.VF.STD");
    if (result != RT_EOK) {
        LOG_E("set apn result: %d", result);
        goto ERROR;
    }
    LOG_D("set apn success");

    result = at_obj_exec_cmd(client, resp, "AT+QIACT?");
    if (result != RT_EOK) {
        LOG_E("at_obj_exec_cmd AT+QIACT?: %d", result);
        goto ERROR;
    }
    LOG_D("set AT+QIACT? success");

ERROR:
    at_delete_resp(resp);
    return result;
}
