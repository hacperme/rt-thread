#ifndef __AIC_CLIENT_HTTP_H__
#define __AIC_CLIENT_HTTP_H__

#include "at.h"


int at_http_upload_file_chunked(at_client_t client, const char *filename);
int example_at_http(void);



#endif
