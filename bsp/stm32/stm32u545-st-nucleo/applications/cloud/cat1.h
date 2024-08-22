#ifndef __AIC_CLIENT_HTTP_H__
#define __AIC_CLIENT_HTTP_H__

#include "at.h"
#include <string.h>

void cat1_at_client_init(void);
rt_err_t cat1_qpowd();
rt_err_t cat1_set_cfun_mode(int mode);
rt_err_t cat1_check_state();
rt_err_t cat1_wait_rdy();
rt_err_t cat1_check_network(int retry_times);
rt_err_t cat1_enable_echo(int enable);

int at_http_upload_file_chunked(const char *filename);
int example_at_http(void);



#endif
