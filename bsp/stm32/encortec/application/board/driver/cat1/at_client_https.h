#ifndef __AT_CLIENT_HTTPS__
#define __AT_CLIENT_HTTPS__

#define AWS_ACCESS_KEY_ID "xxxxxxxxxxxxxxxx"
#define AWS_SECRET_ACCESS_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxx"
#define AWS_REGION "eu-central-1"
#define AWS_SERVICE "s3"
#define AWS_BUCKET "iot-s3-2162346229"
#define AWS_HOST "iot-s3-2162346229.s3.eu-central-1.amazonaws.com"

int at_https_upload_file(const char *filename);
int at_https_open();
int at_https_close();

#endif
