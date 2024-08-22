

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <rtdbg.h>
#include "at_client_http.h"
#include <sys/stat.h>
#include "rtthread.h"

#define BUFFER_CHUNK_SIZE 1024
#define RESP_BUFFER_SIZE 512


#define PRINTF_RESP(X) do{	\
	if(X && X->buf){		\
		LOG_I("resp_buf-> lene:%d : data:%s\n",__LINE__, X->buf);	\
	}	\
}while(0);


#define AT_HTTP_SEND(X, Y, Z, W) do{			\
	if(false == at_http_send_data(X, Y, Z, W))	\
	{											\
		goto err;								\
	}											\
}while(0)


typedef struct
{
	rt_sem_t _qiopen;
	rt_sem_t _qiurc;
} qat_sem_s;

qat_sem_s _ql_at_sem = {0};


static void urc_func(struct at_client *client ,const char *data, rt_size_t size);





static struct at_urc urc_table[] = {
    {"+QIOPEN:",   	"\r\n",     	urc_func},
    {"+QIURC:", 		"\r\n",		urc_func},
    
};


static void urc_func(struct at_client *client ,const char *data, rt_size_t size)
{
    LOG_E("urc data : %s, size:%d\n", data, size);
	if(strncmp(data, "+QIOPEN", strlen("+QIOPEN")) == 0 && data[11] == '0')
	{
		LOG_E("send _qiopen sem\n");
		rt_sem_release(_ql_at_sem._qiopen);
	} else if (strncmp(data, "+QIURC", strlen("+QIURC")) == 0)
	{
		LOG_E("send _qiurc sem\n");
		rt_sem_release(_ql_at_sem._qiurc);
	}
}



void at_multi_client_init(void)
{
    /* 初始化第一个 AT Client */
	if(RT_EOK != at_client_init("uart1", RESP_BUFFER_SIZE, RESP_BUFFER_SIZE))
	{
		LOG_E("AT Client1 (uart1) initialize failed\n");
	}

	_ql_at_sem._qiopen = rt_sem_create("iopen_sem", 0, RT_IPC_FLAG_PRIO);
	_ql_at_sem._qiurc = rt_sem_create("iurc_sem", 0, RT_IPC_FLAG_PRIO);
	
	
    LOG_I("AT Client1 (uart1) initialized success\n");
	at_obj_set_urc_table(at_client_get("uart1"), urc_table, sizeof(urc_table) / sizeof(urc_table[0]));
}


static bool at_http_send_data(at_client_t client, at_response_t resp, const void* data, size_t data_size)
{
	
	char qisend_data[64] = {0};
	snprintf((char*)&qisend_data,sizeof(qisend_data), "AT+QISEND=%d,%d",1, data_size);

	if (at_obj_exec_cmd(client,resp, qisend_data) < 0) {
		 LOG_E("qisend failed\n");
		 return false;
	 }

	 rt_thread_mdelay(10);
	 
	if (at_client_obj_send(client, data, data_size) < 0) {
        LOG_E("Failed to send HTTP header\n");
        return false;
    }
	return true;
	

}



int at_http_upload_file_chunked(at_client_t client, const char *filename)
{
    FILE *file = NULL;
	int ret = -1;
    char buffer[BUFFER_CHUNK_SIZE];
    char header[1024];
    int bytes_read;
    int pdp_status = 0;
	int ip_channel = 0;
    at_response_t resp = NULL;
	at_response_t send_resp = NULL;
	char qisend_data[64] = {0};
	char content_data[1024] = {0}; // for Content-Length
    const char *boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

	at_set_end_sign('>');


    /* 创建 AT Response 对象 */
    resp = at_create_resp(RESP_BUFFER_SIZE, 0, 10000);
	if (!resp) {
        LOG_E("Failed to create AT response object\n");
        return -1;
    }
	send_resp = at_create_resp(RESP_BUFFER_SIZE, 1, 10000000);
	if (!send_resp) {
        LOG_E("Failed to create AT send response object\n");
        return -1;
    }
	
	// 检查网络注册状态
    if (at_obj_exec_cmd(client, resp, "AT+QIACT?") < 0) {
        LOG_E("Failed to execute AT+QIACT?\n");
        goto err;
    }

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

	// 激活 PDP 上下文 (如果需要)
    if (pdp_status == 0) {
        if (at_obj_exec_cmd(client,resp, "AT+QIACT=1") < 0) {
            LOG_E("Failed to activate PDP context\n");
            goto err;
        }
		rt_thread_mdelay(10);
    }

//	if (at_obj_exec_cmd(client,resp, "AT+QICLOSE=1") < 0) {
//		LOG_E("Failed to close TCP connection\n");
//		at_delete_resp(resp);
//	}

	//打开 TCP 连接
    if (at_obj_exec_cmd(client,resp, "AT+QIOPEN=1,1,\"TCP\",\"112.31.84.164\",8300,0,0") < 0) {
        LOG_E("Failed to open TCP connection\n");
        goto err;
    }

	if(rt_sem_take(_ql_at_sem._qiopen, 1000000) != RT_EOK)
	{
		LOG_E("QIOPEN no urc, timeout\n");
		goto err;
	}


	// 检查 TCP 连接是否成功
//    if (at_resp_parse_line_args(resp, 1, "+QIOPEN: %d,%d",&ip_channel, &pdp_status) <= 0 || pdp_status != 0) {
//        LOG_E("Failed to open TCP connection, status: %d\n", pdp_status);
//		PRINTF_RESP(resp);
//        at_delete_resp(resp);
//        return;
//    }

	file = fopen(filename, "rb");
    if (!file) {
        LOG_E("Failed to open file: %s\n", filename);
        goto err;
    }

	fseek(file, 0, SEEK_END);
	long file_length = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	
	snprintf((char*)&content_data, sizeof(content_data), 
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"\r\n--%s--\r\n", 
		boundary, filename, boundary
	);
	

	// 6. 构建并发送 HTTP 报文头部
    snprintf(header, sizeof(header),
             "POST /upload.php HTTP/1.1\r\n"
             "Host: 112.31.84.164:8300\r\n"
             "Authorization: Basic dGVzdDp0ZXN0\r\n"
             "Content-Length: %d\r\n"
             "Content-Type: multipart/form-data; boundary=%s\r\n"
             "\r\n"
             "--%s\r\n"
             "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
             "Content-Type: text/plain\r\n"
             "\r\n",
             strlen(content_data)+file_length, boundary, boundary, filename);

	memset(buffer, 0, BUFFER_CHUNK_SIZE);
	snprintf(buffer, sizeof(buffer), "\r\n--%s--\r\n", boundary);
	//snprintf(&qisend_data,sizeof(qisend_data), "AT+QISEND=%d,%d",1, strlen(header) + file_length + strlen(buffer));
	snprintf((char*)&qisend_data,sizeof(qisend_data), "AT+QISEND=%d,%d",1, strlen(header));

	
	LOG_I("send2 %d : %s\n", strlen(qisend_data), qisend_data);

	AT_HTTP_SEND(client, send_resp, header, strlen(header));

	
	 // 7. 分包读取文件并发送
    while ((bytes_read = fread(buffer, 1, BUFFER_CHUNK_SIZE, file)) > 0) {
		AT_HTTP_SEND(client, send_resp, buffer, bytes_read);
    }


	if (ferror(file)) {
        LOG_E("Error reading from file: %s\n", filename);
        goto err;
    }

	// 8. 发送结束的 boundary 和 HTTP 报文尾部
    snprintf(buffer, sizeof(buffer), "\r\n--%s--\r\n", boundary);
	AT_HTTP_SEND(client, send_resp, buffer, strlen(buffer));

	if(rt_sem_take(_ql_at_sem._qiurc, 10000000) != RT_EOK)
	{
		LOG_E("QIURC no urc, timeout.\n");
		goto err;
	}


	// 9. 读取服务器响应
    if (at_obj_exec_cmd(client,resp, "AT+QIRD=1") < 0) {
        LOG_E("Failed to read server response\n");
    } else {
        char response_line[256];
        if (at_resp_parse_line_args(resp, 3, "%[^\r\n]", response_line) > 0) {
            LOG_E("Server response: %s\n", response_line);

			if(!strstr(response_line, "200 OK"))
			{
				LOG_E("response is failed\n");
				goto err;
			}
			
        } else {
            LOG_E("Failed to parse server response\n");
			goto err;
        }
    }

	ret = 0;
err:

	if (at_obj_exec_cmd(client,resp, "AT+QICLOSE=1") < 0) {
		LOG_E("Failed to close TCP connection\n");
	}

	if(resp)
	{
		at_delete_resp(resp);
		resp = NULL;
	}

	if(send_resp)
	{
		at_delete_resp(send_resp);
		send_resp = NULL;
	}

	if(file)
	{
		fclose(file);
		file = NULL;
	}

	

}


static int creat_fs(char* filename, char *date) {
    // 打开文件，使用 "w" 模式表示写入（如果文件不存在则创建，存在则清空）
    FILE *file = fopen(filename, "w");
	int i = 0;
    
    // 检查文件是否成功打开
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }
    
    // 要写入的内容
    const char *content = date;
    
    // 使用 fwrite 写入内容到文件
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
    
//    // 检查是否成功写入
//    if (result < strlen(content)) {
//        perror("Failed to write to file");
//        fclose(file);
//        return 1;
//    }
    
    // 关闭文件
    fclose(file);
    
    LOG_E("File 'felix.txt' created and content written successfully.\n");
    
    return 0;
}


int example_at_http(void)
{
    /* 初始化多个 AT Client 实例 */
	creat_fs("felix.txt", "this is felix file test");

    at_multi_client_init();

    /* 获取客户端实例 */
    at_client_t client1 = at_client_get("uart1");

    /* 传入要上传的文件名 */
    at_http_upload_file_chunked(client1, "felix.txt");
	at_http_upload_file_chunked(client1, "felix.txt");
	at_http_upload_file_chunked(client1, "felix.txt");

    return 0;
}



