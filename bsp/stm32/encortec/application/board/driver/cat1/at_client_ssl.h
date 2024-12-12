#ifndef __AIC_CLIENT_SSL_H__
#define __AIC_CLIENT_SSL_H__

#include "at.h"
#include <stdbool.h>

bool at_ssl_client_init(void);
bool at_ssl_client_deinit(void);
bool at_ssl_connect(at_client_t client, const char *cacert_filename, const char*serveraddr, int server_port);
bool at_ssl_send(at_client_t client, const char* data, size_t data_len);
bool at_ssl_check(at_client_t client);
bool at_ssl_close(at_client_t client);
bool at_ssl_cacert_save(at_client_t client, const char* cacert_name, const char* cacert_data, size_t len);
int example_at_ssl(void);

rt_err_t cat1_qpowd();
rt_err_t cat1_set_cfun_mode(int mode);
rt_err_t cat1_check_state();
rt_err_t cat1_wait_rdy();
rt_err_t cat1_wait_powered_down();
rt_err_t cat1_check_network(int retry_times);
rt_err_t cat1_enable_echo(int enable);
rt_err_t cat1_set_band();
rt_err_t cat1_set_network_config();

#endif
